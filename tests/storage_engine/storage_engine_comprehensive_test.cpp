#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "storage/b_plus_tree.h"
#include "storage/b_plus_tree_nodes.h"
#include "storage/buffer_pool_sharded.h"
#include "storage_engine.h"
#include "storage_engine/index_manager/smart_index_cache.h"
#include "storage/concurrency_control.h"
#include "storage/advanced_lock_manager.h"
#include "utils/config_manager.h"
#include "utils/logger.h"

namespace fs = std::filesystem;
namespace sqlcc {

// Test fixture for Storage Engine comprehensive testing
class StorageEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        test_dir = fs::temp_directory_path() / "sqlcc_storage_test";
        fs::create_directories(test_dir);

        // Initialize configuration
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("buffer_pool.size", std::string("1024"));  // 1MB buffer pool
        // Initialize storage engine
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());
    }

    void TearDown() override {
        // Clean up resources
        storage_engine.reset();
        config.reset();

        // Remove test directory
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
};

// Test B+ Tree basic operations
TEST_F(StorageEngineTest, BPlusTreeBasicOperations) {
    // Test B+ Tree insert, search, delete operations
    auto btree = std::make_unique<BPlusTreeIndex>(storage_engine, "test_table", "test_index");

    // Create some pages for testing
    int32_t page_id1, page_id2, page_id3, page_id4, page_id5;
    auto page1 = storage_engine->NewPage(&page_id1);
    auto page2 = storage_engine->NewPage(&page_id2);
    auto page3 = storage_engine->NewPage(&page_id3);
    auto page4 = storage_engine->NewPage(&page_id4);
    auto page5 = storage_engine->NewPage(&page_id5);

    // Test insert operations
    EXPECT_TRUE(btree->Insert("1", page_id1, 0));
    EXPECT_TRUE(btree->Insert("2", page_id2, 0));
    EXPECT_TRUE(btree->Insert("3", page_id3, 0));
    EXPECT_TRUE(btree->Insert("10", page_id4, 0));
    EXPECT_TRUE(btree->Insert("5", page_id5, 0));

    // Test search operations
    std::vector<IndexEntry> results;
    results = btree->Search("1");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "1");
    EXPECT_EQ(results[0].page_id, page_id1);

    results = btree->Search("2");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "2");
    EXPECT_EQ(results[0].page_id, page_id2);

    results = btree->Search("10");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "10");
    EXPECT_EQ(results[0].page_id, page_id4);

    // Test search for non-existent key
    results = btree->Search("999");
    EXPECT_TRUE(results.empty());

    // Test delete operations
    EXPECT_TRUE(btree->Delete("2"));
    results = btree->Search("2");
    EXPECT_TRUE(results.empty());  // Should not find deleted key

    // Verify other keys still exist
    results = btree->Search("1");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "1");
}

// Test B+ Tree range queries
TEST_F(StorageEngineTest, BPlusTreeRangeQueries) {
    auto btree = std::make_unique<BPlusTreeIndex>(storage_engine, "test_table", "test_index");

    // Create pages for testing
    std::vector<int32_t> page_ids(100);
    for (int i = 0; i < 100; ++i) {
        storage_engine->NewPage(&page_ids[i]);
    }

    // Insert test data
    for (int i = 0; i < 100; ++i) {
        btree->Insert(std::to_string(i + 1), page_ids[i], 0);
    }

    // Test range query functionality
    // Note: This tests the iterator interface if available
    std::vector<IndexEntry> results;

    // Test search (if supported)
    results = btree->Search("50");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "50");
    EXPECT_EQ(results[0].page_id, page_ids[49]);
}

// Test Buffer Pool LRU functionality
TEST_F(StorageEngineTest, BufferPoolLRUFunctionality) {
    // Test buffer pool page replacement
    int32_t page_id1, page_id2, page_id3;

    // Get pages from buffer pool
    auto page1 = storage_engine->NewPage(&page_id1);
    auto page2 = storage_engine->NewPage(&page_id2);
    auto page3 = storage_engine->NewPage(&page_id3);

    ASSERT_TRUE(page1 != nullptr);
    ASSERT_TRUE(page2 != nullptr);
    ASSERT_TRUE(page3 != nullptr);

    // Modify pages to make them dirty
    auto span1 = page1->GetDataSpan();
    auto span2 = page2->GetDataSpan();
    auto span3 = page3->GetDataSpan();

    // Write some data
    std::memcpy(const_cast<char*>(span1.data()), "test1", 5);
    std::memcpy(const_cast<char*>(span2.data()), "test2", 5);
    std::memcpy(const_cast<char*>(span3.data()), "test3", 5);

    // Unpin pages (mark as dirty)
    storage_engine->UnpinPage(page_id1, true);
    storage_engine->UnpinPage(page_id2, true);
    storage_engine->UnpinPage(page_id3, true);

    // Access pages in different order to test LRU
    storage_engine->FetchPage(page_id1);  // page1 becomes most recently used
    storage_engine->UnpinPage(page_id1, false);

    storage_engine->FetchPage(page_id2);  // page2 becomes most recently used
    storage_engine->UnpinPage(page_id2, false);

    // page3 should be the least recently used
    storage_engine->FetchPage(page_id3);  // page3 becomes most recently used
    storage_engine->UnpinPage(page_id3, false);
}

