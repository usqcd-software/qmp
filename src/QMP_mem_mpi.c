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
 *      QMP memory routines for MPI implementation
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.8  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.7  2004/04/08 09:00:20  bjoo
 *   Added experimental support for strided msgmem
 *
 *   Revision 1.6  2003/07/22 02:17:00  edwards
 *   Yet another change/hack of QMP_memalign. Use the obsolete valloc.
 *
 *   Revision 1.5  2003/07/21 21:07:17  edwards
 *   Changed around QMP_memalign.
 *
 *   Revision 1.4  2003/07/21 20:57:12  edwards
 *   Changed QMP_allocated_align_memory to use QMP_memalign instead of
 *   memalign directly.
 *
 *   Revision 1.3  2003/05/23 14:11:01  bjoo
 *   Fixed memory leak in QMP_free_msghandle
 *
 *   Revision 1.2  2003/02/13 16:22:23  chen
 *   qmp version 1.2
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.1  2002/04/22 20:28:42  chen
 *   Version 0.95 Release
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_COMMON.h"
#include "QMP_P_MPI.h"

/**
 * allocate memory with default alignment and flags.
 */
QMP_mem_t *
QMP_allocate_memory (size_t nbytes)
{
  QMP_mem_t *mem;
  mem = malloc(sizeof(QMP_mem_t)+nbytes);
  if(mem) mem->aligned_ptr = mem->mem;
  return mem;
}

/**
 * allocate memory with specified alignment and flags.
 */
QMP_mem_t *
QMP_allocate_aligned_memory (size_t nbytes, size_t alignment, int flags)
{
  QMP_mem_t *mem;
  if(alignment<0) alignment = 0;  /* shouldn't happen but doesn't hurt to check */
  mem = malloc(sizeof(QMP_mem_t)+nbytes+alignment);
  if(mem) {
    if(alignment) {
      mem->aligned_ptr = (void *)
	( ( ( (size_t)(mem->mem+alignment-1) )/alignment )*alignment );
    } else {
      mem->aligned_ptr = mem->mem;
    }
  }
  return mem;
}

/**
 * Get pointer to memory from a memory structure.
 */
void *
QMP_get_memory_pointer (QMP_mem_t* mem)
{
  return mem->aligned_ptr;
}

/**
 * Free aligned memory
 */
void
QMP_free_memory (QMP_mem_t* mem)
{
  free (mem);
}

/* Alloc message handler */
static QMP_msgmem_t MP_allocMsgMem(void)
{
  return (QMP_msgmem_t)malloc(sizeof(Message_Memory));
}

/* Basic buffer constructor */
QMP_msgmem_t
QMP_declare_msgmem(const void *buf, size_t nbytes)
{
  Message_Memory_t mem = (Message_Memory_t)MP_allocMsgMem();

  if (mem)
  {
    mem->type = MM_user_buf;
    mem->mem = (void *)buf;
    mem->nbytes = nbytes;
    mem->mpi_type = MPI_BYTE;
  }
  else
    QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);

  return (QMP_msgmem_t)mem;
}

QMP_msgmem_t
QMP_declare_strided_msgmem (void* base, 
			    size_t blksize,
			    int nblocks,
			    ptrdiff_t stride)
{

  Message_Memory_t mem = (Message_Memory_t)MP_allocMsgMem();

  if (mem) {
    int mpi_stride = stride+blksize;
    int err_code;


    mem->mem = (void *)base;


    if( stride == 0 ) { 
      /* Not really strided */
      mem->type = MM_user_buf;
      mem->nbytes = blksize*nblocks;
      mem->mpi_type = MPI_BYTE;
      return(QMP_msgmem_t)mem;
    }
    else {
      mem->type = MM_strided_buf;
      /* Really strided */
      err_code = MPI_Type_vector(nblocks, blksize, mpi_stride, MPI_BYTE,
				 &(mem->mpi_type));

      /* if MPI_Type_vector fails */
      if( err_code != MPI_SUCCESS) {
	QMP_free_msgmem(mem);
	QMP_SET_STATUS_CODE (QMP_ERROR);
	return(QMP_msgmem_t)0;
      }

      err_code = MPI_Type_commit(&(mem->mpi_type));
      if( err_code != MPI_SUCCESS) { 
	QMP_free_msgmem(mem);
	QMP_SET_STATUS_CODE(QMP_ERROR);
        return(QMP_msgmem_t)0;
      }
      /* It succeeded */
      return (QMP_msgmem_t)mem;
    }
  }
  else {
    QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);
    return(QMP_msgmem_t)0;
  }
}

