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
 *      Some utility functions used by other parts of QMP
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.10  2003/01/08 20:37:49  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.9  2002/12/05 16:41:04  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.8  2002/11/15 15:37:34  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.7  2002/07/25 13:37:36  chen
 *   Fix a bug with declare multiple
 *
 *   Revision 1.6  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.5  2002/03/28 18:48:21  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.4  2002/03/27 20:48:50  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.3  2002/02/15 20:34:55  chen
 *   First Beta Release QMP
 *
 *   Revision 1.2  2002/01/27 20:53:51  chen
 *   minor change
 *
 *   Revision 1.1  2002/01/27 17:38:58  chen
 *   change of file structure
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP.h"
#include "QMP_P_GM.h"


/**
 * Error strings corresponding to the error codes.
 */
static char* QMP_error_strings[] = {
  "successful",
  "general QMP error",
  "message passing system is not initialized yet",
  "run time environment set up error",
  "machine CPU information retrieval error",
  "node run time information retrieval error",
  "malloc failure",
  "sending memory size is larger than receiving memory size",
  "node hostname retrieval error",
  "initiate underlying service (gm) error",
  "logical topology exists already",
  "channel connection time out",
  "not supported",
  "underlying service (gm) is busy",
  "invalid communication message",
  "invalid function arguments",
  "invalid declared topology",
  "topology neighboring information error",
  "declare memory size exceeeds the maximum allowed size",
  "invalid memory for a message handle",
  "no more gm ports available",
  "remote node number is out of range",
  "channel definition error (send/recv to/from itself)",
  "receiving memory is already used by others",
  "a sending/receiving operation applied in a wrong state",
  "communication time out",
};

#ifdef _QMP_USE_MULTI_PORTS
/**
 * Find available gm port
 */
QMP_u32_t
QMP_find_gm_port (QMP_machine_t* glm)
{
  QMP_u32_t i;
  

  QMP_TRACE ("QMP_find_gm_port");

  for (i = 0; i < GM_NUM_PORTS; i++) {
    if (glm->active_ports[i] == 0)
      return i;
  }
  return 0xffffffff;
}

/**
 * Set a particular port to active
 */
void
QMP_set_gm_port (QMP_machine_t* glm, QMP_u32_t port)
{
  QMP_TRACE ("QMP_set_gm_port");
  glm->active_ports[port] = 1;
}


/**
 * Clear a particular port to inactive
 */
void
QMP_clear_gm_port (QMP_machine_t* glm, QMP_u32_t port)
{
  QMP_TRACE ("QMP_clear_gm_port");

  glm->active_ports[port] = 0;
}

#endif

/**
 * Check whether a memory already declared for sending
 * or receiving.
 *
 * return TRUE if the memory is already been declared inside
 * one of the message handles.
 */
QMP_bool_t
QMP_memory_declared (QMP_machine_t* glm, QMP_msgmem_i_t* mem)
{
  QMP_msghandle_i_t* p;
  QMP_TRACE ("QMP_memory_declared");

  p = glm->msg_handles;
  while (p) {
    if (p->memory->mem == mem->mem)
      return QMP_TRUE;
    p = p->next;
  }
  return QMP_FALSE;
}

/**
 * Add a message handle to the end of message handle list.
 * 
 * If memory of mh is already declared or used, nothing is done. return FALSE.
 * Otherwise, return TRUE.
 */
QMP_bool_t
QMP_add_msg_handle (QMP_machine_t* glm, QMP_msghandle_i_t *mh)
{
  QMP_msghandle_i_t *q;

  QMP_TRACE ("QMP_add_msg_handle");

  if (!glm->msg_handles)
    glm->msg_handles = mh;
  else {
    q = glm->msg_handles;
    while (q->next) 
      q = q->next;
    
    q->next = mh;
  }
  return QMP_TRUE;
}

/**
 * Remove a message handle from the message handle list.
 *
 * If this handle is in the list, return QMP_TRUE.
 * Otherwise, return QMP_FALSE.
 */
QMP_bool_t
QMP_remove_msg_handle (QMP_machine_t* glm, QMP_msghandle_i_t *mh)
{
  QMP_msghandle_i_t *q, *prev;
  QMP_bool_t found;

  QMP_TRACE ("QMP_remove_msg_handle"); 

  found = QMP_FALSE;
  prev = 0;
  q = glm->msg_handles;
  while (q) {
    if (q == mh) {
      if (!prev) 
	/* remove first node */
	glm->msg_handles = q->next;
      else 
	prev->next = q->next;
      
      found = QMP_TRUE;
      break;
    }
    prev = q;
    q = q->next;
  }

  return found;
}

#ifdef _QMP_USE_MULTI_PORTS
/**
 * Add a pending connection request to the list.
 */
void
QMP_add_pending_conn_req (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_msghandle_i_t *p;

  QMP_TRACE ("QMP_add_pending_conn_req");

  if (!glm->conn_handles) 
    glm->conn_handles = mh;
  else {
    p = glm->conn_handles;
    while (p->next) {
      p = p->next;
    }
    p->next = mh;
  }
}

/**
 * Remove a pending request from the list.
 */
