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
 *   $Log: not supported by cvs2svn $
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
#include "QMP.h"

/**
 * Currently the following two functions are implemented with different
 * names. We may have to change these.
 *
 * QMP API logical grid specifies physical node layout.
 */
extern QMP_bool_t QMP_declare_ordered_logical_topology (const QMP_u32_t *dims, 
							QMP_u32_t ndim, 
							QMP_u32_t ordering[]);

#undef DEBUG

#define ND  4

typedef struct QMP_geometry
{
  /* Dimension of problem geometry */
  unsigned dimension;

  /* Dimension of geometry used for PAR */
  unsigned PAR_dim;

  /* Number of virtual processors to physical processors */
  unsigned subgrid_vol;

  /* Virtual lattice size */
  unsigned length[ND];

  /* Subgrid length each logical (rotated) direction */
  unsigned subgrid_length[ND];

  /* Total number of words on face for each direction */
  unsigned count[ND];

  /* Number of blocks on face for each direction */
  unsigned numblk[ND];

  /* Block size on each face for each direction */
  unsigned blksize[ND];

} *QMP_geometry_t;



/* The virtual geometry */
static struct QMP_geometry  PAR_geometry;
static QMP_geometry_t geom = &PAR_geometry;

/* Return the logical subgrid size on each node */
const QMP_u32_t *
QMP_get_subgrid_dimensions (void)
{
  return geom->subgrid_length;
}

/* Return the logical subgrid number of sites */
QMP_u32_t
QMP_get_number_of_subgrid_sites (void)
{
  return geom->subgrid_vol;
}

/* Check if a is a factor of b */
static unsigned 
is_factor(unsigned a, unsigned b)
{
  return ((b % a) == 0 ? 1 : 0);
}



/* 
 * QMP_detailed_layout_grid
 *
 * Detailed geometry constructor 
 */
