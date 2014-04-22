#define QSPI_INTER
#include "qspi_internal.h"

static MUSPI_InjFifoSubGroup_t inj_sg[MAX_SUBGROUP];
static uint32_t subgrpid[MAX_SUBGROUP];
static Kernel_MemoryRegion_t fifo_mem[FIFO_MAX];
static void *fifo_buf[FIFO_MAX];
static MUSPI_BaseAddressTableSubGroup_t bat_counter_recvbuf;
static MUSPI_BaseAddressTableSubGroup_t bat_counter_coll;
static unsigned int m_count;
static int batID, batIDcoll;
static int nsub;
static int myrank;
static int tcoord, cmode;

/* actual number of fifos */
static int NFIFOS;

/* number of fifos per sub-group */
static int FIFO_SUB;

static int shmfd, shmsize;
//static char *shmfile = "/dev/shm/qspi";
static char shmfile[32];
//static void *shmptr;

static struct {
  int count;
  int ranksonnode;
  int done[64];
  double sum[64];
} *shmptr;

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
  shmfd = shm_open(shmfile, O_RDWR|O_CREAT, 0600);
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

static void
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

static void
mycoords(int *coords)
{
  rank2coords(Kernel_GetRank(), coords);
}

static int
islocal(int *coords)
{
  int local = 1;
  int mc[6];
  mycoords(mc);
  for(int i=0; i<5; i++) {
    if(mc[i]!=coords[i]) { local = 0; break; }
  }
  return local;
}

void
qspi_init(void)
{
  int32_t rc;

  /* loop over the sub-groups */
  /* nsub= number of sub-groups in the process.
  for example, for c1 mode, nsub = 64  and nsub=1 for c64 mode*/
  tcoord = Kernel_MyTcoord();
  cmode = Kernel_ProcessCount();
  nsub = 64/cmode; 
  switch(cmode) {
  case 1: FIFO_SUB = TFIFO_SUB1; break;
  case 2: FIFO_SUB = TFIFO_SUB2; break;
  case 4: FIFO_SUB = TFIFO_SUB4; break;
  case 8: FIFO_SUB = TFIFO_SUB8; break;
  case 16: FIFO_SUB = TFIFO_SUB16; break;
  case 32: FIFO_SUB = TFIFO_SUB32; break;
  case 64: FIFO_SUB = TFIFO_SUB64; break;
  default: 
    printf("ERROR: c-mode is unknown!! cmode= %d\n", cmode);
  }

  /* actual number of fifos in the process. This depends on the running mode or # of sub-groups */
  NFIFOS = FIFO_SUB*nsub;

  uint32_t fifoids[FIFO_MAX];
  Kernel_InjFifoAttributes_t fifoAttrs[FIFO_MAX];
  for(int i=0; i<NFIFOS; i++) {
    fifoids[i] = i%FIFO_SUB;
    fifoAttrs[i].RemoteGet = 0;
    fifoAttrs[i].System = 0;
    fifoAttrs[i].Priority = 0;
  }

  //volatile int *shm = (volatile int *) shmopen(128*sizeof(double));
  shmopen(sizeof(*shmptr));
  volatile int *shm = &(shmptr->ranksonnode);
  while(shm[0]!=tcoord);

  for(int i=0; i<nsub; i++) {
    subgrpid[i] = nsub*tcoord + i;
    rc = Kernel_AllocateInjFifos(subgrpid[i], &inj_sg[i], FIFO_SUB, fifoids+i*FIFO_SUB, fifoAttrs+i*FIFO_SUB);
    CHECKRC(0);
  }

  size_t fifo_buf_size = FIFO_SIZE_LOCAL;
  for(int i=0; i<NFIFOS; i++) {
    /*which sub-group? - subi:sub-group index*/
    int subi = (int) i/FIFO_SUB;
    rc = posix_memalign(&fifo_buf[i], 64, fifo_buf_size);
    CHECKRC(0);
    rc = Kernel_CreateMemoryRegion(&fifo_mem[i], fifo_buf[i], fifo_buf_size);
    CHECKRC(0);
    rc = Kernel_InjFifoInit(&inj_sg[subi], fifoids[i], &fifo_mem[i], (uint64_t)fifo_buf[i] - (uint64_t)fifo_mem[i].BaseVa, fifo_buf_size-1);
    CHECKRC(0);
  }

  for(int i=0; i<nsub; i++) {
    rc = Kernel_InjFifoActivate(&inj_sg[i], FIFO_SUB, fifoids+i*FIFO_SUB, KERNEL_INJ_FIFO_ACTIVATE);
    CHECKRC(0);
  }

  uint32_t nbat=1, bats[1];
  bats[0] = BAT_DEFAULT;
  rc = Kernel_AllocateBaseAddressTable(subgrpid[0], &bat_counter_recvbuf, nbat, bats, 0);
  CHECKRC(0);
  rc = MUSPI_SetBaseAddress(&bat_counter_recvbuf, bats[0], 0x00);
  CHECKRC(0);
  batID = subgrpid[0]*BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP + BAT_DEFAULT;

  bats[0] = BAT_DEFAULT + 1;
  rc = Kernel_AllocateBaseAddressTable(subgrpid[0], &bat_counter_coll, nbat, bats, 0);
  CHECKRC(0);
  rc = MUSPI_SetBaseAddress(&bat_counter_coll, bats[0], 0x00);
  CHECKRC(0);
  batIDcoll = subgrpid[0]*BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP + BAT_DEFAULT + 1;

  myrank = Kernel_GetRank();
  m_count = 0;

  _bgq_msync();
  shm[0]++;
  //sleep(2);
  sleep(10);
  //shmfree();
  //printf("%i: shmsize: %i\n", myrank, shmsize);
}

