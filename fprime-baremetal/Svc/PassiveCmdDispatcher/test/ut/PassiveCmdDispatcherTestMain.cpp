// ----------------------------------------------------------------------
// PassiveCmdDispatcherTestMain.cpp
// ----------------------------------------------------------------------

#include <gtest/gtest.h>
#include "PassiveCmdDispatcherTester.hpp"
#include "STest/Random/Random.hpp"

TEST(PassiveCmdDispatcher, Nominal) {
    Baremetal::PassiveCmdDispatcherTester tester;
    tester.testNominal();
}

TEST(PassiveCmdDispatcher, OffNominal) {
    Baremetal::PassiveCmdDispatcherTester tester;
    tester.testOffNominal();
}

TEST(PassiveCmdDispatcher, TooManyCommands) {
    Baremetal::PassiveCmdDispatcherTester tester;
    tester.testTooManyCommands();
}

TEST(PassiveCmdDispatcher, FailuresWhilePending) {
    Baremetal::PassiveCmdDispatcherTester tester;
    tester.testFailuresWhilePending();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
