## Process this file with automake to produce Makefile.in

INCDIR=@top_srcdir@/include
INCFLAGS=-I$(INCDIR)
AM_CFLAGS=$(INCFLAGS) @QMP_COMMS_CFLAGS@

lib_LIBRARIES = libqmp.a

QMP_SINGLE_SRC = QMP_comm_single.c  \
                 QMP_error_single.c \
                 QMP_init_single.c  \
                 QMP_mem_single.c   \
                 $(INCDIR)/QMP_P_SINGLE.h

QMP_MPI_SRC = QMP_comm_mpi.c  \
              QMP_error_mpi.c \
              QMP_init_mpi.c  \
              QMP_mem_mpi.c   \
              $(INCDIR)/QMP_P_MPI.h

QMP_COMMON_SRC = QMP_grid.c     \
                 QMP_machine.c  \
                 QMP_topology.c \
                 QMP_util.c     \
	         $(INCDIR)/QMP_P_COMMON.h \
                 $(INCDIR)/qmp.h

if QMP_SINGLE
  libqmp_a_SOURCES = $(QMP_SINGLE_SRC) $(QMP_COMMON_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_MPI_SRC)
endif

if QMP_MPI
  libqmp_a_SOURCES = $(QMP_MPI_SRC) $(QMP_COMMON_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_SINGLE_SRC)
endif

