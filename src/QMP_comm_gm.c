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
 *      QMP Messaging system communication part
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.3  2003/02/13 16:22:23  chen
 *   qmp version 1.2
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.19  2003/01/08 20:37:47  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.18  2002/12/10 20:22:22  chen
 *   Add minor fix for ButterFly alg
 *
 *   Revision 1.17  2002/12/05 16:41:01  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.16  2002/11/15 15:37:33  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.15  2002/10/03 16:46:35  chen
 *   Add memory copy, change event loops
 *
 *   Revision 1.14  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.13  2002/07/25 13:37:35  chen
 *   Fix a bug with declare multiple
 *
 *   Revision 1.12  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.11  2002/05/07 17:18:44  chen
 *   Make spec and header file consistent
 *
 *   Revision 1.10  2002/04/22 20:28:40  chen
 *   Version 0.95 Release
 *
 *   Revision 1.9  2002/03/28 18:48:20  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.8  2002/03/27 20:48:49  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.7  2002/02/15 20:34:52  chen
 *   First Beta Release QMP
 *
 *   Revision 1.6  2002/01/27 20:53:50  chen
 *   minor change
 *
 *   Revision 1.5  2002/01/27 17:38:57  chen
 *   change of file structure
 *
 *   Revision 1.4  2002/01/25 18:52:15  chen
 *   get unix style code back in
 *
 *   Revision 1.3  2002/01/24 20:10:49  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.2  2002/01/22 15:52:29  chen
 *   Minor change in code
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:51  chen
 *   initial import of QMP
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_GM.h"

/**
 * forward decleration for a gm receving main loop.
 */
static void qmp_main_loop_i (QMP_machine_t* glm, QMP_msghandle_i_t* h);

/**
 * forward decleration for processing a general message.
 */
static void qmp_process_genmsg_i (QMP_machine_t* glm,
				  QMP_general_msg_header_t* hdr,
				  void *buffer, QMP_u32_t length,
				  QMP_u32_t pri);

/**
 * Get current time in second
 */
static QMP_u32_t
get_current_time (void)
{
#if 1
  struct timeval tv;

  gettimeofday (&tv, 0);

  return (QMP_u32_t)tv.tv_sec;
#endif

#if 0
  /* gm_ticks increase every 0.5usec */
  return gm_ticks(QMP_global_m.port)/500000;
#endif
}


#ifdef _QMP_USE_MULTI_PORTS
/**
 * Print out control message structure.
 */
static void print_ch_ctrlmsg (QMP_ch_ctrlmsg_t* cmsg)
{
  fprintf (stderr, "Control Message: magic 0x%x version 0x%x op %d status %d port_num %d node_id %d gm_size %d logic_rank %d direction %d id %d\n", 
	   cmsg->magic,
	   cmsg->vers,
	   cmsg->op,
	   cmsg->status,
	   cmsg->gm_port_num,
	   cmsg->gm_node_id,
	   cmsg->gm_size,
	   cmsg->logic_rank,
	   cmsg->direction,
	   cmsg->id);
}

/**
 * Internal control message gm_send_callback's callback function.
 * 
 * Inside this send callback function, allocated send memory
 * will be released.
 */
static void
ctrlmsg_sent (struct gm_port* port, void* arg, gm_status_t status)
{
  QMP_local_ctrlmsg_t* lcm;
  QMP_msghandle_i_t *  mh;
  
  QMP_TRACE ("sender_ctrlmsg_sent");

  lcm = (QMP_local_ctrlmsg_t *)arg;
  mh = lcm->mh;
  if (status == GM_SUCCESS) {
    if (mh->type & QMP_MH_SEND) {
      if (QMP_rt_verbose) {
	QMP_info ("sender message handler %p sent control message out.", mh);
	print_ch_ctrlmsg (lcm->ctrlmsg);
      }
      /**
       * Sometimes this callback is invoked after the connection reply message
       * for this sender is processed. The connection reply message
       * will set channel state to connected state.
       * jie chen 10/3/2002
       */
      if (mh->ch.state != QMP_CONNECTED)
	mh->ch.state = QMP_CONNECTING;
    }
    else {
      if (QMP_rt_verbose) {
	QMP_info ("receiver message handler %p sent back reply.", mh);
	print_ch_ctrlmsg (lcm->ctrlmsg);
      }
      /**
       * If a receiver replys a connection and has a success status,
       * it is connected.
       * 
       * If a receiver replys a disconnection, it is disconnected.
       * 
       */
      if (lcm->ctrlmsg->status == QMP_SUCCESS && 
	  lcm->ctrlmsg->op == QMP_CH_CONN_RPLY)
	mh->ch.state = QMP_CONNECTED;
      else if (lcm->ctrlmsg->status == QMP_SUCCESS &&
	       lcm->ctrlmsg->op == QMP_CH_DISC)
	mh->ch.state = QMP_NOT_CONNECTED;
    }
  }
  /**
   * GM give up a send token occupied by this send. I acquire this send token
   */
  QMP_ACQUIRE_SEND_TOKEN(mh->glm, QMP_CHMSG_PRIORITY);

  /* free memory associated with this control message */
  QMP_delete_ctrlmsg (lcm, QMP_FALSE);
}

/**
 * Send a control message to remote node specified by the message handle.
 *
 * Arguments:
 *   glm:        QMP global machine instance.
 *   mh:         Message Handle.
 *   ctrlmsg:    A control message that is being sent.
 *
 *   Destination is determined by channel remote node inside the message
 *   handle.
 */
static QMP_status_t
send_ctrlmsg (QMP_machine_t* glm, 
	      QMP_msghandle_i_t* mh,
	      QMP_local_ctrlmsg_t* ctrlmsg)
{
  QMP_rtenv_t rem_machine;
  int         i;
  QMP_ch_state_t old_state;

  QMP_TRACE ("send_ctrlmsg");

  /**
   * get remote machine with default communication points.
   * control messages travel on a default port (2).
   *
   * The other end of a communication channel is at the same machine but
   * with a different port.
   */
  rem_machine = QMP_get_machine_info (mh->glm, mh->ch.rem_node.phys);
  if (!rem_machine) {
    QMP_error("cannot get remote machine information to establish a channel.");
    return QMP_RTENV_ERR;
  }

#ifdef _QMP_CTRL_DEBUG
  if (mh->type & QMP_MH_SEND)
    fprintf (stderr, "sender %p send out message op %d to %s with device %d port %d and node_id %d\n",
	    mh,
	    ctrlmsg->ctrlmsg->op,
	    rem_machine->host, 
	    rem_machine->device,
	    rem_machine->port, 
	    rem_machine->node_id);
  else
    fprintf (stderr, "receiver %p send out message op %d to %s with device %d port %d and node_id %d\n",
	    mh,
	    ctrlmsg->ctrlmsg->op,
	    rem_machine->host, 
	    rem_machine->device,
	    rem_machine->port, 
	    rem_machine->node_id);
#endif

  /**
   * Check whether we have a send token or not.
   */
  if (!QMP_HAS_SEND_TOKENS(glm, QMP_CHMSG_PRIORITY)) {
    QMP_error ("no more gm_send tokens available.");
    QMP_delete_ctrlmsg (ctrlmsg, QMP_TRUE);
    return QMP_SVC_BUSY;
  }

  /** 
   * Remember current state.
   */
  old_state = mh->ch.state;
  
  /**
   * Now send this message until the connection is established.
   */
  ctrlmsg->ref_count++;
  gm_send_with_callback (glm->port, (void *)ctrlmsg->ctrlmsg, 
			 QMP_CRLMSG_SIZE,
			 sizeof (QMP_ch_ctrlmsg_t),
			 QMP_CHMSG_PRIORITY,
			 rem_machine->node_id, rem_machine->port,
			 ctrlmsg_sent, (void *)ctrlmsg);

  /**
   * Give up a send token to GM.
   */
  QMP_GIVEUP_SEND_TOKEN(glm, QMP_CHMSG_PRIORITY);

  /**
   * Since this is a control message, I will do a quick gm loop
   */
  i = 0;
  while (mh->ch.state == old_state && i < QMP_NUMLOOPS_PSEC) {
    qmp_main_loop_i (glm, mh);
    i++;
  }

  return QMP_SUCCESS;
}

    

/**
 * Populate a channel control message structure using a message
 * handle, operation and status code.
 *
 * This routine is used by eithera sender or a receiver.
 */
static void
set_ctrlmsg_value (QMP_ch_ctrlmsg_t *ctrlmsg,
		   QMP_u32_t operation,
		   QMP_msghandle_i_t* mh,
		   QMP_status_t status,
		   QMP_bool_t sender)
{
  QMP_TRACE ("set_ctrlmsg_value");

  ctrlmsg->magic = QMP_MAGIC;
  ctrlmsg->vers = QMP_VERSION_CODE;
  ctrlmsg->op = operation;
  ctrlmsg->status = status;
  
  /* fill this sender information. */
  ctrlmsg->gm_port_num = mh->ch.my_port_num;
  ctrlmsg->gm_node_id = mh->ch.my_node_id;
  ctrlmsg->gm_size = mh->ch.gm_msg_size;
  if (mh->memory)
    ctrlmsg->mem_len = mh->memory->nbytes;
  else
    ctrlmsg->mem_len = 0;
  ctrlmsg->logic_rank = mh->ch.my_node.logic_rank;

  /* fill direction field that is in destination's perspective. */
  /* i.e. QMP_DIRXP becoms QMP_DIRXM and so on                  */
  if (mh->ch.rem_node.type != QMP_UNKNOWN) {
    if (mh->ch.rem_node.type %2 == 0)
      /* positive direction */
      ctrlmsg->direction = mh->ch.rem_node.type + 1;
    else
      /* negative direction */
      ctrlmsg->direction = mh->ch.rem_node.type - 1;
  }
  else
    ctrlmsg->direction = QMP_UNKNOWN;

  /* set control message id the same as message channel id */
  ctrlmsg->id = mh->ch.id;
}

/**
 * A message handle receiver sends back reply back to sender.
 */
static QMP_status_t
send_conn_reply_ctrlmsg (QMP_machine_t*     glm,
			 QMP_msghandle_i_t* mh,
			 QMP_ch_ctrlmsg_t*  req)
{
  QMP_status_t err, status;
  QMP_local_ctrlmsg_t* reply;

  QMP_TRACE ("send_conn_reply");


  err = QMP_SUCCESS;
  /**
   * allocate a channel control msg structure which is sent back to 
   * the sender.
   */
  reply = QMP_create_ctrlmsg (glm, mh, 0, 0);

  if (!reply) {
    err = QMP_NOMEM_ERR;
    reply = glm->err_ctrlmsg;
  }
  else {
    if (mh->memory) {
      /**
       * This is a real message handler which has memory.
       */

      /**
       * First check whether a receiver gm size is >= than
       * the sender requested gm size.
       */
      if (req->gm_size > mh->ch.gm_msg_size || 
	  req->mem_len > mh->memory->nbytes) {
	err = QMP_MEMSIZE_ERR;
	mh->ch.id = req->id;
      }
      else {
	/**
	 * update message handle remote node information.
	 */
	mh->ch.rem_port_num = req->gm_port_num;
	mh->ch.rem_node_id = req->gm_node_id;
	mh->ch.id = req->id;
      }

    }
    else {
      /**
       * no matter whether we have a matched receiver or not
       * we do send back connection using a pending connection
       * message handles. We are hoping the message handles will be
       * used when declare_receive is called later.
       */
      mh->ch.gm_msg_size = req->gm_size;
      /**
       * update message handle remote node information.
       */
      mh->ch.rem_port_num = req->gm_port_num;
      mh->ch.rem_node_id = req->gm_node_id;
      mh->ch.id = req->id;
    }


    /**
     * Check receiving port number, if it is == 0xffffffff
     * then set err number to QMP_NO_PORTS
     */
    if (mh->ch.my_port_num == 0xffffffff) 
      err = QMP_NO_PORTS;
  }

  /**
   * Send control message back
   */
  if (err == QMP_SUCCESS) {
    set_ctrlmsg_value (reply->ctrlmsg, QMP_CH_CONN_RPLY, mh, 
		       err, QMP_FALSE);
    /**
     * Send control reply message back.
     */
    status = send_ctrlmsg (glm, mh, reply);
  }
  else {
    set_ctrlmsg_value (reply->ctrlmsg, QMP_CH_CONN_RPLY, mh, 
		       err, QMP_FALSE);

    mh->ch.id = 0;
    status = err;

    send_ctrlmsg (glm, mh, reply);
    /**
     * Remove this message handle if this message handle is from
     * NO_PORT error
     */
    if (err == QMP_NO_PORTS) {
      QMP_remove_pending_conn_req (glm, mh);
      free (mh);
    }
  }
  return status;
}


