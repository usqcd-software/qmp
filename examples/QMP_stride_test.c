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
 *      Simple Strided Memory Test Program for QMP
 *
 * Author:  
 *      James Osborn
 *
 * Modified from QMP_test.c
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
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
  int i, ns, nr;
  int rank, nodes, src, dest, verbose;
  QMP_status_t status;
  QMP_thread_level_t req, prv;

  QMP_mem_t *rmem, *smem;
  QMP_msgmem_t recvmem, sendmem;
  QMP_msghandle_t recvh, sendh;
  int *rvec, *svec;

  ns = 10;
  nr = ns/2;

  verbose = 0;
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = 1;
  QMP_verbose(verbose);

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);
  if (status != QMP_SUCCESS) {
    QMP_fprintf(stderr, "QMP_init failed\n");
    return -1;
  }

  rank = QMP_get_node_number();
  nodes = QMP_get_number_of_nodes();
  dest = (rank+1)%nodes;
  src = (rank+nodes-1)%nodes;

  smem = QMP_allocate_memory(ns*sizeof(int));
  if (!smem) {
    QMP_fprintf (stderr, "cannot allocate sending memory\n");
    exit (1);
  }

  svec = (int *) QMP_get_memory_pointer(smem);
  for(i=0; i<ns; i++) {
    svec[i] = i;
  }

  sendmem = QMP_declare_strided_msgmem( (void *)svec,
					sizeof(int),
					nr,
					2*sizeof(int) );
  if (!sendmem) {
    QMP_fprintf (stderr, "send memory error : %s\n", 
		 QMP_get_error_string(0));
    exit (1);
  }

  sendh = NULL;
  if(nodes>1) {
    sendh = QMP_declare_send_to(sendmem, dest, 0);
    if(!sendh) {
      QMP_fprintf (stderr, "Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }


  rmem = QMP_allocate_memory(nr*sizeof(int));
  if (!rmem) {
    QMP_fprintf (stderr, "cannot allocate receiving memory\n");
    exit (1);
  }

  rvec = (int *) QMP_get_memory_pointer(rmem);

  recvmem = QMP_declare_msgmem((void *)rvec, nr*sizeof(int));
  if(!recvmem) {
    QMP_fprintf (stderr, "recv memory error : %s\n", 
		 QMP_get_error_string(0));
    exit (1);
  }

  recvh = NULL;
  if(nodes>1) {
    recvh = QMP_declare_receive_from (recvmem, src, 0);
    if (!recvh) {
      QMP_fprintf (stderr, "Recv Handle Error: %s\n", 
		   QMP_get_error_string(0));      
      exit (1);
    }
  }


  if(nodes>1) {
    if ((status = QMP_start (recvh)) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Start receiving failed: %s\n", 
		   QMP_error_string(status));
    if ((status = QMP_start (sendh))!= QMP_SUCCESS)
      QMP_fprintf (stderr, "Start sending failed: %s\n",
		   QMP_error_string(status));

    if (QMP_wait (recvh) != QMP_SUCCESS)
      QMP_fprintf (stderr,"Error in receiving %d\n", i);
    else
      QMP_fprintf (stderr, "Received ok %d\n", i);

    if (QMP_wait (sendh) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Error in sending %d\n", i);
    else
      QMP_fprintf (stderr, "Sending success %d\n", i);
  }

  QMP_free_msghandle (recvh);
  QMP_free_msghandle (sendh);

  QMP_free_msgmem (recvmem);
  QMP_free_msgmem (sendmem);


  for(i=0; i<ns; i++) {
    QMP_printf("%i", svec[i]);
  }

  for(i=0; i<nr; i++) {
    QMP_printf("%i", rvec[i]);
  }


  QMP_free_memory (rmem);
  QMP_free_memory (smem);

  QMP_finalize_msg_passing ();

  return 0;
}
