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
 *      Initialization of lqcd messaging system using GM
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.14  2003/01/08 20:37:48  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.13  2002/12/05 16:41:03  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.12  2002/11/15 15:37:33  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.11  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.10  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.9  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.8  2002/04/22 20:28:41  chen
 *   Version 0.95 Release
 *
 *   Revision 1.7  2002/03/28 18:48:20  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.6  2002/03/27 20:48:49  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.5  2002/02/15 20:34:53  chen
 *   First Beta Release QMP
 *
 *   Revision 1.4  2002/01/27 20:53:50  chen
 *   minor change
 *
 *   Revision 1.3  2002/01/27 17:38:57  chen
 *   change of file structure
 *
 *   Revision 1.2  2002/01/24 20:10:50  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:50  chen
 *   initial import of QMP
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
 * Global instance of this machine.
 */
#ifdef _QMP_USE_MULTI_PORTS
QMP_machine_t QMP_global_m = {1, 1, QMP_SWITCH,
			      0, 0, QMP_FALSE, 0,
			      0, 0, 2, 0,
			      0, 0, 
			      {1, 1, 1, 1, 0,},
			      {0, },
			      {0, }, 
			      0, 
			      0, 
			      0, 
			      0, 0, 0,
			      {0,},
			      0, 0, 0, QMP_ID_INIT_VALUE,
			      0, 0, QMP_SUCCESS
};

#else /* _QMP_USE_MULTI_PORTS */
QMP_machine_t QMP_global_m = {1, 1, QMP_SWITCH,
			      0, 0, QMP_FALSE, 0,
			      0, 0, 2, 0,
			      0, 0, 
			      {0, }, 
			      0, 
			      0, 
			      0, 
			      0, 0, 0,
			      {0,},
			      0, 0, 0, QMP_ID_INIT_VALUE,
			      QMP_SUCCESS
};

#endif /* _QMP_USE_MULTI_PORTS */

/**
 * Runtime verbose mode. (default is off)
 */
QMP_bool_t QMP_rt_verbose = QMP_FALSE;

/**
 * Check whether we have an empty line.
 */
static QMP_bool_t
empty_line (char line[], QMP_u32_t size)
{
  char* p;

  QMP_TRACE("empty_line");

  p = line;

  while (p && *p) {
    if (isprint(*p))
      return QMP_FALSE;
    p++;
  }
  return QMP_TRUE;
}


/**
 * Get number of CPUs for this linux box.
 */
static QMP_u32_t
get_num_cpus (void)
{
  FILE* fd;
  char  line[80];
  int   numcpu = 0;

  QMP_TRACE ("get_num_cpus");

  fd = fopen ("/proc/cpuinfo", "r");
  if (!fd) {
    QMP_error ("cannot open /proc/cpuinfo file.");
    return 0;
  }

#if defined (__linux) && defined (__i386)
  while (!feof (fd)) {
    memset (line, 0, sizeof (line));
    if (fgets (line, sizeof (line) - 1, fd) &&
	!empty_line (line, sizeof (line) - 1)) {
      if (strstr (line, "processor"))
	numcpu++;
    }
  }
#endif

#if defined (__linux) && defined (__alpha)
    while (!feof (fd)) {
      memset (line, 0, sizeof (line));
      if (fgets (line, sizeof (line) - 1, fd) &&
	  !empty_line (line, sizeof (line) - 1)) {
	if (strstr (line, "cpus detected")) {
	  char token0[32], token1[32], sep;
	  if (sscanf (line,"%s %s %c %d", token0, token1, &sep, &numcpu) <4){
	    QMP_error ("/proc/cpuinfo format error.");
	    return 0;
	  }
	}
      }
    }
#endif    
    
    fclose (fd);
    return numcpu;
}

/**
 * get machine rank and number of processes.
 */