/**
 * Process a channel connection request message.
 *
 * this routine is called by a receiver only.
 *
 * Arguments:
 *    glm:     global QMP machine instance.
 *    mh:      this message handle.
 *    ctrlmsg: control message received by gm.
 */
static QMP_status_t
process_conn_ctrlmsg (QMP_machine_t*     glm,
		      QMP_msghandle_i_t* mh,
		      QMP_ch_ctrlmsg_t*  ctrlmsg)
{
  QMP_msghandle_i_t *found, *p;
  QMP_status_t err;

  QMP_TRACE ("process_conn_ctrlmsg");

  found = 0;
  err = QMP_SUCCESS;
  /**
   * find a matching receiver for this connection request.
   *
   * check message handle h first :::
   */
  if (mh && (mh->type & QMP_MH_RECV) &&
      ctrlmsg->logic_rank == mh->ch.rem_node.logic_rank &&
      ctrlmsg->direction == mh->ch.rem_node.type) {
    if (mh->ch.id == 0 || ctrlmsg->id == mh->ch.id) 
      /**
       * either the channel is not connected or multiple control
       * messages are sent to the same channel receiver.
       */
      found = mh;
  }
  else {
    /* check whether this message is for one of the message handles */
    p = glm->msg_handles;

    while (p) {
      if ((p->type & QMP_MH_RECV) &&
	  ctrlmsg->logic_rank == p->ch.rem_node.logic_rank &&
	  ctrlmsg->direction == p->ch.rem_node.type) {
	if (p->ch.id == 0 || ctrlmsg->id == p->ch.id) {
	  /**
	   * either the channel is not connected or multiple control
	   * messages are sent to the same channel receiver.
	   */
	  found = p;
	  break;
	}
      }
      p = p->next;
    }
  }
  

  if (!found) {
    /**
     * A connection message may come before a receiver is created.
     * put this message on to the pending request list.
     */
    QMP_msghandle_i_t* mh = QMP_create_conn_recv_msghandle(glm, 
							   ctrlmsg->direction,
							   ctrlmsg->logic_rank);
#ifdef _QMP_CTRL_DEBUG
    QMP_info ("put control message on to the pending request list");
#endif
    found = mh;
  }

  /**
   * send connection reply message back
   */
  err = send_conn_reply_ctrlmsg (glm, found, ctrlmsg);
  
  return err;
}


/**
 * Process a channel connection reply message.
 */
static QMP_status_t
process_conn_reply_ctrlmsg (QMP_machine_t*     glm,
			    QMP_msghandle_i_t* mh,
			    QMP_ch_ctrlmsg_t*  ctrlmsg)
{
  QMP_msghandle_i_t *found, *p;
  QMP_status_t err;

  QMP_TRACE ("process_conn_reply_ctrlmsg");

  err = QMP_SUCCESS;
  found = 0;
  /**
   * Check message handle h first to make sure it is not null
   */
  if (mh && (mh->type & QMP_MH_SEND) && mh->ch.id == ctrlmsg->id) 
    /* this is the message I am waiting for. */
    found = mh;
  else {
    /* check whether this message is for one of the message handles */
    p = glm->msg_handles;

    while (p) {
      /* try to match ctrlmsg id with a sender that has the same id */
      if ((p->type & QMP_MH_SEND) && p->ch.id == ctrlmsg->id) {
	found = p;
	break;
      }
      p = p->next;
    }
  }

  if (!found) {
    QMP_error ("this message has no intended target.");
    err = QMP_BAD_MESSAGE;
  }    
  else {
    err = ctrlmsg->status;

    if (ctrlmsg->status == QMP_SUCCESS &&
	ctrlmsg->logic_rank == found->ch.rem_node.logic_rank) {
      /**
       * Set gm message size to the size returned from the receiver.
       */
      found->ch.gm_msg_size = ctrlmsg->gm_size;
      found->ch.rem_port_num = ctrlmsg->gm_port_num;
      found->ch.rem_node_id = ctrlmsg->gm_node_id;

      /**
       * Now set channel state to connected.
       */
      found->ch.state = QMP_CONNECTED;
#ifdef _QMP_CTRL_DEBUG
      QMP_info ("message handle %p channel id %d now is connected.", 
		found, found->ch.id);
#endif
    }
    else {
      if (err == QMP_NO_PORTS) 
	QMP_info ("Receiver has no gm ports available for this message handle %p\n", found);
      else
	QMP_error ("message handle %p cannot establish connection: error %d\n",
		   found, err);
    }
  }
  return err;
}


/**
 * Process a channel disconnection message.
 */
static QMP_status_t
process_disc_ctrlmsg (QMP_machine_t*     glm,
		      QMP_msghandle_i_t* mh,
		      QMP_ch_ctrlmsg_t*  ctrlmsg)
{
  QMP_msghandle_i_t *found, *p;
  QMP_status_t err;

  QMP_TRACE ("process_disc_ctrlmsg");

  err = QMP_SUCCESS;
  found = 0;
  /**
   * Check message handle h first, make sure it is not 0
   */
  if (mh && (mh->type & QMP_MH_SEND) && mh->ch.id == ctrlmsg->id) 
    /* this is the message I am waiting for. */
    found = mh;
  else {
    /* check whether this message is for one of the message handles */
    p = glm->msg_handles;

    while (p) {
      /* try to match ctrlmsg id with a sender that has the same id */
      if ((p->type & QMP_MH_SEND) && p->ch.id == ctrlmsg->id) {
	found = p;
	break;
      }
      p = p->next;
    }
  }

  if (found) {
    err = ctrlmsg->status;
    if (ctrlmsg->logic_rank == found->ch.rem_node.logic_rank) {
      /**
       * No clean gm port size and other information
       * to prevent this sender from sending something to a wrong
       * receiver.
       */
      found->ch.gm_msg_size = 0;
      found->ch.rem_port_num = 0;
      found->ch.rem_node_id = 0;

      /**
       * Now set channel state to connected.
       */
      found->ch.state = QMP_NOT_CONNECTED;
#ifdef _QMP_CTRL_DEBUG
      QMP_info ("message handle %p channel id %d now is disconnected.", 
		found, found->ch.id);
#endif
    }
  }
  return err;
}


/**
 * Process a control message which could be either a reply or a
 * request message.
 */
static void
qmp_process_ctrlmsg_i (QMP_machine_t* glm,
		       QMP_msghandle_i_t* mh,
		       QMP_ch_ctrlmsg_t* ctrlmsg)
{
  QMP_status_t        err;

  QMP_TRACE("qmp_process_ctrlmsg_i");

  err = QMP_SUCCESS;
  if (ctrlmsg->magic != QMP_MAGIC ||
      ctrlmsg->vers  != QMP_VERSION_CODE) {
    QMP_error ("incoming control message has wrong version or magic number.");
    mh->err_code = QMP_BAD_MESSAGE;
    return;
  }

  switch (ctrlmsg->op) {
  case QMP_CH_CONN_RPLY:
#ifdef _QMP_CTRL_DEBUG
    QMP_fprintf (stderr, "Received connection reply control message\n");
    print_ch_ctrlmsg (ctrlmsg);
#endif 
    err = process_conn_reply_ctrlmsg (glm, mh, ctrlmsg);
    break;
  case QMP_CH_CONN:
#ifdef _QMP_CTRL_DEBUG
    QMP_fprintf (stderr, "Received connection control message\n");
    print_ch_ctrlmsg (ctrlmsg);
#endif
    err = process_conn_ctrlmsg (glm, mh, ctrlmsg);
    break;
  case QMP_CH_DISC:
#ifdef _QMP_CTRL_DEBUG
    QMP_fprintf (stderr, "Received disc connection control message\n");
#endif
    err = process_disc_ctrlmsg (glm, mh, ctrlmsg);
    break;
    break;
  case QMP_CH_DISC_RPLY:
    break;
  default:
    break;
  }
  /**
   * mh could be 0: see note above 
   */
  if (mh)
    mh->err_code = err;
}

/**
 * Wait for a control message and act appropriately for a sender
 * or a receiver.
 *
 * input arguments: 
 *    glm: global machine.
 *    mh:  a message handle.
 *    wait_state: a state mask to wait for.
 *    timeout: total time interval to wait in seconds
 */
static QMP_status_t
wait_for_ctrlmsg (QMP_machine_t*       glm,
		  QMP_msghandle_i_t*   mh,
		  QMP_u32_t            wait_state,
		  QMP_u32_t            timeout)
{
  int              i;
  QMP_u32_t        start, curr;
  
  /**
   * Wait for a control message to establish channel connection
   */
  curr = start = get_current_time ();
  do {
    i = 0;
    /**
     * Process event loop for certain times (roughly one second)
     * before calculting time diffrence again.
     */
    while ((mh->ch.state & wait_state) == 0 && 
	   mh->err_code == QMP_SUCCESS &&
	   i < QMP_NUMLOOPS_PSEC) {
      qmp_main_loop_i (glm, mh);
      i++;
    }
    
    /**
     * Since gm is reliable delivery, so I do not have to send again
     */
    curr = get_current_time ();
  }while ((mh->ch.state & wait_state) == 0 && 
	  mh->err_code == QMP_SUCCESS && curr - start <= timeout);

  if (mh->ch.state != wait_state && curr - start > timeout) {
    mh->err_code = QMP_CH_TIMEOUT;
    return QMP_CH_TIMEOUT;
  }

  return mh->err_code;
}

/**
 * A QMP sender to wait for its receiver to send disconnect information.
 */
QMP_status_t 
QMP_wait_for_receiver_disconnect (QMP_machine_t* glm, 
				  QMP_msghandle_i_t* mh)
{
  return wait_for_ctrlmsg (glm, mh, QMP_NOT_CONNECTED, 
			   QMP_DISC_CTRLMSG_TIMEOUT);
}

/**
 * A QMP Sender (using memory copy) wait for all outstanding send to finish.
 */
QMP_status_t
QMP_wait_for_send_to_finish (QMP_machine_t* glm,
			     QMP_msghandle_i_t* mh)
{
  int              i;
  QMP_u32_t        start, curr, timeout;
  

  /**
   * Wait for 5 seconds for all send to finish
   */
  timeout = 5;
  /**
   * Wait for a control message to establish channel connection
   */
  curr = start = get_current_time ();
  do {
    i = 0;
    /**
     * Process event loop for certain times (roughly one second)
     * before calculting time diffrence again.
     */
    while (!(IS_DATA_LIST_EMPTY(mh->memory)) &&
	   mh->err_code == QMP_SUCCESS &&
	   i < QMP_NUMLOOPS_PSEC) {
      qmp_main_loop_i (glm, mh);
      i++;
    }
    
    /**
     * Since gm is reliable delivery, so I do not have to send again
     */
    curr = get_current_time ();
  }while (!(IS_DATA_LIST_EMPTY(mh->memory)) && 
	  mh->err_code == QMP_SUCCESS && curr - start <= timeout);

  if (!IS_DATA_LIST_EMPTY(mh->memory) && curr - start > timeout) {
    mh->err_code = QMP_TIMEOUT;
    return QMP_TIMEOUT;
  }

  return mh->err_code;  
} 
/**
 * This is routine a sender message handle to send a request
 * to establish a communication channel.
 *
 * called by declare_send routine
 */
