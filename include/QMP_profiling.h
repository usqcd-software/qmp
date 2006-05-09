#ifdef QMP_BUILD_PROFILING

#ifndef __QMP_PROFILING_H
#define __QMP_PROFILING_H
#include "qmp.h" 
QMP_status_t       QMP_init_msg_passing (int* argc, char*** argv,
						QMP_thread_level_t required,
						QMP_thread_level_t *provided)
 __attribute__ ((weak, alias ("PQMP_init_msg_passing")));

 QMP_bool_t         QMP_is_initialized(void)
 __attribute__ ((weak, alias ("PQMP_is_initialized")));

 void               QMP_finalize_msg_passing (void)
 __attribute__ ((weak, alias ("PQMP_finalize_msg_passing")));

 void               QMP_abort (int error_code)
 __attribute__ ((weak, alias ("PQMP_abort")));

 void               QMP_abort_string (int error_code, char *message)
 __attribute__ ((weak, alias ("PQMP_abort_string")));



 QMP_ictype_t       QMP_get_msg_passing_type (void)
 __attribute__ ((weak, alias ("PQMP_get_msg_passing_type")));

 int                QMP_get_number_of_nodes (void)
 __attribute__ ((weak, alias ("PQMP_get_number_of_nodes")));

 int                QMP_get_node_number (void)
 __attribute__ ((weak, alias ("PQMP_get_node_number")));

 QMP_bool_t         QMP_is_primary_node (void)
 __attribute__ ((weak, alias ("PQMP_is_primary_node")));

 int                QMP_get_allocated_number_of_dimensions (void)
 __attribute__ ((weak, alias ("PQMP_get_allocated_number_of_dimensions")));

 const int*         QMP_get_allocated_dimensions (void)
 __attribute__ ((weak, alias ("PQMP_get_allocated_dimensions")));

 const int*         QMP_get_allocated_coordinates (void)
 __attribute__ ((weak, alias ("PQMP_get_allocated_coordinates")));




 int                QMP_io_node(int node)
 __attribute__ ((weak, alias ("PQMP_io_node")));

 int                QMP_master_io_node(void)
 __attribute__ ((weak, alias ("PQMP_master_io_node")));



 QMP_status_t       QMP_declare_logical_topology (const int dims[],
							int ndim)
 __attribute__ ((weak, alias ("PQMP_declare_logical_topology")));

 QMP_bool_t         QMP_logical_topology_is_declared (void)
 __attribute__ ((weak, alias ("PQMP_logical_topology_is_declared ")));

 int                QMP_get_logical_number_of_dimensions (void)
 __attribute__ ((weak, alias ("PQMP_get_logical_number_of_dimensions")));

 const int*         QMP_get_logical_dimensions (void)
 __attribute__ ((weak, alias ("PQMP_get_logical_dimensions")));

 const int*         QMP_get_logical_coordinates (void)
 __attribute__ ((weak, alias ("PQMP_get_logical_coordinates ")));

 int*               QMP_get_logical_coordinates_from (int node)
 __attribute__ ((weak, alias ("PQMP_get_logical_coordinates_from")));

 int                QMP_get_node_number_from (const int coordinates[])
 __attribute__ ((weak, alias ("PQMP_get_node_number_from")));



 QMP_status_t       QMP_layout_grid (const int dimensions[], int ndims)
 __attribute__ ((weak, alias ("PQMP_layout_grid")));

 const int*         QMP_get_subgrid_dimensions (void)
 __attribute__ ((weak, alias ("PQMP_get_subgrid_dimensions")));

 int                QMP_get_number_of_subgrid_sites (void)
 __attribute__ ((weak, alias ("PQMP_get_number_of_subgrid_sites")));




 QMP_mem_t*         QMP_allocate_memory (size_t nbytes)
 __attribute__ ((weak, alias ("PQMP_allocate_memory")));

 QMP_mem_t*         QMP_allocate_aligned_memory (size_t nbytes,
						       size_t alignment,
						       int flags)
 __attribute__ ((weak, alias ("PQMP_allocate_aligned_memory")));

 void*              QMP_get_memory_pointer (QMP_mem_t* mem)
 __attribute__ ((weak, alias ("PQMP_get_memory_pointer")));

 void               QMP_free_memory (QMP_mem_t* mem)
 __attribute__ ((weak, alias ("PQMP_free_memory")));

 QMP_msgmem_t       QMP_declare_msgmem (const void* mem, size_t nbytes)
 __attribute__ ((weak, alias ("PQMP_declare_msgmem")));

 QMP_msgmem_t       QMP_declare_strided_msgmem (void* base, 
						      size_t blksize,
						      int nblocks,
						      ptrdiff_t stride)
 __attribute__ ((weak, alias ("PQMP_declare_strided_msgmem")));

 QMP_msgmem_t   QMP_declare_strided_array_msgmem (void* base[],
							size_t blksize[],
							int nblocks[],
							ptrdiff_t stride[],
							int num)
 __attribute__ ((weak, alias ("PQMP_declare_strided_array_msgmem")));

 void               QMP_free_msgmem (QMP_msgmem_t m)
 __attribute__ ((weak, alias ("PQMP_free_msgmem")));



 QMP_msghandle_t    QMP_declare_receive_relative (QMP_msgmem_t m, 
							int axis,
							int dir,
							int priority)
 __attribute__ ((weak, alias ("PQMP_declare_receive_relative")));

 QMP_msghandle_t    QMP_declare_send_relative    (QMP_msgmem_t m,
							int axis,
							int dir,
							int priority)
 __attribute__ ((weak, alias ("PQMP_declare_send_relative")));

 QMP_msghandle_t    QMP_declare_send_to     (QMP_msgmem_t m, 
						   int rem_node_rank,
						   int priority)
 __attribute__ ((weak, alias ("PQMP_declare_send_to")));

 QMP_msghandle_t    QMP_declare_receive_from(QMP_msgmem_t m, 
						   int rem_node_rank,
						   int priority)
 __attribute__ ((weak, alias ("PQMP_declare_receive_from")));

 void               QMP_free_msghandle (QMP_msghandle_t h)
 __attribute__ ((weak, alias ("PQMP_free_msghandle")));

 QMP_msghandle_t    QMP_declare_multiple (QMP_msghandle_t msgh[], 
						int num)
 __attribute__ ((weak, alias ("PQMP_declare_multiple")));

 QMP_status_t       QMP_start (QMP_msghandle_t h)
 __attribute__ ((weak, alias ("PQMP_start")));

 QMP_status_t       QMP_wait (QMP_msghandle_t h)
 __attribute__ ((weak, alias ("PQMP_wait")));

 QMP_status_t       QMP_wait_all (QMP_msghandle_t h[], int num)
 __attribute__ ((weak, alias ("PQMP_wait_all")));

 QMP_bool_t         QMP_is_complete (QMP_msghandle_t h)
 __attribute__ ((weak, alias ("PQMP_is_complete")));



 QMP_status_t       QMP_barrier (void)
 __attribute__ ((weak, alias ("PQMP_barrier")));

 QMP_status_t       QMP_broadcast (void* buffer, size_t nbytes)
 __attribute__ ((weak, alias ("PQMP_broadcast")));

 QMP_status_t       QMP_sum_int (int *value)
 __attribute__ ((weak, alias ("PQMP_sum_int")));

 QMP_status_t       QMP_sum_float (float *value)
 __attribute__ ((weak, alias ("PQMP_sum_float")));

 QMP_status_t       QMP_sum_double (double *value)
 __attribute__ ((weak, alias ("PQMP_sum_double")));

 QMP_status_t       QMP_sum_double_extended (double *value)
 __attribute__ ((weak, alias ("PQMP_sum_double_extended")));

 QMP_status_t       QMP_sum_float_array (float value[], int length)
 __attribute__ ((weak, alias ("PQMP_sum_float_array")));

 QMP_status_t       QMP_sum_double_array (double value[], int length)
 __attribute__ ((weak, alias ("PQMP_sum_double_array")));

 QMP_status_t       QMP_max_float (float* value)
 __attribute__ ((weak, alias ("PQMP_max_float")));

 QMP_status_t       QMP_max_double (double* value)
 __attribute__ ((weak, alias ("PQMP_max_double")));

 QMP_status_t       QMP_min_float (float* value)
 __attribute__ ((weak, alias ("PQMP_min_float")));

 QMP_status_t       QMP_min_double (double* value)
 __attribute__ ((weak, alias ("PQMP_min_double")));

 QMP_status_t       QMP_xor_ulong (unsigned long* value)
 __attribute__ ((weak, alias ("PQMP_xor_ulong")));

 QMP_status_t       QMP_binary_reduction (void* lbuffer, size_t buflen,
						QMP_binary_func bfunc)
 __attribute__ ((weak, alias ("PQMP_binary_reduction")));



 const char*        QMP_error_string (QMP_status_t code)
 __attribute__ ((weak, alias ("PQMP_error_string")));

 QMP_status_t       QMP_get_error_number (QMP_msghandle_t mh)
 __attribute__ ((weak, alias ("PQMP_get_error_number")));

 const char*        QMP_get_error_string (QMP_msghandle_t mh)
 __attribute__ ((weak, alias ("PQMP_get_error_string")));



 int                QMP_verbose (int level)
 __attribute__ ((weak, alias ("PQMP_verbose")));

 int                QMP_profcontrol (int level)
 __attribute__ ((weak, alias ("PQMP_profcontrol")));

 int   QMP_printf             (const char *format, ...)
 __attribute__ ((weak, alias ("PQMP_printf")));

 int   QMP_fprintf            (FILE* stream, const char *format, ...)
 __attribute__ ((weak, alias ("PQMP_fprintf")));

 int   QMP_info               (const char *format, ...)
 __attribute__ ((weak, alias ("PQMP_info")));
 int   QMP_error              (const char *format, ...)
 __attribute__ ((weak, alias ("PQMP_error")));


 void  QMP_reset_total_qmp_time(void)
 __attribute__ ((weak, alias ("PQMP_reset_total_qmp_time")));
 double QMP_get_total_qmp_time(void)
 __attribute__ ((weak, alias ("PQMP_get_total_qmp_time")));

