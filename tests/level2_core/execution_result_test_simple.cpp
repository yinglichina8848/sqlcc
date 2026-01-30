#include <gtest/gtest.h>
#include <vector>
#include <string>

// 简化版本的ExecutionResult测试，不依赖外部模块
#include "src/core/execution_result.h"

// 模拟所需的类型，如果实际模块有问题
namespace sqlcc {

// 如果Value和Row定义有问题，我们使用最简化的测试
#if !defined(VALUE_DEFINED) || !defined(ROW_DEFINED)

// 简化的Value定义
class Value {
public:
    Value() {}
    Value(int i) : int_val_(i), type_(INT) {}
    Value(const std::string& s) : str_val_(s), type_(STRING) {}
    
private:
    int int_val_ = 0;
    std::string str_val_;
    enum Type { INT, STRING } type_ = INT;
};

// 简化的Row定义  
using Row = std::vector<Value>;

#endif

} // namespace sqlcc

namespace sqlcc {
namespace test {

class ExecutionResultTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
    
    // Helper function to create test rows
    Row createTestRow(int id, const std::string& name) {
        Row row;
        row.push_back(Value(id));
        row.push_back(Value(name));
        return row;
    }
};

// Test construction and basic state
TEST_F(ExecutionResultTest, DefaultConstructor) {
    ExecutionResult result;
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().empty());
    EXPECT_EQ(result.row_count(), 0);
    EXPECT_TRUE(result.is_empty());
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

// Test row management
TEST_F(ExecutionResultTest, AddSingleRow) {
    ExecutionResult result;
    Row test_row = createTestRow(1, "test");
    result.add_row(test_row);
    
    EXPECT_EQ(result.row_count(), 1);
    EXPECT_FALSE(result.is_empty());
}

TEST_F(ExecutionResultTest, AddMultipleRows) {
    ExecutionResult result;
    Row row1 = createTestRow(1, "test1");
    Row row2 = createTestRow(2, "test2");
    
    result.add_row(row1);
    result.add_row(row2);
    
    EXPECT_EQ(result.row_count(), 2);
    EXPECT_FALSE(result.is_empty());
}

// Test warning and error management
TEST_F(ExecutionResultTest, AddWarning) {
    ExecutionResult result(true, "Success with warnings");
    result.add_warning("Warning 1");
    result.add_warning("Warning 2");
    
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_EQ(result.getMessage(), "Success with warnings");
}

TEST_F(ExecutionResultTest, AddError) {
    ExecutionResult result(false, "Operation failed");
    result.add_error("Error 1");
    result.add_error("Error 2");
    
    EXPECT_EQ(result.getStatus(), ExecutionResult::FAILURE);
    EXPECT_EQ(result.getMessage(), "Operation failed");
    EXPECT_TRUE(result.has_error());
}

// Test rows_affected field
TEST_F(ExecutionResultTest, RowsAffectedDefault) {
    ExecutionResult result;
    EXPECT_EQ(result.rows_affected, 0);
}

TEST_F(ExecutionResultTest, SetRowsAffected) {
    ExecutionResult result;
    result.rows_affected = 5;
    EXPECT_EQ(result.rows_affected, 5);
}

// Test error_message compatibility
TEST_F(ExecutionResultTest, ErrorMessageCompatibility) {
    ExecutionResult result(false, "Test error");
    
    // Test non-const version
    result.error_message() = "Modified error";
    EXPECT_EQ(result.error_message(), "Modified error");
    EXPECT_EQ(result.getMessage(), "Modified error");
    
    // Test const version
    const ExecutionResult& const_result = result;
    EXPECT_EQ(const_result.error_message(), "Modified error");
}

// Test edge cases
TEST_F(ExecutionResultTest, EmptyMessage) {
    ExecutionResult result(false, "");
    EXPECT_TRUE(result.has_error());
    EXPECT_TRUE(result.getMessage().empty());
}

TEST_F(ExecutionResultTest, LongMessage) {
    std::string long_message(1000, 'x');
    ExecutionResult result(false, long_message);
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.getMessage(), long_message);
}

// Test copy operations
TEST_F(ExecutionResultTest, CopyConstructor) {
    ExecutionResult original(false, "Original message");
    original.rows_affected = 10;
    original.add_warning("Warning message");
    
    ExecutionResult copy(original);
    
    EXPECT_EQ(copy.getStatus(), original.getStatus());
    EXPECT_EQ(copy.getMessage(), original.getMessage());
    EXPECT_EQ(copy.rows_affected, original.rows_affected);
}

TEST_F(ExecutionResultTest, CopyAssignment) {
    ExecutionResult original(false, "Original message");
    original.rows_affected = 15;
    
    ExecutionResult copy;
    copy = original;
    
    EXPECT_EQ(copy.getStatus(), original.getStatus());
    EXPECT_EQ(copy.getMessage(), original.getMessage());
    EXPECT_EQ(copy.rows_affected, original.rows_affected);
}

// Test move operations
TEST_F(ExecutionResultTest, MoveConstructor) {
    ExecutionResult original(false, "Original message");
    original.rows_affected = 20;
    
    ExecutionResult moved(std::move(original));
    
    EXPECT_EQ(moved.getStatus(), ExecutionResult::FAILURE);
    EXPECT_EQ(moved.getMessage(), "Original message");
    EXPECT_EQ(moved.rows_affected, 20);
}

TEST_F(ExecutionResultTest, MoveAssignment) {
    ExecutionResult original(false, "Original message");
    original.rows_affected = 25;
    
    ExecutionResult moved;
    moved = std::move(original);
    
    EXPECT_EQ(moved.getStatus(), ExecutionResult::FAILURE);
    EXPECT_EQ(moved.getMessage(), "Original message");
    EXPECT_EQ(moved.rows_affected, 25);
}

// Test Status enum values
TEST_F(ExecutionResultTest, StatusEnumValues) {
    EXPECT_EQ(ExecutionResult::SUCCESS, 0);
    EXPECT_EQ(ExecutionResult::FAILURE, 1);
}

// Test complex scenarios
TEST_F(ExecutionResultTest, ComplexScenario) {
    ExecutionResult result(true, "Query completed successfully");
    
    // Add rows
    result.add_row(createTestRow(1, "Alice"));
    result.add_row(createTestRow(2, "Bob"));
    result.add_row(createTestRow(3, "Charlie"));
    
    // Set affected rows
    result.rows_affected = 3;
    
    // Add warnings
    result.add_warning("Query performance warning");
    
    // Verify all fields
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_EQ(result.getMessage(), "Query completed successfully");
    EXPECT_EQ(result.row_count(), 3);
    EXPECT_FALSE(result.is_empty());
    EXPECT_EQ(result.rows_affected, 3);
}

} // namespace test
} // namespace sqlcc