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
#ifndef _QMP_P_MPI_H
#define _QMP_P_MPI_H

#include <mpi.h>
#include "qmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* 
 * Message Passing routines 
 */
#define TAG_CHANNEL  11

enum MM_type
{
  MM_lexico_buf,
  MM_strided_buf,
  MM_strided_array_buf,
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
/* here we use a C99 flexible array member so we can allocate the
   structure and memory together */
struct QMP_mem_struct_t {
  void *aligned_ptr;
  char mem[];
};

/* Message Memory structure */
typedef struct Message_Memory
{
  enum MM_type type;
  void *mem;
  int   nbytes;
  MPI_Datatype mpi_type;
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

typedef Message_Handle * Message_Handle_t;

extern MPI_Comm QMP_COMM_WORLD;

#endif /* _QMP_P_MPI_H */
