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
 *      Private header file for implementation only. 
 *      Users do not need this header file.
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.10  2003/01/08 20:37:47  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.9  2002/12/05 16:41:01  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.8  2002/11/15 15:37:32  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.7  2002/10/03 16:46:34  chen
 *   Add memory copy, change event loops
 *
 *   Revision 1.6  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.3  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.2  2002/04/22 20:28:40  chen
 *   Version 0.95 Release
 *
 *   Revision 1.1  2002/03/28 18:48:19  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.6  2002/02/15 20:34:52  chen
 *   First Beta Release QMP
 *
 *   Revision 1.5  2002/01/27 20:53:49  chen
 *   minor change
 *
 *   Revision 1.4  2002/01/27 17:38:57  chen
 *   change of file structure
 *
 *   Revision 1.3  2002/01/24 20:10:49  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.2  2002/01/22 15:52:29  chen
 *   Minor change in code
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:50  chen
 *   initial import of QMP
 *
 *
 */
#ifndef _QMP_P_GM_H
#define _QMP_P_GM_H

#include <gm.h>

/**
 * Make sure number of GM ports is the same as the one
 * defined inside gm_types.h
 */
#define GM_NUM_PORTS 32

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/**
 * Memory alignment size (SSE)
 */
#define QMP_MEM_ALIGNMENT 16


/**
 * A function pointer for releasing user allocated memory.
 */
typedef void (*QMP_mem_free) (void *memptr);

/**
 * Memory type
 */
typedef enum QMP_mm_type
{
  QMP_MM_LEXICO_BUF = 0,
  QMP_MM_STRIDED_BUF,
  QMP_MM_USER_BUF
}QMP_mm_type_t;

/**
 * Memory handle type 
 */
typedef enum QMP_mh_type
{
  QMP_MH_SEND = 0x0001,
  QMP_MH_RECV = 0x0002,
  QMP_MH_BROADCAST = 0x0004,
  QMP_MH_MULTIPLE = 0x1000,
  QMP_MH_QUEUE = 0x2000,
}QMP_mh_type_t;

/**
 * Memory handle state.
 */
typedef enum QMP_mh_state
{
  QMP_MH_INVALID = 0,
  QMP_MH_IDLE,
  QMP_MH_WAITING
}QMP_mh_state_t;


/**
 * Channel state.
 */
typedef enum QMP_ch_state
{
  QMP_NOT_CONNECTED = 0x0001,
  QMP_CONNECTING = 0x0002,
  QMP_CONNECTION_PENDING = 0x0004,
  QMP_CONNECTED = 0x0008
}QMP_ch_state_t;

/**
 * forward decleration of QMP_machine_t.
 */
typedef struct QMP_machine QMP_machine_t;

/**
 * forward decleration of QMP_msghandle_i_t;
 */
typedef struct QMP_msghandle_i QMP_msghandle_i_t;

/**
 * Relative Direction for Messages.
 */
typedef enum QMP_dir
{
  QMP_DIRXP = 0,
  QMP_DIRXM,
  QMP_DIRYP,
  QMP_DIRYM,
  QMP_DIRZP,
  QMP_DIRZM,
  QMP_DIRTP,
  QMP_DIRTM,
  QMP_UNKNOWN
}QMP_dir_t;

/**
 * Forward decleration of physical geometry.
 */
typedef struct QMP_phys_geometry QMP_phys_geometry_t;

/**
 * Remote node
 */
typedef struct QMP_rem_node
{
  /* type of this remote node: if type == QMP_UNKNOWN,      */
  /* this node is not a neighbor of my process.             */
  QMP_dir_t type;

  /* logic id or rank of this node.                         */
  QMP_u32_t logic_rank;

  /* physical id or rank of this remote node                */
  QMP_phys_geometry_t* phys;

}QMP_rem_node_t;

/**
 * General message passing protocol.
 */
typedef struct QMP_general_msg_header
{
  /* magic number of this system                            */
  QMP_u16_t magic;

  /* version of this system                                 */
  QMP_u16_t vers;

  /* operation of this msg                                  */
  QMP_u16_t op;

  /* source id  (we will have node number < 65535)          */
  QMP_u16_t  source;

  /* dest id                                                */
  QMP_u16_t  dest;

  /* data type (force it to be 8 bit number)                */
  QMP_u8_t  data_type;

  /* additional operation field for channel connection      */
  /* this field is only used in single port implementation  */
  QMP_u8_t adop;

  /* number of elements                                     */
  QMP_u32_t count;

}QMP_general_msg_header_t;

