/* --------------------------------------- */
/* Additions for profiling interface       */
/* Modified Dec 04 by Morten		   */
/* --------------------------------------- */

#ifdef QMP_BUILD_PROFILING

#ifndef __QMP_PROFILING_H
#define __QMP_PROFILING_H

#pragma weak QMP_init_msg_passing = PQMP_init_msg_passing
#undef QMP_init_msg_passing
#define QMP_init_msg_passing PQMP_init_msg_passing

#pragma weak QMP_is_initialized = PQMP_is_initialized
#undef QMP_is_initialized
#define QMP_is_initialized PQMP_is_initialized

#pragma weak QMP_finalize_msg_passing = PQMP_finalize_msg_passing
#undef QMP_finalize_msg_passing
#define QMP_finalize_msg_passing PQMP_finalize_msg_passing

#pragma weak QMP_abort = PQMP_abort
#undef QMP_abort
#define QMP_abort PQMP_abort

#pragma weak QMP_abort_string = PQMP_abort_string
#undef QMP_abort_string
#define QMP_abort_string PQMP_abort_string

#pragma weak QMP_get_msg_passing_type = PQMP_get_msg_passing_type
#undef QMP_get_msg_passing_type
#define QMP_get_msg_passing_type PQMP_get_msg_passing_type

#pragma weak QMP_get_number_of_nodes = PQMP_get_number_of_nodes
#undef QMP_get_number_of_nodes
#define QMP_get_number_of_nodes PQMP_get_number_of_nodes

#pragma weak QMP_get_node_number = PQMP_get_node_number
#undef QMP_get_node_number
#define QMP_get_node_number PQMP_get_node_number

#pragma weak QMP_is_primary_node = PQMP_is_primary_node
#undef QMP_is_primary_node
#define QMP_is_primary_node PQMP_is_primary_node

#pragma weak QMP_get_allocated_number_of_dimensions = PQMP_get_allocated_number_of_dimensions
#undef QMP_get_allocated_number_of_dimensions
#define QMP_get_allocated_number_of_dimensions PQMP_get_allocated_number_of_dimensions

#pragma weak QMP_get_allocated_dimensions = PQMP_get_allocated_dimensions
#undef QMP_get_allocated_dimensions
#define QMP_get_allocated_dimensions PQMP_get_allocated_dimensions

#pragma weak QMP_get_allocated_coordinates = PQMP_get_allocated_coordinates
#undef QMP_get_allocated_coordinates
#define QMP_get_allocated_coordinates PQMP_get_allocated_coordinates

#pragma weak QMP_io_node = PQMP_io_node
#undef QMP_io_node
#define QMP_io_node PQMP_io_node

#pragma weak QMP_master_io_node = PQMP_master_io_node
#undef QMP_master_io_node
#define QMP_master_io_node PQMP_master_io_node

#pragma weak QMP_declare_logical_topology = PQMP_declare_logical_topology
#undef QMP_declare_logical_topology
#define QMP_declare_logical_topology PQMP_declare_logical_topology

#pragma weak QMP_logical_topology_is_declared = PQMP_logical_topology_is_declared
#undef QMP_logical_topology_is_declared
#define QMP_logical_topology_is_declared PQMP_logical_topology_is_declared

#pragma weak QMP_get_logical_number_of_dimensions = PQMP_get_logical_number_of_dimensions
#undef QMP_get_logical_number_of_dimensions
#define QMP_get_logical_number_of_dimensions PQMP_get_logical_number_of_dimensions

#pragma weak QMP_get_logical_dimensions = PQMP_get_logical_dimensions
#undef QMP_get_logical_dimensions
#define QMP_get_logical_dimensions PQMP_get_logical_dimensions

#pragma weak QMP_get_logical_coordinates = PQMP_get_logical_coordinates
#undef QMP_get_logical_coordinates
#define QMP_get_logical_coordinates PQMP_get_logical_coordinates

