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
#include "QMP_P_MPI.h"

/**
 * Allocate memory according to alignment.
 * calling glibc memalign code.
 */
#ifndef DMALLOC
extern void* memalign (unsigned int aignment, unsigned int size);
#endif

/**
 * allocate memory with right alignment.
 */
void *
QMP_memalign (QMP_u32_t size, QMP_u32_t alignment)
{
  return memalign (alignment, size);
}

/**
 * allocate memory with right alignment (16 bytes).
 */
void *
QMP_allocate_aligned_memory (QMP_u32_t nbytes)
{
  return QMP_memalign (QMP_MEM_ALIGNMENT, nbytes);
}

/**
 * Free aligned memory
 */
void QMP_free_aligned_memory (void* mem)
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
QMP_declare_msgmem(const void *buf, QMP_u32_t nbytes)
{
  Message_Memory_t mem = (Message_Memory_t)MP_allocMsgMem();

  if (mem)
  {
    mem->type = MM_user_buf;
    mem->mem = (void *)buf;
    mem->nbytes = nbytes;
  }
  else
    QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);

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
  
  free(mem);
}

QMP_msgmem_t
QMP_declare_strided_msgmem (void* base, 
			    QMP_u32_t blksize,
			    QMP_u32_t nblocks,
			    QMP_u32_t stride)
{
  QMP_error_exit ("QMP_declare_strided_msgmem: not yet implemented.");
  /* make compiler happy */
  return (QMP_msgmem_t)0;
}

/**
 * Declare a strided array memory.
 * Not yet implemented
 */
QMP_msgmem_t
QMP_declare_strided_array_msgmem (void** base, 
				  QMP_u32_t* blksize,
				  QMP_u32_t* nblocks,
				  QMP_u32_t* stride)
{
  QMP_error_exit ("QMP declare_declare_strided_array_msgmem: not yet implemented.");
  /* make compiler happy */
  return (QMP_msgmem_t)0;
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
	  current = current->next;
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
      QMP_fatal(1,"QMP_declare_receive_from: internal error - unknown message handle");
      break;
    }
  }
}

QMP_msghandle_t
QMP_declare_receive_from (QMP_msgmem_t mm, QMP_u32_t sourceNode,
			  QMP_s32_t priority)
{
  Message_Handle_t mh = (Message_Handle_t)MP_allocMsgHandler();

  if (mh)
  {
    mh->type = MH_recv;
    mh->num = 1;
    mh->mm = mm;
    mh->dest_node = QMP_machine->logical_nodeid;
    mh->srce_node = sourceNode;
    mh->tag  = TAG_CHANNEL;
    mh->next = NULL;
  }

  return (QMP_msghandle_t)mh;
}

/* Remote memory write - a send */
QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t mm, QMP_u32_t remoteHost,
		     QMP_s32_t priority)
{
  Message_Handle_t mh = (Message_Handle_t)MP_allocMsgHandler();

  if (mh)
  {
    mh->type = MH_send;
    mh->num = 1;
    mh->mm = mm;
    mh->dest_node = remoteHost;
    mh->srce_node = QMP_machine->logical_nodeid;
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
			     QMP_s32_t priority)
{
  int ii = (isign > 0) ? 1 : 0;

  return QMP_declare_receive_from (mm, QMP_machine->neigh[ii][dir], priority);
}

QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t mm, int dir, int isign,
			   QMP_s32_t priority)
{
  int ii = (isign > 0) ? 1 : 0;

  return QMP_declare_send_to(mm, QMP_machine->neigh[ii][dir], priority);
}

/* Declare multiple messages */
/* What this does is just link the first (non-null) messages
 * to subsequent messages */
QMP_msghandle_t
QMP_declare_multiple(QMP_msghandle_t msgh[], QMP_u32_t nhandle)
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
  mmh->num = nhandle;
  mmh->dest_node = MPI_UNDEFINED;
  mmh->srce_node = MPI_UNDEFINED;
  mmh->tag  = MPI_UNDEFINED;
  mmh0 = mmh;

  /* Link the input messages to the first message */
  /* Count and make sure there are nhandle valid messages */
  for(i=0; i < nhandle; ++i)
  {
    Message_Handle_t mh = (Message_Handle_t)msgh[i];

    if (mh)
    {
      num++;
      
      mmh->next = mh;
      mh->refcount++;
      mmh = mmh->next;
    }
    else
      QMP_error_exit ("QMP_declare_multiple: unexpectedly received a null message handle");
  }

  mmh = mmh0;
  if (num != nhandle)
    QMP_error_exit ("QMP_declare_multiple: invalid message handles");

  return (QMP_msghandle_t)mmh;
}
