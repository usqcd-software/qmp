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
 *   Revision 1.4  2003/12/23 16:54:13  edwards
 *   Added trace macro to this top level code.
 *
 *   Revision 1.3  2003/12/19 04:46:39  edwards
 *   Added return of status.
 *
 *   Revision 1.2  2003/12/19 04:45:31  edwards
 *   Small bug fix.
 *
 *   Revision 1.1  2003/12/19 04:43:00  edwards
 *   First version of generic routines.
 *
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qmp.h"

/**
 * Trace macro
 */
#ifdef _QMP_TRACE
#define QMP_DEBUG 1
#else
#undef QMP_DEBUG
#endif

/**
 * Public function for QMP_routing
 */
QMP_status_t QMP_route (void* buffer, QMP_u32_t count,
			QMP_u32_t src, QMP_u32_t dest)
{
  QMP_status_t status = QMP_SUCCESS;
  QMP_u32_t node;

#if QMP_DEBUG >= 1
  QMP_info("QMP_route");
#endif

  node = QMP_get_node_number();

  if (node == dest)
  {
    QMP_msgmem_t request_msg = QMP_declare_msgmem(buffer, count);
    QMP_msghandle_t request_mh = QMP_declare_receive_from(request_msg, src, 0);

#if QMP_DEBUG >= 1
    QMP_info("starting a recvFromWait, count=%d, srcenode=%d", count, src);
#endif

    if ((status=QMP_start(request_mh)) != QMP_SUCCESS)
      return status;

    QMP_wait(request_mh);

    QMP_free_msghandle(request_mh);
    QMP_free_msgmem(request_msg);

#if QMP_DEBUG >= 1
    QMP_info("finished a recvFromWait");
#endif
  }
    
  if (node == src)
  {
    QMP_msgmem_t request_msg = QMP_declare_msgmem(buffer, count);
    QMP_msghandle_t request_mh = QMP_declare_send_to(request_msg, dest, 0);

#if QMP_DEBUG >= 1
    QMP_info("starting a sendToWait, count=%d, destnode=%d", count,dest);
#endif

    if ((status=QMP_start(request_mh)) != QMP_SUCCESS)
      return status;

    QMP_wait(request_mh);

    QMP_free_msghandle(request_mh);
    QMP_free_msgmem(request_msg);

#if QMP_DEBUG >= 1
    QMP_info("finished a sendToWait");
#endif
  }

  return status;
}


