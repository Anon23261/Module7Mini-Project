#pragma once

#include "allocator.hpp"
#include <vector>
#include <bitset>
#include <cassert>
#include <cmath>

namespace ghost::memory {

class BuddyAllocator : public Allocator {
public:
    // Constants for buddy system
    static constexpr size_t MIN_BLOCK_SIZE = 16;
    static constexpr size_t MAX_ORDER = 20;  // Up to 1MB blocks
    static constexpr size_t MAX_BLOCK_SIZE = MIN_BLOCK_SIZE << MAX_ORDER;

    explicit BuddyAllocator(size_t total_size = MAX_BLOCK_SIZE * 1024) {
        // Round up total size to power of 2
        total_size = next_power_of_2(total_size);
        total_size = std::min(total_size, MAX_BLOCK_SIZE * 1024);

        // Calculate number of blocks needed
        size_t num_blocks = total_size / MIN_BLOCK_SIZE;
        blocks_.resize(num_blocks);
        
        // Initialize free lists
        free_lists_.resize(MAX_ORDER + 1);
        
        // Add initial block to largest free list
        size_t initial_order = log2(total_size) - log2(MIN_BLOCK_SIZE);
        free_lists_[initial_order].push_back(0);
        
        // Initialize block metadata
        blocks_[0].order = initial_order;
        blocks_[0].is_free = true;
    }

    void* allocate(size_t size, AllocFlags flags = AllocFlags::NONE) override {
        // Calculate required order
        size = std::max(size, MIN_BLOCK_SIZE);
        size = next_power_of_2(size);
        size_t order = log2(size) - log2(MIN_BLOCK_SIZE);
        
        if (order > MAX_ORDER) return nullptr;

        // Find suitable block
        size_t block_index = find_free_block(order);
        if (block_index == -1) return nullptr;

        // Mark block as allocated
        blocks_[block_index].is_free = false;
        
        // Update statistics
        stats_.allocated_bytes += size;
        stats_.allocation_count++;

        void* ptr = get_block_address(block_index);
        
        // Handle allocation flags
        if ((flags & AllocFlags::ZERO) == AllocFlags::ZERO) {
            std::memset(ptr, 0, size);
        }
        
        return ptr;
    }

    void deallocate(void* ptr) noexcept override {
        if (!ptr) return;

        // Find block index
        size_t block_index = get_block_index(ptr);
        if (block_index >= blocks_.size()) return;

        Block& block = blocks_[block_index];
        if (block.is_free) return;

        // Update statistics
        size_t size = MIN_BLOCK_SIZE << block.order;
        stats_.freed_bytes += size;
        stats_.deallocation_count++;

        // Mark block as free
        block.is_free = true;

        // Merge with buddy if possible
        merge_buddies(block_index);
    }

    bool owns(void* ptr) const noexcept override {
        if (!ptr) return false;
        size_t block_index = get_block_index(ptr);
        return block_index < blocks_.size();
    }

    MemoryStats get_stats() const noexcept override {
        return stats_;
    }

    void reset_stats() noexcept override {
        stats_.reset();
    }

    size_t get_allocation_size(void* ptr) const override {
        if (!ptr) return 0;
        size_t block_index = get_block_index(ptr);
        if (block_index >= blocks_.size()) return 0;
        return MIN_BLOCK_SIZE << blocks_[block_index].order;
    }

    bool validate_ptr(void* ptr) const noexcept override {
        if (!ptr) return false;
        size_t block_index = get_block_index(ptr);
        if (block_index >= blocks_.size()) return false;
        return !blocks_[block_index].is_free;
    }

    void check_corruption() const override {
        // Verify block consistency
        for (size_t i = 0; i < blocks_.size(); ++i) {
            const Block& block = blocks_[i];
            if (!block.is_free) {
                // Check block boundaries
                void* ptr = get_block_address(i);
                size_t size = MIN_BLOCK_SIZE << block.order;
                
                // Verify block metadata
                assert(block.order <= MAX_ORDER);
                
                // Check for buffer overruns
                uint8_t* guard = static_cast<uint8_t*>(ptr) + size;
                for (size_t j = 0; j < sizeof(size_t); ++j) {
                    assert(guard[j] == 0xFD);  // Guard pattern
                }
            }
        }
    }

private:
    struct Block {
        size_t order;    // Block size = MIN_BLOCK_SIZE << order
        bool is_free;
    };

    std::vector<Block> blocks_;
    std::vector<std::vector<size_t>> free_lists_;  // Free lists for each order
    MemoryStats stats_;

    // Utility functions
    static size_t next_power_of_2(size_t x) {
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x |= x >> 32;
        x++;
        return x;
    }

    static size_t log2(size_t x) {
        return sizeof(size_t) * 8 - 1 - __builtin_clzll(x);
    }

    void* get_block_address(size_t block_index) const {
        return reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(this) + 
            block_index * MIN_BLOCK_SIZE
        );
    }

    size_t get_block_index(void* ptr) const {
        return (reinterpret_cast<uintptr_t>(ptr) - 
                reinterpret_cast<uintptr_t>(this)) / MIN_BLOCK_SIZE;
    }

    size_t find_free_block(size_t required_order) {
        // Look for block of required size or larger
        for (size_t order = required_order; order <= MAX_ORDER; ++order) {
            if (!free_lists_[order].empty()) {
                size_t block_index = free_lists_[order].back();
                free_lists_[order].pop_back();

                // Split block if necessary
                while (order > required_order) {
                    order--;
                    size_t buddy_index = block_index + (MIN_BLOCK_SIZE << order);
                    blocks_[buddy_index].order = order;
                    blocks_[buddy_index].is_free = true;
                    free_lists_[order].push_back(buddy_index);
                }

                return block_index;
            }
        }
        return -1;
    }

    void merge_buddies(size_t block_index) {
        Block& block = blocks_[block_index];
        size_t order = block.order;

        while (order < MAX_ORDER) {
            // Calculate buddy index
            size_t buddy_index = block_index ^ (1 << order);
            Block& buddy = blocks_[buddy_index];

            // Check if buddy is free and of same order
            if (!buddy.is_free || buddy.order != order) break;

            // Remove buddy from free list
            auto& free_list = free_lists_[order];
            free_list.erase(
                std::remove(free_list.begin(), free_list.end(), buddy_index),
                free_list.end()
            );

            // Merge blocks
            size_t merged_index = std::min(block_index, buddy_index);
            blocks_[merged_index].order = order + 1;
            block_index = merged_index;
            block = blocks_[merged_index];
            order++;
        }

        // Add merged block to free list
        free_lists_[block.order].push_back(block_index);
    }
};

} // namespace ghost::memory
