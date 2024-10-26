// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Cpu.hpp
// \brief stub implementation for Os::Baremetal::BaremetalCpu, implementations
// ======================================================================
#include <fprime-baremetal/Os/Baremetal/Cpu.hpp>

namespace Os {
namespace Baremetal {

CpuInterface::Status BaremetalCpu::_getCount(FwSizeType& cpu_count) {
    // Assumes singular CPU
    cpu_count = 1;
    return Status::OP_OK;
}

CpuInterface::Status BaremetalCpu::_getTicks(Os::Cpu::Ticks& ticks, FwSizeType cpu_index) {
    // Baremetal currently returns 100% in all cases
    ticks.total = 1;
    ticks.used = 1;
    return Status::OP_OK;
}

CpuHandle* BaremetalCpu::getHandle() {
    return &this->m_handle;
}

} // namespace Baremetal
} // namespace Os
