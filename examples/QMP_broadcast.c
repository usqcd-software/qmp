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
 *      Robert Edwards
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1  2003/06/13 18:50:41  edwards
 *   Test program for broadcasts. Under MPICH, I see the primary node consume
 *   a huge amount of memory.
 *
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <qmp.h>

void
stupid_broadcast(void *send_buf, QMP_u32_t count)
{
  int node;
  int num_nodes = QMP_get_number_of_nodes();
  QMP_msgmem_t request_msg = QMP_declare_msgmem(send_buf, count);
  QMP_msghandle_t request_mh;

  // Send to each node
  for(node=1; node < num_nodes; ++node)
  {
    if (QMP_get_node_number() == node)
    {
      request_mh = QMP_declare_receive_from(request_msg, 0, 0);

      if (QMP_start(request_mh) != QMP_SUCCESS)
	QMP_error_exit("recvFromWait failed\n");

      QMP_wait(request_mh);
      QMP_free_msghandle(request_mh);
    }

    if (QMP_is_primary_node())
    {
      request_mh = QMP_declare_send_to(request_msg, node, 0);

      if (QMP_start(request_mh) != QMP_SUCCESS)
	QMP_error_exit("sendToWait failed\n");

      QMP_wait(request_mh);
      QMP_free_msghandle(request_mh);
    }
  }

  QMP_free_msgmem(request_msg);
}


int main (int argc, char** argv)
{
  QMP_bool_t status, verbose;
  QMP_status_t err;

  verbose = QMP_FALSE;  
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = QMP_TRUE;
  
  QMP_verbose (verbose);
  status = QMP_init_msg_passing (&argc, &argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) {
    QMP_fprintf(stderr, "QMP_init failed\n");
    return -1;
  }

  {
    char p[288];
    int i;

    for(i=0; i < 288;++i)
      p[i] = 0;

    if (QMP_is_primary_node())
      for(i=0; i < 288;++i)
	p[i] = 65;

#if 0
    for(i=0; i < 1000000;++i)
      QMP_broadcast(p, 288);
#else
    for(i=0; i < 1000000;++i)
      stupid_broadcast(p, 288);
#endif

    QMP_info("result = %c",p[0]);
  }


  QMP_finalize_msg_passing ();

  return 0;
}


