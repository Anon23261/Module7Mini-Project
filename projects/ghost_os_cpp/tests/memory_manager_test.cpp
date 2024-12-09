#include <gtest/gtest.h>
#include "../include/memory_manager.hpp"

using namespace ghost::os;

class MemoryManagerTest : public ::testing::Test {
protected:
    MemoryManager manager;
};

TEST_F(MemoryManagerTest, BasicAllocation) {
    void* ptr = manager.allocateMemory(1024);
    ASSERT_NE(ptr, nullptr);
    
    auto stats = manager.getMemoryStats();
    EXPECT_EQ(stats.allocation_count, 1);
    EXPECT_EQ(stats.current_usage, 1024);
    
    EXPECT_TRUE(manager.freeMemory(ptr));
    stats = manager.getMemoryStats();
    EXPECT_EQ(stats.current_usage, 0);
}

TEST_F(MemoryManagerTest, MemoryProtection) {
    void* ptr = manager.allocateMemory(1024, true);
    ASSERT_NE(ptr, nullptr);
    
    EXPECT_TRUE(manager.isMemoryProtected(reinterpret_cast<uintptr_t>(ptr)));
    
    // Attempt to write to protected memory should fail
    void* other_ptr = manager.allocateMemory(1024);
    EXPECT_FALSE(manager.copyMemory(ptr, other_ptr, 1024));
    
    manager.freeMemory(ptr);
    manager.freeMemory(other_ptr);
}

TEST_F(MemoryManagerTest, MemoryOperations) {
    void* ptr1 = manager.allocateMemory(1024);
    void* ptr2 = manager.allocateMemory(1024);
    
    // Test zero memory
    EXPECT_TRUE(manager.zeroMemory(ptr1, 1024));
    
    // Test memory copy
    const char* test_data = "Hello, World!";
    std::memcpy(ptr1, test_data, strlen(test_data) + 1);
    EXPECT_TRUE(manager.copyMemory(ptr2, ptr1, strlen(test_data) + 1));
    
    // Test memory compare
    EXPECT_TRUE(manager.compareMemory(ptr1, ptr2, strlen(test_data) + 1));
    
    manager.freeMemory(ptr1);
    manager.freeMemory(ptr2);
}

TEST_F(MemoryManagerTest, MemoryStats) {
    void* ptr1 = manager.allocateMemory(1024);
    void* ptr2 = manager.allocateMemory(2048);
    
    auto stats = manager.getMemoryStats();
    EXPECT_EQ(stats.allocation_count, 2);
    EXPECT_EQ(stats.current_usage, 3072);
    EXPECT_EQ(stats.peak_usage, 3072);
    
    manager.freeMemory(ptr1);
    stats = manager.getMemoryStats();
    EXPECT_EQ(stats.current_usage, 2048);
    EXPECT_EQ(stats.peak_usage, 3072);
    
    manager.freeMemory(ptr2);
}
