// ======================================================================
// \title Os/Baremetal/FileSystem.cpp
// \brief Baremetal implementation for Os::FileSystem
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/FileSystem.hpp"
#include "fprime-baremetal/Os/Baremetal/error.hpp"

namespace Os {
namespace Baremetal {
namespace FileSystem {

BaremetalFileSystem::Status BaremetalFileSystem::_removeDirectory(const char* path) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_removeFile(const char* path) {
    Status status = OP_OK;
    return status;
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
