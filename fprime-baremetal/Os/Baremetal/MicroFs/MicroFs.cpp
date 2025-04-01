#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <Utils/Hash/libcrc/lib_crc.h>  // borrow CRC

#ifdef __cplusplus
}
#endif  // __cplusplus

#include <cstdio>
#include <cstring>
#include <limits>

namespace Os {
namespace Baremetal {

//!< set the number of bins in config
void MicroFs::MicroFsSetCfgBins(MicroFsConfig& cfg, const FwSizeType numBins) {
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

void MicroFs::MicroFsInit(const MicroFsConfig& cfg, const FwNativeUIntType id, Fw::MemAllocator& allocator) {
    // Force trigger on the fly singleton setup
    auto microFs = MicroFs::getSingleton();

    // check things...
    FW_ASSERT(cfg.numBins <= MAX_MICROFS_BINS, cfg.numBins, MAX_MICROFS_BINS);
    FW_ASSERT((not MICROFS_SKIP_NULL_CHECK) and (microFs.s_microFsMem == nullptr));

    // copy config to private copy
    microFs.s_microFsConfig = cfg;

    // compute the amount of memory needed to hold the file system state
    // and data

    FwSizeType memSize = 0;
    FwSizeType totalNumFiles = 0;
    // iterate through the bins
    for (FwSizeType bin = 0; bin < cfg.numBins; bin++) {
        // memory per file needed is struct for file state + file buffer size
        memSize += cfg.bins[bin].numFiles * (sizeof(MicroFsFileState) + cfg.bins[bin].fileSize);
        totalNumFiles += cfg.bins[bin].numFiles;
    }

    // request the memory
    FwNativeUIntType reqMem = memSize;

    bool dontcare;
    microFs.s_microFsMem = allocator.allocate(id, reqMem, dontcare);

    // make sure got the amount requested.
    // improvement could be best effort based on received memory
    FW_ASSERT(reqMem >= memSize, reqMem, memSize);
    // make sure we got a non-null pointer
    FW_ASSERT(microFs.s_microFsMem != nullptr);

    // lay out the memory with the state and the buffers after the config section
    MicroFsFileState* statePtr = reinterpret_cast<MicroFsFileState*>(microFs.s_microFsMem);

    // point to memory after state structs for beginning of file data
    BYTE* currFileBuff = reinterpret_cast<BYTE*>(&statePtr[totalNumFiles]);
    // fill in the file state structs
    for (FwSizeType bin = 0; bin < cfg.numBins; bin++) {
        for (FwSizeType file = 0; file < cfg.bins[bin].numFiles; file++) {
            // clear state structure memory
            memset(statePtr, 0, sizeof(MicroFsFileState));
            // initialize state
            statePtr->loc = -1;                           // no operation in progress
            statePtr->currSize = -1;                      // nothing written yet
            statePtr->data = currFileBuff;                // point to data for the file
            statePtr->dataSize = cfg.bins[bin].fileSize;  // store allocated size for file data
#if MICROFS_INIT_FILE_DATA
            ::memset(currFileBuff, 0, cfg.bins[bin].fileSize);
#endif
            // advance file data pointer
            currFileBuff += cfg.bins[bin].fileSize;
            // advance file state pointer
            statePtr += 1;
        }
    }
}

void MicroFs::MicroFsCleanup(const FwNativeUIntType id, Fw::MemAllocator& allocator) {
    auto microFs = MicroFs::getSingleton();
    allocator.deallocate(id, microFs.s_microFsMem);
    microFs.s_microFsMem = 0;
}

// helper to find file state entry from file name. Will return index if found, -1 if not
FwIndexType MicroFs::getFileStateIndex(const char* fileName) {
    // the directory/filename rule is very strict - it has to be /MICROFS_BIN_STRING<n>/MICROFS_FILE_STRING<m>,
    // where n = number of file bins, and m = number of files in a particular bin
    // any other name will return an error

    // Scan the string for the bin and file numbers.
    // We want a failure to find the file if there is any extension
    // after the file number.
    const char* filePathSpec = "/" MICROFS_BIN_STRING "%d/" MICROFS_FILE_STRING "%d.%1s";

    FwSizeType binIndex;
    FwSizeType fileIndex;
    // crcExtension should be 2 bytes because scanf appends a null character at the end.
    char crcExtension[2];
    FwNativeIntType stat = sscanf(fileName, filePathSpec, &binIndex, &fileIndex, &crcExtension[0]);
    if (stat != 2) {
        return -1;
    }

    auto microFs = MicroFs::getSingleton();
    // check to see that indexes don't exceed config
    if (binIndex >= microFs.s_microFsConfig.numBins) {
        return -1;
    }

    if (fileIndex >= microFs.s_microFsConfig.bins[binIndex].numFiles) {
        return -1;
    }

    FwIndexType stateIndex = 0;
    // compute file state index

    // add each chunk of file numbers from full bins
    for (FwSizeType currBin = 0; currBin < binIndex; currBin++) {
        stateIndex += microFs.s_microFsConfig.bins[currBin].numFiles;
    }

    // get residual file number from last bin
    stateIndex += fileIndex;

    return stateIndex;
}

// helper to get state pointer from index
MicroFs::MicroFsFileState* MicroFs::getFileStateFromIndex(FwIndexType index) {
    auto microFs = MicroFs::getSingleton();
    // should be >=0 by the time this is called
    FW_ASSERT(index >= 0, index);
    FW_ASSERT(microFs.s_microFsMem);
    // Get base of state structures
    MicroFsFileState* ptr = reinterpret_cast<MicroFsFileState*>(microFs.s_microFsMem);
    return &ptr[index];
}

#if 0
File::Status File::bulkWrite(const void* buffer, NATIVE_UINT_TYPE& totalSize, NATIVE_INT_TYPE chunkSize) {
    // make sure it has been opened
    if (OPEN_NO_MODE == this->m_mode) {
        return NOT_OPENED;
    }

    // RAM, so use write call
    NATIVE_INT_TYPE writeSize = totalSize;
    return write(buffer, writeSize, true);
}

NATIVE_INT_TYPE File::getLastError() {
    return this->m_lastError;
}

const char* File::getLastErrorString() {
    return strerror(this->m_lastError);
}

File::Status File::calculateCRC32(U32& crc) {
    // make sure it has been opened
    if (OPEN_NO_MODE == this->m_mode) {
        crc = 0;
        return NOT_OPENED;
    }

    const U32 maxChunkSize = 32;
    const U32 initialSeed = 0xFFFFFFFF;

    // Seek to beginning of file
    Status status = seek(0, true);
    if (status != OP_OK) {
        crc = 0;
        return status;
    }

    U8 file_buffer[maxChunkSize];

    bool endOfFile = false;

    U32 seed = initialSeed;
    const U32 maxIters = std::numeric_limits<U32>::max();  // loop limit
    U32 numIters = 0;

    while (!endOfFile && numIters < maxIters) {
        ++numIters;
        NATIVE_INT_TYPE chunkSize = maxChunkSize;

        status = read(file_buffer, chunkSize, false);
        if (status == OP_OK) {
            // chunkSize modified by file.read

            if (chunkSize == 0) {
                endOfFile = true;
                continue;
            }

            int chunkIdx = 0;

            while (chunkIdx < chunkSize) {
                seed = update_crc_32(seed, file_buffer[chunkIdx]);
                chunkIdx++;
            }

        } else {
            crc = 0;
            return status;
        }
    }

    if (!endOfFile) {
        crc = 0;
        return OTHER_ERROR;
    } else {
        crc = seed;
        return OP_OK;
    }
}

}  // namespace Baremetal

// FileSystem functions

namespace Os {

namespace FileSystem {



Status removeDirectory(const char* path) {
    // since we can't create or remove directories anyway,
    // just borrow the createDirectory() logic
    return createDirectory(path);
}

Status moveFile(const char* originPath, const char* destPath) {
    Status copyStat = copyFile(originPath, destPath);
    if (copyStat != OP_OK) {
        return copyStat;
    }

    return removeFile(originPath);
}

Status copyFile(const char* originPath, const char* destPath) {
    if ((not originPath) or (not destPath)) {
        return INVALID_PATH;
    }

    // get file state
    FwIndexType origIndex = getFileStateIndex(originPath);
    if (origIndex == -1) {
        return INVALID_PATH;
    }

    MicroFsFileState* origState = getFileStateFromIndex(origIndex);
    FW_ASSERT(origState);

    // get file state
    FwIndexType destIndex = getFileStateIndex(destPath);
    if (-1 == destIndex) {
        return INVALID_PATH;
    }

    MicroFsFileState* destState = getFileStateFromIndex(destIndex);
    FW_ASSERT(destState);

    // make sure source exists
    if (origState->currSize == -1) {
        return INVALID_PATH;
    }

    // make sure neither is open so we don't corrupt operations
    // in progress
    if ((destState->loc != -1) or (origState->loc != -1)) {
        return BUSY;
    }

    // check sizes to see if going from a bigger slot to a
    // smaller slot

    FwSizeType copySize = (origState->currSize < static_cast<FwNativeIntType>(destState->dataSize))
                              ? origState->currSize
                              : destState->dataSize;

    (void)memcpy(destState->data, origState->data, copySize);
    destState->currSize = copySize;

    return OP_OK;
}

Status appendFile(const char* originPath, const char* destPath, bool createMissingDest) {
    if ((not originPath) or (not destPath)) {
        return INVALID_PATH;
    }

    // get file state
    FwIndexType origIndex = getFileStateIndex(originPath);
    if (origIndex == -1) {
        return INVALID_PATH;
    }

    MicroFsFileState* origState = getFileStateFromIndex(origIndex);
    FW_ASSERT(origState);

    // get file state
    FwIndexType destIndex = getFileStateIndex(destPath);
    if (-1 == destIndex) {
        return INVALID_PATH;
    }

    MicroFsFileState* destState = getFileStateFromIndex(destIndex);
    FW_ASSERT(destState);

    // make sure source exists
    if (origState->currSize == -1) {
        return INVALID_PATH;
    }

    // make sure destination exists if new file is not requested
    if ((not createMissingDest) and (-1 == destState->currSize)) {
        return INVALID_PATH;
    }

    // make sure neither is open so we don't corrupt operations
    // in progress
    if ((destState->loc != -1) or (origState->loc != -1)) {
        return BUSY;
    }

    // initialize destination file if it hasn't been written yet
    if (-1 == destState->currSize) {
        destState->currSize = 0;
    }

    // check sizes to see if going from a bigger slot to a
    // smaller slot

    FwSizeType copySize =
        (origState->currSize < static_cast<FwNativeIntType>(destState->dataSize - destState->currSize))
            ? origState->currSize
            : (destState->dataSize - destState->currSize);

    (void)memcpy(&destState->data[destState->currSize], origState->data, copySize);
    destState->currSize += copySize;

    return OP_OK;
}

Status getFileSize(const char* path, FwSizeType& size) {
    // initialize size
    size = 0;
    // get file state
    FwIndexType index = getFileStateIndex(path);
    if (index == -1) {
        return INVALID_PATH;
    }

    MicroFsFileState* fState = getFileStateFromIndex(index);
    FW_ASSERT(fState);

    size = fState->currSize;

    return OP_OK;
}

// We can get a "free" space number by adding up all the space left in the file bins

Status getFreeSpace(const char* path, FwSizeType& totalBytes, FwSizeType& freeBytes) {
    totalBytes = 0;
    freeBytes = 0;

    // Get first file state struct
    MicroFsFileState* statePtr = reinterpret_cast<MicroFsFileState*>(s_microFsMem);
    FW_ASSERT(statePtr);

    // iterate through bins
    for (FwIndexType currBin = 0; currBin < static_cast<FwIndexType>(s_microFsConfig.numBins); currBin++) {
        // iterate through files in each bin
        for (FwIndexType currFile = 0; currFile < static_cast<FwIndexType>(s_microFsConfig.bins[currBin].numFiles);
             currFile++) {
            totalBytes += statePtr->dataSize;
            // only add unused file slots to free space
            if (-1 == statePtr->currSize) {
                freeBytes += statePtr->dataSize;
            }
            statePtr += 1;
        }
    }

    return OP_OK;
}

}  // namespace FileSystem
#endif
}  // namespace Baremetal
}  // namespace Os
