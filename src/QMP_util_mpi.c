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
 *      QMP Over MPI utility routines
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
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

#include "QMP.h"
#include "QMP_P_MPI.h"

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
  int    errlen = 255;

  if (code == QMP_SUCCESS)
    return QMP_error_strings[code];

  if (code < QMP_MAX_STATUS && code >= QMP_ERROR)
    return QMP_error_strings[code - QMP_ERROR + 1];

  if (code <= MPI_ERR_WIN && code >= MPI_ERR_BUFFER) {
    MPI_Error_string (code, errstr, &errlen);
    return (const char *)&errstr;
  }
  
  snprintf (errstr, sizeof (errstr) - 1, "unknown error code %d", code);
  return (const char *)&errstr;
}

/**
 * Simple QMP specific printf type of routine
 */
int
QMP_printf (const char* format, ...)
{
  va_list argp;
  int     status, rank, size;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf (info, "QMP m%d,n%d@%s : ", 
	   rank, size, hostname);

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  printf ("%s %s\n", info, buffer);
  return status;
}


/**
 * Simple information display routine
 */
int
QMP_info (const char* format, ...)
{
  va_list argp;
  int     status, rank, size;
  char    info[128], hostname[256];
  char    buffer[1024];


  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);
  sprintf (info, "QMP m%d,n%d@%s info: ", 
	   rank, size, hostname);

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  printf ("%s %s\n", info, buffer);
  return status;
}

/**
 * Simple QMP specific fprintf type of routine
 */
int
QMP_fprintf (FILE* stream, const char* format, ...)
{
 va_list argp;
  int     status, rank, size;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf (info, "QMP m%d,n%d@%s : ", 
	   rank, size, hostname);

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stream, "%s %s\n", info, buffer);
  return status;
}


/**
 * Simple error display routine
 */
int
QMP_error (const char* format, ...)
{
  va_list argp;
  int     status, rank, size;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf (info, "QMP m%d,n%d@%s error: ", rank, size, hostname);
	   

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);
  return status;
}

/**
 * Simple error display routine
 */
void
QMP_error_exit (const char* format, ...)
{
  va_list argp;
  int     status, rank, size;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf (info, "QMP m%d,n%d@%s fatal error: ", rank, size, hostname);
	   

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);
  exit (1);
}

/**
 * QMP Fatal error: exit this thing.
 */
void
QMP_fatal (QMP_u32_t rank, const char* format, ...)
{
  va_list argp;
  int     status;
  char    info[128], hostname[256];
  char    buffer[1024];

  /* get machine host name */
  gethostname (hostname, sizeof (hostname) - 1);

  sprintf (info, "QMP rank = %d at host %s fatal: ", rank, hostname);

  va_start (argp, format);
  status = vsprintf (buffer, format, argp);
  va_end (argp);

  fprintf (stderr, "%s %s\n", info, buffer);

  MPI_Abort(MPI_COMM_WORLD, 1);

  exit(1);
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
  if (!mh)
    return QMP_global_m.err_code;
  else {
    Message_Handle* rmh;
    rmh = (Message_Handle *)mh;
    return rmh->err_code;
  }
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
  if (!mh)
    return QMP_error_string (QMP_global_m.err_code);
  else {
    Message_Handle* rmh;
    rmh = (Message_Handle *)mh;
    return QMP_error_string (rmh->err_code);
  }
}


/**
 * Enable or Disable verbose mode.
 */
void
QMP_verbose (QMP_bool_t verbose)
{
  QMP_global_m.verbose = verbose;
}
