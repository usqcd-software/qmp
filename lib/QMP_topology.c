#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

static QMP_status_t
QMP_set_topo(QMP_comm_t comm, const int* dims, int ndim, const int *map, int nmap)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;
  int i;
  int num_nodes = 1;
  for(i=0; i<ndim; ++i) {
    if (dims[i] < 1) {
      QMP_error ("QMP_declare_logical_topology: invalid length\n");
      status = QMP_INVALID_ARG;
      goto leave;
    }
    num_nodes *= dims[i];
  }
  if (num_nodes != QMP_comm_get_number_of_nodes(comm)) {
    QMP_error ("QMP_declare_logical_topology: requested machine size not equal to number of nodes\n");
    status = QMP_INVALID_ARG;
    goto leave;
  }
  QMP_assert(QMP_comm_logical_topology_is_declared(comm)==QMP_FALSE);

  QMP_logical_topology_t *topo;
  QMP_alloc(topo, QMP_logical_topology_t, 1);
  comm->topo = topo;

  topo->dimension = ndim;
  QMP_alloc(topo->logical_size, int, ndim);
  for(i=0; i < ndim; ++i) topo->logical_size[i] = dims[i];

  topo->mapdim = nmap;
  topo->map = NULL;
  if(map) {
    QMP_assert(nmap==ndim);
    QMP_alloc(topo->map, int, nmap);
    for(i=0; i<nmap; ++i) topo->map[i] = map[i];
  }

#ifdef QMP_SET_TOPO
  status = QMP_SET_TOPO(comm);
#endif

  QMP_barrier();

  topo->logical_coord = QMP_comm_get_logical_coordinates_from(comm, comm->nodeid);

  QMP_alloc(topo->neigh[0], int, 2*ndim);
  topo->neigh[1] = topo->neigh[0] + ndim;
  int *coord;
  QMP_alloc(coord, int, ndim);
  for(i=0; i < ndim; ++i) {
    int m;
    for(m=0; m < ndim; ++m) coord[m] = topo->logical_coord[m];

    coord[i] = (topo->logical_coord[i] - 1 + dims[i]) % dims[i];
    topo->neigh[0][i] = QMP_comm_get_node_number_from(comm, coord);

    coord[i] = (topo->logical_coord[i] + 1) % dims[i];
    topo->neigh[1][i] = QMP_comm_get_node_number_from(comm, coord);
  }
  QMP_free(coord);

  QMP_barrier();

 leave:
  LEAVE;
  return status;
}

static QMP_status_t
QMP_set_topo_native(QMP_comm_t comm, const int *map, int nmap)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;
#ifdef QMP_SET_TOPO_NATIVE
  status = QMP_SET_TOPO_NATIVE(comm);
#else
  QMP_FATAL("native topology unknown");
#endif
  LEAVE;
  return status;
}

/* This is called by members of the communicator */
QMP_status_t
QMP_comm_declare_logical_topology_map (QMP_comm_t comm, const int* dims, int ndim,
				       const int *map, int nmap)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;

  QMP_barrier();

  QMP_assert(nmap>=0);
  QMP_assert((nmap==0&&map==NULL)||(nmap>0&&map!=NULL));

  if(ndim>0) {  /// use dims
    QMP_assert(dims!=NULL);
    status = QMP_set_topo(comm, dims, ndim, map, nmap);
  } else if(ndim==-1) {  /// native
    status = QMP_set_topo_native(comm, map, nmap);
  } else {
    if(ndim!=0) { // ndim==0 is allowed and does nothing
      QMP_error ("QMP_declare_logical_topology: invalid ndim = %d\n", ndim);
      status = QMP_INVALID_ARG;
    }
  }

  LEAVE;
  return status;
}

QMP_status_t
QMP_comm_declare_logical_topology (QMP_comm_t comm, const int* dims, int ndim)
{
  QMP_status_t r;
  ENTER;
  r = QMP_comm_declare_logical_topology_map (comm, dims, ndim, NULL, 0);
  LEAVE;
  return r;
}

QMP_status_t
QMP_declare_logical_topology_map (const int* dims, int ndim, const int *map, int nmap)
{
  QMP_status_t r;
  ENTER;
  r = QMP_comm_declare_logical_topology_map (QMP_comm_get_default(), dims, ndim, map, nmap);
  LEAVE;
  return r;
}

QMP_status_t
QMP_declare_logical_topology (const int* dims, int ndim)
{
  QMP_status_t r;
  ENTER;
  r = QMP_comm_declare_logical_topology_map (QMP_comm_get_default(), dims, ndim, QMP_args->lmap, QMP_args->lmaplen);
  LEAVE;
  return r;
}