QMP_status_t
QMP_init_wait_for_gm_connection (QMP_machine_t* glm,
				 QMP_msghandle_i_t* mh)
{
  QMP_status_t         status;
  QMP_local_ctrlmsg_t* ctrlmsg;

  QMP_TRACE("init_wait_for_gm_connection");

  /**
   * allocate a channel control msg structure which is sent to a remote node.
   */
  ctrlmsg = QMP_create_ctrlmsg (glm, mh, 0, 0);
		
  if (!ctrlmsg) 
    return QMP_NOMEM_ERR;

  /**
   * populate control message structure.
   */
  set_ctrlmsg_value (ctrlmsg->ctrlmsg, QMP_CH_CONN, mh, QMP_SUCCESS, 
		     QMP_TRUE);
   
  /**
   * Send out control message
   */
  status = send_ctrlmsg (glm, mh, ctrlmsg);
  if (status != QMP_SUCCESS)
    return status;

  /**
   * Wait for connection reply message until time out.
   */
  status = wait_for_ctrlmsg (glm, mh, QMP_CONNECTED,
			     QMP_CTRLMSG_WAIT_TIMEOUT);

  return status;
}

/**
 * This is a routine called by a receiver when being freed (closed)
 * to notify the receiver's peer.
 */
QMP_status_t
QMP_close_receiver_connection (QMP_machine_t* glm,
			       QMP_msghandle_i_t *mh)
{
  QMP_local_ctrlmsg_t* ctrlmsg;

  QMP_TRACE ("QMP_close_receiver_connection");

  /**
   * allocate a channel control msg structure which is sent to a remote node.
   */
  ctrlmsg = QMP_create_ctrlmsg (glm, mh, 0, 0);
		
  if (!ctrlmsg) 
    return QMP_NOMEM_ERR;

  /**
   * Populate disconnection control message structure.
   */
  set_ctrlmsg_value (ctrlmsg->ctrlmsg, QMP_CH_DISC, mh,
		     QMP_SUCCESS, QMP_FALSE);

  /**
   * Send out control message
   */
  return send_ctrlmsg (glm, mh, ctrlmsg);
}

/**
 * Retrieve received data from shadow buffers.
 */
static void QMP_RETRIEVE_RECEIVED_DATA (QMP_msghandle_i_t* mh) 
{
  /**
   * Check whether there is something to receive.
   */
  if (mh->memory->data_tail) {
    memcpy (mh->memory->mem, mh->memory->data_tail->mem,
	    mh->memory->nbytes);

#ifdef _QMP_RECV_DEBUG
    QMP_fprintf (stderr, "Copy shadow buffer %d to user memory on receiving\n", 
		 mh->memory->data_tail->idx);
#endif

    /**
     * Provide this buffer to gm receiving queue using tag number 
     */
    gm_provide_receive_buffer_with_tag (mh->ch.my_port,
					mh->memory->data_tail->mem, 
					mh->ch.gm_msg_size,           
					QMP_MSG_PRIORITY,
					QMP_NUM_SHADOW_BUFFERS + 
					mh->memory->data_tail->idx); 

    /**
     * Remove tail from received (data) list.
     */
    DEQUEUE_DATA_LIST(mh->memory);
  }
  else
    mh->state = QMP_MH_WAITING;
}
	   
/**
 * Macro to process GM message for channels.
 */
static void QMP_process_ch_message (gm_recv_event_t* e, 
				    QMP_msghandle_i_t* mh) 
{
  if (mh->memory->do_mem_copy) {
    /**
     * Memory size is small enough to justify to use memory copy
     */
    if (mh->state == QMP_MH_WAITING) {
      /**
       * There is no received shadow buffers, user is waiting.
       */
      switch (gm_ntoh_u8 (e->recv.type)) { 
      case GM_NO_RECV_EVENT:
	break;
      case GM_HIGH_RECV_EVENT:
      case GM_RECV_EVENT:
	assert (!mh->memory->data_head);

#ifdef _QMP_RECV_DEBUG
	QMP_fprintf (stderr, "User memory is busy waiting, tag: %d\n",
		     gm_ntoh_u8(e->recv.tag) - QMP_NUM_SHADOW_BUFFERS);
#endif
	mh->ch.cnt++;
	memcpy (mh->memory->mem, gm_ntohp(e->recv.buffer), 
		gm_ntohl(e->recv.length));
	gm_provide_receive_buffer_with_tag (mh->ch.my_port,
					    gm_ntohp (e->recv.buffer),
					    gm_ntoh_u8(e->recv.size),
					    QMP_MSG_PRIORITY,
					    gm_ntoh_u8(e->recv.tag));
	mh->state = QMP_MH_IDLE;
	break;
      case GM_FAST_HIGH_PEER_RECV_EVENT:
      case GM_FAST_HIGH_RECV_EVENT:
      case GM_FAST_RECV_EVENT:
      case GM_FAST_PEER_RECV_EVENT:
	assert (!mh->memory->data_head);
#ifdef _QMP_RECV_DEBUG
	QMP_fprintf (stderr, "User memory is busy waiting, tag: %d\n",
		     gm_ntoh_u8(e->recv.tag) - QMP_NUM_SHADOW_BUFFERS);
#endif
	mh->ch.cnt++;
	memcpy (mh->memory->mem, gm_ntohp(e->recv.message), 
		gm_ntohl(e->recv.length));
	gm_provide_receive_buffer_with_tag (mh->ch.my_port,
					    gm_ntohp (e->recv.buffer),
					    gm_ntoh_u8(e->recv.size),
					    QMP_MSG_PRIORITY,
					    gm_ntoh_u8(e->recv.tag));
	mh->state = QMP_MH_IDLE;
      break;
    default:
      gm_unknown (mh->ch.my_port, e);
      break; 
      }
    }
    else {
      /**
       * user is not waiting, put data into received list
       */
      QMP_buffer_mem_t* sbuffer = 0;
      switch (gm_ntoh_u8 (e->recv.type)) { 
      case GM_NO_RECV_EVENT:
	break;
      case GM_HIGH_RECV_EVENT:
      case GM_RECV_EVENT:
	mh->ch.cnt++;
	sbuffer = &(mh->memory->buffers[gm_ntoh_u8(e->recv.tag)-QMP_NUM_SHADOW_BUFFERS]);
#ifdef _QMP_RECV_DEBUG 
	QMP_fprintf (stderr, "User memory is not waiting received tag: %d buffer tag : %d\n",
		     gm_ntoh_u8(e->recv.tag) - QMP_NUM_SHADOW_BUFFERS, 
		     sbuffer->idx);
#endif
	/**
	 * Insert this buffer into the biginning of the received list
	 */
	ENQUEUE_DATA_LIST(mh->memory,sbuffer);
	break;
    default:
      gm_unknown (mh->ch.my_port, e);
      break; 
      }
    }
  }
  else {
    /**
     * No meomry copy for big message. Data are directly DMAed into user
     * buffer.
     */
    switch (gm_ntoh_u8 (e->recv.type)) { 
    case GM_NO_RECV_EVENT:
      break;
    case GM_HIGH_RECV_EVENT:
    case GM_RECV_EVENT:
      mh->ch.cnt++;
      mh->state = QMP_MH_IDLE;
      break;
    case GM_FAST_HIGH_PEER_RECV_EVENT:
    case GM_FAST_HIGH_RECV_EVENT:
    case GM_FAST_RECV_EVENT:
    case GM_FAST_PEER_RECV_EVENT:
      mh->ch.cnt++;
      mh->state = QMP_MH_IDLE;
      memcpy (gm_ntohp(e->recv.buffer), gm_ntohp(e->recv.message), 
	      gm_ntohl(e->recv.length));
      break;
    default:
      gm_unknown (mh->ch.my_port, e);
      break; 
    }
  }
}


/**
 * Process general and control message from port 2
 */
static void
QMP_process_message (QMP_machine_t* glm,
		     gm_recv_event_t* e,
		     QMP_msghandle_i_t* mh)
{
  /**
   * process the port for general message and sending ports
   */
  switch (gm_ntoh_u8 (e->recv.type)) {
  case GM_HIGH_RECV_EVENT:
    {
      /**
       * General message for global sum and others.
       */
      char*                     buffer;
      QMP_u32_t                 size, length;
      QMP_general_msg_header_t* hdr;

      buffer = (char *)gm_ntohp (e->recv.buffer);
      size = gm_ntoh_u8 (e->recv.size);
      length = gm_ntoh_u32 (e->recv.length);
      
      hdr = (QMP_general_msg_header_t *)buffer;
      qmp_process_genmsg_i (glm, hdr, 
			    &buffer[sizeof(QMP_general_msg_header_t)], 
			    length - sizeof(QMP_general_msg_header_t),
			    GM_HIGH_PRIORITY);
      /**
       * resupply this buffer back to gm for further receiving.
       */
      gm_provide_receive_buffer (glm->port, buffer, size, 
				 QMP_MSG_PRIORITY);
    }
    break;
  case GM_RECV_EVENT:
    {
      void*               buffer;
      QMP_u32_t           size, length;
      QMP_ch_ctrlmsg_t*   ctrlmsg;

      buffer = gm_ntohp (e->recv.buffer);
      size = gm_ntoh_u8 (e->recv.size);
      length = gm_ntoh_u32 (e->recv.length);

      if (length == sizeof (QMP_ch_ctrlmsg_t) && size == QMP_CRLMSG_SIZE) {
	ctrlmsg = (QMP_ch_ctrlmsg_t *)buffer;
	/**
	 * message handle h could 0 since this can be invoked
	 * from main_loop_i (which happens when receiver has no
	 * sending message handles).
	 * All control message processing code must check message handle h.
	 */
	qmp_process_ctrlmsg_i (glm, mh, ctrlmsg);

	/**
	 * resupply this buffer back to gm for further receiving.
	 */
	gm_provide_receive_buffer (glm->port, buffer, size, 
				   QMP_CHMSG_PRIORITY);
      }
    }
    break;
  default:
    gm_unknown (glm->port, e);
    break;
  }
}
  

/**
 * Start receiving operation for a message handle.
 */
static QMP_status_t
qmp_start_recv_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_TRACE ("qmp_start_recv_i");

  if (mh->memory->do_mem_copy) {
    QMP_RETRIEVE_RECEIVED_DATA (mh);
  }
  else {
    gm_provide_receive_buffer (mh->ch.my_port, 
			       mh->memory->mem,
			       mh->ch.gm_msg_size, 
			       QMP_MSG_PRIORITY);

    mh->state = QMP_MH_WAITING;
  }

  return QMP_SUCCESS;
}

/**
 * Nearest neighbor send callback.
 */
static void
qmp_nbcomm_sent_i (struct gm_port* lport, void* arg, gm_status_t status)
{
  QMP_msghandle_i_t* mh;

  QMP_TRACE ("qmp_nbcomm_sent_i");

  mh = (QMP_msghandle_i_t *)arg;
  if (status == GM_SUCCESS)
    mh->ch.cnt++;
  else
    mh->ch.state = QMP_NOT_CONNECTED;

  /**
   * GM Free the send token, I acquire it back.
   */
  QMP_ACQUIRE_SEND_TOKEN(mh->glm, QMP_MSG_PRIORITY);

  mh->err_code = status;
  mh->state = QMP_MH_IDLE;
}

/**
 * GM Send data callback using memory copy
 */
static void qmp_flush_send_i (QMP_msghandle_i_t* mh);


static void
qmp_userdata_sent_i (struct gm_port* lport, void* arg, gm_status_t status)
{
  QMP_msghandle_i_t* mh;
  QMP_buffer_mem_t*  buffer;

  QMP_TRACE ("qmp_userdata_sent_i");

  buffer = (QMP_buffer_mem_t *)arg;
  mh = buffer->mh;

  if (status == GM_SUCCESS)
    mh->ch.cnt++;
  else 
    mh->ch.state = QMP_NOT_CONNECTED;


  /**
   * Remove buffer from data list and put it back to null list.
   */
  REMOVE_DATA_BUFFER(mh->memory,buffer);
  ENQUEUE_NULL_LIST(mh->memory,buffer);

  /**
   * GM Free the send token, I acquire it back.
   */
  QMP_ACQUIRE_SEND_TOKEN(mh->glm, QMP_MSG_PRIORITY);

  mh->err_code = status;

  /**
   * Now try to send more user data if possible.
   */
  if (status == GM_SUCCESS)
    qmp_flush_send_i (mh);
}

