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
 *      Simple Performance Test Program for QMP between two nodes
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1.1.1  2003/01/27 19:31:37  chen
 *   check into lattice group
 *
 *   Revision 1.5  2003/01/08 20:37:49  chen
 *   Add new implementation to use one gm port
 *
 *   Revision 1.4  2002/12/05 16:41:03  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.3  2002/10/04 18:13:18  chen
 *   add oneway test
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:43  chen
 *   Version 0.95 Release
 *
 *   Revision 1.6  2002/03/27 20:48:50  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.5  2002/02/15 20:34:54  chen
 *   First Beta Release QMP
 *
 *   Revision 1.4  2002/01/27 20:53:50  chen
 *   minor change
 *
 *   Revision 1.3  2002/01/24 20:10:50  chen
 *   Nearest Neighbor Communication works
 *
 *   Revision 1.2  2002/01/22 15:52:29  chen
 *   Minor change in code
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:51  chen
 *   initial import of QMP
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <QMP.h>

#define TEST_SIMUL 0
#define TEST_PINGPONG 1
#define TEST_ONEWAY 2

struct perf_argv
{
  QMP_u32_t size;
  QMP_u32_t loops;
  QMP_u32_t verify;
  QMP_u32_t option;
  QMP_u32_t sender;
  QMP_u32_t num_channels;
};

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

static void
usage (char* prog)
{
  printf ("%s usage: -v -size bytes -loops num\n", prog);
  printf ("      -n:         number of channels (1)\n");
  printf ("      -v:         verify data (no verify)\n");
  printf (" -pingpong:       pingpong test (no) \n");
  printf (" -oneway:         oneway test (no) \n");
  printf ("   -size:         transfer data size in 4 byte word (1024)\n");
  printf ("  -loops:         number of loops to run (10000)\n");
}

static int
parse_options (int argc, char** argv, struct perf_argv* pargv)
{
  int i;

  /* set pargv to default values */
  pargv->size = 1024;
  pargv->loops = 5000;
  pargv->verify = 0;
  pargv->option = TEST_SIMUL;
  pargv->num_channels = 1;

  i = 1;

  while (i < argc) {
    if (strcmp (argv[i], "-pingpong") == 0)
      pargv->option = TEST_PINGPONG;
    else if (strcmp (argv[i], "-oneway") == 0)
      pargv->option = TEST_ONEWAY;
    else if (strcmp (argv[i], "-v") == 0)
      pargv->verify = 1;
    else if (strcmp (argv[i], "-n") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-n needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%d", &pargv->num_channels) < 1) {
	fprintf (stderr, "-n needs a numerical argument.\n");
	return -1;
      }
    }
    else if (strcmp (argv[i], "-size") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-size needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%d", &pargv->size) < 1) {
	fprintf (stderr, "-size needs a numerical argument.\n");
	return -1;
      }
    }
    else if (strcmp (argv[i], "-loops") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-loops needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%d", &pargv->loops) < 1) {
	fprintf (stderr, "-loops needs a numerical argument.\n");
	return -1;
      }
    }
    i++;
  }

  return 0;
}

/**
 * Test ping and verify received message.
 */
static void
test_pingpong_verify (QMP_s32_t** smem,
		      QMP_s32_t** rmem,
		      QMP_msghandle_t* sendh,
		      QMP_msghandle_t* recvh,
		      struct perf_argv* pargv)
{
  double it, ft, dt, bwval;
  int    i, j, k;
  QMP_status_t err;
  QMP_u32_t nc;
  QMP_u32_t nloops;
  QMP_u32_t dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;

  it = get_current_time ();

  for (i = 0; i < nloops; i++) {
    for (j = 0; j < nc; j++) {
      for (k = 0; k < dsize; k++) {
	rmem[j][k] = 0;
	smem[j][k] = i + k * j + nc*nc;
      }
    }

    if (sender) {
      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
      }

      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }
      
      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
      }
    }
    else { 
      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }
	
      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
      }
	
      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
      }
    }
    
    /* verify memory */
    for (j = 0; j < nc; j++) {
      for (k = 0; k < dsize; k++)
	if (rmem[j][k] != i + k * j + nc* nc)
	  QMP_printf ("Receiving memory error for memory %d %d %d\n", 
		  j, k, rmem[j][k]);
    }

  }
  ft = get_current_time (); /* In milli seconds */

  dt = (ft - it); /* actual send time milli seconds */

  bwval = dsize/(double)1000.0 * 4 * nloops*nc/dt;

  QMP_printf ("Ping Pong Bandwidth for datasize %d is %lf (MB/s)\n", 
	      dsize * 4, bwval);
      
  QMP_printf ("RTT/2 is %lf micro seconds\n", dt*1000.0/nloops/2);
}

