// ----------------------------------------------------------------------
// TestMain.cpp
// ----------------------------------------------------------------------

#include <gtest/gtest.h>
#include <Fw/Test/UnitTest.hpp>
#include "TlmLinearChanTester.hpp"
#include <Fw/Types/MallocAllocator.hpp>
// FIXME: This should be able to go away once https://github.com/nasa/fprime/issues/4039 is implemented
#include "fprime-baremetal/Os/OverrideNewDelete/OverrideNewDelete.hpp"

static Fw::MallocAllocator mallocator;

TEST(TlmLinearChanTest, InitTest) {
    Baremetal::TlmLinearChanTester tester;
}

TEST(TlmLinearChanTest, NominalChannelTest) {
    TEST_CASE(107.1.1, "Nominal channelized telemetry");
    COMMENT("Write a single channel and verify it is read back and pushed correctly.");

    Baremetal::TlmLinearChanTester tester;
    // run test
    tester.runNominalChannel();
}

TEST(TlmLinearChanTest, MultiChannelTest) {
    TEST_CASE(107.1.2, "Nominal Multi-channel channelized telemetry");
    COMMENT("Write multiple channels and verify they are read back and pushed correctly.");

    Baremetal::TlmLinearChanTester tester;

    // run test
    tester.runMultiChannel();
}

TEST(TlmLinearChanTest, OffNominal) {
    TEST_CASE(107.2.1, "Off-nominal channelized telemetry");
    COMMENT("Attempt to read a channel that hasn't been written.");

    Baremetal::TlmLinearChanTester tester;

    // run test
    tester.runOffNominal();
}

int main(int argc, char** argv) {
    Os::Baremetal::OverrideNewDelete::registerMemAllocator(&mallocator);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}