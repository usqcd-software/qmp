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
 *   $Log: qmp.h,v $
 *   Revision 1.6  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.5  2005/08/27 19:49:16  osborn
 *   Removed a '//' to make people who don't want to use C99 happy.
 *
 *   Revision 1.4  2005/06/20 21:14:53  osborn
 *   Moved autoconf defines into qmp_config.h to make XLC happy.
 *
 *   Revision 1.3  2004/12/19 07:30:14  morten
 *   Added function declerations for the new profiling functions.
 *
 *   Revision 1.2  2004/10/31 23:21:35  osborn
 *   Restored proper behavior of msghandle operations in single node version.
 *   Added CFLAGS to qmp-config script.
 *   Changed QMP_status_code_t to QMP_status_t in qmp.h.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.12  2004/09/04 03:33:29  edwards
 *   Changed QMP_layout_grid to take a const qualifier on args.
 *
 *   Revision 1.11  2004/09/01 21:16:47  osborn
 *   Added QMP_is_initialized().
 *
 *   Revision 1.10  2004/08/26 21:32:21  detar
 *   Change QMP_master_io_node to function -CD
 *
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
#include <stdint.h>
/**
 * Current version information in string form.
 * Provided by autoconf.
 */
/* not anymore.  if needed we'll add it later. */
/* #define QMP_VERSION_STR PACKAGE_VERSION */

typedef int                QMP_bool_t;
#define QMP_TRUE           ((QMP_bool_t)1)
#define QMP_FALSE          ((QMP_bool_t)0)

/**
 * QMP function return status 
 */
typedef enum QMP_status
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
}QMP_status_t;

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

/**
 * Clear to send settings.
 */
typedef enum QMP_clear_to_send
{
  QMP_CTS_DISABLED = -1,
  QMP_CTS_NOT_READY = 0,
  QMP_CTS_READY = 1
} QMP_clear_to_send_t;

#define QMP_ALIGN_ANY     0
#define QMP_ALIGN_DEFAULT 64

#define QMP_MEM_NONCACHE  0x01
#define QMP_MEM_COMMS     0x02
#define QMP_MEM_FAST      0x04
#define QMP_MEM_DEFAULT   (QMP_MEM_COMMS|QMP_MEM_FAST)

/**
 * Memory type 
 */
typedef struct QMP_mem_struct QMP_mem_t;

/**
 * Message Memory type 
 */
typedef struct QMP_msgmem_struct * QMP_msgmem_t;

/**
 * Communicator
 */
typedef struct QMP_comm_struct * QMP_comm_t;

/**
 * Message handle
 */
typedef struct QMP_msghandle_struct * QMP_msghandle_t;

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
 * Check if QMP is initialized.
 *
 * @return QMP_TRUE if initialized otherwise QMP_FALSE.
 */
extern QMP_bool_t         QMP_is_initialized(void);

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


/***************************
 *  Communicator routines  *
 ***************************/

/**
 * Get the allocated communicator.
 */
extern QMP_comm_t QMP_comm_get_allocated(void);

/**
 * Set the allocated communicator.
 */
extern QMP_status_t QMP_comm_set_allocated(QMP_comm_t comm);

/**
 * Get the job communicator.
 */
extern QMP_comm_t QMP_comm_get_job(void);

/**
 * Set the job communicator.
 */
extern QMP_status_t QMP_comm_set_job(QMP_comm_t comm);

/**
 * Get the default communicator.
 */
extern QMP_comm_t QMP_comm_get_default(void);

/**
 * Set the default communicator.
 */
extern QMP_status_t QMP_comm_set_default(QMP_comm_t comm);

/**
 * Split a communicator into one or more disjoint communicators.
 */
extern QMP_status_t QMP_comm_split(QMP_comm_t comm, int color, int key,
				   QMP_comm_t *newcomm);

/**
 * Free a communicator.
 */
extern QMP_status_t QMP_comm_free(QMP_comm_t comm);

/**
 * Get the number of distinct colors used to create communicator
 */
extern int QMP_comm_get_number_of_colors (QMP_comm_t comm);

/**
 * Get the color used to create communicator
 */
extern int QMP_comm_get_color (QMP_comm_t comm);

/**
 * Get the key used to create communicator
 */
extern int QMP_comm_get_key (QMP_comm_t comm);

