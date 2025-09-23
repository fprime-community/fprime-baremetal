#include <gtest/gtest.h>
#include "../atomic_test.hpp"

TEST(StdAtomic, Construction) {
    TestStdAtomic::std::atomic<int> atomic_init(42);
    EXPECT_EQ(atomic_init.load(), 42);
}

TEST(StdAtomic, AssignmentOperator) {
    TestStdAtomic::std::atomic<int> atomic_int(0);

    atomic_int = 100;
    EXPECT_EQ(atomic_int.load(), 100);

    // Test that assignment returns the assigned value
    int result = (atomic_int = 200);
    EXPECT_EQ(result, 200);
    EXPECT_EQ(atomic_int.load(), 200);
}

TEST(StdAtomic, ConversionOperator) {
    TestStdAtomic::std::atomic<int> atomic_int(42);

    // Test implicit conversion to T
    int value = atomic_int;
    EXPECT_EQ(value, 42);
}

TEST(StdAtomic, IncrementOperators) {
    TestStdAtomic::std::atomic<int> counter(0);

    // Test pre-increment
    EXPECT_EQ(++counter, 1);
    EXPECT_EQ(counter.load(), 1);

    // Test post-increment
    EXPECT_EQ(counter++, 1);
    EXPECT_EQ(counter.load(), 2);
}

TEST(StdAtomic, DecrementOperators) {
    TestStdAtomic::std::atomic<int> counter(2);

    // Test pre-decrement
    EXPECT_EQ(--counter, 1);
    EXPECT_EQ(counter.load(), 1);

    // Test post-decrement
    EXPECT_EQ(counter--, 1);
    EXPECT_EQ(counter.load(), 0);
}

TEST(StdAtomic, LoadAndStore) {
    TestStdAtomic::std::atomic<int> atomic_int(0);

    // Test store
    atomic_int.store(50);
    EXPECT_EQ(atomic_int.load(), 50);

    // Test load
    int value = atomic_int.load();
    EXPECT_EQ(value, 50);
}

TEST(StdAtomic, CompareExchangeWeak) {
    TestStdAtomic::std::atomic<int> counter(50);

    // Test successful exchange
    int expected = 50;
    bool result = counter.compare_exchange_weak(expected, 75);
    EXPECT_TRUE(result);
    EXPECT_EQ(counter.load(), 75);
    EXPECT_EQ(expected, 50); // expected should not change on success

    // Test failed exchange
    expected = 50; // Wrong expected value
    result = counter.compare_exchange_weak(expected, 100);
    EXPECT_FALSE(result);
    EXPECT_EQ(expected, 75); // expected should be updated to current value
    EXPECT_EQ(counter.load(), 75); // value should remain unchanged
}

TEST(StdAtomic, CompareExchangeStrong) {
    TestStdAtomic::std::atomic<int> counter(75);

    // Test successful exchange
    int expected = 75;
    bool result = counter.compare_exchange_strong(expected, 200);
    EXPECT_TRUE(result);
    EXPECT_EQ(counter.load(), 200);

    // Test failed exchange
    expected = 100; // Wrong expected value
    result = counter.compare_exchange_strong(expected, 300);
    EXPECT_FALSE(result);
    EXPECT_EQ(expected, 200); // expected should be updated to current value
    EXPECT_EQ(counter.load(), 200); // value should remain unchanged
}

TEST(StdAtomic, FetchAdd) {
    TestStdAtomic::std::atomic<int> counter(200);

    // Test fetch_add
    int old_value = counter.fetch_add(50);
    EXPECT_EQ(old_value, 200);
    EXPECT_EQ(counter.load(), 250);

    // Test with negative value
    old_value = counter.fetch_add(-100);
    EXPECT_EQ(old_value, 250);
    EXPECT_EQ(counter.load(), 150);
}

TEST(StdAtomic, FetchSub) {
    TestStdAtomic::std::atomic<int> counter(250);

    // Test fetch_sub
    int old_value = counter.fetch_sub(100);
    EXPECT_EQ(old_value, 250);
    EXPECT_EQ(counter.load(), 150);

    // Test with negative value (which adds)
    old_value = counter.fetch_sub(-50);
    EXPECT_EQ(old_value, 150);
    EXPECT_EQ(counter.load(), 200);
}

TEST(StdAtomic, DifferentTypes) {
    // Test with bool
    TestStdAtomic::std::atomic<bool> atomic_bool(false);
    atomic_bool = true;
    EXPECT_TRUE(atomic_bool.load());

    // Test with char
    TestStdAtomic::std::atomic<char> atomic_char('A');
    EXPECT_EQ(atomic_char.load(), 'A');
    atomic_char.store('B');
    EXPECT_EQ(atomic_char.load(), 'B');

    // Test with unsigned int
    TestStdAtomic::std::atomic<unsigned int> atomic_uint(100u);
    EXPECT_EQ(atomic_uint.load(), 100u);
    atomic_uint.fetch_add(50u);
    EXPECT_EQ(atomic_uint.load(), 150u);
}

TEST(StdAtomic, CopySemantics) {
    // TestStdAtomic::std::atomic<int> atomic1(42);

    // Verify that copy constructor is deleted (this should not compile)
    // TestStdAtomic::std::atomic<int> atomic2(atomic1); // This line would cause compilation error

    // Verify that copy assignment is deleted (this should not compile)
    // TestStdAtomic::std::atomic<int> atomic3;
    // atomic3 = atomic1; // This line would cause compilation error

    // But assignment from T should work
    TestStdAtomic::std::atomic<int> atomic4;
    atomic4 = 42;
    EXPECT_EQ(atomic4.load(), 42);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}