#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "QMP_P_COMMON.h"


QMP_status_t
QMP_start(QMP_msghandle_t mh)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

  QMP_assert(mh!=NULL);
  QMP_assert((mh->type==MH_send)||(mh->type==MH_recv)||(mh->type==MH_multiple));
  QMP_assert(mh->activeP==0);
  mh->activeP = 1;
  mh->uses++;
#ifdef QMP_START
  QMP_START(mh);
#endif

  LEAVE;
  return err;
}

QMP_bool_t
QMP_is_complete(QMP_msghandle_t mh)
{
  QMP_bool_t done = QMP_TRUE;
  ENTER;

  QMP_assert(mh!=NULL);
  QMP_assert((mh->type==MH_send)||(mh->type==MH_recv)||(mh->type==MH_multiple));
  if(mh->activeP) {
#ifdef QMP_IS_COMPLETE
    done = QMP_IS_COMPLETE(mh);
#endif
    if(done) mh->activeP = 0;
  }

  LEAVE;
  return done;
}

QMP_status_t
QMP_wait(QMP_msghandle_t mh)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

  QMP_assert(mh!=NULL);
  QMP_assert((mh->type==MH_send)||(mh->type==MH_recv)||(mh->type==MH_multiple));
  if(mh->activeP) {
#ifdef QMP_WAIT
    err = QMP_WAIT(mh);
#endif
    if(err==QMP_SUCCESS) mh->activeP = 0;
  }

  LEAVE;
  return err;
}

QMP_status_t
QMP_wait_all(QMP_msghandle_t mh[], int num)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_WAIT_ALL
  int i;
  for(i=0; i<num; i++) {
    QMP_assert(mh[i]!=NULL);
    QMP_assert((mh[i]->type==MH_send)||(mh[i]->type==MH_recv)||(mh[i]->type==MH_multiple));
  }
  err = QMP_WAIT_ALL(mh, num);
  if(err==QMP_SUCCESS) mh->activeP = 0;
#else
  int i;
  for(i=0; i<num; i++) {
    QMP_status_t err2 = QMP_wait(mh[i]);
    if(err2!=QMP_SUCCESS) err = err2;
  }
#endif

  LEAVE;
  return err;
}


/* Global barrier */
QMP_status_t
QMP_comm_barrier(QMP_communicator_t comm)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_BARRIER
  status = QMP_COMM_BARRIER(comm);
#endif

  LEAVE;
  return status;
}

QMP_status_t
QMP_barrier(void)
{
  QMP_status_t status;
  ENTER;

  status = QMP_comm_barrier(QMP_comm_get_default());

  LEAVE;
  return status;
}


/* Broadcast via interface specific routines */
QMP_status_t
QMP_comm_broadcast(QMP_communicator_t comm, void *send_buf, size_t count)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_BROADCAST
  err = QMP_COMM_BROADCAST(comm, send_buf, count);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_broadcast(void *send_buf, size_t count)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_broadcast(QMP_comm_get_default(), send_buf, count);

  LEAVE;
  return err;
}


/* Global sums */
QMP_status_t
QMP_comm_sum_int (QMP_communicator_t comm, int *value)
{
  QMP_status_t err;
  ENTER;

  double x = (double) *value;
  err = QMP_comm_sum_double(comm, &x);
  *value = (int) x;

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_int (int *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_int(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_sum_float (QMP_communicator_t comm, float *value)
{
  QMP_status_t err;
  ENTER;

  double x = (double) *value;
  err = QMP_comm_sum_double(comm, &x);
  *value = (float) x;

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_float (float *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_float(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_sum_double (QMP_communicator_t comm, double *value)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_SUM_DOUBLE
  err = QMP_COMM_SUM_DOUBLE(comm, value);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_double (double *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_double(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_sum_double_extended (QMP_communicator_t comm, double *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_double(comm, value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_double_extended (double *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_double(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_sum_float_array (QMP_communicator_t comm, float value[], int count)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_SUM_FLOAT_ARRAY
  err = QMP_COMM_SUM_FLOAT_ARRAY(comm, value, count);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_float_array (float value[], int count)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_float_array(QMP_comm_get_default(), value, count);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_sum_double_array (QMP_communicator_t comm, double value[], int count)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_SUM_DOUBLE_ARRAY
  err = QMP_COMM_SUM_DOUBLE_ARRAY(comm, value, count);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_sum_double_array (double value[], int count)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_sum_double_array(QMP_comm_get_default(), value, count);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_max_float (QMP_communicator_t comm, float *value)
{
  QMP_status_t err;
  ENTER;

  double x = (double) *value;
  err = QMP_comm_max_double(comm, &x);
  *value = (float) x;

  LEAVE;
  return err;
}

QMP_status_t
QMP_max_float (float *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_max_float(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_min_float (QMP_communicator_t comm, float *value)
{
  QMP_status_t err;
  ENTER;

  double x = (double) *value;
  err = QMP_comm_min_double(comm, &x);
  *value = (float) x;

  LEAVE;
  return err;
}

QMP_status_t
QMP_min_float (float *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_min_float(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_max_double (QMP_communicator_t comm, double *value)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_MAX_DOUBLE
  err = QMP_COMM_MAX_DOUBLE(comm, value);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_max_double (double *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_max_double(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_min_double (QMP_communicator_t comm, double *value)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_MIN_DOUBLE
  err = QMP_COMM_MIN_DOUBLE(comm, value);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_min_double (double *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_min_double(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_xor_ulong (QMP_communicator_t comm, unsigned long *value)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_XOR_ULONG
  err = QMP_COMM_XOR_ULONG(comm, value);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_xor_ulong (unsigned long *value)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_xor_ulong(QMP_comm_get_default(), value);

  LEAVE;
  return err;
}

QMP_status_t
QMP_comm_binary_reduction (QMP_communicator_t comm, void *lbuffer, size_t count, QMP_binary_func bfunc)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

  QMP_assert(bfunc!=NULL);

#ifdef QMP_COMM_BINARY_REDUCTION
  err = QMP_COMM_BINARY_REDUCTION(comm, lbuffer, count, bfunc);
#endif

  LEAVE;
  return err;
}

QMP_status_t
QMP_binary_reduction (void *lbuffer, size_t count, QMP_binary_func bfunc)
{
  QMP_status_t err;
  ENTER;

  err = QMP_comm_binary_reduction(QMP_comm_get_default(), lbuffer, count, bfunc);

  LEAVE;
  return err;
}
