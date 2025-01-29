// ======================================================================
// \title Os/Baremetal/Directory.cpp
// \brief Baremetal implementation for Os::Directory
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Fw/Types/StringUtils.hpp>
#include <fprime-baremetal/Os/Baremetal/Directory.hpp>
#include <fprime-baremetal/Os/Baremetal/error.hpp>

namespace Os {
namespace Baremetal {
namespace Directory {

BaremetalDirectory::BaremetalDirectory() : Os::DirectoryInterface(), m_handle() {}

DirectoryHandle* BaremetalDirectory::getHandle() {
    return &this->m_handle;
}

BaremetalDirectory::Status BaremetalDirectory::open(const char* path, OpenMode mode) {
    Status status = Status::OP_OK;
    return status;
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
