#pragma once

#include "allocator.hpp"
#include <vector>
#include <unordered_map>
#include <mutex>

namespace ghost::memory {

class SlabAllocator : public Allocator {
public:
    // Constants for slab allocation
    static constexpr size_t MIN_SLAB_SIZE = 4096;  // 4KB slabs
    static constexpr size_t MAX_OBJECTS_PER_SLAB = 8192;
    static constexpr size_t MAX_SLABS = 1024;

    explicit SlabAllocator(size_t object_size) 
        : object_size_(align_up(object_size, MIN_ALIGNMENT))
    {
        // Calculate objects per slab
        objects_per_slab_ = std::min(
            MAX_OBJECTS_PER_SLAB,
            (MIN_SLAB_SIZE - sizeof(SlabHeader)) / object_size_
        );

        // Initialize free list head
        free_slabs_ = nullptr;
        
        // Create initial slab
        create_new_slab();
    }

    ~SlabAllocator() {
        // Free all slabs
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto* slab : all_slabs_) {
            ::operator delete(slab);
        }
    }

    void* allocate(size_t size, AllocFlags flags = AllocFlags::NONE) override {
        if (size > object_size_) return nullptr;

        std::lock_guard<std::mutex> lock(mutex_);

        // Find slab with free objects
        SlabHeader* slab = find_slab_with_free_object();
        if (!slab) {
            // Create new slab if needed
            slab = create_new_slab();
            if (!slab) return nullptr;
        }

        // Allocate from slab
        void* ptr = allocate_from_slab(slab);
        if (!ptr) return nullptr;

        // Update statistics
        stats_.allocated_bytes += object_size_;
        stats_.allocation_count++;

        // Handle allocation flags
        if ((flags & AllocFlags::ZERO) == AllocFlags::ZERO) {
            std::memset(ptr, 0, object_size_);
        }

        return ptr;
    }

    void deallocate(void* ptr) noexcept override {
        if (!ptr) return;

        std::lock_guard<std::mutex> lock(mutex_);

        // Find slab containing ptr
        SlabHeader* slab = find_slab_for_ptr(ptr);
        if (!slab) return;

        // Return object to slab
        deallocate_from_slab(slab, ptr);

        // Update statistics
        stats_.freed_bytes += object_size_;
        stats_.deallocation_count++;

        // If slab is empty and we have too many slabs, free it
        if (slab->free_count == objects_per_slab_ && 
            all_slabs_.size() > 1) {
            free_slab(slab);
        }
    }

    bool owns(void* ptr) const noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        return find_slab_for_ptr(ptr) != nullptr;
    }

    MemoryStats get_stats() const noexcept override {
        return stats_;
    }

    void reset_stats() noexcept override {
        stats_.reset();
    }

    size_t get_allocation_size(void* ptr) const override {
        if (!ptr) return 0;
        return object_size_;
    }

    bool validate_ptr(void* ptr) const noexcept override {
        std::lock_guard<std::mutex> lock(mutex_);
        SlabHeader* slab = find_slab_for_ptr(ptr);
        if (!slab) return false;

        // Check if ptr is properly aligned
        size_t offset = static_cast<uint8_t*>(ptr) - 
                       static_cast<uint8_t*>(slab + 1);
        return (offset % object_size_) == 0;
    }

    void check_corruption() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto* slab : all_slabs_) {
            // Verify slab metadata
            assert(slab->magic == SLAB_MAGIC);
            assert(slab->free_count <= objects_per_slab_);

            // Check free list integrity
            size_t free_count = 0;
            void* ptr = slab->free_list;
            while (ptr) {
                // Verify ptr is within slab bounds
                assert(ptr >= slab + 1);
                assert(ptr < static_cast<uint8_t*>(slab + 1) + 
                             objects_per_slab_ * object_size_);
                
                ptr = *static_cast<void**>(ptr);
                free_count++;
            }
            assert(free_count == slab->free_count);
        }
    }

