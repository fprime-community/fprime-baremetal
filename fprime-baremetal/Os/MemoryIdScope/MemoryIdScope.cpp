//! ======================================================================
//! \title fprime-baremetal/Os/Baremetal/MemoryIdScope.cpp
//! \brief Implement weak version of setDefaultMemoryId
// ======================================================================

#include <Fw/FPrimeBasicTypes.h>

extern "C" __attribute__((weak)) void setDefaultMemoryId(FwEnumStoreType tmpId) {
    // Default implementation
}
