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
 *      Simple Test to send/recv data from the same node.
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
 *   Revision 1.1  2002/04/30 19:10:49  chen
 *   Add loopback test
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <QMP.h>

int main (int argc, char** argv)
{
  int i;
  QMP_bool_t status, verbose;
  QMP_u32_t  rank;
  QMP_status_t err;
  QMP_u32_t dims[1] = {2};
  QMP_u32_t ndims = 1;


  void        *rmem, *smem;
  QMP_msgmem_t recvmem;
  QMP_msghandle_t recvh;
  QMP_msgmem_t sendmem;
  QMP_msghandle_t sendh;

  verbose = QMP_FALSE;  
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = QMP_TRUE;
  
  QMP_verbose (verbose);
  status = QMP_init_msg_passing (&argc, &argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) {
    QMP_printf("QMP_init failed\n");
    return -1;
  }

  status = QMP_declare_logical_topology (dims, ndims);

  if (status == QMP_FALSE)
    QMP_printf ("Cannot declare logical grid\n");
  else
    QMP_printf ("Declare logical grid ok\n");

  rank = QMP_get_node_number ();

  /* allocate memory */
  rmem = QMP_allocate_aligned_memory (10234);
  if (!rmem) {
    QMP_printf ("cannot allocate receiving memory\n");
    exit (1);
  }
  recvmem = QMP_declare_msgmem (rmem, 10234);
  if (!recvmem) {
    QMP_printf ("recv memory error : %s\n", QMP_get_error_string(0));
    exit (1);
  }


  smem = QMP_allocate_aligned_memory (10234);
  if (!smem) {
    QMP_printf ("cannot allocate sending memory\n");
    exit (1);
  }

  sendmem= QMP_declare_msgmem (smem, 10234);
  if (!sendmem) {
    QMP_printf ("send memory error : %s\n", QMP_get_error_string(0));
    exit (1);
  }

  recvh = QMP_declare_receive_from (recvmem, rank, 0);
  if (!recvh) {
    QMP_printf ("Recv from Handle Error: %s\n", QMP_get_error_string(0));      
    exit (1);
  }

  /* create message handle */
  sendh = QMP_declare_send_to (sendmem, rank, 0);
  if (!sendh) {
    QMP_printf ("Send to Handle Error: %s\n", QMP_get_error_string(0));
    exit (1);
  }


  i = 0;
  while (i < 10) {

    if ((err = QMP_start (sendh))!= QMP_SUCCESS)
      QMP_printf ("Start sending failed: %s\n", QMP_error_string(err));
    
    if ((err = QMP_start (recvh)) != QMP_SUCCESS)
      QMP_printf ("Start receiving failed: %s\n", QMP_error_string(err));
    
    if (QMP_wait (sendh) != QMP_SUCCESS)
      QMP_printf ("Error in sending %d\n", i);
    else
      QMP_printf ("Sending success %d\n", i);

    
    if (QMP_wait (recvh) != QMP_SUCCESS)
      QMP_printf ("Error in receiving %d\n", i);
    else
      QMP_printf ("Received ok %d\n", i);

    sleep (1);
    i++;
  }

  /* free memory */
  QMP_free_msghandle (recvh);
  QMP_free_msghandle (sendh);

  QMP_free_msgmem (recvmem);

  QMP_free_msgmem (sendmem);

  QMP_free_aligned_memory (rmem);
  QMP_free_aligned_memory (smem);

  QMP_finalize_msg_passing ();

  return 0;
}


  
