## Process this file with automake to produce Makefile.in

lib_LIBRARIES = libqmp.a

QMP_SINGLE_SRC = QMP_comm_single.c  \
                 QMP_error_single.c \
                 QMP_init_single.c  \
                 QMP_mem_single.c   \
                 QMP_P_SINGLE.h

QMP_MPI_SRC = QMP_comm_mpi.c  \
              QMP_error_mpi.c \
              QMP_init_mpi.c  \
              QMP_mem_mpi.c   \
              QMP_P_MPI.h

QMP_COMMON_SRC = QMP_grid.c     \
                 QMP_machine.c  \
                 QMP_topology.c \
                 QMP_util.c     \
	         QMP_P_COMMON.h \
                 qmp.h


libqmp_a_CFLAGS  = @QMP_COMMS_CFLAGS@

## GTF: if qmp.hpp needs to end up in @prefix@/include, then add it here
include_HEADERS = qmp.h

if QMP_SINGLE
  libqmp_a_SOURCES = $(QMP_SINGLE_SRC) $(QMP_COMMON_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_MPI_SRC)
endif

if QMP_MPI
  libqmp_a_SOURCES = $(QMP_MPI_SRC) $(QMP_COMMON_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_SINGLE_SRC)
endif

## GTF: if qmp.hpp is just going along for the ride in the dist until it proves
## to be useful but is not currently not to be installed anywhere, put it here.
EXTRA_DIST=qmp.hpp
