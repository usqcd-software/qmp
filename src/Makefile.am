## Process this file with automake to produce Makefile.in

lib_LIBRARIES = libqmp.a

QMP_GM_SRC = QMP_comm_gm.c     \
             QMP_init_gm.c     \
             QMP_mem_gm.c      \
             QMP_topology_gm.c \
             QMP_util_gm.c     \
             QMP_grid.c        \
             QMP_P_GM.h

QMP_MPI_SRC = QMP_comm_mpi.c \
              QMP_init_mpi.c \
              QMP_mem_mpi.c  \
              QMP_util_mpi.c \
              QMP_grid.c \
              QMP_P_MPI.h

libqmp_a_CFLAGS  = @QMP_COMMS_CFLAGS@

include_HEADERS = QMP.h

if QMP_GM
  libqmp_a_SOURCES = $(QMP_GM_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_MPI_SRC)
endif

if QMP_MPI
  libqmp_a_SOURCES = $(QMP_MPI_SRC)
  EXTRA_libqmp_a_SOURCES = $(QMP_GM_SRC)
endif

#
# The QMP_run script -- this will also get installed intot the ./bin directory
#
bin_SCRIPTS = QMP_run
