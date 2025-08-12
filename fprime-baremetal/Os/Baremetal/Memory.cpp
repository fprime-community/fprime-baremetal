// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Memory.cpp
// \brief implementation for Os::Baremetal::BaremetalMemory
// ======================================================================
#include <fprime-baremetal/Os/Baremetal/Memory.hpp>

namespace Os {
namespace Baremetal {

MemoryInterface::Status BaremetalMemory::_getUsage(Os::Memory::Usage& memory_usage) {
    memory_usage.used = 1;
    memory_usage.total = 1;
    return Status::OP_OK;
}

MemoryHandle* BaremetalMemory::getHandle() {
    return &this->m_handle;
}

}  // namespace Baremetal
}  // namespace Os
