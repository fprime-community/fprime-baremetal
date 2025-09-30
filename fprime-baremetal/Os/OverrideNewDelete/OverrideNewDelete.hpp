// ======================================================================
// \title fprime-baremetal/Os/Baremetal/OverrideNewDelete.hpp
// \brief Header file for overriding new & delete operators with custom implementations
// that use a MemAllocator
// ======================================================================
#ifndef OS_Baremetal_OverrideNewDelete_HPP
#define OS_Baremetal_OverrideNewDelete_HPP
#include <stdio.h>
#include <Fw/Types/MallocAllocator.hpp>

//! delete operator that supports an identifier argument
void operator delete[](void* ptr, const FwEnumStoreType identifier) noexcept;
//! delete[] operator that supports an identifier argument
void operator delete(void* ptr, const FwEnumStoreType identifier) noexcept;
//! new operator that supports an identifier argument
void* operator new[](std::size_t size, const FwEnumStoreType identifier);
//! new[] operator that supports an identifier argument
void* operator new(std::size_t size, const FwEnumStoreType identifier);

namespace Os {
namespace Baremetal {
namespace OverrideNewDelete {

//! ID passed to Fw::MemAllocator::allocate() and Fw::MemAllocator::deallocate() calls
constexpr FwEnumStoreType DEFAULT_ID = -1;

//! \brief Register a memory allocator for all future new operator calls
//!
//! \param allocator MemAllocator to use for all future new/delete calls
//! \return Returns number of bytes allocated before
FwSizeType registerMemAllocator(Fw::MemAllocator* allocator);

void setDefaultId(FwEnumStoreType tmpId);
// Temporary change default ID
class MemoryIdScope {
  public:
    MemoryIdScope(FwEnumStoreType tmpId) {
        puts("set default ID");
        setDefaultId(tmpId);
    }

    ~MemoryIdScope() {
        puts("restore default ID");
        setDefaultId(DEFAULT_ID);
    }
};

}  // namespace OverrideNewDelete
}  // namespace Baremetal
}  // namespace Os
#endif  // OS_Baremetal_OverrideNewDelete_HPP