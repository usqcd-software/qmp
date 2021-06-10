#ifndef _QMP_P_MPI_H
#define _QMP_P_MPI_H

#include <mpi.h>

#define TAG_CHANNEL  11

// machine specific datatypes

#define MM_TYPES MPI_Datatype mpi_type;

#define MH_TYPES MH_TYPES_MPI
#define MH_TYPES_MPI MPI_Request request, *request_array;

#define COMM_TYPES MPI_Comm mpicomm;
#define COMM_TYPES_INIT ,MPI_COMM_NULL

// machine specific routines

#define QMP_INIT_MACHINE QMP_INIT_MACHINE_MPI
#define QMP_FINALIZE_MSG_PASSING QMP_FINALIZE_MSG_PASSING_MPI
#define QMP_ABORT QMP_ABORT_MPI
#define QMP_COMM_SPLIT QMP_COMM_SPLIT_MPI
#define QMP_GET_HIDDEN_COMM QMP_GET_MPI_COMM
#define QMP_COMM_FREE QMP_COMM_FREE_MPI
#define QMP_SET_TOPO QMP_SET_TOPO_MPI
#define QMP_COMM_GET_LOGICAL_COORDINATES_FROM QMP_COMM_GET_LOGICAL_COORDINATES_FROM_MPI
#define QMP_COMM_GET_NODE_NUMBER_FROM QMP_COMM_GET_NODE_NUMBER_FROM_MPI
#define QMP_ERROR_STRING QMP_ERROR_STRING_MPI
#define QMP_DECLARE_MSGMEM QMP_DECLARE_MSGMEM_MPI
#define QMP_FREE_MSGMEM QMP_FREE_MSGMEM_MPI
#define QMP_ALLOC_MSGHANDLE QMP_ALLOC_MSGHANDLE_MPI
#define QMP_FREE_MSGHANDLE QMP_FREE_MSGHANDLE_MPI
#define QMP_DECLARE_RECEIVE QMP_DECLARE_RECEIVE_MPI
#define QMP_DECLARE_SEND QMP_DECLARE_SEND_MPI
#define QMP_DECLARE_MULTIPLE QMP_DECLARE_MULTIPLE_MPI
#define QMP_CHANGE_ADDRESS QMP_CHANGE_ADDRESS_MPI
#define QMP_START QMP_START_MPI
#define QMP_IS_COMPLETE QMP_IS_COMPLETE_MPI
#define QMP_WAIT QMP_WAIT_MPI
#define QMP_COMM_BARRIER QMP_COMM_BARRIER_MPI
#define QMP_COMM_BROADCAST QMP_COMM_BROADCAST_MPI
#define QMP_COMM_SUM_DOUBLE QMP_COMM_SUM_DOUBLE_MPI
#define QMP_COMM_SUM_UINT64_T QMP_COMM_SUM_UINT64_T_MPI
#define QMP_COMM_SUM_LONG_DOUBLE QMP_COMM_SUM_LONG_DOUBLE_MPI
#define QMP_COMM_SUM_FLOAT_ARRAY QMP_COMM_SUM_FLOAT_ARRAY_MPI
#define QMP_COMM_SUM_DOUBLE_ARRAY QMP_COMM_SUM_DOUBLE_ARRAY_MPI
#define QMP_COMM_SUM_LONG_DOUBLE_ARRAY QMP_COMM_SUM_LONG_DOUBLE_ARRAY_MPI
#define QMP_COMM_MAX_DOUBLE QMP_COMM_MAX_DOUBLE_MPI
#define QMP_COMM_MIN_DOUBLE QMP_COMM_MIN_DOUBLE_MPI
#define QMP_COMM_XOR_ULONG QMP_COMM_XOR_ULONG_MPI
#define QMP_COMM_ALLTOALL QMP_COMM_ALLTOALL_MPI
#define QMP_COMM_BINARY_REDUCTION QMP_COMM_BINARY_REDUCTION_MPI

#define QMP_TIME MPI_Wtime

#define QMP_INIT_MACHINE_MPI QMP_init_machine_mpi
extern QMP_status_t QMP_init_machine_mpi (int* argc, char*** argv,
					  QMP_thread_level_t required,
					  QMP_thread_level_t *provided);

#define QMP_FINALIZE_MSG_PASSING_MPI QMP_finalize_msg_passing_mpi
void QMP_finalize_msg_passing_mpi (void);

#define QMP_ABORT_MPI QMP_abort_mpi
void QMP_abort_mpi (int error_code);

#define QMP_COMM_SPLIT_MPI QMP_comm_split_mpi
QMP_status_t QMP_comm_split_mpi(QMP_comm_t comm, QMP_comm_t newcomm);

#define QMP_GET_MPI_COMM QMP_get_mpi_comm
QMP_status_t QMP_get_mpi_comm(QMP_comm_t comm, void** mpicomm);

#define QMP_COMM_FREE_MPI QMP_comm_free_mpi
QMP_status_t QMP_comm_free_mpi(QMP_comm_t comm);

#define QMP_SET_TOPO_MPI QMP_set_topo_mpi
QMP_status_t QMP_set_topo_mpi(QMP_comm_t comm);

