#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "QMP_P_COMMON.h"


QMP_status_t
QMP_start_mpi (QMP_msghandle_t mh)
{
  QMP_status_t err = QMP_SUCCESS;

  if(mh->type==MH_multiple) {
    MPI_Startall(mh->num, mh->request_array);
  } else {
    MPI_Start(&mh->request);
  }

  return err;
}


QMP_bool_t
QMP_is_complete_mpi (QMP_msghandle_t mh)
{
  QMP_bool_t done = QMP_FALSE;

  if(mh->type==MH_multiple) {
    int flag, callst;
    /* MPI_Status status[mh->num]; */
    MPI_Status *status;  QMP_alloc(status, MPI_Status, mh->num);
    callst = MPI_Testall(mh->num, mh->request_array, &flag, status);
    if (callst != MPI_SUCCESS) {
      QMP_fprintf (stderr, "Testall return value is %d\n", callst);
      QMP_FATAL("test unexpectedly failed");
    }
    if(flag) done = QMP_TRUE;
    QMP_free(status);
  } else {
    int flag, callst;
    MPI_Status status;
    callst = MPI_Test(&mh->request, &flag, &status);
    if (callst != MPI_SUCCESS) {
      QMP_fprintf (stderr, "Test return value is %d\n", callst);
      QMP_FATAL("test unexpectedly failed");
    }
    if(flag) done = QMP_TRUE;
  }

  return done;
}


QMP_status_t
QMP_wait_mpi(QMP_msghandle_t mh)
{
  QMP_status_t status = QMP_SUCCESS;

  int flag;
  if(mh->type==MH_multiple) {
    /* MPI_Status status[mh->num]; */
    MPI_Status *status;  QMP_alloc(status, MPI_Status, mh->num);
    flag = MPI_Waitall(mh->num, mh->request_array, status);
    if (flag != MPI_SUCCESS) {
      QMP_fprintf (stderr, "Wait all Flag is %d\n", flag);
      QMP_FATAL("test unexpectedly failed");
    }
    QMP_free(status);
  } else {
    MPI_Status status;
    flag = MPI_Wait(&mh->request, &status);
    if (flag != MPI_SUCCESS) {
      QMP_fprintf (stderr, "Wait all Flag is %d\n", flag);
      QMP_FATAL("test unexpectedly failed");
    }
  }
  if (flag != MPI_SUCCESS) status = (QMP_status_t)flag;

  return status;
}

QMP_status_t 
QMP_get_mpi_comm(QMP_comm_t comm, void** mpicm){
  QMP_status_t status = QMP_SUCCESS;
  *mpicm=(void*)&(comm->mpicomm);

  return status;
}

QMP_status_t
QMP_comm_barrier_mpi(QMP_comm_t comm)
{
  QMP_status_t status = QMP_SUCCESS;

  int err = MPI_Barrier(comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;

  return status;
}

QMP_status_t
QMP_comm_broadcast_mpi(QMP_comm_t comm, void *send_buf, size_t count)
{
  QMP_status_t status = QMP_SUCCESS;

  int err = MPI_Bcast(send_buf, count, MPI_BYTE, 0, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;

  return status;
}


QMP_status_t
QMP_comm_sum_double_mpi(QMP_comm_t comm, double *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  double dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
			  MPI_DOUBLE, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}

QMP_status_t
QMP_comm_sum_uint64_t_mpi(QMP_comm_t comm, uint64_t *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  uint64_t dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
                          MPI_UINT64_T, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}

QMP_status_t
QMP_comm_sum_long_double_mpi(QMP_comm_t comm, long double *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  long double dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
			  MPI_LONG_DOUBLE, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_sum_float_array_mpi(QMP_comm_t comm, float value[], int count)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  int err = MPI_Allreduce(MPI_IN_PLACE, (void *)value, count,
			  MPI_FLOAT, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  
  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_sum_double_array_mpi(QMP_comm_t comm, double value[], int count)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  int err = MPI_Allreduce(MPI_IN_PLACE,(void *)value, count,
			  MPI_DOUBLE, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  
  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_sum_long_double_array_mpi(QMP_comm_t comm, long double value[], int count)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  long double *dest;
  QMP_alloc(dest, long double, count);
  int err = MPI_Allreduce((void *)value, (void *)dest, count,
			  MPI_DOUBLE, MPI_SUM, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else {
    int i;
    for (i = 0; i < count; i++)
      value[i] = dest[i];
  }
  QMP_free(dest);
  
  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_max_double_mpi(QMP_comm_t comm, double *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  double dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
			  MPI_DOUBLE, MPI_MAX, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_min_double_mpi(QMP_comm_t comm, double *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  double dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
			  MPI_DOUBLE, MPI_MIN, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}


QMP_status_t
QMP_comm_xor_ulong_mpi(QMP_comm_t comm, unsigned long *value)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  unsigned long dest;
  int err = MPI_Allreduce((void *)value, (void *)&dest, 1,
			  MPI_UNSIGNED_LONG, MPI_BXOR, comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else *value = dest;

  LEAVE;
  return status;
}

//does global transposition from sendbuffer to recvbuffer of count data (in BYTE). be careful with 2GB limits
QMP_status_t
QMP_comm_alltoall_mpi(QMP_comm_t comm, char* recvbuffer, char* sendbuffer, int count)
{
  QMP_status_t status=QMP_SUCCESS;
  ENTER;

  int err=MPI_Alltoall( (void*)sendbuffer, count, MPI_BYTE,
			(void*)recvbuffer, count, MPI_BYTE,
			comm->mpicomm);
  if(err != MPI_SUCCESS) status = (QMP_status_t)err;

  LEAVE;
  return status;
}

/**
 * This is a pure hack since our user binary function is defined
 * differently from MPI spec. In addition we are not expecting
 * multiple binary reduction can be carried out simultaneously.
 */
static QMP_binary_func qmp_user_bfunc = NULL;
static MPI_Op bop;
static int op_inited=0;

/**
 * This is a internal function which will be passed to MPI all reduce
 */
static void
qmp_bfunc_mpi(void* in, void* inout, int* len, MPI_Datatype* type)
{
 _QMP_UNUSED_ARGUMENT(len);
 _QMP_UNUSED_ARGUMENT(type);
  qmp_user_bfunc(inout, in);
}

QMP_status_t
QMP_comm_binary_reduction_mpi(QMP_comm_t comm, void *lbuffer, size_t count, QMP_binary_func bfunc)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  QMP_assert(qmp_user_bfunc==NULL);

  /* set up user binary reduction pointer */
  qmp_user_bfunc = bfunc;

  int err;
  if(!op_inited) {
    err = MPI_Op_create(qmp_bfunc_mpi, 1, &bop);
    if (err != MPI_SUCCESS) {
      QMP_error ("Cannot create MPI operator for binary reduction.\n");
      goto leave;
    }
    op_inited = 1;
  }

  char *rbuffer;
  QMP_alloc(rbuffer, char, count);

  err = MPI_Allreduce(lbuffer,rbuffer,count, MPI_BYTE, bop, comm->mpicomm);

  if(err != MPI_SUCCESS) status = (QMP_status_t)err;
  else {
    memcpy (lbuffer, rbuffer, count);
    QMP_free(rbuffer);
  }

  /* signal end of the binary reduction session */
  qmp_user_bfunc = NULL;

  return status;
 leave:
  LEAVE;
  return QMP_SUCCESS;
}
