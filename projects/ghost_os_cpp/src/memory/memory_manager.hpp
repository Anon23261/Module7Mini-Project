#pragma once

#include <cstdint>
#include <array>
#include <bitset>
#include <optional>

namespace ghost::memory {

// Constants for memory management
constexpr size_t PAGE_SIZE = 4096;  // 4KB pages
constexpr size_t MAX_PAGES = 1024;  // Support up to 4MB initially

class MemoryManager {
public:
    // Initialize memory manager
    static MemoryManager& instance() {
        static MemoryManager instance;
        return instance;
    }

    // Allocate a page of memory
    std::optional<void*> allocate_page() {
        for (size_t i = 0; i < MAX_PAGES; ++i) {
            if (!page_bitmap[i]) {
                page_bitmap[i] = true;
                return reinterpret_cast<void*>(PAGE_SIZE * i);
            }
        }
        return std::nullopt;
    }

    // Free a page of memory
    bool free_page(void* ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        size_t page_index = addr / PAGE_SIZE;
        
        if (page_index >= MAX_PAGES || !page_bitmap[page_index]) {
            return false;
        }

        page_bitmap[page_index] = false;
        return true;
    }

    // Get total memory usage
    size_t get_used_pages() const {
        return page_bitmap.count();
    }

    // Get total available memory
    size_t get_free_pages() const {
        return MAX_PAGES - get_used_pages();
    }

private:
    MemoryManager() = default;
    ~MemoryManager() = default;

    // Delete copy constructor and assignment operator
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    // Bitmap to track page allocation status
    std::bitset<MAX_PAGES> page_bitmap;
};

} // namespace ghost::memory
