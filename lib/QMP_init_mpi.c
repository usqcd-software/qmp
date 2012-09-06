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
 *   $Log: QMP_init_mpi.c,v $
 *   Revision 1.16  2008/03/06 17:28:42  osborn
 *   fixes to mapping options
 *
 *   Revision 1.15  2008/03/06 08:06:56  osborn
 *   testing
 *
 *   Revision 1.14  2008/03/06 07:54:11  osborn
 *   added -qmp-alloc-map command line argument
 *
 *   Revision 1.13  2008/03/05 17:49:29  osborn
 *   added QMP_show_geom example and prepare for adding new command line options
 *
 *   Revision 1.12  2008/01/29 02:53:21  osborn
 *   Fixed single node version.  Bumped version to 2.2.0.
 *
 *   Revision 1.11  2008/01/25 20:07:39  osborn
 *   Added BG/P personality info.  Now uses MPI_Cart_create to layout logical
 *   topology.
 *
 *   Revision 1.10  2007/12/14 23:32:00  osborn
 *   added --enable-bgp option to use BG/P personality info
 *
 *   Revision 1.9  2006/10/03 21:31:14  osborn
 *   Added "-qmp-geom native" command line option for BG/L.
 *
 *   Revision 1.8  2006/03/10 08:38:07  osborn
 *   Added timing routines.
 *
 *   Revision 1.7  2006/01/04 20:27:01  osborn
 *   Removed C99 named initializer.
 *
 *   Revision 1.6  2005/08/18 05:53:09  osborn
 *   Changed to use persistent communication requests.
 *
 *   Revision 1.5  2005/06/29 19:44:32  edwards
 *   Removed ANSI-99-isms. Now compiles under c89.
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
 *   Revision 1.8  2004/06/14 20:36:31  osborn
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
#include <ctype.h>
#include <stdarg.h>
//#ifndef __USE_UNIX98
//#define __USE_UNIX98 /* needed to get gethostname from GNU unistd.h */
//#endif
//#include <unistd.h>

#include "QMP_P_MPI.h"

/* global communicator */
MPI_Comm QMP_COMM_WORLD;

/**
 * This machine information
 */
static struct QMP_machine par_machine = QMP_MACHINE_INIT;
QMP_machine_t QMP_global_m = &par_machine;

static struct QMP_logical_topology par_logical_topology;
QMP_logical_topology_t QMP_topo = &par_logical_topology;

#ifdef HAVE_BGL

#include <rts.h>

static void
set_native_machine(void)
{
  int nd;
  BGLPersonality pers;

  rts_get_personality(&pers, sizeof(pers));
  if(BGLPersonality_virtualNodeMode(&pers)) nd = 4;
  else nd = 3;

  QMP_global_m->ndim = nd;
  QMP_global_m->geom = (int *) malloc(nd*sizeof(int));
  QMP_global_m->coord = (int *) malloc(nd*sizeof(int));
  QMP_global_m->geom[0] = pers.xSize;
  QMP_global_m->geom[1] = pers.ySize;
  QMP_global_m->geom[2] = pers.zSize;
  QMP_global_m->coord[0] = pers.xCoord;
  QMP_global_m->coord[1] = pers.yCoord;
  QMP_global_m->coord[2] = pers.zCoord;
  if(nd==4) {
    QMP_global_m->geom[3] = 2;
    QMP_global_m->coord[3] = rts_get_processor_id();
  }
}

#elif defined(HAVE_BGP)

#include <spi/kernel_interface.h>
#include <common/bgp_personality.h>
#include <common/bgp_personality_inlines.h>

static int
BGP_Personality_tSize(_BGP_Personality_t *p)
{
  int node_config = BGP_Personality_processConfig(p);
  if (node_config == _BGP_PERS_PROCESSCONFIG_VNM) return 4;
  else if (node_config == _BGP_PERS_PROCESSCONFIG_SMP) return 1;
  //else if (node_config == _BGP_PERS_PROCESSCONFIG_2x2) return 2;
  return 2;
}

