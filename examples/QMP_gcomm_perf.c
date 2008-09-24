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
 *   $Log: QMP_gcomm_perf.c,v $
 *   Revision 1.5  2006/01/05 03:12:56  osborn
 *   Added --enable-bgl compilation option.
 *
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

void
usage(char *prog)
{
  if(QMP_get_node_number()==0) {
    fprintf(stderr, "%s [-v] [num iters]\n", prog);
    fprintf(stderr, "  -v : verify on\n");
  }
  QMP_abort(1);
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

  /* parse command line */
  if(argc>3) usage(argv[0]);
  my_arg.verify = QMP_FALSE;
  my_arg.loops = 10;
  for(i=1; i<argc; i++) {
    if(strcmp(argv[i],"-v")==0) {
      my_arg.verify = QMP_TRUE;
    } else {
      my_arg.loops = atoi(argv[i]);
      if(my_arg.loops<=0) usage(argv[0]);
    }
  }

  QMP_info("Running %d iterations for global sum.", my_arg.loops);
  if (my_arg.verify)
    QMP_info("Verification is on.");
  else
    QMP_info("Verification is off.");

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

  QMP_info ("Summing an int on %d nodes takes %lf microseconds.",
	    QMP_get_number_of_nodes(), (ft - it)*1000.0/my_arg.loops);

  QMP_finalize_msg_passing ();

  return 0;
}

