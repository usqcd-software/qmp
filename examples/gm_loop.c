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
 *      Simple program to find out how many loops will gm_receive run
 *      in a single second on a particular machine
 *
 *      This program can be used to adjust 
 *      QMP_NUMLOOPS_PSEC value in QMP_P_GM.h
 *
 * Author:  
 *      Jie Chen
 *      Jefferson Lab HPC Group
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1  2002/08/01 18:16:06  chen
 *   Change implementation on how gm port allocated and QMP_run is using perl
 *
 *   Revision 1.1.1.1  2002/01/21 16:14:50  chen
 *   initial import of QMP
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <gm.h>

int main (int argc, char** argv)
{
  unsigned int     numloops, i, device, port;
  struct timeval   it, ft;
  struct gm_port*  gmport;
  gm_status_t      status;
  char             hostname[128];
  gm_recv_event_t* ev;
  double           dt, loopps;
  

  if (argc < 2) {
    fprintf (stderr, "usage: %s numloops\n", argv[0]);
    exit (1);
  }

  numloops = atoi (argv[1]);
  printf ("Run gm empty receiving loop for %u times\n", numloops);

  if (gm_init () != GM_SUCCESS) {
    fprintf (stderr, "Cannot do gm_init \n");
    exit (1);
  }

  /* get host name */
  gethostname (hostname, sizeof (hostname) - 1);
  device = 0;
  port = 2;
  status = gm_open (&gmport, device, port, hostname, 
		    GM_API_VERSION_1_2);
  if (status != GM_SUCCESS) {
    gm_perror ("could not open a gm port", status);
    exit (1);
  }

  gettimeofday (&it, 0);
  i = 0;
  while (i < numloops) {
    ev = gm_receive (gmport);
    gm_unknown (gmport, ev);
    i++;
  }
  gettimeofday (&ft, 0);

  /* close and shutdown gm */
  gm_close (gmport);
  gm_finalize ();

  /* Time difference in milli second       */
  dt = ft.tv_sec*1000.0 + ft.tv_usec/1000.0 
    - it.tv_sec*1000.0 - it.tv_usec/1000.0;

  loopps = (double)numloops/dt*(double)1000.0;
  printf ("Number of loops per second is %lf\n", loopps);
  return 0;
}
    
