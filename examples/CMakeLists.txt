foreach(prog QMP_test 
                      QMP_msg
                      QMP_grid_test
                      QMP_loopback
                      QMP_qcd_test
                      QMP_stride_test
                      QMP_perf
                      QMP_broadcast
                      QMP_gcomm_perf
                      QMP_MILC_test
                      QMP_show_geom)

add_executable(${prog} "${prog}.c"  )
target_link_libraries(${prog} PUBLIC QMP::qmp m)
set_target_properties(${prog} PROPERTIES C_STANDARD 99)
set_target_properties(${prog} PROPERTIES C_EXTENSIONS OFF)
install(TARGETS ${prog} DESTINATION examples )

endforeach()
