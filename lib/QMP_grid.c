#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QMP_P_COMMON.h"

/* The subgrid geometry */
typedef struct QMP_subgrid
{
  /* Dimension of problem geometry */
  int dimension;

  /* Subgrid length each direction */
  int *length;

  /* Number of sites in each subgrid */
  int vol;

} QMP_subgrid_t;

static QMP_subgrid_t subgrid;

static int prime[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
#define MAXPRIMES (sizeof(prime)/sizeof(int))

#if 0
/* Check if a is a factor of b */
static unsigned 
is_factor(unsigned a, unsigned b)
{
  return ((b % a) == 0 ? 1 : 0);
}
#endif

/*
 * QMP_layout_grid
 *
 * General geometry constructor. Automatically determines the
 * optimal layout -> decides the optimal ordering of axes.
 */
QMP_status_t
QMP_layout_grid (const int *dims, int ndim)
{
  QMP_status_t status = QMP_SUCCESS;
  ENTER;
  int i;
  int *squaresize;
  int *nsquares;

  QMP_alloc(squaresize, int, ndim);
  QMP_alloc(nsquares, int, ndim);
  if(nsquares == NULL || squaresize == NULL) {
     QMP_FATAL("Unable to malloc in QMP_layout_grid");
  }

  if(QMP_machine->ic_type!=QMP_SWITCH) {
    QMP_declare_logical_topology(QMP_get_job_geometry(), QMP_get_number_of_job_geometry_dimensions());
  }

  /* If logical topology not set, this machine allows configuration of size */
  if (! QMP_logical_topology_is_declared()) {
    int j, k, n;

    /* Initialize subgrid and machine size */
    for(i=0; i<ndim; ++i) {
      squaresize[i] = dims[i];
      nsquares[i] = 1;
    }

    n = QMP_get_number_of_nodes();
    k = MAXPRIMES-1;
    while(n>1) {
      /* figure out which prime to divide by starting with largest */
      while( (n%prime[k]!=0) && (k>0) ) --k;

      /* figure out which direction to divide */
      /* find largest divisible dimension of h-cubes */
      /* if one direction with largest dimension has already been
	 divided, divide it again.  Otherwise divide first direction
	 with largest dimension. */
      j = -1;
      for(i=0; i<ndim; i++) {
	if(squaresize[i]%prime[k]==0) {
	  if( (j<0) || (squaresize[i]>squaresize[j]) ) {
	    j = i;
	  } else if(squaresize[i]==squaresize[j]) {
	    if(nsquares[j]==1) j = i;
	  }
	}
      }
 
      /* This can fail if we run out of prime factors in the dimensions */
      if(j<0) {
	QMP_error("Not enough prime factors for QMP_layout_grid\n");
	status = QMP_ERROR;
	goto leave;
      }

      /* do the surgery */
      n /= prime[k];
      squaresize[j] /= prime[k];
      nsquares[j] *= prime[k];
    }

    /* now set logical topology */
    status = QMP_declare_logical_topology(nsquares, ndim);
    if (status != QMP_SUCCESS) {
      QMP_error("QMP_layout_grid: error creating logical topology\n");
      goto leave;
    }

  } else {  /* Logical topology is already declared */

    if(ndim < QMP_get_logical_number_of_dimensions()) {
      QMP_error("grid dimension is less than logical dimension\n");
      status = QMP_ERROR;
      goto leave;
    }

    /* copy logical size and pad with ones */
    for(i=0; i<QMP_get_logical_number_of_dimensions(); i++) {
      nsquares[i] = QMP_get_logical_dimensions()[i];
    }
    for( ; i<ndim; i++) nsquares[i] = 1;

    /* calculate subgrid size */
    for(i=0; i<ndim; i++) {
      if(dims[i]%nsquares[i] != 0) {
	QMP_error("grid size does not fit on logical topology\n");
	status = QMP_ERROR;
	goto leave;
      }
      squaresize[i] = dims[i]/nsquares[i];
    }

  }

  /* now we should have the layout done so we just set the results */
  QMP_alloc(subgrid.length, int, ndim);

  subgrid.vol = 1;
  for(i=0; i<ndim; i++) {
    subgrid.length[i] = squaresize[i];
    subgrid.vol *= squaresize[i];
  }

 leave:
  QMP_free(squaresize);
  QMP_free(nsquares);
  LEAVE;
  return status;
}

/* Return the logical subgrid size on each node */
const int *
QMP_get_subgrid_dimensions (void)
{
  ENTER;
  LEAVE;
  return (const int *) subgrid.length;
}

/* Return the logical subgrid number of sites */
int
QMP_get_number_of_subgrid_sites (void)
{
  ENTER;
  LEAVE;
  return subgrid.vol;
}
