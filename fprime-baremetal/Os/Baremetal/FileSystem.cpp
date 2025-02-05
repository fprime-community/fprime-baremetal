// ======================================================================
// \title Os/Baremetal/FileSystem.cpp
// \brief Baremetal implementation for Os::FileSystem
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/FileSystem.hpp"
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include "fprime-baremetal/Os/Baremetal/error.hpp"

namespace Os {
namespace Baremetal {
namespace FileSystem {

BaremetalFileSystem::Status BaremetalFileSystem::_removeDirectory(const char* path) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_removeFile(const char* path) {
    if (path == nullptr) {
        return INVALID_PATH;
    }

    // get file state
    FwIndexType index = MicroFs::getFileStateIndex(path);
    if (index == -1) {
        return INVALID_PATH;
    }

    MicroFs::MicroFsFileState* fState = MicroFs::getFileStateFromIndex(index);
    FW_ASSERT(fState != nullptr);

    if (fState->loc != -1) {
        return BUSY;
    }

    // delete the file by setting current size to be -1
    fState->currSize = -1;
    fState->loc = -1;

    return OP_OK;
}

BaremetalFileSystem::Status BaremetalFileSystem::_rename(const char* originPath, const char* destPath) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_getWorkingDirectory(char* path, FwSizeType bufferSize) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_changeWorkingDirectory(const char* path) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_getFreeSpace(const char* path,
                                                               FwSizeType& totalBytes,
                                                               FwSizeType& freeBytes) {
    return Status::NOT_SUPPORTED;
}

FileSystemHandle* BaremetalFileSystem::getHandle() {
    return &this->m_handle;
}

}  // namespace FileSystem
}  // namespace Baremetal
}  // namespace Os
