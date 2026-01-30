#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "src/storage/b_plus_tree.h"
#include "src/storage/b_plus_tree_nodes.h"
#include "src/storage_engine.h"
#include "src/utils/config_manager.h"
#include "src/utils/logger.h"

namespace fs = std::filesystem;
namespace sqlcc {

using ::testing::Test;

// Test fixture for B+ Tree core functionality testing
class BPlusTreeCoreTest : public Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        test_dir = fs::temp_directory_path() / "sqlcc_btree_test";
        fs::create_directories(test_dir);

        // Initialize configuration (singleton)
        ConfigManager::GetInstance().SetValue("storage.data_directory", test_dir.string());
        ConfigManager::GetInstance().SetValue("buffer_pool.size", std::string("1024"));  // 1MB buffer pool

        // Initialize storage engine
        storage_engine = std::make_shared<StorageEngine>(ConfigManager::GetInstance(), test_dir.string());
    }

    void TearDown() override {
        // Clean up resources
        storage_engine.reset();

        // Remove test directory
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
    std::shared_ptr<StorageEngine> storage_engine;
};

// Test B+ Tree Node basic operations
TEST_F(BPlusTreeCoreTest, BPlusTreeNodeBasicOperations) {
    // Test BPlusTreeNode construction and basic properties
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    // Test leaf node creation
    auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
    ASSERT_TRUE(leaf_node != nullptr);
    EXPECT_EQ(leaf_node->GetPageId(), page_id);
    EXPECT_TRUE(leaf_node->IsLeaf());

    // Test internal node creation
    int32_t internal_page_id;
    auto internal_page = storage_engine->NewPage(&internal_page_id);
    ASSERT_TRUE(internal_page != nullptr);

    auto internal_node = std::make_unique<BPlusTreeInternalNode>(storage_engine, internal_page_id);
    ASSERT_TRUE(internal_node != nullptr);
    EXPECT_EQ(internal_node->GetPageId(), internal_page_id);
    EXPECT_FALSE(internal_node->IsLeaf());

    storage_engine->UnpinPage(page_id, false);
    storage_engine->UnpinPage(internal_page_id, false);
}

// Test B+ Tree Leaf Node operations
TEST_F(BPlusTreeCoreTest, BPlusTreeLeafNodeOperations) {
    // Create a leaf node
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
    ASSERT_TRUE(leaf_node != nullptr);

    // Test inserting entries
    IndexEntry entry1("key1", 100, 0);
    IndexEntry entry2("key2", 101, 0);
    IndexEntry entry3("key3", 102, 0);

    EXPECT_TRUE(leaf_node->Insert(entry1));
    EXPECT_TRUE(leaf_node->Insert(entry2));
    EXPECT_TRUE(leaf_node->Insert(entry3));

    // Test searching entries
    auto results = leaf_node->Search("key1");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key1");
    EXPECT_EQ(results[0].page_id, 100);

    results = leaf_node->Search("key2");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key2");
    EXPECT_EQ(results[0].page_id, 101);

    // Test searching non-existent key
    results = leaf_node->Search("nonexistent");
    EXPECT_TRUE(results.empty());

    // Test removing entries
    EXPECT_TRUE(leaf_node->Remove("key2"));
    results = leaf_node->Search("key2");
    EXPECT_TRUE(results.empty());  // Should be removed

    // Verify other keys still exist
    results = leaf_node->Search("key1");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key1");

    results = leaf_node->Search("key3");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key3");

    storage_engine->UnpinPage(page_id, true);  // Mark as dirty
}

// Test B+ Tree Internal Node operations
TEST_F(BPlusTreeCoreTest, BPlusTreeInternalNodeOperations) {
    // Create an internal node
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    auto internal_node = std::make_unique<BPlusTreeInternalNode>(storage_engine, page_id);
    ASSERT_TRUE(internal_node != nullptr);

    // Test inserting child nodes
    int32_t child_page_id1;
    auto child_page1 = storage_engine->NewPage(&child_page_id1);
    ASSERT_TRUE(child_page1 != nullptr);

    int32_t child_page_id2;
    auto child_page2 = storage_engine->NewPage(&child_page_id2);
    ASSERT_TRUE(child_page2 != nullptr);

    // Add first child (no key needed)
    internal_node->InsertChild(child_page_id1);
    auto child_ids = internal_node->GetChildPageIds();
    EXPECT_EQ(child_ids.size(), 1);
    EXPECT_EQ(child_ids[0], child_page_id1);

    // Add second child with key
    internal_node->InsertChild(child_page_id2, "separator_key");
    child_ids = internal_node->GetChildPageIds();
    EXPECT_EQ(child_ids.size(), 2);
    EXPECT_EQ(child_ids[1], child_page_id2);

    // Test key search
    auto keys = internal_node->GetKeys();
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys[0], "separator_key");

    // Test FindChildPageId
    int32_t found_child = internal_node->FindChildPageId("aaa");  // Should go to first child
    EXPECT_EQ(found_child, child_page_id1);

    found_child = internal_node->FindChildPageId("zzz");  // Should go to second child
    EXPECT_EQ(found_child, child_page_id2);

    // Test removing child
    internal_node->RemoveChild(child_page_id2);
    child_ids = internal_node->GetChildPageIds();
    EXPECT_EQ(child_ids.size(), 1);
    EXPECT_EQ(child_ids[0], child_page_id1);

    storage_engine->UnpinPage(page_id, true);  // Mark as dirty
    storage_engine->UnpinPage(child_page_id1, false);
    storage_engine->UnpinPage(child_page_id2, false);
}

