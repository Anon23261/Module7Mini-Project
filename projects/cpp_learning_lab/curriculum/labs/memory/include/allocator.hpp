#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <new>
#include <atomic>

namespace ghost::memory {

// Memory alignment constants
constexpr size_t MIN_ALIGNMENT = alignof(std::max_align_t);
constexpr size_t CACHE_LINE_SIZE = 64;
constexpr size_t PAGE_SIZE = 4096;

// Memory allocation flags
enum class AllocFlags {
    NONE = 0,
    ZERO = 1 << 0,        // Zero memory after allocation
    ALIGNED = 1 << 1,     // Align to cache line
    NUMA_LOCAL = 1 << 2,  // Allocate from local NUMA node
    NO_THROW = 1 << 3     // Don't throw on failure
};

inline AllocFlags operator|(AllocFlags a, AllocFlags b) {
    return static_cast<AllocFlags>(
        static_cast<std::underlying_type_t<AllocFlags>>(a) |
        static_cast<std::underlying_type_t<AllocFlags>>(b)
    );
}

// Memory statistics
struct MemoryStats {
    std::atomic<size_t> allocated_bytes{0};
    std::atomic<size_t> freed_bytes{0};
    std::atomic<size_t> allocation_count{0};
    std::atomic<size_t> deallocation_count{0};
    std::atomic<size_t> fragmentation_bytes{0};
    
    void reset() noexcept {
        allocated_bytes = 0;
        freed_bytes = 0;
        allocation_count = 0;
        deallocation_count = 0;
        fragmentation_bytes = 0;
    }
};

// Base allocator interface
class Allocator {
public:
    virtual ~Allocator() = default;

    // Core allocation functions
    virtual void* allocate(size_t size, AllocFlags flags = AllocFlags::NONE) = 0;
    virtual void deallocate(void* ptr) noexcept = 0;
    virtual bool owns(void* ptr) const noexcept = 0;

    // Memory management
    virtual void* reallocate(void* ptr, size_t new_size) {
        if (!ptr) return allocate(new_size);
        if (new_size == 0) {
            deallocate(ptr);
            return nullptr;
        }

        void* new_ptr = allocate(new_size);
        if (!new_ptr) return nullptr;

        // Copy old data
        size_t old_size = get_allocation_size(ptr);
        std::memcpy(new_ptr, ptr, std::min(old_size, new_size));
        deallocate(ptr);
        return new_ptr;
    }

    // Alignment support
    virtual void* aligned_allocate(size_t size, size_t alignment) {
        if (alignment <= MIN_ALIGNMENT)
            return allocate(size);

        // Allocate extra space for alignment and metadata
        size_t padded_size = size + alignment + sizeof(void*);
        char* raw = static_cast<char*>(allocate(padded_size));
        if (!raw) return nullptr;

        // Calculate aligned address
        char* aligned = reinterpret_cast<char*>(
            (reinterpret_cast<std::uintptr_t>(raw) + sizeof(void*) + alignment - 1) 
            & ~(alignment - 1)
        );

        // Store original pointer for deallocation
        *(reinterpret_cast<void**>(aligned - sizeof(void*))) = raw;
        return aligned;
    }

    virtual void aligned_deallocate(void* ptr) noexcept {
        if (!ptr) return;
        void* original = *(reinterpret_cast<void**>(
            static_cast<char*>(ptr) - sizeof(void*)
        ));
        deallocate(original);
    }

    // Statistics and debugging
    virtual MemoryStats get_stats() const noexcept = 0;
    virtual void reset_stats() noexcept = 0;
    virtual size_t get_allocation_size(void* ptr) const = 0;
    
    // Memory safety
    virtual bool validate_ptr(void* ptr) const noexcept = 0;
    virtual void check_corruption() const = 0;
    
    // NUMA support
    virtual int get_numa_node() const noexcept { return 0; }
    virtual void set_numa_node(int node) noexcept {}

protected:
    // Utility functions for derived classes
    static constexpr size_t align_up(size_t size, size_t alignment) noexcept {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    static constexpr size_t align_down(size_t size, size_t alignment) noexcept {
        return size & ~(alignment - 1);
    }

    static void* align_ptr(void* ptr, size_t alignment) noexcept {
        return reinterpret_cast<void*>(
            align_up(reinterpret_cast<std::uintptr_t>(ptr), alignment)
        );
    }

    // Memory pattern utilities for debugging
    static void fill_pattern(void* ptr, size_t size, uint8_t pattern) noexcept {
        std::memset(ptr, pattern, size);
    }

    static bool check_pattern(const void* ptr, size_t size, uint8_t pattern) noexcept {
        const uint8_t* bytes = static_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < size; ++i) {
            if (bytes[i] != pattern) return false;
        }
        return true;
    }
};

// Smart pointer for automatic memory management
template<typename T>
class AllocPtr {
public:
    AllocPtr(Allocator& alloc) : allocator_(&alloc), ptr_(nullptr) {}
    
    template<typename... Args>
    bool create(Args&&... args) {
        void* mem = allocator_->allocate(sizeof(T));
        if (!mem) return false;
        ptr_ = new(mem) T(std::forward<Args>(args)...);
        return true;
    }

    ~AllocPtr() {
        if (ptr_) {
            ptr_->~T();
            allocator_->deallocate(ptr_);
        }
    }

    T* get() const noexcept { return ptr_; }
    T* operator->() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }

private:
    Allocator* allocator_;
    T* ptr_;
};

} // namespace ghost::memory
