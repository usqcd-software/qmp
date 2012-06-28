#define QSPI_INTER
#include "qspi_internal.h"

static MUSPI_InjFifoSubGroup_t inj_sg;
static  uint32_t subgrpid;
static Kernel_MemoryRegion_t fifo_mem[FIFO_MAX];
static  void *fifo_buf[FIFO_MAX];
static  MUSPI_BaseAddressTableSubGroup_t bat_counter_recvbuf;
static int m_count;
//static Kernel_MemoryRegion_t  counter_mem[FIFO_MAX], rbuf_mem[FIFO_MAX];
static int batID;
//static int injed_desc[FIFO_MAX];
static int nfifos;

static int shmfd, shmsize;
static char shmfile[32];
static void *shmptr;

static int
blockhash(void)
{
  uint32_t b = Kernel_BlockThreadId();
  return b/68; // remove thread part
}

static void *
shmopen(int size)
{
  shmsize = size;
  int h = blockhash();
  snprintf(shmfile, sizeof(shmfile), "/qspi%i", h);
  //fprintf(stderr, "%i: using shmfile %s\n", Kernel_GetRank(), shmfile);
  shmfd = shm_open(shmfile, O_RDWR, 0600);
  if(shmfd<0) {
    printf("shmfd = %i at %s: %i\n", shmfd, __func__, __LINE__);
    exit(shmfd);
  }
  int rc = ftruncate(shmfd, size);
  CHECKRC(0);
  shmptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
  if(shmptr==NULL) {
    printf("shmptr = %p at %s: %i\n", shmptr, __func__, __LINE__);
    exit(shmfd);
  }
  return shmptr;
}

static void
shmfree(void)
{
  munmap(shmptr, shmsize);
  close(shmfd);
  shm_unlink(shmfile);
}

void
qspi_init(void)
{
  int32_t rc;

  int nt = Kernel_ProcessCount();
#define SET(x) case x: nfifos = NFIFOS##x; break
  switch(nt) {
    SET(1);
    SET(2);
    SET(4);
    SET(8);
    SET(16);
    SET(32);
    SET(64);
  }
#undef SET

  Kernel_InjFifoAttributes_t fifoAttrs[nfifos];
  uint32_t fifoids[nfifos];
  size_t fifo_buf_size;
  uint32_t nbat, bats[4];

  volatile int *shm = (volatile int *) shmopen(sizeof(int));
  int t = Kernel_MyTcoord();
  //if(t==0 && shm[0]!=0) fprintf(stderr, "%i: shm[0] = %i\n", Kernel_GetRank(), shm[0]);
  while(shm[0]!=t);
  //if(Kernel_GetRank()==0) printf("%i: shm[0] %i\n", t, shm[0]);

  subgrpid = Kernel_MyTcoord()*64/Kernel_ProcessCount();
  batID = subgrpid*BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP + BAT_DEFAULT;

  for(int i=0; i<nfifos; i++) fifoids[i] = i;
  m_count = 0;

  for(int i=0; i<nfifos; i++) {
    fifoAttrs[i].RemoteGet = 0;
    fifoAttrs[i].System = 0;
    fifoAttrs[i].Priority = 0;
  }

  rc = Kernel_AllocateInjFifos(subgrpid, &inj_sg, nfifos, fifoids, fifoAttrs);
  CHECKRC(0);

  fifo_buf_size = FIFO_SIZE_LOCAL;
  for(int i=0; i<nfifos; i++) {
    rc = posix_memalign(&fifo_buf[i], 64, fifo_buf_size);
    CHECKRC(0);
    rc = Kernel_CreateMemoryRegion(&fifo_mem[i], fifo_buf[i], fifo_buf_size);
    CHECKRC(0);
    rc = Kernel_InjFifoInit(&inj_sg, fifoids[i], &fifo_mem[i], (uint64_t)fifo_buf[i] - (uint64_t)fifo_mem[i].BaseVa, fifo_buf_size-1);
    CHECKRC(0);
  }

  rc = Kernel_InjFifoActivate(&inj_sg, nfifos, fifoids, KERNEL_INJ_FIFO_ACTIVATE);
  CHECKRC(0);

  nbat = 1;
  bats[0] = BAT_DEFAULT;
  bats[1] = 1;
  bats[2] = 2;
  bats[3] = 3;
  rc = Kernel_AllocateBaseAddressTable(subgrpid, &bat_counter_recvbuf, nbat, bats, 0);
  CHECKRC(0);

  rc = MUSPI_SetBaseAddress(&bat_counter_recvbuf, bats[0], 0x00);
  CHECKRC(0);
  //rc = MUSPI_SetBaseAddress(&bat_counter_recvbuf, bats[1], 0x00);
  //CHECKRC(0);
  //rc = MUSPI_SetBaseAddress(&bat_counter_recvbuf, bats[2], 0x00);
  //CHECKRC(0);
  //rc = MUSPI_SetBaseAddress(&bat_counter_recvbuf, bats[3], 0x00);
  //CHECKRC(0);

  shm[0]++;
  //if(Kernel_GetRank()==0) printf("%i: shm[0] %i\n", t, shm[0]);
  sleep(1);
  shmfree();
}