#pragma weak QMP_get_logical_coordinates_from = PQMP_get_logical_coordinates_from
#undef QMP_get_logical_coordinates_from
#define QMP_get_logical_coordinates_from PQMP_get_logical_coordinates_from

#pragma weak QMP_get_node_number_from = PQMP_get_node_number_from
#undef QMP_get_node_number_from
#define QMP_get_node_number_from PQMP_get_node_number_from

#pragma weak QMP_layout_grid = PQMP_layout_grid
#undef QMP_layout_grid
#define QMP_layout_grid PQMP_layout_grid

#pragma weak QMP_get_subgrid_dimensions = PQMP_get_subgrid_dimensions
#undef QMP_get_subgrid_dimensions
#define QMP_get_subgrid_dimensions PQMP_get_subgrid_dimensions

#pragma weak QMP_get_number_of_subgrid_sites = PQMP_get_number_of_subgrid_sites
#undef QMP_get_number_of_subgrid_sites
#define QMP_get_number_of_subgrid_sites PQMP_get_number_of_subgrid_sites

#pragma weak QMP_allocate_memory = PQMP_allocate_memory
#undef QMP_allocate_memory
#define QMP_allocate_memory PQMP_allocate_memory

#pragma weak QMP_allocate_aligned_memory = PQMP_allocate_aligned_memory
#undef QMP_allocate_aligned_memory
#define QMP_allocate_aligned_memory PQMP_allocate_aligned_memory

#pragma weak QMP_get_memory_pointer = PQMP_get_memory_pointer
#undef QMP_get_memory_pointer
#define QMP_get_memory_pointer PQMP_get_memory_pointer

#pragma weak QMP_free_memory = PQMP_free_memory
#undef QMP_free_memory
#define QMP_free_memory PQMP_free_memory

#pragma weak QMP_declare_msgmem = PQMP_declare_msgmem
#undef QMP_declare_msgmem
#define QMP_declare_msgmem PQMP_declare_msgmem

#pragma weak QMP_declare_strided_msgmem = PQMP_declare_strided_msgmem
#undef QMP_declare_strided_msgmem
#define QMP_declare_strided_msgmem PQMP_declare_strided_msgmem

#pragma weak QMP_declare_strided_array_msgmem = PQMP_declare_strided_array_msgmem
#undef QMP_declare_strided_array_msgmem
#define QMP_declare_strided_array_msgmem PQMP_declare_strided_array_msgmem

#pragma weak QMP_free_msgmem = PQMP_free_msgmem
#undef QMP_free_msgmem
#define QMP_free_msgmem PQMP_free_msgmem

#pragma weak QMP_declare_receive_relative = PQMP_declare_receive_relative
#undef QMP_declare_receive_relative
#define QMP_declare_receive_relative PQMP_declare_receive_relative

#pragma weak QMP_declare_send_relative = PQMP_declare_send_relative
#undef QMP_declare_send_relative
#define QMP_declare_send_relative PQMP_declare_send_relative

#pragma weak QMP_declare_send_to = PQMP_declare_send_to
#undef QMP_declare_send_to
#define QMP_declare_send_to PQMP_declare_send_to

#pragma weak QMP_declare_receive_from = PQMP_declare_receive_from
#undef QMP_declare_receive_from
#define QMP_declare_receive_from PQMP_declare_receive_from

#pragma weak QMP_free_msghandle = PQMP_free_msghandle
#undef QMP_free_msghandle
#define QMP_free_msghandle PQMP_free_msghandle

#pragma weak QMP_declare_multiple = PQMP_declare_multiple
#undef QMP_declare_multiple
#define QMP_declare_multiple PQMP_declare_multiple

#pragma weak QMP_start = PQMP_start
#undef QMP_start
#define QMP_start PQMP_start

#pragma weak QMP_wait = PQMP_wait
#undef QMP_wait
#define QMP_wait PQMP_wait

#pragma weak QMP_wait_all = PQMP_wait_all
#undef QMP_wait_all
#define QMP_wait_all PQMP_wait_all