void
qspi_finalize()
{
  int rc;

  //volatile int *shm = (volatile int *) shmopen(sizeof(int));
  volatile int *shm = &(shmptr->count);
  while(shm[0]!=tcoord);

  /* free recvbuf BAT */
  uint32_t nbat=1, bats[1];
  bats[0] = BAT_DEFAULT;
  rc = Kernel_DeallocateBaseAddressTable(&bat_counter_recvbuf, nbat, bats);
  CHECKRC(0);

  /* free the Inj fifos */
  uint32_t fifoids[FIFO_MAX];
  for(int i=0; i<NFIFOS; i++) fifoids[i] = i%FIFO_SUB;
  for(int i=0; i<nsub; i++) {
    rc = Kernel_DeallocateInjFifos(&inj_sg[i], FIFO_SUB, fifoids+i*FIFO_SUB);
    CHECKRC(0);
  }

  for(int i=0; i<NFIFOS; i++) {
    rc = Kernel_DestroyMemoryRegion(&fifo_mem[i]);
    CHECKRC(0);
    free(fifo_buf[i]);
  }

  shm[0]++;
  sleep(2);
  shmfree();
}

void
qspi_set_send_multi(int dest, void *buf[], size_t size[],int n_msg, qspi_msg_t send_msg)
{
  int32_t rc;

  MUHWI_Descriptor_t *desc;
  rc = posix_memalign((void**)&desc, 64, n_msg*sizeof(MUHWI_Descriptor_t));
  CHECKRC(0);

  Kernel_MemoryRegion_t *sbuf_mem;
  rc = posix_memalign((void**)&sbuf_mem, 64, n_msg*sizeof(Kernel_MemoryRegion_t));
  CHECKRC(0);

  size_t *msg_size; 
  rc = posix_memalign((void**)&msg_size, 64, n_msg*sizeof(size_t));
  CHECKRC(0);

  void **buf_tmp;
  rc = posix_memalign((void**)&buf_tmp, 64, n_msg*sizeof(void *));
  CHECKRC(0);

  for(int i=0; i<n_msg; i++) {
    rc = Kernel_CreateMemoryRegion(&sbuf_mem[i], buf[i], size[i]);
    CHECKRC(0);
    msg_size[i] = size[i];
    buf_tmp[i] = buf[i];
  }

  send_msg->flag = QSPI_SEND;
  send_msg->sender = myrank;
  send_msg->receiver = dest;
  send_msg->ndesc = n_msg;
  send_msg->desc = desc;
  send_msg->sbuf_mem = sbuf_mem; 
  send_msg->buf = buf_tmp;
  send_msg->msg_size = msg_size;
}

void
qspi_set_send(int dest, void *buf, size_t size, qspi_msg_t send_msg)
{
  qspi_set_send_multi(dest, &buf, &size, 1, send_msg);
}