/**
 * Flush out previously copied data which have not been sent out.
 */
static void
qmp_flush_send_i (QMP_msghandle_i_t* mh)
{
  QMP_buffer_mem_t* data;

  QMP_TRACE ("qmp_flush_send_i");

  /** 
   * Check whether user put new data into his/her buffer
   * Now I should have space to copy data into
   */
  if (mh->state == QMP_MH_WAITING) {
#ifdef _QMP_SEND_DEBUG
    QMP_fprintf (stderr, "User is trying to send data again\n");
#endif
    data = mh->memory->null_tail;

    memcpy (data->mem, mh->memory->mem,
	    mh->memory->nbytes);
    
    /**
     * Dequeue the null list
     */
    DEQUEUE_NULL_LIST(mh->memory);

    /**
     * Add data to data list.
     */
    ENQUEUE_DATA_LIST(mh->memory,data);

    data->done = 0; 

    /**
     * Set flag to idle
     */
    mh->state = QMP_MH_IDLE;
  }
  
  if (mh->memory->data_tail) {
    data = mh->memory->data_tail;
    while (data) {
      if (!data->done) {
#ifdef _QMP_SEND_DEBUG
	QMP_fprintf (stderr, "Sending out previously queued data.\n");
#endif
	if (QMP_HAS_SEND_TOKENS(mh->glm,QMP_MSG_PRIORITY)) {
	  gm_send_with_callback (mh->ch.my_port, data->mem,
				 mh->ch.gm_msg_size, mh->memory->nbytes,
				 QMP_MSG_PRIORITY,
				 mh->ch.rem_node_id,
				 mh->ch.rem_port_num,
				 qmp_userdata_sent_i, (void *)data);
	  /**
	   * Give up a send token to gm
	   */
	  QMP_GIVEUP_SEND_TOKEN(mh->glm, QMP_MSG_PRIORITY);
	  
	  /**
	   * Data sent flag set to 1
	   */
	  data->done = 1;
	}
#ifdef _QMP_SEND_DEBUG
	else
	  QMP_fprintf (stderr, "flush send has no send tokens\n");
#endif
      }
      data = data->prev;
    }
  }
}


/**
 * Send user data via copy mechanism for a small message
 */
static QMP_status_t
qmp_send_user_data_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_buffer_mem_t* data;

  if ((data = mh->memory->null_tail)) {
    /**
     * There are spaces available to copy user data to.
     */
#ifdef _QMP_SEND_DEBUG
    QMP_fprintf (stderr, "Copy sending user data to shadow buffer\n");
#endif
    memcpy (data->mem, mh->memory->mem,
	    mh->memory->nbytes);

    /**
     * Dequeue the null list
     */
    DEQUEUE_NULL_LIST(mh->memory);

    /**
     * Add data to data list.
     */
    ENQUEUE_DATA_LIST(mh->memory,data);


    /**
     * Send data out if possible.
     */

    /**
     * Check whether I have a send token or not.
     */
    data->done = 0;
    if (QMP_HAS_SEND_TOKENS(glm,QMP_MSG_PRIORITY)) {
#ifdef _QMP_SEND_DEBUG
      QMP_fprintf (stderr, "Send user data via gm_send_callback\n");
#endif
      gm_send_with_callback (mh->ch.my_port, data->mem,
			     mh->ch.gm_msg_size, mh->memory->nbytes,
			     QMP_MSG_PRIORITY,
			     mh->ch.rem_node_id,
			     mh->ch.rem_port_num,
			     qmp_userdata_sent_i, (void *)data);
      /**
       * Give up a send token to gm
       */
      QMP_GIVEUP_SEND_TOKEN(glm, QMP_MSG_PRIORITY);

      /**
       * Data sent flag set to 1
       */
      data->done = 1;
    }
#ifdef _QMP_SEND_DEBUG
    else
      QMP_fprintf (stderr, "No send tokens are available, wait\n");
#endif

    return QMP_SUCCESS;
  }
  else {
#ifdef _QMP_SEND_DEBUG
    QMP_fprintf (stderr, "No more data buffer available, waiting\n");
#endif
    mh->state = QMP_MH_WAITING;
    return QMP_SUCCESS;
  }
}  

/**
 * Start sending operation for a message handle.
 */
static QMP_status_t
qmp_start_send_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_TRACE ("qmp_start_send_i");


  /**
   * If memory size is small, we do quick memory copy to
   * shadow buffers to improve latency.
   */
  if (mh->memory->do_mem_copy) 
    return qmp_send_user_data_i (glm, mh);
  else {
    /**
     * Check whether I have a send token or not.
     */
    if (QMP_HAS_SEND_TOKENS(glm,QMP_MSG_PRIORITY)) {
      mh->state = QMP_MH_WAITING;
      gm_send_with_callback (mh->ch.my_port, mh->memory->mem,
			     mh->ch.gm_msg_size, mh->memory->nbytes,
			     QMP_MSG_PRIORITY,
			     mh->ch.rem_node_id,
			     mh->ch.rem_port_num,
			     qmp_nbcomm_sent_i, (void *)mh);
      /**
       * Give up a send token to gm
       */
      QMP_GIVEUP_SEND_TOKEN(glm, QMP_MSG_PRIORITY);
      return QMP_SUCCESS;
    }
    else
      return QMP_SVC_BUSY;
  }
}

/**
 * Start sending a message to a remote queue.
 */
static QMP_status_t
qmp_start_queue_send_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_TRACE ("qmp_start_queue_send_i");

  if (mh->state != QMP_MH_IDLE)
    return QMP_INVALID_OP;

  if (!QMP_HAS_SEND_TOKENS (glm, QMP_MSG_PRIORITY))
    return QMP_SVC_BUSY;

  /**
   * Yet to implement.
   */
  mh->state = QMP_MH_WAITING;

  return QMP_NOTSUPPORTED;
}  

/**
 * Wait for a message handle to finish a communication along polling other
 * message handles. Started by QMP_start
 *
 * return QMP_SUCCESS if the communication is done.
 * return QMP_TIMEOUT if time out occurs before the communication is done.
 */
static QMP_status_t
qmp_wait_for_i (QMP_machine_t* glm,
		QMP_msghandle_i_t* msgh,
		QMP_u32_t timeout)
{
  QMP_u32_t       start, curr;
  int             i;

  QMP_TRACE("qmp_wait_for_i");
  
  if (timeout > 1) {
    start = curr = get_current_time ();
    do {
      i = 0; 
      /**
       * First process event loop for a second before calculating
       * time difference.
       */
      while (msgh->state == QMP_MH_WAITING && i < QMP_NUMLOOPS_PSEC) {
	qmp_main_loop_i (glm, 0);
	i++; 
      }
      if (msgh->state == QMP_MH_WAITING)
	curr = get_current_time ();
    }while (msgh->state == QMP_MH_WAITING && curr - start <= timeout);
    
    if (msgh->state == QMP_MH_IDLE)
      return QMP_SUCCESS;
    return QMP_TIMEOUT;
  }
  else {
    qmp_main_loop_i (glm, 0);
    return QMP_SUCCESS;
  }
}
      

/**
 * Main gm receiving loop
 *
 * Arguments:
 *    glm:     global machine instance.
 *    h:       message handle (could be 0 if we do not care)
 */
static void
qmp_main_loop_i (QMP_machine_t* glm,
		 QMP_msghandle_i_t* h)
{
  QMP_msghandle_i_t* mh;
  gm_recv_event_t *e;

  /**
   * process the port for general message and sending ports
   */
  e = gm_receive (glm->port);
  QMP_process_message (glm, e, h);

  /**
   * This loop goes over every channel.
   */
  mh = glm->msg_handles;
  while (mh) {
    if (mh->ch.my_port_num != 2) {
      e = gm_receive (mh->ch.my_port);
      QMP_process_ch_message (e, mh);
    }
    mh = mh->next;
  }
}

#else /* _QMP_USE_MULTI_PORTS */

static void
qmp_gen_msg_sent (struct gm_port* port, void* arg, gm_status_t status);

static QMP_status_t 
qmp_start_send_genmsg_i (void* buffer, 
			 QMP_u32_t count, 
			 QMP_datatype_t data_type, 
			 QMP_u32_t dest_id,  
			 QMP_u32_t operation, 
			 QMP_u32_t priority,
			 QMP_machine_t* glm, 
			 QMP_msg_queue_slot_t** queueslot);


static QMP_status_t
qmp_wait_for_genmsg_sent_i (QMP_msg_queue_slot_t* qs,
			    QMP_u32_t timeout);

static QMP_status_t
qmp_start_recv_genmsg_i (void* buffer, QMP_u32_t count, 
			 QMP_datatype_t datatype, QMP_u32_t src_id,  
			 QMP_u32_t operation, QMP_u32_t priority,
			 QMP_machine_t* glm, 
			 QMP_msg_queue_slot_t** queueslot);


static QMP_status_t
qmp_wait_for_genmsg_recv_i (void* user_buffer,
			    QMP_msg_queue_slot_t* qs,
			    QMP_u32_t timeout);

static void
qmp_mh_msg_sent (struct gm_port* port, void* arg, gm_status_t status);

/**
 * Flush out pending send
 */
static void
qmp_flush_pending_send_i (QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t* p;
  QMP_rtenv_t rem_machine;

  QMP_TRACE ("qmp_flush_pending_send_i");

  p = glm->pending_send_queue;
  while (p) {
    if (!(p->state & QMP_MH_WAITING)) {
      /**
       * This queue slot has not been sent out yet
       */
      /**
       * Send out the message inside queue slot.
       */
      rem_machine = QMP_get_machine_info_by_id (glm, p->hdr.dest);
      if (!rem_machine) {
	QMP_error_exit ("qmp_flush_pending_send_i: run time information error,\n");     
	return;
      }

      if (QMP_HAS_SEND_TOKENS(glm,QMP_MSG_PRIORITY)) {
	gm_send_with_callback (glm->port, (void *)p->buffer,
			       QMP_GENMSG_SIZE,
			       p->buflen,
			       QMP_MSG_PRIORITY,
			       rem_machine->node_id, rem_machine->port,
			       qmp_mh_msg_sent, (void *)p);
	/**
	 * Give up a send token to gm
	 */
	QMP_GIVEUP_SEND_TOKEN(glm, QMP_MSG_PRIORITY);
	
	if (p->mh) {
	  /**
	   * Now the associated message handle should have idle state.
	   */
	  p->mh->state = QMP_MH_IDLE;
	}
	
	p->state = p->state | QMP_MH_WAITING;
      }
    }
    
    p = p->next;
  }
}  

/**
 * GM general message handle send message callback function.
 */
static void
qmp_mh_msg_sent (struct gm_port* port, void* arg, gm_status_t status)
{
  QMP_msg_queue_slot_t* qs;
  QMP_machine_t*        glm;
  
  QMP_TRACE ("qmp_gen_msg_sent");

  qs = (QMP_msg_queue_slot_t *)arg;
  glm = qs->glm;

  if (status == GM_SUCCESS) {
    if (qs->state & QMP_MH_QUEUE)
      /**
       * This is a flushed out pending send, remove it from the pending list.
       */
      QMP_remove_pending_send_buffer (glm, qs);

    qs->state = QMP_MH_IDLE;
  }

  /**
   * GM give up a send token occupied by this send. I acquire this send token
   */
  QMP_ACQUIRE_SEND_TOKEN(glm, QMP_MSG_PRIORITY);

  /**
   * Remove this request.
   */
  QMP_delete_msg_queue_slot (qs);

  /**
   * Flush out pending send queue
   */
  qmp_flush_pending_send_i (glm); 
}

/**
 * Start send a general tagged message from a message handle
 *
 * Arguments:
 *      buffer:      user buffer.
 *      count:       number of elements of data type 'type'
 *      type:        one of the QMP_datatype_t
 *      dest_id:     logic id of destination.
 *      operation:   what this message is about.
 *      priority:    priority of this message
 *      glm:         global machine pointer.
 *      queueslot:   pointer to a newly allocated queue slot
 */
