//! ======================================================================
//! \title fprime-baremetal/Os/Baremetal/MemoryIdScope.cpp
//! \brief Implementation for MemoryIdScope.hpp
// ======================================================================

#include "MemoryIdScope.hpp"
#include <Fw/FPrimeBasicTypes.h>

namespace Os {
namespace Baremetal {
// global variables
//! Modifiable default (useful before calling code with new/delete to attribute the memory user)
FwEnumStoreType defaultMemoryId = MemoryIdScope::DEFAULT_ID;

}  // namespace Baremetal
}  // namespace Os
