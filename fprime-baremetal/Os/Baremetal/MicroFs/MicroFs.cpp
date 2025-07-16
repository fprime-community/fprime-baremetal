#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>

#include <cstdio>
#include <cstring>

namespace Os {
namespace Baremetal {

//!< set the number of bins in config
void MicroFs::MicroFsSetCfgBins(MicroFsConfig& cfg, const FwIndexType numBins) {
    FW_ASSERT(numBins <= MAX_MICROFS_BINS, numBins);
    cfg.numBins = numBins;
}

//!< add a bin to the config
void MicroFs::MicroFsAddBin(MicroFsConfig& cfg,
                            const FwIndexType binIndex,
                            const FwSizeType fileSize,
                            const FwSizeType numFiles) {
    FW_ASSERT(binIndex <= MAX_MICROFS_BINS, binIndex);
    cfg.bins[binIndex].fileSize = fileSize;
    cfg.bins[binIndex].numFiles = numFiles;
}

MicroFs& MicroFs::getSingleton() {
    static MicroFs s_singleton;
    return s_singleton;
}

void MicroFs::MicroFsInit(const MicroFsConfig& cfg, const FwEnumStoreType id, Fw::MemAllocator& allocator) {
    // Force trigger on the fly singleton setup
    MicroFs& microfs = MicroFs::getSingleton();

    // check things...
    FW_ASSERT(cfg.numBins <= MAX_MICROFS_BINS, cfg.numBins, MAX_MICROFS_BINS);
    FW_ASSERT((not MICROFS_SKIP_NULL_CHECK) and (microfs.s_microFsMem == nullptr));

    // copy config to private copy
    microfs.s_microFsConfig = cfg;

    // compute the amount of memory needed to hold the file system state
    // and data

    FwSizeType memSize = 0;
    FwSizeType totalNumFiles = 0;
    // iterate through the bins
    for (FwIndexType bin = 0; bin < cfg.numBins; bin++) {
        // memory per file needed is struct for file state + file buffer size
        memSize += cfg.bins[bin].numFiles * (sizeof(MicroFsFileState) + cfg.bins[bin].fileSize);
        totalNumFiles += cfg.bins[bin].numFiles;
    }

    // request the memory
    FwSizeType reqMem = memSize;

    bool dontcare;
    microfs.s_microFsMem = allocator.allocate(id, reqMem, dontcare);

    // make sure memory is aligned
    FW_ASSERT((reinterpret_cast<PlatformPointerCastType>(microfs.s_microFsMem) % alignof(MicroFsFileState)) == 0);

    // make sure got the amount requested.
    // improvement could be best effort based on received memory
    FW_ASSERT(reqMem >= memSize, reqMem, memSize);
    // make sure we got a non-null pointer
    FW_ASSERT(microfs.s_microFsMem != nullptr);

    // lay out the memory with the state and the buffers after the config section
    MicroFsFileState* statePtr = static_cast<MicroFsFileState*>(microfs.s_microFsMem);

    // point to memory after state structs for beginning of file data
    BYTE* currFileBuff = reinterpret_cast<BYTE*>(&statePtr[totalNumFiles]);
    // fill in the file state structs
    for (FwIndexType bin = 0; bin < cfg.numBins; bin++) {
        for (FwSizeType file = 0; file < cfg.bins[bin].numFiles; file++) {
            // clear state structure memory
            (void)memset(statePtr, 0, sizeof(MicroFsFileState));
            // initialize state
            for (FwIndexType fdIndex = 0; fdIndex < MAX_MICROFS_FD; fdIndex++) {
                statePtr->fd[fdIndex].loc = 0;                   // no operation in progress
                statePtr->fd[fdIndex].status = Status::INVALID;  // no operation in progress
            }
            statePtr->created = false;                    // has not been created
            statePtr->currSize = 0;                       // nothing written yet
            statePtr->data = currFileBuff;                // point to data for the file
            statePtr->dataSize = cfg.bins[bin].fileSize;  // store allocated size for file data
#if MICROFS_INIT_FILE_DATA
            (void)::memset(currFileBuff, 0, cfg.bins[bin].fileSize);
#endif
            // advance file data pointer
            currFileBuff += cfg.bins[bin].fileSize;
            // advance file state pointer
            statePtr += 1;
        }
    }
}

void MicroFs::MicroFsCleanup(const FwEnumStoreType id, Fw::MemAllocator& allocator) {
    allocator.deallocate(id, MicroFs::getSingleton().s_microFsMem);
    MicroFs::getSingleton().s_microFsMem = nullptr;
}

// helper to find file state entry from file name. Will return VALID if found, INVALID if not
MicroFs::Status MicroFs::getFileStateIndex(const char* fileName, FwIndexType& stateIndex) {
    // the directory/filename rule is very strict - it has to be /MICROFS_BIN_STRING<n>/MICROFS_FILE_STRING<m>,
    // where n = number of file bins, and m = number of files in a particular bin
    // any other name will return an error

    // Scan the string for the bin and file numbers.
    // We want a failure to find the file if there is any extension
    // after the file number.
    const char* filePathSpec =
        "/" MICROFS_BIN_STRING "%" MICROFS_INDEX_SCN_FORMAT "/" MICROFS_FILE_STRING "%" MICROFS_INDEX_SCN_FORMAT ".%1s";

    FwIndexType binIndex = 0;
    FwIndexType fileIndex = 0;
    // crcExtension should be 2 bytes because scanf appends a null character at the end.
    char crcExtension[2];
    int stat = sscanf(fileName, filePathSpec, &binIndex, &fileIndex, &crcExtension[0]);
    if (stat != 2) {
        return MicroFs::Status::INVALID;
    }

    MicroFs& microfs = MicroFs::getSingleton();

    // check to see that indexes don't exceed config
    if (binIndex >= microfs.s_microFsConfig.numBins) {
        return MicroFs::Status::INVALID;
    }

    if (fileIndex < 0 || FwSizeType(fileIndex) >= microfs.s_microFsConfig.bins[binIndex].numFiles) {
        return MicroFs::Status::INVALID;
    }

    stateIndex = 0;
    // compute file state index

    // add each chunk of file numbers from full bins
    for (FwIndexType currBin = 0; currBin < binIndex; currBin++) {
        stateIndex += microfs.s_microFsConfig.bins[currBin].numFiles;
    }

    // get residual file number from last bin
    stateIndex += fileIndex;

    return MicroFs::Status::VALID;
}

// helper to get state pointer from index
MicroFs::MicroFsFileState* MicroFs::getFileStateFromIndex(FwIndexType index) {
    // should be >=0 by the time this is called
    FW_ASSERT(index >= 0, index);
    FW_ASSERT(MicroFs::getSingleton().s_microFsMem);
    // Get base of state structures
    MicroFsFileState* ptr = static_cast<MicroFsFileState*>(MicroFs::getSingleton().s_microFsMem);
    return &ptr[index];
}

MicroFs::Status MicroFs::getFileStateNextFreeFd(const MicroFs::MicroFsFileState* state, FwIndexType& nextFreeFd) {
    if (state == nullptr) {
        return MicroFs::Status::INVALID;
    }

    auto status = MicroFs::Status::INVALID;

    for (FwIndexType i = 0; i < MAX_MICROFS_FD; i++) {
        if (state->fd[i].status == Status::INVALID) {
            nextFreeFd = i;
            status = MicroFs::Status::VALID;
            break;
        }
    }
    return status;
}

}  // namespace Baremetal
}  // namespace Os
