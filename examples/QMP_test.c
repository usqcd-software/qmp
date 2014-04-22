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
 *      Simple Test Program for QMP
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_test.c,v $
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
 *   Revision 1.4  2002/11/15 15:37:34  chen
 *   Fix bugs caused by rapid creating/deleting channels
 *
 *   Revision 1.3  2002/04/30 18:26:36  chen
 *   Allow QMP_gm to send/recv from itself
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:44  chen
 *   Version 0.95 Release
 *
 *   Revision 1.4  2002/03/27 20:48:50  chen
 *   Conform to spec 0.9.8
 *
 *   Revision 1.3  2002/02/15 20:34:54  chen
 *   First Beta Release QMP
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <qmp.h>

#define NUM_HANDLES 4

int main (int argc, char** argv)
{
  int i, j;
  QMP_bool_t status, verbose;
  QMP_status_t err;
  int dims[4] = {1, 1, 1, 1};
  int ndims = 4;
  //int dims[2] = {2,2};
  //int ndims = 2;

  QMP_mem_t *rmem[NUM_HANDLES], *smem[NUM_HANDLES];
  QMP_msgmem_t recvmem[NUM_HANDLES];
  QMP_msghandle_t recvh[NUM_HANDLES];
  QMP_msgmem_t sendmem[NUM_HANDLES];
  QMP_msghandle_t sendh[NUM_HANDLES];
  QMP_thread_level_t req, prv;

  /**
   * Multiple message handles are combined
   */
  QMP_msghandle_t comp_sendh, comp_recvh;

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);
  if (status != QMP_SUCCESS) {
    QMP_fprintf(stderr, "QMP_init failed\n");
    return -1;
  }

  verbose = QMP_FALSE;  
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = QMP_TRUE;
  QMP_verbose (verbose);

  QMP_printf("QMP version str %s", QMP_version_str());
  QMP_printf("QMP version int %i", QMP_version_int());

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

  status = QMP_declare_logical_topology (dims, ndims);

  if (status != QMP_SUCCESS)
    QMP_fprintf (stderr, "Cannot declare logical grid\n");
  else
    QMP_fprintf (stderr, "Declare logical grid ok\n");

  for (i = 0; i < NUM_HANDLES; i++) {
    rmem[i] = QMP_allocate_memory (10234);
    if (!rmem[i]) {
      QMP_fprintf (stderr, "cannot allocate receiving memory\n");
      exit (1);
    }
    recvmem[i] = QMP_declare_msgmem (QMP_get_memory_pointer(rmem[i]), 10234);
    if (!recvmem[i]) {
      QMP_fprintf (stderr, "recv memory error : %s\n", 
		   QMP_get_error_string(0));
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    smem[i] = QMP_allocate_memory (10234);
    if (!smem[i]) {
      QMP_fprintf (stderr, "cannot allocate sending memory\n");
      exit (1);
    }

    sendmem[i] = QMP_declare_msgmem (QMP_get_memory_pointer(smem[i]), 10234);
    if (!sendmem[i]) {
      QMP_fprintf (stderr, "send memory error : %s\n", 
		   QMP_get_error_string(0));
      exit (1);
    }
  }


  int sdir = ndims-1;
  for (i = 0; i < NUM_HANDLES; i++) {
    sendh[i] = QMP_declare_send_relative (sendmem[i], sdir, -1, 0);
    if (!sendh[i]) {
      QMP_fprintf (stderr, "Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }

  for (i = 0; i < NUM_HANDLES; i++) {
    recvh[i] = QMP_declare_receive_relative (recvmem[i], sdir, 1, 0);
    if (!recvh[i]) {
      QMP_fprintf (stderr, "Recv Handle Error: %s\n", 
		   QMP_get_error_string(0));      
      exit (1);
    }
  }

  comp_sendh = QMP_declare_multiple (sendh, NUM_HANDLES);
  comp_recvh = QMP_declare_multiple (recvh, NUM_HANDLES);

  i = 0;
  while (i < 10) {

    if ((err = QMP_start (comp_sendh))!= QMP_SUCCESS)
      QMP_fprintf (stderr, "Start sending failed: %s\n",
		   QMP_error_string(err));
    
    if ((err = QMP_start (comp_recvh)) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Start receiving failed: %s\n", 
		  QMP_error_string(err));
    
    if (QMP_wait (comp_sendh) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Error in sending %d\n", i);
    else
      QMP_fprintf (stderr, "Sending success %d\n", i);


    if (QMP_wait (comp_recvh) != QMP_SUCCESS)
      QMP_fprintf (stderr,"Error in receiving %d\n", i);
    else
      QMP_fprintf (stderr, "Received ok %d\n", i);

    sleep (1);
    i++;
  }


  QMP_free_msghandle (comp_recvh);
  QMP_free_msghandle (comp_sendh);

  for (j = 0; j < NUM_HANDLES; j++) {
    QMP_free_msgmem (recvmem[j]);
    QMP_free_msgmem (sendmem[j]);
    
    QMP_free_memory (rmem[j]);
    QMP_free_memory (smem[j]);
  }

  QMP_finalize_msg_passing ();

  return 0;
}