void
qspi_set_recv(int src, void *buf, size_t size, qspi_msg_t recv_msg)
{
  int32_t rc;

  Kernel_MemoryRegion_t *rbuf_mem = &recv_msg->rbuf_mem;
  rc = Kernel_CreateMemoryRegion(rbuf_mem, buf, size);
  CHECKRC(0);

  Kernel_MemoryRegion_t *counter_mem = &recv_msg->counter_mem;
  rc = Kernel_CreateMemoryRegion(counter_mem, (void *)&(recv_msg->counter), sizeof(recv_msg->counter));
  CHECKRC(0);

  recv_msg->flag = QSPI_RECV;
  recv_msg->counter = (volatile int64_t) size;
  recv_msg->counter_offset = MUSPI_GetAtomicAddress((uint64_t)counter_mem->BasePa+((uint64_t)&(recv_msg->counter)-(uint64_t)counter_mem->BaseVa), MUHWI_ATOMIC_OPCODE_STORE_ADD);
  recv_msg->recvbuf_offset = (uint64_t)rbuf_mem->BasePa+((uint64_t)buf-(uint64_t)rbuf_mem->BaseVa);
  recv_msg->sender = src;
  recv_msg->receiver = myrank;
  recv_msg->recv_size = size;
}

void
qspi_set_descriptor(int n_msg, int destcoords[], qspi_msg_t msg)
{
  MUSPI_Pt2PtDirectPutDescriptorInfo_t info;
  int local = islocal(destcoords);
  int rc;

  Kernel_MemoryRegion_t *sbuf_mem = msg->sbuf_mem + n_msg;
  size_t size = msg->msg_size[n_msg];
  void *buf =  msg->buf[n_msg];
  MUHWI_Descriptor_t *desc = msg->desc + n_msg;

  size_t t_size=0;
  for(int i=0; i<n_msg; i++) {
    t_size = t_size + msg->msg_size[i];
  }
  uint64_t recvbuf_offset = msg->recvbuf_offset + t_size;

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
  info.DirectPut.Rec_Payload_Offset = recvbuf_offset;
  info.DirectPut.Rec_Counter_Base_Address_Id = batID;
  info.DirectPut.Rec_Counter_Offset = msg->counter_offset;
  info.DirectPut.Pacing = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

  rc = MUSPI_CreatePt2PtDirectPutDescriptor(desc, &info);
  CHECKRC(0);
}

void qspi_prepare(qspi_msg_t msgs[], int num)
{
  struct qspi_msg_t from_recv_msg[num];
  MPI_Request req[num];

  for(int i=0; i<num; i++) {
    int flag = msgs[i]->flag;
    if(flag == QSPI_SEND) {
      int dest = msgs[i]->receiver;
      MPI_Irecv(&from_recv_msg[i],sizeof(struct qspi_msg_t),MPI_BYTE,dest,0,MPI_COMM_WORLD,&req[i]);
    } else if(flag == QSPI_RECV) {
      int dest = msgs[i]->sender;
      MPI_Isend(msgs[i],sizeof(struct qspi_msg_t),MPI_BYTE,dest,0,MPI_COMM_WORLD,&req[i]);
    } else {
      printf("ERROR! no flag= %i \n",flag);
    }
  }

  for(int i=0; i<num; i++) {
    int flag = msgs[i]->flag;
    if(flag == QSPI_SEND) {
      int dest = msgs[i]->receiver;
      int destcoords[NDIM];
      MPI_Status status;
      rank2coords(dest, destcoords);
      MPI_Wait(&req[i], &status);
      msgs[i]->counter_offset = from_recv_msg[i].counter_offset;
      msgs[i]->recvbuf_offset = from_recv_msg[i].recvbuf_offset;
      msgs[i]->receiver = from_recv_msg[i].receiver;
      if( msgs[i]->sender != from_recv_msg[i].sender ||
	  msgs[i]->receiver != dest ) {
        printf("ERROR! Parameters don't match \n");
      }
      int ndesc = msgs[i]->ndesc;
      for(int j=0; j<ndesc; j++) {
        qspi_set_descriptor(j, destcoords, msgs[i]);
      }
    } else if(flag == QSPI_RECV) {
      MPI_Status status;
      MPI_Wait(&req[i], &status);
    } else {
      printf("ERROR! no flag2= %i \n",flag);
    }
  }
}

