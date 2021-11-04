#include "QMP_P_COMMON.h"

QMP_status_t
QMP_comm_split_mpi(QMP_comm_t comm, QMP_comm_t newcomm)
{
  QMP_status_t status = QMP_SUCCESS;

  int err;
  err = MPI_Comm_split(comm->mpicomm, newcomm->color, newcomm->key, &newcomm->mpicomm);
  QMP_assert(err==MPI_SUCCESS);

  int PAR_num_nodes, PAR_node_rank;
  if (MPI_Comm_size(newcomm->mpicomm, &PAR_num_nodes) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_size failed");
  if (MPI_Comm_rank(newcomm->mpicomm, &PAR_node_rank) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_rank failed");

  newcomm->num_nodes = PAR_num_nodes;
  newcomm->nodeid = PAR_node_rank;

  return status;
}

QMP_status_t
QMP_comm_free_mpi(QMP_comm_t comm)
{
  QMP_status_t status = QMP_SUCCESS;

  int err = MPI_Comm_free(&comm->mpicomm);
  if(err!=MPI_SUCCESS) status = (QMP_status_t)err;

  return status;
}
