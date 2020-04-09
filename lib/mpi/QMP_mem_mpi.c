#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "QMP_P_COMMON.h"


void
QMP_declare_msgmem_mpi(QMP_msgmem_t mem)
{
  switch(mem->type) {

  case MM_user_buf: {
    mem->mpi_type = MPI_BYTE;
  } break;

  case MM_strided_buf: {
    int err;
    err = MPI_Type_vector(mem->mm.st.nblocks, mem->mm.st.blksize, mem->mm.st.stride,
			  MPI_BYTE, &(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
    err = MPI_Type_commit(&(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
  } break;

  case MM_strided_array_buf: {
    int err, i, *ones, n = mem->mm.sa.narray;
    MPI_Aint *disp;
    MPI_Datatype *dt;
    QMP_alloc(disp, MPI_Aint, n);
    QMP_alloc(dt, MPI_Datatype, n);
    QMP_alloc(ones, int, n);
    for(i=0; i<n; i++) {
      ones[i] = 1;
      disp[i] = mem->mm.sa.disp[i];
      err = MPI_Type_vector(mem->mm.sa.nblocks[i], mem->mm.sa.blksize[i],
			    mem->mm.sa.stride[i], MPI_BYTE, &dt[i]);
      QMP_assert(err==MPI_SUCCESS);
    }
    err = MPI_Type_create_struct(n, ones, disp, dt, &(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
    err = MPI_Type_commit(&(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
    for(i=0; i<n; i++) {
      err = MPI_Type_free(&dt[i]);
      QMP_assert(err==MPI_SUCCESS);
    }
    QMP_free(dt);
    QMP_free(disp);
    QMP_free(ones);
  } break;

  case MM_indexed_buf: {
    int err, freedt=0, n = mem->mm.in.count;
    MPI_Datatype dt;
    switch(mem->mm.in.elemsize) {
    case sizeof(char): dt=MPI_BYTE; break;
    case sizeof(float): dt=MPI_FLOAT; break;
    case sizeof(double): dt=MPI_DOUBLE; break;
    default:
      err = MPI_Type_contiguous(mem->mm.in.elemsize, MPI_BYTE, &dt);
      QMP_assert(err==MPI_SUCCESS);
      freedt = 1;
    }
    err = MPI_Type_indexed(n,mem->mm.in.blocklen,mem->mm.in.index,dt,&(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
    err = MPI_Type_commit(&(mem->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
    if(freedt) {
      err = MPI_Type_free(&dt);
      QMP_assert(err==MPI_SUCCESS);
    }
  } break;

  }
}


void
QMP_free_msgmem_mpi(QMP_msgmem_t mm)
{
  if ( (mm->type == MM_strided_buf) ||
       (mm->type == MM_strided_array_buf) ||
       (mm->type == MM_indexed_buf) ) {
    int err = MPI_Type_free(&(mm->mpi_type));
    QMP_assert(err==MPI_SUCCESS);
  }
}


void
QMP_alloc_msghandle_mpi(QMP_msghandle_t mh)
{
  mh->request = MPI_REQUEST_NULL;
}


void
QMP_free_msghandle_mpi(QMP_msghandle_t mh)
{
  if(mh->type==MH_multiple) {
    QMP_free(mh->request_array);
  } else {
    int err = MPI_Request_free(&mh->request);
    QMP_assert(err==MPI_SUCCESS);
  }
}


void
QMP_declare_receive_mpi(QMP_msghandle_t mh)
{
  int tag = TAG_CHANNEL;
/* change MPI tags for relative send/receive in different directions, protecting against having only 2 nodes in 1 direction */
  if (mh->axis >=0){
    if (mh->dir >0) tag ++;
    else        tag --;
  }
  QMP_assert (tag>=0);
  if(mh->mm->type==MM_user_buf) {
    MPI_Recv_init(mh->base, mh->mm->nbytes,
		  MPI_BYTE, mh->srce_node, tag,
		  mh->comm->mpicomm, &mh->request);
  } else {
    MPI_Recv_init(mh->base, 1,
		  mh->mm->mpi_type,
		  mh->srce_node, tag,
		  mh->comm->mpicomm, &mh->request);
  }
}


void
QMP_declare_send_mpi(QMP_msghandle_t mh)
{
  int tag = TAG_CHANNEL;
/* change MPI tags for relative send/receive in different directions, protecting against having only 2 nodes in 1 direction */
  if (mh->axis >=0){
    if (mh->dir >0) tag --; /* should be reversed from receive tag */
    else        tag ++;
  }
  QMP_assert (tag>=0);
  if(mh->mm->type==MM_user_buf) {
    MPI_Send_init(mh->base, mh->mm->nbytes,
		  MPI_BYTE, mh->dest_node, tag,
		  mh->comm->mpicomm, &mh->request);
  } else {
    MPI_Send_init(mh->base, 1,
		  mh->mm->mpi_type,
		  mh->dest_node,
		  tag,
		  mh->comm->mpicomm,
		  &mh->request);
  }
}


void
QMP_declare_multiple_mpi(QMP_msghandle_t mh)
{
  QMP_alloc(mh->request_array, MPI_Request, mh->num);

  QMP_msghandle_t mhc = mh->next;
  int i=0;
  while(mhc) {
    mh->request_array[i] = mhc->request;
    i++;
    mhc = mhc->next;
  }
}


void
QMP_change_address_mpi(QMP_msghandle_t mh)
{
  if(mh->type==MH_multiple) {
    mh = mh->next;
  }
  while(mh) {
    QMP_free_msghandle_mpi(mh);
    if(mh->type==MH_send) QMP_declare_send_mpi(mh);
    else QMP_declare_receive_mpi(mh);
    mh = mh->next;
  }
}
