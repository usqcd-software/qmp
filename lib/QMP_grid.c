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
 *      MPMPI: Level 1 Message Passing API routines implemented over MPI
 *
 *      The basic structure is from highest to lowest 
 *           Virtual
 *           Logical
 *           Physical
 *
 * This file implements the Virtual level. 
 *
 * Author:  
 *      Robert Edwards
 *      Jefferson Lab
 *
 * Revision History:
 *   $Log: QMP_grid.c,v $
 *   Revision 1.6  2006/06/13 17:43:09  bjoo
 *   Removed some c99isms. Code  compiles on Cray at ORNL using pgcc
 *
 *   Revision 1.5  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.4  2005/06/21 20:18:39  osborn
 *   Added -qmp-geom command line argument to force grid-like behavior.
 *
 *   Revision 1.3  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.2  2004/12/16 02:44:12  osborn
 *   Changed QMP_mem_t structure, fixed strided memory and added test.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.7  2004/09/04 03:33:29  edwards
 *   Changed QMP_layout_grid to take a const qualifier on args.
 *
 *   Revision 1.6  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.5  2004/02/05 02:28:08  edwards
 *   Blanked out debugging section.
 *
 *   Revision 1.4  2003/07/21 02:19:19  edwards
 *   Cleaned up some int* to unsigned int*.
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
 *   Revision 1.1  2002/04/22 20:28:41  chen
 *   Version 0.95 Release
 *
 *
 *
 */
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
  /* int squaresize[ndim], nsquares[ndim]; */
  int i;
  QMP_status_t status = QMP_SUCCESS;
  int *squaresize;
  int *nsquares;
  ENTER;

  squaresize=(int *)malloc(ndim * sizeof(int));
  nsquares  =(int *)malloc(ndim * sizeof(int));
  if(nsquares == NULL || squaresize == NULL) {
     QMP_FATAL("Unable to malloc in QMP_layout_grid");
  }

  if(QMP_global_m->ic_type!=QMP_SWITCH) {
    QMP_declare_logical_topology(QMP_global_m->geom, QMP_global_m->ndim);
  }

  /* If logical topology not set, this machine allows configuration of size */
  if (! QMP_logical_topology_is_declared()) {
    int j, k, n;

    /* Initialize subgrid and machine size */
    for(i=0; i<ndim; ++i) {
      squaresize[i] = dims[i];
      nsquares[i] = 1;
    }

    n = QMP_global_m->num_nodes;
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

    if(ndim < QMP_topo->dimension) {
      QMP_error("grid dimension is less than logical dimension\n");
      status = QMP_ERROR;
      goto leave;
    }

    /* copy logical size and pad with ones */
    for(i=0; i<QMP_topo->dimension; i++) {
      nsquares[i] = QMP_topo->logical_size[i];
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
  subgrid.length = (int *) malloc(ndim*sizeof(int));

  subgrid.vol = 1;
  for(i=0; i<ndim; i++) {
    subgrid.length[i] = squaresize[i];
    subgrid.vol *= squaresize[i];
  }

 leave:
  free(squaresize);
  free(nsquares);
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
