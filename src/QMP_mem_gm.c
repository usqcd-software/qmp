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
 *      Memory handling routines for QMP
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.15  2003/01/08 20:37:48  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.14  2002/11/15 15:37:34  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.13  2002/10/03 16:46:35  chen
 *   Add memory copy, change event loops
 *
 *   Revision 1.12  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.11  2002/04/30 18:26:36  chen
 *   Allow QMP_gm to send/recv from itself
 *
 *   Revision 1.10  2002/04/30 17:51:50  chen
 *   minor fix
 *
 *   Revision 1.9  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.8  2002/04/22 20:28:42  chen
 *   Version 0.95 Release
 *
 *   Revision 1.7  2002/03/28 18:48:20  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.6  2002/03/27 20:48:49  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.5  2002/02/15 20:34:53  chen
 *   First Beta Release QMP
 *
 *   Revision 1.4  2002/01/27 20:53:50  chen
 *   minor change
 *
 *   Revision 1.3  2002/01/27 17:38:57  chen
 *   change of file structure
 *
 *   Revision 1.2  2002/01/24 20:10:50  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:51  chen
 *   initial import of QMP
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "QMP.h"
#include "QMP_P_GM.h"


/**
 * Message Handle State String.
 */
static char* QMP_msghandle_states[] = {"QMP_MH_INVALID",
				       "QMP_MH_IDLE",
				       "QMP_MH_WAITING"
};



/**
 * Channel State String.
 */
static char* QMP_ch_states[] = {"UNKNOWN",
				"QMP_NOT_CONNECTED",
				"QMP_CONNECTING",
				"UNKNOWN",
				"QMP_CONNECTION_PENDING",
				"UNKNOWN",
				"UNKNOWN",
				"UNKNOWN",
				"QMP_CONNECTED",
				"UNKNOWN",
};

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
  return memalign (QMP_MEM_ALIGNMENT, nbytes);
}

/**
 * Free memory allocated by the above routine.
 */
void
QMP_free_aligned_memory (void* mem)
{
  free (mem);
}

#ifdef _QMP_USE_MULTI_PORTS
/**
 * Free memory pointed by QMP_msgmem_t pointer
 */
void
QMP_free_msgmem (QMP_msgmem_t m)
{
  int i;
  QMP_msgmem_i_t* mem;

  QMP_TRACE ("QMP_free_msg_memory");

  mem = (QMP_msgmem_i_t *)m;
  if (!mem->mem)
    return;
  
  if (mem->ref_count > 0) {
    QMP_info ("Free msg memory 0x%x has refcount %d\n", mem,
	      mem->ref_count);
    return;
  }

  /* free memory */
  if (mem->type == QMP_MM_LEXICO_BUF)
    free (mem->mem);
  mem->mem = 0;
  mem->nbytes = 0;
  mem->glm = 0;

  /* free shadow memory buffers */
  if (mem->do_mem_copy) {
    for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++)
      free (mem->buffers[i].mem);
  }

  mem->data_head = mem->data_tail = 0;
  mem->null_head = mem->null_tail = 0;

  /* free memory handle itself */
  free (mem);
}

/**
 * Create a message memory using memory created by user.
 */
static QMP_status_t
qmp_declare_msg_mem_i (QMP_machine_t* glm,
		       const void* mem, 
		       QMP_u32_t nbytes,
		       QMP_msgmem_i_t** pptr)
{
  int i;
  QMP_msgmem_i_t* retmem;

  QMP_TRACE ("qmp_declare_msg_mem_i");

  if (glm->inited == QMP_FALSE) {
    QMP_error ("QMP messaging system has not been initialized yet.");
    return QMP_NOT_INITED;
  }

  retmem = (QMP_msgmem_i_t *)malloc(sizeof(QMP_msgmem_i_t));
  if (!retmem) {
    QMP_error ("cannot allocate msgmem structure.");
    return QMP_NOMEM_ERR;
  }

  /* now everything is ok */
  retmem->mem = (void *)mem;
  retmem->nbytes = nbytes;
  retmem->type = QMP_MM_USER_BUF;
  retmem->ref_count = 0;
  retmem->glm = glm;
  retmem->do_mem_copy = QMP_FALSE;

  for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++) {
    retmem->buffers[i].mem = 0;
    retmem->buffers[i].mh = 0;
    retmem->buffers[i].next = 0;    
    retmem->buffers[i].prev = 0;
    retmem->buffers[i].idx = i;
    retmem->buffers[i].done = 0;
  }
  retmem->data_head = retmem->data_tail = 0;
  retmem->null_head = retmem->null_tail = 0;

  *pptr = retmem;
  return QMP_SUCCESS;
}

/**
 * Set up shadow receiving buffers
 */
static QMP_bool_t
create_shadow_buffers (QMP_msghandle_i_t* mh)
{
  int i;
  QMP_msgmem_i_t* msg_mem;

  QMP_TRACE ("create_shadow_buffers");

  msg_mem = mh->memory;
#ifdef _QMP_DEBUG
  QMP_fprintf (stderr, "Create shadow buffers for msgmem %p\n", msg_mem);
#endif
  for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++) {
    /**
     * This buffer memory may have been created in previous calls.
     */
    if (!msg_mem->buffers[i].mem)
      msg_mem->buffers[i].mem = QMP_allocate_aligned_memory(msg_mem->nbytes);

    /**
     * Check memory creation.
     */
    if (!msg_mem->buffers[i].mem)
      return QMP_FALSE;

    /**
     * Set up all necessary links.
     */
    if (i == 0) {
      msg_mem->buffers[i].prev = 0;
      msg_mem->buffers[i].next = &(msg_mem->buffers[i + 1]);
    }
    else if (i == QMP_NUM_SHADOW_BUFFERS - 1) {
      msg_mem->buffers[i].next = 0;
      msg_mem->buffers[i].prev = &(msg_mem->buffers[i - 1]);
    }
    else {
      msg_mem->buffers[i].next = &(msg_mem->buffers[i + 1]);
      msg_mem->buffers[i].prev = &(msg_mem->buffers[i - 1]);
    }
    msg_mem->buffers[i].idx = i;
    msg_mem->buffers[i].done = 0;
    msg_mem->buffers[i].mh = mh;
  }
  /**
   * No data available yet.
   */
  msg_mem->data_head = msg_mem->data_tail = 0;

  /**
   * Set null head and tail to the correct positions.
   */
  msg_mem->null_head = &(msg_mem->buffers[0]);
  msg_mem->null_tail = &(msg_mem->buffers[QMP_NUM_SHADOW_BUFFERS - 1]);

  /* turn on do memory copy flag */
  msg_mem->do_mem_copy = QMP_TRUE;

  return QMP_TRUE;
}

