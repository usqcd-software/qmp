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
 *     Qcd Message Passing Pacakge C Public Header File 
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.10  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.9  2002/05/07 17:18:44  chen
 *   Make spec and header file consistent
 *
 *   Revision 1.8  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.7  2002/04/25 19:20:22  chen
 *   Minor changes
 *
 *   Revision 1.6  2002/04/25 18:40:01  chen
 *   Pre 1.0 release
 *
 *   Revision 1.5  2002/04/22 20:28:40  chen
 *   Version 0.95 Release
 *
 *   Revision 1.4  2002/03/27 20:48:48  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.3  2002/02/15 20:34:52  chen
 *   First Beta Release QMP
 *
 *   Revision 1.2  2002/01/24 20:10:49  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:50  chen
 *   initial import of QMP
 *
 *
 */
#ifndef _QMP_H
#define _QMP_H

/**
 * Version Information about QCD Message Passing API.
 */
#define QMP_MAJOR_VERSION 1
#define QMP_MINOR_VERSION 0
#define QMP_REVIS_VERSION 0

/**
 * Current version of QCD Message Passing API.
 */
#define QMP_VERSION_CODE (((QMP_MAJOR_VERSION) << 8) + ((QMP_MINOR_VERSION) << 4) + (QMP_REVIS_VERSION))

/**
 * Calculate version code from major, minor, revs numbers.
 */
#define QMP_VERSION (a, b, c) (((a) << 8) + ((b) << 4) + (c))

/**
 * Current version information in string form.
 */
#define QMP_VERSION_STR "1.0.0"

/**
 * 64 bit os environment.
 */
#if defined (__sparcv9) || defined (__alpha)
#define QMP_64BIT_LONG
#endif 

/**
 * Define QCD Message passing own types for better portablity.
 */
typedef unsigned char      QMP_u8_t;
typedef unsigned short     QMP_u16_t;
typedef unsigned int       QMP_u32_t;
typedef char               QMP_s8_t;
typedef short              QMP_s16_t;
typedef int                QMP_s32_t;
typedef int                QMP_bool_t;

#define QMP_TRUE           (QMP_bool_t)1
#define QMP_FALSE          (QMP_bool_t)0

#ifdef QMP_64BIT_LONG
typedef unsigned long      QMP_u64_t;
typedef long               QMP_s64_t;
#else
typedef unsigned long long QMP_u64_t;
typedef long long          QMP_s64_t;
#endif

typedef float              QMP_float_t;
typedef double             QMP_double_t;

/**
 * General data type for QMP
 */
typedef enum QMP_datatype
{
  QMP_UNSIGNED_CHAR = 0,
  QMP_CHAR,
  QMP_BYTE,
  QMP_UNSIGNED_SHORT,
  QMP_SHORT,
  QMP_UNSIGNED_INT,
  QMP_INT,
  QMP_UNSIGNED_64BIT_INT,
  QMP_64BIT_INT,
  QMP_FLOAT,
  QMP_DOUBLE,
  QMP_NUM_S_TYPES
}QMP_datatype_t;

/**
 * Function status code.
 */
typedef enum QMP_status_code
{
  QMP_SUCCESS = 0,
  QMP_ERROR = 0x1001,
  QMP_NOT_INITED,
  QMP_RTENV_ERR,
  QMP_CPUINFO_ERR,
  QMP_NODEINFO_ERR,
  QMP_NOMEM_ERR,
  QMP_MEMSIZE_ERR,
  QMP_HOSTNAME_ERR,
  QMP_INITSVC_ERR,
  QMP_TOPOLOGY_EXISTS,
  QMP_CH_TIMEOUT,
  QMP_NOTSUPPORTED,
  QMP_SVC_BUSY,
  QMP_BAD_MESSAGE,
  QMP_INVALID_ARG,
  QMP_INVALID_TOPOLOGY,
  QMP_NONEIGHBOR_INFO,
  QMP_MEMSIZE_TOOBIG,
  QMP_BAD_MEMORY,
  QMP_NO_PORTS,
  QMP_NODE_OUTRANGE,
  QMP_CHDEF_ERR,
  QMP_MEMUSED_ERR,
  QMP_INVALID_OP,
  QMP_TIMEOUT,
  QMP_MAX_STATUS
}QMP_status_code_t;