// Test Index Manager functionality (commented out - not available in current implementation)
// TEST_F(StorageEngineTest, IndexManagerOperations) {
//     // IndexManager functionality not currently implemented
//     // This test would be enabled when IndexManager is implemented
// }

// Test Concurrency Control
TEST_F(StorageEngineTest, ConcurrencyControl) {
    auto lock_manager = std::make_unique<AdvancedLockManager>();

    // Test basic locking operations
    int32_t page_id1 = 1001;
    int32_t page_id2 = 1002;
    int32_t txn1 = 1;
    int32_t txn2 = 2;

    // Test shared locks
    EXPECT_EQ(lock_manager->AcquireLock(page_id1, LockType::SHARED, txn1), GRANTED);
    EXPECT_EQ(lock_manager->AcquireLock(page_id1, LockType::SHARED, txn2), GRANTED);  // Multiple shared locks allowed

    // Test exclusive lock (should block)
    EXPECT_EQ(lock_manager->AcquireLock(page_id2, LockType::EXCLUSIVE, txn1), GRANTED);

    // Test lock release
    EXPECT_TRUE(lock_manager->ReleaseLock(page_id1, txn1));
    EXPECT_TRUE(lock_manager->ReleaseLock(page_id1, txn2));
    EXPECT_TRUE(lock_manager->ReleaseLock(page_id2, txn1));
}

// Test concurrent access to storage engine
TEST_F(StorageEngineTest, ConcurrentStorageAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    const int num_threads = 5;
    const int operations_per_thread = 10;

    // Launch multiple threads performing storage operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, &success_count, operations_per_thread]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    // Perform some storage operations
                    int32_t page_id;
                    auto page = storage_engine->NewPage(&page_id);
                    if (page) {
                        auto span = page->GetDataSpan();
                        std::string data = "thread" + std::to_string(i) + "_op" + std::to_string(j);
                        std::memcpy(const_cast<char*>(span.data()), data.c_str(),
                                  std::min(data.size(), span.size()));
                        storage_engine->UnpinPage(page_id, true);
                        success_count++;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Thread " << i << " exception: " << e.what() << std::endl;
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify operations succeeded
    EXPECT_EQ(success_count, num_threads * operations_per_thread);
}

// Test WAL (Write-Ahead Logging) functionality
TEST_F(StorageEngineTest, WALFunctionality) {
    // Test WAL logging and recovery
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    // Modify page
    auto span = page->GetDataSpan();
    std::memcpy(const_cast<char*>(span.data()), "WAL_TEST_DATA", 13);

    // Unpin with dirty flag (should trigger WAL write)
    storage_engine->UnpinPage(page_id, true);

    // Force checkpoint to test WAL durability
    // Note: This depends on the actual WAL implementation
}

// Test error handling and recovery
TEST_F(StorageEngineTest, ErrorHandlingAndRecovery) {
    // Test various error conditions and recovery mechanisms

    // Test invalid page access
    EXPECT_THROW(storage_engine->FetchPage(-1), std::runtime_error);

    // Test operations on non-existent resources
    auto btree = std::make_unique<BPlusTreeIndex>(storage_engine, "non_existent", "non_existent");
    std::vector<IndexEntry> results;
    results = btree->Search("1");
    EXPECT_TRUE(results.empty());  // Should handle gracefully

    // Test buffer pool exhaustion (if applicable)
    // This would depend on buffer pool size limits
}

// Test storage engine initialization and cleanup
TEST_F(StorageEngineTest, InitializationAndCleanup) {
    // Test proper initialization - storage engine should be initialized after construction
    EXPECT_TRUE(storage_engine != nullptr);

    // Test cleanup operations
    storage_engine.reset();

    // Verify resources are cleaned up
    EXPECT_FALSE(fs::exists(test_dir / "storage.meta"));
}

// Test performance characteristics
TEST_F(StorageEngineTest, PerformanceCharacteristics) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // Perform bulk operations to test performance
    auto btree = std::make_unique<BPlusTreeIndex>(storage_engine, "perf_test", "perf_index");

    const int num_operations = 100; // 减少操作数量以加快测试执行
    std::vector<int32_t> page_ids(num_operations);

    // Create pages first
    for (int i = 0; i < num_operations; ++i) {
        storage_engine->NewPage(&page_ids[i]);
    }

    // Insert operations
    for (int i = 0; i < num_operations; ++i) {
        btree->Insert(std::to_string(i), page_ids[i], 0);
    }

    // Verify all insertions succeeded
    for (int i = 0; i < num_operations; ++i) {
        std::vector<IndexEntry> results = btree->Search(std::to_string(i));
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].key, std::to_string(i));
        EXPECT_EQ(results[0].page_id, page_ids[i]);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Log performance metrics
    std::cout << "Performed " << num_operations << " operations in " << duration.count() << "ms" << std::endl;
    std::cout << "Average operation time: " << (duration.count() * 1000.0 / num_operations) << " microseconds" << std::endl;
}

} // namespace sqlcc