#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "QMP_P_COMMON.h"
#include <mpix.h>
// for GetTimeBase()
//#include </bgsys/drivers/ppcfloor/hwi/include/bqc/A2_inlines.h>

static void
QMP_declare_multiple_bgspi(QMP_msghandle_t mh)
{
  if(mh->paired) {
    int npaired = 0;
    int src0 = -1;
    while(1) {
      int src = -1;
      for(QMP_msghandle_t m=mh->next; m; m=m->next) {
	if(src<=src0 || (m->srce_node>src0 && m->srce_node<src)) src = m->srce_node;
      }
      if(src<=src0) break;
      for(QMP_msghandle_t m=mh->next; m; m=m->next) {
	if(m->srce_node==src) {
	  int nmsg = 0, nmsgo;
	  switch(m->mm->type) {
	  case MM_user_buf: nmsg = 1; break;
	  case MM_strided_buf:
	    if(m->type==MH_send) nmsg = m->mm->mm.st.nblocks;
	    break;
	  case MM_strided_array_buf:
	    if(m->type==MH_send) {
	      for(int i=0; i<m->mm->mm.sa.narray; i++) {
		nmsg += m->mm->mm.sa.nblocks[i];
	      }
	    }
	    break;
	  case MM_indexed_buf:
	    if(m->type==MH_send) nmsg = m->mm->mm.in.count;
	    break;
	  }
	  //printf("%i pairing %i -> %i\n", QMP_get_node_number(), m->srce_node, m->dest_node);
	  if(m->type==MH_send) {
	    MPI_Status st;
	    MPI_Send(&nmsg, sizeof(nmsg), MPI_BYTE, m->dest_node, 0, m->comm->mpicomm);
	    MPI_Recv(&nmsgo, sizeof(nmsgo), MPI_BYTE, m->dest_node, 0, m->comm->mpicomm, &st);
	  } else if(m->type==MH_recv) {
	    MPI_Status st;
	    MPI_Recv(&nmsgo, sizeof(nmsgo), MPI_BYTE, m->srce_node, 0, m->comm->mpicomm, &st);
	    MPI_Send(&nmsg, sizeof(nmsg), MPI_BYTE, m->srce_node, 0, m->comm->mpicomm);
	  } else {
	    QMP_FATAL("error: bad msg type");
	  }
	  m->paired = 0;
	  if(nmsg>0 && nmsgo>0) {
	    void *base[nmsg];
	    size_t size[nmsg];
	    switch(m->mm->type) {
	    case MM_user_buf: base[0]=m->base; size[0]=m->mm->nbytes; break;
	    case MM_strided_buf:
	      {
		int offset = 0;
		for(int i=0; i<nmsg; i++) {
		  base[i] = (char*)m->base + offset;
		  size[i] = m->mm->mm.st.blksize;
		  offset += m->mm->mm.st.stride;
		}
	      }
	      break;
	    case MM_strided_array_buf:
	      {
		int k = 0;
		for(int i=0; i<m->mm->mm.sa.narray; i++) {
		  int offset = 0;
		  char *basei = (char*)m->base + m->mm->mm.sa.disp[i];
		  for(int j=0; j<m->mm->mm.sa.nblocks[i]; j++) {
		    base[k] = basei + offset;
		    size[k] = m->mm->mm.sa.blksize[i];
		    k++;
		    offset += m->mm->mm.sa.stride[i];
		  }
		}
	      }
	      break;
	    case MM_indexed_buf:
	      {
		double avgbl = 0;
		for(int i=0; i<nmsg; i++) {
		  base[i] = (char*)m->base + m->mm->mm.in.elemsize*m->mm->mm.in.index[i];
		  size[i] = m->mm->mm.in.elemsize*m->mm->mm.in.blocklen[i];
		  avgbl += m->mm->mm.in.blocklen[i];
		}
		avgbl = (m->mm->mm.in.elemsize*avgbl)/nmsg;
		//if(QMP_get_node_number()==0) printf("indexed: n: %6i  e: %6i  a: %9.2f\n", nmsg, m->mm->mm.in.elemsize, avgbl);
	      }
	      break;
	    }
	    if(m->type==MH_send) {
	      int grank; 
	      MPIX_Comm_rank2global(m->comm->mpicomm, m->dest_node, &grank);
	      m->qspimsg = qspi_create_msg();
	      qspi_set_send_multi(grank, base, size, nmsg, m->qspimsg);
	      qspi_prepare(&m->qspimsg, 1);
	      //printf("%i testing msg send to %i\n", m->srce_node, m->dest_node);
	      //qspi_start(m->qspimsg);
	      //qspi_wait(m->qspimsg);
	      //printf("%i testing msg send to %i done\n", m->srce_node, m->dest_node);
	      m->qspicts = qspi_create_msg();
	      qspi_set_recv(grank, &m->clear_to_send, sizeof(m->clear_to_send), m->qspicts);
	      qspi_prepare(&m->qspicts, 1);
	      //printf("%i testing cts send to %i\n", m->srce_node, m->dest_node);
	      //qspi_start(m->qspicts);
	      //qspi_wait(m->qspicts);
	      //printf("%i testing cts send to %i done\n", m->srce_node, m->dest_node);
	    } else { // recv
	      int grank; 
	      MPIX_Comm_rank2global(m->comm->mpicomm, m->srce_node, &grank);
	      m->qspimsg = qspi_create_msg();
	      //qspi_set_recv_multi(grank, base, size, nmsg, m->qspimsg);
	      qspi_set_recv(grank, base[0], size[0], m->qspimsg);
	      qspi_prepare(&m->qspimsg, 1);
	      //printf("%i testing msg recv from %i\n", m->dest_node, m->srce_node);
	      //qspi_start(m->qspimsg);
	      //qspi_wait(m->qspimsg);
	      //printf("%i testing msg recv from %i done\n", m->dest_node, m->srce_node);
	      m->qspicts = qspi_create_msg();
	      qspi_set_send(grank, &m->clear_to_send, sizeof(m->clear_to_send), m->qspicts);
	      qspi_prepare(&m->qspicts, 1);
	      //printf("%i testing cts recv from %i\n", m->dest_node, m->srce_node);
	      //qspi_start(m->qspicts);
	      //qspi_wait(m->qspicts);
	      //printf("%i testing cts recv from %i done\n", m->dest_node, m->srce_node);
	    }
	    npaired++;
	    m->paired = 1;
	    m->useSPI = 1;
	    //printf("%i paired %i -> %i\n", QMP_get_node_number(), m->srce_node, m->dest_node);
	  } else {
	    //printf("%i unpaired %i -> %i type: %i\n", QMP_get_node_number(), m->srce_node, m->dest_node, m->mm->type);
	  }
	}
      }
      src0 = src;
    }
    mh->useSPI = npaired;
  }
  //QMP_declare_multiple_mpi(mh);
}