static void
set_native_machine(void)
{
  int nd, nt;
  _BGP_Personality_t pers;

  Kernel_GetPersonality(&pers, sizeof(pers));
  nt = BGP_Personality_tSize(&pers);
  if(nt!=1) nd = 4;
  else nd = 3;

  QMP_global_m->ndim = nd;
  QMP_global_m->geom = (int *) malloc(nd*sizeof(int));
  QMP_global_m->coord = (int *) malloc(nd*sizeof(int));
  QMP_global_m->geom[0] = BGP_Personality_xSize(&pers);
  QMP_global_m->geom[1] = BGP_Personality_ySize(&pers);
  QMP_global_m->geom[2] = BGP_Personality_zSize(&pers);
  QMP_global_m->coord[0] = BGP_Personality_xCoord(&pers);
  QMP_global_m->coord[1] = BGP_Personality_yCoord(&pers);
  QMP_global_m->coord[2] = BGP_Personality_zCoord(&pers);
  if(nd==4) {
    QMP_global_m->geom[3] = nt;
    QMP_global_m->coord[3] = Kernel_PhysicalProcessorID();
  }
}

#elif defined(HAVE_BGQ)
/*
 * Peter Boyle, Oct 2010. Native map for BG/Q with 
 * space filling dimension folding.
 */ 
#include <firmware/include/personality.h>
#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>
#include <spi/include/mu/Descriptor.h>

MUHWI_Destination_t QMP_neigh[8];
MUHWI_Destination_t QMP_me;
int                 QMP_Labcde[5];

static const int X=0;
static const int Y=1;
static const int Z=2;
static const int T=3;
static const int A=0;
static const int B=1;
static const int C=2;
static const int D=3;
static const int E=4;

static int bgq3d=0;
static void BGQtoLogical(int * xyzt, int *Lxyzt, int *abcde, int *Labcde);
static void BGQtoLogical3d(int * xyzt, int *Lxyzt, int *abcde, int *Labcde);
static void BGQtoLogical4d(int * xyzt, int *Lxyzt, int *abcde, int *Labcde);
static void LogicaltoBGQ(int * xyzt, int *abcde, int *Labcde);

static void BGQtoLogical3d(int * xyzt, int *Lxyzt, int *abcde, int *Labcde)
{
  int Cny,Cnz;
  int Cy,Cz;

  Lxyzt[T] = 1;
  xyzt[T]  = 0;

  // Fold C into AB, E into D
  if ( Labcde[C] == 1)      {Cny=1;Cnz=1;}
  else if ( Labcde[C] == 2) {Cny=2;Cnz=1;}
  else if ( Labcde[C] == 4) {Cny=2;Cnz=2;}
  else {
    printf("QMP: Cannot fold BG/Q to native3d for large C dimensions\n");
    printf("QMP: try 4d \"-qmp-geom native\" instead\n");
    QMP_abort(-1);
  }

  if ( abcde[C] == 0 ) { Cy=0;Cz=0;}
  if ( abcde[C] == 1 ) { Cy=1;Cz=0;}
  if ( abcde[C] == 2 ) { Cy=1;Cz=1;}
  if ( abcde[C] == 3 ) { Cy=0;Cz=1;}

  Lxyzt[X] = Labcde[E]*Labcde[D];
  Lxyzt[Y] = Cny*Labcde[A];
  Lxyzt[Z] = Cnz*Labcde[B];

  if ( abcde[E] == 0 )    xyzt[X] = abcde[D];
  else                    xyzt[X] = 2*Labcde[D]-1-abcde[D];

  if ( Cy==0 )  xyzt[Y] = abcde[A];
  else          xyzt[Y] = 2*Labcde[A]-1-abcde[A];

  if ( Cz==0 )  xyzt[Z] = abcde[B];
  else          xyzt[Z] = 2*Labcde[B]-1-abcde[B];


}
static void BGQtoLogical4d(int * xyzt, int *Lxyzt, int *abcde, int *Labcde)
{
  Lxyzt[X] = Labcde[A];
  Lxyzt[Y] = Labcde[C];
  Lxyzt[Z] = Labcde[D];
  Lxyzt[T] = Labcde[E]*Labcde[B];

  xyzt[X] = abcde[A];
  xyzt[Y] = abcde[C];
  xyzt[Z] = abcde[D];

  if ( abcde[E] == 0 ) xyzt[T] = abcde[B];
  else                 xyzt[T] = 2*Labcde[B]-1-abcde[B];

}

