// ======================================================================
// \title Os/Baremetal/DefaultCpu.cpp
// \brief sets default Os::Baremetal::BaremetalCpu to stub implementation via linker
// ======================================================================
#include "Os/Cpu.hpp"
#include "Os/Delegate.hpp"
#include "fprime-baremetal/Os/Baremetal//Cpu.hpp"

namespace Os {
CpuInterface* CpuInterface::getDelegate(CpuHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<CpuInterface, Os::Baremetal::BaremetalCpu>(aligned_new_memory);
}
}  // namespace Os