#define QMP_init_msg_passing PQMP_init_msg_passing
#define QMP_is_initialized PQMP_is_initialized
#define QMP_finalize_msg_passing PQMP_finalize_msg_passing
#define QMP_abort PQMP_abort
#define QMP_abort_string PQMP_abort_string
#define QMP_get_msg_passing_type PQMP_get_msg_passing_type
#define QMP_get_number_of_nodes PQMP_get_number_of_nodes
#define QMP_get_node_number PQMP_get_node_number
#define QMP_is_primary_node PQMP_is_primary_node
#define QMP_get_allocated_number_of_dimensions PQMP_get_allocated_number_of_dimensions
#define QMP_get_allocated_dimensions PQMP_get_allocated_dimensions
#define QMP_get_allocated_coordinates PQMP_get_allocated_coordinates
#define QMP_io_node PQMP_io_node
#define QMP_master_io_node PQMP_master_io_node
#define QMP_declare_logical_topology PQMP_declare_logical_topology
#define QMP_logical_topology_is_declared PQMP_logical_topology_is_declared
#define QMP_get_logical_number_of_dimensions PQMP_get_logical_number_of_dimensions
#define QMP_get_logical_dimensions PQMP_get_logical_dimensions
#define QMP_get_logical_coordinates PQMP_get_logical_coordinates
#define QMP_get_logical_coordinates_from PQMP_get_logical_coordinates_from
#define QMP_get_node_number_from PQMP_get_node_number_from
#define QMP_layout_grid PQMP_layout_grid
#define QMP_get_subgrid_dimensions PQMP_get_subgrid_dimensions
#define QMP_get_number_of_subgrid_sites PQMP_get_number_of_subgrid_sites
#define QMP_allocate_memory PQMP_allocate_memory
#define QMP_allocate_aligned_memory PQMP_allocate_aligned_memory
#define QMP_get_memory_pointer PQMP_get_memory_pointer
#define QMP_free_memory PQMP_free_memory
#define QMP_declare_msgmem PQMP_declare_msgmem
#define QMP_declare_strided_msgmem PQMP_declare_strided_msgmem
#define QMP_declare_strided_array_msgmem PQMP_declare_strided_array_msgmem
#define QMP_free_msgmem PQMP_free_msgmem
#define QMP_declare_receive_relative PQMP_declare_receive_relative
#define QMP_declare_send_relative PQMP_declare_send_relative
#define QMP_declare_send_to PQMP_declare_send_to
#define QMP_declare_receive_from PQMP_declare_receive_from
#define QMP_free_msghandle PQMP_free_msghandle
#define QMP_declare_multiple PQMP_declare_multiple
#define QMP_start PQMP_start
#define QMP_wait PQMP_wait
#define QMP_wait_all PQMP_wait_all
#define QMP_is_complete PQMP_is_complete
#define QMP_barrier PQMP_barrier
#define QMP_broadcast PQMP_broadcast
#define QMP_sum_int PQMP_sum_int
#define QMP_sum_float PQMP_sum_float
#define QMP_sum_double PQMP_sum_double
#define QMP_sum_double_extended PQMP_sum_double_extended
#define QMP_sum_float_array PQMP_sum_float_array
#define QMP_sum_double_array PQMP_sum_double_array
#define QMP_max_float PQMP_max_float
#define QMP_max_double PQMP_max_double
#define QMP_min_float PQMP_min_float
#define QMP_min_double PQMP_min_double
#define QMP_xor_ulong PQMP_xor_ulong
#define QMP_binary_reduction PQMP_binary_reduction
#define QMP_error_string PQMP_error_string
#define QMP_get_error_number PQMP_get_error_number
#define QMP_get_error_string PQMP_get_error_string
#define QMP_verbose PQMP_verbose
#define QMP_profcontrol PQMP_profcontrol
#define QMP_printf PQMP_printf
#define QMP_fprintf PQMP_fprintf
#define QMP_info PQMP_info
#define QMP_error PQMP_error
#define QMP_reset_total_qmp_time PQMP_reset_total_qmp_time
#define QMP_get_total_qmp_time PQMP_get_total_qmp_time

