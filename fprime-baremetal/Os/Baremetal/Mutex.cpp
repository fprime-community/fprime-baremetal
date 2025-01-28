// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Mutex.cpp
// \brief Baremetal implementation for Os::Mutex
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/Mutex.hpp"

namespace Os {
namespace Baremetal {
namespace Mutex {

BaremetalMutex::Status BaremetalMutex::take() {
    return Status::OP_OK;
}

BaremetalMutex::Status BaremetalMutex::release() {
    return Status::OP_OK;
}

MutexHandle* BaremetalMutex::getHandle() {
    return nullptr;
}

}  // namespace Mutex
}  // namespace Baremetal
}  // namespace Os
