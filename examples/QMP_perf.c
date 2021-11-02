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
 *   $Log: QMP_perf.c,v $
 *   Revision 1.5  2006/10/03 21:31:14  osborn
 *   Added "-qmp-geom native" command line option for BG/L.
 *
 *   Revision 1.5  2006/10/03 21:31:14  osborn
 *   Added "-qmp-geom native" command line option for BG/L.
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
/*#include <sys/time.h>*/
#include <math.h>

#include <qmp.h>

#define TEST_SIMUL 1
#define TEST_PINGPONG 2
#define TEST_ONEWAY 4
#define TEST_ALL 7

struct perf_argv
{
  int size;
  int minsize;
  int maxsize;
  int facsize;
  int loops;
  int verify;
  int option;
  int sender;
  int num_channels;
  int ndims;
};

int strided_send, strided_recv, strided_array_send;

/**
 * Get current time in milli seconds.
 */
static double
get_current_time (void)
{
#if 0
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec*1000.0 + tv.tv_usec/1000.0;
#endif
  return 1000*QMP_time();
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
  printf ("-strided-send:    use strided messages of given length for send\n");
  printf ("-strided-recv:    use strided messages of given length for recv\n");
  printf ("-strided-array-send: use strided array messages of given length for send\n");
}

static int
parse_options (int argc, char** argv, struct perf_argv* pargv)
{
  int i;

  /* set pargv to default values */
  pargv->size = 0;
  pargv->loops = 5000;
  pargv->verify = 0;
  pargv->option = TEST_ALL;
  pargv->num_channels = 1;
  strided_send = 0;
  strided_recv = 0;
  strided_array_send = 0;

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
    else if (strcmp (argv[i], "-strided-send") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-strided-send needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%i", &strided_send) < 1) {
	fprintf (stderr, "-strided-send needs a numerical argument.\n");
	return -1;
      }
    }
    else if (strcmp (argv[i], "-strided-recv") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-strided-recv needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%i", &strided_recv) < 1) {
	fprintf (stderr, "-strided-recv needs a numerical argument.\n");
	return -1;
      }
    }
    else if (strcmp (argv[i], "-strided-array-send") == 0) {
      if (++i >= argc) {
	fprintf (stderr, "-strided-array-send needs an argument.\n");
	return -1;
      }
      if (sscanf (argv[i], "%i", &strided_array_send) < 1) {
	fprintf (stderr, "-strided-array-send needs a numerical argument.\n");
	return -1;
      }
    } else {
      return -1;
    }
    i++;
  }

  if(pargv->size==0) {
    pargv->minsize = 6;
    if(pargv->minsize<strided_send) pargv->minsize = strided_send;
    if(pargv->minsize<strided_array_send) pargv->minsize = strided_array_send;
    if(pargv->minsize<strided_recv) pargv->minsize = strided_recv;
    pargv->maxsize = 65536;
    pargv->size = pargv->maxsize;
  } else {
    pargv->minsize = pargv->size;
    pargv->maxsize = pargv->size;
  }
  pargv->facsize = 2;

  return 0;
}

/**
 * Test ping and verify received message.
 */
static void
test_pingpong_verify (int** smem,
		      int** rmem,
		      QMP_msghandle_t* sendh,
		      QMP_msghandle_t* recvh,
		      struct perf_argv* pargv)
{
  double it, ft, dt, bwval;
  int    i, j, k;
  QMP_status_t err;
  int nc, ndims;
  int nloops;
  int dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;
  ndims = pargv->ndims;

  QMP_barrier();
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

  bwval = 2 * dsize/(double)1000.0 * 4 * nloops*nc*ndims/dt;

  QMP_printf ("Ping Pong Bandwidth for datasize %d is %g (MB/s)", 
	      dsize * 4, bwval);

  QMP_printf ("RTT/2 is %lf micro seconds", dt*1000.0/nloops/2);
}

/**
 * Test ping pong without verifying received message.
 */
