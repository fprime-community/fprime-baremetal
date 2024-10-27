// ======================================================================
// \title fprime-baremetal/Os/Baremetal/DefaultMutex.cpp
// \brief sets default Os::Mutex to no-op Baremetal implementation via linker
// ======================================================================
#include "fprime-baremetal/Os/Baremetal/ConditionVariable.hpp"
#include "fprime-baremetal/Os/Baremetal/Mutex.hpp"
#include "Os/Delegate.hpp"

namespace Os {

//! \brief get a delegate for MutexInterface that intercepts calls for Baremetal file usage
//! \param aligned_new_memory: aligned memory to fill
//! \param to_copy: pointer to copy-constructor input
//! \return: pointer to delegate
MutexInterface* MutexInterface::getDelegate(MutexHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MutexInterface, Os::Baremetal::Mutex::BaremetalMutex>(aligned_new_memory);
}

//! \brief get a delegate for condition variable
//! \param aligned_new_memory: aligned memory to fill
//! \return: pointer to delegate
ConditionVariableInterface* ConditionVariableInterface::getDelegate(
    ConditionVariableHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<ConditionVariableInterface, Os::Baremetal::Mutex::BaremetalConditionVariable,
                                      ConditionVariableHandleStorage>(aligned_new_memory);
}
}  // namespace Os