/**
 * General Message Queue Structure.
 */
typedef struct QMP_msg_queue_slot
{
  /* Protocol header                                        */
  QMP_general_msg_header_t hdr;

  /* type of this queue slot (send / receive)               */
  QMP_mh_type_t            type;

  /* State of this queue slot                               */
  QMP_mh_state_t           state;

  /* priority of this message                               */
  QMP_u32_t                pri;

  /* Buffer size                                            */
  QMP_u32_t                buflen;

  /* Memory buffer                                          */
  char*                    buffer;

  /* global machine pointer                                 */
  QMP_machine_t*           glm;

#ifndef _QMP_USE_MULTI_PORTS
  /* The message handle creating this queue slot            */
  QMP_msghandle_i_t*       mh;
#endif

  /* Next link to a queue slot                              */
  struct QMP_msg_queue_slot* next;

}QMP_msg_queue_slot_t;

/**
 * Channel id or send tag
 */
#define QMP_ID_INIT_VALUE  2048
#define QMP_ID_FINAL_VALUE 65234

#define QMP_START_TAG QMP_ID_INIT_VALUE
#define QMP_END_TAG   QMP_ID_FINAL_VALUE
#define QMP_TAG_WILDCARD    65235

#define QMP_CH_CONNECT      0xcd
#define QMP_CH_DONE         0xef

/**
 * The following definitions for implementation using
 * separate port to communicate.
 */
#ifdef _QMP_USE_MULTI_PORTS

/**
 * Simple macro to check whether a received message slot match
 * receiving critera.
 */
#define QMP_RECVED_MATCH(p,count,datatype,src_id,operation,prio) (p->pri==prio && p->hdr.count == count && p->hdr.data_type == datatype && p->hdr.source == src_id && p->hdr.op == operation) 

/**
 * Simple macro to match a message hdr to a provided receiving buffer queue.
 */
#define QMP_PROVIDED_MATCH(p,hdr,pri) (p->pri == pri && p->hdr.count == hdr->count && p->hdr.data_type == hdr->data_type && p->hdr.source == hdr->source && p->hdr.dest == hdr->dest && p->hdr.op == hdr->op)



/**
 * This is memory size threshold below which memory copies will be used.
 */
#define QMP_MEM_COPY_THRESHOLD 24576

/**
 * Host memory segments to receive user data before being copied 
 * to user memory. This is a correct algorithm but in host memory.
 * Those memory segments should be in Lanai card memory eventually.
 */
typedef struct QMP_buffer_mem_
{
  /* real memory pointer                          */
  void*                   mem;
  /* pointer to the message handle holding this   */
  QMP_msghandle_i_t*      mh;
  /* index to an array of buffers                 */
  QMP_u16_t               idx;
  /* whether this buffer is done operation (send) */
  QMP_u16_t               done;
  /* link to next and previous buffers            */
  struct QMP_buffer_mem_* next;
  struct QMP_buffer_mem_* prev;
}QMP_buffer_mem_t;

/**
 * Some macros to manipulate buffer list.
 */
#define DEQUEUE_DATA_LIST(mem) {                      \
    if (mem->data_head != mem->data_tail) {           \
      mem->data_tail->prev->next = 0;                 \
      mem->data_tail = mem->data_tail->prev;          \
    }                                                 \
    else {                                            \
      mem->data_head = 0;                             \
      mem->data_tail = 0;                             \
    }                                                 \
}

#define REMOVE_DATA_BUFFER(mem,buffer) {              \
    if (mem->data_head != mem->data_tail){            \
       if (buffer == mem->data_head) {                \
          buffer->next->prev = 0;                     \
          mem->data_head = buffer->next;              \
       }                                              \
       else if (buffer == mem->data_tail){            \
          buffer->prev->next = 0;                     \
          mem->data_tail = buffer->prev;              \
       }                                              \
       else {                                         \
          buffer->prev->next = buffer->next;          \
          buffer->next->prev = buffer->prev;          \
       }                                              \
     }                                                \
     else                                             \
       mem->data_head = mem->data_tail = 0;           \
}

