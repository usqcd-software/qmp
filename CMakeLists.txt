cmake_minimum_required(VERSION 3.17)
project(QMP VERSION 2.5.3 LANGUAGES C)

# Boiler plate -- install directories
include(GNUInstallDirs)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Boiler plate -- CTest
include(CTest)

#Options
# --enable-extra-debugging 
# This is a boolean so use option
option(QMP_EXTRA_DEBUG "Enable extra debugging" OFF)

# --enable-profiling
option(QMP_PROFILING "Enable Profiling" OFF)

# --enabe-timing
option(QMP_TIMING "Enable Timing" OFF)

# Change MPI build to Boolean
# If true use MPI
# if false do single
option(QMP_MPI "Enable MPI" OFF)

# Deprecating BGL and BGP personalities

# --enable-bgq
option(QMP_BGQ "Enable BlueGene/Q Personality to set native machine geometry" OFF)

# --enable-bgspi
option(QMP_BGSPI "Enable BlueGene SPI" OFF)

# --enable-testing
option(QMP_TESTING "Enable buidling of the examples" ON)

# Enable Address address and undefined behaviour sanitizers
option(QMP_ENABLE_SANITIZERS "Enable Address and Undefined Behaviour Sanitizers" OFF)

# Configuration file
configure_file(include/qmp_config.h.cmake.in 
                      include/qmp_config.h)
                      
              
set(QMP_COMMS_CFLAGS "" CACHE STRING "Any extra comms flags")
set(QMP_COMMS_LDFLAGS "" CACHE STRING "Any extra comms LDFLAGS")
set(QMP_COMMS_LIBS "" CACHE STRING "Any extra comms libs")

# Use DMalloc
option(QMP_USE_DMALLOC "Enable DMalloc" OFF)

# Build Docs
option(QMP_BUILD_DOCS "Enable Building docs with Doxygen" OFF)

                      


if(QMP_USE_DMALLOC)
find_package(Dmalloc REQUIRED)
endif(QMP_USE_DMALLOC)   


set(QMP_COMMS_TYPE "single")

if(QMP_MPI) 
   # Languages setting in PROJECT to C will require only C version of MPI
   find_package(MPI REQUIRED)
   set(HAVE_MPI 1)
   set(QMP_COMMS_TYPE "MPI")
endif(QMP_MPI)

if(QMP_EXTRA_DEBUG)
  set(_QMP_DEBUG 1)
  set(_QMP_TRACE 1)
endif(QMP_EXTRA_DEBUG)

if(QMP_PROFILING)
  set(QMP_BUILD_PROFILING 1)
endif(QMP_PROFILING)  

if(QMP_TIMING)
  set(QMP_BUILD_TIMING 1)
endif(QMP_TIMING)

if(QMP_BGQ)
  set(HAVE_BGQ 1)
endif(QMP_BGQ)

if(QMP_BGSPI)
 set(HAVE_BGSPI 1)
endif(QMP_BGSPI)

# Deal with Sanitizer
if( QMP_ENABLE_SANITIZERS )
			include(cmake/CheckSanitizeOpts.cmake)
			check_sanitizer_options( "${QMP_ENABLE_SANITIZERS}" QMP_SANITIZER_OPTS )
			message(STATUS "QMP: Setting Sanitizer options: ${QMP_SANITIZER_OPTS}")
endif()  	 
 
# Configuration file
configure_file(include/qmp_config.h.cmake.in  include/qmp_config.h)
configure_file(bin/qmp-config.cmake.in  bin/qmp-config @ONLY)


add_subdirectory(lib)            
if( QMP_TESTING ) 
  add_subdirectory(examples)
endif()

if( QMP_BUILD_DOCS ) 
  add_subdirectory(doc)
endif()        

# Installs the targers (The QMP Library)
# into the 'QMPTargets' export pack


install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/qmp-config DESTINATION bin)
install(DIRECTORY include DESTINATION .  )
	  
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindDmalloc.cmake DESTINATION lib/cmake/QMP)



include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    QMPConfigVersion.cmake
    VERSION "${PACKAGE_VERSION}"
    COMPATIBILITY AnyNewerVersion
    )

# Using this macro we can pass some path vars down...
# But we are not doing this for now.
set( cmakeModulesDir "lib/cmake/QMP" )
configure_package_config_file(QMPConfig.cmake.in QMPConfig.cmake 
	INSTALL_DESTINATION lib/cmake/QMP
	PATH_VARS
		cmakeModulesDir
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QMPConfigVersion.cmake ${CMAKE_CURRENT_BINARY_DIR}/QMPConfig.cmake DESTINATION lib/cmake/QMP)
