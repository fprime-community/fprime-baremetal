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

BaremetalFile::BaremetalFile(const BaremetalFile& other) {
    printf("baremetal: Calling copy constructor\n");
    fflush(stdout);

    this->m_handle.m_state_entry = other.m_handle.m_state_entry;
    this->m_handle.m_file_descriptor = other.m_handle.m_file_descriptor;
    this->m_handle.m_mode = other.m_handle.m_mode;

    if(other.m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY && other.m_handle.m_file_descriptor != BaremetalFileHandle::INVALID_FILE_DESCRIPTOR) {
        MicroFs::MicroFsFileState* state = MicroFs::getFileStateFromIndex(other.m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
        FW_ASSERT(state != nullptr);

        FwIndexType fdEntry = 0;
        auto status = MicroFs::getFileStateNextFreeFd(state, fdEntry);
        FW_ASSERT(status != MicroFs::Status::INVALID);

        state->fd[fdEntry].loc = state->fd[other.m_handle.m_file_descriptor].loc;
        state->fd[fdEntry].status = state->fd[other.m_handle.m_file_descriptor].status;

        // store file descriptor for this file
        this->m_handle.m_file_descriptor = fdEntry;
        printf("baremetal: copy constructor old fd, new fd -> %d %d\n", other.m_handle.m_file_descriptor, this->m_handle.m_file_descriptor);
        fflush(stdout);
    }
}

BaremetalFile& BaremetalFile::operator=(const BaremetalFile& other) {
    printf("baremetal: assignment operator\n");
    fflush(stdout);
    if (this != &other) {
        //this->m_handle.m_file_descriptor = fcntl(other.m_handle.m_file_descriptor, F_DUPFD, 0);
        this->m_handle.m_state_entry = other.m_handle.m_state_entry;
        this->m_handle.m_file_descriptor = other.m_handle.m_file_descriptor;
        this->m_handle.m_mode = other.m_handle.m_mode;

        if(other.m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY && other.m_handle.m_file_descriptor != BaremetalFileHandle::INVALID_FILE_DESCRIPTOR) {
            MicroFs::MicroFsFileState* state = MicroFs::getFileStateFromIndex(other.m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
            FW_ASSERT(state != nullptr);

            FwIndexType fdEntry = 0;
            auto status = MicroFs::getFileStateNextFreeFd(state, fdEntry);
            FW_ASSERT(status != MicroFs::Status::INVALID);

            state->fd[fdEntry].loc = state->fd[other.m_handle.m_file_descriptor].loc;
            state->fd[fdEntry].status = state->fd[other.m_handle.m_file_descriptor].status;

            // store file descriptor for this file
            this->m_handle.m_file_descriptor = fdEntry;
            printf("baremetal: assignment operator old fd, new fd -> %d %d\n", other.m_handle.m_file_descriptor, this->m_handle.m_file_descriptor);
            fflush(stdout);
        }
    }
    return *this;
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
    FwIndexType entry = 0;
    auto status = MicroFs::getFileStateIndex(path, entry);
    // not found
    if (status == MicroFs::INVALID) {
        return Os::File::Status::DOESNT_EXIST;
    }

    MicroFs::MicroFsFileState* state = MicroFs::getFileStateFromIndex(entry);
    FW_ASSERT(state != nullptr);

    FwIndexType fdEntry = 0;
    status = MicroFs::getFileStateNextFreeFd(path, fdEntry);
    if (status == MicroFs::Status::INVALID) {
        return Os::File::Status::NO_MORE_RESOURCES;
    }

    switch (mode) {
        case OPEN_READ:
            // if not written to yet, doesn't exist for read
            if (!state->created) {
                return Os::File::Status::DOESNT_EXIST;
            }
            state->fd[fdEntry].loc = 0;
            break;
        case OPEN_WRITE:
        case OPEN_SYNC_WRITE:  // fall through; same for microfs
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (!state->created) {
                state->currSize = 0;
            }
            state->fd[fdEntry].loc = 0;
            break;
        case OPEN_CREATE:
            if (state->created && (overwrite == BaremetalFile::OverwriteType::NO_OVERWRITE)) {
                return Os::File::Status::FILE_EXISTS;
            }
            // truncate file length to zero
            state->currSize = 0;
            state->fd[fdEntry].loc = 0;
            break;
        case OPEN_APPEND:
            // initialize write location to length of file for append
            // If the file has never previously been opened, then initialize the
            // size to 0.
            if (!state->created) {
                state->currSize = 0;
            }

            state->fd[fdEntry].loc = state->currSize;
            break;
        default:
            FW_ASSERT(0, mode);
            break;
    }

    state->created = true;
    state->fd[fdEntry].status = MicroFs::Status::VALID;

    // store mode
    this->m_handle.m_mode = mode;

    // store file descriptor for this file
    this->m_handle.m_file_descriptor = fdEntry;

    // store state entry into state structure
    this->m_handle.m_state_entry = entry + MicroFs::MICROFS_FD_OFFSET;

    return stat;
}

bool BaremetalFile::_isOpen() const {
    FW_ASSERT((0 <= this->m_handle.m_mode) && (this->m_handle.m_mode < Mode::MAX_OPEN_MODE), this->m_handle.m_mode);
    return (this->m_handle.m_mode != OPEN_NO_MODE);
}

BaremetalFile::Status BaremetalFile::preallocate(FwSizeType offset, FwSizeType length) {
    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);
    FwSizeType sum = offset + length;
    auto status = (sum > state->dataSize) ? Os::File::Status::BAD_SIZE : Os::File::Status::OP_OK;
    if (status == Os::File::Status::OP_OK) {
        state->currSize = (sum > state->dataSize) ? state->dataSize : sum;
    }
    return status;
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
            state->fd[this->m_handle.m_file_descriptor].loc = 0;
            state->fd[this->m_handle.m_file_descriptor].status = MicroFs::Status::INVALID;
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
    position_result = state->fd[this->m_handle.m_file_descriptor].loc;

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::seek(FwSignedSizeType offset, BaremetalFile::SeekType seekType) {
    // make sure it has been opened
    if (!this->_isOpen()) {
        return NOT_OPENED;
    }

    // get file state entry
    FW_ASSERT(this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY);

    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    FwSizeType oldSize = state->currSize;

    // compute new operation location
    switch (seekType) {
        case SeekType::ABSOLUTE:
            // make sure not too far
            if ((offset >= static_cast<FwSignedSizeType>(state->dataSize)) or (offset < 0)) {
                return INVALID_ARGUMENT;
            }
            state->fd[this->m_handle.m_file_descriptor].loc = offset;
            break;
        case SeekType::RELATIVE:
            // make sure not too far
            if (static_cast<FwSizeType>(state->fd[this->m_handle.m_file_descriptor].loc + offset) >= state->dataSize) {
                return INVALID_ARGUMENT;
            }
            state->fd[this->m_handle.m_file_descriptor].loc = state->fd[this->m_handle.m_file_descriptor].loc + offset;
            break;
        default:
            FW_ASSERT(0, seekType);
            break;
    }

    // Calculate new size. New size to be used below
    FwSizeType newSize = 0;
    if (state->fd[this->m_handle.m_file_descriptor].loc > state->currSize) {
        newSize = state->fd[this->m_handle.m_file_descriptor].loc;
    }

    // fill with zeros if seek went past old size
    if (newSize > oldSize) {
        (void)memset(&state->data[oldSize], 0, newSize - oldSize);
    }

    return OP_OK;
}

BaremetalFile::Status BaremetalFile::flush() {
    // make sure it has been opened
    if (!this->_isOpen()) {
        return NOT_OPENED;
    }

    // RAM, so no need to flush
    return OP_OK;
}

BaremetalFile::Status BaremetalFile::read(U8* buffer, FwSizeType& size, BaremetalFile::WaitType wait) {
    FW_ASSERT(buffer != nullptr);

    // make sure it has been opened
    if (!this->_isOpen()) {
        size = 0;
        return NOT_OPENED;
    }
    // get file state entry
    FW_ASSERT(this->m_handle.m_state_entry != BaremetalFileHandle::INVALID_STATE_ENTRY);

    MicroFs::MicroFsFileState* state =
        MicroFs::getFileStateFromIndex(this->m_handle.m_state_entry - MicroFs::MICROFS_FD_OFFSET);
    FW_ASSERT(state != nullptr);

    // Make code more readable
    auto& loc = state->fd[this->m_handle.m_file_descriptor].loc;

    // find size to copy

    // check to see if already at the end of the file. If so, return 0 for size
    if (loc >= (state->currSize - 1)) {
        size = 0;
        return OP_OK;
    }

    // copy requested bytes, unless it would be more than the file size.
    // If it would be more than the file size, copy the remainder and set
    // the size to the actual copied
    if ((loc + size) > (state->currSize - 1)) {
        size = state->currSize - loc;
    }

    // copy data from location to buffer
    (void)memcpy(buffer, state->data + loc, size);

    // move location pointer
    loc += size;

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
    if (state->fd[this->m_handle.m_file_descriptor].loc + size > state->dataSize) {
        size = state->dataSize - state->fd[this->m_handle.m_file_descriptor].loc;
    }

    // copy data to file buffer
    (void)memcpy(&state->data[state->fd[this->m_handle.m_file_descriptor].loc], buffer, size);

    // increment location
    state->fd[this->m_handle.m_file_descriptor].loc += size;

    // Check if the currSize is to be increased.
    if (state->fd[this->m_handle.m_file_descriptor].loc > state->currSize) {
        state->currSize = state->fd[this->m_handle.m_file_descriptor].loc;
    }

    return OP_OK;
}

FileHandle* BaremetalFile::getHandle() {
    return &this->m_handle;
}

}  // namespace File
}  // namespace Baremetal
}  // namespace Os
