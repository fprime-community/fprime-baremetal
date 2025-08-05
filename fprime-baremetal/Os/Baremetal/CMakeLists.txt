####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
add_custom_target("${FPRIME_CURRENT_MODULE}")

# -----------------------------------------
## Shared Section
# -----------------------------------------

register_fprime_module(
    Os_Baremetal_Shared
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/error.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/error.hpp"
    DEPENDS
        Fw_Types
)

# Setup MicroFs
register_fprime_module(
    Os_Baremetal_MicroFs
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/MicroFs.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/MicroFs.hpp"
    DEPENDS
        Fw_Types
)

# Set up Baremetal implementation
register_os_implementation("Cpu" Baremetal)
register_os_implementation("Memory" Baremetal)
register_os_implementation("Task" Baremetal fprime-baremetal_Os_TaskRunner)
register_os_implementation("File;FileSystem;Directory" Baremetal_MicroFs Os_Baremetal_Shared Os_Baremetal_MicroFs)

# -----------------------------------------
# MicroFs Full Test Section
# -----------------------------------------

register_fprime_ut(
    MicroFsFullTest
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MicroFsTest.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/Tester.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MyRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/SimFileSystem.cpp"
    DEPENDS
       Os
       STest
    CHOOSES_IMPLEMENTATIONS
        Os_File_Baremetal_MicroFs
)

# -----------------------------------------
# MicroFs File Test Section
# -----------------------------------------

register_fprime_ut(
    MicroFsFileTest
    SOURCES
        "${FPRIME_FRAMEWORK_PATH}/Os/test/ut/file/CommonTests.cpp"
        "${FPRIME_FRAMEWORK_PATH}/Os/test/ut/file/FileRules.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/MicroFs/test/ut/MicroFsFileTests.cpp"
    DEPENDS
        Os
        Os_RawTime
        STest
        Os_Test_File_SyntheticFileSystem
    CHOOSES_IMPLEMENTATIONS
        Os_File_Baremetal_MicroFs
)
