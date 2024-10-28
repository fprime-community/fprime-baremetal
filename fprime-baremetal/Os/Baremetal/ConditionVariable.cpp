// ======================================================================
// \title fprime-baremetal/Os/Baremetal/ConditionVariable.cpp
// \brief Baremetal implementations for Os::ConditionVariable
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/ConditionVariable.hpp"
#include "Fw/Types/Assert.hpp"

namespace Os {
namespace Baremetal {

void BaremetalConditionVariable::wait(Os::Mutex& mutex) {}
void BaremetalConditionVariable::notify() {}
void BaremetalConditionVariable::notifyAll() {}

ConditionVariableHandle* BaremetalConditionVariable::getHandle() {
    return nullptr;
}

}  // namespace Baremetal
}  // namespace Os
