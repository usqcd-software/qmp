#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>

#include "QMP_P_COMMON.h"

/**
 *  Check if QMP is initialized.
 */
QMP_bool_t
QMP_is_initialized(void)
{
  return QMP_machine->inited;
}

/**
 *  Set verbosity level.
 */
int
QMP_verbose (int level)
{
  int old = QMP_machine->verbose;
  QMP_machine->verbose = level;
  return old;
}

/**
 *  Set profiling level.
 */
int
QMP_profcontrol (int level)
{
  int old = QMP_machine->proflevel;
  QMP_machine->proflevel = level;
  return old;
}

/**
 *  functions for profiling
 */
void  
QMP_reset_total_qmp_time(void)
{
  QMP_machine->total_qmp_time = 0.0;
}

double 
QMP_get_total_qmp_time(void)
{
  return QMP_machine->total_qmp_time;
}

/**
 * Simple QMP specific fprintf type of routine
 */
static int
QMP_fprintf_tag (FILE* stream, char *tag, const char* format, va_list argp)
{
  int     status, nodeid, num_nodes;
  char    info[128];
  char    buffer[1024];

  // these can't be function calls
  num_nodes = QMP_machine->mnodes;
  nodeid = QMP_machine->mnodeid;

  snprintf(info, sizeof(info), "QMP m%d,n%d@%s%s:", nodeid,
	   num_nodes, QMP_machine->host, tag);

  status = vsnprintf (buffer, sizeof(buffer), format, argp);

  fprintf (stream, "%s %s\n", info, buffer);
  return status;
}

/**
 * Simple QMP specific printf type of routine
 */
int
QMP_printf (const char* format, ...)
{
  int status;
  va_list argp;

  va_start (argp, format);
  status = QMP_fprintf_tag (stdout, "", format, argp);
  va_end (argp);

  return status;
}

/**
 * Simple QMP specific fprintf type of routine
 */
int
QMP_fprintf (FILE* stream, const char* format, ...)
{
  int status;
  va_list argp;

  va_start (argp, format);
  status = QMP_fprintf_tag (stream, "", format, argp);
  va_end (argp);

  return status;
}

/**
 * Simple information display routine
 */
int
QMP_info (const char* format, ...)
{
  int status;
  va_list argp;

  va_start (argp, format);
  status = QMP_fprintf_tag (stdout, " info", format, argp);
  va_end (argp);

  return status;
}

/**
 * Simple error display routine
 */
int
QMP_error (const char* format, ...)
{
  int status;
  va_list argp;

  va_start (argp, format);
  status = QMP_fprintf_tag (stderr, " error", format, argp);
  va_end (argp);

  return status;
}

/* don't time or debug this function */
double
QMP_time(void)
{
#ifdef QMP_TIME
  return QMP_TIME();
#else
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
#endif
}

/**
 *  Version information
 */

static const char *vs = VERSION;

const char *
QMP_version_str(void)
{
  return vs;
}

int
QMP_version_int(void)
{
  int maj, min, bug;
  sscanf(vs, "%i.%i.%i", &maj, &min, &bug);
  return ((maj*1000)+min)*1000 + bug;
}
