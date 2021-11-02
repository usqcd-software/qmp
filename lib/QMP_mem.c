#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "QMP_P_COMMON.h"


/**
 * allocate memory with default alignment and flags.
 */
QMP_mem_t *
QMP_allocate_memory (size_t nbytes)
{
  QMP_mem_t *mem;
  ENTER;

  mem = QMP_allocate_aligned_memory(nbytes, QMP_ALIGN_DEFAULT, QMP_MEM_DEFAULT);

  LEAVE;
  return mem;
}


/**
 * allocate memory with specified alignment and flags.
 */
QMP_mem_t *
QMP_allocate_aligned_memory (size_t nbytes, size_t alignment, int flags)
{
  _QMP_UNUSED_ARGUMENT(flags);

  QMP_mem_t *mem;
  ENTER;

  QMP_alloc(mem, QMP_mem_t, 1);
  if(mem) {
    QMP_alloc(mem->allocated_ptr, char, nbytes+alignment);
    if(mem->allocated_ptr) {
      if(alignment) {
	mem->aligned_ptr = (void *)
	  (((((size_t)(mem->allocated_ptr))+alignment-1)/alignment)*alignment);
      } else {
	mem->aligned_ptr = mem->allocated_ptr;
      }
    } else {
      QMP_free(mem);
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
    QMP_free(mem->allocated_ptr);
    QMP_free(mem);
  }
  LEAVE;
}


/* Basic buffer constructor */
QMP_msgmem_t
QMP_declare_msgmem(const void *buf, size_t nbytes)
{
  struct QMP_msgmem_struct *mem;
  ENTER;

  QMP_alloc(mem, struct QMP_msgmem_struct, 1);

  if (mem) {
    mem->type = MM_user_buf;
    mem->mem = (char *)buf;
    mem->nbytes = nbytes;
#ifdef QMP_DECLARE_MSGMEM
    QMP_DECLARE_MSGMEM(mem);
#endif
  } else {
    QMP_SET_STATUS_CODE(QMP_NOMEM_ERR);
  }

  LEAVE;
  return mem;
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
  struct QMP_msgmem_struct *mem;
  ENTER;

  if( stride == (ptrdiff_t)blksize || nblocks == 1 ) { /* Not really strided */
    mem = QMP_declare_msgmem(base, blksize*nblocks);
  } else { /* Really strided */
    QMP_alloc(mem, struct QMP_msgmem_struct, 1);

    if (mem) {
      mem->type = MM_strided_buf;
      mem->mem = (char *)base;
      mem->nbytes = blksize*nblocks;
      mem->mm.st.blksize = blksize;
      mem->mm.st.nblocks = nblocks;
      mem->mm.st.stride = stride;
#ifdef QMP_DECLARE_MSGMEM
      QMP_DECLARE_MSGMEM(mem);
#endif
    } else {
      QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);
    }
  }

  LEAVE;
  return mem;
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
  struct QMP_msgmem_struct *mem;
  ENTER;

  if(narray==1) {
    mem = QMP_declare_strided_msgmem(base[0], blksize[0], nblocks[0], stride[0]);
  } else {
    QMP_alloc(mem, struct QMP_msgmem_struct, 1);

    if (mem) {
      mem->type = MM_strided_array_buf;
      mem->mem = (char *)base[0];
      mem->mm.sa.narray = narray;
      QMP_alloc(mem->mm.sa.disp, ptrdiff_t, narray);
      QMP_alloc(mem->mm.sa.blksize, size_t, narray);
      QMP_alloc(mem->mm.sa.nblocks, int, narray);
      QMP_alloc(mem->mm.sa.stride, ptrdiff_t, narray);
      int i, nb=0;
      for(i=0; i<narray; i++) {
	nb += blksize[i]*nblocks[i];
	mem->mm.sa.disp[i] = (ptrdiff_t)(((char *)base[i]) - ((char *)base[0]));
	mem->mm.sa.blksize[i] = blksize[i];
	mem->mm.sa.nblocks[i] = nblocks[i];
	mem->mm.sa.stride[i] = stride[i];
      }
      mem->nbytes = nb;
#ifdef QMP_DECLARE_MSGMEM
      QMP_DECLARE_MSGMEM(mem);
#endif
    } else {
      QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);
    }
  }

  LEAVE;
  return mem;
}

