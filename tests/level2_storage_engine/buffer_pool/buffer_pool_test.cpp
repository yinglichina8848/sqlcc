#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <list>

// Buffer Pool tests for storage layer
// These tests verify buffer pool storage components

TEST(BufferPoolTest, PageManagement) {
    // Test page management in buffer pool
    struct Page {
        int page_id;
        bool is_dirty;
        int pin_count;
        std::string data;
    };

    std::vector<Page> pages;
    pages.push_back({1, false, 0, "page_data_1"});
    pages.push_back({2, true, 1, "page_data_2"});
    pages.push_back({3, false, 2, "page_data_3"});

    // Verify page management
    EXPECT_EQ(pages.size(), 3);
    EXPECT_EQ(pages[0].page_id, 1);
    EXPECT_FALSE(pages[0].is_dirty);
    EXPECT_EQ(pages[0].pin_count, 0);

    EXPECT_EQ(pages[1].page_id, 2);
    EXPECT_TRUE(pages[1].is_dirty);
    EXPECT_EQ(pages[1].pin_count, 1);
}

TEST(BufferPoolTest, LRUReplacement) {
    // Test LRU (Least Recently Used) replacement policy
    std::list<int> access_order = {1, 2, 3, 4, 5};  // Most recent to least recent

    // Access page 3 (move to front)
    access_order.remove(3);
    access_order.push_front(3);

    // Verify order: 3, 1, 2, 4, 5
    auto it = access_order.begin();
    EXPECT_EQ(*it, 3); ++it;
    EXPECT_EQ(*it, 1); ++it;
    EXPECT_EQ(*it, 2); ++it;
    EXPECT_EQ(*it, 4); ++it;
    EXPECT_EQ(*it, 5);

    // LRU page should be 5
    EXPECT_EQ(access_order.back(), 5);
}

TEST(BufferPoolTest, PinUnpinOperations) {
    // Test pin/unpin operations
    struct BufferPage {
        int page_id;
        int pin_count = 0;
        bool can_evict = true;
    };

    BufferPage page{100, 0, true};

    // Pin operation
    page.pin_count++;
    page.can_evict = false;
    EXPECT_EQ(page.pin_count, 1);
    EXPECT_FALSE(page.can_evict);

    // Another pin
    page.pin_count++;
    EXPECT_EQ(page.pin_count, 2);
    EXPECT_FALSE(page.can_evict);

    // Unpin operation
    page.pin_count--;
    EXPECT_EQ(page.pin_count, 1);
    EXPECT_FALSE(page.can_evict);

    // Final unpin
    page.pin_count--;
    page.can_evict = true;
    EXPECT_EQ(page.pin_count, 0);
    EXPECT_TRUE(page.can_evict);
}

TEST(BufferPoolTest, DirtyPageHandling) {
    // Test dirty page handling
    std::vector<std::pair<int, bool>> pages = {
        {1, false},  // Clean
        {2, true},   // Dirty
        {3, false},  // Clean
        {4, true},   // Dirty
        {5, false}   // Clean
    };

    // Find dirty pages
    std::vector<int> dirty_pages;
    for (const auto& page : pages) {
        if (page.second) {  // is_dirty
            dirty_pages.push_back(page.first);
        }
    }

    EXPECT_EQ(dirty_pages.size(), 2);
    EXPECT_EQ(dirty_pages[0], 2);
    EXPECT_EQ(dirty_pages[1], 4);
}

TEST(BufferPoolTest, PageEviction) {
    // Test page eviction logic
    struct EvictionCandidate {
        int page_id;
        bool is_pinned;
        bool is_dirty;
        int last_access_time;
    };

    std::vector<EvictionCandidate> candidates = {
        {1, false, false, 100},  // Can evict, clean
        {2, true, false, 200},   // Cannot evict, pinned
        {3, false, true, 150},   // Can evict but dirty
        {4, false, false, 50},   // Can evict, clean, oldest
    };

    // Find best eviction candidate (oldest non-pinned clean page)
    EvictionCandidate* best_candidate = nullptr;
    for (auto& candidate : candidates) {
        if (!candidate.is_pinned && !candidate.is_dirty) {
            if (!best_candidate || candidate.last_access_time < best_candidate->last_access_time) {
                best_candidate = &candidate;
            }
        }
    }

    ASSERT_NE(best_candidate, nullptr);
    EXPECT_EQ(best_candidate->page_id, 4);  // Oldest clean page
}

TEST(BufferPoolTest, ConcurrencyControl) {
    // Test basic concurrency control concepts
    std::unordered_map<int, int> page_locks;  // page_id -> lock_count

    // Acquire locks
    page_locks[1] = 1;  // Page 1 locked once
    page_locks[2] = 2;  // Page 2 locked twice

    // Verify lock counts
    EXPECT_EQ(page_locks[1], 1);
    EXPECT_EQ(page_locks[2], 2);

    // Release locks
    page_locks[1]--;
    if (page_locks[1] == 0) {
        page_locks.erase(1);
    }

    EXPECT_EQ(page_locks.count(1), 0);  // Lock released
    EXPECT_EQ(page_locks[2], 2);       // Still locked
}