// ======================================================================
// \title fprime-baremetal/Os/Baremetal/Memory.hpp
// \brief implementation for Os::Baremetal::BaremetalMemory, header definitions
// ======================================================================
#include <cstdio>
#include <Os/Memory.hpp>
#ifndef OS_Baremetal_Memory_HPP
#define OS_Baremetal_Memory_HPP

namespace Os {
namespace Baremetal {

//! MemoryHandle class definition for stub implementations.
//!
struct BaremetalMemoryHandle : public MemoryHandle {
};

//! \brief stub implementation of Os::MemoryInterface
//!
//! Stub implementation of `MemoryInterface` for use as a delegate class handling stub console operations.
//!
class BaremetalMemory : public MemoryInterface {
  public:
    //! \brief constructor
    //!
    BaremetalMemory() = default;

    //! \brief copy constructor
    BaremetalMemory(const BaremetalMemory& other) = delete;

    //! \brief default copy assignment
    MemoryInterface& operator=(const MemoryInterface& other) override = delete;

    //! \brief destructor
    //!
    ~BaremetalMemory() override = default;

    // ------------------------------------
    // Functions overrides
    // ------------------------------------

    //! \brief get system memory usage
    //!
    //! This method delegates to the underlying implementation.
    //!
    //! \param memory_usage: (output) data structure used to store memory usage
    //! \return:  ERROR when error occurs, OK otherwise.
    Status _getUsage(Usage& memory_usage) override;

    //! \brief returns the raw console handle
    //!
    //! Gets the raw console handle from the implementation. Note: users must include the implementation specific
    //! header to make any real use of this handle. Otherwise it will be as an opaque type.
    //!
    //! \return raw console handle
    //!
    MemoryHandle *getHandle() override;
  private:
    //! Internal handle
    BaremetalMemoryHandle m_handle;
};
} // namespace Stub
} // namespace Os

#endif // OS_Stub_Memory_HPP
