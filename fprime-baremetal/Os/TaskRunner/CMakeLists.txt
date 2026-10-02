####
# F prime CMakeLists.txt:
#
# SOURCES: source files
# DEPENDS: module dependencies
#
####
# Fw/Time/TimeInterval.hpp is included by Os/Task.hpp
register_fprime_module(
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/TaskRunner.cpp"
    DEPENDS
        Fw_Types
        Os
)

register_fprime_ut(
    TaskRunnerTest
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/TaskRunnerTests.cpp"
    DEPENDS
        Os
        STest
        fprime-baremetal_Os_TaskRunner
    CHOOSES_IMPLEMENTATIONS
        Os_Task_Baremetal
)