/**
 * Declare a strided array memory.
 */
QMP_msgmem_t
QMP_declare_strided_array_msgmem (void* base[], 
				  size_t blksize[],
				  int nblocks[],
				  ptrdiff_t stride[],
				  int narray)
{
  Message_Memory_t mem = (Message_Memory_t)MP_allocMsgMem();

  if (mem) {
    int err_code, i;
    int tlen[2];
    MPI_Aint tdisp[2], disp[narray];
    MPI_Datatype tdt[2], dt[narray];

#define check_error if(err_code!=MPI_SUCCESS) { \
      QMP_free_msgmem(mem); \
      QMP_SET_STATUS_CODE (QMP_ERROR); \
      return(QMP_msgmem_t)0; \
    }

    mem->type = MM_strided_array_buf;

    tlen[1] = 1;
    tdisp[0] = 0;
    tdt[0] = MPI_BYTE;
    tdt[1] = MPI_UB;

    for(i=0; i<narray; i++) {
      err_code = MPI_Address(base[i], &disp[i]);
      check_error;

      tlen[0] = blksize[i];
      tdisp[1] = stride[i];
      err_code = MPI_Type_struct(2, tlen, tdisp, tdt, &dt[i]);
      check_error;
    }

    err_code = MPI_Type_struct(narray, nblocks, disp, dt, &(mem->mpi_type));
    check_error;
    err_code = MPI_Type_commit(&(mem->mpi_type));
    check_error;

    for(i=0; i<narray; i++) {
      err_code = MPI_Type_free(&dt[i]);
      check_error;
    }
#undef check_error
  }
  else {
    QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);
  }
  return (QMP_msgmem_t)mem;
}

/* Basic buffer destructor */
void QMP_free_msgmem(QMP_msgmem_t mm)
{
  struct Message_Memory* mem = (struct Message_Memory *)mm;

  if (!mem)
    return;
  if (mem->type == MM_lexico_buf)
    free (mem->mem);

  if ( (mem->type == MM_strided_buf) ||
       (mem->type == MM_strided_array_buf) ) {
    MPI_Type_free(&(mem->mpi_type));
  }

  free(mem);
}


/* Alloc message handler */
static QMP_msghandle_t 
MP_allocMsgHandler(void)
{
  Message_Handle_t mh = (Message_Handle_t)malloc(sizeof(Message_Handle));

  if (mh)
  {
    mh->type = MH_empty;
    mh->activeP = 0;
    mh->num = 0;
    mh->mm = NULL;
    mh->dest_node = MPI_UNDEFINED;
    mh->srce_node = MPI_UNDEFINED;
    mh->tag  = MPI_UNDEFINED;
    mh->request = MPI_REQUEST_NULL;
    mh->refcount = 1;
    mh->next = NULL;
    mh->err_code = QMP_SUCCESS;
  }

  return (QMP_msghandle_t)mh;
}