static void
test_pingpong (int** smem,
	       int** rmem,
	       QMP_msghandle_t* sendh,
	       QMP_msghandle_t* recvh,
	       struct perf_argv* pargv)

{
  _QMP_UNUSED_ARGUMENT(smem);
  _QMP_UNUSED_ARGUMENT(rmem);

  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  int nc, ndims;
  int nloops;
  int dsize;
  QMP_bool_t sender;
  QMP_clear_to_send_t cts = QMP_CTS_READY;
  //QMP_clear_to_send_t cts = QMP_CTS_DISABLED;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;
  ndims = pargv->ndims;

  QMP_barrier();
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
	QMP_clear_to_send(sendh[j], cts);
      }

      for (j = 0; j < nc; j++) {
	/* receive operation */
	if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
      }
      
      for (j = 0; j < nc; j++) {
	if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in receiving %d\n", j);
	QMP_clear_to_send(recvh[j], cts);
      }
    }
    ft = get_current_time (); /* In milli seconds */

    dt = (ft - it); /* actual send time milli seconds */

    bwval = 2 * dsize/(double)1000.0 * 4 * nloops*nc*ndims/dt;

    if(QMP_get_node_number()==0) {
      QMP_printf ("Ping Pong Bandwidth for datasize %d is %g (MB/s)", 
		  dsize * 4, bwval);
      QMP_printf ("RTT/2 is %lf micro seconds", dt*1000.0/nloops/2);
      fflush(stdout);
    }
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
	QMP_clear_to_send(recvh[j], cts);
      }

      for (j = 0; j < nc; j++) {
	/* Send operation */
	if ((err = QMP_start (sendh[j])) != QMP_SUCCESS)
	  QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
      }

      for (j = 0; j < nc; j++) {
	if (QMP_wait (sendh[j]) != QMP_SUCCESS)
	  QMP_printf ("Error in sending %d\n", j );
	QMP_clear_to_send(sendh[j], cts);
      }
    }
  }
  QMP_barrier();
}

/**
 * Test oneway blast send
 */
static void
test_oneway (int** smem,
	     int** rmem,
	     QMP_msghandle_t* sendh,
	     QMP_msghandle_t* recvh,
	     struct perf_argv* pargv)
{
  _QMP_UNUSED_ARGUMENT(smem);
  _QMP_UNUSED_ARGUMENT(rmem);

  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  int nc, ndims;
  int nloops;
  int dsize;
  QMP_bool_t sender;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  sender = pargv->sender;
  ndims = pargv->ndims;

  QMP_barrier();
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

    bwval = dsize/(double)1000.0 * 4 * nloops*nc*ndims/dt;

    if(QMP_get_node_number()==0) {
      QMP_printf ("Oneway Bandwidth for datasize %d is %g (MB/s)", 
		  dsize * 4, bwval);
      QMP_printf ("time is %lf micro seconds", dt*1000.0/nloops);
      fflush(stdout);
    }
    QMP_barrier();
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

    bwval = dsize/(double)1000.0 * 4 * nloops*nc*ndims/dt;

    QMP_barrier();
    if(QMP_get_node_number()==1) {
      QMP_printf ("Oneway Bandwidth for datasize %d is %g (MB/s)", 
		  dsize * 4, bwval);
      QMP_printf ("time is %lf micro seconds", dt*1000.0/nloops);
      fflush(stdout);
    }
  }
  QMP_barrier();
}

static void
test_simultaneous_send (int** smem,
			int** rmem,
			QMP_msghandle_t* sendh,
			QMP_msghandle_t* recvh,
			struct perf_argv* pargv)
{
  _QMP_UNUSED_ARGUMENT(smem);
  _QMP_UNUSED_ARGUMENT(rmem);

  double it, ft, dt, bwval;
  int    i, j;
  QMP_status_t err;
  int nc, ndims;
  int nloops;
  int dsize;

