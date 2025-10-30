// ======================================================================
// \title fprime-baremetal/Os/Baremetal/OverrideNewDelete.cpp
// \brief Implementation for new/delete overrides
// =====================================================================
#include "OverrideNewDelete.hpp"
#include <malloc.h>
#include <stdio.h>
#include <Fw/Types/Assert.hpp>
#include <atomic>
#include <new>        // included to get std::nothrow_t
#include <stdexcept>  // For standard exception types

namespace Os {
namespace Baremetal {
namespace OverrideNewDelete {

// Determine whether it's possible to throw exceptions
// Whether __cpp_exceptions is undefined, 0, or other varies
// by compiler, so set to the year the macro was instroduced
#if defined(__cpp_exceptions) && __cpp_exceptions == 199711
#define ENABLE_EXCEPTIONS
#endif

// global variables
//! Modifiable default (useful before calling code with new/delete to attribute the memory user)
FwEnumStoreType customId = DEFAULT_ID;
//! Pointer to MemAllocator
static Fw::MemAllocator* pAllocator = nullptr;

// Prototypes
void deallocateMemoryWithoutId(void* ptr);
void deallocateMemory(const FwEnumStoreType identifier, void* ptr);
void* allocateMemoryWithoutId(const FwSizeType size);
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

void deallocateMemoryWithoutId(void* ptr) {
    deallocateMemory(customId, ptr);
}

void deallocateMemory(const FwEnumStoreType identifier, void* ptr) {
    if (pAllocator == nullptr) {
        ::free(ptr);
    } else {
        pAllocator->deallocate(identifier, ptr);
    }
}

void* allocateMemoryWithoutId(const FwSizeType size) {
    return allocateMemory(customId, size);
}

void* allocateMemory(const FwEnumStoreType identifier, const FwSizeType size) {
    void* ptr;
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

void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
}
void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    return Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
}
// Global operator new
void* operator new(std::size_t size) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
    if (mem == nullptr) {
#ifdef ENABLE_EXCEPTIONS
        throw std::bad_alloc();
#else
        FW_ASSERT(false);
#endif
    }
    return mem;
}
// Global operator new w/ identifier
void* operator new(std::size_t size, const FwEnumStoreType identifier) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
    if (mem == nullptr) {
#ifdef ENABLE_EXCEPTIONS
        throw std::bad_alloc();
#else
        FW_ASSERT(false);
#endif
    }
    return mem;
}
// Global override for operator new[]
void* operator new[](std::size_t size) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemoryWithoutId(size);
    if (mem == nullptr) {
#ifdef ENABLE_EXCEPTIONS
        throw std::bad_alloc();
#else
        FW_ASSERT(false);
#endif
    }
    return mem;
}

// Global override for operator new[] w/ identifier
void* operator new[](std::size_t size, const FwEnumStoreType identifier) {
    void* mem = Os::Baremetal::OverrideNewDelete::allocateMemory(identifier, size);
    if (mem == nullptr) {
#ifdef ENABLE_EXCEPTIONS
        throw std::bad_alloc();
#else
        FW_ASSERT(false);
#endif
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
