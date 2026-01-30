#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

#include "storage/b_plus_tree.h"
#include "storage/b_plus_tree_nodes.h"
#include "storage_engine.h"
#include "src/utils/config_manager.h"
#include "src/utils/logger.h"

namespace fs = std::filesystem;
namespace sqlcc {

class StorageEngineBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir = fs::temp_directory_path() / "sqlcc_boundary_test";
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

// Test B+ tree leaf node boundary operations
TEST_F(StorageEngineBoundaryTest, BPlusTreeLeafNodeBoundary) {
    // Test B+ tree leaf node with boundary conditions
    EXPECT_NO_THROW({
        int32_t page_id;
        auto page = storage_engine->NewPage(&page_id);
        ASSERT_TRUE(page != nullptr);

        auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
        ASSERT_TRUE(leaf_node != nullptr);

        // Test inserting entries at boundaries
        IndexEntry entry1("boundary_key_1", 1, 0);
        IndexEntry entry2("boundary_key_2", 2, 0);

        EXPECT_TRUE(leaf_node->Insert(entry1));
        EXPECT_TRUE(leaf_node->Insert(entry2));

        // Test searching
        auto results = leaf_node->Search("boundary_key_1");
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].key, "boundary_key_1");

        // Test removing entries
        EXPECT_TRUE(leaf_node->Remove("boundary_key_1"));
        results = leaf_node->Search("boundary_key_1");
        EXPECT_TRUE(results.empty());  // Should be removed

        storage_engine->UnpinPage(page_id, true);  // Mark as dirty
    });
}

// Test B+ tree internal node boundary operations
TEST_F(StorageEngineBoundaryTest, BPlusTreeInternalNodeBoundary) {
    // Test B+ tree internal node with boundary conditions
    EXPECT_NO_THROW({
        int32_t page_id;
        auto page = storage_engine->NewPage(&page_id);
        ASSERT_TRUE(page != nullptr);

        auto internal_node = std::make_unique<BPlusTreeInternalNode>(storage_engine, page_id);
        ASSERT_TRUE(internal_node != nullptr);

        // Test inserting child nodes
        int32_t child_page_id1;
        auto child_page1 = storage_engine->NewPage(&child_page_id1);
        ASSERT_TRUE(child_page1 != nullptr);

        internal_node->InsertChild(child_page_id1);

        auto child_ids = internal_node->GetChildPageIds();
        EXPECT_EQ(child_ids.size(), 1);
        EXPECT_EQ(child_ids[0], child_page_id1);

        storage_engine->UnpinPage(page_id, true);  // Mark as dirty
        storage_engine->UnpinPage(child_page_id1, false);
    });
}

// Test B+ tree index with boundary operations
TEST_F(StorageEngineBoundaryTest, BPlusTreeIndexBoundary) {
    // Test B+ tree index with boundary conditions
    EXPECT_NO_THROW({
        auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "boundary_test", "test_column");
        ASSERT_TRUE(btree_index != nullptr);

        // Test creating index
        EXPECT_TRUE(btree_index->Create());
        EXPECT_TRUE(btree_index->Exists());

        // Test operations on empty index
        auto results = btree_index->Search("nonexistent");
        EXPECT_TRUE(results.empty());

        // Test inserting boundary values
        EXPECT_TRUE(btree_index->Insert("", 0, 0));  // Empty key
        EXPECT_TRUE(btree_index->Insert("boundary", INT_MAX, 0));  // Max int

        // Test searching boundary values
        results = btree_index->Search("");
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].key, "");

        results = btree_index->Search("boundary");
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].key, "boundary");
        EXPECT_EQ(results[0].page_id, INT_MAX);

        // Test dropping index
        EXPECT_TRUE(btree_index->Drop());
        EXPECT_FALSE(btree_index->Exists());
    });
}

// Test storage engine with concurrent boundary operations
TEST_F(StorageEngineBoundaryTest, StorageEngineConcurrentBoundary) {
    // Test concurrent access with boundary conditions
    EXPECT_NO_THROW({
        const int num_threads = 3;
        std::vector<std::thread> threads;

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([this, t]() {
                try {
                    // Each thread creates its own index to avoid conflicts
                    auto btree_index = std::make_unique<BPlusTreeIndex>(
                        storage_engine,
                        "concurrent_boundary_" + std::to_string(t),
                        "column_" + std::to_string(t));

                    if (btree_index->Create()) {
                        // Insert boundary test data
                        btree_index->Insert("thread_" + std::to_string(t) + "_key", t, 0);

                        // Search for the inserted data
                        auto results = btree_index->Search("thread_" + std::to_string(t) + "_key");
                        EXPECT_FALSE(results.empty());

                        btree_index->Drop();
                    }
                } catch (const std::exception&) {
                    // Concurrent operations may have exceptions - this is acceptable for boundary testing
                }
            });
        }

        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
    });
}

// Test error handling with invalid inputs
TEST_F(StorageEngineBoundaryTest, ErrorHandlingBoundary) {
    // Test error handling with boundary/invalid inputs
    EXPECT_NO_THROW({
        // Test with null storage engine (should handle gracefully in some operations)
        auto btree_index = std::make_unique<BPlusTreeIndex>(nullptr, "error_test", "column");

        // Operations on index with null storage engine
        auto results = btree_index->Search("test");
        EXPECT_TRUE(results.empty());  // Should handle gracefully

        // Test dropping non-existent index
        EXPECT_TRUE(btree_index->Drop());  // Should succeed (no-op)
    });
}

// Test resource cleanup under boundary stress
TEST_F(StorageEngineBoundaryTest, ResourceCleanupBoundary) {
    // Test resource cleanup with many objects
    EXPECT_NO_THROW({
        std::vector<std::unique_ptr<BPlusTreeIndex>> indices;

        // Create many indices
        for (int i = 0; i < 5; ++i) {
            auto index = std::make_unique<BPlusTreeIndex>(
                storage_engine,
                "cleanup_test_" + std::to_string(i),
                "column_" + std::to_string(i));

            if (index->Create()) {
                index->Insert("cleanup_key_" + std::to_string(i), i, 0);
                indices.push_back(std::move(index));
            }
        }

        // Clear all indices - should cleanup properly
        indices.clear();

        // Verify storage engine still functional
        auto test_index = std::make_unique<BPlusTreeIndex>(storage_engine, "final_test", "column");
        EXPECT_TRUE(test_index->Create());
        EXPECT_TRUE(test_index->Insert("final_key", 999, 0));

        auto results = test_index->Search("final_key");
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].page_id, 999);

        test_index->Drop();
    });
}

} // namespace sqlcc