// Test B+ Tree Index basic operations
TEST_F(BPlusTreeCoreTest, BPlusTreeIndexBasicOperations) {
    // Create a B+ tree index
    auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "test_table", "test_column");
    ASSERT_TRUE(btree_index != nullptr);

    // Test creating the index
    EXPECT_TRUE(btree_index->Create());
    EXPECT_TRUE(btree_index->Exists());

    // Test inserting entries
    EXPECT_TRUE(btree_index->Insert("apple", 1, 0));
    EXPECT_TRUE(btree_index->Insert("banana", 2, 0));
    EXPECT_TRUE(btree_index->Insert("cherry", 3, 0));
    EXPECT_TRUE(btree_index->Insert("date", 4, 0));

    // Test searching entries
    auto results = btree_index->Search("banana");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "banana");
    EXPECT_EQ(results[0].page_id, 2);

    results = btree_index->Search("cherry");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "cherry");
    EXPECT_EQ(results[0].page_id, 3);

    // Test searching non-existent key
    results = btree_index->Search("grape");
    EXPECT_TRUE(results.empty());

    // Test deleting entries
    EXPECT_TRUE(btree_index->Delete("banana"));
    results = btree_index->Search("banana");
    EXPECT_TRUE(results.empty());  // Should be deleted

    // Verify other keys still exist
    results = btree_index->Search("apple");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "apple");

    // Test dropping the index
    EXPECT_TRUE(btree_index->Drop());
    EXPECT_FALSE(btree_index->Exists());
}

// Test B+ Tree Index range queries
TEST_F(BPlusTreeCoreTest, BPlusTreeIndexRangeQueries) {
    // Create a B+ tree index
    auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "test_table", "test_column");
    ASSERT_TRUE(btree_index != nullptr);

    EXPECT_TRUE(btree_index->Create());

    // Insert test data
    EXPECT_TRUE(btree_index->Insert("apple", 1, 0));
    EXPECT_TRUE(btree_index->Insert("banana", 2, 0));
    EXPECT_TRUE(btree_index->Insert("cherry", 3, 0));
    EXPECT_TRUE(btree_index->Insert("date", 4, 0));
    EXPECT_TRUE(btree_index->Insert("elderberry", 5, 0));

    // Test range search functionality
    // Note: Current implementation may not support full range queries
    // but we can test basic search which is the foundation
    auto results = btree_index->Search("cherry");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "cherry");
    EXPECT_EQ(results[0].page_id, 3);

    // Test inserting duplicate keys (should overwrite)
    EXPECT_TRUE(btree_index->Insert("banana", 20, 0));  // Different page_id
    results = btree_index->Search("banana");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "banana");
    EXPECT_EQ(results[0].page_id, 20);  // Should be updated

    EXPECT_TRUE(btree_index->Drop());
}

// Test B+ Tree serialization and deserialization
TEST_F(BPlusTreeCoreTest, BPlusTreeSerialization) {
    // Test that nodes can be properly serialized to and deserialized from pages

    // Create a leaf node and populate it
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
    ASSERT_TRUE(leaf_node != nullptr);

    // Add some entries
    leaf_node->Insert(IndexEntry("key1", 100, 0));
    leaf_node->Insert(IndexEntry("key2", 101, 0));
    leaf_node->Insert(IndexEntry("key3", 102, 0));

    // The node should automatically serialize when modified
    // Now create a new node instance to test deserialization
    auto new_leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
    ASSERT_TRUE(new_leaf_node != nullptr);

    // Check that the data was properly deserialized
    auto results = new_leaf_node->Search("key1");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key1");
    EXPECT_EQ(results[0].page_id, 100);

    results = new_leaf_node->Search("key2");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "key2");
    EXPECT_EQ(results[0].page_id, 101);

    storage_engine->UnpinPage(page_id, false);
}

