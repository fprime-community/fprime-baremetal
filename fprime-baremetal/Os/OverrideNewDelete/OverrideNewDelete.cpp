// ======================================================================
// \title fprime-baremetal/Os/Baremetal/OverrideNewDelete.cpp
// \brief Implementation for new/delete overrides
// =====================================================================
#include "OverrideNewDelete.hpp"
#include <malloc.h>
#include <stdio.h>
#include <Fw/Types/Assert.hpp>
#include <atomic>
#include <new>  // included to get std::nothrow_t

namespace Os {
namespace Baremetal {
namespace OverrideNewDelete {

// global variables
//! Modifiable default (useful before calling code with new/delete to attribute the memory user)
FwEnumStoreType customId = DEFAULT_ID;
//! Pointer to MemAllocator
static Fw::MemAllocator* pAllocator = nullptr;

// Prototypes
void deallocateMemoryWoId(void* ptr);
void deallocateMemory(const FwEnumStoreType identifier, void* ptr);
void* allocateMemoryWoId(const FwSizeType size);
void* allocateMemory(const FwEnumStoreType identifier, const FwSizeType size);

// Function implementations
void setDefaultId(FwEnumStoreType tmpId) {
    customId = tmpId;
}
FwSizeType registerMemAllocator(Fw::MemAllocator* allocator) {
    FW_ASSERT(pAllocator == nullptr);
    FW_ASSERT(allocator != nullptr);
    pAllocator = allocator;
    // Get & return number of bytes already allocated
    struct mallinfo mi = mallinfo();
    return mi.uordblks;
}

void deallocateMemoryWoId(void* ptr) {
    deallocateMemory(customId, ptr);
}

void deallocateMemory(const FwEnumStoreType identifier, void* ptr) {
    FW_ASSERT(pAllocator != nullptr);
    if (pAllocator == nullptr) {
        ::free(ptr);
    } else {
        pAllocator->deallocate(identifier, ptr);
    }
}

void* allocateMemoryWoId(const FwSizeType size) {
    return allocateMemory(customId, size);
}

void* allocateMemory(const FwEnumStoreType identifier, const FwSizeType size) {
    void* ptr;
    if (pAllocator == nullptr) {
        ptr = ::malloc(size);
    } else {
        FwSizeType cSize = size;
        bool recoverable = false;
        ptr = pAllocator->allocate(identifier, cSize, recoverable);
        if (cSize != size) {
            ptr = nullptr;
        }
    }
    return ptr;
}

}  // namespace OverrideNewDelete
}  // namespace Baremetal
}  // namespace Os

void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWoId(size);
}
void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWoId(size);
}
// Global operator new
void* operator new(std::size_t size) {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWoId(size);
}
// Global operator new w/ identifier
void* operator new(std::size_t size, const FwEnumStoreType identifier) {
    return Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
}
// Global override for operator new[]
void* operator new[](std::size_t size) {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWoId(size);
}

// Global override for operator new[] w/ identifier
void* operator new[](std::size_t size, const FwEnumStoreType identifier) {
    return Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
}

// Global operator delete
void operator delete(void* ptr) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWoId(ptr);
}

// Global operator delete w/ identifier
void operator delete(void* ptr, const FwEnumStoreType identifier) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemory(identifier, ptr);
}

// Global operator delete[]
void operator delete[](void* ptr) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWoId(ptr);
}

// Global operator delete[] w/ identifier
void operator delete[](void* ptr, const FwEnumStoreType identifier) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemory(identifier, ptr);
}
void operator delete(void* ptr, std::size_t size) {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWoId(ptr);
}
void operator delete[](void* ptr, std::size_t size) {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWoId(ptr);
}