#define DEQUEUE_NULL_LIST(mem) {                      \
    if (mem->null_head != mem->null_tail) {           \
      mem->null_tail->prev->next = 0;                 \
      mem->null_tail = mem->null_tail->prev;          \
    }                                                 \
    else {                                            \
      mem->null_head = 0;                             \
      mem->null_tail = 0;                             \
    }                                                 \
}

#define ENQUEUE_DATA_LIST(mem,sbuffer) {              \
    if (mem->data_head) {                             \
      sbuffer->next = mem->data_head;                 \
      mem->data_head->prev = sbuffer;                 \
      sbuffer->prev = 0;                              \
      mem->data_head = sbuffer;                       \
    }                                                 \
    else {                                            \
      sbuffer->next = sbuffer->prev = 0;              \
      mem->data_head = mem->data_tail = sbuffer;      \
    }                                                 \
}

#define ENQUEUE_NULL_LIST(mem,sbuffer) {              \
    if (mem->null_head) {                             \
      sbuffer->next = mem->null_head;                 \
      mem->null_head->prev = sbuffer;                 \
      sbuffer->prev = 0;                              \
      mem->null_head = sbuffer;                       \
    }                                                 \
    else {                                            \
      sbuffer->next = sbuffer->prev = 0;              \
      mem->null_head = mem->null_tail = sbuffer;      \
    }                                                 \
}

#define IS_DATA_LIST_EMPTY(mem) (!mem->data_head && !mem->data_tail)

/**
 * Number of shadow buffers for each message memory
 */
#define QMP_NUM_SHADOW_BUFFERS 2

/**
 * Internal message memory structure
 */
typedef struct QMP_msgmem_i
{
  QMP_mm_type_t      type;
  void*              mem;
  QMP_u32_t          nbytes;
  QMP_u32_t          ref_count;
  QMP_machine_t*     glm;

  /* Flag to find out whether we have shadow buffers */
  QMP_bool_t         do_mem_copy;
  /* number of buffers to receive data from LaNai    */
  QMP_buffer_mem_t   buffers[QMP_NUM_SHADOW_BUFFERS]; 
  /* Buffers have data received. Data can be copied  */
  QMP_buffer_mem_t  *data_head, *data_tail;
  /* Buffers have nothing, user can copy data into   */
  QMP_buffer_mem_t  *null_head, *null_tail;
}QMP_msgmem_i_t;

/**
 * Channel Structure.
 */
typedef struct QMP_channel
{
  /* whether this channel is established or not.            */
  QMP_ch_state_t state;

  /* my node information                                    */
  QMP_rem_node_t my_node;

  /* remote node information                                */
  QMP_rem_node_t rem_node;

  /* 32 bit id assigned to this message handle              */
  QMP_u32_t      id;

  /* number of packet sent/received                         */
  QMP_u32_t      cnt;

  /* gm new port structure                                  */
  struct gm_port* my_port;

  /* my device                                              */
  QMP_u32_t      my_device;
  
  /* my port num                                            */
  QMP_u32_t      my_port_num;
  
  /* node id of this process                                */
  QMP_u32_t      my_node_id;

  /* gm message size: sender should have msg size < recver  */
  QMP_u32_t      gm_msg_size;

  /* remote port number                                     */
  QMP_u32_t      rem_port_num;

  /* remote node id                                         */
  QMP_u32_t      rem_node_id;

}QMP_channel_t;

/**
 * Message handle
 */
struct QMP_msghandle_i
{
  /* message handle type                                    */
  QMP_mh_type_t   type;

  /* message handle state                                   */
  QMP_mh_state_t  state;

  /* real memory information                                */
  QMP_msgmem_i_t* memory;

  /* channel pointer for this message handle                */
  QMP_channel_t   ch;

  /* Message passing process                                */
  QMP_machine_t* glm;

  /* operation error code on this message handle            */
  QMP_status_t   err_code;

  /* Linked list for message handles                        */
  /* This linked list is used by the QMP machine            */
  struct QMP_msghandle_i* next;

