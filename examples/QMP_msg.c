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
 *      Simple Test Program for QMP general message
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.4  2002/12/05 16:41:03  chen
 *   Add new global communication BufferFly algorithm
 *
 *   Revision 1.3  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:42  chen
 *   Version 0.95 Release
 *
 *   Revision 1.2  2002/03/27 20:48:49  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.1  2002/02/15 20:34:53  chen
 *   First Beta Release QMP
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <QMP.h>

/* test purpose only */
#define QMP_QUEUE              1100
#define QMP_REDUCE             1150
#define QMP_ANY                1200

#define NUM_HANDLES 4

int main (int argc, char** argv)
{
  int i;
  QMP_bool_t status;
  QMP_u32_t num_nodes;

  void *rmem[NUM_HANDLES], *smem[NUM_HANDLES];
  QMP_msgmem_t recvmem[NUM_HANDLES];
  QMP_msghandle_t recvh[NUM_HANDLES];
  QMP_msgmem_t sendmem[NUM_HANDLES];
  QMP_msghandle_t sendh[NUM_HANDLES];

  /*
  QMP_u32_t dims[4] = {2, 2, 4, 2};
  QMP_u32_t ndims = 4;
  */
  QMP_u32_t dims[1] = {2};
  QMP_u32_t ndims = 1;

  status = QMP_init_msg_passing (argc, argv, QMP_SMP_MULTIPLE_ADDRESS);

  if (status != QMP_SUCCESS) {
    QMP_printf ( "QMP_init failed\n");
    return -1;
  }

  /*
  status = QMP_declare_logical_topology (dims, ndims);

  if (status == QMP_FALSE)
    QMP_printf ( "Cannot declare logical grid\n");
  else
    QMP_printf ("Declare logical grid ok\n");

  for (i = 0; i < NUM_HANDLES; i++) {
    rmem[i] = QMP_allocate_aligned_memory (1024);
    if (!rmem[i]) {
      QMP_printf ("cannot allocate receiving memory\n");
      exit (1);
    }
    recvmem[i] = QMP_declare_msgmem (rmem[i], 1024);
    if (!recvmem[i]) {
      QMP_printf ("recv memory error : %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    smem[i] = QMP_allocate_aligned_memory (1024);
    if (!rmem[i]) {
      QMP_printf ("cannot allocate sending memory\n");
      exit (1);
    }
    sendmem [i]= QMP_declare_msgmem (smem[i], 1024);
    if (!sendmem[i]) {
      QMP_printf ("send memory error : %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }

  
  for (i = 0; i < NUM_HANDLES; i++) {
    recvh[i] = QMP_declare_receive_relative (recvmem[i], 0, 1, 0);
    if (!recvh[i]) {
      QMP_printf ("Recv Handle Error: %s\n", QMP_get_error_string(0));      
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    sendh[i] = QMP_declare_send_relative (sendmem[i], 0, -1, 0);
    if (!sendh[i]) {
      QMP_printf ("Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }
  */

  num_nodes = QMP_get_number_of_nodes ();
  QMP_printf ("There are %d nodes for this job\n", num_nodes);

  /* do a broacast test */

  {
    double bcast_value;

    if (QMP_get_node_number () == 0) 
      bcast_value = 1243.3232;
    else
      bcast_value = 2321333.0;
    
    QMP_broadcast (&bcast_value, 8);
    
    QMP_printf ("bcast value is %lf\n", bcast_value);
  }

  /* global sum test.
   */
  {
    int value;

    value = 4;
    QMP_sum_int (&value);
    
    QMP_printf ("sum is %d\n", value);
  }

  /**
   * Global array sum
   */
  {
    double dvalue[10];
    QMP_u32_t length = 10;
    QMP_u32_t rank;
    int    i;

    rank = QMP_get_node_number ();
    for (i = 0; i < 10; i++) {
      dvalue[i] = rank * 10.0 + i;
    }

    if (QMP_sum_double_array (dvalue, length) == QMP_SUCCESS) {
      for (i = 0; i < length; i++) 
	QMP_printf ("dvalue[%d] is %lf\n", i, dvalue[i]);
    }
    else
      QMP_printf ("Error in global array sum\n");
  }

  /**
   * Global float array sum
   */
  {
    float fvalue[10];
    QMP_u32_t length = 10;
    QMP_u32_t rank;
    int    i;

    rank = QMP_get_node_number ();
    for (i = 0; i < length; i++) {
      fvalue[i] = rank * 10.0 + i;
    }

    if (QMP_sum_float_array (fvalue, length) == QMP_SUCCESS) {
      for (i = 0; i < length; i++) 
	QMP_printf ("fvalue[%d] is %lf\n", i, fvalue[i]);
    }
    else
      QMP_printf ("Error in global array sum\n");
  }

  /**
   * Sync barrier
   */
  QMP_printf ("Wait for a barrier\n");
  QMP_wait_for_barrier (1000.0);
  QMP_printf ("Done Wait for a barrier\n");

  /**
   * Global XOR Test
   */
  {
    long mask;
    QMP_u32_t rank;

    rank = QMP_get_node_number ();
#ifdef QMP_64BIT_LONG
    if (rank % 2 == 0)
      mask = 0xffff0000ffff0000;
    else
      mask = 0x0000ffff0000ffff;
#else
    mask = 1 << rank;
#endif

    QMP_global_xor (&mask);

    QMP_printf ("final mask is 0x%lx\n", mask);
  }

  

  /*
  for (i = 0; i < NUM_HANDLES; i++) {
    QMP_free_msghandle (recvh[i]);
    QMP_free_msgmem (recvmem[i]);

    QMP_free_msghandle (sendh[i]);
    QMP_free_msgmem (sendmem[i]);

    QMP_free_aligned_memory (rmem[i]);
    QMP_free_aligned_memory (smem[i]);
  }
  */

  QMP_finalize_msg_passing ();

  return 0;
}