int
QMP_detailed_layout_grid(QMP_u32_t *length, QMP_u32_t rank, 
			 QMP_u32_t *ordering)
{
  int i;
  int logical_size[ND];

  if (rank < 1 || rank > ND)
  {
    fprintf(stderr,"QMP_detailed_layout_grid: invalid rank = %d\n", rank);
    return 1;
  }

  /* Geometry should already have been allocated. Fill it in */
  geom->dimension = rank;
  geom->subgrid_vol = 1;

  for(i=0; i < ND; ++i)
  {
    geom->length[i] = 1;
    geom->subgrid_length[i] = 1;
  }

  /* If the physical topology not declared, this machine allows configuration of size */
  if (! QMP_logical_topology_is_declared())
  {   /* This is something like a switch */
#define NUM_PRIME 4
    unsigned prime_factors[NUM_PRIME] = {2, 3, 5, 7};
    unsigned num_nodes;
    unsigned virt_size[ND];
    unsigned found_prime;
    int phys_num_nodes = QMP_get_number_of_nodes();
    int k;

    for(i=0; i < rank; ++i)
    {
      virt_size[i] = length[ordering[i]];
      logical_size[i] = 1;
    }

    /* A simple (primitive) method to determine lattice sizes */
    /* Loop over each dimension repeatedly pulling out prime factors */
    /* No attempt is made to determine if the resulting subgrid is optimal */
    /* in surface area to volume */
    num_nodes = phys_num_nodes;

    do
    {
      /* Reset prime factor count */
      found_prime = 0;

      /* Check and keep track if any prime factor found */
      for(k=0; k < NUM_PRIME; ++k)
      {
	unsigned p = prime_factors[k];

	for(i=rank-1; i >= 0; --i)
	{
	  if (is_factor(p, num_nodes))
	  {
	    if (is_factor(p, virt_size[i]))
	    {
	      logical_size[i] *= p;
	      virt_size[i] /= p;
	      num_nodes    /= p;
	      found_prime = 1;
	    }
	  }
	}
      }
    }
    while (num_nodes > 1 && found_prime);


    /* Sanity checks */
    if (num_nodes != 1)
    {
      fprintf(stderr,"QMP_detailed_layout_grid: node number not composed of selected prime factors, still %d remaining\n",
	      num_nodes);
      return 1;
    }

    num_nodes = 1;
    for(i=0; i < rank; ++i)
      num_nodes *= logical_size[i];

    if (num_nodes != phys_num_nodes)
    {
      fprintf(stderr,"QMP_detailed_layout_grid: some error matching physical problem size to number of nodes\n");
      return 1;
    }

    /* Create the logical machine geometry */
    if (QMP_declare_logical_topology(logical_size, rank) != QMP_TRUE)
    {
      fprintf(stderr,"QMP_detailed_layout_grid: error creating physical geometry\n");
      return 1;
    }

    /* Layout should be okay, proceed with subgrid assignment */
    for(i=0; i < rank; ++i)
    {
      unsigned s = logical_size[i];

      geom->length[i] = length[i];

      if (geom->length[i] % s != 0)
      {
	fprintf(stderr,"QMP_detailed_layout_grid: virtual lattice not divisible by physical lattice\n");
      }
      if (geom->length[i] < s)
      {
	fprintf(stderr,"QMP_detailed_layout_grid: virtual lattice smaller than physical lattice\n");
      }

      num_nodes *= s;
      geom->subgrid_length[i] = geom->length[i] / s;
      geom->subgrid_vol *= geom->subgrid_length[i];
    }
  }
  else /* Physical machine size is fixed already */
  {
    /* Get info about the machine configuration */
    const int *physs_size = QMP_get_logical_dimensions();

    if (QMP_get_logical_number_of_dimensions() != rank)
    {
      fprintf(stderr,"QMP_detailed_layout_grid: currently only support equal physical and problem rank\n");
      return 1;
    }

    /* If this is set, something has already ordered the axes */
    if (! QMP_logical_topology_is_declared())
    {
      fprintf(stderr,"QMP_detailed_layout_grid: did not expect logical topology declared\n");
      return 1;
    }

    for(i=0; i < rank; ++i)
    {
      unsigned s = physs_size[i];

      logical_size[i] = s;
      geom->length[i] = length[i];

      if (geom->length[i] % s != 0)
      {
	fprintf(stderr,"QMP_detailed_layout_grid: virtual lattice not divisible by physical lattice\n");
      }
      if (geom->length[i] < s)
      {
	fprintf(stderr,"QMP_detailed_layout_grid: virtual lattice smaller than physical lattice\n");
      }

      geom->subgrid_length[i] = geom->length[i] / s;
      geom->subgrid_vol *= geom->subgrid_length[i];
    }

    for(i=rank; i < ND; ++i)
    {
      unsigned a = ordering[i];
      unsigned s = logical_size[a];

      if (s > 1)
      {
	fprintf(stderr,"QMP_detailed_layout_grid: unused machine axis is nontrivial: %d\n", a);
      }
    }

    /* Create the logical machine geometry */
    /* Even though there is some physical machine layout, this will allow some
       possibly remapping of axes. */
    if (QMP_declare_ordered_logical_topology(logical_size, rank, ordering) != QMP_TRUE) {
      fprintf(stderr,"QMP_detailed_layout_grid: error creating physical geometry\n");
      return 1;
    }
  }


  /* Need some better mechanism here */
  geom->PAR_dim = rank;

  return 0;
}

/* 
 * QMP_layout_grid
 *
 * General geometry constructor. Automatically determine sthe 
 * optimal layout -> decides the optimal ordering of axes.
 */