/**
 * Interconnect type for LQCD.
 */
typedef enum QMP_ictype
{
  QMP_SWITCH = 0,
  QMP_GRID = 1,
  QMP_MESH = 1,
  QMP_FATTREE = 2
}QMP_ictype_t;

/**
 * Use single address or multiple address for a smp box.
 */
typedef enum QMP_smpaddr_type
{
  QMP_SMP_ONE_ADDRESS,
  QMP_SMP_MULTIPLE_ADDRESS
}QMP_smpaddr_type_t;


/**
 * QMP status 
 */
typedef int QMP_status_t;

/**
 * Message Memory type 
 */
typedef void* QMP_msgmem_t;

/**
 * Message handle
 */
typedef void* QMP_msghandle_t;

/**
 * binary reduction function.
 */
typedef void  (*QMP_binary_func) (void* dest, void* source1, void* source2);


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * QMP Functions C APIs
 */

/**
 * Initialization message passing. This must be called before 
 * real message passing begins.
 *
 * @param argc number of command line arguments.
 * @param argv command line arguments.
 * @param option QMP_SMP_ONE_ADDRESS or QMP_SMP_MULTIPLE_ADDRES.
 * @return QMP_SUCCESS if the QMP system is initialized correctly.
 */
extern QMP_status_t       QMP_init_msg_passing (int argc, char** argv,
						QMP_smpaddr_type_t option);

/**
 * Shutdown QMP message passing system. Release system resource and memory.
 * No more message passing after this routine.
 */
extern void               QMP_finalize_msg_passing (void);

/**
 * Verbose mode of execution.
 * @param verbose true turn on verbose mode.
 */
extern void               QMP_verbose  (QMP_bool_t verbose);

/**
 * Get SMP Count for this node.
 * @return number of CPUs in this node.
 */
extern const QMP_u32_t    QMP_get_SMP_count (void);

/**
 * Get network inter_connection type
 * @return QMP_SWITCH, QMP_GRID (QMP_MESH) or QMP_FATTREE.
 */
extern const QMP_ictype_t QMP_get_msg_passing_type (void);

/**
 * Get number of physical nodes available
 *
 * @return number of allocated nodes for this job.
 */
extern const QMP_u32_t    QMP_get_number_of_nodes (void);

/**
 * Get number of allocated dimensions.
 *
 * @return number of dimensions in a grid type of machines, 0 for
 * switched configuration.
 */
extern const QMP_u32_t    QMP_get_allocated_number_of_dimensions (void);

/**
 * Return allocated size of grid machines.
 * Callers should not free memory associated with this pointer.
 *
 * @return null if underlying machines are switched machines.
 * 
 */
extern const QMP_u32_t*   QMP_get_allocated_dimensions (void);

/**
 * Get coordinate information about allocated machines.
 * Callers should not free memory associated with this pointer.
 *
 * @return 0 if the configuration is a network switched configuration.
 *
 */
extern const QMP_u32_t*   QMP_get_allocated_coordinates (void);


/**
 * Forces the logical topology to be a simple grid of given dimensions.
 *
 * @param dims size of each dimension.
 * @param ndim number of logical dimensions.
 * @return QMP_TRUE: if success, otherwise return QMP_FALSE
 */
extern QMP_bool_t         QMP_declare_logical_topology (const QMP_u32_t* dims,
							QMP_u32_t ndim);

/**
 * Check whether a logical topology is declared or not.
 *
 * @return QMP_TRUE if a logical topology is declared, 
 * otherwise return QMP_FALSE
 */