static QMP_status_t
qmp_start_send_mh_message_i (QMP_msghandle_i_t* mh)
{
  QMP_msg_queue_slot_t* ret;
  QMP_rtenv_t           rem_machine;
  QMP_u8_t              adop;

  QMP_TRACE ("qmp_start_send_mh_message_i");

  if (mh->cnt)
    adop = QMP_CH_DONE;
  else
    adop = QMP_CH_CONNECT;

  ret = QMP_create_msg_queue_slot (mh->memory->mem, 
				   mh->memory->nbytes, QMP_BYTE,
				   mh->dest, mh->tag, QMP_MSG_PRIORITY, adop,
				   QMP_TRUE, QMP_FALSE,
				   mh->glm);

  if (!ret)
    return QMP_NOMEM_ERR;

  /**
   * Send out the message inside queue slot.
   */
  rem_machine = QMP_get_machine_info_by_id (mh->glm, mh->dest);
  if (!rem_machine) 
    return QMP_RTENV_ERR;
  
  /**
   * Let this queue slot remember this message handle and vice versa
   */
  ret->mh = mh;
  mh->req = ret;

  if (QMP_HAS_SEND_TOKENS(mh->glm,QMP_MSG_PRIORITY)) {
    ret->state = QMP_MH_WAITING;

    gm_send_with_callback (mh->glm->port, (void *)ret->buffer,
			   QMP_GENMSG_SIZE,
			   ret->buflen,
			   QMP_MSG_PRIORITY,
			   rem_machine->node_id, rem_machine->port,
			   qmp_mh_msg_sent, (void *)ret);
    /**
     * Give up a send token to gm
     */
    QMP_GIVEUP_SEND_TOKEN(mh->glm, QMP_MSG_PRIORITY);

    mh->state = QMP_MH_IDLE;
  }
  else {
    mh->state = QMP_MH_WAITING;
    ret->state = QMP_MH_QUEUE;

    QMP_add_pending_send_buffer (mh->glm, ret);
  }

  return QMP_SUCCESS;
}

/**
 * Wait for a message handle send to finish.
 */
static QMP_status_t
qmp_wait_for_mhmsg_sent_i (QMP_msghandle_i_t* mh,
			   QMP_u32_t timeout)
{
  QMP_u32_t       start, curr;
  int             i;


  QMP_TRACE("qmp_wait_for_mhmsg_sent_i");

  start = curr = get_current_time ();
  do {
    i = 0; 
    /**
     * First process event loop for a second before calculating
     * time difference.
     */
    while (mh->state == QMP_MH_WAITING && i < QMP_NUMLOOPS_PSEC) {
      /* go through other loops */
      qmp_main_loop_i (mh->glm, 0);
      i++;
    }

    if (mh->state == QMP_MH_WAITING)
      curr = get_current_time ();
  }while (mh->state == QMP_MH_WAITING && curr - start <= timeout);

  if (mh->state == QMP_MH_IDLE)
    return QMP_SUCCESS;

  return QMP_TIMEOUT;
}

/**
 * Process general message from port 2
 */
static void
QMP_process_message (QMP_machine_t* glm,
		     gm_recv_event_t* e,
		     QMP_msghandle_i_t* mh)
{
  /**
   * process the port for general message and sending ports
   */
  switch (gm_ntoh_u8 (e->recv.type)) {
  case GM_HIGH_RECV_EVENT:
    {
      /**
       * General message for global sum and send/receive
       */
      char*                     buffer;
      QMP_u32_t                 size, length;
      QMP_general_msg_header_t* hdr;

      buffer = (char *)gm_ntohp (e->recv.buffer);
      size = gm_ntoh_u8 (e->recv.size);
      length = gm_ntoh_u32 (e->recv.length);
      
      hdr = (QMP_general_msg_header_t *)buffer;
      qmp_process_genmsg_i (glm, hdr, 
			    &buffer[sizeof(QMP_general_msg_header_t)], 
			    length - sizeof(QMP_general_msg_header_t),
			    GM_HIGH_PRIORITY);
      /**
       * resupply this buffer back to gm for further receiving.
       */
      gm_provide_receive_buffer (glm->port, buffer, size, 
				 QMP_MSG_PRIORITY);
    }
    break;
  default:
    gm_unknown (glm->port, e);
    break;
  }
}
  

/**
 * Main gm receiving loop
 *
 * Arguments:
 *    glm:     global machine instance.
 *    h:       message handle (could be 0 if we do not care)
 */
static void
qmp_main_loop_i (QMP_machine_t* glm,
		 QMP_msghandle_i_t* h)
{
  gm_recv_event_t *e;

  /**
   * process the port for general message
   */
  e = gm_receive (glm->port);
  QMP_process_message (glm, e, h);
}

/**
 * Wait for a message handle to finish a communication along polling other
 * message handles. Started by QMP_start
 *
 * return QMP_SUCCESS if the communication is done.
 * return QMP_TIMEOUT if time out occurs before the communication is done.
 */
static QMP_status_t
qmp_wait_for_i (QMP_machine_t* glm,
		QMP_msghandle_i_t* msgh,
		QMP_u32_t timeout)
{
  QMP_status_t status;
  QMP_TRACE("qmp_wait_for_i");
  
  status = QMP_SUCCESS;
  if (msgh->type & QMP_MH_RECV) {
    status = qmp_wait_for_genmsg_recv_i (msgh->memory->mem, 
					 msgh->req, timeout);
    if (status == QMP_SUCCESS) 
      msgh->cnt++;
    else
      QMP_error ("Receiving timeout ........\n");
    
    msgh->err_code = status;
    msgh->state = msgh->req->state;

    msgh->req->mh = 0;
    QMP_delete_msg_queue_slot (msgh->req);
    msgh->req = 0;
  }    
  else if (msgh->type & QMP_MH_SEND) {
    status = qmp_wait_for_mhmsg_sent_i (msgh, timeout);
    if (status == QMP_SUCCESS) 
      msgh->cnt++;
    else
      QMP_error ("Sending timeout ..........\n");
    
    msgh->err_code = status;
    msgh->req = 0;
  }
  else {
    QMP_error_exit ("qmp_wait_for_i: message handle has wrong type.\n");
    return QMP_ERROR;
  }
  return status;
}


/**
 * Start sending operation for a message handle.
 */
static QMP_status_t
qmp_start_send_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_status_t status;

  QMP_TRACE ("qmp_start_send_i");

  status = qmp_start_send_mh_message_i (mh);

  return status;
}

/**
 * Start receiving operation for a message handle.
 */
static QMP_status_t
qmp_start_recv_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_status_t status;
  QMP_msg_queue_slot_t* req;

  QMP_TRACE ("qmp_start_recv_i");

  status = qmp_start_recv_genmsg_i (mh->memory->mem,
				    mh->memory->nbytes,
				    QMP_BYTE,
				    mh->source,
				    mh->tag, QMP_MSG_PRIORITY,
				    mh->glm,
				    &req);

  if (status == QMP_SUCCESS) {
    mh->req = req;
    req->mh = mh;
    mh->state = mh->req->state;
    if (mh->cnt == 0 && mh->state == QMP_MH_IDLE) {
      /**
       * This is the first message received, and set my tag
       * to the tag a sender send to me.
       */
      mh->tag = req->hdr.op;
    }
  }
  return status;
}

#endif /* _QMP_USE_MULTI_PORTS */

/**
 * Real communication code for a message handle.
 */
static QMP_status_t
qmp_start_comm_i (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_TRACE ("qmp_start_comm_i");

  if ((mh->type & QMP_MH_RECV) && mh->state == QMP_MH_IDLE) {
    return qmp_start_recv_i (glm, mh);
  }
  else if ((mh->type & QMP_MH_SEND) && mh->state == QMP_MH_IDLE) {
    return qmp_start_send_i (glm, mh);
  }
  return QMP_INVALID_OP;
}


/**
 * Start real communication.
 * We are not checking whether channels are established since users
 * are required to check message handle before calling this routine.
 *
 * Send/Recv depends on the message handle type.
 * 
 * If the message handle is a collection of multiple message handles,
 * a loop will be used to go over each handle.
 */
QMP_status_t
QMP_start (QMP_msghandle_t h)
{
  QMP_msghandle_i_t* mh = (QMP_msghandle_i_t *)h;

  if (mh->type < QMP_MH_MULTIPLE) 
    /**
     * Single instance of message handle.
     */
    return qmp_start_comm_i (mh->glm, mh);
  else {
    QMP_msghandle_i_t* p;

    p = mh;
    while (p) {
      qmp_start_comm_i (p->glm, p);
      p = p->m_next;
    }
    return QMP_SUCCESS;
  }
}

/**
 * Internal routine testing whether an operation is complete.
 */
static QMP_bool_t
qmp_is_complete_i (QMP_machine_t* glm,
		   QMP_msghandle_i_t* mh)
{
  QMP_TRACE ("qmp_is_complete_i");

  if (mh->state == QMP_MH_IDLE)
    return QMP_TRUE;
  
  /* do a quick poll and check again */
  qmp_wait_for_i (mh->glm, mh, 0);
  
  return (mh->state == QMP_MH_IDLE);
}


/**
 * Test whether a communication started by QMP_start with a message handle
 * has been completed.
 */
QMP_bool_t
QMP_is_complete (QMP_msghandle_t h)
{
  QMP_msghandle_i_t* mh = (QMP_msghandle_i_t *)h; 

  if (mh->type < QMP_MH_MULTIPLE)
    /**
     * Single instance of message handle.
     */
    return qmp_is_complete_i (mh->glm, mh);
  else {
    QMP_msghandle_i_t* p;
    QMP_bool_t         done = QMP_TRUE;

    p = mh;
    while (p) {
      if (qmp_is_complete_i (p->glm, mh) != QMP_TRUE) 
	done = QMP_FALSE;
      p = p->m_next;
    }
    return done;
  }
}

/**
 * Wait for an operation to complete for a particular message handle.
 */
QMP_status_t
QMP_wait (QMP_msghandle_t h)
{
  QMP_bool_t         done;
  QMP_status_t       ist;
  QMP_msghandle_i_t* mh = (QMP_msghandle_i_t *)h;

  if (mh->type < QMP_MH_MULTIPLE)
    /**
     * Single instance of message handle.
     */
    return qmp_wait_for_i (mh->glm, mh, QMP_MSG_WAIT_TIMEOUT);
  else {
    QMP_msghandle_i_t* p;

    /**
     * Go over each message handle for maximum ~2 second.
     * If all of message handles are done, the loop ends.
     */
    done = QMP_FALSE;
    while (!done) {
      done = QMP_TRUE;
      p = mh;
      while (p) {
	ist = qmp_wait_for_i (p->glm, p, QMP_SINGLE_WAIT_TIMEOUT);
	if (ist != QMP_SUCCESS && ist != QMP_TIMEOUT)
	  return ist;
	else if (ist == QMP_TIMEOUT)
	  done = QMP_FALSE;

	p = p->m_next;
      }
    }
    return QMP_SUCCESS;
  }
}



/**************************************************************************
 *   From hereon are the implementations for general message passing      *
 *************************************************************************/
/**
 * Global table of data sizes for different QMP data types.
 */
QMP_u32_t QMP_data_size_table[] = {1, 1, 1, 2, 2, 4, 4, 8, 8, 4, 8};