private:
    static constexpr uint32_t SLAB_MAGIC = 0xDEADBEEF;

    struct SlabHeader {
        uint32_t magic;          // Magic number for validation
        size_t free_count;       // Number of free objects
        void* free_list;         // List of free objects
        SlabHeader* next;        // Next slab in free list
        SlabHeader* prev;        // Previous slab in free list
    };

    size_t object_size_;         // Size of each object
    size_t objects_per_slab_;    // Number of objects per slab
    SlabHeader* free_slabs_;     // List of slabs with free objects
    std::vector<SlabHeader*> all_slabs_;  // All allocated slabs
    mutable std::mutex mutex_;   // Mutex for thread safety
    MemoryStats stats_;          // Memory statistics

    SlabHeader* create_new_slab() {
        if (all_slabs_.size() >= MAX_SLABS) return nullptr;

        // Allocate slab memory
        size_t slab_size = sizeof(SlabHeader) + 
                          objects_per_slab_ * object_size_;
        void* memory = ::operator new(slab_size, std::nothrow);
        if (!memory) return nullptr;

        // Initialize slab header
        SlabHeader* slab = static_cast<SlabHeader*>(memory);
        slab->magic = SLAB_MAGIC;
        slab->free_count = objects_per_slab_;
        slab->next = nullptr;
        slab->prev = nullptr;

        // Initialize free list
        uint8_t* objects = reinterpret_cast<uint8_t*>(slab + 1);
        slab->free_list = objects;
        
        for (size_t i = 0; i < objects_per_slab_ - 1; ++i) {
            *reinterpret_cast<void**>(objects + i * object_size_) = 
                objects + (i + 1) * object_size_;
        }
        *reinterpret_cast<void**>(
            objects + (objects_per_slab_ - 1) * object_size_
        ) = nullptr;

        // Add to slab lists
        all_slabs_.push_back(slab);
        add_to_free_list(slab);

        return slab;
    }

    void free_slab(SlabHeader* slab) {
        // Remove from free list
        remove_from_free_list(slab);

        // Remove from all slabs list
        all_slabs_.erase(
            std::remove(all_slabs_.begin(), all_slabs_.end(), slab),
            all_slabs_.end()
        );

        // Free memory
        ::operator delete(slab);
    }

    void add_to_free_list(SlabHeader* slab) {
        slab->next = free_slabs_;
        slab->prev = nullptr;
        if (free_slabs_) free_slabs_->prev = slab;
        free_slabs_ = slab;
    }

    void remove_from_free_list(SlabHeader* slab) {
        if (slab->prev) slab->prev->next = slab->next;
        if (slab->next) slab->next->prev = slab->prev;
        if (free_slabs_ == slab) free_slabs_ = slab->next;
    }

    SlabHeader* find_slab_with_free_object() const {
        return free_slabs_;
    }

    void* allocate_from_slab(SlabHeader* slab) {
        if (!slab->free_count) return nullptr;

        // Get object from free list
        void* ptr = slab->free_list;
        slab->free_list = *static_cast<void**>(ptr);
        slab->free_count--;

        // Remove from free list if full
        if (slab->free_count == 0) {
            remove_from_free_list(slab);
        }

        return ptr;
    }

    void deallocate_from_slab(SlabHeader* slab, void* ptr) {
        // Add to free list
        *static_cast<void**>(ptr) = slab->free_list;
        slab->free_list = ptr;
        slab->free_count++;

        // Add to free slabs list if this was first free object
        if (slab->free_count == 1) {
            add_to_free_list(slab);
        }
    }

    SlabHeader* find_slab_for_ptr(void* ptr) const {
        // Binary search for slab containing ptr
        auto it = std::lower_bound(
            all_slabs_.begin(), 
            all_slabs_.end(), 
            ptr,
            [](SlabHeader* slab, void* ptr) {
                return static_cast<void*>(slab) < ptr;
            }
        );

        if (it == all_slabs_.begin()) return nullptr;
        --it;

        SlabHeader* slab = *it;
        if (ptr >= slab && 
            ptr < static_cast<uint8_t*>(slab + 1) + 
                  objects_per_slab_ * object_size_) {
            return slab;
        }

        return nullptr;
    }
};

} // namespace ghost::memory
