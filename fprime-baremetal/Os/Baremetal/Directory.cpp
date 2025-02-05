// ======================================================================
// \title Os/Baremetal/Directory.cpp
// \brief Baremetal implementation for Os::Directory
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <cstdio>
#include <fprime-baremetal/Os/Baremetal/Directory.hpp>
#include <fprime-baremetal/Os/Baremetal/MicroFs/MicroFs.hpp>
#include <fprime-baremetal/Os/Baremetal/error.hpp>

namespace Os {
namespace Baremetal {
namespace Directory {

BaremetalDirectory::BaremetalDirectory() : Os::DirectoryInterface(), m_handle() {}

DirectoryHandle* BaremetalDirectory::getHandle() {
    return &this->m_handle;
}

BaremetalDirectory::Status BaremetalDirectory::open(const char* path, OpenMode /*mode*/) {
    // The directory can only be one of the
    // bin file names, and it is already "created" if
    // the bin exists. See if the directory matches
    // one of the bins, and if it does, return OK, otherwise
    // return NO_PERMISSION.

    const char* dirPathSpec = "/" MICROFS_BIN_STRING "%d";

    // if the directory number can be scanned out following the directory path spec,
    // the directory name has the correct format

    FwIndexType binIndex = 0;

    FwNativeUIntType stat = sscanf(path, dirPathSpec, &binIndex);
    if (stat != 1) {
        return NO_PERMISSION;
    }

    // If the path format is correct, check to see if it is in the
    // range of bins
    if (binIndex < static_cast<FwIndexType>(MicroFs::getSingleton().s_microFsConfig.numBins)) {
        return OP_OK;
    } else {
        return NO_PERMISSION;
    }
}

BaremetalDirectory::Status BaremetalDirectory::rewind() {
    Status status = Status::OP_OK;
    return status;
}

BaremetalDirectory::Status BaremetalDirectory::read(char* fileNameBuffer, FwSizeType bufSize) {
    FW_ASSERT(fileNameBuffer);

    Status status = Status::OP_OK;

    return status;
}

void BaremetalDirectory::close() {}

}  // namespace Directory
}  // namespace Baremetal
}  // namespace Os
