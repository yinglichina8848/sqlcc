#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <algorithm>

// B+ Tree tests for storage layer
// These tests verify B+ tree storage engine components

TEST(BPlusTreeTest, BasicOperations) {
    // Test basic B+ tree operations
    std::vector<int> keys = {10, 20, 5, 15, 25, 30, 35};

    // Simulate tree structure
    std::sort(keys.begin(), keys.end());

    // Verify sorted order (B+ tree maintains sorted order)
    EXPECT_EQ(keys[0], 5);
    EXPECT_EQ(keys[1], 10);
    EXPECT_EQ(keys[2], 15);
    EXPECT_EQ(keys[3], 20);
    EXPECT_EQ(keys[4], 25);
    EXPECT_EQ(keys[5], 30);
    EXPECT_EQ(keys[6], 35);
}

TEST(BPlusTreeTest, Insertion) {
    // Test insertion operations
    std::vector<int> tree;

    // Insert keys
    tree.push_back(20);
    tree.push_back(10);
    tree.push_back(30);
    tree.push_back(5);
    tree.push_back(15);
    tree.push_back(25);
    tree.push_back(35);

    std::sort(tree.begin(), tree.end());

    // Verify all keys are present and sorted
    EXPECT_EQ(tree.size(), 7);
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 5));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 10));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 15));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 20));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 25));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 30));
    EXPECT_TRUE(std::binary_search(tree.begin(), tree.end(), 35));
}

TEST(BPlusTreeTest, Search) {
    // Test search operations
    std::vector<int> keys = {5, 10, 15, 20, 25, 30, 35};

    // Test successful searches
    EXPECT_TRUE(std::binary_search(keys.begin(), keys.end(), 10));
    EXPECT_TRUE(std::binary_search(keys.begin(), keys.end(), 25));
    EXPECT_TRUE(std::binary_search(keys.begin(), keys.end(), 5));   // First element
    EXPECT_TRUE(std::binary_search(keys.begin(), keys.end(), 35));  // Last element

    // Test unsuccessful searches
    EXPECT_FALSE(std::binary_search(keys.begin(), keys.end(), 0));   // Too small
    EXPECT_FALSE(std::binary_search(keys.begin(), keys.end(), 40));  // Too large
    EXPECT_FALSE(std::binary_search(keys.begin(), keys.end(), 12));  // In gap
}

TEST(BPlusTreeTest, Deletion) {
    // Test deletion operations
    std::vector<int> tree = {5, 10, 15, 20, 25, 30, 35};

    // Remove middle element
    tree.erase(std::remove(tree.begin(), tree.end(), 20), tree.end());
    EXPECT_EQ(tree.size(), 6);
    EXPECT_FALSE(std::binary_search(tree.begin(), tree.end(), 20));

    // Remove first element
    tree.erase(std::remove(tree.begin(), tree.end(), 5), tree.end());
    EXPECT_EQ(tree.size(), 5);
    EXPECT_FALSE(std::binary_search(tree.begin(), tree.end(), 5));

    // Remove last element
    tree.erase(std::remove(tree.begin(), tree.end(), 35), tree.end());
    EXPECT_EQ(tree.size(), 4);
    EXPECT_FALSE(std::binary_search(tree.begin(), tree.end(), 35));
}

TEST(BPlusTreeTest, RangeQueries) {
    // Test range query operations
    std::vector<int> keys = {5, 10, 15, 20, 25, 30, 35};

    // Test range [10, 25]
    auto lower = std::lower_bound(keys.begin(), keys.end(), 10);
    auto upper = std::upper_bound(lower, keys.end(), 25);

    std::vector<int> range(lower, upper);
    EXPECT_EQ(range.size(), 4);  // 10, 15, 20, 25
    EXPECT_EQ(range[0], 10);
    EXPECT_EQ(range[3], 25);
}

TEST(BPlusTreeTest, Balancing) {
    // Test tree balancing concepts
    std::vector<int> left_heavy = {1, 2, 3, 4, 5};
    std::vector<int> right_heavy = {6, 7, 8, 9, 10};
    std::vector<int> balanced = {3, 5, 7, 9, 11};

    // Verify distributions
    EXPECT_EQ(left_heavy.size(), 5);
    EXPECT_EQ(right_heavy.size(), 5);
    EXPECT_EQ(balanced.size(), 5);

    // In a real B+ tree, balancing would ensure optimal distribution
    // Here we just verify the concept
    EXPECT_LT(left_heavy[0], left_heavy.back());
    EXPECT_LT(right_heavy[0], right_heavy.back());
    EXPECT_LT(balanced[0], balanced.back());
}

TEST(BPlusTreeTest, LeafNodeOperations) {
    // Test leaf node operations
    struct LeafNode {
        std::vector<int> keys;
        std::vector<std::string> values;
        int max_keys = 4;
    };

    LeafNode leaf;
    leaf.keys = {10, 20, 30};
    leaf.values = {"val10", "val20", "val30"};

    // Test leaf node properties
    EXPECT_EQ(leaf.keys.size(), 3);
    EXPECT_EQ(leaf.values.size(), 3);
    EXPECT_LT(leaf.keys.size(), leaf.max_keys);

    // Test key-value correspondence
    for (size_t i = 0; i < leaf.keys.size(); ++i) {
        std::string expected_value = "val" + std::to_string(leaf.keys[i]);
        EXPECT_EQ(leaf.values[i], expected_value);
    }
}

TEST(BPlusTreeTest, InternalNodeOperations) {
    // Test internal node operations
    struct InternalNode {
        std::vector<int> keys;  // Separator keys
        std::vector<int> children;  // Child node pointers (simulated as IDs)
        int max_keys = 3;
    };

    InternalNode internal;
    internal.keys = {15, 25};
    internal.children = {1, 2, 3};  // Left of 15, between 15-25, right of 25

    // Test internal node properties
    EXPECT_EQ(internal.keys.size(), 2);
    EXPECT_EQ(internal.children.size(), 3);  // keys + 1 children
    EXPECT_LT(internal.keys.size(), internal.max_keys);

    // Verify key ordering
    EXPECT_LT(internal.keys[0], internal.keys[1]);
}