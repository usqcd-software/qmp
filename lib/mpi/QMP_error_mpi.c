#include "QMP_P_COMMON.h"

static char errstr[MPI_MAX_ERROR_STRING];

const char*
QMP_error_string_mpi (QMP_status_t code)
{
  const char *retval=NULL;

  int errlen;
  int err = MPI_Error_string (code, errstr, &errlen);
  if(err==MPI_SUCCESS) retval = (const char *)&errstr;

  return retval;
}
