#include <gtest/gtest.h>

// 简单的测试函数
int simple_function(int a, int b) {
    if (a > 0) {
        return a + b;
    } else {
        return a - b;
    }
}

// 测试用例
TEST(SimpleCoverageTest, BasicTest) {
    EXPECT_EQ(simple_function(5, 3), 8);
    EXPECT_EQ(simple_function(-2, 3), -5);
    EXPECT_EQ(simple_function(0, 5), -5);
}

TEST(SimpleCoverageTest, EdgeCases) {
    EXPECT_EQ(simple_function(1, 1), 2);
    EXPECT_EQ(simple_function(-1, -1), 0);
}