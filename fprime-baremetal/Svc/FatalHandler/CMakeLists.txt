####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/FatalHandler.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/FatalHandler.cpp"
)
set(MOD_DEPS
  Fw/Logger
)
register_fprime_module()