int
QMP_layout_grid (QMP_u32_t *dims, QMP_u32_t num_dims)
{
  unsigned ordering[ND];

  if (num_dims < 1 || num_dims > ND)
  {
    fprintf(stderr,"QMP_layout_grid: invalid num_dims = %d\n", num_dims);
    exit(1);
  }

  /* If physical topology not set, this machine allows configuration of size */
  if (! QMP_logical_topology_is_declared())
  {
    unsigned m;

    for(m=0; m < ND; ++m)
      ordering[m] = m;
  }
  else /* Physical machine is fixed already */
  {
    unsigned m, nd;
    unsigned min_size, used_size[ND], used_len[ND];

    /* Get info about the machine configuration */
    const int *phys_size = QMP_get_logical_dimensions();

    min_size = 0;

    for(m=0; m < ND; ++m)
    {
      ordering[m] = 9999;
      used_len[m] = 0;
      used_size[m] = 0;
    }

    nd = num_dims;

    while(nd > 0)
    {
      unsigned ind = 0;

      /* Find the current smallest physical machine size */
      for(m=0; m < ND; ++m)
      {
	if (! used_size[m])
	{
	  min_size = m;
	  break;
	}
      }

      for(m=0; m < ND; ++m)
      {
	if ((! used_size[m]) && (phys_size[m] < phys_size[min_size]))
	  min_size = m;
      }

      /* Find the first divisible lattice length */
      for(m=0; m < num_dims; ++m)
      {
	ind = m;
	if ((! used_len[m]) && (dims[m] % phys_size[min_size]) == 0)
	  break;
      }

      if (m == num_dims)
      {
	fprintf(stderr,"QMP_layout_grid: lattice size does not fit in the machine\n");
	fprintf(stderr,"physical machine size\n");
	for(m=0; m < ND; ++m)
	  fprintf(stderr," size[%d] = %d\n",m,phys_size[m]);
	fprintf(stderr,"requested lattice size\n");
	for(m=0; m < num_dims; ++m)
	  fprintf(stderr," len[%d] = %d\n",m, dims[m]);
	exit(1);
      }

      for(m=ind; m < num_dims; ++m)
      {
	if ((! used_len[m]) && 
	    (dims[m] % phys_size[min_size] == 0) && 
	    (dims[m] < dims[ind]))
	  ind = m;
      }

      ordering[ind] = min_size;
      used_len[ind] = 1;
      used_size[min_size] = 1;
      --nd;
    }

    /* Sanity check */
    /* Reset all remaining unsed  */
    {
      for(m=0; m < ND; ++m)
	printf("phys_size[%d] = %d\n",m,phys_size[m]);

      for(m=0; m < ND; ++m)
	printf("ordering[%d] = %d\n",m,ordering[m]);
      
      for(m=0; m < ND; ++m)
	printf("used_len[%d] = %d\n",m,used_len[m]);

      for(m=0; m < ND; ++m)
	printf("used_size[%d] = %d\n",m,used_size[m]);
    }

    for(m=0; m < num_dims; ++m)
    {
      if (ordering[m] == 9999)
      {
	fprintf(stderr,"QMP_layout_grid: ordering not correct\n");
	exit(1);
      }
    }

    for(m=num_dims; m < ND; ++m)
    {
      if (ordering[m] != 9999)
      {
	fprintf(stderr,"QMP_layout_grid: ordering not correct\n");
	exit(1);
      }

      ordering[m] = m;
    }
  }

  return QMP_detailed_layout_grid(dims, num_dims, ordering);
}


/* Slow send-receive (blocking) */
void
QMP_sendrecv_wait(void *send_buf, void *recv_buf, 
		  int count, int isign, int dir)
{
  QMP_msgmem_t msg[2];
  QMP_msghandle_t mh_a[2], mh;

  msg[0]  = QMP_declare_msgmem(send_buf, count);
  msg[1]  = QMP_declare_msgmem(recv_buf, count);
  mh_a[0] = QMP_declare_send_relative(msg[0], dir, isign, 0);
  mh_a[1] = QMP_declare_receive_relative(msg[1], dir, -isign, 0);
  mh = QMP_declare_multiple(mh_a, 2);

  QMP_start(mh);
  QMP_wait(mh);

  QMP_free_msghandle(mh_a[1]);
  QMP_free_msghandle(mh_a[0]);
  QMP_free_msghandle(mh);
  QMP_free_msgmem(msg[1]);
  QMP_free_msgmem(msg[0]);
}


