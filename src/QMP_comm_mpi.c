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
 *      Communication part of QMP implementation over MPI
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.2  2003/02/13 16:22:23  chen
 *   qmp version 1.2
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.4  2002/05/31 15:55:00  chen
 *   Fix a bug on sum_double
 *
 *   Revision 1.3  2002/05/07 17:18:44  chen
 *   Make spec and header file consistent
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:41  chen
 *   Version 0.95 Release
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_MPI.h"

/* Start send (non-blocking) from possibly ganged message handles */
/* Start receive (non-blocking) from possibly ganged message handles */
/* NOTE: need to check more carefully for errors */
QMP_status_t
QMP_start (QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int err = QMP_SUCCESS;
  int multipleP = 0;
    
#ifdef _QMP_DEBUG
  QMP_info ("starting QMP_start: id=%d\n", QMP_get_node_number());
#endif

  if (!mh) {
    QMP_SET_STATUS_CODE(QMP_INVALID_ARG);
    return QMP_INVALID_ARG;
  }

  while (mh)
  {
    Message_Memory_t mm;

    switch (mh->type)
    {
    case MH_multiple:
      multipleP = 1;
      break;

    case MH_send:
      mm = (Message_Memory_t)(mh->mm);
#ifdef _QMP_DEBUG
      QMP_info("Node %d: isend to %d with tag=%d and %d bytes long\n", 
	       QMP_get_node_number(),
	       mh->dest_node, mh->tag, mm->nbytes);
#endif
      MPI_Isend(mm->mem, mm->nbytes, 
		MPI_BYTE, mh->dest_node, mh->tag,
		MPI_COMM_WORLD, &mh->request);
      mh->activeP = 1;
      break;

    case MH_recv:
      mm = (Message_Memory_t)(mh->mm);
#ifdef _QMP_DEBUG
      QMP_info ("Node %d: irecv from %d with tag=%d and %d bytes long\n", 
		QMP_get_node_number(),
		mh->srce_node, mh->tag, mm->nbytes);
#endif
      MPI_Irecv(mm->mem, mm->nbytes,
		MPI_BYTE, mh->srce_node, mh->tag,
		MPI_COMM_WORLD, &mh->request);
      mh->activeP = 1;
      break;

    case MH_freed:
    case MH_empty:
      err = QMP_INVALID_OP;
      QMP_SET_STATUS_CODE (QMP_INVALID_OP);
      break;
    default:
      QMP_fatal(1,"QMP_start: internal error - unknown message type");
      break;
    }

    mh = (multipleP) ? mh->next : NULL;
  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_start: id=%d\n",QMP_get_node_number());
#endif

  return err;
}


/* Internal routine for checking on waiting send & receive messages */
static QMP_status_t
QMP_wait_send_receive(QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int flag = QMP_SUCCESS;
  MPI_Request request[MAXBUFFER];
  MPI_Status  status[MAXBUFFER];
  int num = 0;

#ifdef _QMP_DEBUG
  QMP_info ("Calling QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  /* Collect together all the outstanding send/recv requests */
  while (mh)
  {
    if (mh->type == MH_send || mh->type == MH_recv)
    {
#if 1
      /* (Slow) Paranoid test for activity. Check against both the activity
       * and the request field */
      if (mh->activeP && (mh->request == MPI_REQUEST_NULL))
	QMP_fatal(1,"QMP_wait: internal error: found null request but active message");
      
      if (! mh->activeP && (mh->request != MPI_REQUEST_NULL))
	QMP_fatal(1,"QMP_wait: internal error: found active request but inactive message");
#endif

      /* Quick test for activity. Be careful this flag matches reality! */
      if (mh->activeP)
      {
	if (num == MAXBUFFER)
	  QMP_fatal(1,"QMP_wait: internal error: too many active requests");

	request[num++] = mh->request;
      }
    }

    mh = mh->next;
  }

  if (num == 0)
  {
#ifdef _QMP_DEBUG
    fprintf(stderr,"QMP_wait: no send/recvs outstanding, id=%d\n",QMP_get_node_number());
#endif    
    return flag;
  }

#ifdef _QMP_DEBUG
  fprintf(stderr,"QMP_wait: waiting on %d sends\n",num);
#endif    

  /* Wait on all the messages */
  if ((flag = MPI_Waitall(num, request, status)) != MPI_SUCCESS) {
    printf ("Falg is %d\n", flag);
    QMP_fatal(1,"QMP_wait: test unexpectedly failed");
  }

#ifdef _QMP_DEBUG
  fprintf(stderr,"QMP_wait: finished %d sends\n",num);
#endif    

  /* Mark all sends as inactive */
  /* To be safe, update all the send request handles. Probably not needed in
   * general, but is used in tests above */
  /* NOTE: not handling errors at all */
  mh = (Message_Handle_t)msgh;

  while (mh)
  {
    if (mh->type == MH_send)
      mh->request = MPI_REQUEST_NULL;

    mh->activeP = 0;
    mh = mh->next;
  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished waitForSend, id=%d\n",QMP_get_node_number());
#endif

  return flag;
}


/* Check if all messages in msgh are received. 
 * NOTE: Does not attempt to receive any messages 
 */
QMP_bool_t
QMP_is_complete(QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int activeP = 0;
  
#ifdef _QMP_DEBUG
  fprintf(stderr,"Calling QMP_is_complete, id=%d\n",QMP_get_node_number());
#endif

  /* Just OR all message activities */
  while (mh)
  {
    switch (mh->type)
    {
    case MH_send:
    case MH_recv:
      activeP |= mh->activeP;
      break;

    default:
      break;
    }

    mh = mh->next;
  }

#ifdef _QMP_DEBUG
  fprintf(stderr,"Finished QMP_is_complete, id=%d\n",QMP_get_node_number());
#endif

  return !activeP;
}


/* Wait on communications (blocks) */
/* NOTE: this routine allows mixtures of sends and receives */
QMP_status_t
QMP_wait(QMP_msghandle_t msgh)
{
  int err = QMP_SUCCESS;

#ifdef _QMP_DEBUG
  QMP_info ("Starting QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  if (!QMP_is_complete(msgh))
    if (QMP_wait_send_receive(msgh))
      QMP_fatal(1,"QMP_wait: some error in QMP_wait_send_receive");

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  return err;
}



/* Global barrier */
QMP_status_t
QMP_wait_for_barrier(QMP_s32_t millisec)
{
  /* In this implementation, ignore time for barrier */
  return MPI_Barrier(MPI_COMM_WORLD);
}

/* Broadcast via interface specific routines */
QMP_status_t
QMP_broadcast(void *send_buf, QMP_u32_t count)
{
  return MPI_Bcast(send_buf, count, MPI_BYTE, 0, MPI_COMM_WORLD);
}

/* Global sums */
QMP_status_t
QMP_sum_int (QMP_s32_t *value)
{
  QMP_status_t status;
  QMP_s32_t dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_float(QMP_float_t *value)
{
  QMP_status_t status;
  QMP_float_t  dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;
  
  return status;
}

QMP_status_t
QMP_sum_double (QMP_double_t *value)
{
  QMP_status_t status;
  QMP_double_t dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_double_extended (QMP_double_t *value)
{
  QMP_status_t status;
  QMP_double_t dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_float_array (QMP_float_t *value, QMP_u32_t count)
{
  QMP_status_t status;
  QMP_float_t* dest;
  int          i;

  dest = (QMP_float_t *) malloc (count * sizeof (QMP_float_t));

  if (!dest) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_float_array.");
    return QMP_NOMEM_ERR;
  }
  status = MPI_Allreduce((void *)value, (void *)dest, count,
			 MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
  if (status == MPI_SUCCESS) {
    for (i = 0; i < count; i++)
      value[i] = dest[i];
  }
  
  free (dest);
  return status;
}


QMP_status_t
QMP_sum_double_array (QMP_double_t *value, QMP_u32_t count)
{
  QMP_status_t status;
  QMP_double_t* dest;
  int          i;

  dest = (QMP_double_t *) malloc (count * sizeof (QMP_double_t));

  if (!dest) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_double_array.");
    return QMP_NOMEM_ERR;
  }
  status = MPI_Allreduce((void *)value, (void *)dest, count,
			 MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  
  if (status == MPI_SUCCESS) {
    for (i = 0; i < count; i++)
      value[i] = dest[i];
  }
  
  free (dest);
  return status;
}

/**
 * This is a pure hack since our user binary function is defined
 * differently from MPI spec. In addition we are not expecting
 * multiple binary reduction can be carried out simultaneously.
 */
static QMP_binary_func qmp_user_bfunc_ = 0;

/**
 * This is a internal function which will be passed to MPI all reduce
 */
static int 
qmp_MPI_bfunc_i (void* in, void* inout, int* len, MPI_Datatype* type)
{
  if (qmp_user_bfunc_)
    (*qmp_user_bfunc_)(inout, in);
}

QMP_status_t
QMP_binary_reduction (void *lbuffer, QMP_u32_t count, 
		      QMP_binary_func bfunc)
{
  MPI_Op       bop;
  void*        rbuffer;
  QMP_status_t status;

  /* first check whether there is a binary reduction is in session */
  if (qmp_user_bfunc_) 
    QMP_error_exit ("Another binary reduction is in progress.\n");

  rbuffer = QMP_memalign (count, QMP_MEM_ALIGNMENT);
  if (!rbuffer)
    return QMP_NOMEM_ERR;

  /* set up user binary reduction pointer */
  qmp_user_bfunc_ = bfunc;

  if ((status = MPI_Op_create (qmp_MPI_bfunc_i, 1, &bop)) != MPI_SUCCESS) {
    QMP_error ("Cannot create MPI operator for binary reduction.\n");
    return status;
  }
  
  status = MPI_Allreduce(lbuffer,rbuffer,count, MPI_BYTE, bop, MPI_COMM_WORLD);

  if (status == QMP_SUCCESS) 
    memcpy (lbuffer, rbuffer, count);
  free (rbuffer);
  
  /* free binary operator */
  MPI_Op_free (&bop);

  /* signal end of the binary reduction session */
  qmp_user_bfunc_ = 0;

    
  return QMP_SUCCESS;
}


QMP_status_t
QMP_max_float(QMP_float_t* value)
{
  QMP_float_t dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_min_float(QMP_float_t* value)
{
  QMP_float_t dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}  

QMP_status_t
QMP_max_double(QMP_double_t* value)
{
  QMP_double_t dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_min_double(QMP_double_t *value)
{
  QMP_double_t dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_global_xor(long* value)
{
  long dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_LONG, MPI_BXOR, MPI_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}


