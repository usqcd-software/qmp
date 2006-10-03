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
#ifndef __USE_UNIX98
#define __USE_UNIX98 /* needed to get gethostname from GNU unistd.h */
#endif
#include <unistd.h>

#include "QMP_P_MPI.h"
#ifdef HAVE_BGL
#include <rts.h>
#endif

/* global communicator */
MPI_Comm QMP_COMM_WORLD;

/**
 * This machine information
 */
static struct QMP_machine par_machine = QMP_MACHINE_INIT;
QMP_machine_t QMP_global_m = &par_machine;

static struct QMP_logical_topology par_logical_topology;
QMP_logical_topology_t QMP_topo = &par_logical_topology;


/**
 * Populate this machine information.
 */
static void
QMP_init_machine_i(int* argc, char*** argv)
{
  ENTER_INIT;

  /* get host name of this machine */
  gethostname (QMP_global_m->host, sizeof (QMP_global_m->host));

  int first=-1, last=-1;
  int i, nd, n;
  for(i=0; i<*argc; i++) {
    if(strcmp((*argv)[i], "-qmp-geom")==0) {
      first = i;
      if( ((i+1)<*argc) && (strcmp((*argv)[i+1], "native")==0) ) last = i+1;
      else {
	while( (++i<*argc) && (isdigit((*argv)[i][0])) );
	last = i-1;
      }
    }
  }

  nd = last - first;
  if(nd<=0) {
    QMP_global_m->ic_type = QMP_SWITCH;
    QMP_global_m->ndim = 0;
    QMP_global_m->geom = NULL;
    QMP_global_m->coord = NULL;
  } else { /* act like a mesh */
    QMP_global_m->ic_type = QMP_MESH;
    if(strcmp((*argv)[last], "native")==0) {
#ifdef HAVE_BGL
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
#else  /* native only supported on BG/L */
      QMP_global_m->ic_type = QMP_SWITCH;
      QMP_global_m->ndim = 0;
      QMP_global_m->geom = NULL;
      QMP_global_m->coord = NULL;
#endif
    } else {
      QMP_global_m->ndim = nd;
      QMP_global_m->geom = (int *) malloc(nd*sizeof(int));
      QMP_global_m->coord = (int *) malloc(nd*sizeof(int));
      n = QMP_global_m->nodeid;
      for(i=0; i<nd; i++) {
	QMP_global_m->geom[i] = atoi((*argv)[i+first+1]);
	QMP_global_m->coord[i] = n % QMP_global_m->geom[i];
	n /= QMP_global_m->geom[i];
      }
    }
  }
  nd = last - first;
  if(first>=0) {  /* remove from arguments */
    for(i=last+1; i<*argc; i++) (*argv)[i-nd-1] = (*argv)[i];
    *argc -= nd + 1;
  }
  /* QMP_printf("allocated dimensions = %i\n", QMP_global_m->ndim); */
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

  *provided = QMP_THREAD_SINGLE;  /* just single for now */
  if (MPI_Init(argc, argv) != MPI_SUCCESS) 
    QMP_abort_string (-1, "MPI_Init failed");
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
