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
 *      Simple Test Program for QMP
 *
 * Author:  
 *      Robert Edwards
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <qmp.h>

#define NUM_HANDLES 4

int main (int argc, char** argv)
{
  int i, j;
  QMP_bool_t status, sender, verbose;
  QMP_u32_t  rank;
  QMP_status_t err;

#if 1
  QMP_u32_t dims[4] = {1, 1, 3, 1};
  QMP_u32_t ndims = 4;
#else
  QMP_u32_t dims[1] = {2};
  QMP_u32_t ndims = 1;
#endif

  verbose = QMP_FALSE;  
  if (argc > 1 && strcmp (argv[1], "-v") == 0)
    verbose = QMP_TRUE;
  
  QMP_verbose (verbose);
  status = QMP_init_msg_passing (&argc, &argv, QMP_SMP_ONE_ADDRESS);

  if (status != QMP_SUCCESS) {
    QMP_fprintf(stderr, "QMP_init failed\n");
    return -1;
  }

  status = QMP_declare_logical_topology (dims, ndims);

  if (status == QMP_FALSE)
    QMP_fprintf (stderr, "Cannot declare logical grid\n");
  else
    QMP_fprintf (stderr, "Declare logical grid ok\n");

  {
    char p[288];
    int i;
    for(i=0; i < 288;++i)
      p[i] = 0;

    for(i=0; i < 1000000;++i)
      QMP_broadcast(p, 288);
  }


  QMP_finalize_msg_passing ();

  return 0;
}


