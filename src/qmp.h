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
 *   Revision 1.9  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.8  2004/04/08 09:00:20  bjoo
 *   Added experimental support for strided msgmem
 *
 *   Revision 1.7  2003/12/19 04:51:29  edwards
 *   Added prototype for QMP_route.
 *
 *   Revision 1.6  2003/11/04 01:04:32  edwards
 *   Changed QMP_get_logical_coordinates_from to not have const modifier.
 *   Now, user must explicitly call "free".
 *
 *   Revision 1.5  2003/07/29 15:23:48  chen
 *   remove not used decleration
 *
 *   Revision 1.4  2003/06/04 19:19:39  edwards
 *   Added a QMP_abort() function.
 *
 *   Revision 1.3  2003/02/14 03:22:25  flemingg
 *   src/qmp.hpp is no longer #include-d by src/qmp.h #ifdef __cplusplus.
 *   Also, since qmp.hpp doesn't need to end up in $prefix/include anymore,
 *   moved it back to EXTRA_DIST in src/Makefile.am
 *
 *   Revision 1.2  2003/02/13 16:37:55  chen
 *   add qmp.hpp remove qmp.hh
 *
 *   Revision 1.1  2003/02/13 16:22:24  chen
 *   qmp version 1.2
 *
 *
 *
 */
#ifndef _QMP_H
#define _QMP_H

/**
 * GTF: must #include <stdio.h> so that FILE* type is declared before
 * the QMP_fprintf() prototype below.  Previous implementation depended
 * on the user including it before this file.
 */

#include <stdio.h>
#include <stddef.h>  /* needed for ptrdiff_t */

/**
 * Current version information in string form.
 * Provided by autoconf.
 */
#define QMP_VERSION_STR PACKAGE_VERSION

/**
 * Version Information about QCD Message Passing API.
 */
#define QMP_MAJOR_VERSION ((QMP_VERSION_STR)[0])
#define QMP_MINOR_VERSION ((QMP_VERSION_STR)[2])
#define QMP_REVIS_VERSION ((QMP_VERSION_STR)[4])

/**
 * Current version of QCD Message Passing API.
 */
#define QMP_VERSION_CODE QMP_VERSION(QMP_MAJOR_VERSION, QMP_MINOR_VERSION, QMP_REVIS_VERSION)

/**
 * Calculate version code from major, minor, revs numbers.
 */
#define QMP_VERSION (a, b, c) (((a) << 8) + ((b) << 4) + (c))

typedef int                QMP_bool_t;
#define QMP_TRUE           (QMP_bool_t)1
#define QMP_FALSE          (QMP_bool_t)0

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
 * Thread Safety Level.
 */
typedef enum QMP_thread_level
{
  QMP_THREAD_SINGLE,
  QMP_THREAD_FUNNELED,
  QMP_THREAD_SERIALIZED,
  QMP_THREAD_MULTIPLE
}QMP_thread_level_t;

#define QMP_ALIGN_ANY     0
#define QMP_ALIGN_DEFAULT QMP_ALIGN_ANY

#define QMP_MEM_NONCACHE  0x01
#define QMP_MEM_COMMS     0x02
#define QMP_MEM_FAST      0x04
#define QMP_MEM_DEFAULT   (QMP_MEM_COMMS|QMP_MEM_FAST)

/**
 * QMP status 
 */
typedef int QMP_status_t;

/**
 * Memory type 
 */
typedef struct QMP_mem_struct_t QMP_mem_t;

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
 *
 * operation is defined as inout = inout op in;
 * this is the same definition as the MPI
 */
typedef void (*QMP_binary_func) (void* inout, void* in);


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * QMP Functions C APIs
 */


/*************************************
 *  Initialization and finalization  *
 *************************************/

/**
 * Initialization message passing. This must be called before 
 * real message passing begins.
 *
 * @param argc number of command line arguments.
 * @param argv command line arguments.
 * @param option QMP_SMP_ONE_ADDRESS or QMP_SMP_MULTIPLE_ADDRES.
 * @return QMP_SUCCESS if the QMP system is initialized correctly.
 */
extern QMP_status_t       QMP_init_msg_passing (int* argc, char*** argv,
						QMP_thread_level_t required,
						QMP_thread_level_t *provided);

/**
 * Shutdown QMP message passing system. Release system resource and memory.
 * No more message passing after this routine.
 */
extern void               QMP_finalize_msg_passing (void);

/**
 * Shutdown QMP message passing system and abort program. The error_code may
 * be used for the return code of the program. 
 */
extern void               QMP_abort (int error_code);

/**
 * Shutdown QMP message passing system and abort program and print string.
 * The error_code may be used for the return code of the program. 
 */
extern void               QMP_abort_string (int error_code, char *message);


/********************************
 *  Allocated machine routines  *
 ********************************/

/**
 * Get network inter_connection type
 * @return QMP_SWITCH, QMP_GRID (QMP_MESH) or QMP_FATTREE.
 */
extern QMP_ictype_t       QMP_get_msg_passing_type (void);

