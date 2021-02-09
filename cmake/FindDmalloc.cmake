# Find Module for DMalloc
#
# 

# If the user has passed a Dmalloc_DIR cache variable
# then make up relative lib and include paths
#
message(STATUS "Looking for DMalloc")
if( Dmalloc_DIR )
  set( Dmalloc_DIR_LIBDIR ${Dmalloc_DIR}/lib )
  set( Dmalloc_DIR_INCDIR ${Dmalloc_DIR}/include )
endif()

# Look for the library file
find_library(Dmalloc_LIBRARY 
	NAMES dmalloc 
	PATHS /usr/lib /usr/local/lib ${Dmalloc_DIR_LIBDIR} 
	REQUIRED
)

# Look for the include file
find_path(Dmalloc_INCLUDE_DIR 
	NAMES dmalloc.h 
	PATHS /usr/include /usr/local/include ${Dmalloc_DIR_INCDIR}
	REQUIRED
)
message( STATUS "Found dmalloc.h in ${Dmalloc_INCLUDE_DIR}")
message( STATUS "Found dmalloc library in ${Dmalloc_LIBRARY}")

set(Dmalloc_FOUND TRUE)

# Create a fake imported library 
add_library(dmalloc UNKNOWN IMPORTED)

# Set the location to be the library file we found
set_property(TARGET dmalloc PROPERTY
             IMPORTED_LOCATION "${Dmalloc_LIBRARY}")
             
# Add the include directory as a usage requirement, and
# Mark it as system so we don't care about wraning from the include dir
target_include_directories(dmalloc INTERFACE ${Dmalloc_INCLUDE_DIR})
		
add_library(Dmalloc::dmalloc ALIAS dmalloc) 
set(WITH_DMALLOC "1")
