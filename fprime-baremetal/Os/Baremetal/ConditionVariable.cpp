// ======================================================================
// \title fprime-baremetal/Os/Baremetal/ConditionVariable.cpp
// \brief Baremetal implementations for Os::ConditionVariable
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/ConditionVariable.hpp"
#include "Fw/Types/Assert.hpp"

namespace Os {
namespace Baremetal {
namespace Mutex {

ConditionVariableInterface::Status BaremetalConditionVariable::pend(Os::Mutex& mutex) {
    return ConditionVariableInterface::Status::ERROR_NOT_IMPLEMENTED;
}
void BaremetalConditionVariable::notify() {}
void BaremetalConditionVariable::notifyAll() {}

ConditionVariableHandle* BaremetalConditionVariable::getHandle() {
    return nullptr;
}

}  // namespace Mutex
}  // namespace Baremetal
}  // namespace Os
