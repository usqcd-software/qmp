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
 *      Logical Topology for LQCD Messaging System
 *
 * Author:  
 *      Jie Chen, Chip Watson and Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:36  chen
 *   check into lattice group
 *
 *   Revision 1.7  2002/07/18 18:10:24  chen
 *   Fix broadcasting bug and add several public functions
 *
 *   Revision 1.6  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.5  2002/04/22 20:28:44  chen
 *   Version 0.95 Release
 *
 *   Revision 1.4  2002/03/28 18:48:20  chen
 *   ready for multiple implementation within a single source tree
 *
 *   Revision 1.3  2002/02/15 20:34:55  chen
 *   First Beta Release QMP
 *
 *   Revision 1.2  2002/01/27 20:53:51  chen
 *   minor change
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:50  chen
 *   initial import of QMP
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "qmp.h"
#include "QMP_P_GM.h"

/**
 * Internal routine to convert logical node number to physical node number.
 */
static const QMP_u32_t    
qmp_logical_to_allocated_i (QMP_machine_t* glm, QMP_u32_t logic_rank);

/**
 * String forms of neighbors
 */
static char* QMP_neighbors[] = {"DIRXP",
				"DIRXM",
				"DIRYP",
				"DIRYM",
				"DIRZP",
				"DIRZM",
				"DIRTP",
				"DIRTM",
				"DIRUNKNOWN"
};

/**
 * Create and initialize all neighbors structure inside a topology.
 *
 * This routine is called after dimension and size information are done.
 *
 * All neighbors are stored in an array of pointers that points
 * to a 2 element array (0: neighbor on the positive side, 1: negative side)
 *
 * Here are the illustration: [0]--->[DIRXP][DIRXM]
 *                            [1]--->[DIRYP][DIRYM]
 *                             ..................
 */
static void
init_logic_neighbors (QMP_topology_t* tpl)
{
  int i;
  
  QMP_TRACE ("init_logic_neighbors");
  for (i = 0; i < tpl->dimension; i++) {
    tpl->neighbors[i][0].phys = 0;
    tpl->neighbors[i][1].phys = 0;

    /**
     * The following only works because we define direction 
     * from DIRXP, DIRXM.... from 0 to 8.
     */    
    tpl->neighbors[i][0].type = i * 2;
    tpl->neighbors[i][1].type = i * 2 + 1;
  }
}

/**
 * Free memory associated with all neighbors.
 * 
 * Free neighbors structure inside a topology.
 */
static void
clean_logic_neighbors (QMP_topology_t* tpl)
{
  int i;

  QMP_TRACE ("clean_logic_neighbors");

  for (i = 0; i < tpl->dimension; i++) {
    /**
     * First check whether physical geometry structure is allocated.
     * if yes, we do free memory of the physical geometry structure.
     */
    if (tpl->neighbors[i][0].phys) {
      free (tpl->neighbors[i][0].phys);
      tpl->neighbors[i][0].phys = 0;
    }
    if (tpl->neighbors[i][1].phys) {
      free (tpl->neighbors[i][1].phys);
      tpl->neighbors[i][1].phys = 0;
    }

    tpl->neighbors[i][0].type = QMP_UNKNOWN;
    tpl->neighbors[i][1].type = QMP_UNKNOWN;

    tpl->neighbors[i][0].logic_rank = 0;
    tpl->neighbors[i][1].logic_rank = 0;
  }
}


/**
 * Calculate this processs logic coordinates in a particular topology
 * 
 */
static void
calculate_logic_coordinates (QMP_topology_t* tpl)
{
  int       i;
  QMP_u32_t pos;

  QMP_TRACE ("calculate_logic_coordinates");

  /* Calculate the Cartesian coordinates of the VALUE of IPOS where the 
   * value is defined by
   *
   *     for i = 0 to NDIM-1  {
   *        X_i  <- mod( IPOS, L(i) )
   *        IPOS <- int( IPOS / L(i) )
   *     }
   *
   * NOTE: here the coord(i) and IPOS have their origin at 0. 
   * for 3 dimension, pos = z * Ny * Nx + y * Nx + x;
   */
  pos = tpl->logic_rank;

  for (i = 0; i < tpl->dimension; i++) {
    tpl->coordinates[i] = pos % tpl->size[i];
    pos = pos / tpl->size[i];
  }

}


