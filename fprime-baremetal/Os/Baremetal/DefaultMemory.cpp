// ======================================================================
// \title fprime-baremetal/Os/Baremetal/DefaultMemory.cpp
// \brief sets default Os::Memory to stub implementation via linker
// ======================================================================
#include "Os/Delegate.hpp"
#include "Os/Memory.hpp"
#include "fprime-baremetal/Os/Baremetal/Memory.hpp"

namespace Os {
MemoryInterface* MemoryInterface::getDelegate(MemoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MemoryInterface, Os::Baremetal::BaremetalMemory>(aligned_new_memory);
}
}  // namespace Os
