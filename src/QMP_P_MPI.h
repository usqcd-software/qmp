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
#ifndef _QMP_P_MPI_H
#define _QMP_P_MPI_H

#include <mpi.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/* number of dimensions                         */

#define ND  4


/*
 * Logical machine geometry
 */
typedef struct QMP_physical_geometry
{
  /* Dimension of physical geometry */
  unsigned dimension;

  /* Total number of nodes */
  unsigned num_nodes; 

  /* My logical (rotated) lexicographic node id */
  unsigned logical_nodeid;

  /* Lexicographic node id */
  unsigned physical_nodeid; 

  /* Index mapping logical lattice axes to physical axes */
  unsigned ordering[ND];

  /* My logical (rotated) coordinates */
  unsigned logical_coord[ND];

  /* Processor coordinate within machine grid */
  unsigned physical_coord[ND];
 
  /* Neighboring nodes of the form */
  /* neigh(isign,direction)   */ 
  /*  where  isign    = +1 : plus direction */
  /*                  =  0 : negative direction */
  /*  where  dir      = 0 .. ND-1 */
  unsigned neigh[2][ND];

  /* Logical size of the machine */
  unsigned logical_size[ND];

  /* Machine grid size */
  unsigned physical_size[ND];

} *QMP_physical_geometry_t;

/* topology pointer */
extern QMP_physical_geometry_t QMP_machine;


/**
 * Simple information holder for this machine
 */
typedef struct QMP_machine
{
  /* number of processors.                                   */
  QMP_u16_t  num_cpus;

  /* do we count those processors or user manage them.       */
  /* if collapse_smp == 1. this machine is viewed as a       */
  /* single cpu.                                             */
  QMP_bool_t collapse_smp;

  /* interconnection type for this machine.                  */
  QMP_ictype_t ic_type;

  /* host name of this machine.                              */
  char        host[256];

  /* whether this machine is initialized                     */
  QMP_bool_t inited;

  /* running in verbose mode                                 */
  QMP_bool_t verbose;

  /* last error code                                         */
  QMP_status_t err_code;

}QMP_machine_t;

/**
 * Memory alignment size (SSE)
 */
#define QMP_MEM_ALIGNMENT 16

/* 
 * Message Passing routines 
 */
#define MAXBUFFER  64

#define TAG_CHANNEL  11

enum MM_type
{
  MM_lexico_buf,
  MM_strided_buf,
  MM_user_buf
};

enum MH_type
{
  MH_empty,
  MH_freed,
  MH_multiple,
  MH_send,
  MH_recv
};

/* Message Memory structure */
typedef struct Message_Memory
{
  enum MM_type type;
  void *mem;
  int   nbytes;
} Message_Memory;

typedef Message_Memory * Message_Memory_t;


/* Message Handle structure */
typedef struct Message_Handle
{
  enum MH_type type;
  int          activeP;     /* Indicate whether the message is active */
  int          num;
  QMP_msgmem_t mm;
  int          dest_node;
  int          srce_node;
  int          tag;
  QMP_msghandle_t    next;
  MPI_Request  request;
  int          refcount;
  QMP_status_t err_code;
} Message_Handle;

/**
 * Trace macro
 */
#ifdef _QMP_TRACE
#define QMP_TRACE(x) (printf("%s at line %d of file %s\n", x, __LINE__, __FILE__))
#else
#define QMP_TRACE(x)
#endif

typedef Message_Handle * Message_Handle_t;

extern QMP_machine_t QMP_global_m;

/**
 * Get and set error code macros.
 */
#define QMP_SET_STATUS_CODE(code) (QMP_global_m.err_code = code)

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Print out information.
 */

extern QMP_bool_t QMP_declare_ordered_logical_topology (const QMP_u32_t *dims, 
							QMP_u32_t ndim, 
							QMP_u32_t ordering[]);

/**
 * Convert a physical node number to a logical node number. 
 * For switched configuration, they are the same.
 *
 * @return logical node number.
 */
extern QMP_u32_t          QMP_allocated_to_logical (QMP_u32_t node);

/**
 * Convert a logical node number to a physical node number.
 *
 * @return a physical node number.
 */
extern QMP_u32_t          QMP_logical_to_allocated (QMP_u32_t logic_rank);


/**
 * Allocate aligned memory of size 'size'
 */
extern void *QMP_memalign (QMP_u32_t size, 
			   QMP_u32_t alignment);


#ifdef __cplusplus
}
#endif

#endif