/**
 * Calculate logic coordinates (allocated already) with a given logic
 * node number (rank), dimensionality and size of the grid.
 */
static void
calculate_logic_coordinates_from_rank (QMP_u32_t logic_rank,
				       QMP_u32_t dimension,
				       QMP_u32_t* size,
				       QMP_u32_t coordinates[])

{
  int       i;
  QMP_u32_t pos;

  QMP_TRACE ("calculate_logic_coordinates_from_rank");

  /* Calculate the Cartesian coordinates of the VALUE of IPOS where the 
   * value is defined by
   *
   *     for i = 0 to NDIM-1  {
   *        X_i  <- mod( IPOS, L(i) )
   *        IPOS <- int( IPOS / L(i) )
   *     }
   *
   * NOTE: here the coord(i) and IPOS have their origin at 0. 
   * for 3 dimension, pos = z * Ny * Nx + y * Nx + x;
   */
  pos = logic_rank;

  for (i = 0; i < dimension; i++) {
    coordinates[i] = pos % size[i];
    pos = pos / size[i];
  }

}

/**
 * Get logical neighbor's coordinates.
 *
 * arguments:  direction: 0 positive direction neighbor
 *                        1 negative direction neighbor
 *             dim_index:   which dimension
 *             dimension:   number of dimensions
 *             my_coordinates: coordinates of this process.
 */
static QMP_u32_t *
get_logic_neighbor_coordinates (int direction, 
				int dim_index,
				QMP_u32_t dimension,
				QMP_u32_t* size,
				QMP_u32_t* my_coordinates)
{
  int i;
  QMP_u32_t* nc;

  QMP_TRACE ("get_logic_neighbor_coordinates");

  nc = (QMP_u32_t *)malloc(dimension*sizeof(QMP_u32_t));
  if (!nc) {
    QMP_error ("cannot allocate coordinates for a neighbor.");
    return 0;
  }

  for (i = 0; i < dimension; i++)
    nc[i] = my_coordinates[i];

  /* positive direction */
  if (direction == 0) {
    if (nc[dim_index] == size[dim_index] - 1)
      nc[dim_index] = 0;
    else
      nc[dim_index] = nc[dim_index] + 1;
  }
  else { 
    /* negative direction */
    if (nc[dim_index] == 0)
      nc[dim_index] = size[dim_index] - 1;
    else
      nc[dim_index] = nc[dim_index] - 1;
  }

  return nc;
}

/**
 * Get physical geometry information for a node that has a coordinates
 * given in the argument list.
 *
 * return a physical geometry and logic rank of a neighbor.
 */
static QMP_phys_geometry_t *
get_info_for_neighbor (QMP_topology_t* tpl,
		       QMP_u32_t* coordinates,
		       QMP_u32_t* rem_logic_rank)
{
  int i;
  QMP_u32_t rank;
  QMP_phys_geometry_t* phys;

  QMP_TRACE ("get_info_for_neighbor");

  if (tpl->phys->type != QMP_SWITCH) {
    QMP_error ("QMP only supports switched network configuration.");
    exit (1);
  }

  /* get logic rank of the neighbor process */
  rank = 0;
  for (i = tpl->dimension - 1; i >= 0; i--) 
    rank = rank * tpl->size[i] + coordinates[i];
  *rem_logic_rank = rank;

  phys = (QMP_phys_geometry_t *)malloc(sizeof (QMP_phys_geometry_t));
  if (!phys) {
    QMP_error ("cannot allocate memory for a neighbor's physical geometry/");
    return 0;
  }
  phys->type = tpl->phys->type;
  phys->num_nodes = tpl->phys->num_nodes;
  
  /**
   * The following is for switched configuration: 
   * Logic Rank == Physical Rank
   */
  QMP_PHYS_RANK(phys) = rank;

  return phys;
}

