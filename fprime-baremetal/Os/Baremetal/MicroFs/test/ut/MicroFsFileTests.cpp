// ======================================================================
// \title Os/Posix/test/ut/PosixFileTests.cpp
// \brief tests for posix implementation for Os::File
// ======================================================================
#include <gtest/gtest.h>
#include <unistd.h>
#include <Fw/Types/MallocAllocator.hpp>
#include <csignal>
#include <cstdio>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include <list>
#include "config/MicroFsCfg.hpp"
#include "Os/File.hpp"
#include "Os/test/ut/file/CommonTests.hpp"
#include "STest/Pick/Pick.hpp"
namespace Os {
namespace Test {
namespace File {

std::vector<std::shared_ptr<const std::string> > FILES;

Fw::MallocAllocator alloc;

static const U32 MAX_FILES = 100;
static constexpr U16 MAX_FILE_PATH = 256;
static const char BASE_PATH[] = "/" MICROFS_BIN_STRING "0";
static const char TEST_FILE[] = MICROFS_FILE_STRING;
static constexpr U32 MAX_BINS = 10;
// Use 32K because that is what F Prime's Os::File UTs uses
static constexpr U32 FILE_SIZE = 32 * 1024;
static constexpr U32 MAX_FILES_PER_BIN = MAX_FILES;

//! Check if we can use the file. F_OK file exists, R_OK, W_OK are read and write.
//! \return true if it exists, false otherwise.
//!
bool check_permissions(const char* path, int permission) {
    return true;
}

//! Get a filename, randomly if random is true, otherwise use a basic filename.
//! \param random: true if filename should be random, false if predictable
//! \return: filename to use for testing
//!
std::shared_ptr<std::string> get_test_filename(bool random) {
    const char* filename = TEST_FILE;
    char full_buffer[MAX_FILE_PATH];
    char buffer[MAX_FILE_PATH - sizeof(BASE_PATH)];
    // When random, select random characters
    FwIndexType fileIndex = 0;
    if (random) {
        fileIndex = STest::Pick::lowerUpper(0, MAX_FILES_PER_BIN - 1);
    }
    (void)snprintf(full_buffer, MAX_FILE_PATH, "%s/%s%" PRI_FwIndexType, BASE_PATH, filename, fileIndex);
    // Create a shared pointer wrapping our filename buffer
    std::shared_ptr<std::string> pointer(new std::string(full_buffer), std::default_delete<std::string>());
    return pointer;
}

//! Clean-up the files created during this test.
//!
void cleanup(int signal) {
    // Ensure the test files are removed only when the test was run
    // for (const auto& val : FILES) {
    // if (check_permissions(val->c_str(), F_OK)) {
    //::unlink(val->c_str());
    //}
    //}
    FILES.clear();
    Os::Baremetal::MicroFs::MicroFsCleanup(0, alloc);
}

//! Set up for the test ensures that the test can run at all
//!
void setUp(bool requires_io) {
    std::shared_ptr<std::string> non_random_filename = get_test_filename(false);
    int signals[] = {SIGQUIT, SIGABRT, SIGTERM, SIGINT, SIGHUP};
    for (unsigned long i = 0; i < FW_NUM_ARRAY_ELEMENTS(signals); i++) {
        // Could not register signal handler
        if (signal(SIGQUIT, cleanup) == SIG_ERR) {
            GTEST_SKIP() << "Cannot register signal handler for cleanup";
        }
    }

    U32 numBins = STest::Pick::lowerUpper(1, MAX_BINS);
    U32 fileSize = FILE_SIZE;
    U32 numFiles = MAX_FILES_PER_BIN;

    Os::Baremetal::MicroFs::MicroFsConfig testCfg;

    Os::Baremetal::MicroFs::MicroFsSetCfgBins(testCfg, numBins);

    for (U16 i = 0; i < numBins; i++) {
        Os::Baremetal::MicroFs::MicroFsAddBin(testCfg, i, fileSize, numFiles);
    }

    Os::Baremetal::MicroFs::MicroFsInit(testCfg, 0, alloc);
}

//! Tear down for the tests cleans up the test file used
//!
void tearDown() {
    cleanup(0);
}

class MicroFsTester : public Tester {
    //! Check if the test file exists.
    //! \return true if it exists, false otherwise.
    //!
    bool exists(const std::string& filename) const override {
        bool exits = check_permissions(filename.c_str(), F_OK);
        return exits;
    }

    //! Get a filename, randomly if random is true, otherwise use a basic filename.
    //! \param random: true if filename should be random, false if predictable
    //! \return: filename to use for testing
    //!
    std::shared_ptr<const std::string> get_filename(bool random) const override {
        U32 pick = STest::Pick::lowerUpper(0, MAX_FILES_PER_BIN - 1);
        if (random && pick < FILES.size()) {
            return FILES[pick];
        }
        std::shared_ptr<const std::string> filename = get_test_filename(random);
        FILES.push_back(filename);
        return filename;
    }

    //! Posix tester is fully functional
    //! \return true
    //!
    bool functional() const override { return true; }
};

std::unique_ptr<Os::Test::File::Tester> get_tester_implementation() {
    return std::unique_ptr<Os::Test::File::Tester>(new Os::Test::File::MicroFsTester());
}

}  // namespace File
}  // namespace Test
}  // namespace Os

int main(int argc, char** argv) {
    STest::Random::seed();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