static QMP_bool_t
get_rank_info (QMP_u32_t* nump, QMP_u32_t* rank)
{
  char* confinfo;

  QMP_TRACE ("get_rank_info");

  confinfo = getenv (QMP_OPTS_ENV);
  if (!confinfo) {
    QMP_error ("No QMP_OPTS environment variable provided.\n");
    return QMP_FALSE;
  }

  if (sscanf (confinfo, "m%d,n%d", rank, nump) == 2)
    return QMP_TRUE;

  return QMP_FALSE;
}

/**
 * Load runtime environment from a configuration file
 * 
 * This configuration file contains lines containing the following
 * information
 * 
 * Host      [GM_Device]
 *
 * The position of line will be the rank of this process inside
 * a parallel job.
 * 
 */
static QMP_rtenv_t
get_rtenv (const char* conf, QMP_u32_t* retsize)
{
  FILE* fd;
  int   num, i;
  char  line[128], host[QMP_HOSTNAME_LEN];
  int   port, device;
  QMP_rtenv_t rtenv = 0;
  

  QMP_TRACE ("get_rtenv");

  *retsize = 0;
  fd = fopen (conf, "r");
  if (!fd) {
    QMP_error ("cannot open configuration file %s\n", conf);
    return rtenv;
  }

  num = 0;
  while (!feof (fd)) {
    memset (line, 0, sizeof (line));
    if (fgets (line, sizeof (line) - 1, fd) &&
	!empty_line (line, sizeof (line) - 1)) {
      device = 0;
      port   = 2;
      if (line[0] != '#' &&
	  sscanf (line, "%s %d", host, &device) >= 1) {
	if (port >= 2 && device >= 0)
	  num++;
      }
    }

  }
  
  /* rewind file to the beginning */
  fseek (fd, 0L, SEEK_SET);

  if (num == 0) {
    QMP_error ("%s configuration file has no configuration information.\n", conf);
    return rtenv;
  }

  /* now allocate memory for rtenv */
  rtenv = (QMP_rtenv_t)malloc(sizeof(QMP_rtenv_entry_t)*num);
  
  if (!rtenv) {
    QMP_error ("cannot allocate memory for QMP_rtenv.\n");
    exit (1);
  }

  i = 0;
  while (!feof (fd)) {
    memset (line, 0, sizeof (line));
    if (fgets (line, sizeof (line) - 1, fd) &&
	!empty_line (line, sizeof (line) - 1)) {
      device = 0;
      port   = 2;
      if (line[0] != '#' && 
	  sscanf (line, "%s %d", host, &device) >= 1) {
	if (port >= 2 && device >= 0) {
	  rtenv[i].rank = i;
	  strcpy (rtenv[i].host, host);
	  rtenv[i].port = port;
	  rtenv[i].device = device;
	  rtenv[i].node_id = GM_NO_SUCH_NODE_ID;
	  i++;
	}
      }
    }
  }
  fclose (fd);

  *retsize = num;
  return rtenv;
}

/**
 * Update GM node information for our run time environment.
 *
 * This can be only done after gm has been initialized.
 * All node ids should be determined after calling this routine.
 *
 */
static QMP_bool_t
update_nodeid_info (struct gm_port* port, QMP_rtenv_t env,
		    QMP_u32_t size)
{
  char  gm_host[QMP_HOSTNAME_LEN + 2];
  int   i;

  QMP_TRACE ("update_nodeid_info");

  for (i = 0; i < size; i++) {
    if (env[i].device != 0)
      sprintf (gm_host, "%s:%d", env[i].host, env[i].device);
    else
      strcpy (gm_host, env[i].host);

    env[i].node_id = gm_host_name_to_node_id (port, gm_host);

    if (env[i].node_id == GM_NO_SUCH_NODE_ID) {
#ifdef _QMP_DEBUG
      QMP_info ("warning: cannot find gm node id information for host %s\n",
		gm_host);
#endif
    }
  }
  return QMP_TRUE;
}

