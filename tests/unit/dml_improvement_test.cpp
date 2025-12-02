#include <gtest/gtest.h>
#include <memory>
#include "execution_engine.h"
#include "database_manager.h"

using namespace sqlcc;

class DMLImprovementTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>();
        ASSERT_TRUE(db_manager_ != nullptr);
        
        // 初始化DML执行器
        dml_executor_ = std::make_unique<DMLExecutor>(db_manager_);
        ASSERT_TRUE(dml_executor_ != nullptr);
    }

    void TearDown() override {
        dml_executor_.reset();
        db_manager_.reset();
    }

    std::shared_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<DMLExecutor> dml_executor_;
};

// ===================== WHERE条件评估改进测试 =====================

// 测试1：compareValues - 相等操作
TEST_F(DMLImprovementTest, CompareValues_EqualOperator) {
    // 测试字符串相等
    EXPECT_TRUE(dml_executor_->compareValues("100", "100", "="));
    EXPECT_FALSE(dml_executor_->compareValues("100", "200", "="));
    
    // 测试字符串比较
    EXPECT_TRUE(dml_executor_->compareValues("abc", "abc", "="));
    EXPECT_FALSE(dml_executor_->compareValues("abc", "def", "="));
}

// 测试2：compareValues - 不等操作
TEST_F(DMLImprovementTest, CompareValues_NotEqualOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("100", "200", "<>"));
    EXPECT_FALSE(dml_executor_->compareValues("100", "100", "<>"));
    
    EXPECT_TRUE(dml_executor_->compareValues("abc", "def", "<>"));
    EXPECT_FALSE(dml_executor_->compareValues("abc", "abc", "<>"));
}

// 测试3：compareValues - 小于操作
TEST_F(DMLImprovementTest, CompareValues_LessThanOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("100", "200", "<"));
    EXPECT_FALSE(dml_executor_->compareValues("200", "100", "<"));
    EXPECT_FALSE(dml_executor_->compareValues("100", "100", "<"));
    
    // 测试负数
    EXPECT_TRUE(dml_executor_->compareValues("-100", "100", "<"));
    EXPECT_FALSE(dml_executor_->compareValues("100", "-100", "<"));
}

// 测试4：compareValues - 大于操作
TEST_F(DMLImprovementTest, CompareValues_GreaterThanOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("200", "100", ">"));
    EXPECT_FALSE(dml_executor_->compareValues("100", "200", ">"));
    EXPECT_FALSE(dml_executor_->compareValues("100", "100", ">"));
}

// 测试5：compareValues - 小于等于操作
TEST_F(DMLImprovementTest, CompareValues_LessThanOrEqualOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("100", "200", "<="));
    EXPECT_TRUE(dml_executor_->compareValues("100", "100", "<="));
    EXPECT_FALSE(dml_executor_->compareValues("200", "100", "<="));
}

// 测试6：compareValues - 大于等于操作
TEST_F(DMLImprovementTest, CompareValues_GreaterThanOrEqualOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("200", "100", ">="));
    EXPECT_TRUE(dml_executor_->compareValues("100", "100", ">="));
    EXPECT_FALSE(dml_executor_->compareValues("100", "200", ">="));
}

// 测试7：compareValues - LIKE操作
TEST_F(DMLImprovementTest, CompareValues_LikeOperator) {
    EXPECT_TRUE(dml_executor_->compareValues("hello world", "world", "LIKE"));
    EXPECT_TRUE(dml_executor_->compareValues("hello world", "hello", "LIKE"));
    EXPECT_FALSE(dml_executor_->compareValues("hello world", "xyz", "LIKE"));
    
    EXPECT_TRUE(dml_executor_->compareValues("test123", "123", "LIKE"));
    EXPECT_TRUE(dml_executor_->compareValues("alice@example.com", "example.com", "LIKE"));
    EXPECT_FALSE(dml_executor_->compareValues("alice@example.com", "google.com", "LIKE"));
}

// 测试8：compareValues - 字符串和数字的自动类型转换
TEST_F(DMLImprovementTest, CompareValues_TypeConversion) {
    // 字符串数字比较应该按数字比较
    EXPECT_TRUE(dml_executor_->compareValues("10", "20", "<"));
    EXPECT_TRUE(dml_executor_->compareValues("100", "20", ">"));
    
    // 测试负数
    EXPECT_TRUE(dml_executor_->compareValues("-5", "5", "<"));
    EXPECT_FALSE(dml_executor_->compareValues("-5", "-10", "<"));
    EXPECT_TRUE(dml_executor_->compareValues("-10", "-5", "<"));
}

// 测试9：compareValues - 整数和浮点数
TEST_F(DMLImprovementTest, CompareValues_DecimalNumbers) {
    EXPECT_TRUE(dml_executor_->compareValues("10.5", "20.5", "<"));
    EXPECT_TRUE(dml_executor_->compareValues("20.5", "10.5", ">"));
    EXPECT_TRUE(dml_executor_->compareValues("10.5", "10.5", "="));
}

// 测试10：综合比较操作测试
TEST_F(DMLImprovementTest, CompareValues_Comprehensive) {
    // 使用comparev​Values模拟WHERE条件评估
    // WHERE age > 18 AND age < 65
    int age = 30;
    std::string age_str = std::to_string(age);
    
    // age > 18
    EXPECT_TRUE(dml_executor_->compareValues(age_str, "18", ">"));
    // age < 65
    EXPECT_TRUE(dml_executor_->compareValues(age_str, "65", "<"));
    
    // 边界测试
    EXPECT_FALSE(dml_executor_->compareValues("18", "18", ">"));
    EXPECT_TRUE(dml_executor_->compareValues("18", "18", "="));
}

// 测试11：验证executorNOT NULL约束验证能力
TEST_F(DMLImprovementTest, DMLExecutorInitialization) {
    // 确保执行器已正确初始化
    EXPECT_TRUE(dml_executor_ != nullptr);
    EXPECT_TRUE(db_manager_ != nullptr);
}

// 测试12：验证executor支持多种操作符
TEST_F(DMLImprovementTest, DMLExecutorSupportedOperators) {
    // 验证所有支持的操作符都能正常工作
    std::vector<std::string> operators = {"=", "<>", "<", ">", "<=", ">=", "LIKE"};
    
    for (const auto& op : operators) {
        // 简单验证操作符是否被识别和处理
        if (op == "LIKE") {
            EXPECT_TRUE(dml_executor_->compareValues("test", "test", op));
        } else {
            EXPECT_FALSE(dml_executor_->compareValues("1", "2", op) && 
                        dml_executor_->compareValues("2", "1", op));
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