QMP_status_t
QMP_clear_to_send_bgspi(QMP_msghandle_t mh)
{
  QMP_status_t err = QMP_SUCCESS;
  ENTER;

  if(mh->clear_to_send==QMP_CTS_READY || mh->clear_to_send==QMP_CTS_NOT_READY) {
    if(mh->useSPI>0) { // useSPI is always multiple
      for(QMP_msghandle_t m=mh->next; m; m=m->next) {
	if(m->useSPI>0) {
	  if(m->type==MH_recv) {
	    if(m->clear_to_send!=mh->clear_to_send) {
	      m->clear_to_send = mh->clear_to_send;
	      qspi_start(m->qspicts);
	      // FIXME - need to keep track of if CTS was sent
	    }
	  }
	}
      }
    }
  }

  LEAVE;
  return err;
}

QMP_status_t
QMP_start_bgspi(QMP_msghandle_t mh)
{
  if(mh->paired && mh->useSPI<0 && mh->uses>=2) QMP_declare_multiple_bgspi(mh);
  QMP_status_t status = QMP_SUCCESS;
  //unsigned long long tmsgstart=0, tctsstart=0, tctswait=0, ttot=0, t0;
  //unsigned long long t0, ttot = GetTimeBase();
  if(mh->useSPI>0) { // useSPI is always multiple
    //if(QMP_comm_get_default()->nodeid==0) printf("using SPI");
    int nleft = mh->num;
    // do recvs first
    //t0 = GetTimeBase();
    for(QMP_msghandle_t m=mh->next; m; m=m->next) {
      if(m->type!=MH_recv) continue;
      QMP_status_t err = QMP_SUCCESS;
      if(m->useSPI>0) {
	if(mh->clear_to_send!=QMP_CTS_DISABLED && m->clear_to_send!=QMP_CTS_READY) {
	  m->clear_to_send = QMP_CTS_READY;
	  //t0 = GetTimeBase();
	  qspi_start(m->qspicts);
	  //ttot += GetTimeBase() - t0;
	  //tctsstart += GetTimeBase() - t0;
	}
	//t0 = GetTimeBase();
	qspi_start(m->qspimsg);
	//ttot += GetTimeBase() - t0;
	//tmsgstart += GetTimeBase() - t0;
      } else {
	err = QMP_start_mpi(m);
      }
      nleft--;
      m->activeP = 1;
      if(status==QMP_SUCCESS) status = err;
    }
    //ttot += GetTimeBase() - t0;
    // now sends
    while(nleft>0) {
      for(QMP_msghandle_t m=mh->next; m; m=m->next) {
	if(m->activeP || m->type!=MH_send) continue;
	QMP_status_t err = QMP_SUCCESS;
	if(m->useSPI>0) {
	  if(mh->clear_to_send==QMP_CTS_DISABLED || m->clear_to_send!=QMP_CTS_NOT_READY) {
	    //t0 = GetTimeBase();
	    qspi_start(m->qspimsg);
	    //ttot += GetTimeBase() - t0;
	    //tmsgstart += GetTimeBase() - t0;
	    nleft--;
	    m->activeP = 1;
	    if(mh->clear_to_send!=QMP_CTS_DISABLED && m->clear_to_send==QMP_CTS_READY) {
	      //t0 = GetTimeBase();
	      qspi_wait(m->qspicts);
	      //ttot += GetTimeBase() - t0;
	      //tctswait += GetTimeBase() - t0;
	      m->clear_to_send = QMP_CTS_NOT_READY;
	    }
	  }
	} else {
	  //t0 = GetTimeBase();
	  err = QMP_start_mpi(m);
	  nleft--;
	  m->activeP = 1;
	  //ttot += GetTimeBase() - t0;
	}
	if(status==QMP_SUCCESS) status = err;
      }
    }
  } else {
    status = QMP_start_mpi(mh);
  }
  //if(QMP_machine->proflevel>0) {
  //ttot = GetTimeBase() - ttot;
  //QMP_machine->total_qmp_time += ((double)ttot)/1.6e9;
  //QMP_machine->proflevel++;
  //if(QMP_get_node_number()==0) {
  //double s = 1./1.6e3;
  //printf("start: %6.2f  ctsstart: %6.2f  ctswait: %6.2f  msgstart: %6.2f\n", s*ttot, s*tctsstart, s*tctswait, s*tmsgstart);
  //}
  //}

  return status;
}