void
qspi_start(qspi_msg_t msg)
{
  int flag = msg->flag;
  if(flag == QSPI_SEND) {
    uint32_t fifoID = m_count%FIFO_SUB;
    int subi = ((int)m_count/FIFO_SUB)%nsub;
    MUSPI_InjFifo_t *inj_fifo = MUSPI_IdToInjFifo(fifoID, &inj_sg[subi]);
    MUHWI_Descriptor_t *desc = msg->desc;
    int ndesc = msg->ndesc;
    msg->injed_desc = MUSPI_InjFifoInjectMultiple(inj_fifo, desc, ndesc);
    m_count++;
    msg->fifoID = fifoID;
    msg->subi = subi;
    if(msg->injed_desc<0) {
      for(int i=0; i<ndesc; i++) {
	do {
	  msg->injed_desc = MUSPI_InjFifoInjectMultiple(inj_fifo, desc+i, 1);
	  //msg->injed_desc = MUSPI_InjFifoInject(inj_fifo, desc+i);
	} while(msg->injed_desc<0);
      }
    }
  } else if(flag == QSPI_RECV) {
    /* do nothing */
  } else {
    printf("ERROR!1 flag= %i \n",flag);
  }
}

void
qspi_wait(qspi_msg_t msg)
{
  int flag = msg->flag;
  if(flag == QSPI_SEND) {
    uint32_t fifoID = msg->fifoID;
    int subi = msg->subi;
    MUSPI_InjFifo_t *inj_fifo = MUSPI_IdToInjFifo(fifoID, &inj_sg[subi]);
    int injed_desc = msg->injed_desc;
    int ndc;
    do {
      ndc = MUSPI_getHwDescCount(inj_fifo);
    } while(ndc<injed_desc);
  } else if(flag == QSPI_RECV) {
    size_t size = msg->recv_size;
    while(msg->counter>0);
    msg->counter = (volatile int64_t)size;
  } else {
    printf("ERROR!2 flag= %i \n", flag);
  }
}

qspi_msg_t
qspi_create_msg(void)
{
  qspi_msg_t msg;
  msg = (qspi_msg_t ) malloc(sizeof(struct qspi_msg_t));
  msg->flag = QSPI_UNDEF;
  return msg;
}

void
qspi_free_msg(qspi_msg_t msg)
{
  if(msg->flag==QSPI_SEND) {
    for(int i=0; i<msg->ndesc; i++) {
      Kernel_DestroyMemoryRegion(&msg->sbuf_mem[i]);
    }
    free(msg->sbuf_mem);
    free(msg->desc);
    free(msg->msg_size);
    free(msg->buf);
  } else if(msg->flag==QSPI_RECV) {
    Kernel_DestroyMemoryRegion(&msg->counter_mem);
    Kernel_DestroyMemoryRegion(&msg->rbuf_mem);
  } else if(msg->flag!=QSPI_UNDEF) {
    printf("ERROR!3 flag= %i \n",msg->flag);
  }
  free(msg);
}

void
qspi_set_collective_descriptor(MUHWI_Descriptor_t *desc, void *sbuf, int size,
			       int sizeoftype, Kernel_MemoryRegion_t *sbuf_mem,
			       void *rbuf, Kernel_MemoryRegion_t *rbuf_mem,
			       void *count, Kernel_MemoryRegion_t *count_mem)
{
  MUSPI_CollectiveDirectPutDescriptorInfo_t info;
  int rc;

  memset(&info, 0, sizeof(info));
  info.Base.Pre_Fetch_Only = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
  info.Base.Payload_Address = (uint64_t)sbuf_mem->BasePa+((uint64_t)sbuf-(uint64_t)sbuf_mem->BaseVa);
  info.Base.Message_Length = size;
  info.Base.Torus_FIFO_Map = MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CUSER;
  info.Base.Dest.Destination.Reserved2 = 0;
  info.Base.Dest.Destination.A_Destination = 0;
  info.Base.Dest.Destination.B_Destination = 0;
  info.Base.Dest.Destination.C_Destination = 0;
  info.Base.Dest.Destination.D_Destination = 0;
  info.Base.Dest.Destination.E_Destination = 0;

  info.Collective.Op_Code     = MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD;
  info.Collective.Word_Length = sizeoftype;
  info.Collective.Class_Route = 0;
  info.Collective.Misc        = MUHWI_PACKET_VIRTUAL_CHANNEL_USER_SUB_COMM | MUHWI_COLLECTIVE_TYPE_ALLREDUCE;
  info.Collective.Skip        = 0;

  uint64_t po = (uint64_t)rbuf_mem->BasePa+((uint64_t)rbuf-(uint64_t)rbuf_mem->BaseVa);
  uint64_t co = (uint64_t)count_mem->BasePa+((uint64_t)count-(uint64_t)count_mem->BaseVa);
  rc = MUSPI_SetBaseAddress(&bat_counter_coll, BAT_DEFAULT+1, co);
  CHECKRC(0);
  po -= co;
  co = 0;

  info.DirectPut.Rec_Payload_Base_Address_Id = batIDcoll;
  info.DirectPut.Rec_Payload_Offset = po;
  info.DirectPut.Rec_Counter_Base_Address_Id = batIDcoll;
  info.DirectPut.Rec_Counter_Offset = MUSPI_GetAtomicAddress(co, MUHWI_ATOMIC_OPCODE_STORE_ADD);
  info.DirectPut.Pacing = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;
  //printf("%i:  po: %lu  co: %lu\n", myrank, info.DirectPut.Rec_Payload_Offset, info.DirectPut.Rec_Counter_Offset);

  rc = MUSPI_CreateCollectiveDirectPutDescriptor(desc, &info);
  CHECKRC(0);
}

