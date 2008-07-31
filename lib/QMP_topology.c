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
 *      QMP logical topology code
 *
 * Author:  
 *      Jie Chen, Robert Edwards and Chip Watson
 *      Jefferson Lab HPC Group
 *      James Osborn (BU)
 *
 * Revision History:
 *   $Log: QMP_topology.c,v $
 *   Revision 1.12  2008/03/06 17:28:42  osborn
 *   fixes to mapping options
 *
 *   Revision 1.11  2008/03/06 07:54:11  osborn
 *   added -qmp-alloc-map command line argument
 *
 *   Revision 1.10  2008/01/29 02:53:21  osborn
 *   Fixed single node version.  Bumped version to 2.2.0.
 *
 *   Revision 1.9  2008/01/25 20:07:39  osborn
 *   Added BG/P personality info.  Now uses MPI_Cart_create to layout logical
 *   topology.
 *
 *   Revision 1.8  2006/10/03 21:31:14  osborn
 *   Added "-qmp-geom native" command line option for BG/L.
 *
 *   Revision 1.7  2006/06/13 17:43:09  bjoo
 *   Removed some c99isms. Code  compiles on Cray at ORNL using pgcc
 *
 *   Revision 1.6  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.5  2006/01/05 19:32:09  osborn
 *   Fixes to BG/L personality code.  Ready for version 2.1.3.
 *
 *   Revision 1.4  2006/01/05 03:12:56  osborn
 *   Added --enable-bgl compilation option.
 *
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

#ifdef HAVE_MPI
#include "QMP_P_MPI.h"

static MPI_Comm comm_cart;
static int *c2w, *w2c;

