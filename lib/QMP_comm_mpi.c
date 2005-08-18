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
 *   Revision 1.4  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.3  2005/03/02 18:21:35  morten
 *   Minor bug fix for QMP_wait_all which always returned QMP_ERROR
 *
 *   Revision 1.2  2004/12/16 02:44:12  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.7  2004/06/25 18:08:05  bjoo
 *   DONT USE C++ COMMENTS IN C CODElsls!
 *
 *   Revision 1.6  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
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

#include "QMP_P_MPI.h"

/* Start send (non-blocking) from possibly ganged message handles */
/* Start receive (non-blocking) from possibly ganged message handles */
/* NOTE: need to check more carefully for errors */
QMP_status_t
QMP_start (QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int err = QMP_SUCCESS;

#ifdef _QMP_DEBUG
  QMP_info ("starting QMP_start: id=%d\n", QMP_get_node_number());
#endif

  if (!mh) {
    QMP_SET_STATUS_CODE(QMP_INVALID_ARG);
    return QMP_INVALID_ARG;
  }

  switch (mh->type) {
  case MH_multiple:
    MPI_Startall(mh->num, mh->request_array);
    mh->activeP = 1;
    break;

  case MH_send:
  case MH_recv:
    MPI_Start(&mh->request);
    mh->activeP = 1;
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

#ifdef _QMP_DEBUG
  QMP_info ("Calling QMP_wait, id=%d\n",QMP_get_node_number());
#endif

  switch (mh->type) {

  case MH_multiple:
    if(mh->activeP) {
      MPI_Status status[mh->num];
      flag = MPI_Waitall(mh->num, mh->request_array, status);
      if (flag != MPI_SUCCESS) {
	QMP_fprintf (stderr, "Wait all Flag is %d\n", flag);
	QMP_FATAL("test unexpectedly failed");
      }
      mh->activeP = 0;
    }
    break;

  case MH_send:
  case MH_recv:
    if(mh->activeP) {
      MPI_Status status;
      flag = MPI_Wait(&mh->request, &status);
      if (flag != MPI_SUCCESS) {
	QMP_fprintf (stderr, "Wait all Flag is %d\n", flag);
	QMP_FATAL("test unexpectedly failed");
      }
      mh->activeP = 0;
    }
    break;

  case MH_freed:
  case MH_empty:
    flag = QMP_INVALID_OP;
    QMP_SET_STATUS_CODE (QMP_INVALID_OP);
    break;

  default:
    QMP_FATAL("internal error: unknown message type");
    break;

  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished waitForSend, id=%d\n",QMP_get_node_number());
#endif

  return flag;
}

/* Check if all messages in msgh are completed */
QMP_bool_t
QMP_is_complete(QMP_msghandle_t msgh)
{
  Message_Handle_t mh = (Message_Handle_t)msgh;
  int flag, callst;
  QMP_bool_t done = QMP_FALSE;

#ifdef _QMP_DEBUG
  QMP_info ("Calling QMP_is_complete, id=%d\n", QMP_get_node_number());
#endif

  switch (mh->type) {

  case MH_multiple:
    if(mh->activeP) {
      MPI_Status status[mh->num];
      callst = MPI_Testall(mh->num, mh->request_array, &flag, status);
      if (callst != MPI_SUCCESS) {
	QMP_fprintf (stderr, "Testall return value is %d\n", callst);
	QMP_FATAL("test unexpectedly failed");
      }
      if(flag) {
	done = QMP_TRUE;
	mh->activeP = 0;
      }
    }
    break;

  case MH_send:
  case MH_recv:
    if(mh->activeP) {
      MPI_Status status;
      callst = MPI_Test(&mh->request, &flag, &status);
      if (callst != MPI_SUCCESS) {
	QMP_fprintf (stderr, "Test return value is %d\n", callst);
	QMP_FATAL("test unexpectedly failed");
      }
      if(flag) {
	done = QMP_TRUE;
	mh->activeP = 0;
      }
    }
    break;

  case MH_freed:
  case MH_empty:
    QMP_SET_STATUS_CODE (QMP_INVALID_OP);
    break;

  default:
    QMP_FATAL("internal error: unknown message type");
    break;

  }

#ifdef _QMP_DEBUG
  QMP_info ("Finished QMP_is_complete, id=%d\n", QMP_get_node_number());
#endif

  return done;
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

  /* if (!QMP_is_complete(msgh)) */
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
  QMP_status_t err=QMP_SUCCESS;
  int i;

#ifdef _QMP_DEBUG
  QMP_info ("Starting QMP_wait_all, id=%d\n",QMP_get_node_number());
#endif

  for(i=0; i<num; i++) {
    /* if(!QMP_is_complete(msgh[i])) { */
    err = QMP_wait_send_receive(msgh[i]);
    if(err!=QMP_SUCCESS) {
      QMP_FATAL("some error in QMP_wait_send_receive");
      break;
    }
    /*}*/
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

  rbuffer = malloc(count);
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
  free(rbuffer);

  /* free binary operator */
  /* MPI_Op_free (&bop); */

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
