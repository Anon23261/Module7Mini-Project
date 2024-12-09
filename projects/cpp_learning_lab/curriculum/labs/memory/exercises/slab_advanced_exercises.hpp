#pragma once

#include "../include/slab_allocator.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>

namespace ghost::memory::exercises {

// Exercise 1: Stress Testing with Multiple Threads
void test_multithreaded_stress() {
    SlabAllocator allocator(64);
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};
    std::atomic<size_t> total_allocations{0};
    
    // Create multiple threads that continuously allocate and deallocate
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&allocator, &stop, &total_allocations]() {
            std::vector<void*> local_ptrs;
            while (!stop) {
                // Randomly allocate or deallocate
                if (local_ptrs.empty() || (rand() % 2 == 0 && local_ptrs.size() < 100)) {
                    void* ptr = allocator.allocate(64);
                    if (ptr) {
                        local_ptrs.push_back(ptr);
                        total_allocations++;
                    }
                } else {
                    size_t index = rand() % local_ptrs.size();
                    allocator.deallocate(local_ptrs[index]);
                    local_ptrs.erase(local_ptrs.begin() + index);
                }
            }
            
            // Clean up remaining allocations
            for (void* ptr : local_ptrs) {
                allocator.deallocate(ptr);
            }
        });
    }
    
    // Let the test run for a few seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    stop = true;
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify allocator state
    auto stats = allocator.get_stats();
    assert(stats.allocation_count >= total_allocations);
    allocator.check_corruption();
}

// Exercise 2: Memory Pattern Testing
class PatternTester {
    static constexpr uint8_t PATTERN = 0xAA;
    
public:
    static void test_memory_patterns(SlabAllocator& allocator) {
        std::vector<void*> ptrs;
        
        // Allocate and fill with pattern
        for (int i = 0; i < 100; i++) {
            void* ptr = allocator.allocate(128);
            assert(ptr != nullptr);
            std::memset(ptr, PATTERN, 128);
            ptrs.push_back(ptr);
        }
        
        // Verify patterns
        for (void* ptr : ptrs) {
            uint8_t* bytes = static_cast<uint8_t*>(ptr);
            for (size_t i = 0; i < 128; i++) {
                assert(bytes[i] == PATTERN);
            }
        }
        
        // Clean up
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
    }
};

// Exercise 3: Fragmentation Analysis
void test_fragmentation() {
    SlabAllocator allocator(32);
    std::vector<void*> ptrs;
    
    // Allocate many objects
    for (int i = 0; i < 1000; i++) {
        void* ptr = allocator.allocate(32);
        assert(ptr != nullptr);
        ptrs.push_back(ptr);
    }
    
    // Deallocate every other object
    for (size_t i = 0; i < ptrs.size(); i += 2) {
        allocator.deallocate(ptrs[i]);
        ptrs[i] = nullptr;
    }
    
    // Try to allocate large objects
    std::vector<void*> large_ptrs;
    for (int i = 0; i < 10; i++) {
        void* ptr = allocator.allocate(32);
        assert(ptr != nullptr);
        large_ptrs.push_back(ptr);
    }
    
    // Clean up
    for (void* ptr : ptrs) {
        if (ptr) allocator.deallocate(ptr);
    }
    for (void* ptr : large_ptrs) {
        allocator.deallocate(ptr);
    }
}

// Exercise 4: Edge Case Testing
void test_edge_cases() {
    SlabAllocator allocator(64);
    
    // Test nullptr deallocation
    allocator.deallocate(nullptr);  // Should not crash
    
    // Test zero-size allocation
    void* ptr = allocator.allocate(0);
    assert(ptr == nullptr);
    
    // Test oversized allocation
    ptr = allocator.allocate(65);  // Larger than slab size
    assert(ptr == nullptr);
    
    // Test maximum allocations
    std::vector<void*> ptrs;
    while (true) {
        void* p = allocator.allocate(64);
        if (!p) break;
        ptrs.push_back(p);
    }
    
    // Clean up
    for (void* p : ptrs) {
        allocator.deallocate(p);
    }
}

// Exercise 5: Performance Benchmarking
void benchmark_allocator() {
    SlabAllocator allocator(128);
    const int NUM_ITERATIONS = 100000;
    
    // Measure allocation time
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<void*> ptrs;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptrs.push_back(allocator.allocate(128));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto alloc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Measure deallocation time
    start = std::chrono::high_resolution_clock::now();
    for (void* ptr : ptrs) {
        allocator.deallocate(ptr);
    }
    end = std::chrono::high_resolution_clock::now();
    auto dealloc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Allocation time per operation: " 
              << (alloc_time.count() / NUM_ITERATIONS) << "µs\n";
    std::cout << "Deallocation time per operation: " 
              << (dealloc_time.count() / NUM_ITERATIONS) << "µs\n";
}

// Run all advanced exercises
void run_advanced_exercises() {
    std::cout << "Running advanced Slab Allocator exercises...\n";
    
    test_multithreaded_stress();
    std::cout << "✓ Multithreaded stress test passed\n";
    
    SlabAllocator allocator(128);
    PatternTester::test_memory_patterns(allocator);
    std::cout << "✓ Memory pattern test passed\n";
    
    test_fragmentation();
    std::cout << "✓ Fragmentation test passed\n";
    
    test_edge_cases();
    std::cout << "✓ Edge cases test passed\n";
    
    benchmark_allocator();
    std::cout << "✓ Performance benchmark completed\n";
    
    std::cout << "All advanced exercises completed successfully!\n";
}

} // namespace ghost::memory::exercises