/* Internal op used for shifting */
static void QMP_m_site(int site,int *s_p,int *s_v)
{
  int i,k;  
  int coord[4];
  const int *logical_size = QMP_get_logical_dimensions();


  /* The lattice coordinates corresponding to this site */
  for(i=0; i<geom->dimension; ++i)
  {
    coord[i] = site % geom->length[i];
    site = site / geom->length[i];
  }

  /* The logical site and logical node */
  k = 0;
  for(i=geom->dimension-1;i>=0;i--)
    k = k*(logical_size[i]) + coord[i]/geom->subgrid_length[i];
  *s_p = k;
  k = 0;
  for(i=geom->dimension-1;i>=0;i--)
    k = k*(geom->subgrid_length[i]) + (coord[i] % geom->subgrid_length[i]);
  *s_v = k;
}


int 
QMP_shift(int site, unsigned char *data, int prim_size, int sn)
{
  static int save_old_node;
  static int cur_src[4];
  static int cur_lsite[4];
  int nd;
  const int *logical_size = QMP_get_logical_dimensions();
  
  int old_node,cur_node,i,j,shift_size,dir;
  int local_site;
  unsigned int b1;
  int send_dir = 0;
  
  nd = geom->dimension;
  
  if(site == sn)
  {
    save_old_node = 0;
    for(i=0;i<nd;i++)
      cur_lsite[i] = cur_src[i] = 0;
  }  

  QMP_m_site(site,&cur_node,&local_site);
  old_node = save_old_node;
  save_old_node = cur_node;

#ifdef DEBUG
  fprintf(stderr," logical site %d node %d local_site %d old_node %d\n",site,cur_node,local_site,old_node);
#endif

  shift_size = prim_size;
  for(dir = 0;dir < nd;dir++)
  {
    shift_size *= geom->subgrid_length[dir];
    b1 =  logical_size[dir];
    i = cur_node % b1;
    j = old_node % b1;
    cur_node = cur_node / b1;
    old_node = old_node / b1;

    if(j != i)
    { 
      /* the coordinates are different, i.e. we have to do a shift */
#ifdef DEBUG
      fprintf(stderr," diff coordinates for dir = %d new %d old %d\n",dir,i,j);
#endif

      if(((i-j+b1) % b1) != 1)
      {
	fprintf(stderr," par_shift: internal error 1\n");
	exit(1);
      }
      if(i != 0)
	cur_lsite[dir] = local_site;
    
#ifdef DEBUG
      fprintf(stderr,"shift in dir %d, offset = %d size = %d\n",
	     dir,cur_src[dir]*geom->subgrid_vol + cur_lsite[dir],shift_size);
#endif

      if (b1 > 1)
      { /* machine_size[dir] > 1 */
	QMP_sendrecv_wait(
	  (void *)(data + (cur_src[dir]*geom->subgrid_vol
			   + cur_lsite[dir])*prim_size),
	  (void *)(data + ((1-cur_src[dir])*geom->subgrid_vol
			   + cur_lsite[dir])*prim_size),
	  shift_size, send_dir, dir);
      }
      else /* machine_size[dir] = 1 */
      {
	/* Has arguments memcpy(dest, src, count) */
	memcpy((void *)(data + ((1-cur_src[dir])*geom->subgrid_vol
				+ cur_lsite[dir])*prim_size),
	       (void *)(data + (cur_src[dir]*geom->subgrid_vol
				+ cur_lsite[dir])*prim_size),
	       shift_size);
      }

      cur_src[dir] = 1 - cur_src[dir];
      for(j=0;j<dir;j++)
	cur_src[j] = cur_src[dir];
    }
  }

#ifdef DEBUG
  fprintf(stderr,"new offset for read %d\n",local_site + cur_src[0]*geom->subgrid_vol);
#endif

  return local_site + cur_src[0]*geom->subgrid_vol;
}
