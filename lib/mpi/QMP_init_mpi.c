#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"


QMP_status_t
QMP_init_machine_mpi (int* argc, char*** argv, QMP_thread_level_t required,
		      QMP_thread_level_t *provided)
{
#if 1

  int mpi_req=-1, mpi_prv;
  switch ( required ) { 
  case QMP_THREAD_SINGLE :
    mpi_req = MPI_THREAD_SINGLE;
    break;
  case QMP_THREAD_FUNNELED : 
    mpi_req = MPI_THREAD_FUNNELED;
    break;
  case QMP_THREAD_SERIALIZED :
    mpi_req = MPI_THREAD_SERIALIZED;
    break;
  case QMP_THREAD_MULTIPLE:
    mpi_req = MPI_THREAD_MULTIPLE;
    break;
  default:
    QMP_abort_string(-1, "Invalid value for required QMP thread level");
    break;
  }

  int flag;
  MPI_Initialized(&flag); // needed to coexist with other libs apparently
    if ( !flag ) {
      if (MPI_Init_thread(argc, argv, mpi_req, &mpi_prv) != MPI_SUCCESS) 
	QMP_abort_string (-1, "MPI_Init failed");
    }else{
      MPI_Query_thread(&mpi_prv);
    }

  switch(mpi_prv) { 
  case MPI_THREAD_SINGLE:
    *provided = QMP_THREAD_SINGLE;
    break;
  case MPI_THREAD_FUNNELED:
    *provided = QMP_THREAD_FUNNELED;
    break;
  case MPI_THREAD_SERIALIZED:
    *provided = QMP_THREAD_SERIALIZED;
    break;
  case MPI_THREAD_MULTIPLE:
    *provided = QMP_THREAD_MULTIPLE;
    break;
  default:
    QMP_abort_string(-1, "MPI_Init returned unknown thread safety level");
    break;
  }
#endif

#if 0
  // Old style of initialization
  *provided = QMP_THREAD_SINGLE;  /* just single for now */
  if (MPI_Init(argc, argv) != MPI_SUCCESS) 
    QMP_abort_string (-1, "MPI_Init failed");
#endif

  MPI_Comm *mcomm = &QMP_allocated_comm->mpicomm;
  if (MPI_Comm_dup(MPI_COMM_WORLD, mcomm) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_dup failed");

  int PAR_num_nodes, PAR_node_rank;
  if (MPI_Comm_size(*mcomm, &PAR_num_nodes) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_size failed");
  if (MPI_Comm_rank(*mcomm, &PAR_node_rank) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_rank failed");

  QMP_allocated_comm->num_nodes = PAR_num_nodes;
  QMP_allocated_comm->nodeid = PAR_node_rank;
  QMP_allocated_comm->ncolors = 1;
  QMP_allocated_comm->color = 0;
  QMP_allocated_comm->key = PAR_node_rank;

  QMP_alloc(QMP_machine->host, char, MPI_MAX_PROCESSOR_NAME);
  MPI_Get_processor_name(QMP_machine->host, &QMP_machine->hostlen);

  return QMP_SUCCESS;
}


void
QMP_finalize_msg_passing_mpi (void)
{
  int flag;
  MPI_Finalized(&flag);

  if (!flag) {
    MPI_Finalize();
  }
}


void 
QMP_abort_mpi (int error_code)
{
  MPI_Abort(MPI_COMM_WORLD, error_code);
}
