/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP intialize code for single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_COMMON.h"
#include "QMP_P_SINGLE.h"


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
  if(alignment<0) alignment = 0;  // shouldn't happen but doesn't hurt to check
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


/* Basic buffer constructor */
QMP_msgmem_t
QMP_declare_msgmem(const void *buf, size_t nbytes)
{
  return QMP_MSGMEM_ALLOCATED;
}

/**
 * Declare a strided memory.
 */
QMP_msgmem_t
QMP_declare_strided_msgmem (void* base, 
			    size_t blksize,
			    int nblocks,
			    ptrdiff_t stride)
{
  return QMP_MSGMEM_ALLOCATED;
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
  return QMP_MSGMEM_ALLOCATED;
}

/* Basic buffer destructor */
void
QMP_free_msgmem(QMP_msgmem_t mm)
{
  if(mm!=QMP_MSGMEM_ALLOCATED) {
    QMP_FATAL("passed QMP_msgmem_t not allocated");
  }
}

/* Message handle routines */
QMP_msghandle_t
QMP_declare_receive_from (QMP_msgmem_t mm, int sourceNode, int priority)
{
  return QMP_MSGHANDLE_ALLOCATED;
}

QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t mm, int remoteHost, int priority)
{
  return QMP_MSGHANDLE_ALLOCATED;
}

QMP_msghandle_t
QMP_declare_receive_relative(QMP_msgmem_t mm, int dir, int isign,
			     int priority)
{
  return QMP_MSGHANDLE_ALLOCATED;
}

QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t mm, int dir, int isign,
			   int priority)
{
  return QMP_MSGHANDLE_ALLOCATED;
}

QMP_msghandle_t
QMP_declare_multiple(QMP_msghandle_t msgh[], int nhandle)
{
  return QMP_MSGHANDLE_ALLOCATED;
}

void
QMP_free_msghandle (QMP_msghandle_t msgh)
{
  if(msgh!=QMP_MSGHANDLE_ALLOCATED) {
    QMP_FATAL("passed QMP_msghandle_t not allocated");
  }
}