static struct collCR {
  volatile int64_t count;
  char recv[];
} *collCR;
static MUHWI_Descriptor_t desc;

static void
setup_collective(void *buf, Kernel_MemoryRegion_t *mr, int typesize, int size)
{
  static int recvsize = 0;
  static Kernel_MemoryRegion_t collMRcr;
  if(size>recvsize) {
    if(recvsize>0) {
      Kernel_DestroyMemoryRegion(&collMRcr);
      free(collCR);
    }
    int crsize = sizeof(struct collCR) + size;
    collCR = malloc(crsize);
    recvsize = size;
    int rc = Kernel_CreateMemoryRegion(&collMRcr, collCR, crsize);
    CHECKRC(0);
    qspi_set_collective_descriptor(&desc, buf, size, typesize, mr, collCR->recv,
				   &collMRcr, (void*)&collCR->count, &collMRcr);
  } else {
    uint64_t pa = (uint64_t)mr->BasePa+((uint64_t)buf-(uint64_t)mr->BaseVa);
    MUSPI_SetPayload(&desc, pa, size);
    //MUSPI_SetWordLength(&desc, typesize);
  }
  collCR->count = size;
}

void
qspi_sum_double(double *x)
{
  int n = 1;
  int typesize = sizeof(double);
  int size = n*typesize;
  int dnum = 0;
#if 1
  if(cmode>1) { // do local reduction
    if(tcoord==0) {
      double sum = *x;
      int nranks = shmptr->ranksonnode;
      int nleft = nranks - 1;
      //if(myrank==0) printf("nleft: %i\n", nleft);
      while(nleft) {
	for(int i=1; i<nranks; i++) {
	  if(shmptr->done[i]!=0) {
	    _bgq_msync();
	    sum += shmptr->sum[i];
	    shmptr->done[i] = 0;
	    nleft--;
	    //if(myrank==0) printf("nleft: %i\n", nleft);
	    if(nleft==0) break;
	  }
	}
      }
      //if(myrank==0) printf("xnleft: %i\n", nleft);
      *x = sum;
    } else {
      dnum = shmptr->done[0] + 1;
      shmptr->sum[tcoord] = *x;
      _bgq_msync();
      shmptr->done[tcoord] = 1;
    }
  }
#endif
  //if(myrank%23==1) sleep(1);
  if(tcoord==0) { // do global reduction
    int rc;
    Kernel_MemoryRegion_t collMRsend;
    rc = Kernel_CreateMemoryRegion(&collMRsend, x, size);
    CHECKRC(0);
    setup_collective(x, &collMRsend, sizeof(double), size);
    uint32_t fifoID = m_count%FIFO_SUB;
    int subi = ((int)m_count/FIFO_SUB)%nsub;
    MUSPI_InjFifo_t *inj_fifo = MUSPI_IdToInjFifo(fifoID, &inj_sg[subi]);
    int dnum = MUSPI_InjFifoInject(inj_fifo, &desc);
    while(collCR->count!=0);
    *x = ((double*)collCR->recv)[0];
    Kernel_DestroyMemoryRegion(&collMRsend);
    //printf("%i: %g\n", myrank, *x);
  }
#if 1
  if(cmode>1) { // do local broadcast
    if(tcoord==0) {
      shmptr->sum[0] = *x;
      _bgq_msync();
      shmptr->done[0]++;
    } else {
      //if(myrank==1) printf("dnum: %i\n", dnum);
      while(((volatile int *)(shmptr->done))[0]!=dnum);
      _bgq_msync();
      *x = ((volatile double *)(shmptr->sum))[0];
    }
  }
#endif
}