/**
 * Print out information for message handle
 */
void
QMP_print_msg_handle (QMP_msghandle_t h)
{
  QMP_msghandle_i_t *mh = (QMP_msghandle_i_t *)h;
  char              type_str[128];

  fprintf (stderr, "Message Handle %p has information: \n", mh);
  if (mh->type & QMP_MH_MULTIPLE) 
    sprintf (type_str, "Multiple ");
  else
    sprintf (type_str, "Single   ");
  
  if (mh->type & QMP_MH_RECV)
    strcat (type_str, "QMP_MH_RECV");
  else
    strcat (type_str, "QMP_MH_SEND");
  
    
  fprintf (stderr, "Type: %s\n", type_str);
  fprintf (stderr,"State: %s\n", QMP_msghandle_states[mh->state]);
  fprintf (stderr,"ID: %d\n", mh->ch.id);
  fprintf (stderr,"Memory: %p length %d ref %d\n", 
	   mh->memory->mem, mh->memory->nbytes, mh->memory->ref_count);
  fprintf (stderr,"Channel State: %s\n", 
	   QMP_ch_states[mh->ch.state]);
  fprintf (stderr,"Channel: transfer count is %d\n", mh->ch.cnt);
  fprintf (stderr,"Channel: my_node rank %d\n", mh->ch.my_node.logic_rank);
  fprintf (stderr,"Channel: my_device %d\n", mh->ch.my_device);
  fprintf (stderr,"Channel: my_port_num %d\n", mh->ch.my_port_num);
  fprintf (stderr,"Channel: my_node_id %d\n", mh->ch.my_node_id);
  fprintf (stderr,"Channel: gm_msg_size %d\n", mh->ch.gm_msg_size);  
  fprintf (stderr,"Channel: rem_node rank %d\n", mh->ch.rem_node.logic_rank);
  fprintf (stderr,"Channel: rem_node type %d\n", mh->ch.rem_node.type);
  fprintf (stderr,"Channel: rem_port_num %d\n", mh->ch.rem_port_num);
  fprintf (stderr,"Channel: rem_node_id %d\n", mh->ch.rem_node_id);
}

/**
 * Declare a message handle for send/recv operation.
 *
 * Arguments:
 *    glm:   global machine
 *    mm:    message memory handler (could be null).
 *    ready_to_use: boolean variable that denotes whether this message
 *                  handle is going to be used to do send/receive.
 *    recv:         a boolean variable that denotes this message handle
 *                  is for receiving.
 */
static QMP_status_t
qmp_create_msg_handle (QMP_machine_t* glm, QMP_msgmem_i_t* mm, 
		       QMP_bool_t ready_to_use,
		       QMP_bool_t recv,
		       QMP_msghandle_i_t** pptr)
{
  int                i;
  QMP_msghandle_i_t* mh;
  QMP_u32_t          port;
  gm_status_t        status;

  QMP_TRACE ("qmp_create_msg_handle");

  /**
   * check whether our system is initialize correctly 
   */
  if (glm->inited != QMP_TRUE || !glm->tpl) {
    QMP_error ("qmp_create_msg_handles: QMP system has not been initialized correctly.");
    return QMP_NOT_INITED;
  }

  /**
   * Check memory handle first to make sure it is ok.
   */
  if (ready_to_use && (!mm->mem || !mm->nbytes)) {
    QMP_error ("bad memory handle for message handle.");
    return QMP_BAD_MEMORY;
  }

  if (*pptr == 0) { 
    /**
     * allocate memory for internal message handle.
     */
    mh = (QMP_msghandle_i_t *)malloc(sizeof (QMP_msghandle_i_t));
    if (!mh) {
      QMP_error ("cannot allocate reveive message handle.");
      return QMP_NOMEM_ERR;
    }
    mh->memory = mm;

    /**
     * State of this message handle is invalid in the beginning.
     */
    mh->state = QMP_MH_INVALID;
    mh->err_code = QMP_SUCCESS;
  
    /**
     * QMP message passing machine pointer.
     */
    mh->glm = glm;

    /**
     * Now create a new gm_port for this channel.
     */
    mh->ch.state = QMP_NOT_CONNECTED;
  
    /*
     * Set packet count to 0.
     */
    mh->ch.cnt = 0;

    /**
     * This node information: copy from tpl.
     */
    mh->ch.my_node.type = QMP_UNKNOWN;
    mh->ch.my_node.logic_rank = glm->tpl->logic_rank;
    /**
     * This a pointer to tpl->phys. do not free.
     */
    mh->ch.my_node.phys = glm->tpl->phys;
    mh->ch.my_device = glm->device;
    mh->ch.my_node_id = glm->node_id;
  }
  else
    mh = *pptr;

  
  /**
   * Allocate a new GM port for receivers only.
   */
  port = 2;
  if (recv) {
    /**
     * Find a gm port that is not used.
     */
    port = QMP_find_gm_port (glm);
    if (port == 0xffffffff) {
      QMP_info ("no more gm port available.");
      /**
       * Return a partially populated mh
       */
      mh->ch.my_port_num = port;

      /**
       * Remote node information: unset.
       */
      mh->ch.rem_node.type = QMP_UNKNOWN;
      mh->ch.rem_node.logic_rank = 0;
      mh->ch.rem_node.phys = 0;
      mh->ch.rem_port_num = 0;
      mh->ch.rem_node_id = 0;

      mh->next = mh->m_next = 0;
      
      /**
       * Ready to return
       */
      *pptr = mh;
      return QMP_NO_PORTS;
    }
    mh->ch.my_port_num = port;
    status = gm_open (&mh->ch.my_port, mh->ch.my_device,
		      mh->ch.my_port_num, "Receive_CH",
		      GM_API_VERSION_1_2);
    if (status != GM_SUCCESS) {
      gm_perror ("could not open gm port", status);
      free (mh);
      return status;
    }
    /**
     * Let gm manage send tokens for this port
     */
    gm_free_send_tokens (mh->ch.my_port, QMP_MSG_PRIORITY,
			 gm_num_send_tokens (mh->ch.my_port)/2);
  }
  else {
    mh->ch.my_port = glm->port;
    mh->ch.my_port_num = glm->port_num;
    mh->ch.my_device = glm->device;
  }


  /**
   * Register memory.
   */
  if (mm && mm->ref_count == 0) {
    /**
     * Only first memory handle can ping memory
     */
    mh->ch.gm_msg_size = gm_min_size_for_length (mm->nbytes);

    /**
     * Ping memory into OS.
     */
    gm_register_memory (mh->ch.my_port, mh->memory->mem, 
			mh->memory->nbytes);

    /**
     * if the message size is smaller than the threshold, we create
     * a few shadow buffers
     */
    if (mh->memory->nbytes <= QMP_MEM_COPY_THRESHOLD) {
      if (create_shadow_buffers (mh) == QMP_FALSE) {
	QMP_error ("Cannot allocate shadow memory buffers\n");
	gm_deregister_memory (mh->ch.my_port, mh->memory->mem, 
			      mh->memory->nbytes);
	if (mh->type == QMP_MH_RECV)
	  gm_close (mh->ch.my_port);
	return QMP_NOMEM_ERR;
      }
      else {
	for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++) {
	  gm_register_memory (mh->ch.my_port, mh->memory->buffers[i].mem,
			      mh->memory->nbytes);
	  /**
	   * Use tag number to find out which buffer is receiving data
	   */
#ifdef _QMP_DEBUG
	QMP_fprintf (stderr, "Msg handle %p provide memory buffer with tag %d\n",
		     mh, QMP_NUM_SHADOW_BUFFERS + mh->memory->buffers[i].idx);
#endif
	/**
	 * If this is a receiving message handle, provide the shadow buffers
	 * to GM receiving system.
	 */
	if (recv)
	  gm_provide_receive_buffer_with_tag (mh->ch.my_port, 
					      mh->memory->buffers[i].mem,
					      mh->ch.gm_msg_size, 
					      QMP_MSG_PRIORITY,
					      QMP_NUM_SHADOW_BUFFERS + 
					      mh->memory->buffers[i].idx);
#ifdef _QMP_DEBUG
	  QMP_fprintf (stderr, "Provide %d shadow buffers to receive data\n",
		       QMP_NUM_SHADOW_BUFFERS);
#endif
	}
      }
    }
  }
  else
    mh->ch.gm_msg_size = 0;

  if (recv) 
    /**
     * Update active_ports mask for receivers since senders use port 2.
     */
    QMP_set_gm_port (mh->glm, port);

  /**
   * Remote node information: unset.
   */
  mh->ch.rem_node.type = QMP_UNKNOWN;
  mh->ch.rem_node.logic_rank = 0;
  mh->ch.rem_node.phys = 0;
  mh->ch.rem_port_num = 0;
  mh->ch.rem_node_id = 0;

  mh->next = mh->m_next = 0;

  *pptr = mh;
  return QMP_SUCCESS;
}