static void LogicalToBGQ(int * xyzt, int *abcde, int *Labcde)
{
  int xyzt_seek[4];
  int lxyzt[4];

  for(abcde[A]=0;abcde[A]<Labcde[A];abcde[A]++){
  for(abcde[B]=0;abcde[B]<Labcde[B];abcde[B]++){
  for(abcde[C]=0;abcde[C]<Labcde[C];abcde[C]++){
  for(abcde[D]=0;abcde[D]<Labcde[D];abcde[D]++){
  for(abcde[E]=0;abcde[E]<Labcde[E];abcde[E]++){
    BGQtoLogical(xyzt_seek,lxyzt,abcde,Labcde);
    if ( (xyzt[A] == xyzt_seek[A])
      && (xyzt[B] == xyzt_seek[B])
      && (xyzt[C] == xyzt_seek[C])
      && (xyzt[D] == xyzt_seek[D])
      && (xyzt[E] == xyzt_seek[E])
	 ) { 
      return;
    }
  }}}}}
  printf("QMP: Logical error seeking node\n");
  QMP_abort(-1);
}
static void BGQtoLogical(int * xyzt, int *Lxyzt, int *abcde, int *Labcde)
{

  if (bgq3d) BGQtoLogical3d(xyzt,Lxyzt,abcde,Labcde);
  else BGQtoLogical4d(xyzt,Lxyzt,abcde,Labcde);

}