/**
 * Create a physical geometry from a logical node number (rank)
 */
static QMP_phys_geometry_t *
qmp_physical_from_logical_i (QMP_machine_t* glm, QMP_u32_t logic_rank)
{
  QMP_phys_geometry_t* phys;
  QMP_topology_t* tpl;

  QMP_TRACE ("qmp_physical_from_logical_i");

  tpl = glm->tpl;
  if (tpl->phys->type != QMP_SWITCH) {
    QMP_error ("QMP only supports switched network configuration.");
    exit (1);
  }

  phys = (QMP_phys_geometry_t *)malloc(sizeof (QMP_phys_geometry_t));
  if (!phys) {
    QMP_error ("cannot allocate memory for a neighbor's physical geometry/");
    return 0;
  }
  phys->type = tpl->phys->type;
  phys->num_nodes = tpl->phys->num_nodes;
  
  /**
   * The following is for switched configuration: 
   * Logic Rank == Physical Rank
   */
  QMP_PHYS_RANK(phys) = logic_rank;

  return phys;
}


/**
 * Create a physical geometry from a logical node number (rank)
 *
 * This is a public function.
 */
QMP_phys_geometry_t *
QMP_physical_from_logical (QMP_u32_t logic_rank)
{
  return qmp_physical_from_logical_i (&QMP_global_m, logic_rank);
}

/**
 * Get logical neighbors of this process.
 *
 * This routine will fill all neighbors structure inside a topology.
 */
static QMP_bool_t
get_logic_neighbors (QMP_topology_t* tpl)
{
  int i, j;

  QMP_TRACE ("get_logic_neighbors");

  /* initialize all values for all neighbors */
  init_logic_neighbors (tpl);

  /* find neighbors for each direction */
  for (i = 0; i < tpl->dimension; i++) {
    QMP_u32_t* ncp = 0;
    QMP_u32_t* ncm = 0;
    
    /**
     * Get coordinates for plus direction neighbor
     */
    ncp = get_logic_neighbor_coordinates (0, i, tpl->dimension, tpl->size,
					  tpl->coordinates);
    if (!ncp) {
      clean_logic_neighbors (tpl);
      return QMP_FALSE;
    }

    /**
     * Get coordinates for minus direction neighbor
     */
    ncm = get_logic_neighbor_coordinates (1, i, tpl->dimension, tpl->size,
					  tpl->coordinates);

    if (!ncm) {
      clean_logic_neighbors (tpl);
      free (ncp);
      return QMP_FALSE;
    }

    if (QMP_rt_verbose == QMP_TRUE) {
      printf ("%s neighbors has coordinate (", QMP_neighbors[2*i]);
      for (j = 0; j < tpl->dimension; j++)
	printf ("%d ", ncp[j]);
      printf (")\n");

      printf ("%s neighbors has coordinate (", QMP_neighbors[2*i + 1]);
      for (j = 0; j < tpl->dimension; j++)
	printf ("%d ", ncm[j]);
      printf (")\n");
    }

  
    /**
     * Get physical geometry information for neighbors.
     *
     * This pointer is pointing to a new allocated memory.
     */
    tpl->neighbors[i][0].phys = get_info_for_neighbor (tpl, ncp,
				    &tpl->neighbors[i][0].logic_rank);
    if (!tpl->neighbors[i][0].phys) {
      clean_logic_neighbors (tpl);
      free (ncp);
      free (ncm);
      return QMP_FALSE;
    }


    tpl->neighbors[i][1].phys = get_info_for_neighbor (tpl, ncm,
				    &tpl->neighbors[i][1].logic_rank);
    if (!tpl->neighbors[i][1].phys) {
      clean_logic_neighbors (tpl);
      free (ncp);
      free (ncm);
      return QMP_FALSE;
    }

    /**
     * Free memory for ncp and ncm.
     */
    free (ncp);
    free (ncm);
  }
  return QMP_TRUE;
}


