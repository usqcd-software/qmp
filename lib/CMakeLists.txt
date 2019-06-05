set(QMP_SRC
   QMP_comm.c
   QMP_error.c
   QMP_grid.c
   QMP_init.c
   QMP_machine.c
   QMP_mem.c
   QMP_split.c
   QMP_topology.c
   QMP_util.c)
   

if( QMP_MPI )    
  list(APPEND QMP_SRC
    mpi/QMP_comm_mpi.c
    mpi/QMP_error_mpi.c
    mpi/QMP_init_mpi.c
    mpi/QMP_mem_mpi.c
    mpi/QMP_split_mpi.c
    mpi/QMP_topology_mpi.c)
endif( QMP_MPI)
   
   
#BGSPI Soures
if( QMP_BGSPI ) 
 list(APPEND QMP_SRC
    bgspi/QMP_comm_bgspi.c
    bgspi/QMP_init_bgspi.c
    bgspi/QMP_mem_bgspi.c
    bgspi/qspi.c)
endif( QMP_BGSPI) 

# define target
add_library(qmp ${QMP_SRC})


	
set_target_properties(qmp PROPERTIES C_STANDARD 99)
set_target_properties(qmp PROPERTIES C_EXTENSIONS OFF)


# Configure the target
# Includes from source
target_include_directories(qmp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<INSTALL_INTERFACE:include>)

# generated includes
target_include_directories(qmp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
  $<INSTALL_INTERFACE:include>)
  
# Use the MPI Library 
if( QMP_MPI )
  target_link_libraries(qmp PUBLIC MPI::MPI_C)
endif(QMP_MPI) 
