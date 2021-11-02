#include "QMP_P_COMMON.h"

#if 0
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
get_coord(int *x, int n, int *l, int *p, int nd)
{
  int i;
  for(i=0; i<nd; i++) {
    int k;
    if(p) k = p[i]; else k = i;
    x[k] = n % l[k];
    n /= l[k];
  }
}

static int
get_rank(const int *x, int *l, int *p, int nd)
{
  int i, n;
  n = 0;
  for(i=nd-1; i>=0; i--) {
    int k;
    if(p) k = p[i]; else k = i;
    n = (n*l[k]) + x[k];
  }
  return n;
}

QMP_status_t
QMP_set_topo_mpi(QMP_comm_t comm)
{
  _QMP_UNUSED_ARGUMENT(comm);
  //remap_mpi((int *)dims, ndim);
  return QMP_SUCCESS;
}

void
QMP_comm_get_logical_coordinates_from_mpi(int *c, int nd, QMP_comm_t comm, int node)
{
  //int cart_node = w2c[node];
  //MPI_Cart_coords(comm_cart, cart_node, nd, nc);
  get_coord(c, node, comm->topo->logical_size, comm->topo->map, nd);
}

int 
QMP_comm_get_node_number_from_mpi(QMP_comm_t comm, const int* coords)
{
  //int cart_node;
  //MPI_Cart_rank(comm_cart, (int *)coordinates, &cart_node);
  //world_node = c2w[cart_node];
  int n;
  n = get_rank(coords, comm->topo->logical_size, comm->topo->map, comm->topo->dimension);
  return n;
}
