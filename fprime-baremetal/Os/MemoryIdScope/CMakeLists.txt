####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
register_fprime_module(
    Os_Baremetal_MemoryIdScope
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/MemoryIdScope.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/MemoryIdScope.hpp"
    DEPENDS
        Fw_Types
)
