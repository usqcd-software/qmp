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
#include "QMP_P_MPI.h"

static struct QMP_physical_geometry par_machine_geometry;

QMP_physical_geometry_t QMP_machine  = &par_machine_geometry;

/* whether a topology is declared or not */
static int QMP_topology_declaredP = 0;

/**
 * This machine information
 */
QMP_machine_t QMP_global_m;

/**
 * Check whether we have an empty line.
 */
static QMP_bool_t
empty_line (char line[], QMP_u32_t size)
{
  char* p;

  QMP_TRACE("empty_line");

  p = line;

  while (p && *p) {
    if (isprint(*p))
      return QMP_FALSE;
    p++;
  }
  return QMP_TRUE;
}

/**
 * Get number of CPUs for this linux box.
 */
static QMP_u32_t
get_num_cpus (void)
{
  FILE* fd;
  char  line[80];
  int   numcpu = 0;

  QMP_TRACE ("get_num_cpus");

  /*
   * GTF: Using /proc/cpuinfo is totally non-portable.  Short circuit for now
   * because QMP doesn't really use this info anyways.
   */

  return 1;

  fd = fopen ("/proc/cpuinfo", "r");
  if (!fd) {
    QMP_error ("cannot open /proc/cpuinfo file.");
    return 0;
  }

#if defined (__linux) && defined (__i386)
  while (!feof (fd)) {
    memset (line, 0, sizeof (line));
    if (fgets (line, sizeof (line) - 1, fd) &&
	!empty_line (line, sizeof (line) - 1)) {
      if (strstr (line, "processor"))
	numcpu++;
    }
  }
#endif

#if defined (__linux) && defined (__alpha)
    while (!feof (fd)) {
      memset (line, 0, sizeof (line));
      if (fgets (line, sizeof (line) - 1, fd) &&
	  !empty_line (line, sizeof (line) - 1)) {
	if (strstr (line, "cpus detected")) {
	  char token0[32], token1[32], sep;
	  if (sscanf (line,"%s %s %c %d", token0, token1, &sep, &numcpu) <4){
	    QMP_error ("/proc/cpuinfo format error.");
	    return 0;
	  }
	}
      }
    }
#endif    
    
    fclose (fd);
    return numcpu;
}

/**
 * Populate this machine information.
 */
static void
QMP_init_machine_i (void)
{
  /* get host name of this machine */
  gethostname (QMP_global_m.host, sizeof (QMP_global_m.host));

  /* get cpu number of this machine */
  QMP_global_m.num_cpus = get_num_cpus ();

#ifdef _QMP_IC_SWITCH
  QMP_global_m.ic_type = QMP_SWITCH;
#else
#error "QMP over MPICH only supports switched configuration or shared memory."
#endif

  QMP_global_m.inited = QMP_FALSE;
  QMP_global_m.collapse_smp = QMP_FALSE;
  QMP_global_m.err_code = QMP_SUCCESS;
}

/* This is called by the parent */
QMP_status_t
QMP_init_msg_passing (int* argc, char*** argv, QMP_smpaddr_type_t option)
{
  /* Basic variables containing number of nodes and which node this process is */
  int PAR_num_nodes;
  int PAR_node_rank;
  unsigned i,m;
  
  if (MPI_Init(argc, argv) != MPI_SUCCESS) 
    QMP_fatal (-1, "MPI_Init failed");
  if (MPI_Comm_size(MPI_COMM_WORLD, &PAR_num_nodes) != MPI_SUCCESS)
    QMP_fatal (-1, "MPI_Comm_size failed");
  if (MPI_Comm_rank(MPI_COMM_WORLD, &PAR_node_rank) != MPI_SUCCESS)
    QMP_fatal (-1, "MPI_Comm_rank failed");

  /* Set the dimension and default neighbors to something trivial to indicate */
  /* there is no initialized physical geometry */
  QMP_machine->num_nodes = PAR_num_nodes;
  QMP_machine->physical_nodeid = PAR_node_rank;

  QMP_machine->dimension = 0;

  for (m=0; m < ND; ++m)
  {
    QMP_machine->physical_coord[m] = 0;
    QMP_machine->physical_size[m] = 1;

    for (i=0; i < 2; ++i)
      QMP_machine->neigh[i][m] = PAR_node_rank;
  }

  QMP_topology_declaredP = 0;

  QMP_init_machine_i ();
  QMP_global_m.inited = QMP_TRUE;

  if (option == QMP_SMP_ONE_ADDRESS)
    QMP_global_m.collapse_smp = QMP_TRUE;
  else
    QMP_global_m.collapse_smp = QMP_FALSE;

  if (QMP_global_m.verbose) {
    QMP_info ("PAR_num_nodes = %d\n",PAR_num_nodes);
    QMP_info ("PAR_node_rank = %d\n",PAR_node_rank);
  }
  return QMP_SUCCESS;
}

