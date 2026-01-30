#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "src/core/execution_result.h"

namespace sqlcc {
namespace test {

class ExecutionResultTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
    
    // Helper function to create test rows
    ExecutionResult::Row createTestRow(int id, const std::string& name) {
        ExecutionResult::Row row;
        // Note: This may need adjustment based on the actual Row definition
        // For now, we'll use a simple approach
        (void)id;  // Suppress unused parameter warning
        (void)name;  // Suppress unused parameter warning
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

// Test empty method (if exists)
TEST_F(ExecutionResultTest, CheckEmptyMethod) {
    ExecutionResult result;
    EXPECT_TRUE(result.is_empty()) << "is_empty() should return true for new result";
    
    // If the class has an add_row method, test it
    // result.add_row(createTestRow(1, "test"));
    // EXPECT_FALSE(result.is_empty()) << "is_empty() should return false after adding rows";
}

// Test statistics
TEST_F(ExecutionResultTest, RowsAffectedDefault) {
    ExecutionResult result;
    EXPECT_EQ(result.rows_affected, 0);
}

TEST_F(ExecutionResultTest, SetRowsAffected) {
    ExecutionResult result;
    result.rows_affected = 5;
    EXPECT_EQ(result.rows_affected, 5);
}

// Test error handling
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

// Test optional methods that may exist
TEST_F(ExecutionResultTest, OptionalMethods) {
    ExecutionResult result(true, "Success");
    
    // Test methods that might exist
    // result.add_warning("Test warning");  // If warning system exists
    // result.add_error("Test error");    // If error collection system exists
    
    // Verify status remains unchanged
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
}

} // namespace test
} // namespace sqlcc