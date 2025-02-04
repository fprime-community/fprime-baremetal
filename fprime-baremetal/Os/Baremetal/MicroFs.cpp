#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <Os/MicroFs/MicroFs.hpp>

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

// private pointer to allocated memory for microfs
STATIC void* s_microFsMem = 0;
// private copy of configuration struct passed by 
// user
MicroFsConfig s_microFsConfig;
// offset from zero for fds to allow zero checks
STATIC const FwSizeType MICROFS_FD_OFFSET = 1;

// private data structure for managing file state
struct MicroFsFileState {
    FwIndexType loc;       //!< location in file where last operation left off
    FwNativeIntType currSize;  //!< current size of the file after writes were done. -1 = not created yet.
    FwSizeType dataSize;  //!< alloted size of the file
    BYTE* data;                //!< location of file data
};

//!< set the number of bins in config
void MicroFsSetCfgBins(MicroFsConfig& cfg, const FwSizeType numBins) {
    FW_ASSERT(numBins <= MAX_MICROFS_BINS,numBins);
    cfg.numBins = numBins;
}

//!< add a bin to the config
void MicroFsAddBin(MicroFsConfig& cfg, const FwIndexType binIndex, const FwSizeType fileSize, const FwSizeType numFiles) {
    FW_ASSERT(binIndex <= MAX_MICROFS_BINS,binIndex);
    cfg.bins[binIndex].fileSize = fileSize;
    cfg.bins[binIndex].numFiles = numFiles;
}


