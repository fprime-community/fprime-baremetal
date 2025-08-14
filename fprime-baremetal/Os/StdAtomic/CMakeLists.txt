set(SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/empty.cpp")
register_fprime_module()


set(UT_SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/test/ut/StdAtomic.cpp"
)
set(UT_MOD_DEPS
  gtest
)
register_fprime_ut()