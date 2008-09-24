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
 *      Geometry layout test
 *
 * Author:  
 *      Robert Edwards
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: QMP_grid_test.c,v $
 *   Revision 1.5  2006/01/05 03:12:56  osborn
 *   Added --enable-bgl compilation option.
 *
 *   Revision 1.4  2004/06/14 20:36:30  osborn
 *   Updated to API version 2 and added single node target
 *
 *   Revision 1.3  2003/02/13 16:23:04  chen
 *   qmp version 1.2
 *
 *   Revision 1.2  2003/02/11 03:39:24  flemingg
 *   GTF: Update of automake and autoconf files to use qmp-config in lieu
 *        of qmp_build_env.sh
 *
 *   Revision 1.1.1.1  2003/01/27 19:31:37  chen
 *   check into lattice group
 *
 *   Revision 1.2  2002/04/26 18:35:44  chen
 *   Release 1.0.0
 *
 *   Revision 1.1  2002/04/22 20:28:41  chen
 *   Version 0.95 Release
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include "qmp.h"

#define ND 4

int
main(int argc, char **argv)
{
  int length[ND] = {4, 2, 4, 8};
  /* int phys_size[ND] = {1, 1, 1, 1}; */
  int i,err;
  int num;
  QMP_status_t status;
  QMP_thread_level_t req, prv;

  req = QMP_THREAD_SINGLE;
  status = QMP_init_msg_passing (&argc, &argv, req, &prv);

  fprintf(stderr, "lattice =");
  for(i=0; i< ND; ++i) fprintf(stderr, " %d", length[i]);
  fprintf(stderr, "\n");

  /* Let the geometry arrange itself */
  fprintf(stderr,"Dynamical machine size\n");
  err = QMP_layout_grid(length, ND);
  num = QMP_get_number_of_nodes();
  {
    const int *s = QMP_get_subgrid_dimensions();
    fprintf(stderr, "subgrid =");
    for(i=0; i< ND; ++i) fprintf(stderr, " %d", s[i]);
    fprintf(stderr, "\n");
  }

#if 0
  /* Set the geometry manually */
  fprintf(stderr,"Fixed machine size\n");
  phys_size[ND-1] = num;
  err = QMP_declare_logical_topology(phys_size, ND);
  err = QMP_layout_grid(length, ND);
  {
    const int *s = QMP_get_subgrid_dimensions();
    fprintf(stderr, "subgrid =");
    for(i=0; i< ND; ++i) fprintf(stderr, " %d", s[i]);
    fprintf(stderr, "\n");
  }
#endif

  /* Try sending messages */
  fprintf(stderr,"starting messages\n");
  {
    const int *ldims = QMP_get_logical_dimensions();
    const int *lcoords = QMP_get_logical_coordinates();
    char str0[] = "test string";
    char str1[] = "another test";

    int dir;
    int isign = 1;
    for(dir=0; dir<ND; dir++) {
      if(ldims[dir]<2) continue;
      fprintf(stderr,"dir = %i\n", dir);

      if((lcoords[dir]%2==0)&&(lcoords[dir]!=ldims[dir]-1)) {
	QMP_msgmem_t msg[2];
	QMP_msghandle_t mh_a[2], mh;
      
	msg[0] = QMP_declare_msgmem(str0,sizeof(str0)+1);
	msg[1] = QMP_declare_msgmem(str1,sizeof(str1)+1);
	mh_a[0] = QMP_declare_send_relative(msg[0], dir, isign, 0);
	mh_a[1] = QMP_declare_send_relative(msg[1], dir, isign, 0);
	mh = QMP_declare_multiple(mh_a, 2);
	QMP_start(mh);
	QMP_wait(mh);
	fprintf(stderr,"Node %d: wait done\n", QMP_get_node_number());
      } else if(lcoords[dir]%2==1) {
#define STRSIZE 50
	int strsize = STRSIZE;
	QMP_msgmem_t msg[2];
	QMP_msghandle_t mh_a[2], mh;
	QMP_mem_t *recv0 = QMP_allocate_memory(strsize);
	QMP_mem_t *recv1 = QMP_allocate_memory(strsize);

	msg[0] = QMP_declare_msgmem(QMP_get_memory_pointer(recv0),strsize);
	msg[1] = QMP_declare_msgmem(QMP_get_memory_pointer(recv1),strsize);
	mh_a[0] = QMP_declare_receive_relative(msg[0], dir, -isign, 0);
	mh_a[1] = QMP_declare_receive_relative(msg[1], dir, -isign, 0);
	mh = QMP_declare_multiple(mh_a, 2);
	QMP_start(mh);
	QMP_wait(mh);
	fprintf(stderr,"Node %d: recv0 = XX%sXX\n", QMP_get_node_number(), (char *)QMP_get_memory_pointer(recv0));
	fprintf(stderr,"Node %d: recv1 = XX%sXX\n", QMP_get_node_number(), (char *)QMP_get_memory_pointer(recv1));
      }
    }
  }
  fprintf(stderr,"finished messages\n");

  QMP_finalize_msg_passing();
  return 0;
}
