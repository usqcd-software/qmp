#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

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
  const char *retval = NULL;
  ENTER;

  if (code == QMP_SUCCESS) {
    retval = QMP_error_strings[code];
  }
  else if (code < QMP_MAX_STATUS && code >= QMP_ERROR) {
    retval = QMP_error_strings[code - QMP_ERROR + 1];
  }
  else {
#ifdef QMP_ERROR_STRING
    retval = QMP_ERROR_STRING(code);
#endif
  }

  if(retval == NULL) {
    static char errstr[256];
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
  QMP_status_t err;
  ENTER;
  if (!mh)
    err = QMP_machine->err_code;
  else {
    err = mh->err_code;
  }
  LEAVE;
  return err;
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
  const char *errstr;
  ENTER;
  if (!mh)
    errstr = QMP_error_string (QMP_machine->err_code);
  else {
    errstr = QMP_error_string (mh->err_code);
  }
  LEAVE;
  return errstr;
}
