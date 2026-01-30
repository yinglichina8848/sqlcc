#include <gtest/gtest.h>
#include "src/core/execution_result.h"

namespace sqlcc {
namespace test {

class ExecutionResultTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
    
    void TearDown() override {
        // Cleanup code
    }
};

// Test construction
TEST_F(ExecutionResultTest, DefaultConstructor) {
    ExecutionResult result;
    EXPECT_TRUE(result.getStatus() == ExecutionResult::SUCCESS);
}

TEST_F(ExecutionResultTest, ConstructorWithMessage) {
    ExecutionResult result(false, "Test error message");
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.getMessage(), "Test error message");
}

// Test success/failure states
TEST_F(ExecutionResultTest, SuccessResult) {
    ExecutionResult result(true);
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().empty());
}

TEST_F(ExecutionResultTest, FailureResult) {
    ExecutionResult result(false, "Test failure");
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.getMessage(), "Test failure");
}

// Test error message handling
TEST_F(ExecutionResultTest, ErrorMessageHandling) {
    std::string error_msg = "Detailed error message";
    ExecutionResult result(false, error_msg);
    
    EXPECT_EQ(result.getMessage(), error_msg);
    EXPECT_FALSE(result.getMessage().empty());
}

TEST_F(ExecutionResultTest, EmptyErrorMessage) {
    ExecutionResult result(false, "");
    EXPECT_TRUE(result.has_error());
    EXPECT_TRUE(result.getMessage().empty());
}

// Test copy operations
TEST_F(ExecutionResultTest, CopyConstructor) {
    ExecutionResult original(false, "Original message");
    ExecutionResult copy(original);
    
    EXPECT_EQ(copy.getStatus(), original.getStatus());
    EXPECT_EQ(copy.getMessage(), original.getMessage());
}

TEST_F(ExecutionResultTest, CopyAssignment) {
    ExecutionResult original(false, "Original message");
    ExecutionResult copy;
    copy = original;
    
    EXPECT_EQ(copy.getStatus(), original.getStatus());
    EXPECT_EQ(copy.getMessage(), original.getMessage());
}

// Test move operations
TEST_F(ExecutionResultTest, MoveConstructor) {
    ExecutionResult original(false, "Original message");
    ExecutionResult moved(std::move(original));
    
    EXPECT_TRUE(moved.has_error());
    EXPECT_EQ(moved.getMessage(), "Original message");
}

TEST_F(ExecutionResultTest, MoveAssignment) {
    ExecutionResult original(false, "Original message");
    ExecutionResult moved;
    moved = std::move(original);
    
    EXPECT_TRUE(moved.has_error());
    EXPECT_EQ(moved.getMessage(), "Original message");
}

} // namespace test
} // namespace sqlcc