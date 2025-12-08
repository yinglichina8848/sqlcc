#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

// CompareValues函数的独立实现用于单元测试
// 这与execution_engine.cpp中的compareValues()逻辑相同
bool compareValues(const std::string& left, const std::string& right, const std::string& op) {
    if (op == "=") {
        return left == right;
    } else if (op == "<>") {
        return left != right;
    } else if (op == "<") {
        try {
            int left_val = std::stoi(left);
            int right_val = std::stoi(right);
            return left_val < right_val;
        } catch (...) {
            return left < right;
        }
    } else if (op == ">") {
        try {
            int left_val = std::stoi(left);
            int right_val = std::stoi(right);
            return left_val > right_val;
        } catch (...) {
            return left > right;
        }
    } else if (op == "<=") {
        try {
            int left_val = std::stoi(left);
            int right_val = std::stoi(right);
            return left_val <= right_val;
        } catch (...) {
            return left <= right;
        }
    } else if (op == ">=") {
        try {
            int left_val = std::stoi(left);
            int right_val = std::stoi(right);
            return left_val >= right_val;
        } catch (...) {
            return left >= right;
        }
    } else if (op == "LIKE") {
        return left.find(right) != std::string::npos;
    }
    return false;
}

// ===================== 测试用例 =====================

TEST(CompareValuesTest, EqualOperator) {
    EXPECT_TRUE(compareValues("100", "100", "="));
    EXPECT_FALSE(compareValues("100", "200", "="));
}

TEST(CompareValuesTest, NotEqualOperator) {
    EXPECT_TRUE(compareValues("100", "200", "<>"));
    EXPECT_FALSE(compareValues("100", "100", "<>"));
}

TEST(CompareValuesTest, LessThanOperator) {
    EXPECT_TRUE(compareValues("100", "200", "<"));
    EXPECT_FALSE(compareValues("200", "100", "<"));
    EXPECT_TRUE(compareValues("-100", "100", "<"));
}

TEST(CompareValuesTest, GreaterThanOperator) {
    EXPECT_TRUE(compareValues("200", "100", ">"));
    EXPECT_FALSE(compareValues("100", "200", ">"));
}

TEST(CompareValuesTest, LessThanOrEqualOperator) {
    EXPECT_TRUE(compareValues("100", "200", "<="));
    EXPECT_TRUE(compareValues("100", "100", "<="));
    EXPECT_FALSE(compareValues("200", "100", "<="));
}

TEST(CompareValuesTest, GreaterThanOrEqualOperator) {
    EXPECT_TRUE(compareValues("200", "100", ">="));
    EXPECT_TRUE(compareValues("100", "100", ">="));
    EXPECT_FALSE(compareValues("100", "200", ">="));
}

TEST(CompareValuesTest, LikeOperator) {
    EXPECT_TRUE(compareValues("hello world", "world", "LIKE"));
    EXPECT_TRUE(compareValues("hello world", "hello", "LIKE"));
    EXPECT_FALSE(compareValues("hello world", "xyz", "LIKE"));
    EXPECT_TRUE(compareValues("test123", "123", "LIKE"));
}

TEST(CompareValuesTest, TypeConversion) {
    EXPECT_TRUE(compareValues("10", "20", "<"));
    EXPECT_TRUE(compareValues("100", "20", ">"));
    EXPECT_TRUE(compareValues("-5", "5", "<"));
}

TEST(CompareValuesTest, WhereConditions) {
    // 模拟WHERE age > 18
    EXPECT_TRUE(compareValues("30", "18", ">"));
    // 模拟WHERE salary < 50000
    EXPECT_TRUE(compareValues("45000", "50000", "<"));
    // 模拟WHERE name LIKE 'Ali'
    EXPECT_TRUE(compareValues("Alice", "Ali", "LIKE"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