/* Is the logical topology declared? */
QMP_bool_t
QMP_comm_logical_topology_is_declared (QMP_comm_t comm)
{
  ENTER;
  LEAVE;
  return QMP_topo_declared(comm);
}

QMP_bool_t
QMP_logical_topology_is_declared (void)
{
  QMP_bool_t r;
  ENTER;
  r = QMP_comm_logical_topology_is_declared (QMP_comm_get_default());
  LEAVE;
  return  r;
}


/* Return the logical size of the machine */
int
QMP_comm_get_logical_number_of_dimensions(QMP_comm_t comm)
{
  int ndim = 0;
  ENTER;
  if(QMP_comm_logical_topology_is_declared(comm)) {
    ndim = comm->topo->dimension;
  }
  LEAVE;
  return ndim;
}

int
QMP_get_logical_number_of_dimensions(void)
{
  int nd;
  ENTER;
  nd = QMP_comm_get_logical_number_of_dimensions(QMP_comm_get_default());
  LEAVE;
  return nd;
}


/* Return the logical size of the machine */
const int *
QMP_comm_get_logical_dimensions(QMP_comm_t comm)
{
  const int *dims = NULL;
  ENTER;
  if(QMP_comm_logical_topology_is_declared(comm)) {
    dims = comm->topo->logical_size;
  }
  LEAVE;
  return dims;
}

const int *
QMP_get_logical_dimensions(void)
{
  const int *size;
  ENTER;
  size = QMP_comm_get_logical_dimensions(QMP_comm_get_default());
  LEAVE;
  return size;
}


/* Return the coordinate of the underlying grid */
const int *
QMP_comm_get_logical_coordinates(QMP_comm_t comm)
{
  ENTER;
  QMP_assert(QMP_comm_logical_topology_is_declared(comm));
  LEAVE;
  return comm->topo->logical_coord;
}

const int *
QMP_get_logical_coordinates(void)
{
  const int *coord;
  ENTER;
  coord = QMP_comm_get_logical_coordinates(QMP_comm_get_default());
  LEAVE;
  return coord;
}


/* Return logical coordinate from a node id */
int *
QMP_comm_get_logical_coordinates_from (QMP_comm_t comm, int node)
{
  int *c;
  ENTER;

  QMP_assert(QMP_comm_logical_topology_is_declared(comm));

  int nd = QMP_comm_get_logical_number_of_dimensions(comm);
  QMP_alloc(c, int, nd);

#ifdef QMP_COMM_GET_LOGICAL_COORDINATES_FROM
  QMP_COMM_GET_LOGICAL_COORDINATES_FROM(c, nd, comm, node);
#else
  int i;
  for(i=0; i<nd; i++) c[i] = 0;
#endif

  LEAVE;
  return c;
}

void
QMP_comm_get_logical_coordinates_from2 (QMP_comm_t comm, int coords[], int node)
{
  ENTER;

  QMP_assert(QMP_comm_logical_topology_is_declared(comm));

  int nd = QMP_comm_get_logical_number_of_dimensions(comm);

#ifdef QMP_COMM_GET_LOGICAL_COORDINATES_FROM
  QMP_COMM_GET_LOGICAL_COORDINATES_FROM(coords, nd, comm, node);
#else
  int i;
  for(i=0; i<nd; i++) coords[i] = 0;
#endif

  LEAVE;
}

int *
QMP_get_logical_coordinates_from (int node)
{
  int *c;
  ENTER;

  c = QMP_comm_get_logical_coordinates_from (QMP_comm_get_default(), node);

  LEAVE;
  return c;
}

void
QMP_get_logical_coordinates_from2 (int coords[], int node)
{
  ENTER;

  QMP_comm_get_logical_coordinates_from2 (QMP_comm_get_default(), coords, node);

  LEAVE;
}


/**
 * Return a node number from a logical coordinate.
 */
int 
QMP_comm_get_node_number_from (QMP_comm_t comm, const int* coordinates)
{
  int world_node = 0;
  ENTER;
  QMP_assert(QMP_comm_logical_topology_is_declared(comm));

#ifdef QMP_COMM_GET_NODE_NUMBER_FROM
  world_node = QMP_COMM_GET_NODE_NUMBER_FROM(comm, coordinates);
#endif

  LEAVE;
  return world_node;
}

int 
QMP_get_node_number_from (const int* coordinates)
{
  int world_node = 0;
  ENTER;

  world_node = QMP_comm_get_node_number_from(QMP_comm_get_default(), coordinates);

  LEAVE;
  return world_node;
}