/**
 * Test ping pong without verifying received message.
 */
static void
test_pingpong (QMP_s32_t** smem,
	       QMP_s32_t** rmem,
	       QMP_msghandle_t* sendh,
	       QMP_msghandle_t* recvh,
	       struct perf_argv* pargv)

{
  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  QMP_u32_t nc;
  QMP_u32_t nloops;
  QMP_u32_t dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;

  if (sender) {
    it = get_current_time ();
    for (i = 0; i < nloops; i++) {
      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
      }

      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }
      
      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
      }
    }
    ft = get_current_time (); /* In milli seconds */

    dt = (ft - it); /* actual send time milli seconds */

    bwval = dsize/(double)1000.0 * 4 * nloops*nc/dt;

    QMP_printf ("Ping Pong Bandwidth for datasize %d is %lf (MB/s)\n", 
	    dsize * 4, bwval);
      
    QMP_printf ("RTT/2 is %lf micro seconds\n", dt*1000.0/nloops/2);
  }
  else { 
    for (i = 0; i < nloops; i++) {
      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
      }

      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
      }
    }
  }    
}

/**
 * Test oneway blast send
 */
static void
test_oneway (QMP_s32_t** smem,
	     QMP_s32_t** rmem,
	     QMP_msghandle_t* sendh,
	     QMP_msghandle_t* recvh,
	     struct perf_argv* pargv)
{
  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  QMP_u32_t nc;
  QMP_u32_t nloops;
  QMP_u32_t dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;

  if (sender) {
    it = get_current_time ();
    for (i = 0; i < nloops; i++) {
      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
      }
    }
    ft = get_current_time (); /* In milli seconds */

    dt = (ft - it); /* actual send time milli seconds */

    bwval = dsize/(double)1000.0 * 4 * nloops*nc/dt;

    QMP_printf ("Oneway Bandwidth for datasize %d is %lf (MB/s)\n", 
	    dsize * 4, bwval);
      
    QMP_printf ("RTT/2 is %lf micro seconds\n", dt*1000.0/nloops/2);
  }
  else { 
    it = get_current_time ();
    for (i = 0; i < nloops; i++) {
      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
      }
    }
    ft = get_current_time (); /* In milli seconds */

    dt = (ft - it); /* actual send time milli seconds */

    bwval = dsize/(double)1000.0 * 4 * nloops*nc/dt;

    QMP_printf ("Oneway Bandwidth for datasize %d is %lf (MB/s)\n", 
	    dsize * 4, bwval);
      
    QMP_printf ("RTT/2 is %lf micro seconds\n", dt*1000.0/nloops/2);
  }    
}

static void
test_simultaneous_send (QMP_s32_t** smem,
			QMP_s32_t** rmem,
			QMP_msghandle_t* sendh,
			QMP_msghandle_t* recvh,
			struct perf_argv* pargv)
{
  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  QMP_u32_t nc;
  QMP_u32_t nloops;
  QMP_u32_t dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;

  /* do a test for nloops */
  it = get_current_time ();
  for (i = 0; i < nloops; i++) {
    for (j = 0; j < nc; j++) {
      /* Send operation */
      if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
    }

    for (j = 0; j < nc; j++) {
      /* receive operation */
      if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
    }


    for (j = 0; j < nc; j++) {
      if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	QMP_printf ("Error in receiving %d\n", j);
    }

    for (j = 0; j < nc; j++) {
      if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	QMP_printf ("Error in sending %d\n", j );
    }

  }
  ft = get_current_time ();

  if (sender) {
    dt = (ft - it); /* in milli seconds */

    bwval = dsize/(double)1000.0 * 4.0 * nloops * nc/dt;
    
    QMP_printf ("Simultaneous send bandwidth for %d channels with datasize %d is %lf (MB/s)\n", nc, dsize * 4, bwval);
    
    QMP_printf ("Time difference is %lf micro seconds\n", dt*1000.0/nloops);
  }
}
    
