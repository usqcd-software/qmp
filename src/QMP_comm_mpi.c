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
 *   Revision 1.5  2004/04/08 09:00:20  bjoo
 *   Added experimental support for strided msgmem
 *
 *   Revision 1.4  2003/02/19 20:37:44  chen
 *   Make QMP_is_complete a polling function
 *
 *   Revision 1.3  2003/02/18 18:16:18  chen
 *   Fix a minor bug for is_complete
 *
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
#include "QMP_P_COMMON.h"
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
      
      /* Deal with strided sends */
      switch( mm->type) { 
      case MM_user_buf:
	MPI_Isend(mm->mem, mm->nbytes, 
		  MPI_BYTE, mh->dest_node, mh->tag,
		  QMP_COMM_WORLD, &mh->request);
	mh->activeP = 1;
	break;
      case MM_strided_buf:
	MPI_Isend(mm->mem, 1, 
		  mm->mpi_type,
		  mh->dest_node,
		  mh->tag,
		  QMP_COMM_WORLD, 
		  &mh->request);
	mh->activeP = 1;
	break;
      case MM_strided_array_buf:
	MPI_Isend(MPI_BOTTOM, 1,
		  mm->mpi_type,
		  mh->dest_node,
		  mh->tag,
		  QMP_COMM_WORLD,
		  &mh->request);
	mh->activeP = 1;
	break;
      default:
	QMP_FATAL("internal error: unknown memory type");
	break;
      }
      
      break;

    case MH_recv:
      mm = (Message_Memory_t)(mh->mm);
#ifdef _QMP_DEBUG
      QMP_info ("Node %d: irecv from %d with tag=%d and %d bytes long\n", 
		QMP_get_node_number(),
		mh->srce_node, mh->tag, mm->nbytes);
#endif
      switch(mm->type) { 
      case MM_user_buf:
	MPI_Irecv(mm->mem, mm->nbytes,
		  MPI_BYTE, mh->srce_node, mh->tag,
		  QMP_COMM_WORLD, &mh->request);
	mh->activeP = 1;
	break;
      case MM_strided_buf:
	MPI_Irecv(mm->mem, 1,
		  mm->mpi_type,
		  mh->srce_node, mh->tag,
		  QMP_COMM_WORLD, &mh->request);
	mh->activeP = 1;
	break;
      case MM_strided_array_buf:
	MPI_Irecv(MPI_BOTTOM, 1,
		  mm->mpi_type,
		  mh->srce_node, mh->tag,
		  QMP_COMM_WORLD, &mh->request);
	mh->activeP = 1;
	break;
      default:
	QMP_FATAL("internal error: unknown memory type");
	break;
      }
      break;

    case MH_freed:
    case MH_empty:
      err = QMP_INVALID_OP;
      QMP_SET_STATUS_CODE (QMP_INVALID_OP);
      break;
    default:
      QMP_FATAL("internal error: unknown message type");
      break;
    }

    mh = (multipleP) ? mh->next : NULL;
  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_start: id=%d\n",QMP_get_node_number());
#endif

  return err;
}


