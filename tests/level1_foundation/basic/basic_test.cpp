#include <gtest/gtest.h>
#include "exception/base_exception.h"
#include "exception/io_exception.h"

// Basic test for foundation level
TEST(BasicTest, SimpleTest) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
}

TEST(BasicTest, StringTest) {
    std::string test_str = "hello";
    EXPECT_EQ(test_str.length(), 5);
    EXPECT_EQ(test_str, "hello");
}

TEST(BasicTest, VectorTest) {
    std::vector<int> test_vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(test_vec.size(), 5);
    EXPECT_EQ(test_vec[0], 1);
    EXPECT_EQ(test_vec.back(), 5);
}

// Test for exception module
TEST(ExceptionTest, BaseExceptionTest) {
    try {
        throw sqlcc::Exception("Test exception");
    } catch (const sqlcc::Exception& e) {
        EXPECT_STREQ(e.what(), "Test exception");
    }
}

TEST(ExceptionTest, IOExceptionTest) {
    try {
        throw sqlcc::IOException("IO error occurred");
    } catch (const sqlcc::IOException& e) {
        EXPECT_STREQ(e.what(), "I/O Error: IO error occurred");
    } catch (const sqlcc::Exception&) {
        // IOException should inherit from Exception
        EXPECT_TRUE(true);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
