add_library(qmp)
target_sources(qmp PRIVATE
  	QMP_comm.c
   	QMP_error.c
   	QMP_grid.c
   	QMP_init.c
   	QMP_machine.c
   	QMP_mem.c
   	QMP_split.c
   	QMP_topology.c
   	QMP_util.c
)


# Append MPI Sources 
if( QMP_MPI )    
	target_sources(qmp PRIVATE
    	mpi/QMP_comm_mpi.c
    	mpi/QMP_error_mpi.c
    	mpi/QMP_init_mpi.c
    	mpi/QMP_mem_mpi.c
    	mpi/QMP_split_mpi.c
    	mpi/QMP_topology_mpi.c)
endif()
   
# Append BGSPI Sources -- Deprecate this?
if( QMP_BGSPI ) 
	target_sources(qmp PRIVATE
    	bgspi/QMP_comm_bgspi.c
    	bgspi/QMP_init_bgspi.c
    	bgspi/QMP_mem_bgspi.c
    	bgspi/qspi.c)
endif()

set_target_properties(qmp PROPERTIES C_STANDARD 99)
set_target_properties(qmp PROPERTIES C_EXTENSIONS OFF)
if( QMP_SANITIZER_OPTS )
  target_compile_options(qmp PRIVATE ${QMP_SANITIZER_OPTS} )
  target_link_options(qmp PUBLIC ${QMP_SANITIZER_OPTS} )
endif()

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

if( QMP_USE_DMALLOC )
  target_link_libraries(qmp PUBLIC Dmalloc::dmalloc )
endif()

# Install the targes
install(TARGETS qmp
        EXPORT QMPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)


install(EXPORT QMPTargets
        FILE QMPTargets.cmake
        NAMESPACE QMP::
        DESTINATION lib/cmake/QMP
)
# Executables can link with QMP::qmp 
add_library(QMP::qmp ALIAS qmp)