/**
 * Print out Run Time Env Information.
 */
void
QMP_print_rtenv (QMP_rtenv_t rtenv, QMP_u32_t size)
{
  int i;

  QMP_TRACE ("QMP_print_rtenv");

  printf ("QMP Run Time Environment has %d participants: \n", size);
  for (i = 0; i < size; i++) {
    printf ("%d:    Rank %d   Host %s    port %d     device %d    node %d\n",
	    i,
	    rtenv[i].rank, rtenv[i].host, rtenv[i].port, rtenv[i].device,
	    rtenv[i].node_id);
  }
}


/**
 * Local initialization of global machine.
 */
static QMP_status_t
init_machine (QMP_machine_t* glm)
{
  int i;
  QMP_u32_t num_machines, rank;

  QMP_TRACE ("init_machine");

  /* get number of CPUs */
  glm->num_cpus = get_num_cpus ();

  if (QMP_rt_verbose)
    QMP_info ("This machine has %d CPU(s)\n", glm->num_cpus);

  if (glm->num_cpus == 0) {
    QMP_error ("This machine has no cpu information.\n");
    return QMP_CPUINFO_ERR;
  }
    
  glm->collapse_smp  = 1;

#ifdef _QMP_IC_SWITCH
  glm->ic_type  = QMP_SWITCH;

  /* get rank and number of machines information */
  if (get_rank_info (&num_machines, &rank) == QMP_FALSE)
    return QMP_NODEINFO_ERR;

  glm->phys = (QMP_phys_geometry_t *)malloc(sizeof (QMP_phys_geometry_t));
  if (!glm->phys) {
    QMP_error ("cannot allocate memory for physical geometry information.");
    return QMP_NOMEM_ERR;
  }

  glm->phys->type = QMP_SWITCH;
  QMP_NUM_NODES(glm->phys) = num_machines;
  QMP_PHYS_RANK(glm->phys) = rank;

  if (QMP_rt_verbose)
    QMP_info ("This machine has rank %d within %d processors\n",
	      QMP_PHYS_RANK(glm->phys),
	      QMP_NUM_NODES(glm->phys));
#else
#error "QMP only supports switched configuration."
#endif

  /* get host name */
  glm->host = (char *)malloc (QMP_HOSTNAME_LEN*sizeof (char));
  if (!glm->host) {
    QMP_error ("cannot allocate memory for host name\n");
    return QMP_NOMEM_ERR;
  }

  if (gethostname (glm->host, QMP_HOSTNAME_LEN - 1) != 0) {
    QMP_error ("cannot get host name for this machine.\n");
    return QMP_HOSTNAME_ERR;
  }
  
  glm->inited = QMP_FALSE;
  glm->tpl = 0;
  glm->port = 0;
  glm->device = 0;
  glm->port_num = 2;
  glm->port_name = 0;
  glm->node_id = 0;

#ifdef _QMP_USE_MULTI_PORTS
  /* control message buffer */
  for (i = 0; i < QMP_CRLMSG_NBUF; i++)
    glm->ctrl_msg_buffers[i] = 0;
#endif

  /* Genral message buffer   */
  for (i = 0; i < QMP_GENMSG_NBUF; i++)
    glm->gen_msg_buffers[i] = 0;

  /**
   * free list is a pointer to the free list. It could change in the run
   * time. However, free list buf is a fixed value.
   */
  glm->send_free_list = 0;
  glm->recving_free_list = 0;
  glm->recved_free_list = 0;

  /* user provided buffer to receive messages */
  glm->provided_buffer_queue = 0;

  /* messages that are stored here if they are not claimed by anyone */
  glm->received_queue = 0;

  /* gm send token cnt */
  for (i = 0; i < GM_NUM_PRIORITIES; i++)
    glm->send_token_cnt[i] = 0;

  glm->max_msg_length = 0;

#ifdef _QMP_USE_MULTI_PORTS
  /* Initialize active ports flag */
  for (i = 0; i < GM_NUM_PORTS; i++) {
    if (i < QMP_GM_PREVI_PORTS)
      glm->active_ports[i] = 1;
    else
      glm->active_ports[i] = 0;
  }

  /* Set channel id */
  glm->chid = QMP_ID_INIT_VALUE;

  /* set pending requests */
  glm->conn_handles = 0;

  /* set error control message with a very large refrence count */
  glm->err_ctrlmsg = 0;
#endif

  return QMP_SUCCESS;
}