void
qspi_set_send(int dest, void *buf, size_t size, qspi_msg_t send_msg)
{
  int32_t rc;
  MUHWI_Descriptor_t *desc;
  int destcoords[NDIM];
  struct qspi_msg_t from_recv_msg;
  MPI_Status status;

  rank2coords(dest, destcoords);


  MPI_Recv(&from_recv_msg,sizeof(struct qspi_msg_t),MPI_BYTE,dest,0,MPI_COMM_WORLD,&status);
//TRACE;
  send_msg->subgrpid = subgrpid;
  send_msg->flag = QSPI_SEND;
  send_msg->counter_offset = from_recv_msg.counter_offset;
  send_msg->recvbuf_offset = from_recv_msg.recvbuf_offset;
  send_msg->sender = Kernel_GetRank();
  send_msg->receiver = from_recv_msg.receiver;
  send_msg->msg_size = size;
  send_msg->ndesc = 1;

  if( 
      send_msg->sender != from_recv_msg.sender    ||
      send_msg->receiver != dest || send_msg->msg_size != from_recv_msg.msg_size) {
    printf("ERROR! Parameters don't match \n");
  }

  Kernel_MemoryRegion_t *sbuf_mem;
  rc = posix_memalign((void**)&sbuf_mem, 64, sizeof(Kernel_MemoryRegion_t));
  CHECKRC(0);
  rc = Kernel_CreateMemoryRegion(sbuf_mem, buf, size);
  CHECKRC(0);

  //desc = (MUHWI_Descriptor_t *) malloc(sizeof(MUHWI_Descriptor_t));
  rc = posix_memalign((void**)&desc, 64, sizeof(MUHWI_Descriptor_t));
  CHECKRC(0);
  send_msg->desc = desc;
  send_msg->sbuf_mem = sbuf_mem;

  qspi_set_descriptor(buf, size, destcoords, send_msg);

}


void
qspi_set_send_multi(int dest, void *buf[], size_t size[],int n_msg, qspi_msg_t send_msg)
{
  int32_t rc;
  MUHWI_Descriptor_t *desc;
  int destcoords[NDIM];
  int i;
  struct qspi_msg_t from_recv_msg;
  MPI_Status status;

  rank2coords(dest, destcoords);


  MPI_Recv(&from_recv_msg,sizeof(struct qspi_msg_t),MPI_BYTE,dest,0,MPI_COMM_WORLD,&status);

  send_msg->subgrpid = subgrpid;
  send_msg->flag = QSPI_SEND;
  send_msg->counter_offset = from_recv_msg.counter_offset;
  send_msg->recvbuf_offset = from_recv_msg.recvbuf_offset;
  send_msg->sender = Kernel_GetRank();
  send_msg->receiver = from_recv_msg.receiver;
  send_msg->ndesc = n_msg;

  if( 
      send_msg->sender != from_recv_msg.sender    ||
      send_msg->receiver != dest ) {
    printf("ERROR! Parameters don't match \n");
  }
  rc = posix_memalign((void**)&desc, 64, n_msg*sizeof(MUHWI_Descriptor_t));
  CHECKRC(0);

  Kernel_MemoryRegion_t *sbuf_mem;
  rc = posix_memalign((void**)&sbuf_mem, 64, n_msg*sizeof(Kernel_MemoryRegion_t));
  CHECKRC(0);

  for(i=0;i<n_msg;i++){
    rc = Kernel_CreateMemoryRegion(&sbuf_mem[i], buf[i], size[i]);
    CHECKRC(0);
    send_msg->sbuf_mem = &(sbuf_mem[i]);
  
    if(i!=0){
      send_msg->recvbuf_offset = send_msg->recvbuf_offset + size[i-1];
    }
    send_msg->desc = &(desc[i]);
    send_msg->msg_size = size[i];
    qspi_set_descriptor(buf[i], size[i], destcoords, send_msg);
  }
  send_msg->sbuf_mem = sbuf_mem;
  send_msg->desc = desc;
}


