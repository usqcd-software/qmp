## Process this file with automake to produce Makefile.in

INCDIR=@top_srcdir@/include
INCFLAGS=-I$(INCDIR)
AM_CFLAGS=$(INCFLAGS) @QMP_COMMS_CFLAGS@

lib_LIBRARIES = libqmp.a

QMP_SRC = QMP_comm.c  \
          QMP_error.c \
	  QMP_grid.c     \
          QMP_init.c  \
          QMP_machine.c  \
          QMP_mem.c   \
          QMP_split.c   \
          QMP_topology.c \
          QMP_util.c     \
	  $(INCDIR)/QMP_P_COMMON.h \
          $(INCDIR)/qmp.h

QMP_MPI_SRC = mpi/QMP_comm_mpi.c  \
              mpi/QMP_error_mpi.c \
              mpi/QMP_init_mpi.c  \
              mpi/QMP_mem_mpi.c   \
              mpi/QMP_split_mpi.c   \
	      mpi/QMP_topology_mpi.c \
              $(INCDIR)/QMP_P_MPI.h

QMP_BGSPI_SRC = bgspi/QMP_comm_bgspi.c \
                bgspi/QMP_init_bgspi.c \
                bgspi/QMP_mem_bgspi.c \
                bgspi/qspi.c \
                bgspi/qspi.h \
                bgspi/qspi_internal.h \
	        $(INCDIR)/QMP_P_BGSPI.h

libqmp_a_SOURCES = $(QMP_SRC)
EXTRA_libqmp_a_SOURCES =

if QMP_MPI
  libqmp_a_SOURCES += $(QMP_MPI_SRC)
else
  EXTRA_libqmp_a_SOURCES += $(QMP_MPI_SRC)
endif

if QMP_BGSPI
  libqmp_a_SOURCES += $(QMP_BGSPI_SRC)
else
  EXTRA_libqmp_a_SOURCES += $(QMP_BGSPI_SRC)
endif