/**
 * Create a message handle that is used when a sender sends a request
 * to a node that does not have a matching receiver. The message handle
 * then will be used as if it were real matching receiver for the sender.
 * When a declare_receive is called, the linked list will be checked to
 * see whether there are any message handles already matching declare_receive
 */
QMP_msghandle_i_t *
QMP_create_conn_recv_msghandle (QMP_machine_t* glm, 
				QMP_dir_t direction,
				QMP_u32_t rem_rank)
{
  QMP_msghandle_i_t* mh;
  QMP_status_t status;

  QMP_TRACE ("QMP_create_conn_recv_msghandle");

  mh = 0;
  status = qmp_create_msg_handle (glm, 0, QMP_FALSE, QMP_TRUE, &mh);

  /**
   * If error is not related no gm port available
   */
  if (status != QMP_SUCCESS && status != QMP_NO_PORTS)
    return 0;

  /* receiver has channel id 0 */
  mh->ch.id = 0;
  /* update this message handle type */
  mh->type = QMP_MH_RECV;

  /* update remote node information  */
  mh->ch.rem_node.type = direction;
  mh->ch.rem_node.logic_rank = rem_rank;
  mh->ch.rem_node.phys = QMP_physical_from_logical(rem_rank);

  /**
   * Add this msg handler to message handle list.
   */
  mh->next = 0;
  QMP_add_pending_conn_req (glm, mh);

  return mh;
}

/**
 * Find a matching message handle from the connection message
 * handle list.
 */
static QMP_msghandle_i_t *
qmp_find_matching_msg_handle (QMP_machine_t* glm,
			      QMP_msgmem_i_t* mm, 
			      QMP_dir_t direction,
			      QMP_u32_t rem_rank)
{
  int                i;
  QMP_msghandle_i_t *p;
  QMP_bool_t         found;

  QMP_TRACE ("qmp_find_matching_msg_handle");

  found = QMP_FALSE;
  p = glm->conn_handles;
  while (p) {
    if (direction == p->ch.rem_node.type && 
	rem_rank == p->ch.rem_node.logic_rank) {
      found = QMP_TRUE;
      break;
    }
    p = p->next;
  }

  if (!found)
    return 0;

  /**
   * Check memory handle first to make sure it is ok.
   */
  if (!mm->mem || !mm->nbytes) {
    QMP_error ("bad memory handle for message handle.");
    return 0;
  }

  /**
   * attach memory pointer to the message handle.
   * increase ref count by 1 for memory
   */
  p->memory = mm;
  assert (p->type == QMP_MH_RECV);
  mm->ref_count++;

  p->ch.state = QMP_CONNECTED;
  p->ch.gm_msg_size = gm_min_size_for_length (mm->nbytes);

  /**
   * Ping memory into OS.
   */
  gm_register_memory (p->ch.my_port, p->memory->mem, 
		      p->memory->nbytes);

  /**
   * we will create shadow buffers
   * if the message size is smaller than the threshold
   */
  if (p->memory->nbytes <= QMP_MEM_COPY_THRESHOLD) {
    if (create_shadow_buffers (p) == QMP_FALSE) {
      gm_deregister_memory (p->ch.my_port, p->memory->mem, 
			    p->memory->nbytes);
      if (p->type == QMP_MH_RECV)
	gm_close (p->ch.my_port);
      QMP_error_exit ("Fatal: Cannot allocate shadow memory buffers\n");
    }
    else {
      for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++) {
	gm_register_memory (p->ch.my_port, p->memory->buffers[i].mem,
			    p->memory->nbytes);
	/**
	 * Use tag number to find out which buffer is receiving data
	 */
#ifdef _QMP_DEBUG
	QMP_fprintf (stderr, "Msg handle %p provide memory buffer with tag %d\n",
		     p, QMP_NUM_SHADOW_BUFFERS + p->memory->buffers[i].idx);
#endif
	gm_provide_receive_buffer_with_tag (p->ch.my_port, 
					    p->memory->buffers[i].mem,
					    p->ch.gm_msg_size, 
					    QMP_MSG_PRIORITY,
					    QMP_NUM_SHADOW_BUFFERS +
					    p->memory->buffers[i].idx);
      }
    }
  }

  /**
   * Remove this message handle from pending list
   */
  QMP_remove_pending_conn_req (glm, p);

  /**
   * Add this message handle to message handle list.
   */
  QMP_add_msg_handle (glm, p);

  return p;
}

