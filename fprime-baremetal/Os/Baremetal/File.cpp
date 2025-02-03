// ======================================================================
// \title Os/Baremetal/File.cpp
// \brief baremetal implementation for Os::File
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Os/File.hpp>
#include <cstdio>
#include <cstring>
#include <fprime-baremetal/Os/Baremetal/File.hpp>
#include <fprime-baremetal/Os/Baremetal/error.hpp>

namespace Os {
namespace Baremetal {
namespace File {
///////////////////////////////////////
// Helper static functions
///////////////////////////////////////

// private pointer to allocated memory for microfs
STATIC void* s_microFsMem = 0;
// private copy of configuration struct passed by
// user
MicroFsConfig s_microFsConfig;
// offset from zero for fds to allow zero checks
STATIC const FwSizeType MICROFS_FD_OFFSET = 1;

// Find file state entry from file name. Will return index if found, -1 if not
STATIC FwIndexType getFileStateIndex(const char* fileName) {
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

// Get state pointer from index
STATIC MicroFsFileState* getFileStateFromIndex(FwIndexType index) {
    // should be >=0 by the time this is called
    FW_ASSERT(index >= 0, index);
    FW_ASSERT(s_microFsMem != nullptr);
    // Get base of state structures
    MicroFsFileState* ptr = reinterpret_cast<MicroFsFileState*>(s_microFsMem);
    return &ptr[index];
}

BaremetalFile::~BaremetalFile() {
    if (this->m_handle.m_mode != OPEN_NO_MODE) {
        this->close();
    }
}

BaremetalFile::Status BaremetalFile::open(const char* path,
                                          BaremetalFile::Mode mode,
                                          BaremetalFile::OverwriteType overwrite) {
    FW_ASSERT(path != nullptr);
    Status stat = OP_OK;

    // common checks
    // retrieve index to file entry
    FwIndexType entry = getFileStateIndex(path);
    // not found
    if (-1 == entry) {
        return Os::File::Status::DOESNT_EXIST;
    }

    MicroFsFileState* state = getFileStateFromIndex(entry);
    FW_ASSERT(state);

    // make sure it isn't already open. If so, return an error
    if (state->loc != -1) {
        return Os::File::Status::NO_PERMISSION;
    }

    switch (mode) {
        case OPEN_READ:
            // if not written to yet, doesn't exist for read
            if (-1 == state->currSize) {
                return Os::File::Status::DOESNT_EXIST;
            }
            state->loc = 0;
            break;
        case OPEN_WRITE:
        case OPEN_SYNC_WRITE:  // fall through; same for microfs
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
    this->m_handle.m_mode = mode;

    // set file descriptor to index into state structure
    this->m_handle.m_file_descriptor = entry + MICROFS_FD_OFFSET;

    return stat;
}

bool BaremetalFile::isOpen() const {
    FW_ASSERT((0 <= this->m_handle.m_mode) && (this->m_handle.m_mode < Mode::MAX_OPEN_MODE), this->m_handle.m_mode);
    return (this->m_handle.m_mode != OPEN_NO_MODE);
}

BaremetalFile::Status BaremetalFile::preallocate(FwSignedSizeType offset, FwSignedSizeType length) {
    // do nothing; in RAM
    return Os::File::Status::OP_OK;
}

void BaremetalFile::close() {
    if (this->m_handle.m_file_descriptor != -1) {
        // only do cleanup of file state
        // if file system memory is still around
        // catches case where file objects are still
        // lingering after cleanup
        if (s_microFsMem) {
            // get state to clear it
            MicroFsFileState* state = getFileStateFromIndex(this->m_handle.m_file_descriptor - MICROFS_FD_OFFSET);
            FW_ASSERT(state != nullptr);
            state->loc = -1;
        }
    }
    // reset fd
    this->m_handle.m_file_descriptor = -1;
    this->m_handle.m_mode = OPEN_NO_MODE;
    // clear
}

BaremetalFile::Status BaremetalFile::size(FwSignedSizeType& size_result) {
    FwSignedSizeType current_position = 0;
    Status status = this->position(current_position);
    // TODO
    return status;
}

BaremetalFile::Status BaremetalFile::position(FwSignedSizeType& position_result) {
    Status status = OP_OK;
    // TODO
    return status;
}

BaremetalFile::Status BaremetalFile::seek(FwSignedSizeType offset, BaremetalFile::SeekType seekType) {
    // make sure it has been opened
    if (!this->isOpen()) {
        return NOT_OPENED;
    }

    // get file state entry
    FW_ASSERT(this->m_handle.m_file_descriptor != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_handle.m_file_descriptor - MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    FwNativeIntType oldSize = state->currSize;

    // compute new operation location
    switch (seekType) {
        case SeekType::ABSOLUTE:
            // make sure not too far
            if ((offset >= static_cast<NATIVE_INT_TYPE>(state->dataSize)) or (offset < 0)) {
                return BAD_SIZE;
            }
            state->loc = offset;
            break;
        case SeekType::RELATIVE:
            // make sure not too far
            if (state->loc + offset >= static_cast<FwIndexType>(state->dataSize)) {
                return BAD_SIZE;
            }
            state->loc = state->loc + offset;
            break;
        default:
            FW_ASSERT(0, seekType);
            break;
    }

    // move current size as well if needed
    if (state->loc > state->currSize) {
        state->currSize = state->loc;
    }

    // fill with zeros if seek went past old size
    if (state->currSize > oldSize) {
        (void)memset(&state->data[oldSize], 0, state->currSize - oldSize);
    }

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::flush() {
    // make sure it has been opened
    if (!this->isOpen()) {
        return NOT_OPENED;
    }

    // RAM, so no need to flush
    return OP_OK;
}

BaremetalFile::Status BaremetalFile::read(U8* buffer, FwSignedSizeType& size, BaremetalFile::WaitType wait) {
    FW_ASSERT(buffer != nullptr);

    // make sure it has been opened
    if (!this->isOpen()) {
        size = 0;
        return NOT_OPENED;
    }

    // get file state entry
    FW_ASSERT(this->m_handle.m_file_descriptor != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_handle.m_file_descriptor - MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

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

BaremetalFile::Status BaremetalFile::write(const U8* buffer, FwSignedSizeType& size, BaremetalFile::WaitType wait) {
    FW_ASSERT(buffer != nullptr);

    // make sure it has been opened in write mode
    switch (this->m_handle.m_mode) {
        case OPEN_NO_MODE:
            return NOT_OPENED;
        case OPEN_WRITE:  // all fall through for write cases
        case OPEN_SYNC_WRITE:
        case OPEN_CREATE:
        case OPEN_APPEND:
            break;
        default:
            size = 0;
            return OTHER_ERROR;
    }

    // get file state entry
    FW_ASSERT(this->m_handle.m_file_descriptor != -1);
    MicroFsFileState* state = getFileStateFromIndex(this->m_handle.m_file_descriptor - MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

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
    if (state->loc > state->currSize) {
        state->currSize = state->loc;
    }

    return OP_OK;
}

FileHandle* BaremetalFile::getHandle() {
    return &this->m_handle;
}

}  // namespace File
}  // namespace Baremetal
}  // namespace Os