/* Basic handler destructor */
void QMP_free_msghandle (QMP_msghandle_t msgh)
{
  Message_Handle_t first = (Message_Handle_t)msgh;
  Message_Handle_t current;
  Message_Handle_t previous;

  if (first) {
    switch (first->type) {
    case MH_multiple:
      previous=first; 
      current =first->next;

      while (current)
      {

	/* Only place a freed MH can actually be deleted */
	current->refcount--;

	if (current->refcount == 0) {
	  /* Zero refcount -- free it */
	  previous->next = current->next;
	  free(current);

	  /* Previous stays the same */
	  current = previous->next;
	}
	else {
	  /* Increment previous */
	  previous = current;

	  /* Increment current */
	  current = current->next;
	}
      }

      /* Now decrement myself. Note, I could be part of another ganged message */
      first = (Message_Handle_t)msgh;
      first->refcount--;
      if (first->refcount == 0)
	free(first);
      else
	first->type = MH_empty;
      break;

    case MH_empty:
    case MH_send:
    case MH_recv:
      /* If non-null refcount, then mark for free. Must be ganged */
      first->refcount--;
      if (first->refcount == 0)
	free(first);
      else
	first->type = MH_empty;
      break;

    default:
      QMP_FATAL("internal error: unknown message handle");
      break;
    }
  }
}

QMP_msghandle_t
QMP_declare_receive_from (QMP_msgmem_t mm, int sourceNode, int priority)
{
  Message_Handle_t mh = (Message_Handle_t)MP_allocMsgHandler();

  if (mh)
  {
    mh->type = MH_recv;
    mh->num = 1;
    mh->mm = mm;
    mh->dest_node = QMP_global_m->nodeid;
    mh->srce_node = sourceNode;
    mh->tag  = TAG_CHANNEL;
    mh->next = NULL;
  }

  return (QMP_msghandle_t)mh;
}

/* Remote memory write - a send */
QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t mm, int remoteHost, int priority)
{
  Message_Handle_t mh = (Message_Handle_t)MP_allocMsgHandler();

  if (mh)
  {
    mh->type = MH_send;
    mh->num = 1;
    mh->mm = mm;
    mh->dest_node = remoteHost;
    mh->srce_node = QMP_global_m->nodeid;
    mh->tag  = TAG_CHANNEL;
    mh->next = NULL;
  }

  return (QMP_msghandle_t)mh;
}

/* (Supposedly) fast nearest neighbor communication */
/* NOTE: I refuse to use enums here or the goofy starting at 1 stuff */
/* 
 *  dir    - [0,1,2,3,...,Nd-1]
 *  isign  - +1: send in +dir direction
 *         - -1: send in -dir direction
 */
QMP_msghandle_t
QMP_declare_receive_relative(QMP_msgmem_t mm, int dir, int isign,
			     int priority)
{
  int ii = (isign > 0) ? 1 : 0;

  return QMP_declare_receive_from (mm, QMP_topo->neigh[ii][dir], priority);
}

QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t mm, int dir, int isign,
			   int priority)
{
  int ii = (isign > 0) ? 1 : 0;

  return QMP_declare_send_to(mm, QMP_topo->neigh[ii][dir], priority);
}

/* Declare multiple messages */
/* What this does is just link the first (non-null) messages
 * to subsequent messages */
QMP_msghandle_t
QMP_declare_multiple(QMP_msghandle_t msgh[], int nhandle)
{
  Message_Handle_t mmh = (Message_Handle_t)MP_allocMsgHandler();
  Message_Handle_t mmh0;
  int num=0;
  int i;

  if (! mmh)
    return mmh;

  /* The first handle is a indicator it is a placeholder */
  mmh->type = MH_multiple;
  mmh->mm = NULL;
  /* mmh->num = nhandle;  fill this in later */
  mmh->dest_node = MPI_UNDEFINED;
  mmh->srce_node = MPI_UNDEFINED;
  mmh->tag  = MPI_UNDEFINED;
  mmh0 = mmh;

  /* Link the input messages to the first message */
  /* Count and make sure there are nhandle valid messages */
  for(i=0; i < nhandle; ++i) {
    Message_Handle_t mh = (Message_Handle_t)msgh[i];

    if (mh) {
      if(mh->type==MH_multiple) {
	mmh->next = mh->next;
	free(mh);
	while(mmh->next) {
	  mmh = mmh->next;
	  num++;
	}
      } else {
	mmh->next = mh;
	mmh = mmh->next;
	num++;
      }
    } else
      QMP_FATAL("unexpectedly received a null message handle");
  }
  mmh0->num = num;

  return (QMP_msghandle_t)mmh0;
}
