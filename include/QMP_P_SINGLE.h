/*----------------------------------------------------------------------------
 *
 * Description:
 *      Private header file for QMP single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: QMP_P_SINGLE.h,v $
 *   Revision 1.6  2008/03/05 17:49:29  osborn
 *   added QMP_show_geom example and prepare for adding new command line options
 *
 *   Revision 1.5  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.4  2004/12/16 02:44:12  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
 *
 *   Revision 1.3  2004/10/31 23:21:35  osborn
 *   Restored proper behavior of msghandle operations in single node version.
 *   Added CFLAGS to qmp-config script.
 *   Changed QMP_status_code_t to QMP_status_t in qmp.h.
 *
 *   Revision 1.2  2004/10/18 18:17:22  edwards
 *   Added support for calling msghandle functions.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *
 */
#ifndef _QMP_P_SINGLE_H
#define _QMP_P_SINGLE_H

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define QMP_MSGMEM_ALLOCATED ((QMP_msgmem_t)12345)

/* Message Memory structure */
struct QMP_mem_struct_t {
  void *aligned_ptr;
  void *allocated_ptr;
};

#define MAX_HOST_LEN 256

#include "QMP_P_COMMON.h"

#endif /* _QMP_P_SINGLE_H */