static void set_native_machine(void)
{
  int abcde[5];
  int xyzt[4];
  int neighbour[4];
  int neigh_abcde[5];

  int Labcde[5];
  int Lxyzt[4];

  int MPItasks, allNodes, tasksPerNode;

  int nd,n,k;
  int mu;

  int tasks, taskInNode, dim;

  Personality_t personality;
  Kernel_GetPersonality(&personality, sizeof(personality));

  Labcde[A] = personality.Network_Config.Anodes;
  Labcde[B] = personality.Network_Config.Bnodes;
  Labcde[C] = personality.Network_Config.Cnodes;
  Labcde[D] = personality.Network_Config.Dnodes;
  Labcde[E] = personality.Network_Config.Enodes;

  abcde[A] = personality.Network_Config.Acoord;
  abcde[B] = personality.Network_Config.Bcoord;
  abcde[C] = personality.Network_Config.Ccoord;
  abcde[D] = personality.Network_Config.Dcoord;
  abcde[E] = personality.Network_Config.Ecoord;

  BGQtoLogical(xyzt,Lxyzt,abcde,Labcde);

  MPItasks = QMP_global_m->num_nodes;
  allNodes = Lxyzt[X]*Lxyzt[Y]*Lxyzt[Z]*Lxyzt[T];
  tasksPerNode = MPItasks/allNodes;

  nd=4;
  QMP_global_m->ndim = nd;
  QMP_global_m->geom = (int *) malloc(nd*sizeof(int));
  QMP_global_m->coord = (int *) malloc(nd*sizeof(int));

  QMP_global_m->geom[0] = Lxyzt[X];
  QMP_global_m->geom[1] = Lxyzt[Y];
  QMP_global_m->geom[2] = Lxyzt[Z];
  QMP_global_m->geom[3] = Lxyzt[T];

  QMP_global_m->coord[0] = xyzt[X];
  QMP_global_m->coord[1] = xyzt[Y];
  QMP_global_m->coord[2] = xyzt[Z];
  QMP_global_m->coord[3] = xyzt[T];

  /*Assumes ranks go within node fastest, code strategy taken by Walkup*/
  taskInNode = QMP_global_m->nodeid%allNodes;
  dim=0;

  while(tasksPerNode>1){
    QMP_global_m->geom[dim]*=2;
    QMP_global_m->coord[dim]*=2;
    if ( taskInNode&0x1 ) {
      QMP_global_m->coord[dim]++;
    }
    taskInNode>>=1;
    tasksPerNode>>=1;
    dim = (dim+1)%3;
  }

  QMP_me.Destination.Destination=0;
  QMP_me.Destination.A_Destination=abcde[A];
  QMP_me.Destination.B_Destination=abcde[B];
  QMP_me.Destination.C_Destination=abcde[C];
  QMP_me.Destination.D_Destination=abcde[D];
  QMP_me.Destination.E_Destination=abcde[E];

  for(mu=0;mu<5;mu++) QMP_Labcde[mu]=Labcde[mu];

  // FIXME this mapping for SPI use is broken for more than 1 MPI task per node
  for(mu=0;mu<4;mu++){
    neighbour[X] = xyzt[X];
    neighbour[Y] = xyzt[Y];
    neighbour[Z] = xyzt[Z];
    neighbour[T] = xyzt[T];

    neighbour[mu]= (xyzt[mu]+Lxyzt[mu]-1)%Lxyzt[mu];
    LogicalToBGQ(neighbour,neigh_abcde,Labcde);
    QMP_neigh[mu*2].Destination.A_Destination=neigh_abcde[A];
    QMP_neigh[mu*2].Destination.B_Destination=neigh_abcde[B];
    QMP_neigh[mu*2].Destination.C_Destination=neigh_abcde[C];
    QMP_neigh[mu*2].Destination.D_Destination=neigh_abcde[D];
    QMP_neigh[mu*2].Destination.E_Destination=neigh_abcde[E];

    neighbour[mu]= (xyzt[mu]+1)%Lxyzt[mu];
    LogicalToBGQ(neighbour,neigh_abcde,Labcde);
    QMP_neigh[mu*2+1].Destination.A_Destination=neigh_abcde[A];
    QMP_neigh[mu*2+1].Destination.B_Destination=neigh_abcde[B];
    QMP_neigh[mu*2+1].Destination.C_Destination=neigh_abcde[C];
    QMP_neigh[mu*2+1].Destination.D_Destination=neigh_abcde[D];
    QMP_neigh[mu*2+1].Destination.E_Destination=neigh_abcde[E];
  }

  if (QMP_global_m->nodeid==0) {
    printf("*********************************\n");
    printf("* QMP BlueGene/Q Native topology\n");
    printf("*********************************\n");
    printf("* Torus: %d x %d x %d x %d x%d \n",Labcde[A],Labcde[B],Labcde[C],Labcde[D],Labcde[E]);
    printf("* SMP: %d tasks per node\n",tasksPerNode);
    printf("* MPI: %d tasks\n",MPItasks);
    printf("* Physical XxYxZxT = %dx%dx%dx%d\n",Lxyzt[X],Lxyzt[Y],Lxyzt[Z],Lxyzt[T]);
    printf("* Logical  XxYxZxT = %dx%dx%dx%d\n",
	   QMP_global_m->geom[0],QMP_global_m->geom[1],
	   QMP_global_m->geom[2],QMP_global_m->geom[3]);
    printf("*********************************\n");
  }

  n = 0;
  for(k=nd-1;k>=0;k--){
    n=n*QMP_global_m->geom[k]+QMP_global_m->coord[k];
  }
  QMP_global_m->nodeid=n;

  //  printf("QMP MAP %d %d %d %d <==> %d %d %d %d %d Rank %d\n",xyzt[X],xyzt[Y],xyzt[Z],xyzt[T],
  //	 abcde[A],abcde[B],abcde[C],abcde[D],abcde[E],n
  //	 );

  if ( MPI_Comm_split(MPI_COMM_WORLD,1,QMP_global_m->nodeid,&QMP_COMM_WORLD) != MPI_SUCCESS ) {
    printf("* MPI_Comm_split failed\n");
    QMP_abort_string (-1, "MPI_Comm_split failed");
  }

}

#else  /* native only supported on BG/L and BG/P and BG/Q */

static void
set_native_machine(void)
{
  QMP_global_m->ic_type = QMP_SWITCH;
  QMP_global_m->ndim = 0;
  QMP_global_m->geom = NULL;
  QMP_global_m->coord = NULL;
}

#endif

