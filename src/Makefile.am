## Process this file with automake to produce Makefile.in

lib_LIBRARIES = libqmp.a

QMP_GENERIC_SRC = QMP_grid.c \
                  qmp.h

QMP_GM_SRC = QMP_comm_gm.c     \
             QMP_init_gm.c     \
             QMP_mem_gm.c      \
             QMP_topology_gm.c \
             QMP_util_gm.c     \
             QMP_genutil.c     \
             QMP_P_GM.h

QMP_MPI_SRC = QMP_comm_mpi.c \
              QMP_init_mpi.c \
              QMP_mem_mpi.c  \
              QMP_util_mpi.c \
              QMP_genutil.c  \
              QMP_P_MPI.h

libqmp_a_CFLAGS  = @QMP_COMMS_CFLAGS@

## GTF: if qmp.hpp needs to end up in @prefix@/include, then add it here
include_HEADERS = qmp.h

if QMP_GM
  libqmp_a_SOURCES = $(QMP_GENERIC_SRC) $(QMP_GM_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_MPI_SRC)
endif

if QMP_MPI
  libqmp_a_SOURCES = $(QMP_GENERIC_SRC) $(QMP_MPI_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_GM_SRC)
endif

## GTF: if qmp.hpp is just going along for the ride in the dist until it proves
## to be useful but is not currently not to be installed anywhere, put it here.
EXTRA_DIST=qmp.hpp


#
# The QMP_run script -- this will also get installed into the ./bin directory
#
bin_SCRIPTS = QMP_run
