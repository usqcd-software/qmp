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
 *      QMP intialize code for MPI
 *
 * Author:  
 *      Jie Chen, Robert Edwards and Chip Watson
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.7  2004/02/05 02:33:37  edwards
 *   Removed a debugging statement.
 *
 *   Revision 1.6  2003/11/04 02:14:55  edwards
 *   Bug fix. The malloc in QMP_get_logical_coordinates_from had
 *   an invalid argument.
 *
 *   Revision 1.5  2003/11/04 01:04:32  edwards
 *   Changed QMP_get_logical_coordinates_from to not have const modifier.
 *   Now, user must explicitly call "free".
 *
 *   Revision 1.4  2003/06/04 19:19:39  edwards
 *   Added a QMP_abort() function.
 *
 *   Revision 1.3  2003/02/13 16:22:23  chen
 *   qmp version 1.2
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.3  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:42  chen
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

/* return message passing type */
QMP_ictype_t 
QMP_get_msg_passing_type (void)
{
  return QMP_global_m->ic_type;
}

/* Return the total number of nodes within the machine */
int
QMP_get_number_of_nodes(void)
{
  return QMP_global_m->num_nodes;
}

/* Return the nodeid within the machine */
int
QMP_get_node_number(void)
{
  return QMP_global_m->nodeid;
}

/* Is this the primary node in the machine? */
QMP_bool_t
QMP_is_primary_node(void)
{
  return (QMP_global_m->nodeid == 0) ? QMP_TRUE : QMP_FALSE;
}

/* Return the physical size of the machine */
int
QMP_get_allocated_number_of_dimensions(void)
{
  if (!QMP_global_m->inited) {
    QMP_FATAL ("QMP system is not initialized.");
    return 0;
  }
  if (QMP_global_m->ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_FATAL ("QMP supports only switched network configuration.");
    return 0;
  }
}

/* Return the physical size of the machine */
const int *
QMP_get_allocated_dimensions(void)
{
  if (!QMP_global_m->inited) {
    QMP_FATAL ("QMP system is not initialized.");
    return 0;
  }
  if (QMP_global_m->ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_FATAL ("QMP supports only switched network configuration.");
    return 0;
  }
}

/* Return the physical coordinate of this node */
const int *
QMP_get_allocated_coordinates(void)
{
  if (!QMP_global_m->inited) {
    QMP_FATAL("QMP system is not initialized.");
    return 0;
  }

  if (QMP_global_m->ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_FATAL("QMP supports only switched network configuration.");
    return 0;
  }
}