extern QMP_status_t       QMP_init_msg_passing (int* argc, char*** argv,
						QMP_thread_level_t required,
						QMP_thread_level_t *provided);

extern QMP_bool_t         QMP_is_initialized(void);

extern void               QMP_finalize_msg_passing (void);

extern void               QMP_abort (int error_code);

extern void               QMP_abort_string (int error_code, char *message);



extern QMP_ictype_t       QMP_get_msg_passing_type (void);

extern int                QMP_get_number_of_nodes (void);

extern int                QMP_get_node_number (void);

extern QMP_bool_t         QMP_is_primary_node (void);

extern int                QMP_get_allocated_number_of_dimensions (void);

extern const int*         QMP_get_allocated_dimensions (void);

extern const int*         QMP_get_allocated_coordinates (void);




extern int                QMP_io_node(int node);

extern int                QMP_master_io_node(void);



extern QMP_status_t       QMP_declare_logical_topology (const int dims[],
							int ndim);

extern QMP_bool_t         QMP_logical_topology_is_declared (void);

extern int                QMP_get_logical_number_of_dimensions (void);

extern const int*         QMP_get_logical_dimensions (void);

extern const int*         QMP_get_logical_coordinates (void);

extern int*               QMP_get_logical_coordinates_from (int node);

extern int                QMP_get_node_number_from (const int coordinates[]);



