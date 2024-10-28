// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Mutex.cpp
// \brief Baremetal implementation for Os::Mutex
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/Mutex.hpp"

namespace Os {
namespace Baremetal {

BaremetalMutex::Status BaremetalMutex::take() {
    return Status::OP_OK;
}

BaremetalMutex::Status BaremetalMutex::release() {
    return Status::OP_OK;
}

MutexHandle* BaremetalMutex::getHandle() {
    return &this->m_handle;
}

}  // namespace Baremetal
}  // namespace Os