/**
 * Declare a indexed memory.
 */
QMP_msgmem_t
QMP_declare_indexed_msgmem (void* base, 
			    int blocklen[],
			    int index[],
			    int elemsize,
			    int count)
{
  struct QMP_msgmem_struct *mem;
  ENTER;

  QMP_alloc(mem, struct QMP_msgmem_struct, 1);

  if (mem) {
    mem->type = MM_indexed_buf;
    mem->mem = (char *)base;
    mem->mm.in.elemsize = elemsize;
    mem->mm.in.count = count;
    QMP_alloc(mem->mm.in.blocklen, int, count);
    QMP_alloc(mem->mm.in.index, int, count);
    int i, nb=0;
    for(i=0; i<count; i++) {
      mem->mm.in.blocklen[i] = blocklen[i];
      mem->mm.in.index[i] = index[i];
      nb += elemsize*blocklen[i];
    }
    mem->nbytes = nb;
#ifdef QMP_DECLARE_MSGMEM
    QMP_DECLARE_MSGMEM(mem);
#endif
  } else {
    QMP_SET_STATUS_CODE (QMP_NOMEM_ERR);
  }

  LEAVE;
  return mem;
}


/* Basic buffer destructor */
void
QMP_free_msgmem(QMP_msgmem_t mem)
{
  ENTER;
  QMP_assert(mem!=NULL);

#ifdef QMP_FREE_MSGMEM
  QMP_FREE_MSGMEM(mem);
#endif
  if ( mem->type == MM_indexed_buf) {
    QMP_free(mem->mm.in.index);
  } else if ( mem->type == MM_strided_array_buf) {
    QMP_free(mem->mm.sa.disp);
    QMP_free(mem->mm.sa.blksize);
    QMP_free(mem->mm.sa.nblocks);
    QMP_free(mem->mm.sa.stride);
  }
  QMP_free(mem);

  LEAVE;
}


/* Alloc message handler */
static QMP_msghandle_t 
alloc_msghandle(void)
{
  QMP_msghandle_t mh;
  ENTER;

  QMP_alloc(mh, struct QMP_msghandle_struct, 1);
  if (mh) {
    mh->type = MH_empty;
    mh->activeP = 0;
    mh->clear_to_send = 0;
    mh->num = 0;
    mh->base = NULL;
    mh->mm = NULL;
    mh->dest_node = -1;
    mh->srce_node = -1;
    mh->next = NULL;
    mh->err_code = QMP_SUCCESS;
    mh->uses = 0;
    mh->priority = 0;
    mh->paired = 0;
  }
#ifdef QMP_ALLOC_MSGHANDLE
  QMP_ALLOC_MSGHANDLE(mh);
#endif

  LEAVE;
  return mh;
}


/* Basic handler destructor */
void
QMP_free_msghandle (QMP_msghandle_t msgh)
{
  ENTER;

  if (msgh) {
    if(msgh->num==0) 
      QMP_FATAL("error: attempt to free one message handle of a multiple");

#ifdef QMP_FREE_MSGHANDLE
    QMP_FREE_MSGHANDLE(msgh);
#endif
    switch (msgh->type) {
    case MH_multiple: {
      QMP_msghandle_t current = msgh->next;
      while (current) {
	msgh->next = current->next;
#ifdef QMP_FREE_MSGHANDLE
	QMP_FREE_MSGHANDLE(current);
#endif
	QMP_free(current);
	current = msgh->next;
      }
      QMP_free(msgh);
    } break;

    case MH_empty:
    case MH_send:
    case MH_recv:
      QMP_free(msgh);
      break;

    default:
      QMP_FATAL("internal error: unknown message handle");
      break;
    }
  }

  LEAVE;
}


