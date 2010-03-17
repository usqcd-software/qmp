#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"


/* return message passing type */
QMP_ictype_t 
QMP_get_msg_passing_type (void)
{
  ENTER;
  LEAVE;
  return QMP_machine->ic_type;
}


/* Return the total number of nodes within the machine */
int
QMP_comm_get_number_of_nodes(QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return comm->num_nodes;
}

int
QMP_get_number_of_nodes(void)
{
  int r;
  ENTER;
  r = QMP_comm_get_number_of_nodes(QMP_comm_get_default());
  LEAVE;
  return r;
}


/* Return the nodeid within the machine */
int
QMP_comm_get_node_number(QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return comm->nodeid;
}

int
QMP_get_node_number(void)
{
  int r;
  ENTER;
  r = QMP_comm_get_node_number(QMP_comm_get_default());
  LEAVE;
  return r;
}


/* Return the total number of nodes within the machine */
int
QMP_get_number_of_jobs(void)
{
  int r;
  ENTER;
  r = QMP_comm_get_number_of_colors(QMP_comm_get_job());
  LEAVE;
  return r;
}


/* Return the nodeid within the machine */
int
QMP_get_job_number(void)
{
  int r;
  ENTER;
  r = QMP_comm_get_color(QMP_comm_get_job());
  LEAVE;
  return r;
}


/* Return the number of job geometry dimensions */
int
QMP_get_number_of_job_geometry_dimensions(void)
{
  int r;
  ENTER;
  r = QMP_comm_get_logical_number_of_dimensions(QMP_comm_get_job());
  LEAVE;
  return r;
}


/* Return the job geometry */
const int *
QMP_get_job_geometry(void)
{
  const int *r;
  ENTER;
  r = QMP_comm_get_logical_dimensions(QMP_comm_get_job());
  LEAVE;
  return r;
}


/* Is this the primary node in the machine? */
QMP_bool_t
QMP_comm_is_primary_node(QMP_comm_t comm)
{
  QMP_bool_t r;
  ENTER;
  r = (comm->nodeid == 0) ? QMP_TRUE : QMP_FALSE;
  LEAVE;
  return r;
}

QMP_bool_t
QMP_is_primary_node(void)
{
  QMP_bool_t r;
  ENTER;
  r = QMP_comm_is_primary_node(QMP_comm_get_default());
  LEAVE;
  return r;
}


/* Return the physical size of the machine */
int
QMP_get_allocated_number_of_dimensions(void)
{
  int ndim;
  ENTER;
  ndim = QMP_comm_get_logical_number_of_dimensions(QMP_comm_get_allocated());
  LEAVE;
  return ndim;
}


/* Return the physical size of the machine */
const int *
QMP_get_allocated_dimensions(void)
{
  const int *geom;
  ENTER;
  geom = QMP_comm_get_logical_dimensions(QMP_comm_get_allocated());
  LEAVE;
  return geom;
}


/* Return the physical coordinate of this node */
const int *
QMP_get_allocated_coordinates(void)
{
  const int *coord;
  ENTER;
  coord = QMP_comm_get_logical_coordinates(QMP_comm_get_allocated());
  LEAVE;
  return coord;
}


/* For partitioned I/O, nodes are partitioned into subsets.  Each
   subset includes a designated I/O node.  This function maps a node
   to its I/O node. */
/* The default partitioning scheme for switched clusters has each node
   perform as its own I/O node, subsets have only one member. */
int
QMP_io_node(int node)
{
  ENTER;
  LEAVE;
  return node;
}


/* For binary file I/O we designate a master I/O node for the entire
   machine.  This global constant defines it. */
int
QMP_master_io_node(void)
{
  ENTER;
  LEAVE;
  return 0;
}
