/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP intialize code for single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: QMP_mem_single.c,v $
 *   Revision 1.6  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.5  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.4  2004/12/16 02:44:12  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
 *
 *   Revision 1.3  2004/10/31 23:21:36  osborn
 *   Restored proper behavior of msghandle operations in single node version.
 *   Added CFLAGS to qmp-config script.
 *   Changed QMP_status_code_t to QMP_status_t in qmp.h.
 *
 *   Revision 1.2  2004/10/18 18:17:22  edwards
 *   Added support for calling msghandle functions.
 *
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

#include "QMP_P_SINGLE.h"

/**
 * allocate memory with default alignment and flags.
 */
QMP_mem_t *
QMP_allocate_memory (size_t nbytes)
{
  QMP_mem_t *mem;
  ENTER;

  mem =
    QMP_allocate_aligned_memory(nbytes, QMP_ALIGN_DEFAULT, QMP_MEM_DEFAULT);

  LEAVE;
  return mem;
}

/**
 * allocate memory with specified alignment and flags.
 */
QMP_mem_t *
QMP_allocate_aligned_memory (size_t nbytes, size_t alignment, int flags)
{
  QMP_mem_t *mem;
  ENTER;

  mem = (QMP_mem_t *) malloc(sizeof(QMP_mem_t));
  if(mem) {
    mem->allocated_ptr = malloc(nbytes+alignment);
    if(mem->allocated_ptr) {
      if(alignment) {
	mem->aligned_ptr = (void *)
	  (((((size_t)(mem->allocated_ptr))+alignment-1)/alignment)*alignment);
      } else {
	mem->aligned_ptr = mem->allocated_ptr;
      }
    } else {
      free(mem);
      mem = NULL;
    }
  }

  LEAVE;
  return mem;
}

/**
 * Get pointer to memory from a memory structure.
 */
void *
QMP_get_memory_pointer (QMP_mem_t* mem)
{
  ENTER;
  LEAVE;
  return mem->aligned_ptr;
}

/**
 * Free aligned memory
 */
void
QMP_free_memory (QMP_mem_t* mem)
{
  ENTER;
  if(mem) {
    free(mem->allocated_ptr);
    free(mem);
  }
  LEAVE;
}

/* Basic buffer constructor */
QMP_msgmem_t
QMP_declare_msgmem(const void *buf, size_t nbytes)
{
  ENTER;
  LEAVE;
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
  ENTER;
  LEAVE;
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
  ENTER;
  LEAVE;
  return QMP_MSGMEM_ALLOCATED;
}

/* Basic buffer destructor */
void
QMP_free_msgmem(QMP_msgmem_t mm)
{
  ENTER;
  if(mm!=QMP_MSGMEM_ALLOCATED) {
    QMP_FATAL("passed QMP_msgmem_t not allocated");
  }
  LEAVE;
}

/* Message handle routines */
QMP_msghandle_t
QMP_declare_receive_from (QMP_msgmem_t mm, int sourceNode, int priority)
{
  ENTER;
  QMP_error("QMP_declare_receive_from: invalid source node %i\n", sourceNode);
  QMP_abort(1);
  LEAVE;
  return (QMP_msghandle_t)0;
}

QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t mm, int remoteHost, int priority)
{
  ENTER;
  QMP_error("QMP_declare_send_to: invalid destination node %i\n", remoteHost);
  QMP_abort(1);
  LEAVE;
  return (QMP_msghandle_t)0;
}

QMP_msghandle_t
QMP_declare_receive_relative(QMP_msgmem_t mm, int dir, int isign,
			     int priority)
{
  ENTER;
  QMP_error("QMP_declare_receive_relative: invalid direction %i\n", dir);
  QMP_abort(1);
  LEAVE;
  return (QMP_msghandle_t)0;
}

QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t mm, int dir, int isign,
			   int priority)
{
  ENTER;
  QMP_error("QMP_declare_send_relative: invalid direction %i\n", dir);
  QMP_abort(1);
  LEAVE;
  return (QMP_msghandle_t)0;
}

QMP_msghandle_t
QMP_declare_multiple(QMP_msghandle_t msgh[], int nhandle)
{
  ENTER;
  QMP_error("QMP_declare_multiple: invalid arguments\n");
  QMP_abort(1);
  LEAVE;
  return (QMP_msghandle_t)0;
}

void
QMP_free_msghandle (QMP_msghandle_t msgh)
{
  ENTER;
  QMP_error("QMP_free_msghandle: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  LEAVE;
}
