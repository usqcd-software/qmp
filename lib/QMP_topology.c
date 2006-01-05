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
 *   Revision 1.3  2005/06/21 20:18:39  osborn
 *   Added -qmp-geom command line argument to force grid-like behavior.
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

#include "QMP_P_COMMON.h"

static int *remap=NULL;

#ifndef HAVE_BGL /* not on a BG/L */

static void
remap_axes(int ndim, int *dims)
{
  int i;
  remap = (int *) malloc(ndim*sizeof(int));
  for(i=0; i<ndim; i++) remap[i] = i;
}

#else /* HAVE_BGL -- use BG/L personality */

#include <bglpersonality.h>

static void
sort(int *index, int *key, int n)
{
  int i, j;
  for(i=0; i<n; i++) index[i] = i;

  for(i=0; i<n; i++) {
    for(j=i+1; j<n; j++) {
      if(key[index[i]]<key[index[j]]) {
	int k = index[i];
	index[i] = index[j];
	index[j] = k;
      }
    }
  }
}

/* try to have dims[remap[i]] == bgl_dims[i] */
static void
remap_axes(int ndim, int *dims)
{
  int i;
  int bgl_ndim=4, bgl_dims[4], bgl_index[4];
  int *index;
  BGLPersonality pers;

  rts_get_personality(&pers, sizeof(pers));
  bgl_dims[0] = pers.xSize;
  bgl_dims[1] = pers.ySize;
  bgl_dims[2] = pers.zSize;
  bgl_dims[3] = (p->opFlags & BGLPERSONALITY_OPFLAGS_VIRTUALNM) ? 2 : 1;
  sort(bgl_index, bgl_dims, bgl_ndim);

  index = (int *) malloc(ndim*sizeof(int));
  sort(index, dims, ndim);

  remap = (int *) malloc(ndim*sizeof(int));
  for(i=0; i<ndim; i++) {
    k = i;
    if(i<bgl_ndim) k = bgl_index[i];
    remap[k] = index[i];
  }

  free(index);
}

#endif /* HAVE_BGL */

static void 
crtesn_coord(int ipos, int coordf[], int latt_size[], int ndim)
{
  int i;
  for(i=0; i<ndim; ++i) {
    int ir = remap[i];
    coordf[ir] = ipos % latt_size[ir];
    ipos = ipos / latt_size[ir];
  }
}

static int 
crtesn_pos(int coordf[], int latt_size[], int ndim)
{
  int k, ipos;
  ipos = 0;
  for(k=ndim-1; k>=0; k--) {
    int kr = remap[k];
    ipos = ipos * latt_size[kr] + coordf[kr]; 
  }
  return ipos;
}

/* This is called by all children */
QMP_status_t  
QMP_declare_logical_topology (const int* dims, int ndim)
{
  int i;
  int num_nodes = 1;

  if (ndim < 0) {
    QMP_error ("QMP_declare_logical_topology: invalid ndim = %d\n", ndim);
    return QMP_INVALID_ARG;
  }
  for(i=0; i < ndim; ++i) {
    if (dims[i] < 1) {
      QMP_error ("QMP_declare_logical_topology: invalid length\n");
      return QMP_INVALID_ARG;
    }

    num_nodes *= dims[i];
  }

  if (num_nodes != QMP_global_m->num_nodes)
  {
    QMP_error ("QMP_declare_logical_topology: requested machine size not equal to number of nodes\n");
    return QMP_INVALID_ARG;
  }

  if(QMP_global_m->ic_type!=QMP_SWITCH) {
    if(ndim!=QMP_global_m->ndim) {
      QMP_error ("QMP_declare_logical_topology: requested ndim (%d) not equal to machine ndim (%d)\n", ndim, QMP_global_m->ndim);
      return QMP_INVALID_ARG;
    }
    for(i=0; i<ndim; ++i) {
      if(dims[i] != QMP_global_m->geom[i]) {
	QMP_error ("QMP_declare_logical_topology: requested dim (%d) not equal to machine geom (%d) in direction %d\n", dims[i], QMP_global_m->geom[i], i);
	return QMP_INVALID_ARG;
      }
    }
  }

  remap_axes(ndim, dims);

  QMP_topo->dimension = ndim;
  QMP_topo->logical_size = (int *) malloc(ndim*sizeof(int));
  for(i=0; i < ndim; ++i) QMP_topo->logical_size[i] = dims[i];

  QMP_topo->logical_coord = (int *) malloc(ndim*sizeof(int));
  crtesn_coord(QMP_global_m->nodeid, QMP_topo->logical_coord,
	       QMP_topo->logical_size, ndim);

  QMP_topo->neigh[0] = (int *) malloc(2*ndim*sizeof(int));
  QMP_topo->neigh[1] = QMP_topo->neigh[0] + ndim;
  for(i=0; i < ndim; ++i)
  {
    int coord[ndim];
    int m;

    for(m=0; m < ndim; ++m)
      coord[m] = QMP_topo->logical_coord[m];

    coord[i] = (QMP_topo->logical_coord[i] - 1 + dims[i]) % dims[i];
    QMP_topo->neigh[0][i] = 
      crtesn_pos(coord, QMP_topo->logical_size, ndim);

    coord[i] = (QMP_topo->logical_coord[i] + 1) % dims[i];
    QMP_topo->neigh[1][i] =
      crtesn_pos(coord, QMP_topo->logical_size, ndim);
  }

  QMP_topo->topology_declared = QMP_TRUE;

  return QMP_SUCCESS;
}

/* Is the logical topology declared? */
QMP_bool_t
QMP_logical_topology_is_declared (void)
{
  return  QMP_topo->topology_declared;
}

/* Return the logical size of the machine */
int
QMP_get_logical_number_of_dimensions(void)
{
  return QMP_topo->dimension;
}

/* Return the logical size of the machine */
const int *
QMP_get_logical_dimensions(void)
{
  return QMP_topo->logical_size;
}

/* Return the coordinate of the underlying grid */
const int *
QMP_get_logical_coordinates(void)
{
  return QMP_topo->logical_coord;
}

/* Return logical coordinate from a node id */
int *
QMP_get_logical_coordinates_from (int node)
{
  int* nc;

  nc = (int *)malloc (sizeof(int)*QMP_topo->dimension);

  crtesn_coord (node, nc, QMP_topo->logical_size,
		QMP_topo->dimension);

  return nc;
}

/**
 * Return a node number from a logical coordinate.
 */
int 
QMP_get_node_number_from (const int* coordinates)
{
  int logic_node;

  logic_node = crtesn_pos ((int *)coordinates, QMP_topo->logical_size,
			   QMP_topo->dimension);

  return logic_node;
}