static QMP_msghandle_t
declare_receive(QMP_comm_t comm, QMP_msgmem_t mm, int sourceNode, int axis, int dir, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  QMP_assert(mm != NULL);
  QMP_assert(sourceNode >= 0);
  QMP_assert(sourceNode < QMP_comm_get_number_of_nodes(comm));

  mh = alloc_msghandle();
  if (mh) {
    mh->type = MH_recv;
    mh->num = 1;
    mh->base = mm->mem;
    mh->mm = mm;
    mh->comm = comm;
    mh->dest_node = QMP_comm_get_node_number(comm);
    mh->srce_node = sourceNode;
    mh->axis = axis;
    mh->dir = dir;
    mh->priority = priority;
    mh->next = NULL;

#ifdef _QMP_DEBUG
    QMP_info ("node %d recv from %d of %d bytes\n",
	      mh->dest_node, mh->srce_node, mm->nbytes);
#endif

#ifdef QMP_DECLARE_RECEIVE
    QMP_DECLARE_RECEIVE(mh);
#else
    // can't send to self in single-node mode
    QMP_assert(sourceNode != QMP_comm_get_node_number(comm));
#endif
  }

  LEAVE;
  return mh;
}


static QMP_msghandle_t
declare_send(QMP_comm_t comm, QMP_msgmem_t mm, int destNode, int axis, int dir, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  QMP_assert(mm != NULL);
  QMP_assert(destNode >= 0);
  QMP_assert(destNode < QMP_comm_get_number_of_nodes(comm));

  mh = alloc_msghandle();
  if (mh) {
    mh->type = MH_send;
    mh->num = 1;
    mh->base = mm->mem;
    mh->mm = mm;
    mh->comm = comm;
    mh->dest_node = destNode;
    mh->srce_node = QMP_comm_get_node_number(comm);
    mh->axis = axis;
    mh->dir = dir;
    mh->priority = priority;
    mh->next = NULL;

#ifdef _QMP_DEBUG
    QMP_info ("node %d send to %d of %d bytes\n",
	      mh->srce_node, mh->dest_node, mm->nbytes);
#endif

#ifdef QMP_DECLARE_SEND
    QMP_DECLARE_SEND(mh);
#else
    // can't send to self in single-node mode
    QMP_assert(destNode != QMP_comm_get_node_number(comm));
#endif
  }

  LEAVE;
  return mh;
}


/* Message handle routines */
QMP_msghandle_t
QMP_comm_declare_receive_from (QMP_comm_t comm, QMP_msgmem_t mm, int sourceNode, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = declare_receive(comm, mm, sourceNode, -1, 0, priority);

  LEAVE;
  return mh;
}

QMP_msghandle_t
QMP_declare_receive_from (QMP_msgmem_t mm, int sourceNode, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = declare_receive(QMP_comm_get_default(), mm, sourceNode, -1, 0, priority);

  LEAVE;
  return mh;
}

QMP_msghandle_t
QMP_comm_declare_receive_relative(QMP_comm_t comm, QMP_msgmem_t mm, int dir, int isign, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  QMP_assert(QMP_comm_logical_topology_is_declared(comm));
  QMP_assert(dir>=0);
  QMP_assert(dir<QMP_comm_get_logical_number_of_dimensions(comm));

  int ii = (isign > 0) ? 1 : 0;
  int sourceNode = comm->topo->neigh[ii][dir];
  mh = declare_receive(comm, mm, sourceNode, dir, isign, priority);

  LEAVE;
  return mh;
}

QMP_msghandle_t
QMP_declare_receive_relative(QMP_msgmem_t mm, int dir, int isign, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = QMP_comm_declare_receive_relative(QMP_comm_get_default(), mm, dir, isign, priority);

  LEAVE;
  return mh;
}


QMP_msghandle_t
QMP_comm_declare_send_to (QMP_comm_t comm, QMP_msgmem_t mm, int destNode, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = declare_send(comm, mm, destNode, -1, 0, priority);

  LEAVE;
  return mh;
}


QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t mm, int destNode, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = declare_send(QMP_comm_get_default(), mm, destNode, -1, 0, priority);

  LEAVE;
  return mh;
}


