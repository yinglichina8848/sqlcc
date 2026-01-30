#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "src/core/execution_result.h"
#include "src/transaction_manager/wal_manager.h"  // For Row definition

namespace sqlcc {
namespace test {

class ExecutionResultTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test data
        test_row1.emplace_back();
        test_row1.back().push_back(Value(1));
        test_row1.back().push_back(Value("test1"));
        
        test_row2.emplace_back();
        test_row2.back().push_back(Value(2));
        test_row2.back().push_back(Value("test2"));
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    // Helper function to create test rows
    Row createTestRow(int id, const std::string& name) {
        Row row;
        row.emplace_back().push_back(Value(id));
        row.emplace_back().push_back(Value(name));
        return row;
    }
    
    std::vector<Row> test_row1;
    std::vector<Row> test_row2;
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
    // Note: warnings are internal, we can't directly test them without exposing API
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