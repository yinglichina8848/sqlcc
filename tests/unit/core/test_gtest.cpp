#include <gtest/gtest.h>
#include <memory>

// 简单测试Google Test是否正常工作
TEST(SimpleTest, BasicAssertions) {
  // 测试基本的断言
  EXPECT_EQ(1, 1);
  EXPECT_NE(1, 2);
  EXPECT_TRUE(true);
  EXPECT_FALSE(false);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}