/**
 * Gget the communicator behind the scenes
 */
extern QMP_status_t QMP_get_hidden_comm(QMP_comm_t comm, void** hiddencomm);

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
 * @return number of nodes in default communicator.
 */
extern int                QMP_get_number_of_nodes (void);

/**
 * Get number of physical nodes available
 *
 * @return number of nodes in specified communicator.
 */
extern int                QMP_comm_get_number_of_nodes (QMP_comm_t comm);

/**
 * Get node number of this process
 *
 * @return node number of this process in default communicator
 */
extern int                QMP_get_node_number (void);

/**
 * Get node number of this process
 *
 * @return node number of this process in specified communicator
 */
extern int                QMP_comm_get_node_number (QMP_comm_t comm);

/**
 * Get number of jobs
 *
 * @return number of job partitions.
 */
extern int                QMP_get_number_of_jobs (void);

/**
 * Get the job number of this partition
 *
 * @return the unique jobid for this partition.
 */
extern int                QMP_get_job_number (void);

/**
 * Get number of job geometry dimensions
 *
 * @return job geometry dimensions.
 */
extern int                QMP_get_number_of_job_geometry_dimensions(void);

/**
 * Get job geometry
 *
 * @return job geometry array.
 */
extern const int *        QMP_get_job_geometry(void);

/**
 * Check whether a node is the physical root node or not in default communictor
 */
extern QMP_bool_t         QMP_is_primary_node (void);

/**
 * Check whether a node is the physical root node or not in specified communicator
 */
extern QMP_bool_t         QMP_comm_is_primary_node (QMP_comm_t comm);

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
extern int                QMP_io_node(int node);

/**
 * For binary file I/O we designate a master I/O node for the entire
 * machine.  This function defines it 
 *
 */
extern int                QMP_master_io_node(void);


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
extern QMP_status_t       QMP_declare_logical_topology (const int dims[], int ndim);

/**
 * Forces the logical topology to be a simple grid of given dimensions.
 *
 * @param communicator
 * @param dims size of each dimension.
 * @param ndim number of logical dimensions.
 * @return QMP_TRUE: if success, otherwise return QMP_FALSE
 */
extern QMP_status_t     QMP_comm_declare_logical_topology (QMP_comm_t comm,
							   const int dims[], int ndim);

/**
 * Forces the logical topology to be a simple grid of given dimensions
 *  with a given permutation map of the axes.
 *
 * @param dims size of each dimension.
 * @param ndim number of logical dimensions.
 * @return QMP_TRUE: if success, otherwise return QMP_FALSE
 */
extern QMP_status_t      QMP_declare_logical_topology_map (const int dims[], int ndim,
							   const int map[], int mapdim);

/**
 * Forces the logical topology to be a simple grid of given dimensions
 *  with a given permutation map of the axes.
 *
 * @param communicator
 * @param dims size of each dimension.
 * @param ndim number of logical dimensions.
 * @return QMP_TRUE: if success, otherwise return QMP_FALSE
 */
extern QMP_status_t QMP_comm_declare_logical_topology_map (QMP_comm_t comm,
							   const int dims[], int ndim,
							   const int map[], int mapdim);

/**
 * Check whether a logical topology is declared or not.
 *
 * @return QMP_TRUE if a logical topology is declared, 
 * otherwise return QMP_FALSE
 */
extern QMP_bool_t         QMP_logical_topology_is_declared (void);

/**
 * Check whether a logical topology is declared or not.
 *
 * @param communicator
 * @return QMP_TRUE if a logical topology is declared, 
 * otherwise return QMP_FALSE
 */
extern QMP_bool_t         QMP_comm_logical_topology_is_declared (QMP_comm_t comm);

/**
 * Get number of dimensions of logical topology.
 *
 * @return dimensionality of the logical topology. If there is no
 * logical topology, return physical topology information.
 */
extern int                QMP_get_logical_number_of_dimensions (void);

/**
 * Get number of dimensions of logical topology.
 *
 * @param communicator
 * @return dimensionality of the logical topology. If there is no
 * logical topology, return physical topology information.
 */
extern int                QMP_comm_get_logical_number_of_dimensions (QMP_comm_t comm);

/**
 * Get dimension size information for a logical topology.
 *
 * @return dimension size of the logical topology. If there is no logical 
 * topology, return information from physical geometry.
 */
