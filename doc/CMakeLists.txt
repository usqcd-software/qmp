  # set input and output files
 find_package(Doxygen)
 if( DOXYGEN_FOUND ) 
 
    # You can leave CMakeVariable substitutions etc in the QMPdoxyfile
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/QMPdoxyfile)
    
    # This creates a doxyfile with CMake @vars substituted
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/QMPdoxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    
    message("Doxygen build started")
    # Build the doxygen with custom target
    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
        
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/oxydocs DESTINATION doc)
    
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)