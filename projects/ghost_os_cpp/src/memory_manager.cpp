#include "memory_manager.hpp"
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace ghost {
namespace os {

MemoryManager::MemoryManager() {
    stats_ = {0, 0, 0, 0, 0, 0};
}

MemoryManager::~MemoryManager() {
    // Free all allocated memory
    for (const auto& block : allocated_blocks_) {
        free(block.second.ptr);
    }
}

bool MemoryManager::protectMemoryRegion(uintptr_t address, size_t size, bool protect) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if the region overlaps with any existing protected regions
    for (const auto& region : protected_regions_) {
        if (address < region.start + region.size && address + size > region.start) {
            return false; // Overlapping regions not allowed
        }
    }
    
    MemoryRegion new_region = {
        address,
        size,
        protect,
        "Protected memory region"
    };
    
    protected_regions_.push_back(new_region);
    if (protect) {
        stats_.protected_regions++;
    }
    
    return true;
}

bool MemoryManager::isMemoryProtected(uintptr_t address) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isAddressInProtectedRegion(address);
}

void* MemoryManager::allocateMemory(size_t size, bool protected_memory) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    void* ptr = malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    
    MemoryBlock block = {
        ptr,
        size,
        protected_memory
    };
    
    allocated_blocks_[ptr] = block;
    updateStats(size, true);
    
    if (protected_memory) {
        protectMemoryRegion(reinterpret_cast<uintptr_t>(ptr), size, true);
    }
    
    return ptr;
}

bool MemoryManager::freeMemory(void* ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = allocated_blocks_.find(ptr);
    if (it == allocated_blocks_.end()) {
        return false;
    }
    
    const MemoryBlock& block = it->second;
    if (block.is_protected) {
        protectMemoryRegion(reinterpret_cast<uintptr_t>(ptr), block.size, false);
    }
    
    free(ptr);
    updateStats(block.size, false);
    allocated_blocks_.erase(it);
    
    return true;
}

std::vector<MemoryRegion> MemoryManager::getMemoryMap() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return protected_regions_;
}

size_t MemoryManager::getTotalMemory() const {
    return stats_.total_allocated;
}

size_t MemoryManager::getAvailableMemory() const {
    return stats_.total_allocated - stats_.current_usage;
}

MemoryManager::MemoryStats MemoryManager::getMemoryStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool MemoryManager::copyMemory(void* dest, const void* src, size_t size) {
    if (isMemoryProtected(reinterpret_cast<uintptr_t>(dest)) ||
        isMemoryProtected(reinterpret_cast<uintptr_t>(src))) {
        return false;
    }
    
    memcpy(dest, src, size);
    return true;
}

bool MemoryManager::zeroMemory(void* ptr, size_t size) {
    if (isMemoryProtected(reinterpret_cast<uintptr_t>(ptr))) {
        return false;
    }
    
    memset(ptr, 0, size);
    return true;
}

bool MemoryManager::compareMemory(const void* ptr1, const void* ptr2, size_t size) const {
    if (isMemoryProtected(reinterpret_cast<uintptr_t>(ptr1)) ||
        isMemoryProtected(reinterpret_cast<uintptr_t>(ptr2))) {
        return false;
    }
    
    return memcmp(ptr1, ptr2, size) == 0;
}

bool MemoryManager::isAddressInProtectedRegion(uintptr_t address) const {
    return std::any_of(protected_regions_.begin(), protected_regions_.end(),
        [address](const MemoryRegion& region) {
            return address >= region.start && address < region.start + region.size;
        });
}

void MemoryManager::updateStats(size_t size, bool allocating) {
    if (allocating) {
        stats_.total_allocated += size;
        stats_.current_usage += size;
        stats_.allocation_count++;
        stats_.peak_usage = std::max(stats_.peak_usage, stats_.current_usage);
    } else {
        stats_.total_freed += size;
        stats_.current_usage -= size;
    }
}

} // namespace os
} // namespace ghost