/**
 * Initialize a topology structure with a grid dimension, size of the grid,
 * and physical geometry information of this node.
 */
static QMP_status_t 
create_topology (const QMP_u32_t* size,
		 const QMP_u32_t  dimension,
		 QMP_phys_geometry_t* phys,
		 QMP_topology_t** tptr)
{
  int             i;
  QMP_topology_t* ret;

  QMP_TRACE ("create_topology");

  if (phys->type != QMP_SWITCH) {
    QMP_error ("QMP only supports switched network configuration.");
    return QMP_NOTSUPPORTED;
  }

  if (dimension == 0) {
    QMP_error ("cannot create a logic grid topology with 0 dimension.");
    return QMP_INVALID_ARG;
  }

  ret =(QMP_topology_t *)malloc(sizeof(QMP_topology_t));
  if (!ret) {
    QMP_error ("cannot allocate memory for topology structure.");
    return QMP_NOMEM_ERR;
  }

  /**
   * assign pointer of phys geometry to the pointer
   * of physcal geometry which is a pointer inside of a global
   * data object. No duplicate done here
   */
  ret->phys = phys;
  
  /**
   * for simple switched network configuration, we use physical rank
   * as logical rank.
   */
  ret->logic_rank = QMP_PHYS_RANK(phys);
  ret->dimension = dimension;

  /**
   * calculate total number of nodes required for this logical
   * configuration.
   */
  ret->num_nodes = 1;
  for (i = 0; i < dimension; i++) {
    ret->size[i] = size[i];
    ret->num_nodes = ret->num_nodes * size[i];
  }

  if (ret->logic_rank >= ret->num_nodes) {
    QMP_error ("rank is larger than total number of nodes.");
    free (ret);
    return QMP_INVALID_TOPOLOGY;
  }

  /* calculate coordinates from rank and other information */
  calculate_logic_coordinates (ret);

  /* At the very moment, hardcode the ordering */
  for (i = 0; i < ret->dimension; i++)
    ret->ordering[i] = i;

  /* get all neighbors */
  if (get_logic_neighbors (ret) == QMP_FALSE) {
    free (ret);    
    return QMP_NONEIGHBOR_INFO;
  }

  *tptr = ret;

  /* return this topology pointer */
  return QMP_SUCCESS;
}

/**
 * Free memory associated with a topology.
 */
void
QMP_delete_topology (QMP_topology_t* t)
{
  QMP_TRACE ("QMP_delete_topology");

  if (t) {
    /* clean neighbors */
    clean_logic_neighbors (t);

    /* free topology */
    free (t);
  }
}

/**
 * Create a new logical grid topology with grid dimension and grid size
 */
static QMP_bool_t    
qmp_declare_logical_topology_i (QMP_machine_t* glm,
				const QMP_u32_t* dims,
				QMP_u32_t ndim)
{
  QMP_topology_t* tpl;
  QMP_status_t    status;

  QMP_TRACE ("qmp_declare_logical_topology_i");

  if (glm->inited == QMP_FALSE) {
    QMP_error ( "QMP has not been initialized.");
    QMP_SET_STATUS_CODE(glm, QMP_NOT_INITED);
    return QMP_FALSE;
  }

  /* If a topology is created, we cannot create a different one. */
  tpl = glm->tpl;
  if (tpl) {
    QMP_error ("a logical topology exists already.");
    QMP_SET_STATUS_CODE(glm, QMP_TOPOLOGY_EXISTS);
    return QMP_FALSE;
  }

  /* create a topology and use the physical geometry information */
  tpl = 0;
  status = create_topology (dims, ndim, glm->phys, &tpl);
  if (status != QMP_SUCCESS) {
    QMP_SET_STATUS_CODE(glm, status);
    return QMP_FALSE;
  }

  /* if total physical machines does not equal to the number of 
   * nodes requested by this logical topology. we return false.
   */
  if (QMP_NUM_NODES(glm->phys) != tpl->num_nodes) {
    QMP_error ("mismatch between the number of allocated physical machines and number of machines requesed by the logical topology.");
    QMP_delete_topology (tpl);
    glm->tpl = 0;
    QMP_SET_STATUS_CODE(glm, QMP_INVALID_TOPOLOGY);
    return QMP_FALSE;
  }
  
  /**
   * finally assign pointer of global tpl to this local tpl.
   */
  glm->tpl = tpl;

  if (QMP_rt_verbose)
    QMP_print_topology (tpl);

  QMP_SET_STATUS_CODE(glm, QMP_SUCCESS);
  return QMP_TRUE;
}


