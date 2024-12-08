#pragma once

#include <cstddef>
#include <new>
#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <type_traits>

namespace ghost {

// Forward declarations
class MemoryPool;
class MemoryStats;

/**
 * @brief Custom allocator with multiple allocation strategies
 * 
 * This allocator implements various memory allocation strategies:
 * - First-fit
 * - Best-fit
 * - Segregated storage
 * - Buddy system
 * 
 * Features:
 * - Memory coalescing
 * - Boundary tags
 * - Memory tracking
 * - Thread safety (optional)
 */
template<typename T, size_t PoolSize = 1048576> // 1MB default pool size
class CustomAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = CustomAllocator<U, PoolSize>;
    };

    // Strategy selection
    enum class Strategy {
        FirstFit,
        BestFit,
        Segregated,
        Buddy
    };

    CustomAllocator() noexcept : strategy_(Strategy::BestFit) {
        initializePool();
    }

    explicit CustomAllocator(Strategy s) noexcept : strategy_(s) {
        initializePool();
    }

    template<typename U>
    CustomAllocator(const CustomAllocator<U, PoolSize>&) noexcept {}

    ~CustomAllocator() {
        cleanup();
    }

    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }

        const size_type bytes = n * sizeof(T);
        void* ptr = nullptr;

        switch (strategy_) {
            case Strategy::FirstFit:
                ptr = firstFitAllocate(bytes);
                break;
            case Strategy::BestFit:
                ptr = bestFitAllocate(bytes);
                break;
            case Strategy::Segregated:
                ptr = segregatedAllocate(bytes);
                break;
            case Strategy::Buddy:
                ptr = buddyAllocate(bytes);
                break;
        }

        if (!ptr) {
            throw std::bad_alloc();
        }

        updateStats(bytes, true);
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type n) noexcept {
        if (!p) return;

        const size_type bytes = n * sizeof(T);
        switch (strategy_) {
            case Strategy::FirstFit:
            case Strategy::BestFit:
                standardDeallocate(p, bytes);
                break;
            case Strategy::Segregated:
                segregatedDeallocate(p, bytes);
                break;
            case Strategy::Buddy:
                buddyDeallocate(p, bytes);
                break;
        }

        updateStats(bytes, false);
    }

    size_type max_size() const noexcept {
        return PoolSize / sizeof(T);
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    // Memory statistics
    MemoryStats getStats() const noexcept;

    // Debug utilities
    void dumpMemoryMap() const;
    void validateHeap() const;

private:
    Strategy strategy_;
    std::unique_ptr<MemoryPool> pool_;
    MemoryStats stats_;

    struct BlockHeader {
        size_type size;
        bool used;
        BlockHeader* prev;
        BlockHeader* next;
    };

    void initializePool();
    void cleanup();

    void* firstFitAllocate(size_type bytes);
    void* bestFitAllocate(size_type bytes);
    void* segregatedAllocate(size_type bytes);
    void* buddyAllocate(size_type bytes);

    void standardDeallocate(pointer p, size_type bytes);
    void segregatedDeallocate(pointer p, size_type bytes);
    void buddyDeallocate(pointer p, size_type bytes);

    void coalesce(BlockHeader* block);
    void splitBlock(BlockHeader* block, size_type size);
    void updateStats(size_type bytes, bool isAllocation);

    // Utility functions
    static constexpr size_type alignUp(size_type n) {
        return (n + sizeof(std::max_align_t) - 1) & ~(sizeof(std::max_align_t) - 1);
    }

    static constexpr size_type headerSize() {
        return alignUp(sizeof(BlockHeader));
    }
};

// Implementation of MemoryPool
class MemoryPool {
    // Implementation details...
};

// Implementation of MemoryStats
class MemoryStats {
public:
    size_t totalAllocations{0};
    size_t currentlyAllocated{0};
    size_t peakAllocation{0};
    size_t totalFragmentation{0};
    
    void clear() {
        totalAllocations = 0;
        currentlyAllocated = 0;
        peakAllocation = 0;
        totalFragmentation = 0;
    }
};

} // namespace ghost

// Required operator== for allocator comparisons
template<typename T, size_t P1, typename U, size_t P2>
bool operator==(const ghost::CustomAllocator<T, P1>&, 
                const ghost::CustomAllocator<U, P2>&) noexcept {
    return P1 == P2;
}

template<typename T, size_t P1, typename U, size_t P2>
bool operator!=(const ghost::CustomAllocator<T, P1>& a, 
                const ghost::CustomAllocator<U, P2>& b) noexcept {
    return !(a == b);
}