extern QMP_status_t       QMP_layout_grid (const int dimensions[], int ndims);

extern const int*         QMP_get_subgrid_dimensions (void);

extern int                QMP_get_number_of_subgrid_sites (void);




extern QMP_mem_t*         QMP_allocate_memory (size_t nbytes);

extern QMP_mem_t*         QMP_allocate_aligned_memory (size_t nbytes,
						       size_t alignment,
						       int flags);

extern void*              QMP_get_memory_pointer (QMP_mem_t* mem);

extern void               QMP_free_memory (QMP_mem_t* mem);

extern QMP_msgmem_t       QMP_declare_msgmem (const void* mem, size_t nbytes);

extern QMP_msgmem_t       QMP_declare_strided_msgmem (void* base, 
						      size_t blksize,
						      int nblocks,
						      ptrdiff_t stride);

extern QMP_msgmem_t   QMP_declare_strided_array_msgmem (void* base[],
							size_t blksize[],
							int nblocks[],
							ptrdiff_t stride[],
							int num);

extern void               QMP_free_msgmem (QMP_msgmem_t m);



extern QMP_msghandle_t    QMP_declare_receive_relative (QMP_msgmem_t m, 
							int axis,
							int dir,
							int priority);

extern QMP_msghandle_t    QMP_declare_send_relative    (QMP_msgmem_t m,
							int axis,
							int dir,
							int priority);

