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
register_os_implementation("File;FileSystem;Directory" Baremetal Os_Baremetal_Shared Os_Baremetal_MicroFs)
register_os_implementation("Memory" Baremetal)
register_os_implementation("Mutex;ConditionVariable" Baremetal)
register_os_implementation("Task" Baremetal fprime-baremetal_Os_TaskRunner)
