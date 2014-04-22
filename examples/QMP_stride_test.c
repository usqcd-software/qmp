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
 *   $Log: QMP_stride_test.c,v $
 *   Revision 1.1  2004/12/16 02:45:51  osborn
 *   Really add QMP_stride_test.c.
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <qmp.h>

#define NUM_HANDLES 4

void send_recv(QMP_msghandle_t sendh, QMP_msghandle_t recvh, int i){
  int nodes = QMP_get_number_of_nodes();
  QMP_status_t status;
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
      QMP_fprintf (stdout, "Received ok %d\n", i);

    if (QMP_wait (sendh) != QMP_SUCCESS)
      QMP_fprintf (stderr, "Error in sending %d\n", i);
    else
      QMP_fprintf (stdout, "Sending success %d\n", i);
  }
}

int main (int argc, char** argv)
{
  int i, ns, nr, iter;
  int rank, nodes, src, dest, verbose;
  QMP_status_t status;
  QMP_thread_level_t req, prv;

  QMP_mem_t *rmem, *smem;
  QMP_msgmem_t recvmem, sendmem;
  QMP_msgmem_t recvmem2, sendmem2;
  QMP_msghandle_t recvh, sendh;
  QMP_msghandle_t recvh2, sendh2;
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

  smem = QMP_allocate_memory(ns*2*sizeof(int));
  if (!smem) {
    QMP_fprintf (stderr, "cannot allocate sending memory\n");
    exit (1);
  }

  svec = (int *) QMP_get_memory_pointer(smem);
  for(i=0; i<ns*2; i++) {
    svec[i] = i;
  }

  void *base[2];
  size_t blksize[2];
  int nblocks[2];
  ptrdiff_t stride[2];

  sendmem = QMP_declare_strided_msgmem( (void *)svec,
					sizeof(int),
  			nr,
  			2*sizeof(int) );
  if (!sendmem) {
    QMP_fprintf (stderr, "send memory error : %s\n", 
		 QMP_get_error_string(0));
    exit (1);
  }

  for(i=0;i<2;i++){
	base[i]=(void*)(svec+i);
    blksize[i]=sizeof(int);
    nblocks[i]=nr;
    stride[i]=2*sizeof(int);
  }
  sendmem2 = QMP_declare_strided_array_msgmem( base,blksize,nblocks,stride,2);
  if (!sendmem2) {
    QMP_fprintf (stderr, "send memory error : %s\n", 
		 QMP_get_error_string(0));
    exit (1);
  }
  
  sendh = NULL;
  sendh2 = NULL;
  if(nodes>1) {
    sendh = QMP_declare_send_to(sendmem, dest, 0);
    if(!sendh) {
      QMP_fprintf (stderr, "Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
    sendh2 = QMP_declare_send_to(sendmem2, dest, 0);
    if(!sendh2) {
      QMP_fprintf (stderr, "Send Handle Error: %s\n", QMP_get_error_string(0));
      exit (1);
    }
  }


  rmem = QMP_allocate_memory(2*ns*sizeof(int));
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

  for(i=0;i<2;i++){
	base[i]=(void*)(rvec+i*nr);
    blksize[i]=sizeof(int);
    nblocks[i]=nr;
    stride[i]=sizeof(int);
  }
  recvmem2 = QMP_declare_strided_array_msgmem( base,blksize,nblocks,stride,2);
  if (!recvmem2) {
    QMP_fprintf (stderr, "recv memory error : %s\n", 
		 QMP_get_error_string(0));
    exit (1);
  }

  for(i=0; i<ns; i++) {
    rvec[i] = 0;
  }

  recvh = NULL;
  recvh2 = NULL;
  if(nodes>1) {
    recvh = QMP_declare_receive_from (recvmem, src, 0);
    if (!recvh) {
      QMP_fprintf (stderr, "Recv Handle Error: %s\n", 
		   QMP_get_error_string(0));      
      exit (1);
    }
    recvh2 = QMP_declare_receive_from (recvmem2, src, 0);
    if (!recvh2) {
      QMP_fprintf (stderr, "Recv Handle Error: %s\n", 
		   QMP_get_error_string(0));      
      exit (1);
    }
  }
  
for (iter=0;iter<1000;iter++){
  QMP_change_address(recvh,(void *)(rvec));
  QMP_change_address(sendh,(void *)(svec));

  send_recv (sendh,recvh,iter*4);

  QMP_change_address(recvh,(void *)(rvec+nr));
  QMP_change_address(sendh,(void *)(svec+1));

  send_recv (sendh,recvh,iter*4+1);

  for(i=0; i<ns; i++) {
    QMP_printf("%d: sent=%d rec=%d", i, svec[i],rvec[i]);
    rvec[i]=0;
  }

  QMP_change_address(sendh2,svec);
  QMP_change_address(recvh2,rvec);

  send_recv (sendh2,recvh2,iter*4+2);

  for(i=0; i<ns; i++) {
    QMP_printf("%d: sent=%d rec=%d", i, svec[i],rvec[i]);
    rvec[i]=0;
  }

  QMP_change_address(sendh2,svec+ns);
  QMP_change_address(recvh2,rvec+ns);

  send_recv (sendh2,recvh2,iter*4+3);

  for(i=0; i<ns; i++) {
    QMP_printf("%d: sent=%d rec=%d", i, svec[i+ns],rvec[i+ns]);
    rvec[i+ns]=0;
  }
}

  QMP_free_msghandle (recvh);
  QMP_free_msghandle (sendh);
  QMP_free_msghandle (recvh2);
  QMP_free_msghandle (sendh2);

  QMP_free_msgmem (recvmem);
  QMP_free_msgmem (sendmem);
  QMP_free_msgmem (recvmem2);
  QMP_free_msgmem (sendmem2);



  QMP_free_memory (rmem);
  QMP_free_memory (smem);

  QMP_finalize_msg_passing ();

  return 0;
}