/**
 * Declare an endpoint for message channel operation using remote
 * node's direction or rank.
 *
 * Directions are named as: QMP_DIRXP, QMP_DIRXM, QMP_DIRYP, QMP_DIRYM,
 * QMP_DIRZP,  QMP_DIRZM,  QMP_DIRTP,  QMP_DIRTM
 */
static QMP_status_t
qmp_declare_receive_i (QMP_msgmem_i_t* mm, 
		       QMP_dir_t direction,
		       QMP_u32_t node,
		       QMP_msghandle_i_t** pptr)
{
  QMP_msghandle_i_t* mh;
  QMP_status_t status;
  QMP_u32_t rem_node_rank;

  QMP_TRACE ("qmp_declare_receive_i");

  mh = 0;
  /**
   * Get remote node rank for this direction.
   */
  if (direction < QMP_UNKNOWN) 
    rem_node_rank = QMP_get_logical_node_number_for_neighbor (direction);
  else 
    /* Get logical node id from physical node */
    rem_node_rank = QMP_allocated_to_logical (node);

  if (rem_node_rank >= mm->glm->tpl->num_nodes) {
    QMP_error ("remote node rank is too big for QMP_declare_receive_from.");
    return QMP_NODE_OUTRANGE;
  }
  else if (rem_node_rank == mm->glm->tpl->logic_rank) {
    QMP_info ("Declare receive from the same node.");
  }
  
  /**
   * Check whether this memory is already used or declared by others.
   *
   * Check this for only receiver since a registered memory
   * is assoicated with a gm port.
   */
  if (QMP_memory_declared (mm->glm, mm)) {
    QMP_error ("QMP_declare_recv: this memory handle is already used by others.");
    return QMP_MEMUSED_ERR;
  }

  if (mm->ref_count > 0) {
    QMP_error ("QMP_declare_recv: this memory handle is in use by another message handle.");
    return QMP_MEMUSED_ERR;
  }

  /**
   * Check whether there is a matching message handle created
   * before declare_receive called.
   */
  mh = qmp_find_matching_msg_handle(mm->glm, mm, direction, rem_node_rank);
  if (mh) {
    /* update this message state to IDLE */
    mh->state = QMP_MH_IDLE;

    *pptr = mh;
    return QMP_SUCCESS;
  }

  /**
   * Now we have to create a new message handle.
   */
  mh = 0;
  status = qmp_create_msg_handle (mm->glm, mm, QMP_TRUE, QMP_TRUE, &mh);
  if (status != QMP_SUCCESS) 
    return status;

  /* increase ref count of this memory */
  mm->ref_count++;

  /* receiver has channel id 0 */
  mh->ch.id = 0;
  /* update this message handle type */
  mh->type = QMP_MH_RECV;
  /* update this message state to IDLE */
  mh->state = QMP_MH_IDLE;

  /* update remote node information  */
  mh->ch.rem_node.type = direction;
  mh->ch.rem_node.logic_rank = rem_node_rank;
  mh->ch.rem_node.phys = QMP_physical_from_logical(rem_node_rank);

  /**
   * Add this msg handler to message handle list.
   */
  QMP_add_msg_handle (mh->glm, mh);

  *pptr = mh;
  return QMP_SUCCESS;
}
  

/**
 * Free a message handle.
 */
void
QMP_free_msghandle (QMP_msghandle_t h)
{
  int                i;
  QMP_msghandle_i_t* mh;

  QMP_TRACE ("QMP_free_msghandle");

  mh = (QMP_msghandle_i_t *)h;
  if (!(mh->type & QMP_MH_MULTIPLE)) {
    /**
     * This is a single message handle.
     */
    if (mh->memory) {
      if (mh->memory->do_mem_copy && (mh->type & QMP_MH_SEND)) {
	/**
	 * For a sender we have to check whether there are outstanding
	 * send request pending.
	 */
	QMP_wait_for_send_to_finish (mh->glm, mh);
      }

      --mh->memory->ref_count;

      /**
       * No one is using this memory now.
       */
      if (mh->memory->ref_count <= 0) {
	/**
	 * First unpin memory
	 */
	gm_deregister_memory (mh->ch.my_port, mh->memory->mem,
			      mh->memory->nbytes);

      }

      /**
       * Unping shadow memory buffer
       */
      if (mh->memory->do_mem_copy) {
#ifdef _QMP_DEBUG
	QMP_fprintf (stderr, "Release shadow memory buffers\n");
#endif
	for (i = 0; i < QMP_NUM_SHADOW_BUFFERS; i++) 
	  gm_deregister_memory (mh->ch.my_port, mh->memory->buffers[i].mem,
				mh->memory->nbytes);
      }
    }

    if (mh->ch.state == QMP_CONNECTED) {
      if (mh->type == QMP_MH_RECV) {
	/**
	 * Send to peer to notify this receiver is gone.
	 */
	QMP_close_receiver_connection (mh->glm, mh);
	
	/**
	 * close this gm_port for receiver only
	 */
	gm_close (mh->ch.my_port);
    
	/**
	 * Remove this port from the active ports list.
	 */
	QMP_clear_gm_port (mh->glm, mh->ch.my_port_num);
      }
      else 
	/**
	 * For sender it has to wait to disconnection message to show up
	 */
	QMP_wait_for_receiver_disconnect (mh->glm, mh);
    }


    /**
     * Check whether remote node physical geoemtry.
     */
    if (mh->ch.rem_node.phys) {
      free (mh->ch.rem_node.phys);
      mh->ch.rem_node.phys = 0;
    }
    /**
     * I do not free memory here, user has to do it.
     * need more discussion.
     */
    QMP_remove_msg_handle (mh->glm, mh);

    /**
     * Finally free handle.
     */
    free (mh);
  }
  else {
    /**
     * This is a message handle that is the head of a list
     * of message handles.
     */
    QMP_msghandle_i_t *p, *q;

    /**
     * Do we delete individual message handle ? Not yet.
     * Need discussion. -- pending
     */
    p = mh;
    /**
     * Turn off multiple flag.
     */
    p->type ^= QMP_MH_MULTIPLE;

    while (p) {
      q = p->m_next;
      p->m_next = 0;
      p = q;
    }
  }
}

