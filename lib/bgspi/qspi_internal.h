#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <mpi.h>
#include <mpix.h>

#include <kernel/location.h>
#include <kernel/process.h>
#include <kernel/MU.h>
#include <mu/Addressing_inlines.h>
#include <mu/Descriptor_inlines.h>
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

/* number of fifos for a sub-group (hardware max number is 8) depend on c-mode*/
#define TFIFO_SUB1 1
#define TFIFO_SUB2 1
#define TFIFO_SUB4 1
#define TFIFO_SUB8 2
#define TFIFO_SUB16 4
#define TFIFO_SUB32 4
#define TFIFO_SUB64 4

/* maximum number of subgroups */
#define MAX_SUBGROUP 64

/* maximum number of total fifos for all subgroups */
#define FIFO_MAX  4*MAX_SUBGROUP

/* flag for the send and receive */
#define QSPI_UNDEF -1
#define QSPI_SEND 0
#define QSPI_RECV 1

/* default Base address table ID */
#define BAT_DEFAULT 0

#define TRACE fprintf(stderr, "TRACE: %s:%i\n", __func__, __LINE__)
#define CHECKRC(x) if(rc!=x) { printf("rc = %i at %s: %i\n", rc, __func__, __LINE__); exit(rc); }

/* communication message structure. 
  uint32_t fifoID;                        fifo ID
  uint32_t subgrpid;                      subgroup ID
  int flag;                               QSPI_SEND or QSPI_RECV
  uint64_t counter_offset;                physical address offset for counter
  uint64_t recvbuf_offset;                physical address offset for receive buffer
  int sender;                             sender rank ID
  int receiver;                           receiver rank ID
  size_t msg_size;                        message size in byte
  MUHWI_Descriptor_t *desc;               pointer to the descriptors
  int ndesc;                              number of descriptors
  volatile int64_t counter;               counter

  Kernel_MemoryRegion_t *sbuf_mem;        send buffer memory region
  Kernel_MemoryRegion_t *counter_mem;     counter memory region
  Kernel_MemoryRegion_t *rbuf_mem;        receive buffer memory region
  int injed_desc;                         number of injected descriptors
*/
struct qspi_msg_t {
  uint32_t fifoID;
  //  uint32_t subgrpid;
  int subi;
  int flag; 
  uint64_t counter_offset;
  uint64_t recvbuf_offset;
  int sender;
  int receiver; 
  size_t *msg_size; 
  size_t recv_size;
  MUHWI_Descriptor_t *desc;
  int ndesc;
  volatile int64_t counter;
  Kernel_MemoryRegion_t *sbuf_mem;
  Kernel_MemoryRegion_t counter_mem;
  Kernel_MemoryRegion_t rbuf_mem;
  int injed_desc;
  void **buf;
}; 
