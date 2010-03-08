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
 *   $Log: QMP_broadcast.c,v $
 *   Revision 1.4  2004/12/16 02:44:11  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
 *
 *   Revision 1.3  2004/06/14 20:36:30  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.2  2003/06/14 03:31:10  edwards
 *   Added another example of a broadcast. This also shows the MPICH memory problem.
 *
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
stupid_broadcast(void *send_buf, int count)
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
	QMP_abort_string(1, "recvFromWait failed\n");

      QMP_wait(request_mh);
      QMP_free_msghandle(request_mh);
    }

    if (QMP_is_primary_node())
    {
      request_mh = QMP_declare_send_to(request_msg, node, 0);

      if (QMP_start(request_mh) != QMP_SUCCESS)
	QMP_abort_string(1, "sendToWait failed\n");

      QMP_wait(request_mh);
      QMP_free_msghandle(request_mh);
    }
  }

  QMP_free_msgmem(request_msg);
}


int main (int argc, char** argv)
{
  int verbose;
  QMP_status_t status;
  QMP_thread_level_t req, prv;

  verbose = 0;
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = 1;
  
  QMP_verbose (verbose);
  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);

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

#if 1
    for(i=0; i < 10; ++i)
      QMP_broadcast(p, 288);
#else
    for(i=0; i < 10; ++i)
      stupid_broadcast(p, 288);
#endif

    QMP_info("result = %c",p[0]);
  }


  QMP_finalize_msg_passing ();

  return 0;
}


