// Google Test cases for SqlExecutor
#include "sql_executor.h"
#include <gtest/gtest.h>
#include <string>

using namespace sqlcc;

class SqlExecutorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Use default constructor which creates lightweight in-memory managers
    executor_ = std::make_unique<SqlExecutor>();
  }

  void TearDown() override { executor_.reset(); }

  std::unique_ptr<SqlExecutor> executor_;
};

TEST_F(SqlExecutorTest, ExecuteEmptySql_ReturnsParseError) {
  std::string res = executor_->Execute("");
  // Executor should return an error message for empty/invalid SQL
  EXPECT_FALSE(res.empty());
  EXPECT_NE(res.find("Error:"), std::string::npos);
  // Last error must be populated
  EXPECT_FALSE(executor_->GetLastError().empty());
}

TEST_F(SqlExecutorTest, ExecuteWhitespaceOnly_ReturnsParseError) {
  std::string res = executor_->Execute("   \t\n  ");
  EXPECT_FALSE(res.empty());
  EXPECT_NE(res.find("Error:"), std::string::npos);
  EXPECT_FALSE(executor_->GetLastError().empty());
}

TEST_F(SqlExecutorTest, ExecuteFile_NotFound_ReturnsFileError) {
  const std::string fname = "this_file_should_not_exist_12345.sql";
  std::string res = executor_->ExecuteFile(fname);
  EXPECT_FALSE(res.empty());
  EXPECT_NE(res.find("Error:"), std::string::npos);
  EXPECT_NE(executor_->GetLastError().find("Failed to open file"), std::string::npos);
}

TEST_F(SqlExecutorTest, ExecuteSelect_Smoke_NoThrow) {
  // Basic smoke test: ensure SELECT does not throw and returns a non-empty string
  EXPECT_NO_THROW({
    std::string res = executor_->Execute("SELECT 1;");
    EXPECT_FALSE(res.empty());
  });
}

// Expected coverage uplift: these focused unit tests target SQL parsing and
// file handling error branches in SqlExecutor. Estimated coverage uplift for
// SqlExecutor: ~8-12% (depends on baseline), focused on error/edge handling.
