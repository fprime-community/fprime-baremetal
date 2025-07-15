####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
set(MOD_DEPS Fw_Types Fw_Time)
set(SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/TaskRunner.cpp")
register_fprime_module()