/**
 * Open gm port using machine information from run time environment.
 */
static QMP_status_t
open_gm_port (QMP_machine_t* m)
{
  QMP_u32_t   ctrl_msg_len, gen_msg_len;
  gm_status_t status;
  char        port_name[80];
  int         i;
  QMP_rtenv_t machine;

  QMP_TRACE ("open_gm_port");

  /* get GM specific machine information from physical geometry information */
  machine = QMP_get_machine_info (m, m->phys);

  if (!machine) {
    QMP_error ("cannot get this machine information.");
    return QMP_NODEINFO_ERR;
  }
  m->device = machine->device;
  m->port_num = machine->port;

  if (m->phys->type == QMP_SWITCH) 
    sprintf (port_name, "QMP_m%d.n%d@%s", 
	     QMP_PHYS_RANK(m->phys),
	     QMP_NUM_NODES(m->phys),
	     m->host);
  else {
    fprintf (stderr, "invoking routines for unsupported network configuration.");
    exit (1);
  }

  m->port_name = (char *)malloc ((strlen (port_name) + 1) * sizeof(char));
  if (!m->port_name) {
    QMP_error ("cannot allocate memory for port_name.");
    return QMP_NOMEM_ERR;
  }
  
  status = gm_open (&m->port, m->device, m->port_num, m->port_name,
		    GM_API_VERSION_1_2);
  if (status != GM_SUCCESS) {
    gm_perror ("could not open gm port", status);
    return QMP_INITSVC_ERR;
  }

  /* remeber this node id */
  status = gm_get_node_id (m->port, &m->node_id);

  if (status != GM_SUCCESS) {
    QMP_error ("cannot determine this gm node id.\n");
    gm_close (m->port);
    free (m->port_name);
    m->port_name = 0;
    m->port = 0;

    return QMP_INITSVC_ERR;
  }

  /**
   * Manage gm send tokens by ourself
   */
  for (i = 0; i < GM_NUM_PRIORITIES; i++)
    m->send_token_cnt[i] = gm_num_send_tokens (m->port)/GM_NUM_PRIORITIES;

  /* calculate maximum message length we are supporting */
  m->max_msg_length = gm_max_length_for_size (QMP_GENMSG_SIZE) - 
    sizeof (QMP_general_msg_header_t);

  if (QMP_rt_verbose)
    QMP_info ("maximum general message length is %d\n", m->max_msg_length);

#ifdef _QMP_USE_MULTI_PORTS
  /* calculate maximum control message length        */
  ctrl_msg_len = gm_max_length_for_size (QMP_CRLMSG_SIZE);
  
  /* allocate memory for control message information */
  for (i = 0; i < QMP_CRLMSG_NBUF; i++) {
    m->ctrl_msg_buffers[i] = gm_dma_malloc (m->port, ctrl_msg_len);
    if (!m->ctrl_msg_buffers[i]) {
      QMP_error ("cannot do gm_dma_malloc, quit\n");
      gm_close (m->port);
      gm_finalize ();
      return QMP_NOMEM_ERR;
    }
  }

  /* now provide this buffer to gm */
  for (i = 0; i < QMP_CRLMSG_NBUF; i++) 
    gm_provide_receive_buffer (m->port, m->ctrl_msg_buffers[i], 
			       QMP_CRLMSG_SIZE,
			       QMP_CHMSG_PRIORITY);

  /* set error control message with a very large refrence count */
  m->err_ctrlmsg = QMP_create_ctrlmsg (m, 0, 0x0fffffff, 0);
  if (!m->err_ctrlmsg) {
    QMP_error ("cannot allocate error control message.");
    exit (1);
  }
#endif

  /* calculate maximum queued message length        */
  gen_msg_len = gm_max_length_for_size (QMP_GENMSG_SIZE);

  /* allocate memory for general message information */
  for (i = 0; i < QMP_GENMSG_NBUF; i++) {
    m->gen_msg_buffers[i] = gm_dma_malloc (m->port, gen_msg_len);
    if (!m->gen_msg_buffers[i]) {
      QMP_error ("cannot do gm_dma_malloc for the receiving , quit\n");
      gm_close (m->port);
      gm_finalize ();
      return QMP_NOMEM_ERR;
    }
  }

  /**
   * Provide receiving buffer to GM
   */
  for (i = 0; i < QMP_GENMSG_NBUF; i++) 
    gm_provide_receive_buffer (m->port, m->gen_msg_buffers[i], 
			       QMP_GENMSG_SIZE,
			       QMP_MSG_PRIORITY);

  /**
   * Create primitive type free list for general message sending/receiving
   */
  QMP_create_primitive_free_list (m);

  return QMP_SUCCESS;
}

