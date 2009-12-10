/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP error reporting code for single node build
 *
 * Author:
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: QMP_error_single.c,v $
 *   Revision 1.5  2006/05/25 21:08:52  edwards
 *   Changed error strings to match function name.
 *
 *   Revision 1.4  2006/03/10 18:15:57  osborn
 *   Fixed warning about types.
 *
 *   Revision 1.3  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.2  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_SINGLE.h"

/**
 * Error strings corresponding to the error codes.
 */
static char* QMP_error_strings[] = {
  "successful",
  "general QMP error",
  "message passing system is not initialized yet",
  "run time environment set up error",
  "machine CPU information retrieval error",
  "node run time information retrieval error",
  "malloc failure",
  "sending memory size is larger than receiving memory size",
  "node hostname retrieval error",
  "initiate underlying service (gm) error",
  "logical topology exists already",
  "channel connection time out",
  "not supported",
  "underlying service (gm) is busy",
  "invalid communication message",
  "invalid function arguments",
  "invalid declared topology",
  "topology neighboring information error",
  "declare memory size exceeeds the maximum allowed size",
  "invalid memory for a message handle",
  "no more gm ports available",
  "remote node number is out of range",
  "channel definition error (send/recv to/from itself)",
  "receiving memory is already used by others",
  "a sending/receiving operation applied in a wrong state",
  "communication time out",
};

/**
 * Return an error string from a error code.
 */
const char*
QMP_error_string (QMP_status_t code)
{
  static char errstr[256];
  const char *retval;
  ENTER;

  if (code == QMP_SUCCESS) {
    retval = QMP_error_strings[code];
  }
  else if (code < QMP_MAX_STATUS && code >= QMP_ERROR) {
    retval = QMP_error_strings[code - QMP_ERROR + 1];
  }
  else {
    snprintf (errstr, sizeof (errstr), "unknown error code %d", code);
    retval = (const char *)&errstr;
  }
  LEAVE;
  return retval;
}

/**
 * Return error number of last function call if there is an error.
 * If a function returns a status code, this function will not produce
 * any useful information. 
 *
 * This function should be used when no status
 * code is returned from a function call.
 */
QMP_status_t
QMP_get_error_number (QMP_msghandle_t mh)
{
  ENTER;
  QMP_error("QMP_get_error_number: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  LEAVE;
  return QMP_ERROR;
}

/**
 * Return error string of last function call if there is an error.
 * If a function returns a status code, this function will not produce
 * any useful information. 
 *
 * This function should be used when no status
 * code is returned from a function call.
 */
const char*
QMP_get_error_string (QMP_msghandle_t mh)
{
  ENTER;
  QMP_error("QMP_get_error_string: passed QMP_msghandle_t not allocated\n");
  QMP_abort(1);
  LEAVE;
  return (const char *)0;
}