static void
sumint(int *v, int n)
{
  int i, *t;
  t = malloc(n*sizeof(int));
  MPI_Allreduce((void *)v, (void *)t, n, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  for(i=0; i<n; i++) v[i] = t[i];
  free(t);
}

static void
remap_mpi(int *dims, int ndim)
{
  int i, periods[ndim], reorder=1, rankc, rankw, size;

  for(i=0; i<ndim; i++) periods[i]=1;
  MPI_Cart_create(QMP_COMM_WORLD, ndim, dims, periods, reorder, &comm_cart);
  MPI_Comm_rank(comm_cart, &rankc);
  rankw = QMP_global_m->nodeid;
  size = QMP_global_m->num_nodes;

  c2w = malloc(size*sizeof(int));
  w2c = malloc(size*sizeof(int));
  for(i=0; i<size; i++) c2w[i] = w2c[i] = 0;
  c2w[rankc] = rankw;
  w2c[rankw] = rankc;
  sumint(c2w, size);
  sumint(w2c, size);
}
#endif

static void
get_coord(int *x, int n)
{
  int i;
  int nd = QMP_topo->dimension;
  int *l = QMP_topo->logical_size;
  int *p = QMP_global_m->lmap;

  for(i=0; i<nd; i++) {
    int k;
    if(p) k = p[i]; else k = i;
    x[k] = n % l[k];
    n /= l[k];
  }
}

static int
get_rank(int *x)
{
  int i, n;
  int nd = QMP_topo->dimension;
  int *l = QMP_topo->logical_size;
  int *p = QMP_global_m->lmap;

  n = 0;
  for(i=nd-1; i>=0; i--) {
    int k;
    if(p) k = p[i]; else k = i;
    n = (n*l[k]) + x[k];
  }
  return n;
}

/* This is called by all children */
QMP_status_t
QMP_declare_logical_topology (const int* dims, int ndim)
{
  int i;
  int num_nodes = 1;
  int *coord;
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  if (ndim < 0) {
    QMP_error ("QMP_declare_logical_topology: invalid ndim = %d\n", ndim);
    status = QMP_INVALID_ARG;
    goto leave;
  }
  for(i=0; i < ndim; ++i) {
    if (dims[i] < 1) {
      QMP_error ("QMP_declare_logical_topology: invalid length\n");
      status = QMP_INVALID_ARG;
      goto leave;
    }

    num_nodes *= dims[i];
  }
  if (num_nodes != QMP_global_m->num_nodes) {
    QMP_error ("QMP_declare_logical_topology: requested machine size not equal to number of nodes\n");
    status = QMP_INVALID_ARG;
    goto leave;
  }
  if ( QMP_global_m->lmaplen && (ndim != QMP_global_m->lmaplen) ) {
    QMP_error ("QMP_declare_logical_topology: logical topology dimension not equal to logical map dimension\n");
    status = QMP_INVALID_ARG;
    goto leave;
  }

#ifdef HAVE_MPI
  //remap_mpi((int *)dims, ndim);
#endif

  QMP_topo->dimension = ndim;
  QMP_topo->logical_size = (int *) malloc(ndim*sizeof(int));
  for(i=0; i < ndim; ++i) QMP_topo->logical_size[i] = dims[i];

  QMP_topo->logical_coord = QMP_get_logical_coordinates_from(QMP_global_m->nodeid);

  QMP_topo->neigh[0] = (int *) malloc(2*ndim*sizeof(int));
  QMP_topo->neigh[1] = QMP_topo->neigh[0] + ndim;
  for(i=0; i < ndim; ++i)
  {
    int m;

    coord = (int *)malloc(ndim*sizeof(int));
    if(coord == NULL) { 
      QMP_FATAL("Couldnt alloc coord\n");
    }

    for(m=0; m < ndim; ++m)
      coord[m] = QMP_topo->logical_coord[m];

    coord[i] = (QMP_topo->logical_coord[i] - 1 + dims[i]) % dims[i];
    QMP_topo->neigh[0][i] = QMP_get_node_number_from(coord);

    coord[i] = (QMP_topo->logical_coord[i] + 1) % dims[i];
    QMP_topo->neigh[1][i] = QMP_get_node_number_from(coord);

    free(coord);
  }

  QMP_topo->topology_declared = QMP_TRUE;

 leave:
  LEAVE;
  return status;
}

/* Is the logical topology declared? */
QMP_bool_t
QMP_logical_topology_is_declared (void)
{
  ENTER;
  LEAVE;
  return  QMP_topo->topology_declared;
}

/* Return the logical size of the machine */
int
QMP_get_logical_number_of_dimensions(void)
{
  ENTER;
  LEAVE;
  return QMP_topo->dimension;
}

/* Return the logical size of the machine */
const int *
QMP_get_logical_dimensions(void)
{
  ENTER;
  LEAVE;
  return QMP_topo->logical_size;
}

/* Return the coordinate of the underlying grid */
const int *
QMP_get_logical_coordinates(void)
{
  ENTER;
  LEAVE;
  return QMP_topo->logical_coord;
}

/* Return logical coordinate from a node id */
int *
QMP_get_logical_coordinates_from (int node)
{
  int *nc, nd;
  ENTER;

  nd = QMP_topo->dimension;
  nc = (int *) malloc(nd*sizeof(int));
#ifdef HAVE_MPI
  //int cart_node = w2c[node];
  //MPI_Cart_coords(comm_cart, cart_node, nd, nc);
  get_coord(nc, node);
#else /* single node version */
  int i;
  for(i=0; i<nd; i++) nc[i] = 0;
#endif

  LEAVE;
  return nc;
}

/**
 * Return a node number from a logical coordinate.
 */
int 
QMP_get_node_number_from (const int* coordinates)
{
  int world_node;
  ENTER;

#ifdef HAVE_MPI
  //int cart_node;
  //MPI_Cart_rank(comm_cart, (int *)coordinates, &cart_node);
  //world_node = c2w[cart_node];
  world_node = get_rank(coordinates);
#else
  world_node = 0;
#endif

  LEAVE;
  return world_node;
}
