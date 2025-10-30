//! ======================================================================
//! \title fprime-baremetal/Os/Baremetal/MemoryIdScope.hpp
//! \brief Header file defining a class that can be used to temporarily
//! over set the default memory ID (which is used if OverrideNewDelete is
//! included in the deployment, else is a no-op)
// ======================================================================
#ifndef OS_Baremetal_MemoryIdScope_HPP
#define OS_Baremetal_MemoryIdScope_HPP

#include <Fw/FPrimeBasicTypes.h>

// Define a weak linkage for setDefaultMemoryId(), that
// weak link will be replaced with the function from OverrideNewDelete
// if that module is used by the deployment
extern "C" void setDefaultMemoryId(FwEnumStoreType tmpId) __attribute__((weak));

namespace Os {
namespace Baremetal {

//! Default memory ID scoped configuration class (sets the default that will be detected by
//! OverrideNewDelete when the constructor is called and restores the default when destructor is called)
class MemoryIdScope {
  public:
    //! Default memory ID
    static constexpr FwEnumStoreType DEFAULT_ID = -1;

    MemoryIdScope(FwEnumStoreType tmpId) { setDefaultMemoryId(tmpId); }

    ~MemoryIdScope() { setDefaultMemoryId(DEFAULT_ID); }
};

}  // namespace Baremetal
}  // namespace Os
#endif  // OS_Baremetal_MemoryIdScope_HPP