/**
 * Get number of physical nodes available
 *
 * @return number of allocated nodes for this job.
 */
extern int                QMP_get_number_of_nodes (void);

/**
 * Get node number of this machine
 *
 * @return node number of this machine
 */
extern int                QMP_get_node_number (void);

/**
 * Check whether a node is the physical root node or not.
 */
extern QMP_bool_t         QMP_is_primary_node (void);

/**
 * Get number of allocated dimensions.
 *
 * @return number of dimensions in a grid type of machines, 0 for
 * switched configuration.
 */
extern int                QMP_get_allocated_number_of_dimensions (void);

/**
 * Return allocated size of grid machines.
 * Callers should not free memory associated with this pointer.
 *
 * @return null if underlying machines are switched machines.
 * 
 */
extern const int*         QMP_get_allocated_dimensions (void);

/**
 * Get coordinate information about allocated machines.
 * Callers should not free memory associated with this pointer.
 *
 * @return 0 if the configuration is a network switched configuration.
 *
 */
extern const int*         QMP_get_allocated_coordinates (void);



/****************
 *  I/O layout  *
 ****************/

/**
 * For partitioned I/O, nodes are partitioned into subsets.  Each
 * subset includes a designated I/O node.  This function maps a node
 * to its I/O node. 
 *
 */

int 
QMP_io_node(int node);

/**
 * For binary file I/O we designate a master I/O node for the entire
 * machine.  This function defines it 
 *
 */

int 
QMP_master_io_node(void);


/******************************
 *  Logical topology routines *
 ******************************/

/**
 * Forces the logical topology to be a simple grid of given dimensions.
 *
 * @param dims size of each dimension.
 * @param ndim number of logical dimensions.
 * @return QMP_TRUE: if success, otherwise return QMP_FALSE
 */
extern QMP_status_t       QMP_declare_logical_topology (const int dims[],
							int ndim);

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
extern int                QMP_get_logical_number_of_dimensions (void);

/**
 * Get dimension size information for a logical topology.
 *
 * @return dimension size of the logical topology. If there is no logical 
 * topology, return information from physical geometry.
 */
extern const int*         QMP_get_logical_dimensions (void);

/**
 * Get coordinate of this node within the logical topology.
 *
 *
 * @return coordinate of this node. If no logical topology declared, 
 * return information from physical geometry.
 */
extern const int*         QMP_get_logical_coordinates (void);

/**
 * Get a logical coordinate from a node number.
 *
 *
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry. Callers should free
 * memory of the returned pointer.
 */
extern int*               QMP_get_logical_coordinates_from (int node);

/**
 * Get the node number from its logical coordinates.
 * @return node number.
 */
extern int                QMP_get_node_number_from (const int coordinates[]);


/**********************************************
 *  Problem Specification (physical lattice)  *
 **********************************************/

/**
 * General geometry constructor. Automatically determines the 
 * optimal layout -> decides the optimal ordering of axes.
 */
extern QMP_status_t       QMP_layout_grid (int dimensions[], int ndims);

/**
 * Return logical (lattice) subgrid sizes.
 */
extern const int*         QMP_get_subgrid_dimensions (void);

/**
 * Return logical (lattice) subgrid number of sites.
 */
extern int                QMP_get_number_of_subgrid_sites (void);


/***********************************
 *  Communication memory routines  *
 ***********************************/

/*
 *  Declare and Free Memory Addresses for Messages (and other uses)
 */

/**
 * Allocate memory of length nbytes with default alignment and type.
 * @param nbytes number of bytes of memory to allocate.
 * @return pointer to a newly allocated memory structure, 0 if no memory.
 */
extern QMP_mem_t*         QMP_allocate_memory (size_t nbytes);

/**
 * Allocate memory of a specified alignment and type.
 * @param nbytes number of bytes of memory to allocate.
 * @param alignment required alignment for memory.
 * @param flags memory type flags.
 * @return pointer to a newly allocated memory structure, 0 if no memory.
 */
extern QMP_mem_t*         QMP_allocate_aligned_memory (size_t nbytes,
						       size_t alignment,
						       int flags);

/**
 * Get pointer to memory from a memory structure.
 * @param pointer to memory structure.
 * @return pointer to memory.
 */
extern void*              QMP_get_memory_pointer (QMP_mem_t* mem);

/**
 * Free allocated memory structure.
 *
 * @param mem pointer to an allocated memory structure.
 */
extern void               QMP_free_memory (QMP_mem_t* mem);

/**
 * Create a message memory using memory created by user.
 * 
 * @param mem pointer to user memory.
 * @param nbytes size of the user memory.
 *
 * @return QMP_msgmem_t.
 */
extern QMP_msgmem_t       QMP_declare_msgmem (const void* mem, size_t nbytes);

/**
 * Declare a strided memory.
 */
extern QMP_msgmem_t       QMP_declare_strided_msgmem (void* base, 
						      size_t blksize,
						      int nblocks,
						      ptrdiff_t stride);

/**
 * Declare a strided array memory.
 */