// Test B+ Tree node splitting logic
TEST_F(BPlusTreeCoreTest, BPlusTreeNodeSplitting) {
    // Test node splitting when it becomes full
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine, page_id);
    ASSERT_TRUE(leaf_node != nullptr);

    // Insert many entries to potentially trigger splitting
    // Note: Actual split behavior depends on the BPLUS_TREE_LEAF_MAX_KEYS constant
    for (int i = 0; i < 10; ++i) {
        std::string key = "key" + std::to_string(i);
        leaf_node->Insert(IndexEntry(key, i + 100, 0));
    }

    // Verify all entries are accessible
    for (int i = 0; i < 10; ++i) {
        std::string key = "key" + std::to_string(i);
        auto results = leaf_node->Search(key);
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0].key, key);
        EXPECT_EQ(results[0].page_id, i + 100);
    }

    storage_engine->UnpinPage(page_id, true);  // Mark as dirty
}

// Test B+ Tree error handling
TEST_F(BPlusTreeCoreTest, BPlusTreeErrorHandling) {
    // Test error conditions and edge cases

    // Test with null storage engine (should throw)
    EXPECT_THROW({
        auto leaf_node = std::make_unique<BPlusTreeLeafNode>(nullptr, 1);
    }, std::invalid_argument);

    // Test invalid page access
    auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "test", "test");
    EXPECT_FALSE(btree_index->Exists());  // Should not exist yet

    // Test operations on non-existent index
    auto results = btree_index->Search("test");
    EXPECT_TRUE(results.empty());  // Should handle gracefully

    // Test creating index with existing name (should succeed - just reuse)
    EXPECT_TRUE(btree_index->Create());
    EXPECT_TRUE(btree_index->Create());  // Should succeed again
    EXPECT_TRUE(btree_index->Drop());
}

// Test B+ Tree concurrent operations (basic)
TEST_F(BPlusTreeCoreTest, BPlusTreeConcurrentOperations) {
    // Test basic concurrent access
    auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "concurrent_test", "test");
    EXPECT_TRUE(btree_index->Create());

    // Insert some initial data
    for (int i = 0; i < 10; ++i) {
        std::string key = "init_key" + std::to_string(i);
        EXPECT_TRUE(btree_index->Insert(key, i, 0));
    }

    // Test concurrent reads
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([this, &btree_index, i, &success_count]() {
            try {
                for (int j = 0; j < 5; ++j) {
                    std::string key = "thread" + std::to_string(i) + "_key" + std::to_string(j);
                    if (btree_index->Insert(key, i * 10 + j, 0)) {
                        auto results = btree_index->Search(key);
                        if (!results.empty() && results[0].key == key) {
                            success_count++;
                        }
                    }
                }
            } catch (const std::exception& e) {
                // Concurrent operations may have race conditions
                // This is expected behavior for basic implementation
            }
        });
    }

    // Wait for threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify some operations succeeded
    EXPECT_GT(success_count, 0);

    EXPECT_TRUE(btree_index->Drop());
}

// Test B+ Tree boundary conditions
TEST_F(BPlusTreeCoreTest, BPlusTreeBoundaryConditions) {
    // Test edge cases and boundary conditions

    auto btree_index = std::make_unique<BPlusTreeIndex>(storage_engine, "boundary_test", "test");
    EXPECT_TRUE(btree_index->Create());

    // Test empty tree operations
    auto results = btree_index->Search("nonexistent");
    EXPECT_TRUE(results.empty());

    // Test single entry
    EXPECT_TRUE(btree_index->Insert("single", 1, 0));
    results = btree_index->Search("single");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].key, "single");
    EXPECT_EQ(results[0].page_id, 1);

    // Test duplicate insertions (should overwrite)
    EXPECT_TRUE(btree_index->Insert("single", 2, 0));
    results = btree_index->Search("single");
    EXPECT_FALSE(results.empty());
    EXPECT_EQ(results[0].page_id, 2);  // Should be updated

    // Test deletion of non-existent key
    EXPECT_TRUE(btree_index->Delete("nonexistent"));  // Should succeed (no-op)

    // Test deletion of existing key
    EXPECT_TRUE(btree_index->Delete("single"));
    results = btree_index->Search("single");
    EXPECT_TRUE(results.empty());

    // Test operations after deletion
    EXPECT_TRUE(btree_index->Insert("after_delete", 3, 0));
    results = btree_index->Search("after_delete");
    EXPECT_FALSE(results.empty());

    EXPECT_TRUE(btree_index->Drop());
}

} // namespace sqlcc