extern const int*         QMP_get_logical_dimensions (void);

/**
 * Get dimension size information for a logical topology.
 *
 * @param communicator
 * @return dimension size of the logical topology. If there is no logical 
 * topology, return information from physical geometry.
 */
extern const int*         QMP_comm_get_logical_dimensions (QMP_comm_t comm);

/**
 * Get coordinate of this node within the logical topology.
 *
 * @return coordinate of this node. If no logical topology declared, 
 * return information from physical geometry.
 */
extern const int*         QMP_get_logical_coordinates (void);

/**
 * Get coordinate of this node within the logical topology.
 *
 * @param communicator
 * @return coordinate of this node. If no logical topology declared, 
 * return information from physical geometry.
 */
extern const int*         QMP_comm_get_logical_coordinates (QMP_comm_t comm);

/**
 * Get a logical coordinate from a node number.
 *
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry. Callers should free
 * memory of the returned pointer.
 */
extern int*               QMP_get_logical_coordinates_from (int node);

/**
 * Get a logical coordinate from a node number.
 *
 * @param communicator
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry. Callers should free
 * memory of the returned pointer.
 */
extern int*               QMP_comm_get_logical_coordinates_from (QMP_comm_t comm,
								 int node);

/**
 * Get a logical coordinate from a node number.
 *
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry. Callers should free
 * memory of the returned pointer.
 */
extern void               QMP_get_logical_coordinates_from2 (int coords[], int node);

/**
 * Get a logical coordinate from a node number.
 *
 * @param communicator
 * @return a coordinate. If no logical topology declared, 
 * return information from physical geometry. Callers should free
 * memory of the returned pointer.
 */
extern void             QMP_comm_get_logical_coordinates_from2 (QMP_comm_t comm,
								int coords[], int node);

/**
 * Get the node number from its logical coordinates.
 *
 * @return node number.
 */
extern int                QMP_get_node_number_from (const int coordinates[]);

/**
 * Get the node number from its logical coordinates.
 *
 * @param communicator
 * @return node number.
 */
extern int                QMP_comm_get_node_number_from (QMP_comm_t comm,
							 const int coordinates[]);


/**********************************************
 *  Problem Specification (physical lattice)  *
 **********************************************/

/**
 * General geometry constructor. Automatically determines the 
 * optimal layout -> decides the optimal ordering of axes.
 */
extern QMP_status_t       QMP_layout_grid (const int dimensions[], int ndims);

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
 * Declare a indexed memory.
 */
extern QMP_msgmem_t QMP_declare_indexed_msgmem (void* base, 
						int blocklen[],
						int index[],
						int elemsize,
						int count);

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