static void qmp_sum_i (void* result, void* source,
		       QMP_u32_t count, QMP_datatype_t datatype)
{
  if (count == 1) {
    switch (datatype) {
    case QMP_INT:
      *(int *)result = *(int *)result + *(int *)source;
      break;
    case QMP_FLOAT:
      *(float *)result = *(float *)result + *(float *)source;
      break;
    case QMP_DOUBLE:
      *(double *)result = *(double *)result + *(double *)source;
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
  else {
    int i;
    
    switch (datatype) {
    case QMP_INT:
      {
	int* retvalue = (int *)result;
	int* src = (int *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = retvalue[i] + src[i];
      }
      break;
    case QMP_FLOAT:
      {
	float* retvalue = (float *)result;
	float* src = (float *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = retvalue[i] + src[i];
      }
      break;
    case QMP_DOUBLE:
      {
	double* retvalue = (double *)result;
	double* src = (double *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = retvalue[i] + src[i];
      }
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
}


static void qmp_max_i (void* result, void* source,
		       QMP_u32_t count, QMP_datatype_t datatype)
{
  if (count == 1) {
    switch (datatype) {
    case QMP_INT:
      *(int *)result = ((*(int *)result) > (*(int *)source)) ? (*(int *)result): (*(int *)source);
      break;
    case QMP_FLOAT:
      *(float *)result = ((*(float *)result) > (*(float *)source)) ? (*(float *)result): (*(float *)source);
      break;
    case QMP_DOUBLE:
      *(double *)result = ((*(double *)result) > (*(double *)source)) ? (*(double *)result): (*(double *)source);
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
  else {
    int i;
    
    switch (datatype) {
    case QMP_INT:
      {
	int* retvalue = (int *)result;
	int* src = (int *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] > src[i]) ? retvalue[i] : src[i];
      }
      break;
    case QMP_FLOAT:
      {
	float* retvalue = (float *)result;
	float* src = (float *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] > src[i]) ? retvalue[i] : src[i];
      }
      break;
    case QMP_DOUBLE:
      {
	double* retvalue = (double *)result;
	double* src = (double *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] > src[i]) ? retvalue[i] : src[i];
      }
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
}

static void qmp_min_i (void* result, void* source,
		       QMP_u32_t count, QMP_datatype_t datatype)
{
  if (count == 1) {
    switch (datatype) {
    case QMP_INT:
      *(int *)result = ((*(int *)result) < (*(int *)source)) ? (*(int *)result): (*(int *)source);
      break;
    case QMP_FLOAT:
      *(float *)result = ((*(float *)result) < (*(float *)source)) ? (*(float *)result): (*(float *)source);
      break;
    case QMP_DOUBLE:
      *(double *)result = ((*(double *)result) < (*(double *)source)) ? (*(double *)result): (*(double *)source);
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
  else {
    int i;
    
    switch (datatype) {
    case QMP_INT:
      {
	int* retvalue = (int *)result;
	int* src = (int *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] < src[i]) ? retvalue[i] : src[i];
      }
      break;
    case QMP_FLOAT:
      {
	float* retvalue = (float *)result;
	float* src = (float *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] < src[i]) ? retvalue[i] : src[i];
      }
      break;
    case QMP_DOUBLE:
      {
	double* retvalue = (double *)result;
	double* src = (double *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] = (retvalue[i] < src[i]) ? retvalue[i] : src[i];
      }
      break;
    default:
      QMP_error ("qmp sum only supports integer, float and double\n");
      exit (1);
      break;
    }
  }
}

static void qmp_xor_i (void* result, void* source,
		       QMP_u32_t count, QMP_datatype_t datatype)
{
  if (count == 1) {
    switch (datatype) {
    case QMP_INT:
    case QMP_64BIT_INT:
      (*(long *)result) =  (*(long *)result) ^ (*(long *)source);
      break;
    default:
      QMP_error ("qmp or only supports integer\n");
      exit (1);
      break;
    }
  }
  else {
    int i;
    
    switch (datatype) {
    case QMP_INT:
    case QMP_64BIT_INT:
      {
	long* retvalue = (long *)result;
	long* src = (long *)source;
	for (i = 0; i < count; i++) 
	  retvalue[i] =  retvalue[i] ^ src[i];
      }
      break;
    default:
      QMP_error ("qmp or only supports long integer\n");
      exit (1);
      break;
    }
  }
}


/**
 * Global arithmatic operator table.
 */
QMP_rop_t QMP_operator_table[]={
  {QMP_SUM, QMP_TRUE,  qmp_sum_i},
  {QMP_MAX, QMP_TRUE,  qmp_max_i},
  {QMP_MIN, QMP_TRUE,  qmp_min_i},
  {QMP_XOR, QMP_TRUE,  qmp_xor_i}
};

/**
 * GM general send message callback function.
 */
static void
qmp_gen_msg_sent (struct gm_port* port, void* arg, gm_status_t status)
{
  QMP_msg_queue_slot_t* qs;
  
  QMP_TRACE ("qmp_gen_msg_sent");

  qs = (QMP_msg_queue_slot_t *)arg;

  if (status == GM_SUCCESS) 
    qs->state = QMP_MH_IDLE;


  /**
   * GM give up a send token occupied by this send. I acquire this send token
   */
  QMP_ACQUIRE_SEND_TOKEN(qs->glm, QMP_MSG_PRIORITY);
}


/**
 * Start send a general message.
 *
 * Arguments:
 *      buffer:      user buffer.
 *      count:       number of elements of data type 'type'
 *      type:        one of the QMP_datatype_t
 *      dest_id:     logic id of destination.
 *      operation:   what this message is about.
 *      priority:    priority of this message
 *      glm:         global machine pointer.
 *      queueslot:   pointer to a newly allocated queue slot
 */
static QMP_status_t
qmp_start_send_genmsg_i (void* buffer, QMP_u32_t count, 
			 QMP_datatype_t data_type, QMP_u32_t dest_id,  
			 QMP_u32_t operation, QMP_u32_t priority,
			 QMP_machine_t* glm, 
			 QMP_msg_queue_slot_t** queueslot)
{
  QMP_msg_queue_slot_t* ret;
  QMP_rtenv_t           rem_machine;

  QMP_TRACE ("qmp_start_send_genmsg_i");

  ret = QMP_create_msg_queue_slot (buffer, count, data_type,
				   dest_id, operation, priority, QMP_CH_DONE,
				   QMP_TRUE, QMP_FALSE, 
				   glm);

  if (!ret)
    return QMP_NOMEM_ERR;

  /**
   * Send out the message inside queue slot.
   */
  rem_machine = QMP_get_machine_info_by_id (glm, dest_id);
  if (!rem_machine) 
    return QMP_RTENV_ERR;
  
  *queueslot = ret;
  if (QMP_HAS_SEND_TOKENS(glm,QMP_MSG_PRIORITY)) {
    ret->state = QMP_MH_WAITING;

    gm_send_with_callback (glm->port, (void *)ret->buffer,
			   QMP_GENMSG_SIZE,
			   ret->buflen,
			   QMP_MSG_PRIORITY,
			   rem_machine->node_id, rem_machine->port,
			   qmp_gen_msg_sent, (void *)ret);
    /**
     * Give up a send token to gm
     */
    QMP_GIVEUP_SEND_TOKEN(glm, QMP_MSG_PRIORITY);

    return QMP_SUCCESS;
  }

  return QMP_SVC_BUSY;
}


/**
 * Wait for a send to finish.
 */
static QMP_status_t
qmp_wait_for_genmsg_sent_i (QMP_msg_queue_slot_t* qs,
			    QMP_u32_t timeout)
{
  QMP_u32_t       start, curr;
  int             i;


  QMP_TRACE("qmp_wait_for_genmsg_sent_i");

  start = curr = get_current_time ();
  do {
    i = 0; 
    /**
     * First process event loop for a second before calculating
     * time difference.
     */
    while (qs->state == QMP_MH_WAITING && i < QMP_NUMLOOPS_PSEC) {
      /* go through other loops */
      qmp_main_loop_i (qs->glm, 0);
      i++;
    }

    if (qs->state == QMP_MH_WAITING)
      curr = get_current_time ();
  }while (qs->state == QMP_MH_WAITING && curr - start <= timeout);

  if (qs->state == QMP_MH_IDLE)
    return QMP_SUCCESS;

  return QMP_TIMEOUT;
}


/**
 * General send function similar to MPI_send.
 */
QMP_status_t
QMP_send_message (void* buffer, QMP_u32_t count, QMP_datatype_t datatype,
		  QMP_u32_t dest_id, QMP_u32_t operation,
		  QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t* qs;
  QMP_status_t          status;

  QMP_TRACE ("QMP_send_message");

  if (count * QMP_DATA_SIZE(datatype) > glm->max_msg_length) {
    QMP_error ("trying to send a message that is too long.");
    return QMP_MEMSIZE_TOOBIG;
  }

  qs = 0;
  status = qmp_start_send_genmsg_i (buffer, count, datatype,
				    dest_id, operation, 
				    QMP_MSG_PRIORITY, glm,
				    &qs);

  if (status == QMP_SUCCESS) {
    status = qmp_wait_for_genmsg_sent_i (qs, QMP_MSG_WAIT_TIMEOUT);
    QMP_delete_msg_queue_slot (qs);
  }
  return status;
}  

/**
 * Start receiving a general message.
 */
static QMP_status_t
qmp_start_recv_genmsg_i (void* buffer, QMP_u32_t count, 
			 QMP_datatype_t datatype, QMP_u32_t src_id,  
			 QMP_u32_t operation, QMP_u32_t priority,
			 QMP_machine_t* glm, 
			 QMP_msg_queue_slot_t** queueslot)
{
  QMP_msg_queue_slot_t* qs;

  QMP_TRACE ("QMP_start_recv_message_i");

  /**
   * Check whether there is a matching received message
   * in the received queue.
   */
  qs = QMP_match_received_msg (buffer, count, datatype,
			       src_id, operation,
			       QMP_MSG_PRIORITY, glm);

  if (!qs) {
    /**
     * No matching found from the received list. we have to put
     * this message on to the receiving buffer list.
     */
    qs = QMP_create_msg_queue_slot (buffer, count, datatype,
				    src_id, operation,
				    QMP_MSG_PRIORITY, QMP_CH_DONE,
				    QMP_FALSE, QMP_FALSE, glm);

    if (qs) {
      QMP_add_provided_receiving_buffer (glm, qs);
      qs->state = QMP_MH_WAITING;
    }
    else
      return QMP_NOMEM_ERR;
  }
  *queueslot = qs;

  return QMP_SUCCESS;
}


/**
 * Wait for a receive to finish.
 */
static QMP_status_t
qmp_wait_for_genmsg_recv_i (void* user_buffer,
			    QMP_msg_queue_slot_t* qs,
			    QMP_u32_t timeout)
{
  QMP_u32_t       start, curr;
  int             i;

  QMP_TRACE("qmp_wait_for_genmsg_recv_i");

  if (qs->type & QMP_MH_QUEUE) {
    /**
     * This is the slot from the received list. just copy data from
     * the queued buffer to user buffer.
     */
    memcpy (user_buffer, qs->buffer, qs->buflen);
    return QMP_SUCCESS;
  }

  start = curr = get_current_time ();
  do {
    i = 0; 
    /**
     * First process event loop for a second before calculating
     * time difference.
     */
    while (qs->state == QMP_MH_WAITING && i < QMP_NUMLOOPS_PSEC) {
      /* go through other loops */
      qmp_main_loop_i (qs->glm, 0);
      i++;
    }

    if (qs->state == QMP_MH_WAITING)
      curr = get_current_time ();
  }while (qs->state == QMP_MH_WAITING && curr - start <= timeout);
  
  /**
   *  remove this qs from provided buffer list.
   */
  QMP_remove_provided_receiving_buffer (qs->glm, qs);

  if (qs->state == QMP_MH_IDLE) 
    return QMP_SUCCESS;


  return QMP_TIMEOUT;
}


/**
 * General blocked receive function similar to MPI_recv.
 */
QMP_status_t
QMP_recv_message (void* buffer, QMP_u32_t count, QMP_datatype_t datatype,
		  QMP_u32_t src_id, QMP_u32_t operation,
		  QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t* qs;
  QMP_status_t          status;

  QMP_TRACE ("QMP_recv_message");

  if (count * QMP_DATA_SIZE(datatype) > glm->max_msg_length) {
    QMP_error ("trying to receive a message that is too long.");
    return QMP_MEMSIZE_TOOBIG;
  }
  
  qs = 0;
  status = qmp_start_recv_genmsg_i (buffer, count, datatype, src_id,
				    operation, QMP_MSG_PRIORITY,
				    glm, &qs);

  if (status == QMP_SUCCESS) {
    status = qmp_wait_for_genmsg_recv_i (buffer, qs, QMP_MSG_WAIT_TIMEOUT);
    if (status == QMP_SUCCESS)
      QMP_delete_msg_queue_slot (qs);
  }
  return status;
}  


/**
 * Public function for receiving a message.
 */
QMP_status_t
QMP_recv (void* buffer, QMP_u32_t count, QMP_datatype_t datatype,
	  QMP_u32_t src_id, QMP_u32_t operation)
{
  return QMP_recv_message (buffer, count, datatype, src_id, operation,
			   &QMP_global_m);
}

/**
 * Public function for sending a message.
 */
QMP_status_t
QMP_send (void* buffer, QMP_u32_t count, QMP_datatype_t datatype,
	  QMP_u32_t dest_id, QMP_u32_t operation)
{
  return QMP_send_message (buffer, count, datatype, dest_id, operation,
			   &QMP_global_m);
}

/**
 * Processing incoming general message
 * Arguments:
 *      glm:     global machine pointer
 *      hdr:     protocol header.
 *      buffer:  payload
 *      length:  length of payload
 *      pri:     priority of this message.
 */
void
qmp_process_genmsg_i (QMP_machine_t* glm,
		      QMP_general_msg_header_t* hdr,
		      void *buffer, QMP_u32_t length,
		      QMP_u32_t pri)
{
  QMP_TRACE ("qmp_process_genmsg_i");

  if (hdr->magic == QMP_MAGIC && hdr->vers == QMP_VERSION_CODE) {
    /**
     * first check whether there is a provided buffer waiting
     * for this message.
     */
    if (QMP_match_provided_buffer (hdr, buffer, length, pri, glm)) 
      ;
    else {
      /**
       * Put this message on the received list.
       */
      QMP_add_received_msg (hdr, buffer, length, pri, glm);
    }
  }
}

/**************************************************************************
 *   Global communication part of code.                                   *
 *       Parts of code are taken from MPICH distribution.                 *
 **************************************************************************/

/**
 * Global reduce.
 *    sendbuf contains value of this process,
 *    recvbuf will be final result of the reduction for the root.
 */
static QMP_status_t
qmp_reduce_i (QMP_machine_t* glm,
	      void* sendbuf, void* recvbuf,
	      QMP_u32_t count, QMP_datatype_t datatype,
	      QMP_rop_t* op_ptr, QMP_u32_t root)
{
  QMP_u32_t    rank, size, buflen;
  QMP_u32_t    mask, relrank, source, lroot;
  QMP_opfunc   uop;
  QMP_status_t status;
  void*        buffer;

  QMP_TRACE ("qmp_reduce_i");

  /**
   * Check whether the root is valid.
   */
  size = QMP_get_logical_number_of_nodes ();
  if (root >= size) {
    QMP_error ("qmp_reduce has an invalid root.");
    return QMP_INVALID_ARG;
  }

  if (!op_ptr->commute) {
    QMP_error ("non commutitive operations should be managed by applications.");
    return QMP_NOTSUPPORTED;
  }

  if (count == 0)
    return QMP_SUCCESS;

  /**
   * Get my logic id
   */
  rank =  QMP_get_logical_node_number ();

  /**
   * Get real function pointer.
   */
  uop = op_ptr->func;

  /* Here's the algorithm.  Relative to the root, look at the bit pattern in 
     my rank.  Starting from the right (lsb), if the bit is 1, send to 
     the node with that bit zero and exit; if the bit is 0, receive from the
     node with that bit set and combine (as long as that node is within the
     group)

     Note that by receiving with source selection, we guarentee that we get
     the same bits with the same input.  If we allowed the parent to receive 
     the children in any order, then timing differences could cause different
     results (roundoff error, over/underflows in some cases, etc).

     Because of the way these are ordered, if root is 0, then this is correct
     for both commutative and non-commutitive operations.  If root is not
     0, then for non-commutitive, we use a root of zero and then send
     the result to the root.  To see this, note that the ordering is
     mask = 1: (ab)(cd)(ef)(gh)            (odds send to evens)
     mask = 2: ((ab)(cd))((ef)(gh))        (3,6 send to 0,4)
     mask = 4: (((ab)(cd))((ef)(gh)))      (4 sends to 0)

     Comments on buffering.  
     If the datatype is not contiguous, we still need to pass contiguous 
     data to the user routine.  
     In this case, we should make a copy of the data in some format, 
     and send/operate on that.
   */
  /* create temp buffer */
  buflen = count*QMP_DATA_SIZE(datatype);
  buffer = QMP_memalign (buflen, QMP_MEM_ALIGNMENT);
  
  /* If I'm not the root, then my recvbuf may not be valid, therefore
     I have to allocate a temporary one */
  if (rank != root) 
    recvbuf = QMP_memalign (buflen, QMP_MEM_ALIGNMENT);
  
  /* This code isn't correct if the source is a more complex datatype */
  memcpy( recvbuf, sendbuf, buflen);
  mask    = 0x1;
  /* we only support commutitive operations */
  lroot   = root;
  relrank = (rank - lroot + size) % size;
  
  status = QMP_SUCCESS;

  while (mask < size) {
    /* receive */
    if ((mask & relrank) == 0) {
      source = (relrank | mask);
      if (source < size) {
	source = (source + lroot) % size;
	status = QMP_recv_message (buffer, count, datatype, source,
				   QMP_REDUCE, glm);
	if (status != QMP_SUCCESS) {
	  if (status == QMP_TIMEOUT)
	    QMP_error ("QMP_reduce receiving from node %d timed out\n", 
		       source);
	  return status;
	}
	/**
	 * Function looks like (result, src).
	 */
	(*uop)(recvbuf, buffer, count, datatype);
      }
    }
    else {
      /* I've received all that I'm going to.  Send my result to 
	 my parent */
      /* sender */
      source = ((relrank & (~ mask)) + lroot) % size;
      status = QMP_send_message (recvbuf, count, datatype, source,
				 QMP_REDUCE, glm);
      if (status != QMP_SUCCESS) {
	QMP_error ("QMP_reduce sending to node %d timed out\n", source);
	return status;
      }
      break;
    }
    mask <<= 1;
  }

  /* free buffer memory */
  free (buffer);

  /* Free the temporarily allocated recvbuf */
  if (rank != root)
    free (recvbuf);

  return QMP_SUCCESS;
}


/**
 * Do a simple broadcast
 */
static QMP_status_t qmp_bcast_i (QMP_machine_t* glm,
				 void *buffer, 
				 QMP_u32_t count, 
				 QMP_datatype_t datatype, 
				 QMP_u32_t root)
{
  QMP_status_t status;
  QMP_u32_t    rank, size, src, dst;
  QMP_u32_t    relative_rank, mask;

  if (count == 0) return QMP_SUCCESS;

  /* Is root within the comm and more than 1 processes involved? */
  size =  QMP_get_logical_number_of_nodes ();

  if (root >= size) {
    QMP_error ("qmp_broadcast has an invalid root.");
    return QMP_INVALID_ARG;
  }

  
  /* If there is only one process */
  if (size == 1) {
    QMP_error ("only one process in qmp_broadcast :-).");
    return QMP_INVALID_ARG;
  }

  /**
   * Get my logic id
   */
  rank =  QMP_get_logical_node_number ();

  
  /* Algorithm:
     This uses a fairly basic recursive subdivision algorithm.
     The root sends to the process size/2 away; the receiver becomes
     a root for a subtree and applies the same process. 

     So that the new root can easily identify the size of its
     subtree, the (subtree) roots are all powers of two (relative to the root)
     If m = the first power of 2 such that 2^m >= the size of the
     communicator, then the subtree at root at 2^(m-k) has size 2^k
     (with special handling for subtrees that aren't a power of two in size).
     
     Optimizations:
     
     The original code attempted to switch to a linear broadcast when
     the subtree size became too small.  As a further variation, the subtree
     broadcast sent data to the center of the block, rather than to one end.
     However, the original code did not properly compute the communications,
     resulting in extraneous (though harmless) communication.    

     For very small messages, using a linear algorithm (process 0 sends to
     process 1, who sends to 2, etc.) can be better, since no one process
     takes more than 1 send/recv time, and successive bcasts using the same
     root can overlap.  

     Another important technique for long messages is pipelining---sending
     the messages in blocks so that the message can be pipelined through
     the network without waiting for the subtree roots to receive the entire
     message before forwarding it to other processors.  This is hard to
     do if the datatype/count are not the same on each processor (note that
     this is allowed - only the signatures must match).  Of course, this can
     be accomplished at the byte transfer level, but it is awkward 
     from the MPI point-to-point routines.

     Nonblocking operations can be used to achieve some "horizontal"
     pipelining (on some systems) by allowing multiple send/receives
     to begin on the same processor.
  */

  relative_rank = (rank >= root) ? rank - root : rank - root + size;

  /* Do subdivision.  There are two phases:
     1. Wait for arrival of data.  Because of the power of two nature
        of the subtree roots, the source of this message is alwyas the
        process whose relative rank has the least significant bit CLEARED.
        That is, process 4 (100) receives from process 0, process 7 (111) 
        from process 6 (110), etc.   
     2. Forward to my subtree

     Note that the process that is the tree root is handled automatically
     by this code, since it has no bits set.
     
   */
  mask = 0x1;
  while (mask < size) {
    if (relative_rank & mask) {
      src = rank - mask; 
      if (src < 0) src += size;
      status = QMP_recv_message (buffer, count, datatype, src,
				 QMP_BCAST, glm);
      if (status != QMP_SUCCESS) {
	if (status == QMP_TIMEOUT) 
	  QMP_error ("QMP_broadcast receiving from node %d timed out.\n",
		     src);
	return status;
      }
      break;
    }
    mask <<= 1;
  }

  /* This process is responsible for all processes that have bits set from
     the LSB upto (but not including) mask.  Because of the "not including",
     we start by shifting mask back down one.

     We can easily change to a different algorithm at any power of two
     by changing the test (mask > 1) to (mask > block_size) 

     One such version would use non-blocking operations for the last 2-4
     steps (this also bounds the number of MPI_Requests that would
     be needed).
   */
  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < size) {
      dst = rank + mask;
      if (dst >= size) dst -= size;
      status = QMP_send_message (buffer, count, datatype, dst,
				 QMP_BCAST, glm);
      if (status != QMP_SUCCESS) {
	if (status == QMP_TIMEOUT) 
	  QMP_error ("QMP_broadcast sending to node %d timeou.\n", dst);
	return status;
      }
    }
    mask >>= 1;
  }

  return QMP_SUCCESS;
}

/**
 * There are alternatives to this algorithm, particular one in which
 * the values are computed on all processors at the same time.  
 * However, this routine should be used on heterogeneous systems, since
 * the "same" value is required on all processors, and small changes
 * in floating-point arithmetic (including choice of round-off mode and
 * the infamous fused multiply-add) can lead to different results.
 */
QMP_status_t
qmp_simple_all_reduce_i ( QMP_machine_t* glm, 
			  void* sendbuf, void* recvbuf, QMP_u32_t count,
			  QMP_datatype_t datatype, 
			  QMP_rop_t* op_ptr)
{
  QMP_status_t status;

  /* reduce to 0, the bcast */
  status = qmp_reduce_i (glm, sendbuf, recvbuf, count, datatype, op_ptr, 0);
  
  if (status != QMP_SUCCESS)
    return status;

  status = qmp_bcast_i (glm, recvbuf, count, datatype, 0);

  return status;
}

/**
 * My static binary reduction function which call user 
 * provided reduction function.
 */

/**
 * This is a pure hack because multiple binary reductions cannot
 * be called simultaneously 
 */
static QMP_binary_func qmp_user_bfunc_ = 0;

static void
qmp_reduce_func_i (void* inout, void* in,
		   QMP_u32_t count, QMP_datatype_t type)
{
  if (qmp_user_bfunc_)
    (*qmp_user_bfunc_)(inout, in);
}  

/**
 * Binary reduction.
 */
QMP_status_t
QMP_binary_reduction (void* lbuffer, QMP_u32_t buflen,
		      QMP_binary_func bfunc)
{
  QMP_rop_t    op_ptr;
  QMP_status_t status;
  void*        rbuffer;

  QMP_TRACE ("QMP_binary_reduction");

  /* first check whether there is a binary reduction is in session */
  if (qmp_user_bfunc_) 
    QMP_error_exit ("Another binary reduction is in progress.\n");

  rbuffer = QMP_memalign (buflen, QMP_MEM_ALIGNMENT);
  if (!rbuffer)
    return QMP_NOMEM_ERR;

  /* set up user binary reduction pointer */
  qmp_user_bfunc_ = bfunc;

  op_ptr.op = 0;
  op_ptr.commute = QMP_TRUE;
  op_ptr.func = qmp_reduce_func_i;

  status = qmp_simple_all_reduce_i (&QMP_global_m, lbuffer, 
				    rbuffer, buflen, QMP_BYTE, &op_ptr);

  if (status == QMP_SUCCESS) 
    memcpy (lbuffer, rbuffer, buflen);
  free (rbuffer);

  /* signal end of the binary reduction session */
  qmp_user_bfunc_ = 0;

  return status;
}

/**
 * Check whether a number is of form 2^n
 */
static int qmp_is_power_of_two_i (unsigned int num)
{
  int cnt = 0;

  while (num != 0) {
    num = num & (num - 1);
    cnt++;
  }
  if (cnt == 1)
    return 1;

  return 0;
}

/**
 * This is an alternative qmp_all_reduce_i to use bufferfly algorithm
 * to calculate global reduction. 
 * This algorithm can be applied to a homogeneous system with full
 * duplex communication capabilities.
 */
QMP_status_t
qmp_butterfly_all_reduce_i (QMP_machine_t* glm, 
			    void* sendbuf, void* recvbuf, QMP_u32_t count,
			    QMP_datatype_t datatype, 
			    QMP_rop_t* op_ptr)
{
  int          perfect;
  QMP_status_t status;
  QMP_u32_t    rank, size, buflen, i;
  QMP_opfunc   uop;
  void*        buffer;
  QMP_u32_t    mask;
  QMP_u32_t    remote, num_sends;
  QMP_msg_queue_slot_t* all_sends[QMP_GENMSG_NBUF];


  QMP_TRACE ("qmp_butterfly_all_reduce_i");

  if (count == 0) return QMP_SUCCESS;

  if (!op_ptr->commute) {
    QMP_error ("non commutitive operations should be managed by applications.");
    return QMP_NOTSUPPORTED;
  }

  /**
   * Check data size not to exceed our limit
   */
  if (count * QMP_DATA_SIZE(datatype) > glm->max_msg_length) {
    QMP_error ("trying to send a message that is too long.");
    return QMP_MEMSIZE_TOOBIG;
  }

  /**
   * Size of this global communication.
   */
  size = QMP_get_logical_number_of_nodes ();

  /* If there is only one process */
  if (size == 1) {
    QMP_error ("only one process in qmp_all_reduce :-).");
    return QMP_INVALID_ARG;
  }

  /**
   * Find out whether number of nodes involved is power of 2
   */
  perfect = qmp_is_power_of_two_i (size);

  if (!perfect) {
    /**
     * If this is not a perfect power of 2 use tradiational algorithm
     */
    return qmp_simple_all_reduce_i (glm, sendbuf, recvbuf, count,
				    datatype, op_ptr);
  }

  /**
   * Get my node id
   */
  rank = QMP_get_logical_node_number ();

  /**
   * Get real function pointer.
   */
  uop = op_ptr->func;

  /* create temp buffer */
  buflen = count*QMP_DATA_SIZE(datatype);
  buffer = QMP_memalign (buflen, QMP_MEM_ALIGNMENT);

  /* Initialize all sends */
  num_sends = 0;
  for (i = 0; i < QMP_GENMSG_NBUF; i++)
    all_sends[i] = 0;

  status = QMP_SUCCESS;
  /* copy send buffer to recv buffer */
  memcpy( recvbuf, sendbuf, buflen);
  mask = 0x1;

  while (mask < size) {
    /**
     * Calculate receiving node rank, and send node rank.
     */
    remote = rank ^ mask;

    /**
     * This is a perfect balanced tree 2^n
     */
    if (remote < size && remote != rank) {
      /**
       * First start sending to the remote node.
       */
      status = qmp_start_send_genmsg_i (recvbuf, count, datatype,
					remote, QMP_REDUCE, 
					QMP_MSG_PRIORITY, glm,
					&all_sends[num_sends]);
      if (status != QMP_SUCCESS) {
	if (status == QMP_TIMEOUT) 
	  QMP_error ("QMP_all_reduce sending message to node %d timed out.\n",
		     remote);
	return status;
      }

      num_sends++;
	
      /**
       * Receiving message from a source node and do operation.
       */
      status = QMP_recv_message (buffer, count, datatype, remote,
				 QMP_REDUCE, glm);
      if (status != QMP_SUCCESS) {
	if (status == QMP_TIMEOUT)
	  QMP_error ("QMP_all_reduce receiving message from node %d timed out.\n",
		     remote);
	return status;
      }
      
      /**
       * Function looks like (result, src).
       */
      (*uop)(recvbuf, buffer, count, datatype);
    }
	   
    mask <<= 1;
  }

  free (buffer);

  /**
   * Now let us make sure all sends are finished.
   */
  for (i = 0; i < num_sends; i++) {
    status = qmp_wait_for_genmsg_sent_i (all_sends[i], QMP_MSG_WAIT_TIMEOUT);
    if (status != QMP_SUCCESS) {
      QMP_fprintf (stderr, 
		   "QMP_all_reduce a previous send is not finished.\n");

      return status;
    }
    QMP_delete_msg_queue_slot (all_sends[i]);
  }


  return QMP_SUCCESS;
}
  
		  

/**
 * Broadcast from root (logic node id 0) to every other node.
 */
QMP_status_t
QMP_broadcast (void* buffer, QMP_u32_t nbytes)
{
  if (nbytes == QMP_DATA_SIZE (QMP_INT))
    return qmp_bcast_i (&QMP_global_m, buffer, 1, QMP_INT, 0);
  else if (nbytes == QMP_DATA_SIZE (QMP_DOUBLE))
    return qmp_bcast_i (&QMP_global_m, buffer, 1, QMP_DOUBLE, 0);
  else
    return qmp_bcast_i (&QMP_global_m, buffer, nbytes, QMP_BYTE, 0);
}

/**
 * GLobal Sum of an integer
 */
QMP_status_t
QMP_sum_int (QMP_s32_t *value)
{
  QMP_s32_t rvalue;
  QMP_status_t status;
  QMP_u32_t count;

  QMP_TRACE ("QMP_sum_int");
  count = 1;
#ifdef _QMP_QUICK_GCOMM
  status = qmp_butterfly_all_reduce_i (&QMP_global_m, value, &rvalue,
				       count, QMP_INT, 
				       &QMP_operator_table[QMP_SUM]);
#else
  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    count, QMP_INT, 
				    &QMP_operator_table[QMP_SUM]);
#endif

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}

/**
 * GLobal Sum of a float
 */
QMP_status_t
QMP_sum_float (QMP_float_t *value)
{
  QMP_float_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_sum_float");

#ifdef _QMP_QUICK_GCOMM
  status = qmp_butterfly_all_reduce_i (&QMP_global_m, value, &rvalue,
				       1, QMP_FLOAT, 
				       &QMP_operator_table[QMP_SUM]);
#else
  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_FLOAT, 
				    &QMP_operator_table[QMP_SUM]);
#endif

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}


/**
 * GLobal Sum of a double
 */
QMP_status_t
QMP_sum_double (QMP_double_t *value)
{
  QMP_double_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_sum_double");

#ifdef _QMP_QUICK_GCOMM
  status = qmp_butterfly_all_reduce_i (&QMP_global_m, value, &rvalue,
				       1, QMP_DOUBLE, 
				       &QMP_operator_table[QMP_SUM]);
#else
  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_DOUBLE, 
				    &QMP_operator_table[QMP_SUM]);
#endif

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}

/**
 * GLobal Sum of a double
 */
QMP_status_t
QMP_sum_double_extended (QMP_double_t *value)
{
  QMP_double_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_sum_double_extended");

#ifdef _QMP_QUICK_GCOMM
  status = qmp_butterfly_all_reduce_i (&QMP_global_m, value, &rvalue,
				       1, QMP_DOUBLE, 
				       &QMP_operator_table[QMP_SUM]);
#else
  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_DOUBLE, 
				    &QMP_operator_table[QMP_SUM]);
#endif

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}

/**
 * Global sum of a float array.
 */
QMP_status_t
QMP_sum_float_array (QMP_float_t* value, QMP_u32_t length)
{
  QMP_float_t* rvalue;
  QMP_status_t status;
  int          i;

  QMP_TRACE ("QMP_sum_float_array");

  rvalue = (QMP_float_t *)QMP_memalign (length * sizeof (QMP_float_t),
					QMP_MEM_ALIGNMENT);
  if (!rvalue) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_float_array.");
    return QMP_NOMEM_ERR;
  }

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, rvalue,
				    length, QMP_FLOAT, 
				    &QMP_operator_table[QMP_SUM]);

  if (status == QMP_SUCCESS) {
    for (i = 0; i < length; i++)
      value[i] = rvalue[i];
  }

  free (rvalue);
  return status;
}


