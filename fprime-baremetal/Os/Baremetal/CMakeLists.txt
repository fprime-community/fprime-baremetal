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

add_library(fprime-baremetal_Os_Baremetal INTERFACE)

add_fprime_supplied_os_module(Cpu Baremetal)
register_os_implementation("File;FileSystem;Directory" Baremetal_MicroFs Os_Baremetal_Shared Os_Baremetal_MicroFs)
add_fprime_supplied_os_module(Memory Baremetal)
add_fprime_supplied_os_module(Task Baremetal fprime-baremetal_Os_TaskRunner)

# -----------------------------------------
# MicroFs Full Test Section
# -----------------------------------------

set(UT_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MicroFsTest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/Tester.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MyRules.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/SimFileSystem.cpp"
)

add_fprime_os_test(MicroFsFullTest "${UT_SOURCE_FILES}" "Os/File\;Os/File/Baremetal/MicroFs")

# -----------------------------------------
# MicroFs File Test Section
# -----------------------------------------

set(UT_SOURCE_FILES
    "${FPRIME_FRAMEWORK_PATH}/Os/test/ut/file/CommonTests.cpp"
    "${FPRIME_FRAMEWORK_PATH}/Os/test/ut/file/FileRules.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MicroFsFileTests.cpp"
)

add_fprime_os_test(MicroFsFileTest "${UT_SOURCE_FILES}" "Os/File\;Os/File/Baremetal/MicroFs" Os_Test_File_SyntheticFileSystem)
