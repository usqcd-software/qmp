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
 *   $Log: not supported by cvs2svn $
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
#include "QMP.h"

#define ND 4

int main(int argc, char **argv)
{
  int length[ND] = {4, 2, 4, 8};
  int phys_size[ND] = {1, 1, 1, 1};
  int i,err;
  int num;

  QMP_init_msg_passing(&argc, &argv, QMP_SMP_ONE_ADDRESS);

  /* Let the geometry arrange itself */
  fprintf(stderr,"Dynamical machine size\n");
  err = QMP_layout_grid(length, ND);
  num = QMP_get_number_of_nodes();
  {
    const int *s = QMP_get_subgrid_dimensions();
    for(i=0; i < ND; ++i)
      fprintf(stderr,"subgrid[%d] = %d\n",i,s[i]);
  }

  /* Set the geometry manually */
  fprintf(stderr,"Fixed machine size\n");
  phys_size[ND-1] = num;
  err = QMP_declare_logical_topology(phys_size, ND);
  err = QMP_layout_grid(length, ND);
  {
    const int *s = QMP_get_subgrid_dimensions();
    for(i=0; i < ND; ++i)
      fprintf(stderr,"subgrid[%d] = %d\n",i,s[i]);
  }
  
  /* Try sending messages */
  fprintf(stderr,"starting messages\n");
  {
    char str0[] = "test string";
    char str1[] = "another test";
    int dir = 3;
    int isign = 1;

    if (QMP_get_node_number() == 0)
    {
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
    }
    else if (QMP_get_node_number() == 1)
    {
#define STRSIZE 50
      int strsize = STRSIZE;
      QMP_msgmem_t msg[2];
      QMP_msghandle_t mh_a[2], mh;
      char *recv0 = (char *)QMP_allocate_aligned_memory(strsize);
      char *recv1 = (char *)QMP_allocate_aligned_memory(strsize);

      msg[0] = QMP_declare_msgmem(recv0,strsize);
      msg[1] = QMP_declare_msgmem(recv1,strsize);
      mh_a[0] = QMP_declare_receive_relative(msg[0], dir, -isign, 0);
      mh_a[1] = QMP_declare_receive_relative(msg[1], dir, -isign, 0);
      mh = QMP_declare_multiple(mh_a, 2);
      QMP_start(mh);
      QMP_wait(mh);
      fprintf(stderr,"Node %d: recv0 = XX%sXX\n", QMP_get_node_number(), (char *)recv0);
      fprintf(stderr,"Node %d: recv1 = XX%sXX\n", QMP_get_node_number(), (char *)recv1);
    }
  }
  fprintf(stderr,"finished messages\n");


  QMP_finalize_msg_passing();
  return 0;
}
