/*----------------------------------------------------------------------------
 *
 * Description:
 *      Private header file for QMP single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
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

#include "qmp.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define QMP_MSGMEM_ALLOCATED ((QMP_msgmem_t)12345)
#define QMP_MSGHANDLE_ALLOCATED ((QMP_msghandle_t)23456)

/* Message Memory structure */
/* here we use a C99 flexible array member so we can allocate the
   structure and memory together */
struct QMP_mem_struct_t {
  void *aligned_ptr;
  char mem[];
};

#endif /* _QMP_P_COMMON_H */