void
qspi_set_descriptor(void *buf, size_t size, int destcoords[],qspi_msg_t msg)
{
  MUSPI_Pt2PtDirectPutDescriptorInfo_t info;
  int local = islocal(destcoords);
  Kernel_MemoryRegion_t *sbuf_mem;
  MUHWI_Descriptor_t *desc;
  int rc;

  sbuf_mem = msg->sbuf_mem;

  info.Base.Pre_Fetch_Only = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
  info.Base.Payload_Address = (uint64_t)sbuf_mem->BasePa+((uint64_t)buf-(uint64_t)sbuf_mem->BaseVa);
  info.Base.Message_Length = size;
  if(local) {
    info.Base.Torus_FIFO_Map = 0x30;
  } else {
    info.Base.Torus_FIFO_Map = 0xffc0;
  }
  info.Base.Dest.Destination.Reserved2 = 0;
  info.Base.Dest.Destination.A_Destination = destcoords[0];
  info.Base.Dest.Destination.B_Destination = destcoords[1];
  info.Base.Dest.Destination.C_Destination = destcoords[2];
  info.Base.Dest.Destination.D_Destination = destcoords[3];
  info.Base.Dest.Destination.E_Destination = destcoords[4];
  info.Pt2Pt.Hints_ABCD = 0;
  info.Pt2Pt.Misc1 = MUHWI_PACKET_USE_DYNAMIC_ROUTING;
  info.Pt2Pt.Misc2 = MUHWI_PACKET_VIRTUAL_CHANNEL_DYNAMIC;
  info.Pt2Pt.Skip = 0;

  info.DirectPut.Rec_Payload_Base_Address_Id = batID;
  info.DirectPut.Rec_Payload_Offset = msg->recvbuf_offset;
  info.DirectPut.Rec_Counter_Base_Address_Id = batID;
  info.DirectPut.Rec_Counter_Offset = msg->counter_offset;
  info.DirectPut.Pacing = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

  desc = msg->desc;

  rc = MUSPI_CreatePt2PtDirectPutDescriptor(desc, &info);

  CHECKRC(0);
}

void
qspi_set_recv(int src, void *buf, size_t size, qspi_msg_t recv_msg)
{
  int32_t rc;
  Kernel_MemoryRegion_t *rbuf_mem = &recv_msg->rbuf_mem;
  Kernel_MemoryRegion_t *counter_mem = &recv_msg->counter_mem;

  recv_msg->counter = (volatile int64_t) size;

  rc = Kernel_CreateMemoryRegion(rbuf_mem, buf, size);
  CHECKRC(0);

  rc = Kernel_CreateMemoryRegion(counter_mem, (void *)&(recv_msg->counter), sizeof(recv_msg->counter));
  CHECKRC(0);

  recv_msg->subgrpid = subgrpid;
  recv_msg->flag = QSPI_RECV;
  recv_msg->counter_offset = MUSPI_GetAtomicAddress((uint64_t)counter_mem->BasePa+((uint64_t)&(recv_msg->counter)-(uint64_t)counter_mem->BaseVa), MUHWI_ATOMIC_OPCODE_STORE_ADD);
  recv_msg->recvbuf_offset = (uint64_t)rbuf_mem->BasePa+((uint64_t)buf-(uint64_t)rbuf_mem->BaseVa);
  recv_msg->sender = src;
  recv_msg->receiver = Kernel_GetRank();
  recv_msg->msg_size = size;

  MPI_Send(recv_msg,sizeof(struct qspi_msg_t),MPI_BYTE,src,0,MPI_COMM_WORLD);
}

