// ======================================================================
// \title fprime-baremetal/Os/Baremetal/OverrideNewDelete.cpp
// \brief Implementation for new/delete overrides
// =====================================================================
#include "OverrideNewDelete.hpp"
#include <malloc.h>
#include <stdio.h>
#include <Fw/Types/Assert.hpp>
#include <atomic>
#include <fprime-baremetal/Os/MemoryIdScope/MemoryIdScope.hpp>
#include <new>        // included to get std::nothrow_t
#include <stdexcept>  // For standard exception types

namespace Os {
namespace Baremetal {
namespace OverrideNewDelete {

//! Pointer to MemAllocator
static Fw::MemAllocator* pAllocator = nullptr;

// Prototypes
static void deallocateMemoryWithoutId(void* ptr);
static void deallocateMemory(const FwEnumStoreType identifier, void* ptr);
static void* allocateMemoryWithoutId(const FwSizeType size);
static void* allocateMemory(const FwEnumStoreType identifier, const FwSizeType size);

// Function implementations
FwSizeType registerMemAllocator(Fw::MemAllocator* allocator) {
    FW_ASSERT(pAllocator == nullptr);
    FW_ASSERT(allocator != nullptr);
    pAllocator = allocator;
    // Get & return number of bytes already allocated
    struct mallinfo mi = mallinfo();
    return mi.uordblks;
}

static void deallocateMemoryWithoutId(void* ptr) {
    deallocateMemory(Os::Baremetal::defaultMemoryId, ptr);
}

static void deallocateMemory(const FwEnumStoreType identifier, void* ptr) {
    if (pAllocator == nullptr) {
        ::free(ptr);
    } else {
        pAllocator->deallocate(identifier, ptr);
    }
}

static void* allocateMemoryWithoutId(const FwSizeType size) {
    return allocateMemory(Os::Baremetal::defaultMemoryId, size);
}

static void* allocateMemory(const FwEnumStoreType identifier, const FwSizeType size) {
    void* ptr = nullptr;
#ifndef BUILD_UT
    FW_ASSERT(pAllocator != nullptr);
#endif
    if (pAllocator == nullptr) {
        ptr = ::malloc(size);
    } else {
        FwSizeType cSize = size;
        bool recoverable = false;
        ptr = pAllocator->allocate(identifier, cSize, recoverable);
        if (cSize != size) {
            // Most MemAllocator implementations free the memory if
            // if allocation fails, but handle the case where that's
            // not the case
            if (ptr != nullptr) {
                pAllocator->deallocate(identifier, ptr);
                ptr = nullptr;
            }
        }
    }
    return ptr;
}

}  // namespace OverrideNewDelete
}  // namespace Baremetal
}  // namespace Os

// Global operator new[] w/ nothrow
void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
}
// Global operator new w/ nothrow
void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
}
// Global operator new
// NOTE: Exceptions are not thrown (instead software asserts) if the allocation
// fails, because raising exceptions is against the coding standard used by fprime
void* operator new(std::size_t size) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
    if (mem == nullptr) {
        FW_ASSERT(false);
    }
    return mem;
}
// Global operator new w/ identifier
void* operator new(std::size_t size, const FwEnumStoreType identifier) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
    if (mem == nullptr) {
        FW_ASSERT(false);
    }
    return mem;
}
// Global override for operator new[]
void* operator new[](std::size_t size) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
    if (mem == nullptr) {
        FW_ASSERT(false);
    }
    return mem;
}

// Global override for operator new[] w/ identifier
void* operator new[](std::size_t size, const FwEnumStoreType identifier) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
    if (mem == nullptr) {
        FW_ASSERT(false);
    }
    return mem;
}

// Global operator delete
void operator delete(void* ptr) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWithoutId(ptr);
}

// Global operator delete w/ identifier
void operator delete(void* ptr, const FwEnumStoreType identifier) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemory(identifier, ptr);
}

// Global operator delete[]
void operator delete[](void* ptr) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWithoutId(ptr);
}

// Global operator delete[] w/ identifier
void operator delete[](void* ptr, const FwEnumStoreType identifier) noexcept {
    Os::Baremetal::OverrideNewDelete::deallocateMemory(identifier, ptr);
}
void operator delete(void* ptr, std::size_t size) {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWithoutId(ptr);
}
void operator delete[](void* ptr, std::size_t size) {
    Os::Baremetal::OverrideNewDelete::deallocateMemoryWithoutId(ptr);
}