QMP_msghandle_t
QMP_comm_declare_send_relative (QMP_comm_t comm, QMP_msgmem_t mm, int dir, int isign, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  QMP_assert(QMP_comm_logical_topology_is_declared(comm));
  QMP_assert(dir>=0);
  QMP_assert(dir<QMP_comm_get_logical_number_of_dimensions(comm));

  int ii = (isign > 0) ? 1 : 0;
  int destNode = comm->topo->neigh[ii][dir];
  mh = declare_send(comm, mm, destNode, dir, isign, priority);

  LEAVE;
  return mh;
}

QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t mm, int dir, int isign, int priority)
{
  QMP_msghandle_t mh;
  ENTER;

  mh = QMP_comm_declare_send_relative(QMP_comm_get_default(), mm, dir, isign, priority);

  LEAVE;
  return mh;
}


/* Declare multiple messages */
/* What this does is just link the first (non-null) messages
 * to subsequent messages */
/* paired specifies whether all sends and receives are paired with in this group */
static QMP_msghandle_t
QMP_declare_multiple_paired(QMP_msghandle_t msgh[], int nhandle, int paired)
{
  QMP_msghandle_t mh0, mh;
  ENTER;
  int num=0;
  int i;

  mh0 = alloc_msghandle();
  if(mh0) {
    /* The first handle is a indicator it is a placeholder */
    mh0->type = MH_multiple;
    mh0->paired = paired;

    /* Count and make sure there are nhandle valid messages */
    for(i=0; i < nhandle; ++i) {
      QMP_assert(msgh[i]!=NULL);
      if(msgh[i]->type==MH_multiple) {
	num += msgh[i]->num;
      } else {
	QMP_assert(msgh[i]->num==1);
	num++;
      }
    }
    mh0->num = num;
    mh = mh0;
    num = 0;
    /* Link the input messages to the first message */
    for(i=0; i < nhandle; ++i) {
      if(msgh[i]->type==MH_multiple) {
	mh->next = msgh[i]->next;
#ifdef QMP_FREE_MSGHANDLE
	QMP_FREE_MSGHANDLE(msgh[i]);
#endif
	QMP_free(msgh[i]);
	while(mh->next) {
	  mh = mh->next;
	  mh->num = 0;
	  num++;
	}
      } else {
	mh->next = msgh[i];
	mh = mh->next;
	mh->num = 0;
	num++;
      }
    }
    QMP_assert(mh0->num==num);

#ifdef QMP_DECLARE_MULTIPLE
    QMP_DECLARE_MULTIPLE(mh0);
#endif
  }

  LEAVE;
  return mh0;
}


QMP_msghandle_t
QMP_declare_multiple(QMP_msghandle_t msgh[], int nhandle)
{
  QMP_msghandle_t mh;
  ENTER;
  mh = QMP_declare_multiple_paired(msgh, nhandle, 0);
  LEAVE;
  return mh;
}


QMP_msghandle_t
QMP_declare_send_recv_pairs(QMP_msghandle_t msgh[], int nhandle)
{
  QMP_msghandle_t mh;
  ENTER;
  mh = QMP_declare_multiple_paired(msgh, nhandle, 1);
  LEAVE;
  return mh;
}


QMP_status_t
QMP_change_address (QMP_msghandle_t mh, void *addr)
{
  QMP_status_t ret_val = QMP_SUCCESS;
  ENTER;

  QMP_assert(mh->type!=MH_multiple);
  QMP_assert(mh->num!=0);

  mh->base = (char *) addr;
#ifdef QMP_CHANGE_ADDRESS
  QMP_CHANGE_ADDRESS(mh);
#endif

  LEAVE;
  return ret_val;
}


QMP_status_t
QMP_change_address_multiple (QMP_msghandle_t mh0, void *addr[], int naddr)
{
  QMP_status_t ret_val = QMP_SUCCESS;
  ENTER;

  QMP_assert(mh0->type==MH_multiple);
  QMP_assert(mh0->num==naddr);

  int i=0;
  QMP_msghandle_t mh = mh0;
  while(mh->next) {
    mh = mh->next;
    mh->base = (char *) addr[i];
    i++;
  }
#ifdef QMP_CHANGE_ADDRESS
  QMP_CHANGE_ADDRESS(mh0);
#endif

  LEAVE;
  return ret_val;
}
