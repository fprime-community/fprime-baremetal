// ======================================================================
// \title Os/Baremetal/File.cpp
// \brief baremetal implementation for Os::File
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Os/File.hpp>
#include <cstring>
#include <fprime-baremetal/Os/Baremetal/File.hpp>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include <fprime-baremetal/Os/Baremetal/error.hpp>

namespace Os {
namespace Baremetal {
namespace File {

BaremetalFile::~BaremetalFile() {
    if (this->isOpen()) {
        this->close();
    }
}

BaremetalFile::Status BaremetalFile::open(const char* path,
                                          BaremetalFile::Mode mode,
                                          BaremetalFile::OverwriteType overwrite) {
    if (path == nullptr) {
        return OTHER_ERROR;
    }
    Status stat = OP_OK;

    // common checks
    // retrieve index to file entry
    FwIndexType entry = MicroFs::getFileStateIndex(path);
    // not found
    if (BaremetalFileHandle::INVALID_STATE_ENTRY == entry) {
        return Os::File::Status::DOESNT_EXIST;
    }

    MicroFs::MicroFsFileState* state = MicroFs::getFileStateFromIndex(entry);
    FW_ASSERT(state != nullptr);

    FwIndexType fdEntry = MicroFs::getFileStateNextFreeFd(path);
    if (fdEntry == BaremetalFileHandle::INVALID_FILE_DESCRIPTOR) {
        return Os::File::Status::NO_PERMISSION;
    }

    switch (mode) {
        case OPEN_READ:
            // if not written to yet, doesn't exist for read
            if (-1 == state->currSize) {
                return Os::File::Status::DOESNT_EXIST;
            }
            state->loc[fdEntry] = 0;
            break;
        case OPEN_WRITE:
        case OPEN_SYNC_WRITE:  // fall through; same for microfs
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (-1 == state->currSize) {
                state->currSize = 0;
            }
            state->loc[fdEntry] = 0;
            break;
        case OPEN_CREATE:
            // truncate file length to zero
            state->currSize = 0;
            state->loc[fdEntry] = 0;
            break;
        case OPEN_APPEND:
            // initialize write location to length of file for append
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (-1 == state->currSize) {
                state->currSize = 0;
            }

            state->loc[fdEntry] = state->currSize;
            break;
        default:
            FW_ASSERT(0, mode);
            break;
    }

    // store mode
    this->m_handle.m_mode = mode;

    // store file descriptor for this file
    this->m_handle.m_file_descriptor = fdEntry;

    // store state entry into state structure
    this->m_handle.m_state_entry = entry + MicroFs::MICROFS_FD_OFFSET;

    return stat;
}

bool BaremetalFile::isOpen() const {
    FW_ASSERT((0 <= this->m_handle.m_mode) && (this->m_handle.m_mode < Mode::MAX_OPEN_MODE), this->m_handle.m_mode);
    return (this->m_handle.m_mode != OPEN_NO_MODE);
}

BaremetalFile::Status BaremetalFile::preallocate(FwSizeType offset, FwSizeType length) {
    // do nothing; in RAM
    return Os::File::Status::OP_OK;
}

void BaremetalFile::close() {
    if (this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY) {
        // only do cleanup of file state
        // if file system memory is still around
        // catches case where file objects are still
        // lingering after cleanup
        if ((MicroFs::getSingleton()).s_microFsMem) {
            // get state to clear it
            MicroFs::MicroFsFileState* state =
                MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
            FW_ASSERT(state != nullptr);
            state->loc[this->m_handle.m_file_descriptor] = -1;
        }
    }
    // reset fd
    this->m_handle.m_file_descriptor = BaremetalFileHandle::INVALID_FILE_DESCRIPTOR;
    this->m_handle.m_mode = OPEN_NO_MODE;
    // clear
}

BaremetalFile::Status BaremetalFile::size(FwSizeType& size_result) {
    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);
    size_result = state->currSize;

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::position(FwSizeType& position_result) {
    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);
    position_result = state->loc[this->m_handle.m_file_descriptor];

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::seek(FwSignedSizeType offset, BaremetalFile::SeekType seekType) {
    // make sure it has been opened
    if (!this->isOpen()) {
        return NOT_OPENED;
    }
    // get file state entry
    FW_ASSERT(this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY);

    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    FwNativeIntType oldSize = state->currSize;

    // compute new operation location
    switch (seekType) {
        case SeekType::ABSOLUTE:
            // make sure not too far
            if ((offset >= static_cast<NATIVE_INT_TYPE>(state->dataSize)) or (offset < 0)) {
                return BAD_SIZE;
            }
            state->loc[this->m_handle.m_file_descriptor] = offset;
            break;
        case SeekType::RELATIVE:
            // make sure not too far
            if (state->loc[this->m_handle.m_file_descriptor] + offset >= static_cast<FwIndexType>(state->dataSize)) {
                return BAD_SIZE;
            }
            state->loc[this->m_handle.m_file_descriptor] = state->loc[this->m_handle.m_file_descriptor] + offset;
            break;
        default:
            FW_ASSERT(0, seekType);
            break;
    }

    // move current size as well if needed
    if (state->loc[this->m_handle.m_file_descriptor] > state->currSize) {
        state->currSize = state->loc[this->m_handle.m_file_descriptor];
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

BaremetalFile::Status BaremetalFile::read(U8* buffer, FwSizeType& size, BaremetalFile::WaitType wait) {
    FW_ASSERT(buffer != nullptr);

    // make sure it has been opened
    if (!this->isOpen()) {
        size = 0;
        return NOT_OPENED;
    }
    // get file state entry
    FW_ASSERT(this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY);

    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    // find size to copy

    // check to see if already at the end of the file. If so, return 0 for size
    if (state->loc[this->m_handle.m_file_descriptor] == (state->currSize - 1)) {
        size = 0;
        return OP_OK;
    }

    // copy requested bytes, unless it would be more than the file size.
    // If it would be more than the file size, copy the remainder and set
    // the size to the actual copied
    if ((state->loc[this->m_handle.m_file_descriptor] + size) > (state->currSize - 1)) {
        size = state->currSize - state->loc[this->m_handle.m_file_descriptor];
    }

    // copy data from location to buffer
    (void)memcpy(buffer, state->data + state->loc[this->m_handle.m_file_descriptor], size);

    // move location pointer
    state->loc[this->m_handle.m_file_descriptor] += size;

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::write(const U8* buffer, FwSizeType& size, BaremetalFile::WaitType wait) {
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
    FW_ASSERT(this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY);

    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    // write up to the end of the allocated buffer
    // if write size is greater, truncate the write
    // and set size to what was actually written
    if (state->loc[this->m_handle.m_file_descriptor] + size > static_cast<FwIndexType>(state->dataSize)) {
        size = state->dataSize - state->loc[this->m_handle.m_file_descriptor];
    }

    // copy data to file buffer
    (void)memcpy(&state->data[state->loc[this->m_handle.m_file_descriptor]], buffer, size);

    // increment location
    state->loc[this->m_handle.m_file_descriptor] += size;

    // Check if the currSize is to be increased.
    if (state->loc[this->m_handle.m_file_descriptor] > state->currSize) {
        state->currSize = state->loc[this->m_handle.m_file_descriptor];
    }

    return OP_OK;
}

FileHandle* BaremetalFile::getHandle() {
    return &this->m_handle;
}

}  // namespace File
}  // namespace Baremetal
}  // namespace Os
