/*----------------------------------------------------------------------------
 * Copyright (c) 2001      Southeastern Universities Research Association,
 *                         Thomas Jefferson National Accelerator Facility
 *
 * This software was developed under a United States Government license
 * described in the NOTICE file included as part of this distribution.
 *
 * Jefferson Lab HPC Group, 12000 Jefferson Ave., Newport News, VA 23606
 *----------------------------------------------------------------------------
 *
 * Description:
 *      Private header file for QMP over MPI implementation
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_P_COMMON.h,v $
 *   Revision 1.10  2008/03/06 17:28:42  osborn
 *   fixes to mapping options
 *
 *   Revision 1.9  2008/03/06 07:54:10  osborn
 *   added -qmp-alloc-map command line argument
 *
 *   Revision 1.8  2008/03/05 17:49:29  osborn
 *   added QMP_show_geom example and prepare for adding new command line options
 *
 *   Revision 1.7  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.6  2006/01/04 20:27:01  osborn
 *   Removed C99 named initializer.
 *
 *   Revision 1.5  2005/06/21 20:18:39  osborn
 *   Added -qmp-geom command line argument to force grid-like behavior.
 *
 *   Revision 1.4  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.3  2005/06/20 21:14:53  osborn
 *   Moved autoconf defines into qmp_config.h to make XLC happy.
 *
 *   Revision 1.2  2004/12/19 07:28:54  morten
 *   Added the include statement for QMP_profiling.h
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.1  2004/06/14 20:36:30  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.4  2004/04/08 09:00:20  bjoo
 *   Added experimental support for strided msgmem
 *
 *   Revision 1.3  2003/02/19 20:37:44  chen
 *   Make QMP_is_complete a polling function
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.3  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:40  chen
 *   Version 0.95 Release
 *
 *
 *
 */
#ifndef _QMP_P_COMMON_H
#define _QMP_P_COMMON_H

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#include "qmp_config.h"
#include "QMP_profiling.h"
#include "qmp.h"
#ifdef HAVE_MPI
#include "QMP_P_MPI.h"
#else
#include "QMP_P_SINGLE.h"
#endif
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

  /* Total number of nodes */
  int num_nodes; 

  /* My node id */
  int nodeid;

  /* number of dimensions if acting like mesh */
  int ndim;

  /* geometry if acting like mesh */
  int *geom;

  /* my coordinate if acting like mesh */
  int *coord;

  /* allocated geometry mapping */
  int *amap;

  /* logical geometry mapping */
  int lmaplen;
  int *lmap;

  /* verbose level                                           */
  int verbose;

  /* profile level                                           */
  int proflevel;

  /* last error code                                         */
  QMP_status_t err_code;

} *QMP_machine_t;
#define QMP_MACHINE_INIT {0.0, 0, QMP_SWITCH, NULL, 0, QMP_FALSE, 0, 0, 0,  \
	                  NULL, NULL, NULL, 0, NULL, 0, 0, QMP_SUCCESS}

extern QMP_machine_t QMP_global_m;



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

  /* Neighboring nodes of the form */
  /* neigh(isign,direction)   */ 
  /*  where  isign    = +1 : plus direction */
  /*                  =  0 : negative direction */
  /*  where  dir      = 0 .. dimension-1 */
  int *neigh[2];

  /* keep track if logical topology is declared yet */
  QMP_bool_t topology_declared;

} *QMP_logical_topology_t;

/* topology pointer */
extern QMP_logical_topology_t QMP_topo;


/**
 * Get and set error code macros.
 */
#define QMP_SET_STATUS_CODE(code) (QMP_global_m->err_code = code)

/**
 * Trace macro
 */
#ifdef _QMP_TRACE
#define QMP_TRACE(x) (printf("%s at line %d of file %s\n", x, __LINE__, __FILE__))
#else
#define QMP_TRACE(x)
#endif
 
 
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
#define ENTER_INIT START_DEBUG
#define LEAVE_INIT END_DEBUG

/**
 *  entry and exit to all other functions
 */
#define ENTER START_DEBUG START_TIMING
#define LEAVE END_DEBUG   END_TIMING

/**
 *  turn on function debugging
 */
#ifdef _QMP_DEBUG
#define START_DEBUG { QMP_info("Starting %s", __func__); }
#define END_DEBUG   { QMP_info("Finished %s", __func__); }
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

#endif /* _QMP_P_COMMON_H */
