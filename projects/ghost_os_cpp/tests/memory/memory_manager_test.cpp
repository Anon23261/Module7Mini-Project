#include <gtest/gtest.h>
#include "../../src/memory/memory_manager.hpp"

namespace ghost::memory::test {

class MemoryManagerTest : public ::testing::Test {
protected:
    MemoryManager& mm = MemoryManager::instance();
};

TEST_F(MemoryManagerTest, AllocateAndFreePage) {
    // Test single page allocation
    auto page1 = mm.allocate_page();
    ASSERT_TRUE(page1.has_value());
    EXPECT_EQ(mm.get_used_pages(), 1);

    // Test page freeing
    EXPECT_TRUE(mm.free_page(page1.value()));
    EXPECT_EQ(mm.get_used_pages(), 0);
}

TEST_F(MemoryManagerTest, MultipleAllocations) {
    std::vector<void*> pages;
    
    // Allocate 10 pages
    for (int i = 0; i < 10; ++i) {
        auto page = mm.allocate_page();
        ASSERT_TRUE(page.has_value());
        pages.push_back(page.value());
    }

    EXPECT_EQ(mm.get_used_pages(), 10);

    // Free all pages
    for (auto* page : pages) {
        EXPECT_TRUE(mm.free_page(page));
    }

    EXPECT_EQ(mm.get_used_pages(), 0);
}

TEST_F(MemoryManagerTest, OutOfMemory) {
    std::vector<void*> pages;
    
    // Try to allocate more pages than available
    for (size_t i = 0; i <= MAX_PAGES; ++i) {
        auto page = mm.allocate_page();
        if (page.has_value()) {
            pages.push_back(page.value());
        } else {
            // Should fail after MAX_PAGES allocations
            EXPECT_EQ(i, MAX_PAGES);
            break;
        }
    }

    // Cleanup
    for (auto* page : pages) {
        mm.free_page(page);
    }
}

} // namespace ghost::memory::test