int main (int argc, char** argv)
{
  int             i, j, nc;
  QMP_bool_t      status;
  QMP_u32_t       rank, dsize;
  QMP_s32_t       **smem, **rmem;
  QMP_msgmem_t    *recvmem;
  QMP_msghandle_t *recvh;
  QMP_msgmem_t    *sendmem;
  QMP_msghandle_t *sendh;
  struct perf_argv pargv;

  /** 
   * Simple point to point topology 
   */
  QMP_u32_t dims[1] = {2};
  QMP_u32_t ndims = 1;
  
  if (parse_options (argc, argv, &pargv) == -1) {
    usage (argv[0]);
    exit (1);
  }

  status = QMP_init_msg_passing (&argc, &argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) {
    fprintf (stderr, "QMP_init failed\n");
    return -1;
  }
  status = QMP_declare_logical_topology (dims, ndims);

  if (status == QMP_FALSE) {
    fprintf (stderr, "Cannot declare logical grid\n");
    return -1;
  }

  /* do a broadcast of parameter */
  if (QMP_broadcast (&pargv, sizeof (pargv)) != QMP_SUCCESS) {
    QMP_printf ("Broadcast parameter failed\n");
    exit (1);
  }

  rank = QMP_get_node_number ();
  if (rank == 0) 
    pargv.sender = 1;
  else
    pargv.sender = 0;

  QMP_fprintf (stderr, "%s options: num_channels [%d] verify [%d] option [%d] datasize [%d] numloops [%d] sender[%d]\n",
	      argv[0], pargv.num_channels, pargv.verify, 
	      pargv.option, pargv.size, pargv.loops, pargv.sender);


  /**
   * Create memory
   */
  nc = pargv.num_channels;
  dsize = pargv.size;
  smem = (QMP_s32_t **)malloc(nc*sizeof (QMP_s32_t *));
  rmem = (QMP_s32_t **)malloc(nc*sizeof (QMP_s32_t *));
  sendmem = (QMP_msgmem_t *)malloc(nc*sizeof (QMP_msgmem_t *));
  recvmem = (QMP_msgmem_t *)malloc(nc*sizeof (QMP_msgmem_t *));
  sendh = (QMP_msghandle_t *)malloc(nc*sizeof (QMP_msghandle_t *));
  recvh = (QMP_msghandle_t *)malloc(nc*sizeof (QMP_msghandle_t *));
  
  for (i = 0; i < nc; i++) {
    smem[i] = (QMP_s32_t *)malloc(dsize*sizeof (QMP_s32_t));
    rmem[i] = (QMP_s32_t *)malloc(dsize*sizeof (QMP_s32_t));
    for (j = 0; j < dsize; j++) {
      rmem[i][j] = 0;
      smem[i][j] = j;
    }

    sendmem[i] = QMP_declare_msgmem (smem[i], dsize*sizeof (QMP_s32_t));
    recvmem[i] = QMP_declare_msgmem (rmem[i], dsize*sizeof (QMP_s32_t));

    recvh[i] = QMP_declare_receive_relative (recvmem[i], 0, 1, 0);
    if (!recvh[i]) {
      QMP_printf ("Recv Handle Error: %s\n", QMP_get_error_string(0));      
      exit (1);
    }

    sendh[i] = QMP_declare_send_relative (sendmem[i], 0, -1, 0);
    
    if (!sendh[i]) {
      QMP_printf ("Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }

  }

  if (pargv.option == TEST_PINGPONG && pargv.verify) 
    test_pingpong_verify(smem, rmem, sendh, recvh, &pargv);
  else if (pargv.option == TEST_PINGPONG)
    test_pingpong(smem, rmem, sendh, recvh, &pargv);
  else if (pargv.option == TEST_ONEWAY)
    test_oneway (smem, rmem, sendh, recvh, &pargv);
  else
    test_simultaneous_send (smem, rmem, sendh, recvh, &pargv);


  /**
   * Free memory 
   */
  for (j = 0; j < nc; j++) {
    QMP_free_msghandle (recvh[j]);
    QMP_free_msghandle (sendh[j]);

    QMP_free_msgmem (recvmem[j]);
    QMP_free_msgmem (sendmem[j]);

    free (smem[j]);
    free (rmem[j]);
  }

  free (smem);
  free (rmem);

  free (sendh);
  free (recvh);

  free (sendmem);
  free (recvmem);

  QMP_finalize_msg_passing ();

  return 0;
}
    
  
