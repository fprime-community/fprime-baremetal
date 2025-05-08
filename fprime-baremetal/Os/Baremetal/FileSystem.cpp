// ======================================================================
// \title Os/Baremetal/FileSystem.cpp
// \brief Baremetal implementation for Os::FileSystem
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/FileSystem.hpp"
#include <cstdio>
#include <cstring>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include "fprime-baremetal/Os/Baremetal/error.hpp"

namespace Os {
namespace Baremetal {
namespace FileSystem {

BaremetalFileSystem::Status BaremetalFileSystem::_removeDirectory(const char* path) {
    FW_ASSERT(path != nullptr);

    const char* dirPathSpec = "/" MICROFS_BIN_STRING "%" MICROFS_INDEX_SCN_FORMAT;

    // if the directory number can be scanned out following the directory path spec,
    // the directory name has the correct format
    FwIndexType binIndex = 0;

    int stat = sscanf(path, dirPathSpec, &binIndex);
    if (stat != 1) {
        return NO_PERMISSION;
    }

    // If the path format is correct, check to see if it is in the
    // range of bins
    if (binIndex < MicroFs::getSingleton().s_microFsConfig.numBins) {
        return OP_OK;
    } else {
        return NO_PERMISSION;
    }
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

    for (FwIndexType i = 0; i < MAX_MICROFS_FD; i++) {
        if (fState->status[i] != MicroFs::Status::INVALID) {
            return BUSY;
        }
    }

    // delete the file by setting created to false
    fState->created = false;

    return OP_OK;
}

BaremetalFileSystem::Status BaremetalFileSystem::_rename(const char* originPath, const char* destPath) {
    // Status copyStat = this->copyFile(originPath, destPath);
    Status copyStat = Os::FileSystem::copyFile(originPath, destPath);
    if (copyStat != OP_OK) {
        return copyStat;
    }

    return this->_removeFile(originPath);
}

BaremetalFileSystem::Status BaremetalFileSystem::_getWorkingDirectory(char* path, FwSizeType bufferSize) {
    return NOT_SUPPORTED;
}

BaremetalFileSystem::Status BaremetalFileSystem::_changeWorkingDirectory(const char* path) {
    return OP_OK;
}

BaremetalFileSystem::Status BaremetalFileSystem::_getFreeSpace(const char* path,
                                                               FwSizeType& totalBytes,
                                                               FwSizeType& freeBytes) {
    totalBytes = 0;
    freeBytes = 0;

    // Get first file state struct
    MicroFs::MicroFsFileState* statePtr =
        reinterpret_cast<MicroFs::MicroFsFileState*>(MicroFs::getSingleton().s_microFsMem);
    FW_ASSERT(statePtr != nullptr);

    // iterate through bins
    for (FwIndexType currBin = 0; currBin < static_cast<FwIndexType>(MicroFs::getSingleton().s_microFsConfig.numBins);
         currBin++) {
        // iterate through files in each bin
        for (FwIndexType currFile = 0;
             currFile < static_cast<FwIndexType>(MicroFs::getSingleton().s_microFsConfig.bins[currBin].numFiles);
             currFile++) {
            totalBytes += statePtr->dataSize;
            // only add unused file slots to free space
            if (!statePtr->created) {
                freeBytes += statePtr->dataSize;
            }
            statePtr += 1;
        }
    }

    return OP_OK;
}

FileSystemHandle* BaremetalFileSystem::getHandle() {
    return &this->m_handle;
}

}  // namespace FileSystem
}  // namespace Baremetal
}  // namespace Os
