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
 *   $Log: QMP_msg.c,v $
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

#include <qmp.h>

static void 
my_binary_func (void* inout, void* in)
{
  float *res, *source;
  int   len, i;
  /* assume this is float array with size 10 */
  
  res = (float *)inout;
  source = (float *)in;
  len = 10;

  for (i = 0; i < 10; i++) 
    res[i] = res[i] + 2 * source[i];

}

int main (int argc, char** argv)
{
  int i;
  QMP_status_t status;
  int num_nodes;
  QMP_thread_level_t req, prv;

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);

  if (status != QMP_SUCCESS) {
    QMP_printf ( "QMP_init failed\n");
    return -1;
  }

  num_nodes = QMP_get_number_of_nodes ();
  QMP_printf ("There are %d nodes for this job\n", num_nodes);

  /* do a broacast test */

  {
    double bcast_value;

    if (QMP_get_node_number () == 0) 
      bcast_value = 1243.3232;
    else
      bcast_value = 2321333.0;
    
    QMP_broadcast (&bcast_value, sizeof(double));
    
    QMP_printf ("bcast value is %f\n", bcast_value);
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
    int length = 10;
    int rank;
    int    i;

    rank = QMP_get_node_number ();
    for (i = 0; i < 10; i++) {
      dvalue[i] = rank * 10.0 + i;
    }

    if (QMP_sum_double_array (dvalue, length) == QMP_SUCCESS) {
      for (i = 0; i < length; i++) 
	QMP_printf ("dvalue[%d] is %f\n", i, dvalue[i]);
    }
    else
      QMP_printf ("Error in global array sum\n");
  }

  /**
   * Global float array sum
   */
  {
    float fvalue[10];
    int length = 10;
    int rank;
    int    i;

    rank = QMP_get_node_number ();
    for (i = 0; i < length; i++) {
      fvalue[i] = rank * 10.0 + i;
    }

    if (QMP_sum_float_array (fvalue, length) == QMP_SUCCESS) {
      for (i = 0; i < length; i++) 
	QMP_printf ("fvalue[%d] is %f\n", i, fvalue[i]);
    }
    else
      QMP_printf ("Error in global array sum\n");
  }

  /**
   * Sync barrier
   */
  QMP_printf("Barrier\n");
  QMP_barrier();
  QMP_printf("Done Barrier\n");

  /**
   * Global XOR Test
   */
  {
    unsigned long mask;
    int rank;

    rank = QMP_get_node_number ();
    mask = 1 << rank;

    QMP_xor_ulong (&mask);

    QMP_printf ("final mask is 0x%lx\n", mask);
  }


  /* global binary reduction test */
  {
    float value[10];
    int rank;

    rank = QMP_get_node_number ();
    for (i = 0; i < 10; i++)
      value[i] = (float)i - (float)rank;

    if (QMP_binary_reduction (value, 10*sizeof(float), 
			      my_binary_func) != QMP_SUCCESS)
      QMP_error ("Binary reduction error.\n");

    QMP_fprintf (stderr, "Reduced float array looks like : ");
    for (i = 0; i < 10; i++) 
      fprintf (stderr, "%10.5f ", value[i]);
    fprintf (stderr, "\n");

  }


  QMP_finalize_msg_passing ();

  return 0;
}


