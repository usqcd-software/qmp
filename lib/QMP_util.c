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
 *      QMP utility routines
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_util.c,v $
 *   Revision 1.4  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.3  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.2  2004/12/19 07:33:11  morten
 *   Added dummy functions for the new profiling functions,
 *   QMP_get_total_qmp_time(void)
 *   QMP_get_total_qmp_time(void)
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.2  2004/09/01 21:16:47  osborn
 *   Added QMP_is_initialized().
 *
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.5  2003/06/04 19:19:39  edwards
 *   Added a QMP_abort() function.
 *
 *   Revision 1.4  2003/04/23 04:59:31  edwards
 *   Ifdef protected check of MPI_ERR_WIN. Doesn't seem to be defined
 *   on IBM's.
 *
 *   Revision 1.3  2003/02/13 16:22:24  chen
 *   qmp version 1.2
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.2  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.1  2002/04/22 20:28:45  chen
 *   Version 0.95 Release
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

/**
 * Check if QMP is initialized.
 */
QMP_bool_t
QMP_is_initialized(void)
{
  ENTER;
  LEAVE;
  return QMP_global_m->inited;
}

/**
 *  Set verbosity level.
 */
int
QMP_verbose (int level)
{
  int old = QMP_global_m->verbose;
  QMP_global_m->verbose = level;
  return old;
}

/**
 *  Set profiling level.
 */
int
QMP_profcontrol (int level)
{
  int old = QMP_global_m->proflevel;
  QMP_global_m->proflevel = level;
  return old;
}

/**
 *  functions for profiling
 */
void  
QMP_reset_total_qmp_time(void)
{
  QMP_global_m->total_qmp_time = 0.0;
}

double 
QMP_get_total_qmp_time(void)
{
  return QMP_global_m->total_qmp_time;
}

/**
 * Simple QMP specific fprintf type of routine
 */
static int
QMP_fprintf_tag (FILE* stream, char *tag, const char* format, va_list argp)
{
  int     status;
  char    info[128];
  char    buffer[1024];

  snprintf(info, sizeof(info), "QMP m%d,n%d@%s%s:", QMP_global_m->nodeid,
	   QMP_global_m->num_nodes, QMP_global_m->host, tag);

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
