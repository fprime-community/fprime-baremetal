# -----------------------------------------
## Shared Section
# -----------------------------------------

set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/error.cpp"
)
set(HEADER_FILES
    "${CMAKE_CURRENT_LIST_DIR}/error.hpp"
)
register_fprime_module(Os_Baremetal_Shared)

# Setup MicroFs
set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/MicroFs.cpp"
)
set(HEADER_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/MicroFs.hpp"
)
register_fprime_module(Os_Baremetal_MicroFs)

register_os_implementation("Cpu" Baremetal)
register_os_implementation("File;FileSystem;Directory" Baremetal_MicroFs Os_Baremetal_Shared Os_Baremetal_MicroFs)
register_os_implementation("Memory" Baremetal)
register_os_implementation("Mutex;ConditionVariable" Baremetal)
register_os_implementation("Task" Baremetal fprime-baremetal_Os_TaskRunner)

add_library(fprime-baremetal_Os_Baremetal INTERFACE)
set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MicroFsTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/Tester.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MyRules.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/SimFileSystem.cpp"
)
choose_fprime_implementation(Os_File Os_File_Baremetal_MicroFs)
set(UT_MOD_DEPS Os STest)
register_fprime_ut()