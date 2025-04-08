#ifndef _MICROFS_HPP_
#define _MICROFS_HPP_

#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/MemAllocator.hpp>
#include <MicroFsCfg.hpp>

// MicroFs - F Prime Micro Filesystem
//
// MicroFs is designed to be a very basic volatile RAM file system for embedded targets
// that don't have storage but the user would like to use file-based features in F Prime
// like `Svc/CmdSequencer`, `Svc/FileUplink`, and `Svc/FileDownlink`. The user can
// generate files from custom components as well and use `Svc/FileDownlink` to send them
// to a ground system. The caveat is that all file system contents are lost when the
// software is restarted or rebooted.
//
// The file system follows this scheme:
//
// The file system has the concept of `bins`. Bins are collections of files that are the
// same maximum size. The `bin` name (see `MicroFsCfg` macro `MICROFS_BIN_STRING`) forms a
// directory with the name `/<MICROFS_BIN_STRING><bin #>`, and all files in that directory
// have size specified in the `MicroFsConfig` data structure below, and have the name
// `/<MICROFS_BIN_STRING><bin #>/<MICROFS_FILE_STRING><file #>`
//
// The user can set `MAX_MICROFS_BINS` to specify the maximum number of defined bins
// to match the needs of the project.
//
// The configuration structure is set as follows:
//
// numBins - the number of bins defined in the file system. Should be <= `MAX_MICROFS_BINS`
//
// For each bin in `numBins`, the file size and number of files defined for that size are:
// `bins[bin #].fileSize` = size of files in that bin
// `bins[bin #].numFiles` = number of files in that bin
//
// As an example, if a project wanted to have two bins, one with three 1K files, and the other
// with ten 10K files, the structure would look like the following:
//
// MicroFsConfig myConfig;
// myConfig.numBins = 2;
// myConfig.bins[0].fileSize = 3*1024;
// myConfig.bins[0].numFiles = 3;
//
// myConfig.bins[1].fileSize = 10*1024;
// myConfig.bins[1].numFiles = 10;
//
// FwNativeUIntType id = 0; // only needed if allocator needs an id
//
// Fw::MallocAllocator mallocator; // memory from the heap for this example
//
// Os::MicroFsInit(myConfig,id,mallocator);
//
// The configuration structure can be temporary as it is copied into
// private storage by the `Init()` call.
//
// At this point, the user can call the `Os::File` and
// `Os::FileSystem` APIs to do file operations.
//
// In the above example, some file names would be:
//
// First bin, second file: `/bin0/file2`
// Second bin, ninth file: `/bin1/file9`
//
// (Based on default name macros in `MicroFsCfg.hpp`)
//
// The files do not appear in the file system until
// an `Os::Open` with `OPEN_WRITE` or `OPEN_CREATE` flag.
// Attempts to open a file to read before a write has occurred
// will return a `File::Status::DOESNT_EXIST` error.
//
// Files can be written up to the max size of the file
// specified for that bin. Attempts to write beyond the
// end will result in a write size returned that is the
// remaining room in the file but not the requested size.
// Attempts to seek past the end will return a `File::Status::BAD_SIZE`
// error.
//
// If the program is meant to terminate at the end,
// the user should call `MicrFsCleanup()` with the allocator
// and ID used to acquire the memory at initialization.
//
// Some idiosyncracies for the sake of simplicity:
// 1) The createDirectory() and removeDirectory() calls
//    won't actually create or remove directories, but
//    will return OP_OK if the directories match the configuration
//    This is to help commands pass if the create/remove are there and correct
//
// 2) A seek past the end of a file will fill the new area with zero bytes
//
// 3) Copying from a larger file slot to a smaller file slot will truncate the file
//    if the source is larger

namespace Os {
namespace Baremetal {
class MicroFs {
  public:
    struct MicroFsBin {
        FwSizeType fileSize;  //<! The size of the files in the bin
        FwSizeType numFiles;  //<! The number of files in the bin
    };

    struct MicroFsConfig {
        FwSizeType numBins;                 //!< The number of bins configured. Must be <= than MAX_MICROFS_BINS
        MicroFsBin bins[MAX_MICROFS_BINS];  //!< The bins containing file sizes and numbers of files
    };

  public:
    // data structure for managing file state
    struct MicroFsFileState {
        FwIndexType loc;           //!< location in file where last operation left off
        FwNativeIntType currSize;  //!< current size of the file after writes were done. -1 = not created yet.
        FwSizeType dataSize;       //!< alloted size of the file
        BYTE* data;                //!< location of file data
    };

  public:
    //! \brief default constructor
    MicroFs() = default;

    //! \brief copy construtor is forbidden due to singleton pattern
    MicroFs(const MicroFs& other) = delete;

    //! \brief copy construtor is forbidden due to singleton pattern
    MicroFs(const MicroFs* other) = delete;

    //! \brief assignment operator is forbidden due to singleton pattern
    MicroFs& operator=(const MicroFs& other) = delete;

  public:
    //!< set the number of bins in config
    static void MicroFsSetCfgBins(MicroFsConfig& cfg, const FwSizeType numBins);

    //!< add a bin to the config
    static void MicroFsAddBin(MicroFsConfig& cfg,
                              const FwIndexType binIndex,
                              const FwSizeType fileSize,
                              const FwSizeType numFiles);

    //!< initialize MicroFs memory by passing the configuration, a memory id (if needed), and a memory allocator

    static void MicroFsInit(
        const MicroFsConfig& cfg,      //!< the configuration of the memory space
        const FwNativeUIntType id,     //!< The memory id. Value doesn't matter if allocator doesn't need it
        Fw::MemAllocator& allocator);  //!< Memory allocator to use for memory

    static void MicroFsCleanup(
        const FwNativeUIntType id,     //!< The memory id. Value doesn't matter if allocator doesn't need it
        Fw::MemAllocator& allocator);  //!< Memory allocator to to deallocate. Should match MicroFsInit allocator

    // helper to get state pointer from index
    static MicroFsFileState* getFileStateFromIndex(FwIndexType index);

    // helper to find file state entry from file name. Will return index if found, -1 if not
    static FwIndexType getFileStateIndex(const char* fileName);

    //! \brief get a reference to singleton
    //! \return reference to singleton
    static MicroFs& getSingleton();

  public:
    // private pointer to allocated memory for microfs
    void* s_microFsMem = nullptr;
    // private copy of configuration struct passed by
    // user
    MicroFsConfig s_microFsConfig;
    // offset from zero for fds to allow zero checks
    static constexpr FwSizeType MICROFS_FD_OFFSET = 1;
};

}  // namespace Baremetal
}  // namespace Os

#endif
