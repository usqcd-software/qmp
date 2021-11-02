#ifndef _QMP_P_COMMON_H
#define _QMP_P_COMMON_H

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "qmp_config.h"
#include "QMP_profiling.h"
#include "qmp.h"

//#ifdef HAVE_BGL
//#include "QMP_P_BGL.h"
//#elif defined(HAVE_BGP)
//#include "QMP_P_BGP.h"
//#elif defined(HAVE_MPI)
#if defined(HAVE_MPI)
#include "QMP_P_MPI.h"
#endif
#if defined(HAVE_BGSPI)
#include "QMP_P_BGSPI.h"
#endif

typedef struct {
  /* allocated geometry */
  int geomlen, *geom;

  /* allocated geometry mapping */
  int amaplen, *amap;

  /* logical geometry mapping */
  int lmaplen, *lmap;

  /* job partition information */
  int num_jobs;
  int jobid;
  int njobdim;
  int *jobgeom;
} QMP_args_t;
#define QMP_ARGS_INIT 0,NULL,0,NULL,0,NULL,0,0,0,NULL
extern QMP_args_t *QMP_args;

/**
 * Simple information holder for this machine
 */
typedef struct QMP_machine
{
  double total_qmp_time;
  int timer_started;

  /* interconnection type for this machine.                  */
  QMP_ictype_t ic_type;

  /* host name of this machine.                              */
  char *host;
  int hostlen;

  /* whether this machine is initialized                     */
  QMP_bool_t inited;

  // machine nodeid and number of nodes
  int mnodes, mnodeid;

  /* verbose level                                           */
  int verbose;

  /* profile level                                           */
  int proflevel;

  /* last error code                                         */
  QMP_status_t err_code;

  QMP_thread_level_t thread_level;
} QMP_machine_t;
#define QMP_MACHINE_INIT 0.0, 0, QMP_SWITCH, NULL, 0, QMP_FALSE, 0, 0, 0,0, QMP_SUCCESS, QMP_THREAD_SINGLE
extern QMP_machine_t *QMP_machine;

/*
 * Logical machine topology
 */
typedef struct QMP_logical_topology
{
  /* Dimension of logical topology */
  int dimension;

  /* Sizes of the logical topology */
  int *logical_size;

  /* My logical coordinates */
  int *logical_coord;

  // permutation map of coordinate axes
  int mapdim, *map;

  /* Neighboring nodes of the form */
  /* neigh(isign,direction)   */ 
  /*  where  isign    = +1 : plus direction */
  /*                  =  0 : negative direction */
  /*  where  dir      = 0 .. dimension-1 */
  int *neigh[2];
} QMP_logical_topology_t;

struct QMP_comm_struct {
  /* Total number of nodes */
  int num_nodes; 

  /* My node id */
  int nodeid;

  int ncolors;
  int color;
  int key;

  QMP_logical_topology_t *topo;

#ifdef COMM_TYPES
  COMM_TYPES
#endif
};
#ifndef COMM_TYPES_INIT
#define COMM_TYPES_INIT
#endif
#define QMP_COMM_INIT 0,0,0,0,0,NULL COMM_TYPES_INIT
#define QMP_topo_declared(comm) ((comm)->topo==NULL?QMP_FALSE:QMP_TRUE)

// predefined communicators
extern QMP_comm_t QMP_allocated_comm;
extern QMP_comm_t QMP_job_comm;
extern QMP_comm_t QMP_default_comm;

/* Message Memory structure */
struct QMP_mem_struct {
  void *aligned_ptr;
  void *allocated_ptr;
};


enum MM_type
{
  MM_user_buf,
  MM_strided_buf,
  MM_strided_array_buf,
  MM_indexed_buf
};

enum MH_type
{
  MH_empty,
  MH_freed,
  MH_multiple,
  MH_send,
  MH_recv
};

struct mm_st { // strided
  size_t blksize;
  int nblocks;
  ptrdiff_t stride;
};

struct mm_sa { // strided array
  ptrdiff_t *disp;
  size_t *blksize;
  int *nblocks;
  ptrdiff_t *stride;
  int narray;
};

struct mm_in { // indexed
  int *blocklen;
  int *index;
  int elemsize;
  int count;
};

/* Message Memory structure */
struct QMP_msgmem_struct
{
  enum MM_type type;
  void *mem;
  int   nbytes;
  union {
    struct mm_st st;
    struct mm_sa sa;
    struct mm_in in;
  } mm;
#ifdef MM_TYPES
  MM_TYPES
#endif
};

struct QMP_msghandle_struct {
  enum MH_type type;
  int          activeP;     /* Indicate whether the message is active */
  int          clear_to_send;
  int          num;
  QMP_msgmem_t mm;
  int          dest_node;
  int          srce_node;
  int          uses;
  int axis;
  int dir;
  int priority;
  int paired;
  char *base;
  QMP_comm_t comm;
  QMP_status_t err_code;
  QMP_msghandle_t next;
#ifdef MH_TYPES
  MH_TYPES
#endif
};

#define QMP_assert(x) if(!(x)) QMP_FATAL("assert failed "#x)
#define QMP_alloc(v,t,n) v = (t *) malloc((n)*sizeof(t))
#define QMP_free(x) free(x)



/**
 * Get and set error code macros.
 */
#define QMP_SET_STATUS_CODE(code) (QMP_machine->err_code = code)

/**
 * Trace macro
 */
#ifdef _QMP_TRACE
#define QMP_TRACE(x) (printf("%s at line %d of file %s\n", x, __LINE__, __FILE__))
#else
#define QMP_TRACE(x)
#endif

#define TRACE { printf("TRACE: file %s func %s line %i\n", __FILE__, __func__, __LINE__); fflush(stdout); QMP_barrier(); }

/**
 * Fatal error macro
 */
#define QMP_FATAL(string) \
{ \
  QMP_error("In file %s, function %s, line %i:", __FILE__, __func__, __LINE__); \
  QMP_error(string); \
  QMP_abort(1); \
}


/**
 *  entry and exit to init and final functions
 */
#define ENTER_INIT
#define LEAVE_INIT

/**
 *  entry and exit to all other functions
 */
#define ENTER START_DEBUG START_TIMING CHECKS
#define LEAVE END_DEBUG   END_TIMING

/**
 *  turn on function debugging
 */
#ifdef _QMP_DEBUG
extern int QMP_stack_level;
#define START_DEBUG { QMP_info("%*s-> %s", QMP_stack_level, "", __func__); QMP_stack_level+=2; }
#define END_DEBUG   { QMP_stack_level-=2; QMP_info("%*s<- %s", QMP_stack_level, "", __func__); }
#else
#define START_DEBUG
#define END_DEBUG
#endif

/**
 *  turn on function timing
 */
#ifdef QMP_BUILD_TIMING
#define START_TIMING					\
  { if(QMP_global_m->inited) {				\
      if(QMP_global_m->timer_started==0)		\
	QMP_global_m->total_qmp_time -= QMP_time();	\
      QMP_global_m->timer_started++; } }
#define END_TIMING					\
  { if(QMP_global_m->inited) {				\
      QMP_global_m->timer_started--;			\
      if(QMP_global_m->timer_started==0)		\
	QMP_global_m->total_qmp_time += QMP_time(); } }
#else
#define START_TIMING
#define END_TIMING
#endif

#if 0
#define CHECKS
#else
#define CHECKS { 				\
    QMP_assert(QMP_machine->inited==QMP_TRUE)	\
  }
#endif

#endif /* _QMP_P_COMMON_H */
