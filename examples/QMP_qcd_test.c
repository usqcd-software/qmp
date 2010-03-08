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
 *      Simple LQCD Style Communication Test
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_qcd_test.c,v $
 *   Revision 1.5  2004/12/16 02:44:12  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
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
 *   Revision 1.4  2002/10/03 16:46:36  chen
 *   Add memory copy, change event loops
 *
 *   Revision 1.3  2002/09/25 14:41:16  chen
 *   allow memory size in the arguments list
 *
 *   Revision 1.1  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
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
#include <sys/time.h>
#include <time.h>

#include <qmp.h>

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

#define MEM_SIZE 81920 

/* This is the memory size for 4x4x4x4: SIZE = 48XPHASE SIZE */

#define Nd 4
#define Nc 3
#define Ns 4
#define Ns2 2


/* Nearest neighbor communication channels */
static QMP_mem_t* forw_mem[Nd][2];
static QMP_mem_t* back_mem[Nd][2];

static QMP_msgmem_t forw_msg[Nd][2];
static QMP_msgmem_t back_msg[Nd][2];
static QMP_msghandle_t forw_mh[Nd][2];
static QMP_msghandle_t back_mh[Nd][2];
static QMP_msghandle_t forw_all_mh;
static QMP_msghandle_t back_all_mh;

static int num = 0;
static int mem_size = MEM_SIZE;
static int verify = 0;


void init_wnxtsu3dslash(void)
{
  int mu;
  const int *size = QMP_get_logical_dimensions();

  if (QMP_get_logical_number_of_dimensions() != Nd)
    QMP_abort_string(1, "ini_wnxtsu3dslash: number of logical dimensions does not match problem");

  /* Loop over all communicating directions and build up the two message
   * handles. If there is no communications, the message handles will not
   * be initialized 
   */
  num = 0;

  for(mu=0; mu < Nd; ++mu)
  {
    //QMP_printf("setup dir %i size %i", mu, size[mu]);
    if(size[mu] > 1)
    {
      forw_mem[num][0] = QMP_allocate_memory (mem_size);
      forw_mem[num][1] = QMP_allocate_memory (mem_size);

      forw_msg[num][0] = QMP_declare_msgmem(QMP_get_memory_pointer(forw_mem[num][0]), mem_size);
      forw_msg[num][1] = QMP_declare_msgmem(QMP_get_memory_pointer(forw_mem[num][1]), mem_size);

      forw_mh[num][0]  = QMP_declare_receive_relative(forw_msg[num][1], mu, +1, 0);
      forw_mh[num][1]  = QMP_declare_send_relative(forw_msg[num][0], mu, -1, 0);

      back_mem[num][0] = QMP_allocate_memory (mem_size);
      back_mem[num][1] = QMP_allocate_memory (mem_size);

      back_msg[num][0] = QMP_declare_msgmem(QMP_get_memory_pointer(back_mem[num][0]), mem_size);
      back_msg[num][1] = QMP_declare_msgmem(QMP_get_memory_pointer(back_mem[num][1]), mem_size);

      back_mh[num][0]  = QMP_declare_receive_relative(back_msg[num][1], mu, -1, 0);
      back_mh[num][1]  = QMP_declare_send_relative(back_msg[num][0], mu, +1, 0);

      num++;
    }
  }

  if (num > 0)
  {
    forw_all_mh = QMP_declare_multiple(&(forw_mh[0][0]), 2*num);
    back_all_mh = QMP_declare_multiple(&(back_mh[0][0]), 2*num);
  }

}


