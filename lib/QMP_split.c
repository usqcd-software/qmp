#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

/**
 * Get the number of distinct colors used to create communicator
 */
int
QMP_comm_get_number_of_colors (QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return comm->ncolors;
}


/**
 * Get the color used to create communicator
 */
int
QMP_comm_get_color (QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return comm->color;
}


/**
 * Get the key used to create communicator
 */
int
QMP_comm_get_key (QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return comm->key;
}


/**
 * Split a communicator into one or more disjoint communicators.
 */
QMP_status_t
QMP_comm_split(QMP_comm_t comm, int color, int key,
	       QMP_comm_t *newcomm)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  QMP_alloc(*newcomm, struct QMP_comm_struct, 1);
  **newcomm = (struct QMP_comm_struct) {QMP_COMM_INIT};
  (*newcomm)->color = color;
  (*newcomm)->key = key;
  (*newcomm)->topo = NULL;

  double t = (double)color;
  QMP_comm_max_double(comm, &t);
  int cmax = 1 + (int)t;
  QMP_assert(cmax>0);
  double ca[cmax];
  int i;
  for(i=0; i<cmax; i++) ca[i] = 0;
  if(color>=0) ca[color] = 1;
  QMP_comm_sum_double_array(comm, ca, cmax);
  int nc = 0;
  for(i=0; i<cmax; i++) if(ca[i]) nc++;
  (*newcomm)->ncolors = nc;

#ifdef QMP_COMM_SPLIT
  status = QMP_COMM_SPLIT(comm, *newcomm);
#else
  (*newcomm)->num_nodes = 1;
  (*newcomm)->nodeid = 0;
#endif

  LEAVE;
  return status;
}

QMP_status_t
QMP_comm_free(QMP_comm_t comm)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

#ifdef QMP_COMM_FREE
  status = QMP_COMM_FREE(comm);
#endif
  QMP_free(comm);

  LEAVE;
  return status;
}
