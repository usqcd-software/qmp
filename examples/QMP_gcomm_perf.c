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
 *      Simple Performance Test for global communication using QMP
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
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
 *   Revision 1.1  2002/12/05 16:41:02  chen
 *   Add new global communication BufferFly algorithm
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <qmp.h>

typedef struct gcomm_arg_
{
  int  loops;
  QMP_bool_t verify;
}gcomm_arg_t;


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

int 
main (int argc, char** argv)
{
  int i;
  QMP_status_t status;
  int    rank, size;
  int          value, result;
  double       it, ft;
  gcomm_arg_t  my_arg;
  char         verify[32];
  QMP_thread_level_t req, prv;
  

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);

  if (status != QMP_SUCCESS) {
    QMP_error ("QMP_init failed: %s\n", QMP_error_string(status));
    QMP_abort (1);
  }

#ifdef USE_CHANNEL_TABLE
  init_channel_table ();
#endif

  /* If this is the root node, get dimension information from key board */
  if (QMP_is_primary_node()) {
    QMP_fprintf (stderr, "Do you want to verify result[y/n]?\n");
    scanf ("%s", verify);
    if (verify[0] == 'y' || verify[0] == 'Y')
      my_arg.verify = QMP_TRUE;
    else
      my_arg.verify = QMP_FALSE;

    QMP_fprintf (stderr, "Enter number of loops to run.\n");
    scanf ("%d", &my_arg.loops);
  }

  if (QMP_broadcast (&my_arg, sizeof(gcomm_arg_t)) != QMP_SUCCESS) 
    QMP_abort_string (1, "Cannot do broadcast for arguments, Quit\n");

  if (my_arg.verify)
    QMP_info ("Running %d number of loops for global sum with verification on.\n", 
	      my_arg.loops);
  else
    QMP_info ("Running %d number of loops for global sum with verification off.\n", 
	      my_arg.loops);

  rank = QMP_get_node_number ();
  size = QMP_get_number_of_nodes ();
  
  it = get_current_time ();
  for (i = 0; i < my_arg.loops; i++) {
    if (my_arg.verify)
      value = rank + 1232 + i % 97;
    else
      value = 1;
    QMP_sum_int (&value);
    if (my_arg.verify) {
      result = size*(size - 1)/2 + 1232 * size + size * (i % 97);
      if (value != result) {
	QMP_error ("Expecting global sum %d and value is %d\n", 
		   result, value);
      }
    }
  }
    
  ft = get_current_time ();

  QMP_info ("Summing for a int value for %d nodes takes %lf microseconds.\n",
	    QMP_get_number_of_nodes(), (ft - it)*1000.0/my_arg.loops);

  QMP_finalize_msg_passing ();

  return 0;
}