QMP_bool_t
QMP_is_complete_bgspi(QMP_msghandle_t mh)
{
  QMP_bool_t done = QMP_TRUE;

  if(mh->useSPI>0) { // useSPI is always multiple
    QMP_wait_bgspi(mh);
  } else {
    done = QMP_is_complete_mpi(mh);
  }

  return done;
}

QMP_status_t
QMP_wait_bgspi(QMP_msghandle_t mh)
{
  QMP_status_t status = QMP_SUCCESS;

  if(mh->useSPI>0) { // useSPI is always multiple
    int nleft = mh->num;
    while(nleft>0) {
      for(QMP_msghandle_t m=mh->next; m; m=m->next) {
	if(!m->activeP) continue;
	QMP_status_t err = QMP_SUCCESS;
	if(m->useSPI>0) {
	  if(m->type==MH_recv) {
	    if(mh->clear_to_send!=QMP_CTS_DISABLED) {
	      qspi_wait(m->qspicts);
	      m->clear_to_send = QMP_CTS_NOT_READY;
	    }
	  }
	  qspi_wait(m->qspimsg);
	  nleft--;
	  m->activeP = 0;
	} else {
	  err = QMP_wait_mpi(m);
	  nleft--;
	  m->activeP = 0;
	}
	if(status==QMP_SUCCESS && err!=QMP_SUCCESS) status = err;
      }
    }
  } else {
    status = QMP_wait_mpi(mh);
  }

  return status;
}


QMP_status_t
QMP_comm_broadcast_bgspi(QMP_comm_t comm, void *send_buf, size_t count)
{
  QMP_status_t status = QMP_SUCCESS;

  // split into 64kB chinks to workaround MPI bug with SPI
#define CHUNK (64*1024)
  for(size_t start=0; start<count; start+=CHUNK) {
    char *buf = ((char*)send_buf) + start;
    size_t size = count - start;
    if(size>CHUNK) size = CHUNK;
    int err = MPI_Bcast(buf, size, MPI_BYTE, 0, comm->mpicomm);
    if(err != MPI_SUCCESS) { status = err; break; }
  }

  return status;
}
