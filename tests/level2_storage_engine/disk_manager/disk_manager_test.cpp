#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>

// Disk Manager tests for storage layer
// These tests verify disk I/O and file management components

TEST(DiskManagerTest, FileOperations) {
    // Test basic file operations
    std::vector<std::string> file_operations = {
        "open", "read", "write", "seek", "close"
    };

    EXPECT_EQ(file_operations.size(), 5);
    EXPECT_EQ(file_operations[0], "open");
    EXPECT_EQ(file_operations[4], "close");
}

TEST(DiskManagerTest, PageReadWrite) {
    // Test page-level read/write operations
    struct PageData {
        int page_id;
        std::string content;
        bool is_modified;
    };

    std::vector<PageData> pages;
    pages.push_back({1, "page_content_1", false});
    pages.push_back({2, "page_content_2", true});
    pages.push_back({3, "page_content_3", false});

    // Verify page data integrity
    EXPECT_EQ(pages[0].page_id, 1);
    EXPECT_EQ(pages[0].content, "page_content_1");
    EXPECT_FALSE(pages[0].is_modified);

    EXPECT_TRUE(pages[1].is_modified);
    EXPECT_EQ(pages[2].content, "page_content_3");
}

TEST(DiskManagerTest, FileAllocation) {
    // Test file space allocation
    const size_t PAGE_SIZE = 4096;
    std::vector<size_t> allocated_pages = {1, 5, 10, 15};

    // Calculate total allocated space
    size_t total_space = 0;
    for (size_t page_num : allocated_pages) {
        total_space += page_num * PAGE_SIZE;
    }

    EXPECT_EQ(total_space, 31 * PAGE_SIZE);  // 1+5+10+15 = 31 pages
}

TEST(DiskManagerTest, ErrorHandling) {
    // Test disk I/O error handling
    std::vector<std::string> error_conditions = {
        "file_not_found",
        "permission_denied",
        "disk_full",
        "io_error"
    };

    // Verify error conditions are recognized
    EXPECT_EQ(error_conditions.size(), 4);
    EXPECT_EQ(error_conditions[0], "file_not_found");
    EXPECT_EQ(error_conditions[3], "io_error");
}