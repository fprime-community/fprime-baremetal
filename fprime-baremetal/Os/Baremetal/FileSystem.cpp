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

    const char* dirPathSpec = "/" MICROFS_BIN_STRING "%d";

    // if the directory number can be scanned out following the directory path spec,
    // the directory name has the correct format
    PlatformSizeType binIndex = 0;

    PlatformIntType stat = sscanf(path, dirPathSpec, &binIndex);
    if (stat != 1) {
        return NO_PERMISSION;
    }

    // If the path format is correct, check to see if it is in the
    // range of bins
    if (binIndex < static_cast<PlatformSizeType>(MicroFs::getSingleton().s_microFsConfig.numBins)) {
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

    if (fState->loc != -1) {
        return BUSY;
    }

    // delete the file by setting current size to be -1
    fState->currSize = -1;
    fState->loc = -1;

    return OP_OK;
}

BaremetalFileSystem::Status BaremetalFileSystem::_rename(const char* originPath, const char* destPath) {
    Status copyStat = this->copyFile(originPath, destPath);
    if (copyStat != OP_OK) {
        return copyStat;
    }

    return this->_removeFile(originPath);
}

BaremetalFileSystem::Status BaremetalFileSystem::_getWorkingDirectory(char* path, FwSizeType bufferSize) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::_changeWorkingDirectory(const char* path) {
    Status status = OP_OK;
    return status;
}

BaremetalFileSystem::Status BaremetalFileSystem::copyFile(const char* originPath, const char* destPath) {
    if ((originPath == nullptr) || (destPath == nullptr)) {
        return INVALID_PATH;
    }

    // get file state of origin
    FwIndexType origIndex = MicroFs::getFileStateIndex(originPath);
    if (origIndex == -1) {
        return INVALID_PATH;
    }

    MicroFs::MicroFsFileState* origState = MicroFs::getFileStateFromIndex(origIndex);
    FW_ASSERT(origState != nullptr);

    // get file state of dest
    FwIndexType destIndex = MicroFs::getFileStateIndex(destPath);
    if (-1 == destIndex) {
        return INVALID_PATH;
    }

    MicroFs::MicroFsFileState* destState = MicroFs::getFileStateFromIndex(destIndex);
    FW_ASSERT(destState != nullptr);

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

BaremetalFileSystem::Status BaremetalFileSystem::_getFreeSpace(const char* path,
                                                               FwSizeType& totalBytes,
                                                               FwSizeType& freeBytes) {
    totalBytes = 0;
    freeBytes = 0;

    auto microFs = MicroFs::getSingleton();

    // Get first file state struct
    MicroFs::MicroFsFileState* statePtr = reinterpret_cast<MicroFs::MicroFsFileState*>(microFs.s_microFsMem);
    FW_ASSERT(statePtr != nullptr);

    // iterate through bins
    for (FwIndexType currBin = 0; currBin < static_cast<FwIndexType>(microFs.s_microFsConfig.numBins); currBin++) {
        // iterate through files in each bin
        for (FwIndexType currFile = 0;
             currFile < static_cast<FwIndexType>(microFs.s_microFsConfig.bins[currBin].numFiles); currFile++) {
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

FileSystemHandle* BaremetalFileSystem::getHandle() {
    return &this->m_handle;
}

}  // namespace FileSystem
}  // namespace Baremetal
}  // namespace Os