extern QMP_bool_t         QMP_logical_topology_is_declared (void);

/**
 * Get number of dimensions of logical topology.
 * @return dimensionality of the logical topology. If there is no
 * logical topology, return physical topology information.
 */
extern const QMP_u32_t    QMP_get_logical_number_of_dimensions (void);

/**
 * Get dimension size information for a logical topology.
 *
 * @return dimension size of the logical topology. If there is no logical 
 * topology, return information from physical geometry.
 */
extern const QMP_u32_t*   QMP_get_logical_dimensions (void);

/**
 * Get node number of this machine
 *
 * @return node number of this machine
 */
extern const QMP_u32_t    QMP_get_node_number (void);


/**
 * Get coordinate of this node within the logical topology.
 *
 *
 * @return coordinate of this node. If no logical topology declared, 
 * return information from physical geometry.
 */
extern const QMP_u32_t*   QMP_get_logical_coordinates (void);

/**
 * Get a logical coordinate from a node number.
 *
 *
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry.
 */
extern const QMP_u32_t*   QMP_get_logical_coordinates_from (QMP_u32_t node);

/**
 * Get the node number from its logical coordinates.
 * @return node number.
 */
extern const QMP_u32_t    QMP_get_node_number_from (const QMP_u32_t* coordinates);

/**
 * Allocate optimally 16 byte aligned memory of length nbytes.
 * @param nbytes number of bytes of memory to allocate.
 * @return pointer to a newly allocated memory, 0 if no memory.
 */
extern void* QMP_allocate_aligned_memory (QMP_u32_t nbytes);


/**
 * Free memory allocated by QMP_allocated_aligned_memory routine.
 *
 * @param mem pointer to an allocated memory.
 */
extern void QMP_free_aligned_memory (void* mem);

/**
 * Create a message memory using memory created by user.
 * 
 * @param mem pointer to user memory.
 * @param nbytes size of the user memory.
 *
 * @return QMP_msgmem_t.
 */
extern QMP_msgmem_t       QMP_declare_msgmem (const void* mem, 
					      QMP_u32_t nbytes);

/**
 * Free memory pointed by QMP_msgmem_t pointer
 * Any call using a QMP_msgmem_t after it has been freed will
 * produce unpredictable results.
 *
 * @param m a QMP_msgmem_t value.
 */
extern void               QMP_free_msgmem (QMP_msgmem_t m);

/**
 * Declare a strided memory. Not yet implemented.
 */
extern QMP_msgmem_t       QMP_declare_strided_msg_mem (void* base, 
						       QMP_u32_t blksize,
						       QMP_u32_t nblocks,
						       QMP_u32_t stride);

/**
 * Declare a strided array memory. Not yet implemented.
 */
extern QMP_msgmem_t QMP_declare_strided_array_msgmem (void** base, 
						      QMP_u32_t* blksize,
						      QMP_u32_t* nblocks,
						      QMP_u32_t* stride);

/**
 * Declares an endpoint for a message channel of receiveing operations
 * between this node and it's neighbor
 *
 * The neighbor is described by an axis and direction.
 * The axes are from 0 to dim, and directions are 1 and -1.
 *
 * @param m QMP_msgmem_t pointer.
 * @param axis communication dimension.
 * @param dir  direction of communicaton, +1 or -1.
 * @param priority priority of this communication.
 *
 * @return QMP_msghandle_t caller should check whether it is null.
 */
extern QMP_msghandle_t    QMP_declare_receive_relative (QMP_msgmem_t m, 
							QMP_s32_t axis,
							QMP_s32_t dir,
							QMP_s32_t priority);