/**
 * Declares an endpoint for a message channel send operation.
 */
static QMP_status_t
qmp_declare_send_i (QMP_msgmem_i_t* mm, QMP_dir_t direction,
		    QMP_u32_t node,
		    QMP_msghandle_i_t** pptr)
{
  QMP_msghandle_i_t* mh;
  QMP_status_t       status;
  QMP_u32_t          rem_node_rank;

  QMP_TRACE ("qmp_declare_send_i");

  /**
   * Get remote node rank for this direction.
   */
  if (direction < QMP_UNKNOWN) 
    rem_node_rank = QMP_get_logical_node_number_for_neighbor (direction);
  else
  /**
   * Convert physical node id to logical node id 
   */
    rem_node_rank = QMP_allocated_to_logical (node);

  if (mm->ref_count > 0) {
    /**
     * Each message handle use a unique memory handle for now.
     */
    QMP_error ("QMP_declare_send: this memory handle is in use by another message handle.");
    return QMP_MEMUSED_ERR;
  }

  mh = 0;
  status = qmp_create_msg_handle (mm->glm, mm, QMP_TRUE, QMP_FALSE, &mh);

  if (status != QMP_SUCCESS)
    return status;

  /**
   * Check whether a remote node rank is a valid remote node.
   */
  if (rem_node_rank >= mm->glm->tpl->num_nodes) {
    QMP_error("remode node number is too big for QMP_declare_send_to.");
    QMP_free_msghandle (mh);
    return QMP_NODE_OUTRANGE;
  }
  else if (rem_node_rank == mm->glm->tpl->logic_rank) 
    QMP_info ("Declare send to the same node.");

  /**
   * Set unique channel id.
   */
  mh->ch.id = mh->glm->chid++;
  if (mh->glm->chid == QMP_ID_FINAL_VALUE)
    mh->glm->chid = QMP_ID_INIT_VALUE;

  /* update this message handle type */
  mh->type = QMP_MH_SEND;
  /* update state of this message handle */
  mh->state = QMP_MH_IDLE;

  /* update remote node information  */
  mh->ch.rem_node.type = direction;
  if (direction == QMP_UNKNOWN) 
    mh->ch.rem_node.logic_rank = rem_node_rank;
  else 
    mh->ch.rem_node.logic_rank = QMP_get_logical_node_number_for_neighbor (direction);
  mh->ch.rem_node.phys =QMP_physical_from_logical(mh->ch.rem_node.logic_rank);

  /**
   * Add this msg handler to message handle list.
   */
  QMP_add_msg_handle (mh->glm, mh);

  /**
   * Now send message and wait for connection reply
   */
  status = QMP_init_wait_for_gm_connection (mh->glm, mh);

  if (status != QMP_SUCCESS) {
    QMP_error ("cannot establish sender channel : %s\n",
	       QMP_error_string(status));
    QMP_print_msg_handle (mh);
    QMP_free_msghandle (mh);
    return status;
  }

  assert(mh->ch.state >= QMP_CONNECTION_PENDING);

  /* increase ref count of this memory */
  mm->ref_count++;

  *pptr = mh;
  return QMP_SUCCESS;
}


/**
 * Create a new QMP channel control message.
 */
QMP_local_ctrlmsg_t *
QMP_create_ctrlmsg (QMP_machine_t* glm,
		    QMP_msghandle_i_t* mh,
		    QMP_u32_t init_refcount,
		    QMP_local_ctrlmsg_t* next)
{
  QMP_local_ctrlmsg_t* ret;

  QMP_TRACE ("QMP_create_ctrlmsg");

  ret = (QMP_local_ctrlmsg_t *)malloc(sizeof(QMP_local_ctrlmsg_t));
  if (!ret) {
    QMP_error ("cannot allocate memory for local QMP_local_ctrlmsg.");
    return 0;
  }

  ret->ctrlmsg = (QMP_ch_ctrlmsg_t *)gm_dma_malloc(glm->port, 
						   sizeof (QMP_ch_ctrlmsg_t));
  if (!ret->ctrlmsg) {
    QMP_error ("cannot dma allocate memory for channel control message.");
    free (ret);
    return 0;
  }
  ret->glm = glm;
  ret->mh = mh;
  ret->ref_count = init_refcount;
  ret->next = next;

  return ret;
}

/**
 * Free memory associated with channel control message.
 *
 * If the boolean variable force == true, then the memory
 * will be deleted no matter whether ref_count <= 0 or not.
 */
void
QMP_delete_ctrlmsg (QMP_local_ctrlmsg_t* lcm, QMP_bool_t force)
{
  QMP_TRACE ("QMP_delete_ctrlmsg");

  if (--lcm->ref_count == 0 || force == QMP_TRUE) { 
    /**
     * free memory
     */
    if (lcm->ctrlmsg)
      gm_dma_free (lcm->glm->port, (void *)lcm->ctrlmsg);

    free (lcm);
  }
}
#else /* _QMP_USE_MULTI_PORTS */

/**
 * Free memory pointed by QMP_msgmem_t pointer
 */
void
QMP_free_msgmem (QMP_msgmem_t m)
{
  QMP_msgmem_i_t* mem;

  QMP_TRACE ("QMP_free_msg_memory");

  mem = (QMP_msgmem_i_t *)m;
  if (!mem->mem)
    return;
  
  if (mem->ref_count > 0) {
    QMP_info ("Free msg memory 0x%x has refcount %d\n", mem,
	      mem->ref_count);
    return;
  }

  /* free memory */
  if (mem->type == QMP_MM_LEXICO_BUF)
    free (mem->mem);
  mem->mem = 0;
  mem->nbytes = 0;
  mem->glm = 0;

  /* free memory handle itself */
  free (mem);
}