void
qspi_start(qspi_msg_t msg)
{
  int flag = msg->flag;
  if(flag == QSPI_SEND) {
    //uint64_t t0 = GetTimeBase();
    uint32_t fifoID = m_count % nfifos;
    m_count = m_count + 1;
    msg->fifoID = fifoID;
    MUSPI_InjFifo_t *inj_fifo = MUSPI_IdToInjFifo(fifoID, &inj_sg);
    MUHWI_Descriptor_t *desc = msg->desc;
    int ndesc = msg->ndesc;
    msg->injed_desc = MUSPI_InjFifoInjectMultiple(inj_fifo, desc, ndesc);
    if(msg->injed_desc<0) {
      for(int i=0; i<ndesc; i++) {
	do {
	  msg->injed_desc = MUSPI_InjFifoInjectMultiple(inj_fifo, desc+i, 1);
	} while(msg->injed_desc<0);
      }
    }
    //uint64_t t1 = GetTimeBase();
    //if(msg->sender==0) printf("inj num: %i  time: %g\n", ndesc, (1./1.6e9)*((double)((t1)-(t0))));
    //printf("injed_desc= %i, fifoID= %i \n",msg->injed_desc,fifoID);
    //TRACE;
  } else if(flag == QSPI_RECV) {
    /* do nothing */
  } else {
    printf("ERROR! flag= %i \n",flag);
  }
}

void
qspi_wait(qspi_msg_t msg)
{
  int flag = msg->flag;
  if(flag == QSPI_SEND) {
    uint32_t fifoID = msg->fifoID;
    MUSPI_InjFifo_t *inj_fifo;
    inj_fifo = MUSPI_IdToInjFifo(fifoID, &inj_sg);
    int injed_desc = msg->injed_desc;
    int ndc;
    do {
      ndc = MUSPI_getHwDescCount(inj_fifo);
    } while(ndc<injed_desc);
    // printf("injed_desc= %i, ndc= %i, fifoID= %i \n",injed_desc,ndc,fifoID);
  } else if(flag == QSPI_RECV) {
    while(msg->counter>0);
    msg->counter = (volatile int64_t) msg->msg_size;
  } else {
    printf("ERROR! flag= %i \n",flag);
  }
}

void
qspi_finalize()
{
  uint32_t nbat, bats[3];
  uint32_t fifoids[nfifos];
  int i,rc;

    /* free recvbuf BAT */
  nbat = 1;
  bats[0] = BAT_DEFAULT;
//  bats[1] = 1;
  rc = Kernel_DeallocateBaseAddressTable(&bat_counter_recvbuf,nbat,bats);
  CHECKRC(0);

  /* free the Inj fifos */
  for(i=0; i<nfifos; i++) fifoids[i] = i;

  rc = Kernel_DeallocateInjFifos(&inj_sg,nfifos,fifoids);
  CHECKRC(0);

  for(i=0; i<nfifos; i++) {
    rc = Kernel_DestroyMemoryRegion(&fifo_mem[i]);
    CHECKRC(0);
    free(fifo_buf[i]);
  }
}

qspi_msg_t  qspi_create_msg(void)
{
  qspi_msg_t msg;

  msg = (qspi_msg_t ) malloc(sizeof(struct qspi_msg_t));

  msg->injed_desc = 0;
  msg->flag = -1;

  return msg;
}

void qspi_free_msg( qspi_msg_t  msg)
{
  if(msg->flag == QSPI_SEND) {
    Kernel_DestroyMemoryRegion(msg->sbuf_mem);
    free(msg->sbuf_mem);
    free(msg->desc);
  }

  if(msg->flag == QSPI_RECV) {
    Kernel_DestroyMemoryRegion(&msg->counter_mem);
    Kernel_DestroyMemoryRegion(&msg->rbuf_mem);
  }

  free(msg);
}


void
rank2coords(int rank, int coords[])
{
  MPIX_Rank2torus(rank, coords);
#define SET(i,x,y) coords[i] += jc.corner.x
  BG_JobCoords_t jc;
  Kernel_JobCoords(&jc);
  SET(0,a,a);
  SET(1,b,b);
  SET(2,c,c);
  SET(3,d,d);
  SET(4,e,e);
  SET(5,core,t);
#undef SET
}

int
islocal(int *coords)
{
  int local = 1;
  int mc[6];
  mycoords(mc);
    int i;
  for(i=0; i<5; i++) {
    if(mc[i]!=coords[i]) { local = 0; break; }
  }
  return local;
}

void
mycoords(int *coords)
{
  rank2coords(Kernel_GetRank(), coords);
}
