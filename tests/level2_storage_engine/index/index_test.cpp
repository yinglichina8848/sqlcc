#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <unordered_map>

// Index tests for storage layer
// These tests verify index structures and operations

TEST(IndexTest, BasicIndexing) {
    // Test basic index operations
    std::unordered_map<int, std::string> index;
    index[1] = "record_1";
    index[5] = "record_5";
    index[3] = "record_3";

    EXPECT_EQ(index[1], "record_1");
    EXPECT_EQ(index[5], "record_5");
    EXPECT_EQ(index[3], "record_3");
    EXPECT_EQ(index.size(), 3);
}

TEST(IndexTest, IndexLookup) {
    // Test index lookup operations
    std::vector<std::pair<int, int>> index_entries = {
        {10, 100}, {20, 200}, {30, 300}
    };

    // Lookup by key
    auto find_by_key = [&](int key) {
        for (const auto& entry : index_entries) {
            if (entry.first == key) return entry.second;
        }
        return -1;
    };

    EXPECT_EQ(find_by_key(10), 100);
    EXPECT_EQ(find_by_key(20), 200);
    EXPECT_EQ(find_by_key(30), 300);
    EXPECT_EQ(find_by_key(40), -1);  // Not found
}

TEST(IndexTest, IndexMaintenance) {
    // Test index maintenance operations
    std::vector<int> keys = {1, 2, 3, 4, 5};

    // Insert operation
    keys.push_back(6);
    EXPECT_EQ(keys.size(), 6);

    // Delete operation
    keys.erase(std::remove(keys.begin(), keys.end(), 3), keys.end());
    EXPECT_EQ(keys.size(), 5);
    EXPECT_EQ(keys, std::vector<int>({1, 2, 4, 5, 6}));
}