/**
 * Create a message memory using memory created by user.
 */
static QMP_status_t
qmp_declare_msg_mem_i (QMP_machine_t* glm,
		       const void* mem, 
		       QMP_u32_t nbytes,
		       QMP_msgmem_i_t** pptr)
{
  QMP_msgmem_i_t* retmem;

  QMP_TRACE ("qmp_declare_msg_mem_i");

  if (glm->inited == QMP_FALSE) {
    QMP_error ("QMP messaging system has not been initialized yet.");
    return QMP_NOT_INITED;
  }
  if (nbytes > glm->max_msg_length) {
    QMP_error ("Trying to allocate memory is bigger than system limit %d\n",
	       glm->max_msg_length);
    return QMP_MEMSIZE_TOOBIG;
  }

  retmem = (QMP_msgmem_i_t *)malloc(sizeof(QMP_msgmem_i_t));
  if (!retmem) {
    QMP_error ("cannot allocate msgmem structure.");
    return QMP_NOMEM_ERR;
  }

  /* now everything is ok */
  retmem->mem = (void *)mem;
  retmem->nbytes = nbytes;
  retmem->type = QMP_MM_USER_BUF;
  retmem->ref_count = 0;
  retmem->glm = glm;

  *pptr = retmem;
  return QMP_SUCCESS;
}

/**
 * Print out information for message handle
 */
void
QMP_print_msg_handle (QMP_msghandle_t h)
{
  QMP_msghandle_i_t *mh = (QMP_msghandle_i_t *)h;
  char              type_str[128];

  fprintf (stderr, "Message Handle %p has information: \n", mh);
  if (mh->type & QMP_MH_MULTIPLE) 
    sprintf (type_str, "Multiple ");
  else
    sprintf (type_str, "Single   ");
  
  if (mh->type & QMP_MH_RECV)
    strcat (type_str, "QMP_MH_RECV");
  else
    strcat (type_str, "QMP_MH_SEND");
  
    
  fprintf (stderr, "Type: %s\n", type_str);
  fprintf (stderr,"State: %s\n", QMP_msghandle_states[mh->state]);
  fprintf (stderr,"Memory: %p length %d ref %d\n", 
	   mh->memory->mem, mh->memory->nbytes, mh->memory->ref_count);
  fprintf (stderr, "Tag: %d and cnt %d\n",mh->tag, mh->cnt);
  fprintf (stderr, "HDR: magic 0x%x version 0x%x operation %d source %d destination %d datatype %d count %d\n", mh->req->hdr.magic,
	   mh->req->hdr.vers, mh->req->hdr.op,
	   mh->req->hdr.source, mh->req->hdr.dest,
	   mh->req->hdr.data_type, mh->req->hdr.count);
  fprintf (stderr, "REQ: type 0x%x state %d priority %d buffer len %d buffer %p\n", 
	   mh->req->type, mh->req->state, mh->req->pri,
	   mh->req->buflen, mh->req->buffer);
}

/**
 * Free a message handle.
 */
void
QMP_free_msghandle (QMP_msghandle_t h)
{
  QMP_msghandle_i_t* mh;

  QMP_TRACE ("QMP_free_msghandle");

  mh = (QMP_msghandle_i_t *)h;
  if (!(mh->type & QMP_MH_MULTIPLE)) {
    /**
     * Reset qs->mh to null since this free_msghandle can be called before
     * send/receive finished
     */
    if (mh->req)
      mh->req->mh = 0;

    /**
     * This is a single message handle.
     */
    --mh->memory->ref_count;

    /**
     * I do not free memory here, user has to do it.
     * need more discussion.
     */
    QMP_remove_msg_handle (mh->glm, mh);

    /**
     * Finally free handle.
     */
    free (mh);
  }
  else {
    /**
     * This is a message handle that is the head of a list
     * of message handles.
     */
    QMP_msghandle_i_t *p, *q;

    /**
     * Do we delete individual message handle ? Not yet.
     * Need discussion. -- pending
     */
    p = mh;
    /**
     * Turn off multiple flag.
     */
    p->type ^= QMP_MH_MULTIPLE;

    while (p) {
      q = p->m_next;
      p->m_next = 0;
      p = q;
    }
  }
}


/**
 * Declare a message handle for send/recv operation.
 *
 * Arguments:
 *    glm:   global machine
 *    mm:    message memory handler
 *    recv:         a boolean variable that denotes this message handle
 *                  is for receiving.
 *    remote: remote node of this send/recv
 *    pptr:   returning message handle pointer
 */
static QMP_status_t
qmp_create_msg_handle (QMP_machine_t* glm, QMP_msgmem_i_t* mm, 
		       QMP_bool_t recv,
		       QMP_u32_t  remote,
		       QMP_msghandle_i_t** pptr)
{
  QMP_msghandle_i_t* mh;

  QMP_TRACE ("qmp_create_msg_handle");

  /**
   * check whether our system is initialize correctly 
   */
  if (glm->inited != QMP_TRUE || !glm->tpl) {
    QMP_error ("qmp_create_msg_handles: QMP system has not been initialized correctly.");
    return QMP_NOT_INITED;
  }


  /**
   * allocate memory for internal message handle.
   */
  mh = (QMP_msghandle_i_t *)malloc(sizeof (QMP_msghandle_i_t));
  if (!mh) {
    QMP_error ("cannot allocate reveive message handle.");
    return QMP_NOMEM_ERR;
  }
  mh->memory = mm;

  /**
   * State of this message handle is invalid in the beginning.
   */
  mh->state = QMP_MH_IDLE;
  mh->err_code = QMP_SUCCESS;

  /**
   * QMP message passing machine pointer.
   */
  mh->glm = glm;

  if (recv) {
    /**
     * This is a receiver
     */
    if (glm->tpl)
      mh->dest = glm->tpl->logic_rank;
    else
      mh->dest = QMP_PHYS_RANK (glm->phys);

    mh->source = remote;

    mh->type = QMP_MH_RECV;

    /**
     * Initially I will receive from anybody before a desired channel
     * is established
     */
    mh->tag  = QMP_TAG_WILDCARD;
  }
  else {
    /**
     * This is a sender
     */
    if (glm->tpl)
      mh->source = glm->tpl->logic_rank;
    else
      mh->source = QMP_PHYS_RANK (glm->phys);

    mh->dest = remote;

    mh->type = QMP_MH_SEND;

    /**
     * Sender will send out a unqiue tag 
     * This has to be changed to Unix like FD_SET/CLR style
     * Jie Chen 1/7/2003
     */
    mh->tag = glm->chid++;
    if (glm->chid == QMP_END_TAG)
      glm->chid = QMP_START_TAG;
  }

  mh->cnt = 0;
  mh->next = mh->m_next = 0;
  mh->req = 0;

  *pptr = mh;

  return QMP_SUCCESS;
}

