// ======================================================================
// \title Os/Baremetal/File.cpp
// \brief baremetal implementation for Os::File
// ======================================================================

#include <Fw/Types/Assert.hpp>
#include <Os/File.hpp>
#include <fprime-baremetal/Os/Baremetal/File.hpp>
#include <fprime-baremetal/Os/Baremetal/error.hpp>

namespace Os {
namespace Baremetal {
namespace File {

//!\brief default copy constructor
BaremetalFile::BaremetalFile(const BaremetalFile& other) {}

BaremetalFile& BaremetalFile::operator=(const BaremetalFile& other) {}

BaremetalFile::Status BaremetalFile::open(const char* filepath,
                                          BaremetalFile::Mode requested_mode,
                                          BaremetalFile::OverwriteType overwrite) {
    Status status = OP_OK;
    return status;
}

void BaremetalFile::close() {}

BaremetalFile::Status BaremetalFile::size(FwSignedSizeType& size_result) {
    FwSignedSizeType current_position = 0;
    Status status = this->position(current_position);
    return status;
}

BaremetalFile::Status BaremetalFile::position(FwSignedSizeType& position_result) {
    Status status = OP_OK;
    return status;
}

BaremetalFile::Status BaremetalFile::preallocate(FwSignedSizeType offset, FwSignedSizeType length) {
    BaremetalFile::Status status = Os::File::Status::NOT_SUPPORTED;
    return status;
}

BaremetalFile::Status BaremetalFile::seek(FwSignedSizeType offset, BaremetalFile::SeekType seekType) {
    Status status = OP_OK;
    return status;
}

BaremetalFile::Status BaremetalFile::flush() {
    BaremetalFile::Status status = OP_OK;
    return status;
}

BaremetalFile::Status BaremetalFile::read(U8* buffer, FwSignedSizeType& size, BaremetalFile::WaitType wait) {
    Status status = OP_OK;
    return status;
}

BaremetalFile::Status BaremetalFile::write(const U8* buffer, FwSignedSizeType& size, BaremetalFile::WaitType wait) {
    Status status = OP_OK;
    return status;
}

FileHandle* BaremetalFile::getHandle() {
    return &this->m_handle;
}

}  // namespace File
}  // namespace Baremetal
}  // namespace Os