/**
 * Declares an endpoint for a message channel of sending operations
 * between this node and it's neighbor
 *
 * The neighbor is described by an axis and direction.
 * The axes are from 0 to dim, and directions are 1 and -1.
 *
 * @param m QMP_msgmem_t pointer.
 * @param axis communication dimension.
 * @param dir  direction of communicaton, +1 or -1.
 * @param priority priority of this communication.
 *
 * @return QMP_msghandle_t caller should check whether it is null.
 */
extern QMP_msghandle_t    QMP_declare_send_relative    (QMP_msgmem_t m,
							QMP_s32_t axis,
							QMP_s32_t dir,
							QMP_s32_t priority);

/**
 * Declare an endpoint for message send channel operation using remote
 * node's number.
 *
 * @param m a message memory handle.
 * @param rem_node_rank logical node number of a remote node.
 * @param priority priority of this communication.
 *
 * @return QMP_msghandle_t caller should check whether it is null.
 */
extern QMP_msghandle_t    QMP_declare_send_to     (QMP_msgmem_t m, 
						   QMP_u32_t rem_node_rank,
						   QMP_s32_t priority);

/**
 * Declare an endpoint for message channel receiving operation using remote
 * node's number.
 *
 * @param m a message memory handle.
 * @param rem_node_rank logical node number of a remote node.
 * @param priority priority of this communication.
 *
 * @return QMP_msghandle_t caller should check whether it is null.
 */
extern QMP_msghandle_t    QMP_declare_receive_from(QMP_msgmem_t m, 
						   QMP_u32_t rem_node_rank,
						   QMP_s32_t priority);

/**
 * Free a message handle.
 *
 * If this message handle is a handle for multiple message handles,
 * this routine will not free the individual handles.
 *
 * @param m message handle.
 */
extern void               QMP_free_msghandle (QMP_msghandle_t h);

/**
 * Print out message handle information for debug or information.
 */
extern void               QMP_print_msg_handle (QMP_msghandle_t h);

/**
 * Collapse multiple message handles into a single one.
 *
 * @param msgh pointer to an array of message handles.
 * @param num  size of the array.
 * @return a complex message handle.
 */
extern QMP_msghandle_t    QMP_declare_multiple (QMP_msghandle_t* msgh, 
						QMP_u32_t num);

/**
 * Return an error string from a error code.
 * 
 * Quick Intro of QMP ERROR CODE:
 * 
 *    SUCCESS always returns 0
 *    QMP SPECIFIC ERROR starts from 0x1001
 *    GM  SPECIFIC ERROR starte from 0x1
 */
extern const char*        QMP_error_string (QMP_status_t code);

/**
 * Return error number of last function call if there is an error.
 * If a function returns a status code, this function may not produce
 * any useful information. 
 *
 * This function should be used when no status
 * code is returned from a function call.
 *
 * If mh is not null, the error code associated with the mh is returned.
 * Otherwise, the global error code is returned.
 */
extern const QMP_status_t        QMP_get_error_number (QMP_msghandle_t mh);

/**
 * Get error string for a message handle.
 */
extern const char*               QMP_get_error_string (QMP_msghandle_t mh);

/**
 * Start a communication for a message handle.
 *
 * If the message handle is a collection of multiple message handles,
 * multiple real communication may take place.
 * 
 * @param h a message handle.
 * @return QMP_SUCCESS if a communication is stared.
 */
extern QMP_status_t       QMP_start (QMP_msghandle_t h);

/**
 * Wait for an operation to complete for a particular message handle.
 * This code will block until a previous communication is finished.
 *
 * @param h a message handle.
 *
 * @return QMP_SUCCESS if a communication is done.
 */
extern QMP_status_t       QMP_wait (QMP_msghandle_t h);

/**
 * Test whether a communication started by QMP_start with a message handle
 * has been completed.
 * This code is non-blocking.
 * 
 * @param h a message handle.
 *
 * @return QMP_TRUE if a communication is done.
 */
extern QMP_bool_t         QMP_is_complete (QMP_msghandle_t h);


/**
 * Public function for receiving a message.
 * Internal Testing only
 */