/**
 * Declare an endpoint for message channel operation using remote
 * node's direction or rank.
 *
 * Directions are named as: QMP_DIRXP, QMP_DIRXM, QMP_DIRYP, QMP_DIRYM,
 * QMP_DIRZP,  QMP_DIRZM,  QMP_DIRTP,  QMP_DIRTM
 */
static QMP_status_t
qmp_declare_receive_i (QMP_msgmem_i_t* mm, 
		       QMP_dir_t direction,
		       QMP_u32_t node,
		       QMP_msghandle_i_t** pptr)
{
  QMP_msghandle_i_t* mh;
  QMP_status_t status;
  QMP_u32_t rem_node_rank;

  QMP_TRACE ("qmp_declare_receive_i");

  mh = 0;

  /**
   * Get remote node rank for this direction.
   */
  if (direction < QMP_UNKNOWN) 
    rem_node_rank = QMP_get_logical_node_number_for_neighbor (direction);
  else 
    /* Get logical node id from physical node */
    rem_node_rank = QMP_allocated_to_logical (node);

  if (rem_node_rank >= mm->glm->tpl->num_nodes) {
    QMP_error ("remote node rank is too big for QMP_declare_receive_from.");
    return QMP_NODE_OUTRANGE;
  }
  else if (rem_node_rank == mm->glm->tpl->logic_rank) {
    QMP_info ("Declare receive from the same node.");
    return QMP_INVALID_ARG;
  }
  
  /**
   * Check whether this memory is already used or declared by others.
   *
   * Check this for only receiver since a registered memory
   * is assoicated with a gm port.
   */
  if (QMP_memory_declared (mm->glm, mm)) {
    QMP_error ("QMP_declare_recv: this memory handle is already used by others.");
    return QMP_MEMUSED_ERR;
  }

  if (mm->ref_count > 0) {
    QMP_error ("QMP_declare_recv: this memory handle is in use by another message handle.");
    return QMP_MEMUSED_ERR;
  }

  status = qmp_create_msg_handle (mm->glm, mm, QMP_TRUE, rem_node_rank, &mh);

  if (status != QMP_SUCCESS) 
    return status;

  /* increase ref count of this memory */
  mm->ref_count++;

  /**
   * Add this msg handler to message handle list.
   */
  QMP_add_msg_handle (mh->glm, mh);

  *pptr = mh;
  return QMP_SUCCESS;
}

/**
 * Declares an endpoint for a message channel send operation.
 */
static QMP_status_t
qmp_declare_send_i (QMP_msgmem_i_t* mm, QMP_dir_t direction,
		    QMP_u32_t node,
		    QMP_msghandle_i_t** pptr)
{
  QMP_msghandle_i_t* mh;
  QMP_status_t       status;
  QMP_u32_t          rem_node_rank;

  QMP_TRACE ("qmp_declare_send_i");

  /**
   * Get remote node rank for this direction.
   */
  if (direction < QMP_UNKNOWN) 
    rem_node_rank = QMP_get_logical_node_number_for_neighbor (direction);
  else
    /**
     * Convert physical node id to logical node id 
     */
    rem_node_rank = QMP_allocated_to_logical (node);

  if (mm->ref_count > 0) {
    /**
     * Each message handle use a unique memory handle for now.
     */
    QMP_error ("QMP_declare_send: this memory handle is in use by another message handle.");
    return QMP_MEMUSED_ERR;
  }

  /**
   * Check whether a remote node rank is a valid remote node.
   */
  if (mm->glm->tpl) {
    if (rem_node_rank >= mm->glm->tpl->num_nodes) {
      QMP_error("remode node number is too big for QMP_declare_send_to.");
      return QMP_NODE_OUTRANGE;
    }
    else if (rem_node_rank == mm->glm->tpl->logic_rank) {
      QMP_error ("Declare send to the same node.");
      return QMP_INVALID_ARG;
    }
  }
  else {
    if (rem_node_rank >= QMP_NUM_NODES(mm->glm->phys)) {
      QMP_error("remode node number is too big for QMP_declare_send_to.");
      return QMP_NODE_OUTRANGE;
    }
    else if (rem_node_rank == QMP_PHYS_RANK(mm->glm->phys)) {
      QMP_error ("Declare send to the same node.");
      return QMP_INVALID_ARG;
    }
  }


  mh = 0;
  status = qmp_create_msg_handle (mm->glm, mm, QMP_FALSE, rem_node_rank,
				  &mh);

  if (status != QMP_SUCCESS)
    return status;


  /**
   * Add this msg handler to message handle list.
   */
  QMP_add_msg_handle (mh->glm, mh);

  /* increase ref count of this memory */
  mm->ref_count++;

  *pptr = mh;
  return QMP_SUCCESS;
}




#endif /* _QMP_USE_MULTI_PORTS */

/**
 * Create a message memory using memory created by user.
 *
 * This is a public function
 */
QMP_msgmem_t 
QMP_declare_msgmem (const void* m, 
		    QMP_u32_t nbytes)
{
  QMP_status_t status;
  QMP_msgmem_i_t* mem = 0;
  QMP_machine_t *glm = &QMP_global_m;

  status =  qmp_declare_msg_mem_i (glm, m, nbytes, &mem);
  QMP_SET_STATUS_CODE(glm, status);

  return (QMP_msgmem_t)mem;
}

/**
 * Declare a strided memory.
 * Not yet implemented
 */
QMP_msgmem_t
QMP_declare_strided_msgmem (void* base, 
			    QMP_u32_t blksize,
			    QMP_u32_t nblocks,
			    QMP_u32_t stride)
{
  QMP_error ("QMP_declare_strided_msgmem: not yet implemented.");
  exit (1);
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
  QMP_error ("QMP declare_declare_strided_array_msgmem: not yet implemented.");
  exit (1);
  /* make compiler happy */
  return (QMP_msgmem_t)0;
}


