//! ======================================================================
//! \title fprime-baremetal/Os/Baremetal/MemoryIdScope.hpp
//! \brief Header file defining a class that can be used to temporarily
//! over set the default memory ID (which is used if OverrideNewDelete is
//! included in the deployment, else is a ignored)
// ======================================================================
#ifndef OS_Baremetal_MemoryIdScope_HPP
#define OS_Baremetal_MemoryIdScope_HPP

#include <Fw/FPrimeBasicTypes.h>

namespace Os {
namespace Baremetal {

//! Configurable default memory ID used by OverrideNewDelete
extern FwEnumStoreType defaultMemoryId;

//! Default memory ID scoped configuration class (sets the default that will be detected by
//! OverrideNewDelete when the constructor is called and restores the default when destructor is called)
class MemoryIdScope {
  public:
    //! Default memory ID
    static constexpr FwEnumStoreType DEFAULT_ID = -1;

    MemoryIdScope(FwEnumStoreType tmpId) { defaultMemoryId = tmpId; }

    ~MemoryIdScope() { defaultMemoryId = DEFAULT_ID; }
};
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_Baremetal_MemoryIdScope_HPP
