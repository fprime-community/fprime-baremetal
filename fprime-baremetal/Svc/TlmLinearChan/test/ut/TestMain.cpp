// ----------------------------------------------------------------------
// TestMain.cpp
// ----------------------------------------------------------------------

#include <gtest/gtest.h>
#include <Fw/Test/UnitTest.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include "TlmLinearChanTester.hpp"

namespace {

class TrackingAllocator final : public Fw::MallocAllocator {
  public:
    TrackingAllocator() : m_deallocateCount(0), m_lastAllocate(nullptr), m_lastDeallocate(nullptr), m_lastDeallocateId(0) {}

    void* allocate(const FwEnumStoreType identifier,
                   FwSizeType& size,
                   bool& recoverable,
                   FwSizeType alignment = alignof(std::max_align_t)) override {
        this->m_lastAllocate = Fw::MallocAllocator::allocate(identifier, size, recoverable, alignment);
        return this->m_lastAllocate;
    }

    void deallocate(const FwEnumStoreType identifier, void* ptr) override {
        this->m_deallocateCount++;
        this->m_lastDeallocateId = identifier;
        this->m_lastDeallocate = ptr;
        Fw::MallocAllocator::deallocate(identifier, ptr);
    }

    FwSizeType m_deallocateCount;
    void* m_lastAllocate;
    void* m_lastDeallocate;
    FwEnumStoreType m_lastDeallocateId;
};

}  // namespace

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

TEST(TlmLinearChanTest, DeinitReleasesAllocatorStorage) {
    TrackingAllocator allocator;
    {
        Baremetal::TlmLinearChan component("TlmLinearChan");
        component.setup(42, allocator);
        component.init(10, 0);

        ASSERT_EQ(0, allocator.m_deallocateCount);
        component.deinit();

        EXPECT_EQ(1, allocator.m_deallocateCount);
        EXPECT_EQ(42, allocator.m_lastDeallocateId);
        EXPECT_EQ(allocator.m_lastAllocate, allocator.m_lastDeallocate);
    }

    EXPECT_EQ(1, allocator.m_deallocateCount);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
