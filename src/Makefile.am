## Process this file with automake to produce Makefile.in

lib_LIBRARIES = libqmp.a

if QMP_GM
QMP_GM_SRC = QMP_comm_gm.c     \
             QMP_init_gm.c     \
             QMP_mem_gm.c      \
             QMP_topology_gm.c \
             QMP_util_gm.c     \
	     QMP_grid.c        \
             QMP_P_GM.h
endif

if QMP_MPI
QMP_MPI_SRC = QMP_comm_mpi.c \
              QMP_init_mpi.c \
              QMP_mem_mpi.c  \
              QMP_util_mpi.c \
              QMP_grid.c \
              QMP_P_MPI.h
endif

libqmp_a_CFLAGS  = @QMP_COMMS_CFLAGS@

include_HEADERS = QMP.h

libqmp_a_SOURCES = $(QMP_GM_SRC) $(QMP_MPI_SRC)

#
# The QMP_run script -- this will also get installed intot the ./bin directory
#
bin_SCRIPTS = QMP_run