void MicroFsInit(const MicroFsConfig& cfg, const FwNativeUIntType id, Fw::MemAllocator& allocator) {
    // check things...
    FW_ASSERT(cfg.numBins <= MAX_MICROFS_BINS, cfg.numBins, MAX_MICROFS_BINS);
    FW_ASSERT((not MICROFS_SKIP_NULL_CHECK) and 0 == s_microFsMem);

    // copy config to private copy 
    s_microFsConfig = cfg;

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
    s_microFsMem = allocator.allocate(id, reqMem, dontcare);

    // make sure got the amount requested.
    // improvement could be best effort based on received memory
    FW_ASSERT(reqMem >= memSize, reqMem, memSize);
    // make sure we got a non-null pointer
    FW_ASSERT(s_microFsMem);

    // lay out the memory with the state and the buffers after the config section
    MicroFsFileState* statePtr = reinterpret_cast<MicroFsFileState*>(s_microFsMem);

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

void MicroFsCleanup(const FwNativeUIntType id,
                 Fw::MemAllocator& allocator) {

    allocator.deallocate(id,s_microFsMem);
    s_microFsMem = 0;
}

// helper to find file state entry from file name. Will return index if found, -1 if not
STATIC FwIndexType 
getFileStateIndex(const char* fileName) {
    // the directory/filename rule is very strict - it has to be /MICROFS_BIN_STRING<n>/MICROFS_FILE_STRING<m>,
    // where n = number of file bins, and m = number of files in a particular bin
    // any other name will return an error

    // Scan the string for the bin and file numbers.  
    // We want a failure to find the file if there is any extension
    // after the file number. 
    const char* filePathSpec = "/" 
        MICROFS_BIN_STRING 
        "%d/" 
        MICROFS_FILE_STRING 
        "%d.%1s";

    FwSizeType binIndex;
    FwSizeType fileIndex;
    // crcExtension should be 2 bytes because scanf appends a null character at the end.
    char crcExtension[2];
    FwNativeIntType stat = sscanf(fileName,filePathSpec,&binIndex,&fileIndex,&crcExtension[0]);
    if (stat != 2) {
        return -1;
    }

    // check to see that indexes don't exceed config
    if (binIndex >= s_microFsConfig.numBins) {
        return -1;
    }

    if (fileIndex >= s_microFsConfig.bins[binIndex].numFiles) {
        return -1;
    }    

    FwIndexType stateIndex = 0;
    // compute file state index

    // add each chunk of file numbers from full bins
    for (FwSizeType currBin = 0; currBin < binIndex; currBin++) {
        stateIndex += s_microFsConfig.bins[currBin].numFiles;
    }

    // get residual file number from last bin
    stateIndex += fileIndex;

    return stateIndex;
}

// helper to get state pointer from index
STATIC MicroFsFileState* getFileStateFromIndex(FwIndexType index) {
    // should be >=0 by the time this is called
    FW_ASSERT(index >= 0, index);
    FW_ASSERT(s_microFsMem);
    // Get base of state structures
    MicroFsFileState* ptr = reinterpret_cast<MicroFsFileState*>(s_microFsMem);
    return &ptr[index];
}

File::File() : m_fd(-1), m_mode(OPEN_NO_MODE), m_lastError(0) {}

File::~File() {
    if (this->m_mode != OPEN_NO_MODE) {
        this->close();
    }
}

File::Status File::open(const char* fileName, File::Mode mode) {
    return this->open(fileName, mode, true);
}
File::Status File::open(const char* fileName, File::Mode mode, bool include_excl) {
    Status stat = OP_OK;

    // common checks
    // retrieve index to file entry
    FwIndexType entry = getFileStateIndex(fileName);
    // not found
    if (-1 == entry) {
        return File::Status::DOESNT_EXIST;
    }

    MicroFsFileState* state = getFileStateFromIndex(entry);
    FW_ASSERT(state);

    // make sure it isn't already open. If so, return an error
    if (state->loc != -1) {
        return File::Status::NO_PERMISSION;
    }

    switch (mode) {
        case OPEN_READ:
            // if not written to yet, doesn't exist for read
            if (-1 == state->currSize) {
                return File::Status::DOESNT_EXIST;
            }
            state->loc = 0;
            break;
        case OPEN_WRITE:
        case OPEN_SYNC_WRITE:         // fall through; same for microfs
        case OPEN_SYNC_DIRECT_WRITE:  // fall through; same for microfs
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (-1 == state->currSize) {
                state->currSize = 0;
            }
            state->loc = 0;
            break;
        case OPEN_CREATE:
            // truncate file length to zero
            state->currSize = 0;
            state->loc = 0;
            break;
        case OPEN_APPEND:
            // initialize write location to length of file for append
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (-1 == state->currSize) {
                state->currSize = 0;
            }

            state->loc = state->currSize;
            break;
        default:
            FW_ASSERT(0, mode);
            break;
    }

    // store mode
    this->m_mode = mode;
    
    // set file descriptor to index into state structure
    this->m_fd = entry + MICROFS_FD_OFFSET;

    return stat;
}

bool File::isOpen() {
    return (this->m_fd > 0);
}

File::Status File::prealloc(NATIVE_INT_TYPE offset, NATIVE_INT_TYPE len) {
    // do nothing; in RAM
    return OP_OK;
}

File::Status File::seek(NATIVE_INT_TYPE offset, bool absolute) {
    // make sure it has been opened

    if (OPEN_NO_MODE == this->m_mode) {
        return NOT_OPENED;
    }

    // get file state entry
    FW_ASSERT(this->m_fd != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_fd - MICROFS_FD_OFFSET);
    FW_ASSERT(state);

    FwNativeIntType oldSize = state->currSize; 

    // compute new operation location
    if (absolute) {
        // make sure not too far
        if ((offset >= static_cast<NATIVE_INT_TYPE>(state->dataSize)) or (offset < 0)) {
            return BAD_SIZE;
        }
        state->loc = offset;
    } else {
        // make sure not too far
        if (state->loc + offset >= static_cast<FwIndexType>(state->dataSize)) {
            return BAD_SIZE;
        }
        state->loc = state->loc + offset;
    }

    // move current size as well if needed
    if (state->loc > state->currSize) {
        state->currSize = state->loc;
    }

    // fill with zeros if seek went past old size
    if (state->currSize > oldSize) {
        memset(&state->data[oldSize],0,state->currSize-oldSize);
    }

    return OP_OK;
}

File::Status File::read(void* buffer, NATIVE_INT_TYPE& size, bool waitForFull) {
    FW_ASSERT(buffer);

    // make sure it has been opened
    if (OPEN_NO_MODE == this->m_mode) {
        size = 0;
        return NOT_OPENED;
    }

    // get file state entry
    FW_ASSERT(this->m_fd != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_fd - MICROFS_FD_OFFSET);
    FW_ASSERT(state);

    // find size to copy

    // check to see if already at the end of the file. If so, return 0 for size
    if (state->loc == state->currSize - 1) {
        size = 0;
        return OP_OK;
    }

    // copy requested bytes, unless it would be more than the file size.
    // If it would be more than the file size, copy the remainder and set
    // the size to the actual copied
    if (state->loc + size > state->currSize - 1) {
        size = state->currSize - state->loc;
    }

    // copy data from location to buffer
    (void)memcpy(buffer, state->data + state->loc, size);

    // move location pointer
    state->loc += size;

    return OP_OK;
}

File::Status File::write(const void* buffer, NATIVE_INT_TYPE& size, bool waitForDone) {
    FW_ASSERT(buffer);

    // make sure it has been opened in write mode
    switch (this->m_mode) {
        case OPEN_NO_MODE:
            return NOT_OPENED;
        case OPEN_WRITE:  // all fall through for write cases
        case OPEN_SYNC_WRITE:
        case OPEN_SYNC_DIRECT_WRITE:
        case OPEN_CREATE:
        case OPEN_APPEND:
            break;
        default:
            size = 0;
            return OTHER_ERROR;
    }

    // get file state entry
    FW_ASSERT(this->m_fd != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_fd - MICROFS_FD_OFFSET);
    FW_ASSERT(state);

    // write up to the end of the allocated buffer
    // if write size is greater, truncate the write
    // and set size to what was actually written
    if (state->loc + size > static_cast<FwIndexType>(state->dataSize)) {
        size = state->dataSize - state->loc;
    } 

    // copy data to file buffer
    (void)memcpy(&state->data[state->loc], buffer, size);

    // increment location
    state->loc += size;

    // Check if the currSize is to be increased.
    if (state->loc > state->currSize)
    {
        state->currSize = state->loc;
    }

    return OP_OK;
}

File::Status File::bulkWrite(const void* buffer, NATIVE_UINT_TYPE& totalSize, NATIVE_INT_TYPE chunkSize) {
    // make sure it has been opened
    if (OPEN_NO_MODE == this->m_mode) {
        return NOT_OPENED;
    }

    // RAM, so use write call
    NATIVE_INT_TYPE writeSize = totalSize;
    return write(buffer, writeSize, true);
}

File::Status File::flush() {
    // make sure it has been opened
    if (OPEN_NO_MODE == this->m_mode) {
        return NOT_OPENED;
    }

    // RAM, so no need to flush
    return OP_OK;
}

void File::close() {
    if (this->m_fd != -1) {
        // only do cleanup of file state
        // if file system memory is still around
        // catches case where file objects are still
        // lingering after cleanup
        if (s_microFsMem) {
            // get state to clear it
            MicroFsFileState* state = getFileStateFromIndex(this->m_fd - MICROFS_FD_OFFSET);
            FW_ASSERT(state);
            state->loc = -1;
        }
    }
    // reset fd
    this->m_fd = -1;
    this->m_mode = OPEN_NO_MODE;
    // clear
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

} // end Os namespace

// FileSystem functions

namespace Os {

namespace FileSystem {

Status createDirectory(const char* path) {

    // The directory can only be one of the
    // bin file names, and it is already "created" if
    // the bin exists. See if the directory matches
    // one of the bins, and if it does, return OK, otherwise
    // return NO_PERMISSION.

    const char* dirPathSpec = "/" 
    MICROFS_BIN_STRING 
    "%d"; 

    // if the directory number can be scanned out following the directory path spec,
    // the directory name has the correct format

    FwIndexType binIndex = 0;

    FwNativeUIntType stat = sscanf(path,dirPathSpec,&binIndex);
    if (stat != 1) {
        return NO_PERMISSION;
    }

    // If the path format is correct, check to see if it is in the
    // range of bins
    if (binIndex < static_cast<FwIndexType>(s_microFsConfig.numBins)) {
        return OP_OK;
    } else {
        return NO_PERMISSION;
    }
}

Status removeDirectory(const char* path) {
    // since we can't create or remove directories anyway,
    // just borrow the createDirectory() logic
    return createDirectory(path);
}  

Status readDirectory(const char* path, const U32 maxNum, Fw::String fileArray[], U32& numFiles) {

    if (not path) {
        return INVALID_PATH;
    }

    // two cases work: the root "/" and one of the bin directories
    numFiles = 0;

    // first, check for root
    if (Fw::StringUtils::string_length(path,sizeof(MICROFS_BIN_STRING)) == 1) {
        if (path[0] != '/') {
            return INVALID_PATH;
        }
        // Add directory names based on number of bins
        for (FwIndexType bin = 0; bin < static_cast<FwIndexType>(s_microFsConfig.numBins); bin++) {
            // make sure we haven't exceeded provided array size
            if (bin >= static_cast<FwIndexType>(maxNum)) {
                return OP_OK;
            }
            // add directory name of bin to file list
            static const char * dirFmt = "/"
                MICROFS_BIN_STRING
                "%d"
                ;
            fileArray[bin].format(dirFmt,bin);
            numFiles++;
        }

        return OP_OK;
    }

    // second, if not root, directory has to match one of the bin paths
    const char* dirPathSpec = "/" 
    MICROFS_BIN_STRING 
    "%d"; 

    // if the directory number can be scanned out following the directory path spec,
    // the directory name has the correct format

    FwIndexType binIndex = 0;

    // verify in the range of bins
    FwNativeUIntType stat = sscanf(path,dirPathSpec,&binIndex);
    if ((stat != 1) or (binIndex >= static_cast<FwIndexType>(s_microFsConfig.numBins))) {
        return INVALID_PATH;
    }

    // get set of files in the bin directory
    Fw::String fileStr;
    for (FwIndexType entry = 0; entry < static_cast<FwIndexType>(s_microFsConfig.bins[binIndex].numFiles); entry++) {

        // make sure we haven't exceeded provided array size
        if (entry >= static_cast<FwIndexType>(maxNum)) {
            return OP_OK;
        }

        const char* filePathSpec = "/"
            MICROFS_BIN_STRING
            "%d/"
            MICROFS_FILE_STRING
            "%d";

        fileStr.format(filePathSpec,binIndex,entry);
        // get file state
        FwIndexType fileIndex = Os::getFileStateIndex(fileStr.toChar());
        // should always find it, since it is from a known valid bin
        FW_ASSERT(fileIndex != -1);
        MicroFsFileState* fState = getFileStateFromIndex(fileIndex);
        FW_ASSERT(fState);

        // check to see if it has been written
        if (fState->currSize != -1) {
            fileArray[numFiles++] = fileStr;
        }

    }

    return OP_OK;

}

Status removeFile(const char* path) {

    if (not path) {
        return INVALID_PATH;
    }

    // get file state
    FwIndexType index = getFileStateIndex(path);
    if (index == -1) {
        return INVALID_PATH;
    }

    MicroFsFileState* fState = getFileStateFromIndex(index);
    FW_ASSERT(fState);

    if (fState->loc != -1)
    {
        return BUSY;
    }

    // delete the file by setting current size to be -1
    fState->currSize = -1;
    fState->loc = -1;

    return OP_OK;
}

Status moveFile(const char* originPath, const char* destPath) {

    Status copyStat = copyFile(originPath,destPath);
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

    FwSizeType copySize = (origState->currSize < static_cast<FwNativeIntType>(destState->dataSize)) ?
        origState->currSize:destState->dataSize;

    (void)memcpy(destState->data,origState->data,copySize);
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

    FwSizeType copySize = (origState->currSize < static_cast<FwNativeIntType>(destState->dataSize-destState->currSize)) ?
        origState->currSize:(destState->dataSize-destState->currSize);

    (void)memcpy(&destState->data[destState->currSize],origState->data,copySize);
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
        for (FwIndexType currFile = 0; currFile < static_cast<FwIndexType>(s_microFsConfig.bins[currBin].numFiles); currFile++) {
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


} // end FileSystem namespace

} // end Os namespace