extern QMP_msghandle_t    QMP_declare_send_to     (QMP_msgmem_t m, 
						   int rem_node_rank,
						   int priority);

extern QMP_msghandle_t    QMP_declare_receive_from(QMP_msgmem_t m, 
						   int rem_node_rank,
						   int priority);

extern void               QMP_free_msghandle (QMP_msghandle_t h);

extern QMP_msghandle_t    QMP_declare_multiple (QMP_msghandle_t msgh[], 
						int num);

extern QMP_status_t       QMP_start (QMP_msghandle_t h);

extern QMP_status_t       QMP_wait (QMP_msghandle_t h);

extern QMP_status_t       QMP_wait_all (QMP_msghandle_t h[], int num);

extern QMP_bool_t         QMP_is_complete (QMP_msghandle_t h);



extern QMP_status_t       QMP_barrier (void);

extern QMP_status_t       QMP_broadcast (void* buffer, size_t nbytes);

extern QMP_status_t       QMP_sum_int (int *value);

extern QMP_status_t       QMP_sum_float (float *value);

extern QMP_status_t       QMP_sum_double (double *value);

extern QMP_status_t       QMP_sum_double_extended (double *value);

extern QMP_status_t       QMP_sum_float_array (float value[], int length);

extern QMP_status_t       QMP_sum_double_array (double value[], int length);

extern QMP_status_t       QMP_max_float (float* value);

extern QMP_status_t       QMP_max_double (double* value);

extern QMP_status_t       QMP_min_float (float* value);

extern QMP_status_t       QMP_min_double (double* value);

extern QMP_status_t       QMP_xor_ulong (unsigned long* value);

extern QMP_status_t       QMP_binary_reduction (void* lbuffer, size_t buflen,
						QMP_binary_func bfunc);



extern const char*        QMP_error_string (QMP_status_t code);

extern QMP_status_t       QMP_get_error_number (QMP_msghandle_t mh);

extern const char*        QMP_get_error_string (QMP_msghandle_t mh);



extern int                QMP_verbose (int level);

extern int                QMP_profcontrol (int level);

extern int   QMP_printf             (const char *format, ...);

extern int   QMP_fprintf            (FILE* stream, const char *format, ...);

extern int   QMP_info               (const char *format, ...);
extern int   QMP_error              (const char *format, ...);


extern void  QMP_reset_total_qmp_time(void);
extern double QMP_get_total_qmp_time(void);
#endif
#endif
