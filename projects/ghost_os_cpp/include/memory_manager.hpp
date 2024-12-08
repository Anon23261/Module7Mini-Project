#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include "os_toolkit.hpp"

namespace ghost {
namespace os {

struct MemoryRegion {
    uintptr_t start;
    size_t size;
    bool is_protected;
    std::string description;
};

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    // Memory protection
    bool protectMemoryRegion(uintptr_t address, size_t size, bool protect);
    bool isMemoryProtected(uintptr_t address) const;
    
    // Memory allocation
    void* allocateMemory(size_t size, bool protected_memory = false);
    bool freeMemory(void* ptr);
    
    // Memory information
    std::vector<MemoryRegion> getMemoryMap() const;
    size_t getTotalMemory() const;
    size_t getAvailableMemory() const;
    
    // Memory statistics
    struct MemoryStats {
        size_t total_allocated;
        size_t total_freed;
        size_t current_usage;
        size_t peak_usage;
        size_t allocation_count;
        size_t protected_regions;
    };
    
    MemoryStats getMemoryStats() const;
    
    // Memory operations
    bool copyMemory(void* dest, const void* src, size_t size);
    bool zeroMemory(void* ptr, size_t size);
    bool compareMemory(const void* ptr1, const void* ptr2, size_t size) const;

private:
    struct MemoryBlock {
        void* ptr;
        size_t size;
        bool is_protected;
    };

    std::map<void*, MemoryBlock> allocated_blocks_;
    std::vector<MemoryRegion> protected_regions_;
    MemoryStats stats_;
    mutable std::mutex mutex_;

    bool isAddressInProtectedRegion(uintptr_t address) const;
    void updateStats(size_t size, bool allocating);
};

} // namespace os
} // namespace ghost