static void
get_arg(int argc, char **argv, char *tag, int *first, int *last,
	char **c, int **a)
{
  int i;
  *first = -1;
  *last = -1;
  *c = NULL;
  *a = NULL;
  for(i=1; i<argc; i++) {
    if(strcmp(argv[i], tag)==0) {
      *first = i;
      //printf("%i %i\n", i, argc);
      if( ((i+1)<argc) && !(isdigit(argv[i+1][0])) ) {
	//printf("c %i %s\n", i+1, argv[i+1]);
	*c = argv[i+1];
	*last = i+1;
      } else {
	//printf("a %i %s\n", i+1, argv[i+1]);
	while( (++i<argc) && isdigit(argv[i][0]) );
	*last = i-1;
	int n = *last - *first;
	if(n) {
	  int j;
	  *a = (int *) malloc(n*sizeof(int));
	  //printf("%i %p\n", n, *a);
	  for(j=0; j<n; j++) {
	    (*a)[j] = atoi(argv[*first+1+j]);
	    //printf(" %i", (*a)[j]);
	  }
	  //printf("\n");
	}
      }
    }
  }
}

static void
remove_from_args(int *argc, char ***argv, int first, int last)
{
  int n = last - first;
  if(first>=0) {
    int i;
    for(i=last+1; i<*argc; i++) (*argv)[i-n-1] = (*argv)[i];
    *argc -= n + 1;
  }
}

static void
permute(int *a, int *p, int n)
{
  int i, t[n];
  for(i=0; i<n; i++) t[i] = a[i];
  for(i=0; i<n; i++) a[i] = t[p[i]];
}

static int 
lex_rank(const int coords[], int dim, int size[])
{
  int d;
  int rank = coords[dim-1];

  for(d = dim-2; d >= 0; d--){
    rank = rank * size[d] + coords[d];
  }
  return rank;
}

/* Create partitions of equal size from the allocated machine, based
   on num_jobs */
