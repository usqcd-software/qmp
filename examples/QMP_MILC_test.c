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
 *   $Log: not supported by cvs2svn $
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

#include <QMP.h>

/*
#define USE_CHANNEL_TABLE 0
*/
struct perf_argv
{
  QMP_u32_t size;
  QMP_u32_t loops;
};

#define SEND 0
#define RECV 1

struct channel
{
  QMP_msgmem_t    mm;
  QMP_msghandle_t mh;
  char *buf;
  int   size;
  int   node;
  int   type;
};

static struct channel* channel_table[100];
static int channel_table_size = 100;

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
 * Initialize message handle table
 */
static void
init_channel_table (void)
{
  int i;

  for (i = 0; i < channel_table_size; i++) {
    channel_table[i] = (struct channel *)malloc (sizeof(struct channel));
    channel_table[i]->mh = 0;
    channel_table[i]->mm = 0;
    channel_table[i]->buf = 0;
    channel_table[i]->size = 0;
    channel_table[i]->node = -1;
    channel_table[i]->type = -1;
  }
}

/**
 * Initialize message handle table
 */
static void
fina_channel_table (void)
{
  int i;

  for (i = 0; i < channel_table_size; i++) {
    if (channel_table[i]->mh) {
      QMP_free_msghandle (channel_table[i]->mh);
      QMP_free_msgmem (channel_table[i]->mm);
    }
  }
}

/**
 * Find a message handle from the table
 */
static QMP_msghandle_t
get_message_handle (char* buf, int size, int node, int type)
{
  int i;
  struct channel* p;

  for (i = 0; i < channel_table_size; i++) {
    p = channel_table[i];
    if (p->mh && p->buf == buf && p->size == size && p->node == node 
	&& p->type == type)
      return p->mh;
  }
  return 0;
}

/**
 * Create a new message handle and put into the table
 */
static QMP_msghandle_t
create_message_handle (char* buf, int size, int node, int type)
{
  int i;
  struct channel* p;
  QMP_msgmem_t mm;
  QMP_msghandle_t mh;

  mm = QMP_declare_msgmem(buf, size);
  if (type == SEND) 
    mh = QMP_declare_send_to(mm, node, 0);
  else 
    mh = QMP_declare_receive_from(mm, node, 0);

  if (!mh) {
    QMP_fprintf (stderr, "Cannot create message handle, QUIT\n");
    exit (1);
  }

  for (i = 0; i < channel_table_size; i++) {
    p = channel_table[i];
    if (p->mh == 0) {
      p->mh = mh;
      p->mm = mm;
      p->buf = buf;
      p->size = size;
      p->node = node;
      p->type = type;
      return p->mh;
    }
  }

  return 0;
}
    


/**
 *  send_field is to be called only by the node doing the sending
 */
static void
send_field(char *buf, int size, int tonode)
{
  QMP_msgmem_t mm;
  QMP_msghandle_t mh;

#ifdef USE_CHANNEL_TABLE
  mh = get_message_handle (buf, size, tonode, SEND);
  if (!mh)
    mh = create_message_handle (buf, size, tonode, SEND);
#else
  mm = QMP_declare_msgmem(buf, size);
  mh = QMP_declare_send_to(mm, tonode, 0);
#endif

  QMP_start(mh);
  QMP_wait(mh);

#ifndef USE_CHANNEL_TABLE
  QMP_free_msghandle(mh);
  QMP_free_msgmem(mm);
#endif
}

/**
 *  get_field is to be called only by the node to which the field was sent
 */
static void
get_field(char *buf, int size, int fromnode)
{
  QMP_msgmem_t mm;
  QMP_msghandle_t mh;

#ifdef USE_CHANNEL_TABLE
  mh = get_message_handle (buf, size, fromnode, RECV);
  if (!mh)
    mh = create_message_handle (buf, size, fromnode, RECV);
#else
  mm = QMP_declare_msgmem(buf, size);
  mh = QMP_declare_receive_from(mm, fromnode, 0);
#endif

  QMP_start(mh);
  QMP_wait(mh);

#ifndef USE_CHANNEL_TABLE
  QMP_free_msghandle(mh);
  QMP_free_msgmem(mm);
#endif
}

int
main (int argc, char** argv)
{
  int i, j, k;
  QMP_status_t status;
  QMP_u32_t num_nodes;
  struct perf_argv pargv;
  char  *sendm, *recvm, *mem;
  int   *value;
  double it, ft, bw;
  QMP_u32_t dims[4];
  QMP_u32_t ndims = 4;


  status = QMP_init_msg_passing (argc, argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) 
    QMP_error_exit ("QMP_init failed: %s\n", QMP_error_string(status));

#ifdef USE_CHANNEL_TABLE
  init_channel_table ();
#endif

  /* If this is the root node, get dimension information from key board */
  if (QMP_is_primary_node()) {
    QMP_fprintf (stderr, "Enter memory size and number of loops to run\n");
    scanf ("%d %d", &pargv.size, &pargv.loops);
  }
  
  if (QMP_broadcast (&pargv, sizeof(struct perf_argv)) != QMP_SUCCESS) 
    QMP_error_exit ("Cannot do broadcast, Quit\n");

  QMP_fprintf (stderr, "Memory size : %d number of loops : %d \n", 
	       pargv.size, pargv.loops);


  num_nodes = QMP_get_number_of_nodes ();
  
  /* set up one dimentionl logical array for now */
  dims[0] = num_nodes;
  for (i = 1; i < ndims; i++)
    dims[i] = 1;
  
  /* declare topology */
  status = QMP_declare_logical_topology (dims, ndims);
  if (status == QMP_FALSE)
    QMP_printf ("Cannot declare logical grid\n");

  if (QMP_is_primary_node()) 
    mem = sendm = (char *)QMP_allocate_aligned_memory (pargv.size);
  else
    mem = recvm = (char *)QMP_allocate_aligned_memory (pargv.size);

  /**
   * Now send/recv memory for num of loops
   */
  if (QMP_is_primary_node ()) {
    it = get_current_time ();
    for (i = 0; i < pargv.loops; i++) {
      value = (int *)sendm;
      for (k = 0; k < pargv.size/sizeof(int); k++)
	value[k] = i;
      for (j = 1; j < num_nodes; j++)
	send_field (sendm, pargv.size, j);
    }
    ft = get_current_time ();
    bw = pargv.size/(double)1000.0 * pargv.loops* (num_nodes - 1)/(ft - it);
    QMP_fprintf (stderr, "Sending bandwidth for %d number nodes with message size %d is %lf (MB/s)\n",
		 num_nodes - 1, pargv.size, bw);
  }
  else {
    it = get_current_time ();
    for (i = 0; i < pargv.loops; i++) {
      get_field (recvm, pargv.size, 0);
      value = (int *)recvm;
      for (k = 0; k < pargv.size/sizeof(int); k++)
	if (value[k] != i) 
	  QMP_fprintf (stderr, "Receiving error on loop %d\n", i);
    }
    ft = get_current_time ();
    bw = pargv.size/(double)1000.0 * pargv.loops/(ft - it);
    QMP_fprintf (stderr, "Receiving bandwidth with message size %d  is %lf (MB/s)\n",
		 pargv.size, bw);
  }
  
#ifdef USE_CHANNEL_TABLE
  fina_channel_table ();
#endif

  QMP_finalize_msg_passing ();

  QMP_free_aligned_memory (mem);
  return 0;
}