#define MAXBUFFER  64
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

  /* Collect together all the outstanding send/recv requests and process */
  while(1) {

    if( (!mh) || (num == MAXBUFFER) ) {  /* Wait on all the messages */
      if(num>0) {
#ifdef _QMP_DEBUG
	fprintf(stderr,"QMP_wait: waiting on %d sends\n",num);
#endif
	if ((flag = MPI_Waitall(num, request, status)) != MPI_SUCCESS) {
	  QMP_fprintf (stderr, "Wait all Flag is %d\n", flag);
	  QMP_FATAL("test unexpectedly failed");
	}
#ifdef _QMP_DEBUG
	fprintf(stderr,"QMP_wait: finished %d sends\n",num);
#endif
	num = 0;
      } else {
#ifdef _QMP_DEBUG
	fprintf(stderr,"QMP_wait: no send/recvs outstanding, id=%d\n",QMP_get_node_number());
#endif
      }
    }

    if(!mh) break;

    if( (mh->type == MH_send) || (mh->type == MH_recv) ) {
#if 1
      /* (Slow) Paranoid test for activity. Check against both the activity
       * and the request field */
      if (mh->activeP && (mh->request == MPI_REQUEST_NULL))
	QMP_FATAL("internal error: found null request but active message");

      if (! mh->activeP && (mh->request != MPI_REQUEST_NULL))
	QMP_FATAL("internal error: found active request but inactive message");
#endif

      /* Quick test for activity. Be careful this flag matches reality! */
      if (mh->activeP) {
	request[num++] = mh->request;
      }
    }

    mh = mh->next;
  }

  /* Mark all sends as inactive */
  /* To be safe, update all the send request handles. Probably not needed in
   * general, but is used in tests above */
  /* NOTE: not handling errors at all */
  mh = (Message_Handle_t)msgh;

  while (mh) {
    mh->request = MPI_REQUEST_NULL;

    mh->activeP = 0;
    mh = mh->next;
  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished waitForSend, id=%d\n",QMP_get_node_number());
#endif

  return flag;
}


/* Internal routine for testing on waiting send & receive messages */
static QMP_bool_t
QMP_test_send_receive (QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int flag, callst;
  QMP_bool_t  done;
  MPI_Request request;
  MPI_Status  status;

#ifdef _QMP_DEBUG
  QMP_info ("Calling QMP_test_send_receive, id=%d\n",QMP_get_node_number());
#endif

  /* Check each request to find out whether it is finished */
  done = QMP_TRUE;

  while (mh) {
    flag = 0;

    if (mh->type == MH_send || mh->type == MH_recv) {
#if 1
      /* (Slow) Paranoid test for activity. Check against both the activity
       * and the request field */
      if (mh->activeP && (mh->request == MPI_REQUEST_NULL))
	QMP_FATAL("internal error: found null request but active message");
      
      if (! mh->activeP && (mh->request != MPI_REQUEST_NULL))
	QMP_FATAL("internal error: found active request but inactive message");
#endif

      /* Quick test for activity. Be careful this flag matches reality! */
      if (mh->activeP) {
	request = mh->request;

	if ((callst = MPI_Test (&request, &flag, &status)) != MPI_SUCCESS) {
	  QMP_FATAL("test unexpected failed.");
	}
	else {
	  if (flag) {
	    /* This request is done */
	    mh->request = MPI_REQUEST_NULL;
	    mh->activeP = 0;
	  }
	  else
	    done = QMP_FALSE;
	}
      }
    }

    /* Go to next */
    mh = mh->next;
  }

  return done;
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

  /* Not finished we do a quick test */
  if (activeP) 
    return QMP_test_send_receive (msgh);
  else 
    return QMP_TRUE;
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
      QMP_FATAL("some error in QMP_wait_send_receive");

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  return err;
}