/**
 * Create a new logical grid topology with grid dimension and grid size
 *
 * This is a public function.
 */
QMP_bool_t    
QMP_declare_logical_topology (const QMP_u32_t* dims,
			      QMP_u32_t ndim)
{
  return qmp_declare_logical_topology_i (&QMP_global_m, dims, ndim);
}

QMP_bool_t
QMP_declare_ordered_logical_topology (const QMP_u32_t *dims, 
				      QMP_u32_t ndim, 
				      QMP_u32_t ordering[])
{
  int i;
  QMP_status_t   status;
  QMP_machine_t* glm;

  QMP_TRACE ("QMP_declare_ordered_logical_topology");

  glm = &QMP_global_m;
  if (glm->inited == QMP_FALSE) {
    QMP_error ( "QMP has not been initialized.");
    QMP_SET_STATUS_CODE(glm, QMP_NOT_INITED);
    return QMP_FALSE;
  }  

  /* If a topology is not declared, do it */
  if (!glm->tpl) {
    status = qmp_declare_logical_topology_i (glm, dims, ndim);
    if (status != QMP_TRUE)
      return status;
  }
    
  for(i=0; i < ndim; ++i) {
#ifdef _QMP_DEBUG
    QMP_info ("ordering[%d] = %d\n", i, ordering[i]);
#endif
    glm->tpl->ordering[i] = i;
  }
    
  return QMP_TRUE;
}


/**
 * Check whether a topology is declared or not.
 */
static QMP_bool_t
qmp_logical_topology_is_declared_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_logical_topology_is_declared_i");

  if (glm->tpl)
    return QMP_TRUE;
  return QMP_FALSE;
}


/**
 * Check whether a topology is declared or not.
 */
QMP_bool_t
QMP_logical_topology_is_declared (void)
{
  return qmp_logical_topology_is_declared_i (&QMP_global_m);
}

/**
 * Return dimensionality of the logical topology. If there is no
 * logical topology,  return physical topology information.
 */
static const QMP_u32_t 
qmp_get_logical_number_of_dimensions_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_number_of_logical_dimensions_i");

  if (glm->inited && glm->tpl)
    return glm->tpl->dimension;
  else
    return QMP_get_allocated_number_of_dimensions ();
}

/**
 * Return dimensionality of the logical topology. If there is no
 * logical topology,  return physical topology information.
 *
 * This is a public function.
 */
QMP_u32_t 
QMP_get_logical_number_of_dimensions (void)
{
  return qmp_get_logical_number_of_dimensions_i (&QMP_global_m);
}

/**
 * Return dimension size of the logical topology.
 *
 * If there is no logical topology, return information from
 * physical geometry.
 */
static const QMP_u32_t*  
qmp_get_logical_dimensions_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_logical_dimensions_i");

  if (glm->inited && glm->tpl)
    return glm->tpl->size;
  else
    return QMP_get_allocated_dimensions ();
}


/**
 * Return dimension size of the logical topology.
 *
 * If there is no logical topology, return information from
 * physical geometry.
 */
const QMP_u32_t*  
QMP_get_logical_dimensions (void)
{
  return qmp_get_logical_dimensions_i (&QMP_global_m);
}

/**
 * Return logical node number of this machine, where first logical
 * dimension increases most rapidly.
 */
