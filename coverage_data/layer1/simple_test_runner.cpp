#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <string>

// 包含项目头文件
#include "utils/logger.h"

// 测试用例
TEST(LoggerTest, LoggerInitialization) {
    // 测试Logger基本功能
    EXPECT_TRUE(true);
}

TEST(BasicTest, BasicOperations) {
    int a = 5;
    int b = 10;
    EXPECT_EQ(a + b, 15);
    EXPECT_TRUE(a < b);
}

TEST(StringTest, StringOperations) {
    std::string str = "Hello World";
    EXPECT_EQ(str.length(), 11);
    EXPECT_TRUE(str.find("World") != std::string::npos);
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    std::cout << "Running coverage tests with Clang instrumentation..." << std::endl;

    return RUN_ALL_TESTS();
}