/**
 * Global sum of a double array.
 */
QMP_status_t
QMP_sum_double_array (QMP_double_t* value, QMP_u32_t length)
{
  QMP_double_t* rvalue;
  QMP_status_t status;
  int          i;

  QMP_TRACE ("QMP_sum_double_array");

  rvalue = (QMP_double_t *)QMP_memalign (length * sizeof (QMP_double_t),
					 QMP_MEM_ALIGNMENT);
  if (!rvalue) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_double_array.");
    return QMP_NOMEM_ERR;
  }
  status = qmp_simple_all_reduce_i (&QMP_global_m, value, rvalue,
				    length, QMP_DOUBLE, 
				    &QMP_operator_table[QMP_SUM]);

  if (status == QMP_SUCCESS) {
    for (i = 0; i < length; i++)
      value[i] = rvalue[i];
  }

  free (rvalue);
  return status;
}

/**
 * Get maximum value of all floats
 */
QMP_status_t
QMP_max_float (QMP_float_t* value)
{
  QMP_float_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_max_float");

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_FLOAT, 
				    &QMP_operator_table[QMP_MAX]);

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}

/**
 * Get maximum value of all doubles
 */
QMP_status_t
QMP_max_double (QMP_double_t* value)
{
  QMP_double_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_max_double");

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_DOUBLE, 
				    &QMP_operator_table[QMP_MAX]);

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}  