const QMP_u32_t    
qmp_get_logical_node_number_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_logical_node_number_i");

  if (glm->inited && glm->tpl)
    return glm->tpl->logic_rank;
  else if (glm->inited)
    return QMP_PHYS_RANK(glm->phys);
  else {
    QMP_error ("no logical and physical node number available yet.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}

/**
 * Return logical node number of this machine, where first logical
 * dimension increases most rapidly.
 *
 * This is a public function.
 */
QMP_u32_t    
QMP_get_logical_node_number (void)
{
  return qmp_get_logical_node_number_i (&QMP_global_m);
}


/**
 * Return physical node number of this machine
 *
 * This is a public function.
 */
QMP_u32_t    
QMP_get_node_number (void)
{
  QMP_machine_t* glm = &QMP_global_m;

  QMP_TRACE ("qmp_get_node_number");

  if (glm->inited)
    return QMP_PHYS_RANK(glm->phys);
  else {
    QMP_error ("no node number available yet.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}


/**
 * Get coordinates within the logical topology.
 * If no logical topology declared, return information from physical
 * geometry.
 */
static const QMP_u32_t*   
qmp_get_logical_coordinates_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_logical_coordinates_i");

  if (glm->inited && glm->tpl)
    return glm->tpl->coordinates;
  else
    return QMP_get_allocated_coordinates ();
}


/**
 * Get coordinates within the logical topology.
 * If no logical topology declared, return information from physical
 * geometry.
 *
 * This is a public function.
 */
const QMP_u32_t*   
QMP_get_logical_coordinates (void)
{
  return qmp_get_logical_coordinates_i (&QMP_global_m);
}


/**
 * Returns logical node number from a physical node number.
 * In a switched environment, these two are the same.
 */
static const QMP_u32_t
qmp_allocated_to_logical_i (QMP_machine_t* glm, QMP_u32_t node)
{
  QMP_TRACE ("qmp_allocated_to_logical_i");

  if (!glm->inited || !glm->tpl) {
    QMP_error ("QMP has no logical topology information yet.");
    exit (1);
  }

  if (glm->phys->type == QMP_SWITCH) {
    if (node >= glm->tpl->num_nodes ||
	node >= QMP_NUM_NODES (glm->phys)) {
      QMP_error ("cannot map physical node %d to a logical node, too big", 
		 node);
      exit (1);
    }
    return node;
  }
  else {
    QMP_error ("QMP supports switched configuration only. No mapping from allocated to logical available yet.");
    exit (1);
    /* make compiler happy */
    return 0;
  }
}


/**
 * Returns logical node number from a physical node number.
 * In a switched environment, these two are the same.
 *
 * This is a public function.
 */
QMP_u32_t
QMP_allocated_to_logical (QMP_u32_t node)
{
  return qmp_allocated_to_logical_i (&QMP_global_m, node);
}

/**
 * Internal function to get logical coordinate for a node.
 */
const QMP_u32_t*   
qmp_get_logical_coordinates_from_i (QMP_machine_t* glm, QMP_u32_t node)
{
  QMP_topology_t* tpl;
  QMP_u32_t logic_id;
  QMP_u32_t* nc;

  QMP_TRACE ("qmp_get_logical_coordinates_from_i");

  if (glm->inited == QMP_FALSE) {
    QMP_error ( "QMP has not been initialized.");
    QMP_SET_STATUS_CODE(glm, QMP_NOT_INITED);
    return 0;
  }

  /* If a topology is created, we cannot create a different one. */
  tpl = glm->tpl;
  if (!tpl) {
    QMP_error ("No logical topology exists.");
    QMP_SET_STATUS_CODE(glm, QMP_NOT_INITED);
    return 0;
  }
  
  /* convert physical id to a logical id */
  logic_id = qmp_allocated_to_logical_i (glm, node);

  /* allocate memory for coordinate */
  nc = (QMP_u32_t *)malloc(tpl->dimension*sizeof (QMP_u32_t));

  calculate_logic_coordinates_from_rank(logic_id,
					tpl->dimension,
					tpl->size,
					nc);

  return nc;
}


/**
 * Get a logical coordinate from a physical node number
 * If no logical topology declared, return information from physical
 * geometry.
 *
 * This is a public function.
 */
const QMP_u32_t*   
QMP_get_logical_coordinates_from (QMP_u32_t node)
{
  return qmp_get_logical_coordinates_from_i (&QMP_global_m, node);
}

/**
 * Get logical node number from relative direction.
 */
static const QMP_u32_t
qmp_get_logical_node_number_for_neighbor_i (QMP_machine_t* glm,
					    QMP_dir_t direction)
{
  int i;
  QMP_u32_t rank;
  QMP_topology_t* tpl;
  QMP_u32_t* nc;

  QMP_TRACE ("qmp_get_node_number_for_neighbor_i");

  if (!glm->inited || !glm->tpl) {
    QMP_error ("QMP has no logical topology information yet.");
    exit (1);
  }

  tpl = glm->tpl;

  /**
   * Get coordinates for a neighbor in direction 'direction'
   */
  nc = 0;
  i = direction / 2;
  if (direction % 2 == 0) 
    /* plus side direction */
    nc = get_logic_neighbor_coordinates (0, i, tpl->dimension, tpl->size,
					 tpl->coordinates);
  else
    /* negative side direction */
    nc = get_logic_neighbor_coordinates (1, i, tpl->dimension, tpl->size,
					 tpl->coordinates);
    
  /* get logic rank of the neighbor process */
  rank = 0;
  for (i = tpl->dimension - 1; i >= 0; i--) 
    rank = rank * tpl->size[i] + nc[i]; 

  /* free memory for neighbor's coordinates */
  free (nc);

  return rank;
}


/**
 * Get logical node number from relative direction.
 *
 * This is a public function.
 */
QMP_u32_t
QMP_get_logical_node_number_for_neighbor (QMP_dir_t direction)
{
  return qmp_get_logical_node_number_for_neighbor_i (&QMP_global_m,
						     direction);
}

/**
 * Get the logical node number from its coordinates inside the logical
 * topology.
 */
static const QMP_u32_t
qmp_get_logical_node_number_from_i (QMP_machine_t* glm,
				    const QMP_u32_t* coordinates)
{
  int i;
  QMP_u32_t rank;

  QMP_TRACE ("qmp_get_logical_node_number_from_i");

  if (!glm->inited || !glm->tpl) {
    QMP_error ("QMP has no logical topology information yet.");
    exit (1);
  }
    
  /* get logic rank of the neighbor process */
  rank = 0;
  for (i = glm->tpl->dimension - 1; i >= 0; i--) 
    rank = rank * glm->tpl->size[i] + coordinates[i]; 
  
  return rank;
}

/**
 * Get the logical node number from its coordinates inside the logical
 * topology.
 *
 * This is a public function.
 */
const QMP_u32_t
QMP_get_logical_node_number_from (const QMP_u32_t* coordinates)
{
  return qmp_get_logical_node_number_from_i(&QMP_global_m,
					    coordinates);
}


/**
 * Get the physical node number from its coordinates inside the logical
 * topology.
 *
 * This is a public function.
 */
QMP_u32_t
QMP_get_node_number_from (const QMP_u32_t* coordinates)
{
  QMP_u32_t logic_node;

  /* Get logical node number */
  logic_node = qmp_get_logical_node_number_from_i(&QMP_global_m,
						  coordinates);

  /* Convert it back to physical */
  return qmp_logical_to_allocated_i (&QMP_global_m, logic_node);
}

const QMP_u32_t 
qmp_get_logical_number_of_nodes_i (QMP_machine_t* glm)
{
  QMP_TRACE ("qmp_get_number_of_logical_nodes_i");

  if (!glm->inited) {
    QMP_error ("QMP has not been initialized yet.");
    exit (1);
  }

  if (glm->tpl) {
    if (glm->phys->type == QMP_SWITCH) 
      return glm->tpl->num_nodes;
    else {
      QMP_error ("QMP supports switched configuration only.");
      exit (1);
      /* make compiler happy */
      return 0;
    }
  }
  else 
    /* If no topology, use physical topology instead */
    return QMP_NUM_NODES (glm->phys);
}  

QMP_u32_t 
QMP_get_logical_number_of_nodes (void)
{
  return qmp_get_logical_number_of_nodes_i (&QMP_global_m);
}

/**
 * Return a physical node number from a logical node number.
 */
static const QMP_u32_t    
qmp_logical_to_allocated_i (QMP_machine_t* glm, QMP_u32_t logic_rank)
{
  QMP_TRACE ("qmp_logical_to_allocated_i");

  if (!glm->inited || !glm->tpl) {
    QMP_error ("QMP has no logical topology information yet.");
    exit (1);
  }

  if (glm->phys->type == QMP_SWITCH) {
    if (logic_rank >= QMP_NUM_NODES (glm->phys) ||
	logic_rank >= glm->tpl->num_nodes) {
      QMP_error ("cannot map logical node %d to a physical node, too big", 
		 logic_rank);
      exit (1);
    }
    return logic_rank;
  }
  else {
    QMP_error ("QMP supports switched configuration only. No mapping from allocated to logical available yet.");
    exit (1);
    /* make compiler happy */
    return 0;
  }  
}

/**
 * Return a physical node number from a logical node number.
 *
 * This is a public function.
 */
QMP_u32_t    
QMP_logical_to_allocated (QMP_u32_t logic_rank)
{
  return qmp_logical_to_allocated_i (&QMP_global_m, logic_rank);
}


/**
 * Print out this topology information.
 */
void
QMP_print_topology (QMP_topology_t* tpl)
{
  int i, j;
  QMP_u32_t nc[QMP_NMD];

  printf ("Topology has %d dimensions: \n", tpl->dimension);
  printf ("Dimension sizes are: ");
  for (i = 0; i < tpl->dimension; i++) {
    if (i != tpl->dimension - 1)
      printf ("%d x ", tpl->size[i]);
    else
      printf ("%d", tpl->size[i]);
  }
  printf ("\n");

  printf ("Number of logical nodes is %d\n", tpl->num_nodes);
  printf ("Logic rank of this node is %d\n", tpl->logic_rank);

  printf ("Coordinates is : (");
  for (i = 0; i < tpl->dimension; i++)
    printf ("%d ", tpl->coordinates[i]);
  printf (")\n");

  printf ("Ordering is : (");
  for (i = 0; i < tpl->dimension; i++)
    printf ("%d ", tpl->ordering[i]);
  printf (")\n");

  printf ("Neighbors are : \n");
  for (i = 0; i < tpl->dimension; i++) {
    printf ("%s neighbors has rank: %d and coordinate (",
	    QMP_neighbors[2*i],
	    QMP_PHYS_RANK (tpl->neighbors[i][0].phys));
    /* recalculate logical coordinates to display */
    calculate_logic_coordinates_from_rank(QMP_PHYS_RANK(tpl->neighbors[i][0].phys),
					  tpl->dimension,
					  tpl->size,
					  nc);
    for (j = 0; j < tpl->dimension; j++)
      printf ("%d ", nc[j]);
    printf (")\n");

    printf ("%s neighbors has rank: %d and coordinate (",
	    QMP_neighbors[2*i + 1],
	    QMP_PHYS_RANK (tpl->neighbors[i][1].phys));

    /* recalculate logical coordinates to display */
    calculate_logic_coordinates_from_rank(QMP_PHYS_RANK(tpl->neighbors[i][1].phys),
					  tpl->dimension,
					  tpl->size,
					  nc);
    for (j = 0; j < tpl->dimension; j++)
      printf ("%d ", nc[j]);
    printf (")\n");
  }

  printf ("This logic node is in physical geometry:\n ");
  printf ("type SWITCH number of nodes %d and physical rank %d\n",
	  tpl->phys->num_nodes, tpl->phys->geometry.switched_geometry.phys_rank);
	    
}

