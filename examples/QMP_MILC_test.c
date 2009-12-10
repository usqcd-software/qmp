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
 *      Simple Test Program for MILC Type of Communication
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_MILC_test.c,v $
 *   Revision 1.4  2004/06/14 20:36:30  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.3  2003/02/13 16:23:04  chen
 *   qmp version 1.2
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:37  chen
 *   check into lattice group
 *
 *   Revision 1.2  2003/01/08 20:37:47  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.1  2002/11/15 15:37:32  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include <qmp.h>

struct perf_argv
{
  int size;
  int loops;
};

#define SEND 0
#define RECV 1

/**
 * Get current time in milli seconds.
 */
static double
get_current_time (void)
{
  struct timeval tv;

  gettimeofday (&tv, 0);

  return tv.tv_sec*1000.0 + tv.tv_usec/1000.0;
}


/**
 *  send_field is to be called only by the node doing the sending
 */
static void
send_field(char *buf, int size, int tonode)
{
  QMP_msgmem_t mm;
  QMP_msghandle_t mh;

  mm = QMP_declare_msgmem(buf, size);
  mh = QMP_declare_send_to(mm, tonode, 0);

  QMP_start(mh);
  QMP_wait(mh);

  QMP_free_msghandle(mh);
  QMP_free_msgmem(mm);
}

/**
 *  get_field is to be called only by the node to which the field was sent
 */
static void
get_field(char *buf, int size, int fromnode)
{
  QMP_msgmem_t mm;
  QMP_msghandle_t mh;

  mm = QMP_declare_msgmem(buf, size);
  mh = QMP_declare_receive_from(mm, fromnode, 0);

  QMP_start(mh);
  QMP_wait(mh);

  QMP_free_msghandle(mh);
  QMP_free_msgmem(mm);
}

int
main (int argc, char** argv)
{
  int i, j, k;
  QMP_status_t status;
  int num_nodes;
  struct perf_argv pargv;
  QMP_mem_t  *mem;
  int   *value;
  double it, ft, bw;
  int dims[4];
  int ndims = 4;
  QMP_thread_level_t req, prv;
  
  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);

  if (status != QMP_SUCCESS) {
    QMP_error ("QMP_init failed: %s\n", QMP_error_string(status));
    QMP_abort(1);
  }

  /* If this is the root node, get dimension information from key board */
  if (QMP_is_primary_node()) {
    QMP_fprintf (stderr, "Enter memory size and number of loops to run\n");
    scanf ("%d %d", &pargv.size, &pargv.loops);
  }
  
  if (QMP_broadcast (&pargv, sizeof(struct perf_argv)) != QMP_SUCCESS) 
    QMP_abort_string (1, "Cannot do broadcast, Quit\n");

  QMP_fprintf (stderr, "Memory size : %d number of loops : %d \n", 
	       pargv.size, pargv.loops);


  num_nodes = QMP_get_number_of_nodes ();
  
  /* set up one dimentionl logical array for now */
  dims[0] = num_nodes;
  for (i = 1; i < ndims; i++)
    dims[i] = 1;
  
  /* declare topology */
  status = QMP_declare_logical_topology (dims, ndims);
  if (status != QMP_SUCCESS)
    QMP_printf ("Cannot declare logical grid\n");

  if (QMP_is_primary_node()) 
    mem = QMP_allocate_memory (pargv.size);
  else
    mem = QMP_allocate_memory (pargv.size);

  /**
   * Now send/recv memory for num of loops
   */
  if (QMP_is_primary_node ()) {
    it = get_current_time ();
    for (i = 0; i < pargv.loops; i++) {
      value = (int *)QMP_get_memory_pointer(mem);
      for (k = 0; k < pargv.size/sizeof(int); k++)
	value[k] = i;
      for (j = 1; j < num_nodes; j++)
	send_field ((char *)value, pargv.size, j);
    }
    ft = get_current_time ();
    bw = pargv.size/(double)1000.0 * pargv.loops* (num_nodes - 1)/(ft - it);
    QMP_fprintf (stderr, "Sending bandwidth for %d number nodes with message size %d is %lf (MB/s)\n",
		 num_nodes - 1, pargv.size, bw);
  }
  else {
    it = get_current_time ();
    for (i = 0; i < pargv.loops; i++) {
      value = (int *)QMP_get_memory_pointer(mem);
      get_field ((char *)value, pargv.size, 0);
      for (k = 0; k < pargv.size/sizeof(int); k++)
	if (value[k] != i) 
	  QMP_fprintf (stderr, "Receiving error on loop %d\n", i);
    }
    ft = get_current_time ();
    bw = pargv.size/(double)1000.0 * pargv.loops/(ft - it);
    QMP_fprintf (stderr, "Receiving bandwidth with message size %d  is %lf (MB/s)\n",
		 pargv.size, bw);
  }
  
  QMP_finalize_msg_passing ();

  QMP_free_memory (mem);
  return 0;
}