  /* Linked list for collection or multiple message handles */
  struct QMP_msghandle_i* m_next;

};

/**
 * Channel Control Message QMP
 */
typedef struct QMP_ch_ctrlmsg
{
  /* magic number of this system                            */
  QMP_u16_t magic;
  /* version of this system                                 */
  QMP_u16_t vers;
  /* operation of this msg                                  */
  QMP_u32_t op;
  /* status of this msg                                     */
  QMP_u32_t status;

  /* remote port number of a ch                             */
  QMP_u32_t gm_port_num;
  /* remote node id of a ch                                 */
  QMP_u32_t gm_node_id;
  /* remote memory gm size                                  */
  QMP_u32_t gm_size;
  /* remote allocated memory length                         */
  QMP_u32_t mem_len;
  /* remote node logic rank                                 */
  QMP_u32_t logic_rank;
  /* which direction this msg coming from                   */
  QMP_u32_t direction;
  /* a unique id to identify this msg and msg handle        */
  QMP_u32_t id;

}QMP_ch_ctrlmsg_t;


/**
 * Local control message structure.
 */
typedef struct QMP_local_ctrlmsg
{
  /* DMA malloced control message that is sent over network */
  QMP_ch_ctrlmsg_t*  ctrlmsg;

  /* Reference count                                        */
  QMP_u32_t          ref_count;

  /* Current machine instance                               */
  QMP_machine_t*     glm;


  /* Current message handle we are dealing with             */
  QMP_msghandle_i_t* mh;


  /* pointer to next this type of object.                   */
  struct QMP_local_ctrlmsg* next;

}QMP_local_ctrlmsg_t;

#else /* #ifdef _QMP_USE_MULTI_PORTS */

/**
 * Simple macro to check whether a received message slot match
 * receiving critera. p is received slot, the rest arguments
 * are user provided requirments
 */
#define QMP_RECVED_MATCH(p,count,datatype,src_id,operation,prio) (p->pri==prio && p->hdr.count == count && p->hdr.data_type == datatype && p->hdr.source == src_id && (p->hdr.op == operation || (operation == QMP_TAG_WILDCARD && p->hdr.op >= QMP_START_TAG && p->hdr.adop == QMP_CH_CONNECT)))

/**
 * Simple macro to match a message hdr to a provided receiving buffer queue p
 */
#define QMP_PROVIDED_MATCH(p,hdr,pri) (p->pri == pri && p->hdr.count == hdr->count && p->hdr.data_type == hdr->data_type && p->hdr.source == hdr->source && p->hdr.dest == hdr->dest && (p->hdr.op == hdr->op || (p->hdr.op == QMP_TAG_WILDCARD && hdr->op >= QMP_START_TAG && hdr->adop == QMP_CH_CONNECT )))

/**
 * Internal message memory structure
 */
typedef struct QMP_msgmem_i
{
  QMP_mm_type_t      type;
  void*              mem;
  QMP_u32_t          nbytes;
  QMP_u32_t          ref_count;
  QMP_machine_t*     glm;
}QMP_msgmem_i_t;


/**
 * Message handle
 */
struct QMP_msghandle_i
{
  /* message handle type                                    */
  QMP_mh_type_t   type;

  /* message handle state                                   */
  QMP_mh_state_t  state;

  /* real memory information                                */
  QMP_msgmem_i_t* memory;

  /* source node                                            */
  QMP_u32_t       source;

  /* destination node                                       */
  QMP_u32_t       dest;

  /* tag to distinguish different send/recv                 */
  QMP_u32_t       tag;

  /* number of messages transmitted using this tag          */
  QMP_u32_t       cnt;

  /* receiving and sending queue slot                       */
  QMP_msg_queue_slot_t* req;  

  /* Message passing process                                */
  QMP_machine_t* glm;
 
  /* operation error code on this message handle            */
  QMP_status_t   err_code;

  /* Linked list for message handles                        */
  /* This linked list is used by the QMP machine            */
  struct QMP_msghandle_i* next;

  /* Linked list for collection or multiple message handles */
  struct QMP_msghandle_i* m_next;
}; 
  

#endif  /* _QMP_USE_MULTI_PORTS */