/**
 * a real routine that is doing initialization using
 * a global machine pointer.
 */
static QMP_bool_t
qmp_init_msg_passing_i (QMP_machine_t* glm,
			QMP_smpaddr_type_t option)
{
  /* load run time environment */
  char         conf[256];
  QMP_status_t status;
  char*        rtconf;

  QMP_TRACE ("qmp_init_msg_passing_i");

  rtconf = getenv (QMP_CONF_ENV);
  if (rtconf) 
    strcpy (conf, rtconf);
  else 
    sprintf (conf, "%s/%s", getenv("HOME"), QMP_DEFAULT_CONF);

  if (QMP_rt_verbose)
    QMP_info ("Loading runtime configuration file: %s\n", conf);

  glm->rtenv = get_rtenv (conf, &glm->num_rtenv_entries);
  if (!glm->rtenv)
    return QMP_RTENV_ERR;

  /* initialize this machine */
  if (glm->inited == QMP_TRUE) {
    if (QMP_rt_verbose)
      QMP_info ("This QMP system is initialized already\n");
    return QMP_SUCCESS;
  }

  if ((status = init_machine (glm)) != QMP_SUCCESS)
    return status;
  
  if (option == QMP_SMP_ONE_ADDRESS)
    glm->collapse_smp = QMP_TRUE;
  else
    glm->collapse_smp = QMP_FALSE;

  if (gm_init () != GM_SUCCESS) {
    QMP_error ("calling gm_init failed\n");
    glm->inited = QMP_FALSE;
    return QMP_INITSVC_ERR;
  }
  else
    glm->inited = QMP_TRUE;
  
  /* now it is time to open gm port */
  if ((status = open_gm_port (glm)) != QMP_SUCCESS) {
    glm->inited = QMP_FALSE;    
    return status;
  }

  /* update node id information in the run time environment */
  if (update_nodeid_info (glm->port, glm->rtenv,
			  glm->num_rtenv_entries) == QMP_FALSE) {
    glm->inited = QMP_FALSE;    
    return QMP_NODEINFO_ERR;
  }
  
  if (QMP_rt_verbose)
    QMP_print_machine_info (glm);

  if (QMP_rt_verbose)
    QMP_info ("QMP_init successfully done.\n");

  
  return QMP_SUCCESS;
}
  
/**
 * Initialize a data structure associated with a node.
 * 
 * This initialization is helped by several environment variable.
 * QMP_OPTS=m0,n20 tells this machine is number 0 in rank and
 * total number of nodes is 20.
 */