#define QMP_COMM_GET_LOGICAL_COORDINATES_FROM_MPI QMP_comm_get_logical_coordinates_from_mpi
void QMP_comm_get_logical_coordinates_from_mpi(int *c, int nd, QMP_comm_t comm, int node);

#define QMP_COMM_GET_NODE_NUMBER_FROM_MPI QMP_comm_get_node_number_from_mpi
int QMP_comm_get_node_number_from_mpi(QMP_comm_t comm, const int* coords);

#define QMP_ERROR_STRING_MPI QMP_error_string_mpi
const char* QMP_error_string_mpi(QMP_status_t code);

#define QMP_DECLARE_MSGMEM_MPI QMP_declare_msgmem_mpi
void QMP_declare_msgmem_mpi(QMP_msgmem_t mem);

#define QMP_FREE_MSGMEM_MPI QMP_free_msgmem_mpi
void QMP_free_msgmem_mpi(QMP_msgmem_t mm);

#define QMP_ALLOC_MSGHANDLE_MPI QMP_alloc_msghandle_mpi
void QMP_alloc_msghandle_mpi(QMP_msghandle_t mh);

#define QMP_FREE_MSGHANDLE_MPI QMP_free_msghandle_mpi
void QMP_free_msghandle_mpi(QMP_msghandle_t mh);

#define QMP_DECLARE_RECEIVE_MPI QMP_declare_receive_mpi
void QMP_declare_receive_mpi(QMP_msghandle_t mh);

#define QMP_DECLARE_SEND_MPI QMP_declare_send_mpi
void QMP_declare_send_mpi(QMP_msghandle_t mh);

#define QMP_DECLARE_MULTIPLE_MPI QMP_declare_multiple_mpi
void QMP_declare_multiple_mpi(QMP_msghandle_t mh);

#define QMP_CHANGE_ADDRESS_MPI QMP_change_address_mpi
void QMP_change_address_mpi(QMP_msghandle_t mh);

#define QMP_START_MPI QMP_start_mpi
QMP_status_t QMP_start_mpi(QMP_msghandle_t mh);

#define QMP_IS_COMPLETE_MPI QMP_is_complete_mpi
QMP_bool_t QMP_is_complete_mpi(QMP_msghandle_t mh);

#define QMP_WAIT_MPI QMP_wait_mpi
QMP_status_t QMP_wait_mpi(QMP_msghandle_t mh);

#define QMP_COMM_BARRIER_MPI QMP_comm_barrier_mpi
QMP_status_t QMP_comm_barrier_mpi(QMP_comm_t comm);

#define QMP_COMM_BROADCAST_MPI QMP_comm_broadcast_mpi
QMP_status_t QMP_comm_broadcast_mpi(QMP_comm_t comm, void *send_buf, size_t count);

#define QMP_COMM_SUM_DOUBLE_MPI QMP_comm_sum_double_mpi
QMP_status_t QMP_comm_sum_double_mpi(QMP_comm_t comm, double *value);

#define QMP_COMM_SUM_UINT64_T_MPI QMP_comm_sum_uint64_t_mpi
QMP_status_t QMP_comm_sum_uint64_t_mpi(QMP_comm_t comm, uint64_t *value);

#define QMP_COMM_SUM_LONG_DOUBLE_MPI QMP_comm_sum_long_double_mpi
QMP_status_t QMP_comm_sum_long_double_mpi(QMP_comm_t comm, long double *value);

#define QMP_COMM_SUM_FLOAT_ARRAY_MPI QMP_comm_sum_float_array_mpi
QMP_status_t QMP_comm_sum_float_array_mpi(QMP_comm_t comm, float value[], int count);

#define QMP_COMM_SUM_DOUBLE_ARRAY_MPI QMP_comm_sum_double_array_mpi
QMP_status_t QMP_comm_sum_double_array_mpi(QMP_comm_t comm, double value[], int count);

#define QMP_COMM_SUM_LONG_DOUBLE_ARRAY_MPI QMP_comm_sum_long_double_array_mpi
QMP_status_t QMP_comm_sum_long_double_array_mpi(QMP_comm_t comm, long double value[], int count);

#define QMP_COMM_MAX_DOUBLE_MPI QMP_comm_max_double_mpi
QMP_status_t QMP_comm_max_double_mpi(QMP_comm_t comm, double *value);

#define QMP_COMM_MIN_DOUBLE_MPI QMP_comm_min_double_mpi
QMP_status_t QMP_comm_min_double_mpi(QMP_comm_t comm, double *value);

#define QMP_COMM_XOR_ULONG_MPI QMP_comm_xor_ulong_mpi
QMP_status_t QMP_comm_xor_ulong_mpi(QMP_comm_t comm, unsigned long *value);

#define QMP_COMM_ALLTOALL_MPI QMP_comm_alltoall_mpi
QMP_status_t QMP_comm_alltoall_mpi(QMP_comm_t comm, char* recvbuffer, char* sendbuffer, int count);

#define QMP_COMM_BINARY_REDUCTION_MPI QMP_comm_binary_reduction_mpi
QMP_status_t QMP_comm_binary_reduction_mpi(QMP_comm_t comm, void *lbuffer, size_t count, QMP_binary_func bfunc);

#endif /* _QMP_P_MPI_H */