extern QMP_msgmem_t   QMP_declare_strided_array_msgmem (void* base[],
							size_t blksize[],
							int nblocks[],
							ptrdiff_t stride[],
							int num);

/**
 * Free memory pointed by QMP_msgmem_t pointer
 * Any call using a QMP_msgmem_t after it has been freed will
 * produce unpredictable results.
 *
 * @param m a QMP_msgmem_t value.
 */
extern void               QMP_free_msgmem (QMP_msgmem_t m);


/********************************
 *  Communication Declarations  *
 ********************************/

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
							int axis,
							int dir,
							int priority);

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
							int axis,
							int dir,
							int priority);

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
						   int rem_node_rank,
						   int priority);

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
						   int rem_node_rank,
						   int priority);

/**
 * Free a message handle.
 *
 * If this message handle is a handle for multiple message handles,
 * this routine will not free the individual handles.
 *
 * @param h message handle.
 */
extern void               QMP_free_msghandle (QMP_msghandle_t h);

/**
 * Collapse multiple message handles into a single one.
 *
 * @param msgh pointer to an array of message handles.
 * @param num  size of the array.
 * @return a complex message handle.
 */
extern QMP_msghandle_t    QMP_declare_multiple (QMP_msghandle_t msgh[], 
						int num);

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
 * Wait for a set of operations to complete for an array message handles.
 * This code will block until all communications are finished.
 *
 * @param h a message handle.
 *
 * @return QMP_SUCCESS if a communication is done.
 */
extern QMP_status_t       QMP_wait (QMP_msghandle_t h);

/**
 * Wait for an operation to complete for a particular message handle.
 * This code will block until a previous communication is finished.
 *
 * @param h an array of message handles.
 * @param num the length of the array.
 *
 * @return QMP_SUCCESS if a communication is done.
 */
extern QMP_status_t       QMP_wait_all (QMP_msghandle_t h[], int num);

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


/***********************
 *  Global Operations  *
 ***********************/

/**
 * Synchronization barrier call.
 */
extern QMP_status_t       QMP_barrier (void);

/**
 * Broadcast bytes from a node. This routine is a blocking routine.
 *
 * @param buffer a pointer to a memory buffer.
 * @param nbytes size of the buffer.
 *
 * @return QMP_SUCCESS for a successful broadcasting.
 */
extern QMP_status_t       QMP_broadcast (void* buffer, size_t nbytes);

/**
 * Global in place sum of an integer 
 * @param value a pointer to a integer.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_int (int *value);

/**
 * Global in place sum of a float.
 * @param value a pointer to a float.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_float (float *value);

/**
 * Global in place sum of a double.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_double (double *value);

/**
 * Global in place sum of a double. Intermediate values kept in extended
 * precision if possible.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_double_extended (double *value);

/**
 * Global in place sum of a float array.
 * @param value a pointer to a float array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t       QMP_sum_float_array (float value[], int length);

/**
 * Global in place sum of a double array.
 * @param value a pointer to a double array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t       QMP_sum_double_array (double value[], int length);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t       QMP_max_float (float* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t       QMP_max_double (double* value);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t       QMP_min_float (float* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t       QMP_min_double (double* value);

/**
 * Get the exclusive ored value of an unsigned long integer
 */
extern QMP_status_t       QMP_xor_ulong (unsigned long* value);

/**
 * Global binary reduction using a user provided function.
 *
 * @param lbuffer a pointer to a memory buffer.
 * @param buflen  size of the buffer.
 * @param bfunc   user provided binary function.
 *
 * @return QMP_SUCCESS if a binary reduction is a success.
 */
extern QMP_status_t       QMP_binary_reduction (void* lbuffer, size_t buflen,
						QMP_binary_func bfunc);


/*******************************
 *  Error reporting functions  *
 *******************************/

/**
 * Return an error string from a error code.
 * 
 * Quick Intro of QMP ERROR CODE:
 * 
 *    SUCCESS always returns 0
 *    QMP SPECIFIC ERROR starts from 0x1001
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
extern QMP_status_t       QMP_get_error_number (QMP_msghandle_t mh);

/**
 * Get error string for a message handle.
 */
extern const char*        QMP_get_error_string (QMP_msghandle_t mh);


/****************
 *  I/O helpers *
 ****************/

/**
 * Verbose mode of execution.
 * @param level sets the level of output messages.
 * @return old verbosity level.
 */
extern int                QMP_verbose (int level);

/**
 * Control profiling mode.
 * @param level sets the level of profiling.
 * @return old profiling level.
 */
extern int                QMP_profcontrol (int level);

/**
 * QMP specific printf function with rank and host information.
 */
extern int   QMP_printf             (const char *format, ...);

/**
 * QMP specific fprintf function with rank and host information.
 */
extern int   QMP_fprintf            (FILE* stream, const char *format, ...);

/** 
 * Information or error printing for QMP.
 */
extern int   QMP_info               (const char *format, ...);
extern int   QMP_error              (const char *format, ...);


#ifdef __cplusplus
}

/* #include "qmp.hpp" */

#endif

#endif
