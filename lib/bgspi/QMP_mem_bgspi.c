#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "QMP_P_COMMON.h"

void
QMP_alloc_msghandle_bgspi(QMP_msghandle_t mh)
{
  mh->useSPI = -1;
}

void
QMP_free_msghandle_bgspi(QMP_msghandle_t mh)
{
  if(mh->useSPI>0) {
    for(QMP_msghandle_t m=mh->next; m; m=m->next) {
      if(m->useSPI>0) {
	//if(mh->clear_to_send!=QMP_CTS_DISABLED) {
	if(mh->clear_to_send==QMP_CTS_READY) {
	  //while(mh->clear_to_send!=m->clear_to_send);
	  //if(m->clear_to_send==QMP_CTS_READY)
	  qspi_wait(m->qspicts);
	  // FIXME - need to keep track of if CTS was sent
	}
	qspi_free_msg(m->qspimsg);
	qspi_free_msg(m->qspicts);
	m->useSPI = -1;
      }
    }
  }
  QMP_free_msghandle_mpi(mh);
}

void
QMP_change_address_bgspi(QMP_msghandle_t mh)
{
  if(mh->useSPI>0) {
    QMP_FATAL("QMP_change_address for SPI not supported\n");
  } else {
    QMP_change_address_mpi(mh);
  }
}