#pragma weak QMP_is_complete = PQMP_is_complete
#undef QMP_is_complete
#define QMP_is_complete PQMP_is_complete

#pragma weak QMP_barrier = PQMP_barrier
#undef QMP_barrier
#define QMP_barrier PQMP_barrier

#pragma weak QMP_broadcast = PQMP_broadcast
#undef QMP_broadcast
#define QMP_broadcast PQMP_broadcast

#pragma weak QMP_sum_int = PQMP_sum_int
#undef QMP_sum_int
#define QMP_sum_int PQMP_sum_int

#pragma weak QMP_sum_float = PQMP_sum_float
#undef QMP_sum_float
#define QMP_sum_float PQMP_sum_float

#pragma weak QMP_sum_double = PQMP_sum_double
#undef QMP_sum_double
#define QMP_sum_double PQMP_sum_double

#pragma weak QMP_sum_double_extended = PQMP_sum_double_extended
#undef QMP_sum_double_extended
#define QMP_sum_double_extended PQMP_sum_double_extended

#pragma weak QMP_sum_float_array = PQMP_sum_float_array
#undef QMP_sum_float_array
#define QMP_sum_float_array PQMP_sum_float_array

#pragma weak QMP_sum_double_array = PQMP_sum_double_array
#undef QMP_sum_double_array
#define QMP_sum_double_array PQMP_sum_double_array

#pragma weak QMP_max_float = PQMP_max_float
#undef QMP_max_float
#define QMP_max_float PQMP_max_float

#pragma weak QMP_max_double = PQMP_max_double
#undef QMP_max_double
#define QMP_max_double PQMP_max_double

#pragma weak QMP_min_float = PQMP_min_float
#undef QMP_min_float
#define QMP_min_float PQMP_min_float

#pragma weak QMP_min_double = PQMP_min_double
#undef QMP_min_double
#define QMP_min_double PQMP_min_double

#pragma weak QMP_xor_ulong = PQMP_xor_ulong
#undef QMP_xor_ulong
#define QMP_xor_ulong PQMP_xor_ulong

#pragma weak QMP_binary_reduction = PQMP_binary_reduction
#undef QMP_binary_reduction
#define QMP_binary_reduction PQMP_binary_reduction

#pragma weak QMP_error_string = PQMP_error_string
#undef QMP_error_string
#define QMP_error_string PQMP_error_string

#pragma weak QMP_get_error_number = PQMP_get_error_number
#undef QMP_get_error_number
#define QMP_get_error_number PQMP_get_error_number

#pragma weak QMP_get_error_string = PQMP_get_error_string
#undef QMP_get_error_string
#define QMP_get_error_string PQMP_get_error_string

#pragma weak QMP_verbose = PQMP_verbose
#undef QMP_verbose
#define QMP_verbose PQMP_verbose

#pragma weak QMP_profcontrol = PQMP_profcontrol
#undef QMP_profcontrol
#define QMP_profcontrol PQMP_profcontrol

#pragma weak QMP_printf = PQMP_printf
#undef QMP_printf
#define QMP_printf PQMP_printf

#pragma weak QMP_fprintf = PQMP_fprintf
#undef QMP_fprintf
#define QMP_fprintf PQMP_fprintf

#pragma weak QMP_info = PQMP_info
#undef QMP_info
#define QMP_info PQMP_info

#pragma weak QMP_error = PQMP_error
#undef QMP_error
#define QMP_error PQMP_error

#pragma weak QMP_reset_total_qmp_time = PQMP_reset_total_qmp_time
#undef QMP_reset_total_qmp_time
#define QMP_reset_total_qmp_time PQMP_reset_total_qmp_time
 
#pragma weak QMP_get_total_qmp_time = PQMP_get_total_qmp_time
#undef QMP_get_total_qmp_time
#define QMP_get_total_qmp_time PQMP_get_total_qmp_time

#endif
#endif
