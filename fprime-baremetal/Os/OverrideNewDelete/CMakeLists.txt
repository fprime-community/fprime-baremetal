####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
register_fprime_module(
    Os_Baremetal_OverrideNewDelete
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/OverrideNewDelete.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/OverrideNewDelete.hpp"
    DEPENDS
        Fw_Types
        Os_Baremetal_MemoryIdScope
)