  nc = pargv->num_channels;
  nloops = pargv->loops;
  dsize = pargv->size;
  ndims = pargv->ndims;

  /* do a test for nloops */
  QMP_barrier();
  it = get_current_time ();
  for (i = 0; i < nloops; i++) {
    for (j = 0; j < nc; j++) {
      /* receive operation */
      if ((err = QMP_start (recvh[j])) != QMP_SUCCESS)
	QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
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

    for (j = 0; j < nc; j++) {
      if (QMP_wait (recvh[j]) != QMP_SUCCESS)
	QMP_printf ("Error in receiving %d\n", j);
    }

  }
  ft = get_current_time ();

  if(QMP_get_node_number()==0) {
    dt = (ft - it); /* in milli seconds */

    bwval = dsize/(double)1000.0 * 4.0 * nloops * nc*ndims/dt;

    QMP_printf ("Simultaneous send B/W for datasize %d is %g (MB/s)", dsize * 4, bwval);

    QMP_printf ("Time difference is %lf micro seconds", dt*1000.0/nloops);
  }
  QMP_barrier();
}

#define NAMAX 32
void
create_msgs(int **smem, int **rmem,
	    QMP_msgmem_t *sendmem, QMP_msgmem_t *recvmem,
	    QMP_msghandle_t *sendh, QMP_msghandle_t *recvh,
	    int ndims, int nc, int size, struct perf_argv *pargv)
{
  int i, j, n;

  for (i = 0; i < nc; i++) {

    if(strided_array_send) {
      void *base[NAMAX];
      size_t bsize[NAMAX];
      int nblocks[NAMAX];
      ptrdiff_t stride[NAMAX];
      int tsize, skip;
      int na, k, bs, nb, nbt, ab, as, st;

      bs = strided_array_send;
      nbt = size/bs;
      na = sqrt(nbt);
      if(na<2) na = nbt;
      if(na>NAMAX) na = NAMAX;
      nb = nbt/na;
      st = 2*bs;
      skip = 3*bs;
      ab = bs*nb;
      as = st*nb+skip;

      tsize = 0;
      for(k=0; k<na; k++) {
	bsize[k] = bs*sizeof(int);
	stride[k] = st*sizeof(int);
	nblocks[k] = nb;
	if(k==na-1) nblocks[k] = nbt - nb*(na-1);
	tsize += skip + st * nblocks[k];
      }

      smem[i] = (int *)malloc(ndims*tsize*sizeof(int));
      for(n=0; n<ndims; n++) {
	for (j = 0; j < tsize; j++) { smem[i][n*tsize+j] = 0; }
	for (j = 0; j < size; j++) {
	  int ai, ak;
	  ak = j/ab;
	  if(ak>=na) ak = na-1;
	  ai = j-(ab*ak);
	  k = (as*ak) + st*(ai/bs) + (ai%bs);
	  smem[i][n*tsize+k] = i+j+1;
	}
	for(k=0; k<na; k++) {
	  base[k] = (void *)&(smem[i][n*tsize+as*k]);
	}
	sendmem[n*nc+i] =
	  QMP_declare_strided_array_msgmem(base, bsize, nblocks, stride, na);
	if(!sendmem[n*nc+i]) {
	  QMP_printf("error in declare strided msgmem\n");
	  QMP_abort(1);
	}
      }
    } else
    if(strided_send) {
      int tsize, bsize, stride, nblocks;

      bsize = strided_send;
      stride = 2*bsize;
      nblocks = size/bsize;
      tsize = stride * nblocks;

      smem[i] = (int *)malloc(ndims*tsize*sizeof (int));
      for(n=0; n<ndims; n++) {
	for (j = 0; j < tsize; j++) { smem[i][n*tsize+j] = 0; }
	for (j = 0; j < size; j++) {
	  int k = stride*(j/bsize) + (j%bsize);
	  smem[i][n*tsize+k] = i+j+1;
	}
	sendmem[n*nc+i] = QMP_declare_strided_msgmem(smem[i]+(n*tsize),
						     bsize*sizeof(int),
						     nblocks,
						     stride*sizeof(int));
	if(!sendmem[n*nc+i]) {
	  QMP_printf("error in declare strided msgmem\n");
	  QMP_abort(1);
	}
      }

    } else {

      smem[i] = (int *)malloc(ndims*size*sizeof(int));
      for(n=0; n<ndims; n++) {
	for (j = 0; j < size; j++) {
	  smem[i][n*size+j] = i+j+1;
	}
	sendmem[n*nc+i] = QMP_declare_msgmem(smem[i]+(n*size),
					     size*sizeof(int));
	if(!sendmem[n*nc+i]) {
	  QMP_printf("error in declare msgmem\n");
	  QMP_abort(1);
	}
      }

    }

    if(strided_recv) {
      int tsize, bsize, stride, nblocks;

      bsize = strided_recv;
      stride = 2*bsize;
      nblocks = size/bsize;
      tsize = stride * nblocks;

      rmem[i] = (int *)malloc(ndims*tsize*sizeof (int));
      for(n=0; n<ndims; n++) {
	for (j = 0; j < tsize; j++) {
	  rmem[i][n*tsize+j] = 0;
	}
	recvmem[n*nc+i] = QMP_declare_strided_msgmem(rmem[i]+(n*tsize),
						     bsize*sizeof(int),
						     nblocks,
						     stride*sizeof(int));
	if(!recvmem[n*nc+i]) {
	  QMP_printf("error in declare strided msgmem\n");
	  QMP_abort(1);
	}
      }

    } else {

      rmem[i] = (int *)malloc(ndims*size*sizeof (int));
      for(n=0; n<ndims; n++) {
	for (j = 0; j < size; j++) {
	  rmem[i][n*size+j] = 0;
	}
	recvmem[n*nc+i] = QMP_declare_msgmem (rmem[i]+(n*size),
					      size*sizeof(int));
	if(!recvmem[n*nc+i]) {
	  QMP_printf("error in declare msgmem\n");
	  QMP_abort(1);
	}
      }

    }

    if(ndims>0) {  // always use
      QMP_msghandle_t *tsend, *trecv;
      tsend = (QMP_msghandle_t *)malloc(ndims*sizeof(QMP_msghandle_t));
      trecv = (QMP_msghandle_t *)malloc(ndims*sizeof(QMP_msghandle_t));
      for(n=0; n<ndims; n++) {
	trecv[n] = QMP_declare_receive_relative (recvmem[n*nc+i], n, 1, 0);
	if (!trecv[n]) {
	  QMP_printf ("Recv Handle Error: %s\n", QMP_get_error_string(0));
	  exit (1);
	}
	tsend[n] = QMP_declare_send_relative (sendmem[n*nc+i], n, -1, 0);
	if (!tsend[n]) {
	  QMP_printf ("Send Handle Error: %s\n", QMP_get_error_string(0));
	  exit (1);
	}
      }
      if(pargv->option & TEST_PINGPONG) {
	if(pargv->sender) {
	  sendh[i] = QMP_declare_send_recv_pairs(tsend, ndims);
	  recvh[i] = QMP_declare_send_recv_pairs(trecv, ndims);
	} else {
	  recvh[i] = QMP_declare_send_recv_pairs(trecv, ndims);
	  sendh[i] = QMP_declare_send_recv_pairs(tsend, ndims);
	}
      } else {
	recvh[i] = QMP_declare_multiple(trecv, ndims);
	sendh[i] = QMP_declare_multiple(tsend, ndims);
      }
      if (!recvh[i]) {
	QMP_printf ("Recv Handle Error: %s\n", QMP_get_error_string(0));
	exit (1);
      }
      if (!sendh[i]) {
	QMP_printf ("Send Handle Error: %s\n", QMP_get_error_string(0));
	exit (1);
      }
      free(tsend);
      free(trecv);
    } else {
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

  }
}

void
free_msgs(int **smem, int **rmem,
	  QMP_msgmem_t *sendmem, QMP_msgmem_t *recvmem,
	  QMP_msghandle_t *sendh, QMP_msghandle_t *recvh,
	  int ndims, int nc)
{
  int j, n;

  for (j = 0; j < nc; j++) {
    QMP_free_msghandle (recvh[j]);
    QMP_free_msghandle (sendh[j]);

    for(n=0; n<ndims; n++) {
      QMP_free_msgmem (recvmem[n*nc+j]);
      QMP_free_msgmem (sendmem[n*nc+j]);
    }

    free (smem[j]);
    free (rmem[j]);
  }
}

void
check_mem(int **rmem, int ndims, int nc, int size)
{
  int i, j, n;
  int tsize, bsize, stride, nblocks;

  if(strided_recv) {
    bsize = strided_recv;
    stride = 2*bsize;
    nblocks = size/bsize;
    tsize = stride * nblocks;
  } else {
    bsize = 0;
    stride = 0;
    nblocks = 0;
    tsize = size;
  }
  //fprintf(stderr,"%i %i\n", size, tsize);
  //return;

  for (i = 0; i < nc; i++) {
    for(n=0; n<ndims; n++) {
      for (j = 0; j < tsize; j++) {
	int k;
	if(strided_recv) {
	  if((j%stride)<bsize) {
	    k = i + 1 + bsize*(j/stride) + (j%stride);
	  } else {
	    k = 0;
	  }
	} else {
	  k = i+j+1;
	}
	if (rmem[i][n*tsize+j] != k)
	  QMP_printf ("Receiving memory error: rmem[%i][%i] = %i =/= %i", 
		      i, j, rmem[i][n*tsize+j], k);
      }
    }
  }
}

int
main (int argc, char** argv)
{
  int             i, nc;
  QMP_status_t      status;
  int       **smem, **rmem;
  QMP_msgmem_t    *recvmem;
  QMP_msghandle_t *recvh;
  QMP_msgmem_t    *sendmem;
  QMP_msghandle_t *sendh;
  struct perf_argv pargv;
  QMP_thread_level_t req, prv;

  /** 
   * Simple point to point topology 
   */
  int dims[4] = {2,2,2,2};
  int ndims = 1;

  //if(QMP_get_node_number()==0)
  //printf("starting init\n"); fflush(stdout);
  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);
  if (status != QMP_SUCCESS) {
    fprintf (stderr, "QMP_init failed\n");
    return -1;
  }
  if(QMP_get_node_number()==0) {
    printf("finished init\n"); fflush(stdout);
  }
 
  if (parse_options (argc, argv, &pargv) == -1) {
    if(QMP_get_node_number()==0)
      usage (argv[0]);
    exit (1);
  }

  {
    int maxdims = 4;
    int k=0;
    int nodes = QMP_get_number_of_nodes();
    ndims = 0;
    while( (nodes&1) == 0 ) {
      if(ndims<maxdims) ndims++;
      else {
	dims[k] *= 2;
	k++;
	if(k>=maxdims) k = 0;
      }
      nodes /= 2;
    }
    if(nodes != 1) {
      QMP_error("invalid number of nodes %i", QMP_get_number_of_nodes());
      QMP_error(" must power of 2");
      QMP_abort(1);
    }
    pargv.ndims = ndims;
  }

  status = QMP_declare_logical_topology (dims, ndims);
  if (status != QMP_SUCCESS) {
    fprintf (stderr, "Cannot declare logical grid\n");
    return -1;
  }

  /* do a broadcast of parameter */
  if (QMP_broadcast (&pargv, sizeof (pargv)) != QMP_SUCCESS) {
    QMP_printf ("Broadcast parameter failed\n");
    exit (1);
  }

  {
    int k=1;
    const int *lc = QMP_get_logical_coordinates();
    for(i=0; i<ndims; i++) k += lc[i];
    pargv.sender = k&1;
  }

  QMP_printf("%s options: num_channels[%d] verify[%d] option[%d] datasize[%d] numloops[%d] sender[%d] strided_send[%i] strided_recv[%i] strided_array_send[%i] ",
	     argv[0], pargv.num_channels, pargv.verify, 
	     pargv.option, pargv.size, pargv.loops, pargv.sender,
	     strided_send, strided_recv, strided_array_send);
  fflush(stdout);


  /**
   * Create memory
   */
  nc = pargv.num_channels;
  smem = (int **)malloc(nc*sizeof (int *));
  rmem = (int **)malloc(nc*sizeof (int *));
  sendmem = (QMP_msgmem_t *)malloc(ndims*nc*sizeof (QMP_msgmem_t));
  recvmem = (QMP_msgmem_t *)malloc(ndims*nc*sizeof (QMP_msgmem_t));
  sendh = (QMP_msghandle_t *)malloc(nc*sizeof (QMP_msghandle_t));
  recvh = (QMP_msghandle_t *)malloc(nc*sizeof (QMP_msghandle_t));

  QMP_barrier();
  if(QMP_get_node_number()==0) {
    printf("\n"); fflush(stdout);
  }
  if(pargv.option & TEST_SIMUL) {
    int opts = pargv.option;
    pargv.option = TEST_SIMUL;
    if(QMP_get_node_number()==0) {
      QMP_printf("starting simultaneous sends"); fflush(stdout);
    }
    for(i=pargv.minsize; i<=pargv.maxsize; i*=pargv.facsize) {
      pargv.size = i;
      create_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc, i, &pargv);
      test_simultaneous_send (smem, rmem, sendh, recvh, &pargv);
      check_mem(rmem, ndims, nc, i);
      free_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc);
    }
    if(QMP_get_node_number()==0) {
      QMP_printf("finished simultaneous sends\n"); fflush(stdout);
    }
    pargv.option = opts;
  }

  if(pargv.option & TEST_PINGPONG) {
    int opts = pargv.option;
    pargv.option = TEST_PINGPONG;
    if(QMP_get_node_number()==0) {
      QMP_printf("starting ping pong sends"); fflush(stdout);
    }
    for(i=pargv.minsize; i<=pargv.maxsize; i*=pargv.facsize) {
      pargv.size = i;
      create_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc, i, &pargv);
      if(pargv.verify)
	test_pingpong_verify(smem, rmem, sendh, recvh, &pargv);
      else
	test_pingpong(smem, rmem, sendh, recvh, &pargv);
      check_mem(rmem, ndims, nc, i);
      free_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc);
    }
    if(QMP_get_node_number()==0) {
      QMP_printf("finished ping pong sends\n"); fflush(stdout);
    }
    pargv.option = opts;
  }

  if(pargv.option & TEST_ONEWAY) {
    int opts = pargv.option;
    pargv.option = TEST_ONEWAY;
    if(QMP_get_node_number()==0) {
      QMP_printf("starting one way sends"); fflush(stdout);
    }
    for(i=pargv.minsize; i<=pargv.maxsize; i*=pargv.facsize) {
      pargv.size = i;
      create_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc, i, &pargv);
      test_oneway (smem, rmem, sendh, recvh, &pargv);
      if(!pargv.sender) check_mem(rmem, ndims, nc, i);
      free_msgs(smem, rmem, sendmem, recvmem, sendh, recvh, ndims, nc);
    }
    if(QMP_get_node_number()==0) {
      QMP_printf("finished one way sends"); fflush(stdout);
    }
    pargv.option = opts;
  }


  /**
   * Free memory 
   */
  free (smem);
  free (rmem);

  free (sendh);
  free (recvh);

  free (sendmem);
  free (recvmem);

  QMP_finalize_msg_passing ();

  return 0;
}
