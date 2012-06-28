#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <mpi.h>
#include <mpix.h>

#include <kernel/process.h>
#include <kernel/location.h>
#include <kernel/MU.h>
#include <mu/Addressing_inlines.h>
#include <mu/GIBarrier.h>
#include "qspi.h"

uint32_t Kernel_ProcessCount(void);
uint32_t Kernel_MyTcoord(void);
uint32_t Kernel_GetRank(void);

#define NDIM 6

/* size of a single fifo. 
   BGQ_MU_DESCRIPTOR_SIZE_IN_BYTES*2 is minimum, which is good for a single descriptor.
   So, multiply the number of desired descriptors to inject to the fifo plus one.
   For example, BGQ_MU_DESCRIPTOR_SIZE_IN_BYTES*3, one can inject two descriptors to 
   the fifo.
   cf. BGQ_MU_DESCRIPTOR_SIZE_IN_BYTES = 64  */
#define FIFO_SIZE_LOCAL BGQ_MU_DESCRIPTOR_SIZE_IN_BYTES*8*1024

/* maximum fifo number */
#define FIFO_MAX 8

#define NFIFOS1 8
#define NFIFOS2 8
#define NFIFOS4 8
#define NFIFOS8 8
#define NFIFOS16 8
#define NFIFOS32 8
#define NFIFOS64 4

#define QSPI_SEND 1
#define QSPI_RECV 0
//#define QSPI_SEND_MULTI 2

#define BAT_DEFAULT 0


#define TRACE fprintf(stderr, "TRACE: %s:%i\n", __func__, __LINE__)
#define CHECKRC(x) if(rc!=x) { printf("rc = %i at %s: %i\n", rc, __func__, __LINE__); exit(rc); }

struct qspi_msg_t{
  uint32_t fifoID;
  uint32_t subgrpid;
  int flag; 
  uint64_t counter_offset;
  uint64_t recvbuf_offset;
  int sender;
  int receiver; 
  size_t msg_size; 
  MUHWI_Descriptor_t *desc;
  int ndesc;
  volatile int64_t counter;

  Kernel_MemoryRegion_t *sbuf_mem;
  Kernel_MemoryRegion_t counter_mem;
  Kernel_MemoryRegion_t rbuf_mem;
//  int msg_N;
  int injed_desc;
};


void qspi_set_descriptor(void *buf, size_t size, int destcoords[],qspi_msg_t msg);


/* routines to get ranks and coords of nodes */
//int nranks(void);
//int myrank(void);
//BG_CoordinateMapping_t *getrankmap(void);
void rank2coords(int rank, int coords[]);
void mycoords(int *coords);
int islocal(int *coords);


//void barrier(void);