int main (int argc, char** argv)
{
  int i, j, k, loops;
  QMP_status_t status, err;
  double it, ft, dt, bwval;
  QMP_thread_level_t req, prv;

  int dims[4] = {1,1,1,1};
  int ndims = 4;

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);
  if (argc < 3) {
    if (QMP_is_primary_node())
      fprintf (stderr, "Usage: %s numloops msgsize [-v]\n", argv[0]);
    exit (1);
  }

  loops = atoi (argv[1]);
  mem_size = atoi (argv[2]);

  if (argc > 3 && strcmp (argv[3], "-v") == 0) {
    verify = 1;
  }

  if (status != QMP_SUCCESS) {
    QMP_error ("QMP_init failed: %s\n", QMP_error_string(status));
    QMP_abort(1);
  }

  {
    int k=ndims-1;
    int nodes = QMP_get_number_of_nodes();
    while( (nodes&1) == 0 ) {
      dims[k] *= 2;
      nodes /= 2;
      k = (k+ndims-1)%ndims;
    }
    if(nodes != 1) {
      QMP_error("invalid number of nodes %i", QMP_get_number_of_nodes());
      QMP_error(" must power of 2");
      QMP_abort(1);
    }
  }

  if (QMP_broadcast (dims, ndims*sizeof(int)) != QMP_SUCCESS) 
    QMP_abort_string (1, "Cannot do broadcast, Quit\n");

  status = QMP_declare_logical_topology (dims, ndims);
  if (status != QMP_SUCCESS)
    QMP_printf ("Cannot declare logical grid\n");
  
  init_wnxtsu3dslash();

  i = 0;

  it = get_current_time ();
  while (i < loops) {
    
    if (verify) {
      for (j = 0; j < num; j++) {
	memset (QMP_get_memory_pointer(forw_mem[j][1]), 0, mem_size);
	memset (QMP_get_memory_pointer(forw_mem[j][0]), (char)i, mem_size);

	memset (QMP_get_memory_pointer(back_mem[j][1]), 0, mem_size);
	memset (QMP_get_memory_pointer(back_mem[j][0]), (char)i, mem_size);
      }
    }

    //fflush(stdout); QMP_barrier();

    if ((err = QMP_start (forw_all_mh))!= QMP_SUCCESS)
      QMP_printf ("Start forward operations failed: %s\n", 
		  QMP_error_string(err));

    //fflush(stdout); QMP_barrier();

    if (QMP_wait (forw_all_mh) != QMP_SUCCESS)
      QMP_printf ("Error in sending %d\n", i);


    if ((err = QMP_start (back_all_mh)) != QMP_SUCCESS)
      QMP_printf ("Start backward failed: %s\n", QMP_error_string(err));


    if (QMP_wait (back_all_mh) != QMP_SUCCESS)
      QMP_printf ("Error in wait receiving %d\n", i);


    if (verify) {
      char* fmem;
      char* bmem;
      for (j = 0; j < num; j++) {
	fmem = (char *)QMP_get_memory_pointer(forw_mem[j][1]);
	bmem = (char *)QMP_get_memory_pointer(back_mem[j][1]);
	for (k = 0; k < mem_size; k++) {
	  if (fmem[k] != (char)i)
	    QMP_fprintf (stderr, "Forwared Receiving error\n");
	}

	for (k = 0; k < mem_size; k++) {
	  if (bmem[k] != (char)i)
	    QMP_fprintf (stderr, "Backward Receiving error\n");
	}
      }
    }

    i++;
  }
  ft = get_current_time ();

  dt = (ft - it); /* actual send time milli seconds */

  /* bandwidth in MB/second */
  bwval = 2*num*(double)mem_size/dt/(double)1000.0*loops;
  

  QMP_fprintf (stderr, "Sending using %d channels with memory size %d yields %lf (mriro seconds) latency and bandwidth %lf (MB/s)\n", 2*num, mem_size, dt/(2*num*loops)*1000.0, bwval);

  QMP_free_msghandle (forw_all_mh);
  QMP_free_msghandle (back_all_mh);
  for (i = 0; i < num; i++) {
    QMP_free_msgmem (forw_msg[i][0]);
    QMP_free_msgmem (forw_msg[i][1]);

    QMP_free_msgmem (back_msg[i][0]);
    QMP_free_msgmem (back_msg[i][1]);
  }

  /**
   * Free memory
   */
  for (i = 0; i < num; i++) {
    QMP_free_memory (forw_mem[i][0]);
    QMP_free_memory (forw_mem[i][1]);
    QMP_free_memory (back_mem[i][0]);
    QMP_free_memory (back_mem[i][1]);
  }

  QMP_finalize_msg_passing ();

  return 0;
}

