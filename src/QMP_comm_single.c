/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP communication code for single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_COMMON.h"
#include "QMP_P_SINGLE.h"


QMP_status_t
QMP_start(QMP_msghandle_t msgh)
{
  QMP_error("QMP_start: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  return QMP_ERROR;
}

QMP_bool_t
QMP_is_complete(QMP_msghandle_t msgh)
{
  QMP_error("QMP_start: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  return QMP_FALSE;
}

QMP_status_t
QMP_wait(QMP_msghandle_t msgh)
{
  QMP_error("QMP_start: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  return QMP_ERROR;
}

QMP_status_t
QMP_wait_all(QMP_msghandle_t msgh[], int num)
{
  QMP_error("QMP_start: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  return QMP_ERROR;
}


/* Global barrier */
QMP_status_t
QMP_barrier(void)
{
  return QMP_SUCCESS;
}

/* Broadcast via interface specific routines */
QMP_status_t
QMP_broadcast(void *send_buf, size_t count)
{
  return QMP_SUCCESS;
}

/* Global sums */
QMP_status_t
QMP_sum_int (int *value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_sum_float(float *value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_sum_double (double *value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_sum_double_extended (double *value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_sum_float_array (float value[], int count)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_sum_double_array (double value[], int count)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_max_float(float* value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_min_float(float* value)
{
  return QMP_SUCCESS;
}  

QMP_status_t
QMP_max_double(double* value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_min_double(double *value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_xor_ulong(unsigned long* value)
{
  return QMP_SUCCESS;
}

QMP_status_t
QMP_binary_reduction (void *lbuffer, size_t count, QMP_binary_func bfunc)
{
  return QMP_SUCCESS;
}