extern QMP_status_t QMP_recv (void* buffer, QMP_u32_t count, 
			      QMP_datatype_t datatype,
			      QMP_u32_t src_id, QMP_u32_t operation);

/**
 * Public function for sending a message.
 * Internal Testing only
 */
extern QMP_status_t QMP_send (void* buffer, QMP_u32_t count, 
			      QMP_datatype_t datatype,
			      QMP_u32_t dest_id, QMP_u32_t operation);


/**
 * Broadcast bytes from a node. This routine is a blocking routine.
 *
 * @param buffer a pointer to a memory buffer.
 * @param nbytes size of the buffer.
 *
 * @return QMP_SUCCESS for a successful broadcasting.
 */
extern QMP_status_t QMP_broadcast (void* buffer, QMP_u32_t nbytes);

/**
 * Global in place sum of an integer 
 * @param value a pointer to a integer.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t QMP_sum_int    (QMP_s32_t *value);

/**
 * Global in place sum of a float.
 * @param value a pointer to a float.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t QMP_sum_float  (QMP_float_t *value);

/**
 * Global in place sum of a double.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t QMP_sum_double (QMP_double_t *value);

/**
 * Global in place sum of a double. Intermediate values kept in extended
 * precision if possible.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t QMP_sum_double_extended (QMP_double_t *value);

/**
 * Global in place sum of a float array.
 * @param value a pointer to a float array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t QMP_sum_float_array (QMP_float_t* value, 
					 QMP_u32_t length);


/**
 * Global in place sum of a double array.
 * @param value a pointer to a double array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t QMP_sum_double_array (QMP_double_t* value, 
					  QMP_u32_t length);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t QMP_max_float (QMP_float_t* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t QMP_max_double (QMP_double_t* value);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t QMP_min_float (QMP_float_t* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t QMP_min_double (QMP_double_t* value);

/**
 * Get the exclusive ored value of all long integers
 */
extern QMP_status_t QMP_global_xor (long* value);


/**
 * Global binary reduction using a user provided function.
 *
 * @param lbuffer a pointer to a memory buffer.
 * @param buflen  size of the buffer.
 * @param bfunc   user provided binary function.
 *
 * @return QMP_SUCCESS if a binary reduction is a success.
 */
extern QMP_status_t QMP_binary_reduction (void* lbuffer, QMP_u32_t buflen,
					  QMP_binary_func bfunc);

/**
 * Synchronization barrier call.
 */
extern QMP_status_t QMP_wait_for_barrier (QMP_s32_t millieseconds);

/**
 * Check whether a node is the physical root node or not.
 */
extern QMP_bool_t   QMP_is_primary_node(void);

/**
 * QMP specific printf function with rank and host information.
 */
extern int   QMP_printf             (const char *format, ...);


/**
 * QMP specific fprintf function with rank and host information.
 */
extern int   QMP_fprintf             (FILE* stream, const char *format, ...);

/** 
 * Information or error printing for QMP.
 */
extern int   QMP_info                (const char *format, ...);
extern int   QMP_error               (const char *format, ...);

/**
 * Report QMP error and exit
 */
extern void QMP_error_exit (const char* format, ...);

/**
 * Report QMP error using a given node id number and exit
 */
extern void QMP_fatal (QMP_u32_t rank, const char* format, ...);


/**
 * General geometry constructor. Automatically determine sthe 
 * optimal layout -> decides the optimal ordering of axes.
 */
extern QMP_bool_t QMP_layout_grid (QMP_u32_t* dimensions,
				   QMP_u32_t  rank);

/**
 * Return logical (lattice) grid sizes.
 */
extern const QMP_u32_t* QMP_get_subgrid_dimensions (void);


/**
 * Return subgrid volume of this layout.
 */
extern const QMP_u32_t  QMP_get_subgrid_volume (void);


#ifdef __cplusplus
}
#endif

#endif

