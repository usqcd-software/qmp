/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP intialize code for single node build
 *
 * Author:  
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
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
#include "QMP_P_COMMON.h"
#include "QMP_P_SINGLE.h"

/**
 * This machine information
 */
static struct QMP_machine par_machine = {.inited=QMP_FALSE};
QMP_machine_t QMP_global_m = &par_machine;

static struct QMP_logical_topology par_logical_topology;
QMP_logical_topology_t QMP_topo = &par_logical_topology;


/* Initialize QMP */
QMP_status_t
QMP_init_msg_passing (int* argc, char*** argv, QMP_thread_level_t required,
                      QMP_thread_level_t *provided)
{
  /* get host name of this machine */
  gethostname (QMP_global_m->host, sizeof (QMP_global_m->host));

  QMP_global_m->ic_type = QMP_SWITCH;
  QMP_global_m->err_code = QMP_SUCCESS;
  QMP_global_m->num_nodes = 1;
  QMP_global_m->nodeid = 0;
  QMP_global_m->verbose = 0;
  QMP_global_m->proflevel = 0;
  QMP_global_m->inited = QMP_TRUE;

  QMP_topo->topology_declared = QMP_FALSE;

  return QMP_SUCCESS;
}

/* Shutdown the machine */
void
QMP_finalize_msg_passing(void)
{
  if(!QMP_global_m->inited) {
    QMP_FATAL("QMP_finalize_msg_passing called but QMP is not initialized!");
  }
  QMP_global_m->inited = QMP_TRUE;
}

/* Abort the program */
void 
QMP_abort(int error_code)
{
  fprintf(stderr, "node 0/1: QMP Aborted!\n");
  exit(error_code);
}

/* Print string and abort the program */
void
QMP_abort_string(int error_code, char *message)
{
  fprintf(stderr, message);
  fprintf(stderr, "\n");
  fprintf(stderr, "node 0/1: QMP Aborted!\n");
  exit(error_code);
}