static void
repartition_switch_machine(void){
  int jobid, localnodeid;
  int num_jobs = QMP_global_m->num_jobs;
  int num_nodes = QMP_global_m->num_nodes;
  int nodeid = QMP_global_m->nodeid;
  MPI_Comm jobcomm;
  int localgeom;

  /* localgeom gives the number of nodes in the job partition */
  if(num_nodes % num_jobs != 0){
    fprintf(stderr, "num_jobs %i must divide number of nodes %i\n",
	    num_jobs, num_nodes);
    QMP_abort(-1);
  }
  localgeom = num_nodes/num_jobs;
  jobid = nodeid/localgeom;

  /* Split the communicator */

  if( MPI_Comm_split(QMP_COMM_WORLD, jobid, 0, &jobcomm) != MPI_SUCCESS){
    QMP_abort_string (-1, "MPI_Comm_split failed");
  }

  if (MPI_Comm_rank(jobcomm, &localnodeid) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_rank failed");

  /* Make QMP on this node think I live in just this one job partition */

  if( MPI_Comm_free(&QMP_COMM_WORLD) != MPI_SUCCESS){
    QMP_abort_string (-1, "MPI_Comm_free failed");
  }
  QMP_COMM_WORLD = jobcomm;
  QMP_global_m->num_nodes = localgeom;
  QMP_global_m->nodeid = localnodeid;
  QMP_global_m->jobid = jobid;
}

/* Create partitions of equal size from the allocated machine, based
   on jobgeom */
static void
repartition_mesh_machine(void){
  int i;
  int jobid, localnodeid;
  int nd = QMP_global_m->ndim;
  int num_jobs = QMP_global_m->num_jobs;
  int *jobgeom = QMP_global_m->jobgeom;
  int *geom = QMP_global_m->geom;
  MPI_Comm jobcomm;
  int *jobcoord, *localgeom, *localcoord;
  int *worldcoord = QMP_global_m->coord;

  if(jobgeom == NULL)return;

  /* localgeom gives the node dimensions of the job partition */
  localgeom = (int *)malloc(sizeof(int)*nd);
  for(i=0; i<nd; i++){
    if(geom[i] % jobgeom[i] != 0){
      fprintf(stderr, "job partition[%i] = %i must divide machine geometry %i\n",
	      i, jobgeom[i], geom[i]);
      QMP_abort(-1);
    }
    localgeom[i] = geom[i]/jobgeom[i];
  }

  /* jobcoord locates my job partition in the world of job partitions */
  /* localcoord locates my node within the job partition */
  jobcoord = (int *)malloc(sizeof(int)*nd);
  localcoord = (int *)malloc(sizeof(int)*nd);

  for(i=0; i<nd; i++){
    localcoord[i] = worldcoord[i]%localgeom[i];
    jobcoord[i]   = worldcoord[i]/localgeom[i];
  }

  jobid = lex_rank(jobcoord, nd, jobgeom);

  /* Split the communicator */

  if( MPI_Comm_split(QMP_COMM_WORLD, jobid, 0, &jobcomm) != MPI_SUCCESS){
    QMP_abort_string (-1, "MPI_Comm_split failed");
  }

  if (MPI_Comm_rank(jobcomm, &localnodeid) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_rank failed");

  /* Make QMP on this node think I live in just this one job partition */

  if( MPI_Comm_free(&QMP_COMM_WORLD) != MPI_SUCCESS){
    QMP_abort_string (-1, "MPI_Comm_free failed");
  }
  QMP_COMM_WORLD = jobcomm;
  QMP_global_m->num_nodes = QMP_global_m->num_nodes/num_jobs;
  QMP_global_m->nodeid = localnodeid;
  QMP_global_m->jobid = jobid;
  for(i=0; i<nd; i++){
    QMP_global_m->coord[i] = localcoord[i];
    QMP_global_m->geom[i]  = localgeom[i];
  }

  free(localcoord);
  free(jobcoord);
  free(localgeom);
}

static int 
process_qmp_alloc_map(int* argc, char*** argv){
  int na;
  int first, last, *a=NULL;
  char *c=NULL;

  /* Specifies a permutation of the machine axes */
  /* Syntax -qmp-alloc-map a[0] a[1] ... */
  get_arg(*argc, *argv, "-qmp-alloc-map", &first, &last, &c, &a);
  //printf("%i %i %p %p\n", first, last, c, a);
  if( c ) {
    fprintf(stderr, "unknown argument to -qmp-alloc-map: %s\n", c);
    QMP_abort(-1);
  }
  na = last - first;
  if(na<=0) {
    QMP_global_m->amap = NULL;
  } else {
    QMP_global_m->amap = a;
  }
  remove_from_args(argc, argv, first, last);

  return na;
}

static void
process_qmp_logic_map(int *argc, char ***argv){
  int nl;
  int first, last, *a=NULL;
  char *c=NULL;

  /* Specifies logical machine dimensions */
  /* Syntax -qmp-logic-map a[0] a[1] ... */
   get_arg(*argc, *argv, "-qmp-logic-map", &first, &last, &c, &a);
  //printf("%i %i %p %p\n", first, last, c, a);
  if( c ) {
    fprintf(stderr, "unknown argument to -qmp-logic-map: %s\n", c);
    QMP_abort(-1);
  }
  nl = last - first;
  if(nl<=0) {
    QMP_global_m->lmaplen = 0;
    QMP_global_m->lmap = NULL;
  } else {
    QMP_global_m->lmaplen = nl;
    QMP_global_m->lmap = a;
  }
  remove_from_args(argc, argv, first, last);
}

static void 
process_qmp_geom(int* argc, char*** argv, int na){
  int nd;
  int first, last, *a=NULL;
  char *c=NULL;

  /* Specifies physical machine dimensions */
  /* Syntax -qmp-geom a[0] a[1] ... */
  get_arg(*argc, *argv, "-qmp-geom", &first, &last, &c, &a);
  //printf("%i %i %p %p\n", first, last, c, a);
  if( c && strcmp(c, "native")!=0 ) {
    fprintf(stderr, "unknown argument to -qmp-geom: %s\n", c);
    QMP_abort(-1);
  }
  nd = last - first;
  if(nd<=0) {
    QMP_global_m->ic_type = QMP_SWITCH;
    QMP_global_m->ndim = 0;
    QMP_global_m->geom = NULL;
    QMP_global_m->coord = NULL;
  } else { /* act like a mesh */
    QMP_global_m->ic_type = QMP_MESH;
    if( c && strcmp(c, "native")==0 ) {
      set_native_machine();
      nd = QMP_global_m->ndim;
      if(QMP_global_m->amap) {
	if(na!=nd) {
	  fprintf(stderr, "allocated number of dimensions %i != map dimension %i\n", nd, na);
	  QMP_abort(-1);
	}
	permute(QMP_global_m->geom, QMP_global_m->amap, nd);
	permute(QMP_global_m->coord, QMP_global_m->amap, nd);
      }
#if defined(HAVE_BGQ)
    } else if ( c && strcmp(c, "native3d")==0 ) {
      bgq3d=1;
      set_native_machine();
      nd = QMP_global_m->ndim;
      QMP_global_m->amap=0;
#endif
    } else {
      int i, n;
      QMP_global_m->ndim = nd;
      QMP_global_m->geom = a;
      QMP_global_m->coord = (int *) malloc(nd*sizeof(int));
      if(QMP_global_m->amap) {
	if(na!=nd) {
	  fprintf(stderr, "allocated number of dimensions %i != map dimension %i\n", nd, na);
	  QMP_abort(-1);
	}
	n = QMP_global_m->nodeid;
	for(i=0; i<nd; i++) {
	  QMP_global_m->coord[QMP_global_m->amap[i]] =
	    n % QMP_global_m->geom[QMP_global_m->amap[i]];
	  n /= QMP_global_m->geom[QMP_global_m->amap[i]];
	}
      } else {
	n = QMP_global_m->nodeid;
	for(i=0; i<nd; i++) {
	  QMP_global_m->coord[i] = n % QMP_global_m->geom[i];
	  n /= QMP_global_m->geom[i];
	}
      }
    }
  }
  remove_from_args(argc, argv, first, last);
}

static void 
process_qmp_jobs(int *argc, char ***argv){
  int nj;
  int nd = QMP_global_m->ndim;
  int first, last, *a=NULL;
  char *c=NULL;

  /* Specifies job partitions */
  /* Syntax mesh view: -qmp-jobs a[0] a[1] ... 
     (Mesh view requires -qmp-geom.) */
  /* OR switch view:   -qmp-jobs a[0] */

  /* This option causes the allocated machine to be subdivided into independent
     partitions in which separate jobs run with the same executable. */

  /* The integer a[i] specifies the number of divisions of the ith geom dimension */
  /* The default a[i] = 1 for all i implies no subdivision */

  /* default settings */
  QMP_global_m->jobid = 0;
  QMP_global_m->num_jobs = 1;
  QMP_global_m->jobgeom = NULL;
  QMP_global_m->njobdim = 0;

  get_arg(*argc, *argv, "-qmp-jobs", &first, &last, &c, &a);
  // printf("%i %i %p %p\n", first, last, c, a);
  if( c ) {
    fprintf(stderr, "unknown argument to -qmp-jobs: %s\n", c);
    QMP_abort(-1);
  }
  nj = last - first;
  if(nj) {
    int i;
    QMP_global_m->jobgeom = a;
    QMP_global_m->njobdim = nj;
    /* Check sanity of job partition divisions */
    /* For a swich-based machine we allow only nj = 1 */
    if(nj != 1 && !QMP_global_m->geom){
      fprintf(stderr, "-qmp-jobs requires -qmp-geom\n");
      QMP_abort(-1);
    }
    if(QMP_global_m->geom && nj!=nd) {
      fprintf(stderr, "allocated number dimensions %i != job partition dimensions %i\n", nd, nj);
      QMP_abort(-1);
    }
    for(i=0; i<nj; i++){
      if(QMP_global_m->jobgeom[i]<=0){
	fprintf(stderr, "job partition division[%i] = %i <= 0\n",
		i, QMP_global_m->jobgeom[i]);
	QMP_abort(-1);
      }
      QMP_global_m->num_jobs *= QMP_global_m->jobgeom[i];
    }
  }
  remove_from_args(argc, argv, first, last);
}

/**
 * Populate this machine information.
 */
static void
QMP_init_machine_i(int* argc, char*** argv)
{
  ENTER_INIT;

  /* get host name of this machine */
  /*gethostname (QMP_global_m->host, sizeof (QMP_global_m->host));*/
  QMP_global_m->host = (char *) malloc(MPI_MAX_PROCESSOR_NAME);
  MPI_Get_processor_name(QMP_global_m->host, &QMP_global_m->hostlen);

  /* Process QMP command line arguments */

  /* -qmp-alloc-map */
  int na = process_qmp_alloc_map(argc, argv);

  /* -qmp-logic-map */
  process_qmp_logic_map(argc, argv);

  /* -qmp-geom */
  process_qmp_geom(argc, argv, na);

  /* -qmp-jobs */
  process_qmp_jobs(argc, argv);

  /* Repartition the machine if requested */

  if(QMP_global_m->jobgeom){
    if(QMP_global_m->njobdim==1)
      repartition_switch_machine();
    else
      repartition_mesh_machine();
  }

  LEAVE_INIT;
}

/* This is called by the parent */
QMP_status_t
QMP_init_msg_passing (int* argc, char*** argv, QMP_thread_level_t required,
		      QMP_thread_level_t *provided)
{
 /* Basic variables containing number of nodes and which node this process is */
  int PAR_num_nodes;
  int PAR_node_rank;
  ENTER_INIT;

  if(QMP_global_m->inited) {
    QMP_FATAL("QMP_init_msg_passing called but QMP is already initialized!");
  }

#if 0
  /* MPI_Init_thread seems to be broken on the Cray X1 so we will
     use MPI_Init for now until we need real thread support */
  int mpi_req, mpi_prv;
  mpi_req = MPI_THREAD_SINGLE;  /* just single for now */
  if (MPI_Init_thread(argc, argv, mpi_req, &mpi_prv) != MPI_SUCCESS) 
    QMP_abort_string (-1, "MPI_Init failed");
#endif

#if 1
  if (MPI_Init_thread(argc, argv, QMP_THREAD_MULTIPLE, provided) != MPI_SUCCESS) 
    QMP_abort_string (-1, "MPI_Init failed");
  if (*provided != QMP_THREAD_MULTIPLE)
    QMP_abort_string (-1, "MPI_Init provided != required,");
#else
  if (MPI_Init(argc, argv) != MPI_SUCCESS) 
    QMP_abort_string (-1, "MPI_Init failed");
#endif
  if (MPI_Comm_dup(MPI_COMM_WORLD, &QMP_COMM_WORLD) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_dup failed");
  if (MPI_Comm_size(QMP_COMM_WORLD, &PAR_num_nodes) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_size failed");
  if (MPI_Comm_rank(QMP_COMM_WORLD, &PAR_node_rank) != MPI_SUCCESS)
    QMP_abort_string (-1, "MPI_Comm_rank failed");

  QMP_global_m->num_nodes = PAR_num_nodes;
  QMP_global_m->nodeid = PAR_node_rank;
  QMP_global_m->verbose = 0;
  QMP_global_m->proflevel = 0;
  QMP_global_m->inited = QMP_TRUE;
  QMP_global_m->err_code = QMP_SUCCESS;
  QMP_global_m->total_qmp_time = 0.0;
  QMP_global_m->timer_started = 0;

  QMP_topo->topology_declared = QMP_FALSE;

  QMP_init_machine_i(argc, argv);

  LEAVE_INIT;
  return QMP_global_m->err_code;
}

/* Shutdown the machine */
void
QMP_finalize_msg_passing(void)
{
  ENTER_INIT;
  MPI_Finalize();
  QMP_global_m->inited = QMP_FALSE;
  LEAVE_INIT;
}

/* Abort the program */
void 
QMP_abort(int error_code)
{
  ENTER_INIT;
  MPI_Abort(QMP_COMM_WORLD, error_code);
  QMP_global_m->inited = QMP_FALSE;
  LEAVE_INIT;
}

/* Print string and abort the program */
void
QMP_abort_string(int error_code, char *message)
{
  ENTER_INIT;
  fprintf(stderr, message);
  fprintf(stderr, "\n");
  MPI_Abort(QMP_COMM_WORLD, error_code);
  QMP_global_m->inited = QMP_FALSE;
  LEAVE_INIT;
}