/* Shutdown the machine */
void 
QMP_finalize_msg_passing(void)
{
  MPI_Finalize();
}

/* Abort the program */
void 
QMP_abort(QMP_s32_t error_code)
{
  MPI_Abort(MPI_COMM_WORLD, error_code);
}

/* get number of CPUS */
QMP_u32_t    
QMP_get_SMP_count (void)
{
  return QMP_global_m.num_cpus;
}

/* return message passing type */
QMP_ictype_t 
QMP_get_msg_passing_type (void)
{
  return QMP_global_m.ic_type;
}



static void 
crtesn_coord(QMP_u32_t ipos, QMP_u32_t coordf[], 
	     QMP_u32_t latt_size[], QMP_u32_t ndim)
{
  /* local */
  unsigned i;

  /* Calculate the Cartesian coordinates of the VALUE of IPOS where the 
   * value is defined by
   *
   *     for i = 0 to NDIM-1  {
   *        X_i  <- mod( IPOS, L(i) )
   *        IPOS <- int( IPOS / L(i) )
   *     }
   *
   * NOTE: here the coord(i) and IPOS have their origin at 0. 
   */
   for(i=0; i < ndim; ++i)
   {
     coordf[i] = ipos % latt_size[i];
     ipos = ipos / latt_size[i];
   }
}

static QMP_u32_t 
crtesn_pos(QMP_u32_t coordf[], QMP_u32_t latt_size[], 
	   QMP_u32_t ndim)
{
  int k;
  unsigned ipos;

  ipos = 0;
  for(k=ndim-1; k >= 0; k--)
  {
    ipos = ipos * latt_size[k] + coordf[k]; 
  }

  return ipos;
}


/* This is called by all children */
QMP_bool_t
QMP_declare_physical_topology(const QMP_u32_t *length, unsigned rank)
{
  int i;
  int num_nodes = 1;
  
  if (rank < 1 || rank > ND)
  {
    QMP_error ("QMP_declare_physical_topology: invalid rank = %d\n", rank);
    return QMP_FALSE;
  }

  QMP_machine->dimension = rank;

  for(i=0; i < rank; ++i)
  {
    if (length[i] < 1)
    {
      QMP_error ("QMP_declare_physical_topology: invalid length\n");
      return QMP_FALSE;
    }

    num_nodes *= length[i];
    QMP_machine->physical_size[i] = length[i];
  }

  if (num_nodes != QMP_machine->num_nodes)
  {
    QMP_error ("QMP_declare_physical_topology: requested machine size not equal to number of nodes\n");
    return QMP_FALSE;
  }

  crtesn_coord(QMP_machine->physical_nodeid, QMP_machine->physical_coord, QMP_machine->physical_size, rank);

  for(i=0; i < rank; ++i)
  {
    unsigned coord[ND];
    unsigned m;

    for(m=0; m < rank; ++m)
      coord[m] = QMP_machine->physical_coord[m];

    coord[i] = (QMP_machine->physical_coord[i] - 1 + length[i]) % length[i];
    QMP_machine->neigh[0][i] = 
      crtesn_pos(coord, QMP_machine->physical_size, rank);

    coord[i] = (QMP_machine->physical_coord[i] + 1) % length[i];
    QMP_machine->neigh[1][i] =
      crtesn_pos(coord, QMP_machine->physical_size, rank);
  }

  /* Construct logical node layout */
  QMP_machine->logical_nodeid = 0;
  for(i=rank-1;i>=0;i--)
  {
    unsigned a,s,c;

    /* At the very moment, hardcode the ordering */
    QMP_machine->ordering[i] = i;


    a = QMP_machine->ordering[i];
    s = QMP_machine->logical_size[a] = QMP_machine->physical_size[i];
    c = QMP_machine->logical_coord[a] = QMP_machine->physical_coord[i];
    QMP_machine->logical_nodeid = QMP_machine->logical_nodeid * s + c; 
  }

  QMP_topology_declaredP = 1;

  return QMP_TRUE;
}


/* This is called by all children */
QMP_bool_t  
QMP_declare_logical_topology (const QMP_u32_t* dims,
			      QMP_u32_t ndim)
{
  int i;

  for(i=0; i < ndim; ++i)
    QMP_machine->ordering[i] = i;
    
  return QMP_declare_physical_topology (dims, ndim);
}


QMP_bool_t
QMP_declare_ordered_logical_topology (const QMP_u32_t *length, QMP_u32_t rank, 
				      unsigned ordering[])
{
  int i;

  for(i=0; i < rank; ++i)
  {
    printf("ordering[%d] = %d\n",i,ordering[i]);
    QMP_machine->ordering[i] = i;
  }
    
  return QMP_declare_physical_topology(length, rank);
}


/* This file implements a switch architecture and does so over MPI */
QMP_bool_t
QMP_logical_topology_is_declared (void)
{
  return  QMP_topology_declaredP;
}