/* Wait on array of communications (blocks) */
/* NOTE: this routine allows mixtures of sends and receives */
QMP_status_t
QMP_wait_all(QMP_msghandle_t msgh[], int num)
{
  QMP_status_t err;
  int i;

#ifdef _QMP_DEBUG
  QMP_info ("Starting QMP_wait_all, id=%d\n",QMP_get_node_number());
#endif

  for(i=0; i<num; i++) {
    if(!QMP_is_complete(msgh[i])) {
      err = QMP_wait_send_receive(msgh[i]);
      if(err!=QMP_SUCCESS) {
	QMP_FATAL("some error in QMP_wait_send_receive");
	break;
      }
    }
  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  return err;
}



/* Global barrier */
QMP_status_t
QMP_barrier(void)
{
  return MPI_Barrier(QMP_COMM_WORLD);
}

/* Broadcast via interface specific routines */
QMP_status_t
QMP_broadcast(void *send_buf, size_t count)
{
  return MPI_Bcast(send_buf, count, MPI_BYTE, 0, QMP_COMM_WORLD);
}

/* Global sums */
QMP_status_t
QMP_sum_int (int *value)
{
  QMP_status_t status;
  int dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_INT, MPI_SUM, QMP_COMM_WORLD);
  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_float(float *value)
{
  QMP_status_t status;
  float  dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_SUM, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;
  
  return status;
}

QMP_status_t
QMP_sum_double (double *value)
{
  QMP_status_t status;
  double dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_SUM, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_double_extended (double *value)
{
  QMP_status_t status;
  double dest;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_SUM, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_sum_float_array (float value[], int count)
{
  QMP_status_t status;
  float* dest;
  int          i;

  dest = (float *) malloc (count * sizeof (float));

  if (!dest) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_float_array.");
    return QMP_NOMEM_ERR;
  }
  status = MPI_Allreduce((void *)value, (void *)dest, count,
			 MPI_FLOAT, MPI_SUM, QMP_COMM_WORLD);
  if (status == MPI_SUCCESS) {
    for (i = 0; i < count; i++)
      value[i] = dest[i];
  }
  
  free (dest);
  return status;
}


QMP_status_t
QMP_sum_double_array (double value[], int count)
{
  QMP_status_t status;
  double* dest;
  int          i;

  dest = (double *) malloc (count * sizeof (double));

  if (!dest) {
    QMP_error ("cannot allocate receiving memory in QMP_sum_double_array.");
    return QMP_NOMEM_ERR;
  }
  status = MPI_Allreduce((void *)value, (void *)dest, count,
			 MPI_DOUBLE, MPI_SUM, QMP_COMM_WORLD);
  
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
static MPI_Op bop;
static int op_inited=0;

/**
 * This is a internal function which will be passed to MPI all reduce
 */
static void
qmp_MPI_bfunc_i (void* in, void* inout, int* len, MPI_Datatype* type)
{
  if (qmp_user_bfunc_)
    (*qmp_user_bfunc_)(inout, in);
}

QMP_status_t
QMP_binary_reduction (void *lbuffer, size_t count, QMP_binary_func bfunc)
{
  void*        rbuffer;
  QMP_status_t status;

  /* first check whether there is a binary reduction is in session */
  if (qmp_user_bfunc_) 
    QMP_FATAL ("Another binary reduction is in progress.");

  //rbuffer = QMP_memalign (count, QMP_MEM_ALIGNMENT);
  rbuffer = QMP_allocate_memory (count);
  if (!rbuffer)
    return QMP_NOMEM_ERR;

  /* set up user binary reduction pointer */
  qmp_user_bfunc_ = bfunc;

  if(!op_inited) {
    if ((status = MPI_Op_create (qmp_MPI_bfunc_i, 1, &bop)) != MPI_SUCCESS) {
      QMP_error ("Cannot create MPI operator for binary reduction.\n");
      return status;
    }
    op_inited = 1;
  }

  status = MPI_Allreduce(lbuffer,rbuffer,count, MPI_BYTE, bop, QMP_COMM_WORLD);

  if (status == QMP_SUCCESS) 
    memcpy (lbuffer, rbuffer, count);
  QMP_free_memory (rbuffer);

  /* free binary operator */
  //MPI_Op_free (&bop);

  /* signal end of the binary reduction session */
  qmp_user_bfunc_ = 0;

  return QMP_SUCCESS;
}


QMP_status_t
QMP_max_float(float* value)
{
  float dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_MAX, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_min_float(float* value)
{
  float dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_FLOAT, MPI_MIN, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}  

QMP_status_t
QMP_max_double(double* value)
{
  double dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_MAX, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_min_double(double *value)
{
  double dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_DOUBLE, MPI_MIN, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}

QMP_status_t
QMP_xor_ulong(unsigned long* value)
{
  unsigned long dest;
  QMP_status_t status;

  status = MPI_Allreduce((void *)value, (void *)&dest, 1,
			 MPI_UNSIGNED_LONG, MPI_BXOR, QMP_COMM_WORLD);

  if (status == MPI_SUCCESS)
    *value = dest;

  return status;
}
