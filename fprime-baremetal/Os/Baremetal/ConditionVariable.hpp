// ======================================================================
// \title fprime-baremetal/Os/Baremetal/ConditionVariable.hpp
// \brief Baremetal definitions for Os::ConditionVariable
// ======================================================================
#ifndef OS_BAREMETAL_CONDITION_VARIABLE_HPP
#define OS_BAREMETAL_CONDITION_VARIABLE_HPP
#include <Os/Condition.hpp>

namespace Os {
namespace Baremetal {
namespace Mutex {

//! \brief Baremetal implementation of Os::ConditionVariable
//!
//! Baremetal implementation of `ConditionVariable` for use as a delegate class handling error-only file operations.
//!
class BaremetalConditionVariable : public ConditionVariableInterface {
  public:
    //! \brief constructor
    //!
    BaremetalConditionVariable() = default;

    //! \brief destructor
    //!
    ~BaremetalConditionVariable() override = default;

    //! \brief assignment operator is forbidden
    ConditionVariableInterface& operator=(const ConditionVariableInterface& other) override = delete;

    //! \brief wait releasing mutex
    ConditionVariableInterface::Status pend(Os::Mutex& mutex) override;

    //! \brief notify a single waiter
    void notify() override;

    //! \brief notify all current waiters
    void notifyAll() override;

    //! \brief get handle
    ConditionVariableHandle* getHandle() override;
};

}  // namespace Mutex
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_BAREMETAL_CONDITION_VARIABLE_HPP
