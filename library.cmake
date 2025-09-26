if(FPRIME_BAREMENTAL_OVERRIDE_NEW_DELETE)
    add_definitions(-DFPRIME_BAREMENTAL_OVERRIDE_NEW_DELETE)
endif()
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fprime-baremetal/config")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fprime-baremetal/Os")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fprime-baremetal/Svc")