/**
 * Get minimum value of all floats
 */
QMP_status_t
QMP_min_float (QMP_float_t* value)
{
  QMP_float_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_min_float");

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_FLOAT, 
				    &QMP_operator_table[QMP_MIN]);

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}

/**
 * Get minimum value of all doubles
 */
QMP_status_t
QMP_min_double (QMP_double_t* value)
{
  QMP_double_t rvalue;
  QMP_status_t status;

  QMP_TRACE ("QMP_min_double");

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, QMP_DOUBLE, 
				    &QMP_operator_table[QMP_MIN]);

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}  

/**
 * Get the ored value of all long integers
 */
QMP_status_t
QMP_global_xor (long* value)
{
  QMP_datatype_t type;
  QMP_status_t   status;

#ifdef QMP_64BIT_LONG
  QMP_s64_t      rvalue;
  type = QMP_64BIT_INT;
#else
  QMP_s32_t    rvalue;
  type = QMP_INT;
#endif

  QMP_TRACE ("QMP_global_xor");

  status = qmp_simple_all_reduce_i (&QMP_global_m, value, &rvalue,
				    1, type, &QMP_operator_table[QMP_XOR]);

  if (status == QMP_SUCCESS)
    *value = rvalue;

  return status;
}


/**
 * Synchronization barrier call.
 * This is a non efficient implementation.
 *
 */
QMP_status_t
QMP_wait_for_barrier (QMP_s32_t millieseconds)
{

  QMP_s32_t value, rvalue;
  QMP_status_t status;
  QMP_u32_t count;

  QMP_TRACE ("QMP_wait_for_barrier");
  
  count = 1;
  rvalue = value = 1;
  status = qmp_simple_all_reduce_i (&QMP_global_m, &value, &rvalue,
				    count, QMP_INT, 
				    &QMP_operator_table[QMP_SUM]);

  if (status == QMP_SUCCESS)
    value = rvalue;

  return status;
}

