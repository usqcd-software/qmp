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
 *   $Log: not supported by cvs2svn $
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

#include "qmp.h"

/**
 * Simple information holder for this machine
 */
typedef struct QMP_machine
{
  /* interconnection type for this machine.                  */
  QMP_ictype_t ic_type;

  /* host name of this machine.                              */
  char        host[256];

  /* whether this machine is initialized                     */
  QMP_bool_t inited;

  /* Total number of nodes */
  int num_nodes; 

  /* My node id */
  int nodeid;

  /* verbose level                                           */
  int verbose;

  /* profile level                                           */
  int proflevel;

  /* last error code                                         */
  QMP_status_t err_code;

} *QMP_machine_t;

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
 
#endif /* _QMP_P_COMMON_H */