/**
 * Runtime Environment Variables for QMP system.
 */
#define QMP_OPTS_ENV  (const char *)"QMP_OPTS"
#define QMP_CONF_ENV  (const char *)"QMP_CONF"

/**
 * GM Receiving Message Buffer Size (Log2(size)) == max msg length
 */
#define QMP_GENMSG_SIZE   16
#define QMP_GENMSG_NBUF   64
#define QMP_CRLMSG_SIZE   8
#define QMP_CRLMSG_NBUF   8

/**
 * QMP Messaging environment.
 */
#define QMP_HOSTNAME_LEN    128
typedef struct QMP_rtenv_entry
{
  QMP_u32_t   rank;
  char        host[QMP_HOSTNAME_LEN];
  QMP_u32_t   device;
  QMP_u32_t   port;
  QMP_u32_t   node_id;
}QMP_rtenv_entry_t, *QMP_rtenv_t;

/**
 * Default name for QMP configuration file (HOME Directory)
 */
#define QMP_DEFAULT_CONF ".qmpgmrc"

/**
 * Physical geometry structure
 *
 * Now switched configuration is the one we support.
 */
typedef struct QMP_switched_geometry
{
  /* physical id or rank of this node                       */
  QMP_u32_t phys_rank;

}QMP_switched_geometry_t;

/**
 * Physical geometry structure used by others.
 */
struct QMP_phys_geometry
{
  /* Type of this physical geometry: SWITCH, GRID etc.      */
  QMP_ictype_t type;

  /* total number of nodes available                        */
  QMP_u32_t    num_nodes;

  /* The following union contains structure for different   */
  /* network configuration.                                 */
  union {
    QMP_switched_geometry_t switched_geometry;
  }geometry;

};

#ifdef _QMP_IC_SWITCH
#define QMP_PHYS_RANK(g)  (g->geometry.switched_geometry.phys_rank)
#define QMP_NUM_NODES(g)  (g->num_nodes)
#else
#error "QMP only supports switched configuration"
#endif


/**
 * Simple Logical Grid Topology
 */

/**
 * Maximum number of dimension for a logical grid.
 */
#define QMP_NMD 4

typedef struct QMP_topology
{
  /* Dimension of  geometry                                */
  QMP_u32_t  dimension;

  /* size of this geometry                                 */
  QMP_u32_t  size[QMP_NMD];

  /* Total number of nodes                                 */
  QMP_u32_t  num_nodes;

  /* coordinates inside this geometry                      */
  QMP_u32_t  coordinates[QMP_NMD];

  /* Logical rank of this node.                            */
  QMP_u32_t  logic_rank;

  /* correspoing physical geometry information             */
  QMP_phys_geometry_t*  phys;

  /* Index mapping logical lattice axes to physical axes   */
  QMP_u32_t  ordering[QMP_NMD];

  /* Logical Neighboring information                       */
  /* 2 neighbors for each dimension                        */
  QMP_rem_node_t neighbors[QMP_NMD][2];

}QMP_topology_t;

/**
 * GM uses port 0, 1, 3 for previliged operations. port 2 is 
 * used by QMP for control messages.
 *
 * The following defines a port below which ports are not usable.
 */
#define QMP_GM_PREVI_PORTS 4

/**
 * Machine configuration information.
 */
struct QMP_machine
{
  /* number of processors.                                   */
  QMP_u16_t  num_cpus;

  /* do we count those processors or user manage them.       */
  /* if collapse_smp == 1. this machine is viewed as a       */
  /* single cpu.                                             */
  QMP_bool_t collapse_smp;

  /* interconnection type for this machine.                  */
  QMP_ictype_t ic_type;

  /* Physical geometry for this parallel job.                */
  QMP_phys_geometry_t* phys;
  
  /* host name of this machine.                              */
  char*        host;

  /* whether this machine is initialized                     */
  QMP_bool_t inited;

  /* logical topology information of this machine.           */
  QMP_topology_t* tpl;                          

  /* GM board information.                                   */
  struct gm_port *port;

  /* device id.                                              */
  /* first board is 0, second board is 1, etc...             */
  QMP_u32_t device;

  /* port number. user process starts using 2, 4, 5, 6, 7    */
  QMP_u32_t port_num;