QMP_status_t
QMP_init_msg_passing (int argc, char** argv,
		      QMP_smpaddr_type_t option)
{
  QMP_machine_t* glm = &QMP_global_m;
  QMP_status_t status = qmp_init_msg_passing_i (glm, option);

  if (status == QMP_SUCCESS)
    status = QMP_wait_for_barrier (10000.0);

  if (status == QMP_TIMEOUT) {
    if (QMP_is_primary_node()) 
      QMP_fprintf (stderr, "Timeout for first communication initialization, try again.\n");
    /**
     * I will try it again
     */
    status = QMP_wait_for_barrier (10000.0);
  }

  QMP_SET_STATUS_CODE(glm, status);
  return status;
}

/**
 * Real routine that finalize message passing
 */
static void 
qmp_finalize_msg_passing_i (QMP_machine_t* glm)
{
  int i;

  QMP_TRACE ("qmp_finalize_msg_passing_i");

  if (glm->inited == QMP_FALSE)
    return;

  /* make sure every node is getting here before I quit */
  QMP_wait_for_barrier (10000.0);

  /* Release memory for host name */
  free (glm->host);
  glm->host = 0;

  /* release memory for run time environment */
  free (glm->rtenv);
  glm->rtenv = 0;

  /* release memory for physical geometry information */
  free (glm->phys);

  /* release memory associated with logic topology */
  if (glm->tpl) {
    QMP_delete_topology (glm->tpl);
    glm->tpl = 0;
  }

  /* close gm port */
  if (glm->port) {
#ifdef _QMP_USE_MULTI_PORTS
    /* free memory for receiving control message buffers */
    for (i = 0; i < QMP_CRLMSG_NBUF; i++) {
      gm_dma_free (glm->port, glm->ctrl_msg_buffers[i]);
      glm->ctrl_msg_buffers[i] = 0;
    }
#endif

    /* free memory for receiving general message buffers   */
    for (i = 0; i < QMP_GENMSG_NBUF; i++) {
      gm_dma_free (glm->port, glm->gen_msg_buffers[i]);
      glm->gen_msg_buffers[i] = 0;
    }


    /* free memory for free list for general message */
    QMP_delete_primitive_free_list (glm);

#ifdef _QMP_USE_MULTI_PORTS
    /* free all pending connection handles */
    {
      QMP_msghandle_i_t *p, *q;
      p = glm->conn_handles;
      while (p) {
	q = p->next;
	p->ch.state = QMP_NOT_CONNECTED;
	QMP_free_msghandle (p);
	p = q;
      }
    }

    /* free error control message */
    QMP_delete_ctrlmsg (glm->err_ctrlmsg, QMP_TRUE);
#endif
  }

  /**
   * Wait a while for gm to clean up memory.
   */
  gm_close (glm->port);
  glm->port = 0;


  /* free port name meomry */
  if (glm->port_name) {
    free (glm->port_name);
    glm->port_name = 0;
  }

  /* shut down the gm */
  gm_finalize ();
  
  glm->inited = QMP_FALSE;
}

/**
 * Shutdown this machine.
 */
void QMP_finalize_msg_passing (void)
{
  qmp_finalize_msg_passing_i (&QMP_global_m);
}

/**
 * Enable or Disable verbose mode.
 */
void
QMP_verbose (QMP_bool_t verbose)
{
  QMP_rt_verbose = verbose;
}


/**
 * return smp count for this job on this box.
 */
static const QMP_u32_t
qmp_get_SMP_count_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_smp_count_i");

  if (!glm->inited)
    return 0;

  return (QMP_u32_t)glm->num_cpus;
}

/**
 * Get smp count for this job on this box.
 */
const QMP_u32_t
QMP_get_SMP_count (void)
{
  return qmp_get_SMP_count_i (&QMP_global_m);
}