void
QMP_remove_pending_conn_req (QMP_machine_t* glm, QMP_msghandle_i_t* mh)
{
  QMP_msghandle_i_t *p, *q;

  QMP_TRACE ("QMP_remove_pending_conn_req");

  q = glm->conn_handles;
  p = 0;
  while (q) {
    if (q == mh) {
      if (p == 0) 
	glm->conn_handles = q->next;
      else 
	p->next = q->next;
      break;
    }
    p = q;
    q = q->next;
  }
  mh->next = 0;
}

#endif

/**
 * Simple QMP specific printf type of routine
 */
int
QMP_printf (const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128];
  char    buffer[1024];

  if (QMP_global_m.inited) {
    if (QMP_global_m.phys->type == QMP_SWITCH) 
      sprintf (info, "QMP m%d,n%d@%s : ", 
	       QMP_PHYS_RANK(QMP_global_m.phys),
	       QMP_NUM_NODES(QMP_global_m.phys),
	       QMP_global_m.host);
    else {
      fprintf (stderr, "QMP_printf: invoking unsupported network configuration.");
      exit (1);
    }
  }
  else
    sprintf (info, "QMP : ");


  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  printf ("%s %s\n", info, buffer);
  return status;
}


/**
 * Simple QMP specific fprintf type of routine
 */
int
QMP_fprintf (FILE* stream, const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128];
  char    buffer[1024];

  if (QMP_global_m.inited) {
    if (QMP_global_m.phys->type == QMP_SWITCH) 
      sprintf (info, "QMP m%d,n%d@%s : ", 
	       QMP_PHYS_RANK(QMP_global_m.phys),
	       QMP_NUM_NODES(QMP_global_m.phys),
	       QMP_global_m.host);
    else {
      fprintf (stderr, "QMP_fprintf: invoking unsupported network configuration.\n");
      exit (1);
    }
  }
  else
    sprintf (info, "QMP : ");


  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stream, "%s %s\n", info, buffer);
  return status;
}


/**
 * Simple information display routine
 */
int
QMP_info (const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128];
  char    buffer[1024];

  if (QMP_global_m.inited) {
    if (QMP_global_m.phys->type == QMP_SWITCH) 
      sprintf (info, "QMP m%d,n%d@%s info: ", 
	       QMP_PHYS_RANK(QMP_global_m.phys),
	       QMP_NUM_NODES(QMP_global_m.phys),
	       QMP_global_m.host);
    else {
      fprintf (stderr, "QMP_info: invoking unsupported network configuration.");
      exit (1);
    }
  }
  else
    sprintf (info, "QMP info: ");


  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);
  return status;
}


/**
 * Simple error display routine
 */
int
QMP_error (const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128];
  char    buffer[1024];

  if (QMP_global_m.inited) {
    if (QMP_global_m.phys->type == QMP_SWITCH) 
      sprintf (info, "QMP m%d,n%d@%s error: ", 
	       QMP_PHYS_RANK(QMP_global_m.phys),
	       QMP_NUM_NODES(QMP_global_m.phys),
	       QMP_global_m.host);
    else {
      fprintf (stderr, "QMP_error: invoking unsupported network configuration.\n");
      exit (1);
    }
  }
  else
    sprintf (info, "QMP error: ");


  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);
  return status;
}

/**
 * Simple error display routine and exit
 */
void
QMP_error_exit (const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128];
  char    buffer[1024];

  if (QMP_global_m.inited) {
    if (QMP_global_m.phys->type == QMP_SWITCH) 
      sprintf (info, "QMP m%d,n%d@%s error: ", 
	       QMP_PHYS_RANK(QMP_global_m.phys),
	       QMP_NUM_NODES(QMP_global_m.phys),
	       QMP_global_m.host);
    else {
      fprintf (stderr, "QMP_error_exit: invoking unsupported network configuration.");
      exit (1);
    }
  }
  else
    sprintf (info, "QMP error: ");


  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);

  /* Shutdown gm and so on */
  QMP_finalize_msg_passing ();

  /* Just Quit */
  exit (1);
}

/**
 * QMP Fatal error: exit this thing.
 */
void
QMP_fatal (QMP_u32_t rank, const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);

  sprintf (info, "QMP rank = %d at host %s fatal: ", rank, hostname);

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);

  /* Shutdown gm and so on */
  QMP_finalize_msg_passing ();

  /* Now quit */
  exit(1);
}


/**
 * Print out machine information.
 */
void
QMP_print_machine_info (QMP_machine_t* glm)
{
  if (!glm->inited)
    QMP_error ("this machine is not properly initialized yet.");
  else {
    QMP_info ("this machine has the following information.");
    
    QMP_print_rtenv (glm->rtenv,glm->num_rtenv_entries);

    if (glm->phys->type == QMP_SWITCH) {
      printf ("QMP machine m%d,n%d@%s \n", 
	      QMP_PHYS_RANK(glm->phys),
	      QMP_NUM_NODES(glm->phys),	      
	      glm->host);
    }
    else {
      fprintf (stderr, "QMP_print_machine_info: invoking routines for a unsupported network configuration.");
      exit (1);
    }
    
    if (glm->tpl) 
      QMP_print_topology (glm->tpl);
  }
}

/**
 * Return a machine information from a physical geometry
 */