/**
 * Declares an endpoint for a message channel send operation
 * between this node and it's neighbor
 *
 * The neighbor is described by an axis and direction.
 * The axes are from 0 to dim, and directions are 1 and -1.
 */
QMP_msghandle_t
QMP_declare_send_relative (QMP_msgmem_t m, QMP_s32_t axis,
			   QMP_s32_t direction,
			   QMP_s32_t priority)
{
  QMP_status_t status;
  QMP_msghandle_i_t* mh;
  QMP_msgmem_i_t* mm;
  QMP_dir_t    neighbor;
  
  status = QMP_SUCCESS;
  mh = 0;
  mm = (QMP_msgmem_i_t *)m;

  /** 
   * Check whether axis and direction are all valid.
   */
  if (axis >= QMP_get_logical_number_of_dimensions () || axis < 0) {
    QMP_error ("QMP_declare_send_relative has invalid axis argument.");
    status = QMP_INVALID_ARG;
  }
  if (direction != 1 && direction != -1) {
    QMP_error ("QMP_declare_send_relative has invalid direction argument.");
    status = QMP_INVALID_ARG;
  }
  if (status != QMP_SUCCESS) {
    QMP_SET_STATUS_CODE(mm->glm, status);
    return (QMP_msghandle_t)0;
  } 


  /**
   * describe a neighbor using QMP_dir_t which can be
   * QMP_DIRXP, QMP_DIRXM and so on.
   */
  neighbor = 2 * axis - (direction - 1)/2;

  status = qmp_declare_send_i (mm, neighbor, 0, &mh);
  QMP_SET_STATUS_CODE(mm->glm, status);
  return (QMP_msghandle_t)mh;
}


/**
 * Declares an endpoint for a message channel send operation.
 */
QMP_msghandle_t
QMP_declare_send_to (QMP_msgmem_t m, QMP_u32_t rem_node_rank,
		     QMP_s32_t priority)
{
  QMP_status_t status;
  QMP_msghandle_i_t* mh;
  QMP_msgmem_i_t* mm;

  mh = 0;
  mm = (QMP_msgmem_i_t *)m;
  status = qmp_declare_send_i (mm, QMP_UNKNOWN, rem_node_rank, &mh);
  QMP_SET_STATUS_CODE(mm->glm, status);
  return (QMP_msghandle_t)mh;
}  

/**
 * Declare an endpoint for message channel communication bwteen
 * this node and it's neighbor
 *
 * The neighbor is described by an axis and direction.
 * The axes are from 0 to dim, and directions are 1 and -1.
 */
QMP_msghandle_t 
QMP_declare_receive_relative (QMP_msgmem_t m, QMP_s32_t axis,
			      QMP_s32_t direction,
			      QMP_s32_t priority)
{
  QMP_status_t status;
  QMP_msghandle_i_t* mh;
  QMP_msgmem_i_t* mm;
  QMP_dir_t    neighbor;

  status = QMP_SUCCESS;
  mm = (QMP_msgmem_i_t *)m;
  mh = 0;

  /** 
   * Check whether axis and direction are all valid.
   */
  if (axis >= QMP_get_logical_number_of_dimensions () || axis < 0) {
    QMP_error ("QMP_declare_receive_relative has invalid axis argument.");
    status = QMP_INVALID_ARG;
  }
  if (direction != 1 && direction != -1) {
    QMP_error ("QMP_declare_receive_relative has invalid direction argument.");
    status = QMP_INVALID_ARG;
  }
  if (status != QMP_SUCCESS) {
    QMP_SET_STATUS_CODE(mm->glm, status);
    return (QMP_msghandle_t)0;
  }
    
   
  /**
   * describe a neighbor using QMP_dir_t which can be
   * QMP_DIRXP, QMP_DIRXM and so on.
   */
  neighbor = 2 * axis - (direction - 1)/2;

  status = qmp_declare_receive_i (mm, neighbor, 0, &mh);
  QMP_SET_STATUS_CODE(mm->glm, status);
  return (QMP_msghandle_t)mh;

}

/**
 * Declare an endpoint for message channel operation using remote
 * node's number.
 *
 */
QMP_msghandle_t 
QMP_declare_receive_from (QMP_msgmem_t m, QMP_u32_t rem_node_rank,
			  QMP_s32_t priority)
{
  QMP_status_t status;
  QMP_msghandle_i_t* mh;
  QMP_msgmem_i_t* mm;

  mh = 0;
  mm = (QMP_msgmem_i_t *)m;
  status = qmp_declare_receive_i (mm, QMP_UNKNOWN, rem_node_rank, &mh);
  QMP_SET_STATUS_CODE(mm->glm, status);
  return (QMP_msghandle_t)mh;
}

/**
 * Collapse multiple message handle into a single one.
 * 
 * Put all send message handles in front of those receivers
 */
QMP_msghandle_t
QMP_declare_multiple (QMP_msghandle_t* msgh, QMP_u32_t num)
{
  QMP_msghandle_i_t  *curr, *prev;
  QMP_msghandle_i_t  *last_sender, *first_recver, *first_link;
  int                i;

  QMP_TRACE ("QMP_declare_multiple");

  /**
   * Initialize these variables to null
   */
  first_link = last_sender = first_recver = 0;

  /**
   * Find all senders in the array first, and link them together.
   */
  prev = curr = 0;
  for (i = 0; i < num; i++) {
    curr = (QMP_msghandle_i_t *)msgh[i];
    if (curr->type == QMP_MH_SEND) {
      if (prev)
	prev->m_next = curr;
      else
	first_link = curr;
      prev = curr;
      last_sender = curr;
    }
  }

  /**
   * Find all receivers in the array, and link them together.
   */
  prev = curr = 0;
  for (i = 0; i < num; i++) {
    curr = (QMP_msghandle_i_t *)msgh[i];
    if (curr->type == QMP_MH_RECV) {
      if (prev)
	prev->m_next = curr;
      else
	first_recver = curr;
      prev = curr;
    }
  } 

  /**
   * Link last sender to the first receiver if there are both
   */
  if (first_recver && last_sender) 
    last_sender->m_next = first_recver;
  else if (last_sender) 
    last_sender->m_next = 0;
  else if (first_recver) 
    first_link = first_recver;

  first_link->type |= QMP_MH_MULTIPLE;

  return first_link;
}