  /* name associate with this port.                          */
  char* port_name;

  /* node id of this machine.                                */
  QMP_u32_t node_id;

  /* maximum length this messaging system supports for now   */
  QMP_u32_t max_msg_length;

#ifdef _QMP_USE_MULTI_PORTS
  /* active port map for this machine                        */
  /* this is an array of size GM_NUM_PORTS. If an array      */
  /* element at index i has value 1, this port i is active.  */
  /* If an array element at index i has value 0, the port i  */
  /* can be used by my application.                          */
  QMP_u32_t active_ports[GM_NUM_PORTS];

  /* receiving buffers for reguler msgs                      */
  void* ctrl_msg_buffers[QMP_CRLMSG_NBUF];
#endif

  /* receiving buffers for the receiving queue               */
  void* gen_msg_buffers[QMP_GENMSG_NBUF];

  /* primitive type sender queue free list                   */
  QMP_msg_queue_slot_t* send_free_list;

  /* primitive type receiving queue free list                */
  QMP_msg_queue_slot_t* recving_free_list;

  /* primitive type received queue free list                 */
  QMP_msg_queue_slot_t* recved_free_list;  

  /* user provided buffer to receive messages                */
  QMP_msg_queue_slot_t* provided_buffer_queue;

  /* received general messages which have not been claimed   */
  QMP_msg_queue_slot_t* received_queue;

  /* pending send queue                                      */
  QMP_msg_queue_slot_t* pending_send_queue;

  /* gm send tokens (managed by QMP to speed up applications)*/
  QMP_u32_t send_token_cnt[GM_NUM_PRIORITIES];
  
  /* run time environment of this machine                    */
  QMP_rtenv_t  rtenv;
  QMP_u32_t    num_rtenv_entries;

  /* all message handles                                     */
  QMP_msghandle_i_t* msg_handles;

  /* message handle id                                       */
  QMP_u32_t          chid;


#ifdef _QMP_USE_MULTI_PORTS
  /* Pending connection requests from senders                */
  /* which are pre_created receiving message handles when    */
  /* there are no matching receiving handles that a sender   */
  /* is looking for.                                         */
  QMP_msghandle_i_t* conn_handles;

  /* a local control message that is used to send info       */
  /* when an error                                           */
  QMP_local_ctrlmsg_t* err_ctrlmsg;
#endif

  /* the last function status or error code                  */
  QMP_status_t err_code;
};

/**
 * Macros to manage gm send tokens
 */
#define QMP_HAS_SEND_TOKENS(glm, pri) (glm->send_token_cnt[pri] > 0)
#define QMP_GIVEUP_SEND_TOKEN(glm, pri) (glm->send_token_cnt[pri]--)
#define QMP_ACQUIRE_SEND_TOKEN(glm, pri)(glm->send_token_cnt[pri]++)


/**
 * Protocol Operation Field
 */
#define QMP_CH_CONN            (QMP_u16_t)1001
#define QMP_CH_CONN_RPLY       (QMP_u16_t)1002
#define QMP_CH_DISC            (QMP_u16_t)1003
#define QMP_CH_DISC_RPLY       (QMP_u16_t)1004
#define QMP_QUEUE              (QMP_u16_t)1100
#define QMP_REDUCE             (QMP_u16_t)1150
#define QMP_BCAST              (QMP_u16_t)1151
#define QMP_ANY                (QMP_u16_t)1200

/**
 * QMP Global Operation Operator
 */
typedef enum QMP_op
{
  QMP_SUM,
  QMP_MAX,
  QMP_MIN,
  QMP_XOR,
}QMP_op_t;

/**
 * Function pointer for global operation.
 * dest = source op dest
 */
typedef void  (*QMP_opfunc) (void* dest, void* source1, void* source2,
			     QMP_u32_t count, QMP_datatype_t type);

/**
 * Structure mapping QMP_op to real operation
 */
typedef struct QMP_rop
{
  QMP_op_t    op;
  QMP_bool_t  commute;
  QMP_opfunc  func;
}QMP_rop_t;

/**
 * Magic number
 */
#define QMP_MAGIC        (QMP_u16_t)0x9cdf

/**
 * GM message priority
 */
