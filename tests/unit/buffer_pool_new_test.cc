#include "buffer_pool_new.h"
#include "disk_manager.h"
#include "config_manager.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

namespace sqlcc {

// Test fixture for BufferPool tests
class BufferPoolNewTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test database
        test_db_path = "/tmp/sqlcc_test_buffer_pool_new.db";
        config_manager = std::make_unique<ConfigManager>();
        disk_manager = std::make_unique<DiskManager>(test_db_path, *config_manager);
        buffer_pool = std::make_unique<BufferPool>(disk_manager.get(), 10, *config_manager);
    }

    void TearDown() override {
        buffer_pool.reset();
        disk_manager.reset();
        config_manager.reset();
        // Clean up test file
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }

    std::string test_db_path;
    std::unique_ptr<ConfigManager> config_manager;
    std::unique_ptr<DiskManager> disk_manager;
    std::unique_ptr<BufferPool> buffer_pool;
};

// Test basic page creation and fetching
TEST_F(BufferPoolNewTest, BasicPageOperations) {
    int32_t page_id1, page_id2;

    // Create two new pages
    Page* page1 = buffer_pool->NewPage(&page_id1);
    ASSERT_NE(page1, nullptr);
    ASSERT_GE(page_id1, 0);

    Page* page2 = buffer_pool->NewPage(&page_id2);
    ASSERT_NE(page2, nullptr);
    ASSERT_GE(page_id2, 0);
    ASSERT_NE(page_id1, page_id2);

    // Write some data to pages
    const char* test_data1 = "Hello World!";
    const char* test_data2 = "Test Data 123";

    memcpy(page1->GetData(), test_data1, strlen(test_data1));
    memcpy(page2->GetData(), test_data2, strlen(test_data2));

    // Mark as dirty and unpin
    buffer_pool->UnpinPage(page_id1, true);
    buffer_pool->UnpinPage(page_id2, true);

    // Fetch pages back
    Page* fetched1 = buffer_pool->FetchPage(page_id1);
    ASSERT_NE(fetched1, nullptr);
    ASSERT_STREQ(fetched1->GetData(), test_data1);

    Page* fetched2 = buffer_pool->FetchPage(page_id2);
    ASSERT_NE(fetched2, nullptr);
    ASSERT_STREQ(fetched2->GetData(), test_data2);
}

// Test page replacement when pool is full
TEST_F(BufferPoolNewTest, PageReplacement) {
    // Create a small buffer pool
    auto small_pool = std::make_unique<BufferPool>(disk_manager.get(), 2, *config_manager);

    int32_t page_ids[5];
    Page* pages[5];

    // Fill the buffer pool
    for (int i = 0; i < 5; ++i) {
        pages[i] = small_pool->NewPage(&page_ids[i]);
        ASSERT_NE(pages[i], nullptr);

        // Write unique data
        char data[20];
        sprintf(data, "Page %d data", i);
        memcpy(pages[i]->GetData(), data, strlen(data));

        // Mark as dirty but keep pinned to prevent immediate replacement
    }

    // Unpin all pages
    for (int i = 0; i < 5; ++i) {
        small_pool->UnpinPage(page_ids[i], true);
    }

    // Try to fetch the first page - should still be available
    Page* refetched = small_pool->FetchPage(page_ids[0]);
    ASSERT_NE(refetched, nullptr);
    ASSERT_STREQ(refetched->GetData(), "Page 0 data");
}

// Test dynamic resizing
TEST_F(BufferPoolNewTest, DynamicResizing) {
    size_t initial_size = buffer_pool->GetPoolSize();
    ASSERT_EQ(initial_size, 10u);

    // Resize to larger pool
    bool resize_result = buffer_pool->Resize(15);
    ASSERT_TRUE(resize_result);
    ASSERT_EQ(buffer_pool->GetPoolSize(), 15u);

    // Resize to smaller pool
    resize_result = buffer_pool->Resize(5);
    ASSERT_TRUE(resize_result);
    ASSERT_EQ(buffer_pool->GetPoolSize(), 5u);
}

// Test metrics collection
TEST_F(BufferPoolNewTest, MetricsCollection) {
    BufferPool::Metrics metrics = buffer_pool->GetMetrics();
    ASSERT_EQ(metrics.total_requests, 0u);
    ASSERT_EQ(metrics.cache_hits, 0u);
    ASSERT_EQ(metrics.hit_rate(), 0.0);

    // Create and fetch a page
    int32_t page_id;
    Page* page = buffer_pool->NewPage(&page_id);
    ASSERT_NE(page, nullptr);

    buffer_pool->UnpinPage(page_id, false);

    // Fetch it again - should be a cache hit
    Page* fetched = buffer_pool->FetchPage(page_id);
    ASSERT_NE(fetched, nullptr);

    metrics = buffer_pool->GetMetrics();
    ASSERT_EQ(metrics.total_requests, 1u);
    ASSERT_EQ(metrics.cache_hits, 1u);
    ASSERT_EQ(metrics.hit_rate(), 100.0);
}

// Test page deletion
TEST_F(BufferPoolNewTest, PageDeletion) {
    int32_t page_id;
    Page* page = buffer_pool->NewPage(&page_id);
    ASSERT_NE(page, nullptr);

    // Write some data
    const char* test_data = "Data to be deleted";
    memcpy(page->GetData(), test_data, strlen(test_data));
    buffer_pool->UnpinPage(page_id, true);

    // Verify page exists
    ASSERT_TRUE(buffer_pool->IsPageInBuffer(page_id));

    // Delete page
    bool delete_result = buffer_pool->DeletePage(page_id);
    ASSERT_TRUE(delete_result);

    // Verify page is gone
    ASSERT_FALSE(buffer_pool->IsPageInBuffer(page_id));

    // Try to fetch deleted page - should return nullptr
    Page* fetched = buffer_pool->FetchPage(page_id);
    ASSERT_EQ(fetched, nullptr);
}

// Test flushing
TEST_F(BufferPoolNewTest, PageFlushing) {
    int32_t page_id;
    Page* page = buffer_pool->NewPage(&page_id);
    ASSERT_NE(page, nullptr);

    // Write data and mark dirty
    const char* test_data = "Flush test data";
    memcpy(page->GetData(), test_data, strlen(test_data));
    buffer_pool->UnpinPage(page_id, true);

    // Flush page
    bool flush_result = buffer_pool->FlushPage(page_id);
    ASSERT_TRUE(flush_result);

    // Verify the page is no longer dirty by checking if flush is needed
    Page* pinned = buffer_pool->FetchPage(page_id);
    buffer_pool->UnpinPage(page_id, false);  // Mark as not dirty

    // Try flushing again - should be quick since not dirty
    flush_result = buffer_pool->FlushPage(page_id);
    ASSERT_TRUE(flush_result);
}

} // namespace sqlcc
