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
 *      Generic utility routines for QMP
 *
 * Author:  
 *      Robert Edwards
 *      Jefferson Lab
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qmp.h"

/**
 * Public function for QMP_routing
 */
QMP_status_t QMP_route (void* buffer, QMP_u32_t count,
			QMP_u32_t src, QMP_u32_t dest)
{
  QMP_status_t status = QMP_SUCCESS;
  QMP_u32_t node;

  QMP_TRACE ("QMP_route");

  node = QMP_get_node_number();

  if (node == dest)
  {
    QMP_msgmem_t request_msg = QMP_declare_msgmem(buffer, count);
    QMP_msghandle_t request_mh = QMP_declare_receive_from(request_msg, src, 0);

#if QMP_DEBUG >= 2
    QMP_info("starting a recvFromWait, count=%d, srcenode=%d", count, src);
#endif

    if ((status=QMP_start(request_mh)) != QMP_SUCCESS)
      return status;

    QMP_wait(request_mh);

    QMP_free_msghandle(request_mh);
    QMP_free_msgmem(request_msg);

#if QMP_DEBUG >= 2
    QMP_info("finished a recvFromWait");
#endif
  }
    
  if (node == dest)
  {
    QMP_msgmem_t request_msg = QMP_declare_msgmem(buffer, count);
    QMP_msghandle_t request_mh = QMP_declare_send_to(request_msg, dest, 0);

#if QMP_DEBUG >= 2
    QMP_info("starting a sendToWait, count=%d, destnode=%d", count,dest);
#endif

    if ((status=QMP_start(request_mh)) != QMP_SUCCESS)
      return status;

    QMP_wait(request_mh);

    QMP_free_msghandle(request_mh);
    QMP_free_msgmem(request_msg);

#if QMP_DEBUG >= 2
    QMP_info("finished a sendToWait");
#endif
  }

}