const QMP_rtenv_t
QMP_get_machine_info (QMP_machine_t* glm, 
		      QMP_phys_geometry_t* phys)
{
  QMP_TRACE ("QMP_get_machine_info");

  if (phys->type == QMP_SWITCH) {
    QMP_u32_t rank = QMP_PHYS_RANK(phys);

    if (!glm->inited || !glm->rtenv ||
	glm->num_rtenv_entries <= rank)
      return 0;

    return &glm->rtenv[rank];
  }
  else {
    QMP_error ("unsupported network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}

/**
 * Return a machine information from a rank number.
 */
const QMP_rtenv_t
QMP_get_machine_info_by_id (QMP_machine_t* glm, 
			    QMP_u32_t logic_rank)
{
  QMP_TRACE ("QMP_get_machine_info_by_id");

  if (glm->ic_type == QMP_SWITCH) {

    if (!glm->inited || !glm->rtenv ||
	glm->num_rtenv_entries <= logic_rank)
      return 0;

    return &glm->rtenv[logic_rank];
  }
  else {
    QMP_error ("unsupported network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}

/**
 * Return an error string from a error code.
 */
const char*
QMP_error_string (QMP_status_t code)
{
  static char errstr[256];

  if (code == QMP_SUCCESS)
    return QMP_error_strings[code];

  if (code < QMP_MAX_STATUS && code >= QMP_ERROR)
    return QMP_error_strings[code - QMP_ERROR + 1];

  if (code < GM_NUM_ERROR_CODES && code >= GM_FAILURE) {
#ifdef GM_API_VERSION
    snprintf (errstr, sizeof (errstr) - 1, "GM error: %s", gm_strerror(code));
#else
    snprintf (errstr, sizeof (errstr) - 1, "GM error: %d", code);
#endif
    return (const char *)&errstr;
  }
  
  snprintf (errstr, sizeof (errstr) - 1, "unknown error code %d", code);
  return (const char *)&errstr;
}

/**
 * Return error number of last function call if there is an error.
 * If a function returns a status code, this function will not produce
 * any useful information. 
 *
 * This function should be used when no status
 * code is returned from a function call.
 */
QMP_status_t
QMP_get_error_number (QMP_msghandle_t mh)
{
  if (!mh)
    return QMP_global_m.err_code;
  else {
    QMP_msghandle_i_t* rmh;
    rmh = (QMP_msghandle_i_t *)mh;
    return rmh->err_code;
  }
}


/**
 * Return error string of last function call if there is an error.
 * If a function returns a status code, this function will not produce
 * any useful information. 
 *
 * This function should be used when no status
 * code is returned from a function call.
 */
const char*
QMP_get_error_string (QMP_msghandle_t mh)
{
  if (!mh)
    return QMP_error_string (QMP_global_m.err_code);
  else {
    QMP_msghandle_i_t* rmh;
    rmh = (QMP_msghandle_i_t *)mh;
    return QMP_error_string (rmh->err_code);
  }
}

/**************************************************************************
 *  The following routines are related to general messages passing        *
 **************************************************************************/

static char* QMP_data_type_strs[] = {"unsigned char",
				     "char",
				     "byte",
				     "unsigned short",
				     "short",
				     "unsigned integer 32bit",
				     "integer 32bit",
				     "unsigned integer 64bit",
				     "integer 64bit",
				     "float",
				     "double"
};

/**
 * Set default value for a message queue slot.
 */
static inline void
qmp_default_simple_msg_queue_slot_i (QMP_machine_t* glm,
				     QMP_msg_queue_slot_t* qs,
				     QMP_bool_t sender,
				     QMP_bool_t queued)
{
  QMP_TRACE ("qmp_default_simple_msg_queue_slot_i");

  qs->state = QMP_MH_IDLE;
  qs->pri   = 0;
  qs->glm   = glm;
  qs->next  = 0;

  if (sender) {
    qs->type  = QMP_MH_SEND;

    /**
     * allocate a new buffer which is large enough to hold a double value.
     */
    qs->buflen = QMP_DATA_SIZE(QMP_DOUBLE) + sizeof (QMP_general_msg_header_t);
    qs->buffer = (char *)gm_dma_malloc(glm->port, qs->buflen);
    if (!qs->buffer) {
      QMP_error ("cannot allocate memory for a sending buffer free list.");
      free (qs);
      exit (1);
    }
  }
  else if (!queued) {
    /**
     * This is a queue slot for receiving.
     */
    qs->type = QMP_MH_RECV;
      
    /**
     * Buffer is user provided buffer
     */
    qs->buflen = QMP_DATA_SIZE(QMP_DOUBLE);
    qs->buffer = 0;
  }
  else {
    /**
     * This is a queue slot for user queue or received queue.
     */
    qs->type = QMP_MH_RECV | QMP_MH_QUEUE;
    
    qs->buflen = QMP_DATA_SIZE(QMP_DOUBLE);
    qs->buffer = QMP_memalign (qs->buflen, QMP_MEM_ALIGNMENT);
    if (!qs->buffer) {
      QMP_error ("cannot allocate memory to queue message.");
      free (qs);
      exit (1);
    }
  }
    
  qs->hdr.source = 0;
  qs->hdr.dest = 0;
  /**
   * Populate message header information.
   */
  qs->hdr.magic = QMP_MAGIC;
  qs->hdr.vers  = QMP_VERSION_CODE;
  qs->hdr.op = QMP_ANY;
  qs->hdr.count = 1;
  qs->hdr.data_type = QMP_DOUBLE;

  if (sender)
    memcpy (qs->buffer, &qs->hdr, sizeof (QMP_general_msg_header_t));
}

/**
 * Create a free list for sending data of primitive types.
 */
static QMP_msg_queue_slot_t *
qmp_create_primitive_free_list_i (QMP_machine_t* glm,
				  QMP_bool_t sender,
				  QMP_bool_t queued)
{
  int i;
  QMP_msg_queue_slot_t *slots[QMP_GENMSG_NBUF], *qs;

  QMP_TRACE ("qmp_create_primitive_send_free_list");

  for (i = 0; i < QMP_GENMSG_NBUF; i++) {
    qs = (QMP_msg_queue_slot_t *)QMP_memalign (sizeof (QMP_msg_queue_slot_t), 
					       QMP_MEM_ALIGNMENT);
    /* set appropriate values for a simple message queue slot */
    qmp_default_simple_msg_queue_slot_i (glm, qs, sender, queued);
    slots[i] = qs;
  }

  /**
   * Establish the free list.
   */
  for (i = 0; i < QMP_GENMSG_NBUF - 1; i++)
    slots[i]->next = slots[i + 1];

  return slots[0];
}


/**
 * Create two free list for primitive data type message queue slot.
 */
void 
QMP_create_primitive_free_list (QMP_machine_t* glm)
{
  glm->send_free_list = 
    qmp_create_primitive_free_list_i (glm, QMP_TRUE, QMP_FALSE);

  glm->recving_free_list =  
    qmp_create_primitive_free_list_i (glm,
				      QMP_FALSE, QMP_FALSE);

  glm->recved_free_list = 
    qmp_create_primitive_free_list_i (glm, QMP_FALSE, QMP_TRUE);
}

/**
 * Free the primitive message queue list upon program exit.
 */
void
QMP_delete_primitive_free_list (QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t *n, *p;
  
  /* free send free list */
  if (glm->send_free_list) {
    p = glm->send_free_list;
    while (p) {
      n = p->next;
      gm_dma_free (glm->port, p->buffer);
      free (p);
      p = n;
    }
  }


  /* free recving free list */
  if (glm->recving_free_list) {
    p = glm->recving_free_list;
    while (p) {
      n = p->next;
      free (p);
      p = n;
    }    
  }


  /* Free received free list */
  if (glm->recved_free_list) {
    p = glm->recved_free_list;
    while (p) {
      n = p->next;
      free (p->buffer);
      free (p);
      p = n;
    }
  }

  glm->send_free_list = glm->recving_free_list = glm->recved_free_list = 0;
}

/**
 * Put a queue slot back into the free list.
 */
static void
qmp_put_queue_slot_to_free_list_i (QMP_msg_queue_slot_t** head, 
				   QMP_msg_queue_slot_t* qs)
{
  QMP_TRACE ("qmp_put_queue_slot_to_free_list_i");

  if (*head) {
    qs->next = *head;
    *head = qs;
  }
  else 
    *head = qs;
}

static inline void
qmp_update_simple_send_msg_queue_slot_i (QMP_msg_queue_slot_t* qs,
					 void* buffer, 
					 QMP_datatype_t data_type,
					 QMP_u32_t remote_id, 
					 QMP_u32_t operation, 
					 QMP_u32_t priority,
					 QMP_u8_t  adop,
					 QMP_machine_t* glm)
{
  QMP_general_msg_header_t* hdr;

  QMP_TRACE("qmp_update_simple_send_msg_queue_slot_i");

  qs->buflen = QMP_DATA_SIZE(data_type)+sizeof (QMP_general_msg_header_t);

  /* If no logical topology, use physical topology */
  if (glm->tpl)
    qs->hdr.source = glm->tpl->logic_rank;
  else
    qs->hdr.source = QMP_PHYS_RANK(glm->phys);
  qs->hdr.dest = remote_id;

  qs->pri = priority;
  qs->next = 0;
  qs->hdr.op = operation;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;
  

  /**
   * Copy user buffer into the newly allocated buffer.
   */
  switch (data_type) {
  case QMP_INT:
    *((int *)(&qs->buffer[sizeof (QMP_general_msg_header_t)])) = 
      *(int *)buffer;
    break;
  case QMP_64BIT_INT:
    *((QMP_s64_t *)(&qs->buffer[sizeof (QMP_general_msg_header_t)])) = 
      *(QMP_s64_t *)buffer;
    break;
  case QMP_FLOAT:
    *((float *)(&qs->buffer[sizeof (QMP_general_msg_header_t)])) = 
      *(float *)buffer;
    break;
  case QMP_DOUBLE:
    *((double *)(&qs->buffer[sizeof (QMP_general_msg_header_t)])) = 
      *(double *)buffer;
    break; 
  default:
    QMP_error ("Not supported data type in general message.");
    exit (1);
    break;
  }

  hdr = (QMP_general_msg_header_t *)qs->buffer;
    
  hdr->op = qs->hdr.op;
  hdr->data_type =  qs->hdr.data_type;
  hdr->source = qs->hdr.source;
  hdr->dest = qs->hdr.dest;
  hdr->adop = qs->hdr.adop;
}  

/**
 * Update a simple receiving message queue.
 */
static inline void
qmp_update_simple_recving_msg_queue_slot_i (QMP_msg_queue_slot_t* qs,
					    void* buffer, 
					    QMP_datatype_t data_type,
					    QMP_u32_t remote_id, 
					    QMP_u32_t operation, 
					    QMP_u32_t priority,
					    QMP_u8_t  adop,
					    QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_update_simple_recving_msg_queue_slot_i");

  qs->buflen = QMP_DATA_SIZE(data_type);
  qs->buffer = buffer;

  qs->hdr.source = remote_id;

  /* If there is no logical topology declared, use physical topology */
  if (glm->tpl)
    qs->hdr.dest = glm->tpl->logic_rank;
  else
    qs->hdr.dest = QMP_PHYS_RANK (glm->phys);

  qs->pri = priority;
  qs->next = 0;
  qs->hdr.op = operation;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;
}

/**
 * Update a receivied queue.
 */
static inline void
qmp_update_simple_recved_msg_queue_slot_i (QMP_msg_queue_slot_t* qs,
					   void* buffer, 
					   QMP_datatype_t data_type,
					   QMP_u32_t remote_id, 
					   QMP_u32_t operation, 
					   QMP_u32_t priority,
					   QMP_u8_t  adop,
					   QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_update_simple_recved_msg_queue_slot_i");

  qs->type = QMP_MH_RECV | QMP_MH_QUEUE; 
  qs->buflen = QMP_DATA_SIZE(data_type);

  qs->hdr.source = remote_id;

  /* Use physical topology if there is no logical one */
  if (glm->tpl)
    qs->hdr.dest = glm->tpl->logic_rank;
  else
    qs->hdr.dest = QMP_PHYS_RANK (glm->phys);

  qs->pri = priority;
  qs->next = 0;
  qs->hdr.op = operation;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;


  /**
   * Copy buffer into this new buffer.
   */
  switch (data_type) {
  case QMP_INT:
    *(int *)qs->buffer = *(int *)buffer;
    break;
  case QMP_64BIT_INT:
    *(QMP_s64_t *)qs->buffer = *(QMP_s64_t *)buffer;
    break;
  case QMP_FLOAT:
    *(float *)qs->buffer = *(float *)buffer;
    break;
  case QMP_DOUBLE:
    *(double *)qs->buffer = *(double *)buffer;
    break; 
  default:
    QMP_error ("Not supported data type in general message.");
    exit (1);
    break;
  }  
}

/**
 * Get a primitive data type from one of the free lists.
 */
static QMP_msg_queue_slot_t *
qmp_get_queue_slot_from_free_list_i (void* buffer, 
				     QMP_datatype_t data_type,
				     QMP_u32_t remote_id, 
				     QMP_u32_t operation, 
				     QMP_u32_t priority,
				     QMP_u8_t  adop,
				     QMP_bool_t sender, 
				     QMP_bool_t queued_msg, 
				     QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t* qs;
  QMP_TRACE ("qmp_get_queue_slot_from_free_list_i");

  if (sender) {
    if (glm->send_free_list) {
      qs = glm->send_free_list;
      glm->send_free_list = qs->next;
    }
    else {
      /* get more into the free list. */
      glm->send_free_list = qmp_create_primitive_free_list_i (glm,
							      QMP_TRUE,
							      QMP_FALSE);
      qs = glm->send_free_list;
      glm->send_free_list = qs->next;
    }
    qmp_update_simple_send_msg_queue_slot_i (qs, buffer,
					     data_type, remote_id,
					     operation, priority, adop,
					     glm);
  }
  else if (!queued_msg) {
    if (glm->recving_free_list) {
      qs = glm->recving_free_list;
      glm->recving_free_list = qs->next;
    }
    else {
      /* get more into the free list. */
      glm->recving_free_list = qmp_create_primitive_free_list_i (glm,
								 QMP_FALSE,
								 QMP_FALSE);
      qs = glm->recving_free_list;
      glm->recving_free_list = qs->next;
    }
    qmp_update_simple_recving_msg_queue_slot_i (qs,  buffer,
						data_type, remote_id,
						operation, priority, adop,
						glm);  
  }
  else {
    if (glm->recved_free_list) {
      qs = glm->recved_free_list;
      glm->recved_free_list = qs->next;
    }
    else {
      /* get more into the free list. */
      glm->recved_free_list = qmp_create_primitive_free_list_i (glm,
								QMP_FALSE,
								QMP_TRUE);
      qs = glm->recved_free_list;
      glm->recved_free_list = qs->next;
    }
    qmp_update_simple_recved_msg_queue_slot_i (qs,  buffer,
					       data_type, remote_id,
					       operation, priority, adop,
					       glm);  
  }
  return qs;
}

/**
 * Print out general message protocol header.
 */
void
QMP_print_genmsg_header (QMP_general_msg_header_t* hdr)
{
  printf ("general message header: magic 0x%x vers 0x%x op %d count %d type %s source %d dest %d \n",
	  hdr->magic, hdr->vers, hdr->op, 
	  hdr->count, QMP_data_type_strs[hdr->data_type],
	  hdr->source, hdr->dest);
}

/**
 * Print out message queue slot
 */
void
QMP_print_msg_queue_slot (QMP_msg_queue_slot_t* qs)
{
  char type_str[128];
  fprintf (stderr, "message queue slot %p looks like: \n", qs);
  QMP_print_genmsg_header (&qs->hdr);
  
  if (qs->type & QMP_MH_QUEUE)
    sprintf (type_str, "QUEUED ");
  else
    sprintf (type_str, "REGULAR ");
  
  if (qs->type & QMP_MH_RECV)
    strcat (type_str, "QMP_MH_RECV");
  else
    strcat (type_str, "QMP_MH_SEND");

  fprintf (stderr, "Type: %s\n", type_str);
  fprintf (stderr, "State: %d\n", qs->state);
  fprintf (stderr, "Priority: %d\n", qs->pri);
  fprintf (stderr, "Buflen: %d\n", qs->buflen);

  fprintf (stderr, "HDR: magic 0x%x version 0x%x operation %d source %d destination %d datatype %d count %d\n", qs->hdr.magic,
	   qs->hdr.vers, qs->hdr.op,
	   qs->hdr.source, qs->hdr.dest,
	   qs->hdr.data_type, qs->hdr.count);
}

/**
 * Check whether there is matched message in the received
 * message list.
 */
QMP_msg_queue_slot_t *
QMP_match_received_msg (void* buffer, QMP_u32_t count, 
			QMP_datatype_t datatype, QMP_u32_t src_id,
			QMP_u32_t operation, QMP_u32_t priority,
			QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t *p, *q;
  QMP_bool_t found;

  QMP_TRACE ("qmp_match_received_msg");

  found = QMP_FALSE;
  q = glm->received_queue;
  p = 0;
  while (q) {
    if(QMP_RECVED_MATCH(q, count, datatype, src_id, operation, priority)){
      found = QMP_TRUE;
      /* now remove this from the queue */
      if (p) 
	p->next = q->next;
      else
	glm->received_queue = q->next;

      break;
    }
    p = q;
    q = q->next;
  }
  if (found)
    return q;

  return 0;
}

/**
 * Add an incoming message to the received list.
 */
void
QMP_add_received_msg (QMP_general_msg_header_t* hdr, 
		      void* buffer, 
		      QMP_u32_t length, 
		      QMP_u32_t pri, 
		      QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t *p, *q;

  QMP_TRACE ("QMP_add_received_msg");

  p = QMP_create_msg_queue_slot (buffer, 
				 hdr->count,
				 hdr->data_type,
				 hdr->source,
				 hdr->op,
				 pri,
				 hdr->adop,
				 QMP_FALSE,
				 QMP_TRUE,
				 glm);

  /**
   * Now add this queue slot to the received list.
   */
  if (!glm->received_queue) 
    glm->received_queue = p;
  else {
    q = glm->received_queue;
    while (q->next)
      q = q->next;

    q->next = p;
  }
}

/**
 * Set value of a regular sending message queue.
 */
static void 
qmp_set_reg_send_msg_queue_i (QMP_msg_queue_slot_t* qs,
			      void* buffer, 
			      QMP_u32_t count, 
			      QMP_datatype_t data_type, 
			      QMP_u32_t remote_id, 
			      QMP_u32_t operation, 
			      QMP_u32_t priority,
			      QMP_u8_t  adop,
			      QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_set_reg_send_msg_queue_i");

  qs->state = QMP_MH_IDLE;
  qs->pri   = priority;
  qs->glm   = glm;
  qs->next  = 0;
  qs->type  = QMP_MH_SEND;

  /**
   * Populate message header information.
   * If there is no logical topology, use physical topology.
   */
  if (glm->tpl)
    qs->hdr.source = glm->tpl->logic_rank;
  else
    qs->hdr.source = QMP_PHYS_RANK (glm->phys);
  qs->hdr.dest = remote_id;
  qs->hdr.magic = QMP_MAGIC;
  qs->hdr.vers  = QMP_VERSION_CODE;
  qs->hdr.op = operation;
  qs->hdr.count = count;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;
      
  /**
   * allocate a new buffer which contains message header and the user buffer
   */
  qs->buflen =count*QMP_DATA_SIZE(data_type)+sizeof (QMP_general_msg_header_t);
  qs->buffer = (char *)gm_dma_malloc(glm->port, qs->buflen);
  if (!qs->buffer) {
    QMP_error ("cannot allocate memory for a sending buffer.");
    free (qs);
    exit (1);
  }

  /**
   * Copy user buffer into the newly allocated buffer.
   */
  memcpy (&qs->buffer[sizeof (QMP_general_msg_header_t)], buffer,
	  count*QMP_DATA_SIZE(data_type));

  /**
   * Copy message header onto the beginning of the buffer.
   */
  memcpy (qs->buffer, &qs->hdr, sizeof (QMP_general_msg_header_t));
}

/**
 * Set value of a regular receiving message queue.
 */
static void 
qmp_set_reg_recving_msg_queue_i (QMP_msg_queue_slot_t* qs,
				 void* buffer, 
				 QMP_u32_t count, 
				 QMP_datatype_t data_type, 
				 QMP_u32_t remote_id, 
				 QMP_u32_t operation, 
				 QMP_u32_t priority,
				 QMP_u8_t  adop,
				 QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_set_reg_recving_msg_queue_i");

  qs->state = QMP_MH_IDLE;
  qs->pri   = priority;
  qs->glm   = glm;
  qs->next  = 0;
  /**
   * This is a queue slot for receiving.
   */
  qs->type = QMP_MH_RECV;

  /**
   * Buffer is user provided buffer
   */
  qs->buflen = count*QMP_DATA_SIZE(data_type);
  qs->buffer = buffer;

  /**
   * Populate message header information.
   * If there is no logical topology, use a physical topology
   */
  if (glm->tpl)
    qs->hdr.dest = glm->tpl->logic_rank;
  else
    qs->hdr.dest = QMP_PHYS_RANK (glm->phys);
  qs->hdr.source = remote_id;

  qs->hdr.magic = QMP_MAGIC;
  qs->hdr.vers  = QMP_VERSION_CODE;
  qs->hdr.op = operation;
  qs->hdr.count = count;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;
}


/**
 * Set value of a regular receiving message queue.
 */
static void 
qmp_set_reg_recved_msg_queue_i (QMP_msg_queue_slot_t* qs,
				void* buffer, 
				QMP_u32_t count, 
				QMP_datatype_t data_type, 
				QMP_u32_t remote_id, 
				QMP_u32_t operation, 
				QMP_u32_t priority,
				QMP_u8_t  adop,
				QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_set_reg_recved_msg_queue_i");

  qs->state = QMP_MH_IDLE;
  qs->pri   = priority;
  qs->glm   = glm;
  qs->next  = 0;

  /**
   * This is a queue slot for user queue or received queue.
   */
  qs->type = QMP_MH_RECV | QMP_MH_QUEUE;
    
  qs->buflen = count*QMP_DATA_SIZE(data_type);
  qs->buffer = QMP_memalign (qs->buflen, QMP_MEM_ALIGNMENT);
  if (!qs->buffer) {
    QMP_error ("cannot allocate memory to queue message.");
    free (qs);
    exit (1);
  }
    
  /**
   * Copy buffer into this new buffer.
   */
  memcpy (qs->buffer, buffer, qs->buflen);

  /**
   * Populate message header information.
   * Use a physical topology if there is no logical topology.
   */    
  if (glm->tpl)
    qs->hdr.dest = glm->tpl->logic_rank;
  else
    qs->hdr.dest = QMP_PHYS_RANK (glm->phys);
  qs->hdr.source = remote_id;
  qs->hdr.magic = QMP_MAGIC;
  qs->hdr.vers  = QMP_VERSION_CODE;
  qs->hdr.op = operation;
  qs->hdr.count = count;
  qs->hdr.data_type = data_type;
  qs->hdr.adop = adop;
}

/**
 * Create a message queue slot for sending/receiving
 *
 * Arguments:
 *     Buffer:      User Data Buffer.
 *     count:       number of elements of this data buffer.
 *     data_type:   one of the data type defined by QMP.
 *     remote_id:   remote node id (sending: dest id, recving, source_id)
 *     operation:   what is this message doing.
 *     priority:    priority of this message.
 *     sender:      is this a sender.
 *     queued_msg:  this queue is uesd to save incoming msg.
 *     glm:         global QMP_machine_t
 */
QMP_msg_queue_slot_t *
QMP_create_msg_queue_slot (void* buffer, 
			   QMP_u32_t count, 
			   QMP_datatype_t data_type, 
			   QMP_u32_t remote_id, 
			   QMP_u32_t operation, 
			   QMP_u32_t priority,
			   QMP_u8_t  adop,
			   QMP_bool_t sender, 
			   QMP_bool_t queued_msg,
			   QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t* qs;

  QMP_TRACE ("qmp_create_msg_queue_slot");

  if (count == 1) {
    qs = qmp_get_queue_slot_from_free_list_i (buffer, data_type,
					      remote_id, operation, 
					      priority, adop,
					      sender, queued_msg, glm);
  }
  else {
    qs = (QMP_msg_queue_slot_t *)QMP_memalign (sizeof (QMP_msg_queue_slot_t), 
					       QMP_MEM_ALIGNMENT);
    if (!qs) {
      QMP_error ("cannot allocate memory for sending message queue slot.");
      return 0;
    }
    
    if (sender)
      qmp_set_reg_send_msg_queue_i (qs, buffer, count, data_type,
				    remote_id, operation, priority, adop, glm);
    else if (!queued_msg)
      qmp_set_reg_recving_msg_queue_i (qs, buffer, count, data_type,
				       remote_id, operation, priority, adop,
				       glm);
    else 
      qmp_set_reg_recved_msg_queue_i (qs, buffer, count, data_type,
				      remote_id, operation, priority, adop,
				      glm);
  }

#ifndef _QMP_USE_MULTI_PORTS
  qs->mh = 0;
#endif

  return qs;
}

/**
 * Delete a sending message queue slot
 */
void
QMP_delete_msg_queue_slot (QMP_msg_queue_slot_t* qs)
{
  QMP_TRACE ("qmp_delete_msg_queue_slot");

  if (qs) {
    if (qs->hdr.count == 1) {
      if (qs->type == QMP_MH_SEND) 
	qmp_put_queue_slot_to_free_list_i (&qs->glm->send_free_list, qs);
      else if (qs->type == QMP_MH_RECV)
	qmp_put_queue_slot_to_free_list_i (&qs->glm->recving_free_list,
					   qs);
      else
	qmp_put_queue_slot_to_free_list_i (&qs->glm->recved_free_list, 
					   qs);
    }
    else {
      if (qs->type == QMP_MH_SEND) 
	/* free dma memory buffer */
	gm_dma_free (qs->glm->port, qs->buffer);
      else if (qs->type & QMP_MH_QUEUE)
	/* free regular memory buffer */
	free (qs->buffer);
    
      /* free qs itself         */
      free (qs);
    }
  }
}  


/**
 * Add a queue slot to the pending send buffer list.
 */
void
QMP_add_pending_send_buffer (QMP_machine_t* glm,
			     QMP_msg_queue_slot_t* qs)
{
  QMP_msg_queue_slot_t* p;

  QMP_TRACE ("QMP_add_pending_send_buffer");

  if (!glm->pending_send_queue)
    glm->pending_send_queue = qs;
  else {
    p = glm->pending_send_queue;
    while (p->next)
      p = p->next;
    p->next = qs;
  }
}

/**
 * Remove a queue slot to the pending send buffer list.
 */
void
QMP_remove_pending_send_buffer (QMP_machine_t* glm,
				QMP_msg_queue_slot_t* qs)
{
  QMP_msg_queue_slot_t *p, *q;

  QMP_TRACE ("QMP_remove_pending_send_buffer");
  q = glm->pending_send_queue;
  p = 0;
  while (q) {
    if (q == qs) {
      if (p)
	p->next = q->next;
      else
	glm->pending_send_queue = q->next;

      break;
    }
    p = q;
    q = q->next;
  }
}

/**
 * Add a queue slot to the provided receiving buffer list.
 */
void
QMP_add_provided_receiving_buffer (QMP_machine_t* glm,
				   QMP_msg_queue_slot_t* qs)
{
  QMP_msg_queue_slot_t* p;

  QMP_TRACE ("QMP_add_provided_receiving_buffer");

  if (!glm->provided_buffer_queue)
    glm->provided_buffer_queue = qs;
  else {
    p = glm->provided_buffer_queue;
    while (p->next)
      p = p->next;
    p->next = qs;
  }
}

/**
 * Remove a queue slot to the provided receiving buffer list.
 */
void
QMP_remove_provided_receiving_buffer (QMP_machine_t* glm,
				      QMP_msg_queue_slot_t* qs)
{
  QMP_msg_queue_slot_t *p, *q;

  QMP_TRACE ("QMP_remove_provided_receiving_buffer");
  q = glm->provided_buffer_queue;
  p = 0;
  while (q) {
    if (q == qs) {
      if (p)
	p->next = q->next;
      else
	glm->provided_buffer_queue = q->next;

      break;
    }
    p = q;
    q = q->next;
  }
}

/**
 * Find out whether there is a provided receiving buffer matching an incoming
 * message.
 */
QMP_bool_t
QMP_match_provided_buffer(QMP_general_msg_header_t* hdr, 
			  void* buffer, 
			  QMP_u32_t length, 
			  QMP_u32_t pri, 
			  QMP_machine_t* glm)
{
  QMP_msg_queue_slot_t *p;

  QMP_TRACE ("QMP_match_provided_buffer");
  
  p = glm->provided_buffer_queue;
  while (p) {
    if (p->state == QMP_MH_WAITING && QMP_PROVIDED_MATCH (p, hdr, pri)) {

#ifndef _QMP_USE_MULTI_PORTS
      /**
       * This is the first packet for this pending receiving slot.
       * Change its tag to sender's tag so that all subsquent receiving
       * will use this new tag from the sender.
       */
      if (p->mh && p->mh->cnt == 0 && 
	  hdr->op >= QMP_START_TAG && hdr->adop == QMP_CH_CONNECT) 
	p->mh->tag = hdr->op;
#endif

      p->state = QMP_MH_IDLE;
      if (hdr->count == 1) {
	switch (hdr->data_type) {
	case QMP_INT:
	  *(int *)p->buffer = *(int *)buffer;
	  break;
	case QMP_64BIT_INT:
	  *(QMP_s64_t *)p->buffer = *(QMP_s64_t *)buffer;
	  break;
	case QMP_FLOAT:
	  *(float *)p->buffer = *(float *)buffer;
	  break;
	case QMP_DOUBLE:
	  *(double *)p->buffer = *(double *)buffer;
	  break; 
	default:
	  QMP_error ("Not supported data type in general message.");
	  exit (1);
	  break;
	}  
      }	
      else
	memcpy (p->buffer, buffer, length);
      return QMP_TRUE;
    }
    p = p->next;
  }
  return QMP_FALSE;
}


