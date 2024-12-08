#pragma once

#include "../include/pool_allocator.hpp"
#include <cassert>
#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>

namespace ghost::memory::exercises {

/**
 * Exercise 1: Basic Pool Operations
 * Tests basic allocation and deallocation functionality
 */
void test_basic_pool_operations() {
    PoolAllocator pool(64);  // 64-byte blocks
    
    // Test single allocation
    void* ptr1 = pool.allocate(64);
    assert(ptr1 != nullptr);
    assert(pool.owns(ptr1));
    
    // Test multiple allocations
    void* ptr2 = pool.allocate(32);  // Should still give 64 bytes
    assert(ptr2 != nullptr);
    assert(ptr2 != ptr1);
    
    // Test deallocation
    pool.deallocate(ptr1);
    pool.deallocate(ptr2);
    
    // Verify stats
    auto stats = pool.get_stats();
    assert(stats.allocation_count == 2);
    assert(stats.deallocation_count == 2);
}

/**
 * Exercise 2: Pool Expansion
 * Tests pool growth when initial capacity is exceeded
 */
void test_pool_expansion() {
    PoolAllocator pool(32, 2);  // Small initial pool
    std::vector<void*> ptrs;
    
    // Allocate more blocks than initial capacity
    for (int i = 0; i < 5; i++) {
        void* ptr = pool.allocate(32);
        assert(ptr != nullptr);
        ptrs.push_back(ptr);
    }
    
    // Verify pool expanded
    assert(pool.get_pool_count() > 1);
    
    // Clean up
    for (void* ptr : ptrs) {
        pool.deallocate(ptr);
    }
}

/**
 * Exercise 3: Thread Safety
 * Tests concurrent allocations and deallocations
 */
void test_thread_safety() {
    PoolAllocator pool(128);
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;
    
    // Create multiple threads that allocate and deallocate
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&pool, &success_count]() {
            std::vector<void*> local_ptrs;
            
            for (int j = 0; j < 100; j++) {
                void* ptr = pool.allocate(128);
                if (ptr) {
                    local_ptrs.push_back(ptr);
                    success_count++;
                }
            }
            
            // Deallocate all local pointers
            for (void* ptr : local_ptrs) {
                pool.deallocate(ptr);
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify allocations succeeded
    assert(success_count > 0);
    pool.check_corruption();
}

/**
 * Exercise 4: Memory Patterns
 * Tests memory pattern verification
 */
void test_memory_patterns() {
    PoolAllocator pool(256);
    std::vector<void*> ptrs;
    
    // Allocate and fill blocks with patterns
    for (int i = 0; i < 10; i++) {
        void* ptr = pool.allocate(256);
        assert(ptr != nullptr);
        
        // Fill with pattern
        std::memset(ptr, 0xAA, 256);
        ptrs.push_back(ptr);
    }
    
    // Verify patterns
    for (void* ptr : ptrs) {
        uint8_t* bytes = static_cast<uint8_t*>(ptr);
        for (size_t i = 0; i < 256; i++) {
            assert(bytes[i] == 0xAA);
        }
    }
    
    // Clean up
    for (void* ptr : ptrs) {
        pool.deallocate(ptr);
    }
}

/**
 * Exercise 5: Performance Benchmark
 * Measures allocation and deallocation performance
 */
void benchmark_pool_allocator() {
    PoolAllocator pool(64);
    const int NUM_ITERATIONS = 100000;
    std::vector<void*> ptrs;
    ptrs.reserve(NUM_ITERATIONS);
    
    // Measure allocation time
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptrs.push_back(pool.allocate(64));
    }
    
    auto mid = std::chrono::high_resolution_clock::now();
    
    // Measure deallocation time
    for (void* ptr : ptrs) {
        pool.deallocate(ptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto alloc_time = std::chrono::duration_cast<std::chrono::microseconds>(
        mid - start).count();
    auto dealloc_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end - mid).count();
    
    std::cout << "Pool Allocator Performance:\n"
              << "Allocation time per operation: " 
              << (alloc_time / NUM_ITERATIONS) << "µs\n"
              << "Deallocation time per operation: "
              << (dealloc_time / NUM_ITERATIONS) << "µs\n";
}

/**
 * Run all pool allocator exercises
 */
void run_pool_exercises() {
    std::cout << "Running Pool Allocator exercises...\n";
    
    test_basic_pool_operations();
    std::cout << "✓ Basic operations test passed\n";
    
    test_pool_expansion();
    std::cout << "✓ Pool expansion test passed\n";
    
    test_thread_safety();
    std::cout << "✓ Thread safety test passed\n";
    
    test_memory_patterns();
    std::cout << "✓ Memory patterns test passed\n";
    
    benchmark_pool_allocator();
    std::cout << "✓ Performance benchmark completed\n";
    
    std::cout << "All Pool Allocator exercises completed successfully!\n";
}

} // namespace ghost::memory::exercises
