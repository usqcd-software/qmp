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
 *      QMP error reporting code for MPI build
 *
 * Author:  
 *      Robert Edwards, Jie Chen and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1  2004/06/17 21:37:23  osborn
 *   added file QMP_error_mpi.c
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

#include "qmp.h"
#include "QMP_P_COMMON.h"
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

#if ! defined(_AIX)
  if (code <= MPI_ERR_WIN && code >= MPI_ERR_BUFFER) {
    MPI_Error_string (code, errstr, &errlen);
    return (const char *)&errstr;
  }
#endif

  snprintf (errstr, sizeof (errstr), "unknown error code %d", code);
  return (const char *)&errstr;
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
    return QMP_global_m->err_code;
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
    return QMP_error_string (QMP_global_m->err_code);
  else {
    Message_Handle* rmh;
    rmh = (Message_Handle *)mh;
    return QMP_error_string (rmh->err_code);
  }
}
