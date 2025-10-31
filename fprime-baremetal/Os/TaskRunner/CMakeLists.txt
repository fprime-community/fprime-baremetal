####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
# Fw/Time/TimeInterval.hpp is included by Os/Task.hpp
set(MOD_DEPS Fw_Types Os)
set(SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/TaskRunner.cpp")
register_fprime_module()
