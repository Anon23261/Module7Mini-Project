#pragma once

#include "allocator.hpp"
#include <vector>
#include <mutex>
#include <cassert>
#include <cstring>

namespace ghost::memory {

/**
 * @brief A Pool Allocator that manages fixed-size blocks of memory.
 * 
 * The Pool Allocator is optimized for scenarios where you need to frequently
 * allocate and deallocate objects of the same size. It pre-allocates a pool
 * of fixed-size blocks and manages them using a free list approach.
 */
class PoolAllocator : public Allocator {
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 1024;
    static constexpr size_t MIN_BLOCK_SIZE = 8;

    /**
     * @brief Constructs a Pool Allocator
     * @param block_size Size of each block in the pool
     * @param initial_pool_size Initial number of blocks to allocate
     */
    explicit PoolAllocator(size_t block_size, size_t initial_pool_size = DEFAULT_POOL_SIZE) 
        : block_size_(align_up(std::max(block_size, MIN_BLOCK_SIZE), MIN_ALIGNMENT))
        , pool_size_(initial_pool_size)
        , free_blocks_(nullptr)
    {
        // Allocate initial pool
        expand_pool();
    }

    ~PoolAllocator() {
        // Free all pools
        for (void* pool : pools_) {
            ::operator delete(pool);
        }
    }

    void* allocate(size_t size, AllocFlags flags = AllocFlags::NONE) override {
        if (size > block_size_) return nullptr;

        std::lock_guard<std::mutex> lock(mutex_);

        // Expand pool if needed
        if (!free_blocks_) {
            if (!expand_pool()) return nullptr;
        }

        // Get block from free list
        void* block = free_blocks_;
        free_blocks_ = *static_cast<void**>(free_blocks_);

        // Update statistics
        stats_.allocated_bytes += block_size_;
        stats_.allocation_count++;

        // Handle allocation flags
        if ((flags & AllocFlags::ZERO) == AllocFlags::ZERO) {
            std::memset(block, 0, block_size_);
        }

        return block;
    }

    void deallocate(void* ptr) noexcept override {
        if (!ptr) return;

        std::lock_guard<std::mutex> lock(mutex_);

        // Verify the pointer belongs to one of our pools
        if (!owns(ptr)) return;

        // Add block to free list
        *static_cast<void**>(ptr) = free_blocks_;
        free_blocks_ = ptr;

        // Update statistics
        stats_.freed_bytes += block_size_;
        stats_.deallocation_count++;
    }

    bool owns(void* ptr) const noexcept override {
        if (!ptr) return false;

        // Check if ptr is within any of our pools
        for (void* pool : pools_) {
            uint8_t* pool_start = static_cast<uint8_t*>(pool);
            uint8_t* pool_end = pool_start + (pool_size_ * block_size_);

            if (ptr >= pool && ptr < pool_end) {
                // Verify alignment
                size_t offset = static_cast<uint8_t*>(ptr) - pool_start;
                return (offset % block_size_) == 0;
            }
        }

        return false;
    }

    MemoryStats get_stats() const noexcept override {
        return stats_;
    }

    void reset_stats() noexcept override {
        stats_.reset();
    }

    size_t get_allocation_size(void* ptr) const override {
        return owns(ptr) ? block_size_ : 0;
    }

    bool validate_ptr(void* ptr) const noexcept override {
        return owns(ptr);
    }

    void check_corruption() const override {
        std::lock_guard<std::mutex> lock(mutex_);

        // Count free blocks
        size_t free_count = 0;
        void* current = free_blocks_;

        while (current) {
            // Verify current block is within a pool
            assert(owns(current));

            // Move to next block
            current = *static_cast<void**>(current);
            free_count++;

            // Detect cycles in free list
            assert(free_count <= pools_.size() * pool_size_);
        }
    }

    /**
     * @brief Gets the block size used by this allocator
     * @return The size of each block in bytes
     */
    size_t get_block_size() const noexcept {
        return block_size_;
    }

    /**
     * @brief Gets the current number of pools
     * @return Number of allocated pools
     */
    size_t get_pool_count() const noexcept {
        return pools_.size();
    }

    /**
     * @brief Gets the total capacity in bytes
     * @return Total bytes managed by this allocator
     */
    size_t get_total_capacity() const noexcept {
        return pools_.size() * pool_size_ * block_size_;
    }

    /**
     * @brief Gets the number of free blocks in the allocator
     * @return Number of free blocks
     */
    size_t get_num_free_blocks() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t free_blocks = 0;
        void* current = free_blocks_;
        while (current) {
            free_blocks++;
            current = *static_cast<void**>(current);
        }
        return free_blocks;
    }

private:
    /**
     * @brief Expands the pool by allocating a new chunk of memory
     * @return true if expansion successful, false otherwise
     */
    bool expand_pool() {
        // Allocate new pool
        void* pool = ::operator new(pool_size_ * block_size_, std::nothrow);
        if (!pool) return false;

        // Add pool to list
        pools_.push_back(pool);

        // Initialize free list
        uint8_t* blocks = static_cast<uint8_t*>(pool);
        for (size_t i = 0; i < pool_size_ - 1; ++i) {
            void* current = blocks + (i * block_size_);
            void* next = blocks + ((i + 1) * block_size_);
            *static_cast<void**>(current) = next;
        }

        // Set last block's next pointer to current free list
        void* last = blocks + ((pool_size_ - 1) * block_size_);
        *static_cast<void**>(last) = free_blocks_;

        // Update free list head
        free_blocks_ = pool;

        return true;
    }

    size_t block_size_;             // Size of each block
    size_t pool_size_;              // Number of blocks per pool
    void* free_blocks_;             // Head of free list
    std::vector<void*> pools_;      // List of allocated pools
    mutable std::mutex mutex_;      // Mutex for thread safety
    MemoryStats stats_;             // Memory statistics
};

} // namespace ghost::memory