/**
 * Return Interconnect type.
 * Currently we have switched network only.
 */
static const QMP_ictype_t 
qmp_get_msg_passing_type_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_msg_passing_type_i");
  return glm->ic_type;
}

/**
 * Return Interconnect type.
 * Currently we have switched network only.
 */
const QMP_ictype_t 
QMP_get_msg_passing_type (void)
{
  return qmp_get_msg_passing_type_i (&QMP_global_m);
}


/**
 * Return number of dimensions for hardware configuration.
 * Currently we always return 0 since we have switched configuration.
 */
static const QMP_u32_t
qmp_get_allocated_number_of_dimensions_i  (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_allocated_number_of_dimensions_i");

  if (!glm->inited || !glm->phys) {
    QMP_error ("QMP system is not initialized.");
    exit (1);
  }
  if (glm->phys->type == QMP_SWITCH)
    return 0;
  else {
    QMP_error ("QMP supports only switched network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}


/**
 * Return number of dimensions for hardware configuration.
 * Currently we always return 0 since we have switched configuration.
 */
const QMP_u32_t
QMP_get_allocated_number_of_dimensions  (void)
{
  return qmp_get_allocated_number_of_dimensions_i (&QMP_global_m);
}

/**
 * Return size of allocated dimensions of grid machines.
 * Return null if network configuration is a switched configuration.
 */
static const QMP_u32_t *
qmp_get_allocated_dimensions_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_allocated_dimensions_i");

  if (!glm->inited || !glm->phys) {
    QMP_error ("QMP system is not initialized.");
    exit (1);
  }
  if (glm->phys->type == QMP_SWITCH)
    return 0;
  else {
    QMP_error ("QMP supports only switched network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}

/**
 * Return size of allocated dimensions of grid machines.
 * Return null if network configuration is a switched configuration.
 */
const QMP_u32_t *
QMP_get_allocated_dimensions (void)
{
  return qmp_get_allocated_dimensions_i (&QMP_global_m);
}

/**
 * Get coordinate information about allocated machines.
 * Return 0 if the configuration is a network switched configuration.
 *
 * caller should not free memory associated with this pointer.
 */
static const QMP_u32_t *
qmp_get_allocated_coordinates_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_allocated_coordinates_i");

  if (!glm->inited || !glm->phys) {
    QMP_error ("QMP system is not initialized.");
    exit (1);
  }
  if (glm->phys->type == QMP_SWITCH)
    return 0;
  else {
    QMP_error ("QMP supports only switched network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}


/**
 * Get coordinate information about allocated machines.
 * Return 0 if the configuration is a network switched configuration.
 *
 * caller should not free memory associated with this pointer.
 */
const QMP_u32_t *
QMP_get_allocated_coordinates (void)
{
  return qmp_get_allocated_coordinates_i (&QMP_global_m);
}

/**
 * Get number of allocated physical nodes for this parallel jobs.
 *
 * This number may be greater than the number of nodes a job really needs.
 */
const QMP_u32_t
qmp_get_allocated_number_of_nodes_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_allocated_number_of_nodes_i");

  if (!glm->inited || !glm->phys) {
    QMP_error ("QMP system is not initialized.");
    exit (1);
  }

  if (glm->phys->type == QMP_SWITCH)
    return QMP_NUM_NODES(glm->phys);
  else {
    QMP_error ("QMP supports only switched network configuration.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}

/**
 * Get number of allocated physical nodes for this parallel jobs.
 *
 * This number should be the same as the number requested by
 * a logical topology.
 */
const QMP_u32_t
QMP_get_number_of_nodes (void)
{
  return qmp_get_allocated_number_of_nodes_i (&QMP_global_m);
}


/**
 * Check whether a node is the physical root
 */
QMP_bool_t
QMP_is_primary_node (void)
{
  return (QMP_PHYS_RANK(QMP_global_m.phys) == 0) ? 1 : 0;
}
