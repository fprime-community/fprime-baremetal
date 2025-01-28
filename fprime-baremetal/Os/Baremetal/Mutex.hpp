// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Mutex.hpp
// \brief Baremetal definitions for Os::Mutex
// ======================================================================
#include "Os/Mutex.hpp"

#ifndef OS_BAREMETAL_MUTEX_HPP
#define OS_BAREMETAL_MUTEX_HPP
namespace Os {
namespace Baremetal {
namespace Mutex {

//! \brief Baremetal implementation of Os::Mutex
//!
//! Baremetal implementation of `MutexInterface` for use as a delegate class handling error-only file operations.
//!
class BaremetalMutex : public MutexInterface {
  public:
    //! \brief constructor
    //!
    BaremetalMutex() = default;

    //! \brief destructor
    //!
    ~BaremetalMutex() override = default;

    //! \brief return the underlying mutex handle (implementation specific)
    //! \return internal mutex handle representation
    MutexHandle* getHandle() override;

    Status take() override;     //!<  lock the mutex and get return status
    Status release() override;  //!<  unlock the mutex and get return status
};

}  // namespace Mutex
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_BAREMETAL_MUTEX_HPP
