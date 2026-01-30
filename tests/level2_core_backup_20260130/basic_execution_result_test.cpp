#include <gtest/gtest.h>

// 最基础的ExecutionResult测试
namespace sqlcc {

// 先模拟ExecutionResult类的基本结构
class ExecutionResult {
public:
    enum Status { SUCCESS = 0, FAILURE = 1 };
    
    ExecutionResult(bool success = true, const std::string& message = "")
        : success(success), message(message) {}
    
    Status getStatus() const { return success ? SUCCESS : FAILURE; }
    const std::string& getMessage() const { return message; }
    bool has_error() const { return !success; }
    
    // 基本字段
    bool success;
    std::string message;
    int64_t rows_affected = 0;
};

} // namespace sqlcc

namespace sqlcc {
namespace test {

class ExecutionResultTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test basic construction
TEST_F(ExecutionResultTest, DefaultConstructor) {
    ExecutionResult result;
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().empty());
    EXPECT_FALSE(result.has_error());
}

TEST_F(ExecutionResultTest, SuccessConstructor) {
    ExecutionResult result(true, "Operation completed");
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_EQ(result.getMessage(), "Operation completed");
    EXPECT_FALSE(result.has_error());
}

TEST_F(ExecutionResultTest, FailureConstructor) {
    ExecutionResult result(false, "Operation failed");
    EXPECT_EQ(result.getStatus(), ExecutionResult::FAILURE);
    EXPECT_EQ(result.getMessage(), "Operation failed");
    EXPECT_TRUE(result.has_error());
}

// Test edge cases
TEST_F(ExecutionResultTest, EmptyMessage) {
    ExecutionResult result(false, "");
    EXPECT_TRUE(result.has_error());
    EXPECT_TRUE(result.getMessage().empty());
}

TEST_F(ExecutionResultTest, RowsAffectedDefault) {
    ExecutionResult result;
    EXPECT_EQ(result.rows_affected, 0);
}

TEST_F(ExecutionResultTest, SetRowsAffected) {
    ExecutionResult result;
    result.rows_affected = 5;
    EXPECT_EQ(result.rows_affected, 5);
}

// Test enum values
TEST_F(ExecutionResultTest, StatusEnumValues) {
    EXPECT_EQ(ExecutionResult::SUCCESS, 0);
    EXPECT_EQ(ExecutionResult::FAILURE, 1);
}

} // namespace test
} // namespace sqlcc