extern QMP_msghandle_t    QMP_comm_declare_receive_relative (QMP_comm_t comm,
							     QMP_msgmem_t m, 
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

extern QMP_msghandle_t    QMP_comm_declare_send_relative (QMP_comm_t comm,
							  QMP_msgmem_t m,
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

extern QMP_msghandle_t    QMP_comm_declare_send_to (QMP_comm_t comm,
						    QMP_msgmem_t m, 
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

extern QMP_msghandle_t    QMP_comm_declare_receive_from(QMP_comm_t comm,
							QMP_msgmem_t m, 
							int rem_node_rank,
							int priority);

extern QMP_status_t QMP_change_address(QMP_msghandle_t msg, void *addr);
extern QMP_status_t QMP_change_address_multiple(QMP_msghandle_t msg, void *addr[], int naddr);

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

extern QMP_msghandle_t QMP_declare_send_recv_pairs(QMP_msghandle_t msgh[], 
						   int num);

/**
 * Declare a recv buffer ready for next message.
 *
 * @param mh a message handle.
 * @return QMP_SUCCESS if now ready for message.
 */
extern QMP_status_t QMP_clear_to_send(QMP_msghandle_t mh, QMP_clear_to_send_t cts);

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

extern QMP_status_t       QMP_comm_barrier (QMP_comm_t comm);

/**
 * Broadcast bytes from a node. This routine is a blocking routine.
 *
 * @param buffer a pointer to a memory buffer.
 * @param nbytes size of the buffer.
 *
 * @return QMP_SUCCESS for a successful broadcasting.
 */
extern QMP_status_t       QMP_broadcast (void* buffer, size_t nbytes);

extern QMP_status_t       QMP_comm_broadcast (QMP_comm_t comm,
					      void* buffer, size_t nbytes);

/**
 * Global in place sum of an integer 
 * @param value a pointer to a integer.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_int (int *value);

extern QMP_status_t       QMP_comm_sum_int (QMP_comm_t comm, int *value);

/**
 * Global in place sum of an explicit uint64_t
 * @param value a pointer to a uint64_t
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_uint64_t (uint64_t *value);


extern QMP_status_t       QMP_comm_sum_uint64_t (QMP_comm_t comm, uint64_t *value);


/**
 * Global in place sum of a float.
 * @param value a pointer to a float.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_float (float *value);

extern QMP_status_t       QMP_comm_sum_float (QMP_comm_t comm, float *value);

/**
 * Global in place sum of a double.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_double (double *value);

extern QMP_status_t       QMP_comm_sum_double (QMP_comm_t comm, double *value);

/**
 * Global in place sum of a long double.
 * @param value a pointer to a long double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_long_double (long double *value);

extern QMP_status_t       QMP_comm_sum_long_double (QMP_comm_t comm, long double *value);

/**
 * Global in place sum of a double. Intermediate values kept in extended
 * precision if possible.
 * @param value a pointer to a double.
 *
 * @return QMP_SUCCESS when a global sum is success. 
 */
extern QMP_status_t       QMP_sum_double_extended (double *value);

extern QMP_status_t       QMP_comm_sum_double_extended (QMP_comm_t comm,
							double *value);

/**
 * Global in place sum of a float array.
 * @param value a pointer to a float array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t       QMP_sum_float_array (float value[], int length);

extern QMP_status_t       QMP_comm_sum_float_array (QMP_comm_t comm,
						    float value[], int length);

/**
 * Global in place sum of a double array.
 * @param value a pointer to a double array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t       QMP_sum_double_array (double value[], int length);

extern QMP_status_t       QMP_comm_sum_double_array (QMP_comm_t comm,
						     double value[], int length);

/**
 * Global in place sum of a long double array.
 * @param value a pointer to a long double array.
 * @param length size of the array.
 *
 * @return QMP_SUCCESS when the global sum is a success.
 */
extern QMP_status_t       QMP_sum_long_double_array (long double value[], int length);

extern QMP_status_t       QMP_comm_sum_long_double_array (QMP_comm_t comm,
							  long double value[], int length);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t       QMP_max_float (float* value);

extern QMP_status_t       QMP_comm_max_float (QMP_comm_t comm, float* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t       QMP_max_double (double* value);

extern QMP_status_t       QMP_comm_max_double (QMP_comm_t comm, double* value);

/**
 * Get maximum value of all floats.
 */
extern QMP_status_t       QMP_min_float (float* value);

extern QMP_status_t       QMP_comm_min_float (QMP_comm_t comm, float* value);

/**
 * Get maximum value of all doubles.
 */
extern QMP_status_t       QMP_min_double (double* value);

extern QMP_status_t       QMP_comm_min_double (QMP_comm_t comm, double* value);

/**
 * Get the exclusive ored value of an unsigned long integer
 */
extern QMP_status_t       QMP_xor_ulong (unsigned long* value);

extern QMP_status_t       QMP_comm_xor_ulong (QMP_comm_t comm, unsigned long* value);

/**
 * Transposition
 */
extern QMP_status_t       QMP_comm_alltoall(QMP_comm_t comm, char* recvbuffer, char* sendbuffer, int count);

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

extern QMP_status_t       QMP_comm_binary_reduction (QMP_comm_t comm,
						     void* lbuffer, size_t buflen,
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

/**
 *  Profiling function declarations
 */

extern double QMP_time(void);
extern void   QMP_reset_total_qmp_time(void);
extern double QMP_get_total_qmp_time(void);

/**
 *  Version information
 */

extern const char * QMP_version_str(void);
extern int QMP_version_int(void);

#ifdef __cplusplus
}

/* #include "qmp.hpp" */

#endif

/* Using a trick supplied by Stackoverflow:
 * https://stackoverflow.com/questions/3599160/how-to-suppress-unused-parameter-warnings-in-c/12891181
 * Thanks to several people who suggested it in that thread
 */
#define  _QMP_UNUSED_ARGUMENT(x) ((void)(x))
#endif