#define QMP_MSG_PRIORITY       GM_HIGH_PRIORITY
#define QMP_CHMSG_PRIORITY     GM_LOW_PRIORITY
#define QMP_QMSG_PRIORITY      GM_LOW_PRIORITY

/**
 * Number of loops per second for empty gm_receive call on P4 machine.
 */
#define QMP_NUMLOOPS_PSEC    40000000 

/**
 * SPIN loops for a particular message handle
 */
#define QMP_MSG_MINI_SPIN_LOOPS 4000

/**
 * Timeout value = 30 seconds for control message
 */
#define QMP_CTRLMSG_WAIT_TIMEOUT 30

/**
 * Timeout value = 5 seconds for disconnection control message
 */
#define QMP_DISC_CTRLMSG_TIMEOUT 5

/**
 * Time out value = 50 seconds for regular message
 */
#define QMP_MSG_WAIT_TIMEOUT 50

/**
 * Time out value for individual wait inside a multiple wait
 */
#define QMP_SINGLE_WAIT_TIMEOUT 2

/**
 * Define gm_ntoh macros for older version of GM
 */
#ifndef GM_API_VERSION
#define gm_ntoh_u32 gm_ntohl
#define gm_ntoh_u8  gm_ntohc
#endif

/**
 * data size table for a QMP data type.
 */
extern QMP_u32_t QMP_data_size_table[];

#define QMP_DATA_SIZE(type) (QMP_data_size_table[type])

/**
 * QMP operator table.
 */
extern QMP_rop_t QMP_operator_table[];

#define QMP_OPERATOR(op) (QMP_operator_table[op])

/**
 * One global instance of this machine.
 */
extern QMP_machine_t QMP_global_m;

/**
 * verbose mode of this run.
 */
extern QMP_bool_t    QMP_rt_verbose;


/**
 * Trace macro
 */
#ifdef _QMP_TRACE
#define QMP_TRACE(x) (QMP_fprintf(stderr, "%s at line %d of file %s\n", x, __LINE__, __FILE__))
#else
#define QMP_TRACE(x)
#endif

/**
 * Get and set error code macros.
 */
#define QMP_SET_STATUS_CODE(glm,code) (glm->err_code = code)

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Some utility functions used inside implementations.
 */
extern void  QMP_print_rtenv (QMP_rtenv_t rtenv, QMP_u32_t size);
extern void  QMP_delete_topology  (QMP_topology_t *topology);
extern void  QMP_print_topology   (QMP_topology_t* tpl);
extern const QMP_u32_t QMP_get_logical_node_number_for_neighbor (QMP_dir_t direction);
extern QMP_phys_geometry_t *QMP_physical_from_logical (QMP_u32_t logic_rank);
extern QMP_bool_t QMP_declare_ordered_logical_topology (const QMP_u32_t *dims, 
							QMP_u32_t ndim, 
							QMP_u32_t ordering[]);


/**
 * Convert a physical node number to a logical node number. 
 * For switched configuration, they are the same.
 *
 * @return logical node number.
 */
extern const QMP_u32_t    QMP_allocated_to_logical (QMP_u32_t node);

/**
 * Convert a logical node number to a physical node number.
 *
 * @return a physical node number.
 */
extern const QMP_u32_t    QMP_logical_to_allocated (QMP_u32_t logic_rank);

extern const QMP_u32_t    QMP_get_logical_number_of_nodes (void);

extern const QMP_u32_t    QMP_get_logical_node_number (void);

extern void  QMP_print_machine_info (QMP_machine_t* glm);
extern const QMP_rtenv_t QMP_get_machine_info (QMP_machine_t* glm,
					       QMP_phys_geometry_t* geo);

extern const QMP_rtenv_t QMP_get_machine_info_by_id (QMP_machine_t* glm,
						     QMP_u32_t id);

extern void *QMP_memalign (QMP_u32_t alignment, 
			   QMP_u32_t size);

extern QMP_u32_t  QMP_find_gm_port (QMP_machine_t* glm);
extern void       QMP_set_gm_port (QMP_machine_t* glm, QMP_u32_t port);
extern void       QMP_clear_gm_port (QMP_machine_t* glm, QMP_u32_t port);