/* Return the physical coordinate of this node */
const QMP_u32_t *
QMP_get_allocated_coordinates(void)
{
  if (!QMP_global_m.inited) {
    QMP_error_exit ("QMP system is not initialized.");
    return 0;
  }
  
  if (QMP_global_m.ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_error_exit ("QMP supports only switched network configuration.");
    return 0;
  }
}
  
/* Return the physical size of the machine */
const QMP_u32_t *
QMP_get_allocated_dimensions(void)
{
  if (!QMP_global_m.inited) {
    QMP_error_exit ("QMP system is not initialized.");
    return 0;
  }
  if (QMP_global_m.ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_error_exit ("QMP supports only switched network configuration.");
    return 0;
  }
}
  
/* Return the physical size of the machine */
QMP_u32_t
QMP_get_allocated_number_of_dimensions(void)
{
  if (!QMP_global_m.inited) {
    QMP_error_exit ("QMP system is not initialized.");
    return 0;
  }
  if (QMP_global_m.ic_type == QMP_SWITCH)
    return 0;
  else {
    QMP_error_exit ("QMP supports only switched network configuration.");
    return 0;
  }
}
  
/* Is this the primary node in the machine? */
QMP_bool_t
QMP_is_primary_node(void)
{
  return (QMP_machine->physical_nodeid == 0) ? 1 : 0;
}

/* Return the lexicographic nodeid within the machine */
QMP_u32_t
QMP_get_node_number(void)
{
  return QMP_machine->physical_nodeid;
}

/* Return the total number of nodes within the machine */
QMP_u32_t
QMP_get_number_of_nodes(void)
{
  int num_nodes;

  if (!QMP_global_m.inited) {
    QMP_error_exit ("QMP system is not initialized.");
    return 0;
  }
  
  if (MPI_Comm_size (MPI_COMM_WORLD, &num_nodes) != MPI_SUCCESS) {
    QMP_error_exit ("QMP could not get number of nodes information.");
    return 0;
  }
  return num_nodes;
}

/* Return the neighbor id in the isign and dir direction */
QMP_u32_t
QMP_physical_neigh(int isign, int dir)
{
  int ii = (isign > 0) ? 1 : 0;

  return QMP_machine->neigh[ii][dir];
}

/* Return the coordinate of the underlying grid */
const QMP_u32_t *
QMP_get_logical_coordinates(void)
{
  return QMP_machine->logical_coord;
}

/* Return logical coordinate from a physical node id */
const QMP_u32_t *
QMP_get_logical_coordinates_from (QMP_u32_t node)
{
  QMP_u32_t logic_id;
  QMP_u32_t* nc;

  logic_id = QMP_allocated_to_logical (node);

  nc = (QMP_u32_t *)malloc (QMP_machine->dimension);

  crtesn_coord (logic_id, nc, QMP_machine->logical_size,
		QMP_machine->dimension);

  return nc;
}


/* Return the logical size of the machine */
const QMP_u32_t *
QMP_get_logical_dimensions(void)
{
  return QMP_machine->logical_size;
}
  
/* Return the lexicographic nodeid within the machine */
QMP_u32_t
QMP_get_logical_node_number(void)
{
  if (QMP_global_m.ic_type == QMP_SWITCH) {
    if (QMP_machine->physical_nodeid != QMP_machine->logical_nodeid) {
      QMP_error_exit ("Logical id and physical id should be the same for switched configuration.");
      return 0;
    }
  } 
  return QMP_machine->logical_nodeid;
}

/* Return the physical size of the machine */
QMP_u32_t
QMP_get_logical_number_of_dimensions(void)
{
  return QMP_machine->dimension;
}

QMP_u32_t 
QMP_get_logical_number_of_nodes (void)
{
  return QMP_machine->num_nodes;
}

/**
 * Returns logical node number from a physical node number.
 */
QMP_u32_t
QMP_allocated_to_logical (QMP_u32_t node)
{
  if (QMP_global_m.ic_type == QMP_SWITCH)
    return node;
  else {
    QMP_error_exit ("QMP_allocated_to_logical is not yet implemented.");
    return 0;
  }
}

/**
 * Return a physical node number from a logical node number.
 */
QMP_u32_t
QMP_logical_to_allocated (QMP_u32_t logic_rank)
{
  if (QMP_global_m.ic_type == QMP_SWITCH)
    return logic_rank;
  else {
    QMP_error_exit ("QMP_logical_to_allocated is not yet implemented.");
    return 0;
  }
}


/**
 * Return a physical node number from a logical coordinate.
 */
QMP_u32_t 
QMP_get_node_number_from (const QMP_u32_t* coordinates)
{
  QMP_u32_t logic_node;

  logic_node = crtesn_pos ((QMP_u32_t *)coordinates, QMP_machine->logical_size,
			   QMP_machine->dimension);

  return QMP_allocated_to_logical (logic_node);
}
  
