#pragma once

#include "../include/slab_allocator.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>

namespace ghost::memory::exercises {

// Exercise 1: Basic Allocation and Deallocation
void test_basic_allocation() {
    // Create a slab allocator for 64-byte objects
    SlabAllocator allocator(64);
    
    // Allocate an object
    void* ptr = allocator.allocate(64);
    assert(ptr != nullptr);
    
    // Verify allocation stats
    auto stats = allocator.get_stats();
    assert(stats.allocated_bytes == 64);
    assert(stats.allocation_count == 1);
    
    // Deallocate the object
    allocator.deallocate(ptr);
    
    // Verify deallocation stats
    stats = allocator.get_stats();
    assert(stats.freed_bytes == 64);
    assert(stats.deallocation_count == 1);
}

// Exercise 2: Multiple Allocations
void test_multiple_allocations() {
    SlabAllocator allocator(32);
    std::vector<void*> ptrs;
    
    // Allocate 100 objects
    for (int i = 0; i < 100; i++) {
        void* ptr = allocator.allocate(32);
        assert(ptr != nullptr);
        ptrs.push_back(ptr);
    }
    
    // Verify all allocations are unique
    for (size_t i = 0; i < ptrs.size(); i++) {
        for (size_t j = i + 1; j < ptrs.size(); j++) {
            assert(ptrs[i] != ptrs[j]);
        }
    }
    
    // Deallocate all objects
    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }
}

// Exercise 3: Alignment Test
void test_alignment() {
    SlabAllocator allocator(24);  // Non-power-of-2 size
    
    // Allocate multiple objects and check alignment
    for (int i = 0; i < 10; i++) {
        void* ptr = allocator.allocate(24);
        assert(ptr != nullptr);
        
        // Check if pointer is properly aligned
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        assert((addr % alignof(std::max_align_t)) == 0);
        
        allocator.deallocate(ptr);
    }
}

// Exercise 4: Zero Initialization
void test_zero_initialization() {
    SlabAllocator allocator(128);
    
    // Allocate with zero initialization
    void* ptr = allocator.allocate(128, AllocFlags::ZERO);
    assert(ptr != nullptr);
    
    // Verify all bytes are zero
    uint8_t* bytes = static_cast<uint8_t*>(ptr);
    for (size_t i = 0; i < 128; i++) {
        assert(bytes[i] == 0);
    }
    
    allocator.deallocate(ptr);
}

// Exercise 5: Ownership Validation
void test_ownership() {
    SlabAllocator allocator1(64);
    SlabAllocator allocator2(64);
    
    // Allocate from first allocator
    void* ptr1 = allocator1.allocate(64);
    assert(ptr1 != nullptr);
    
    // Verify ownership
    assert(allocator1.owns(ptr1));
    assert(!allocator2.owns(ptr1));
    
    // Clean up
    allocator1.deallocate(ptr1);
}

// Run all basic exercises
void run_basic_exercises() {
    std::cout << "Running basic Slab Allocator exercises...\n";
    
    test_basic_allocation();
    std::cout << "✓ Basic allocation test passed\n";
    
    test_multiple_allocations();
    std::cout << "✓ Multiple allocations test passed\n";
    
    test_alignment();
    std::cout << "✓ Alignment test passed\n";
    
    test_zero_initialization();
    std::cout << "✓ Zero initialization test passed\n";
    
    test_ownership();
    std::cout << "✓ Ownership validation test passed\n";
    
    std::cout << "All basic exercises completed successfully!\n";
}

} // namespace ghost::memory::exercises
