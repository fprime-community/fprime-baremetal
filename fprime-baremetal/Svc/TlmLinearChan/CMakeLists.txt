####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/TlmLinearChan.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/TlmLinearChan.cpp"
)
set(MOD_DEPS
        Os_Baremetal_MemoryIdScope
)

register_fprime_module()

### UTs ###
set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/TlmLinearChan.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TestMain.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/TlmLinearChanTester.cpp"
)

register_fprime_ut()
