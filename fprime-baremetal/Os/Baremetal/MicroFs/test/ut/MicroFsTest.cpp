#include <gtest/gtest.h>
#include <Fw/Test/UnitTest.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include <Fw/Types/String.hpp>
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include "Tester.hpp"

#define FULL_TEST
#define NUKE_TEST
#define OFF_NOMINAL
#define NEW_TEST
// #define SIM_FILE_TEST

#ifdef FULL_TEST

TEST(Initialization, InitTest) {
    Os::Tester tester;
    tester.InitTest();
}

TEST(FileOps, OpenWriteReadTest) {
    Os::Tester tester;
    tester.OpenWriteReadTest();
}

TEST(FileOps, OpenWriteTwiceReadOnceTest) {
    Os::Tester tester;
    tester.OpenWriteTwiceReadOnceTest();
}

TEST(FileOps, OpenWriteOnceReadTwiceTest) {
    Os::Tester tester;
    tester.OpenWriteOnceReadTwiceTest();
}

TEST(FileSystemOps, ListTest) {
    Os::Tester tester;
    tester.ListTest();
}

TEST(FileSystemOps, OpenFreeSpaceTest) {
    Os::Tester tester;
    tester.OpenFreeSpaceTest();
}

TEST(FileOps, OpenStressTest) {
    Os::Tester tester;
    tester.OpenStressTest();
}

TEST(FileOps, FileSizeTest) {
    Os::Tester tester;
    tester.FileSizeTest();
}

TEST(FileOps, ReWriteTest) {
    Os::Tester tester;
    tester.ReWriteTest();
}

TEST(FileOps, DirectoryTest) {
    Os::Tester tester;
    tester.DirectoryTest();
}

TEST(FileOps, MoveTest) {
    Os::Tester tester;
    tester.MoveTest();
}

TEST(FileOps, SeekTest) {
    Os::Tester tester;
    tester.SeekTest();
}

TEST(FileOps, CrcTest) {
    Os::Tester tester;
    tester.CrcTest();
}

TEST(FileOps, OddTests) {
    Os::Tester tester;
    tester.OddTests();
}

TEST(FileOps, CopyTest) {
    Os::Tester tester;
    tester.CopyTest();
}

TEST(FileOps, AppendTest) {
    Os::Tester tester;
    tester.AppendTest();
}

#endif

#ifdef NUKE_TEST
TEST(FileOps, NukeTest) {
    Os::Tester tester;
    tester.NukeTest();
}
#endif

#ifdef OFF_NOMINAL
TEST(FileOps, OffNominalTests) {
    Os::Tester tester;
    tester.OffNominalTests();
}
#endif

#ifdef SIM_FILE_TEST
TEST(FileOps, SimFileTest) {
    Os::Tester tester;
    tester.SimFileTest();
}
#endif

#ifdef NEW_TEST
TEST(FileOps, NewTest) {
    Os::Tester tester;
    tester.NewTest();
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