extern QMP_bool_t QMP_memory_declared (QMP_machine_t* glm, 
				       QMP_msgmem_i_t* mem);
extern QMP_bool_t QMP_add_msg_handle (QMP_machine_t* glm,
				      QMP_msghandle_i_t *mh);
extern QMP_bool_t QMP_remove_msg_handle (QMP_machine_t* glm, 
					 QMP_msghandle_i_t *mh);

#ifdef _QMP_USE_MULTI_PORTS
extern QMP_msghandle_i_t *QMP_create_conn_recv_msghandle (QMP_machine_t* glm,
							  QMP_dir_t d,
							  QMP_u32_t rem_rank);

extern QMP_local_ctrlmsg_t *QMP_create_ctrlmsg (QMP_machine_t* glm,
						QMP_msghandle_i_t* mh,
						QMP_u32_t init_refcount,
						QMP_local_ctrlmsg_t* next);
extern void QMP_delete_ctrlmsg (QMP_local_ctrlmsg_t* lcm,
				QMP_bool_t force);


extern void QMP_add_pending_conn_req (QMP_machine_t* glm, 
				      QMP_msghandle_i_t* req);

extern void QMP_remove_pending_conn_req (QMP_machine_t* glm, 
					 QMP_msghandle_i_t* req);

extern QMP_status_t QMP_init_wait_for_gm_connection (QMP_machine_t* glm,
						     QMP_msghandle_i_t* mh);

extern QMP_status_t QMP_wait_for_receiver_disconnect (QMP_machine_t* glm, 
						      QMP_msghandle_i_t* mh);

extern QMP_status_t QMP_wait_for_send_to_finish (QMP_machine_t* glm, 
						 QMP_msghandle_i_t* mh);

extern QMP_status_t QMP_close_receiver_connection (QMP_machine_t* glm,
						   QMP_msghandle_i_t *mh);
#endif

extern void QMP_create_primitive_free_list (QMP_machine_t* glm);

extern void QMP_delete_primitive_free_list (QMP_machine_t* glm);

extern void QMP_print_genmsg_header (QMP_general_msg_header_t* hdr);

extern void QMP_print_msg_queue_slot (QMP_msg_queue_slot_t* qs);

extern QMP_msg_queue_slot_t *QMP_create_msg_queue_slot (void* buffer, 
							QMP_u32_t count, 
							QMP_datatype_t dtype, 
							QMP_u32_t dest_id,  
							QMP_u32_t operation, 
							QMP_u32_t priority,
							QMP_u8_t  adop,
							QMP_bool_t sender,
							QMP_bool_t queued,
							QMP_machine_t* glm);

extern void QMP_delete_msg_queue_slot (QMP_msg_queue_slot_t* qs);

extern QMP_msg_queue_slot_t* QMP_match_received_msg (void* buffer, 
                          QMP_u32_t count, 
			  QMP_datatype_t datatype, QMP_u32_t src_id,
			  QMP_u32_t operation, QMP_u32_t priority,
		          QMP_machine_t* glm);

extern void QMP_add_provided_receiving_buffer (QMP_machine_t* glm,
					       QMP_msg_queue_slot_t* qs);

extern void QMP_remove_provided_receiving_buffer (QMP_machine_t* glm,
						   QMP_msg_queue_slot_t* qs);

extern void QMP_add_pending_send_buffer (QMP_machine_t* glm,
					 QMP_msg_queue_slot_t* qs);

extern void QMP_remove_pending_send_buffer (QMP_machine_t* glm,
					    QMP_msg_queue_slot_t* qs);

extern QMP_bool_t QMP_match_provided_buffer(QMP_general_msg_header_t* hdr, 
					    void* buffer, 
					    QMP_u32_t length, 
					    QMP_u32_t pri, 
					    QMP_machine_t* glm);

extern void QMP_add_received_msg (QMP_general_msg_header_t* hdr, 
				  void* buffer, 
				  QMP_u32_t length, 
				  QMP_u32_t pri, 
				  QMP_machine_t* glm);

/**
 * Print out information.
 */
extern int   QMP_info             (const char *format, ...);
extern int   QMP_error            (const char *format, ...);

#ifdef __cplusplus
}
#endif


#endif
