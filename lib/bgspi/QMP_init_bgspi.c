#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_COMMON.h"

QMP_status_t
QMP_init_machine_bgspi(int* argc, char*** argv, QMP_thread_level_t required,
		       QMP_thread_level_t *provided)
{
  QMP_status_t stat;
  qspi_init();
  stat = QMP_init_machine_mpi(argc, argv, required, provided);
  return stat;
}

void
QMP_finalize_msg_passing_bgspi(void)
{
  qspi_finalize();
  QMP_finalize_msg_passing_mpi();
}
