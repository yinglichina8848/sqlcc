#include <gtest/gtest.h>
#include "../include/sql_executor.h"
#include <string>

using namespace std;
using namespace sqlcc;

class SqlExecutorTest : public ::testing::Test {
protected:
  SqlExecutor executor;

  // 在每个测试用例执行前调用
  void SetUp() override {
    // 初始化测试环境
  }

  // 在每个测试用例执行后调用
  void TearDown() override {
    // 清理测试环境
  }
};

// 测试构造函数
TEST_F(SqlExecutorTest, TestConstructor) {
  EXPECT_TRUE(true); // 基本的构造函数测试
}

// 测试Execute方法 - 识别基本SQL语句类型
TEST_F(SqlExecutorTest, TestExecute) {
  string result = executor.Execute("CREATE TABLE test (id INT)");
  EXPECT_TRUE(result.find("CREATE TABLE statement recognized") != string::npos);
  
  result = executor.Execute("DROP TABLE test");
  EXPECT_TRUE(result.find("DROP TABLE statement recognized") != string::npos);
  
  result = executor.Execute("SELECT * FROM test");
  EXPECT_TRUE(result.find("SELECT statement recognized") != string::npos);
  
  result = executor.Execute("INSERT INTO test VALUES (1)");
  EXPECT_TRUE(result.find("INSERT statement recognized") != string::npos);
  
  result = executor.Execute("UPDATE test SET id = 2");
  EXPECT_TRUE(result.find("UPDATE statement recognized") != string::npos);
  
  result = executor.Execute("DELETE FROM test");
  EXPECT_TRUE(result.find("DELETE statement recognized") != string::npos);
}

// 测试ExecuteFile方法
TEST_F(SqlExecutorTest, TestExecuteFile) {
  string result = executor.ExecuteFile("test.sql");
  EXPECT_TRUE(result.find("Executing file") != string::npos);
}

// 测试GetLastError方法
TEST_F(SqlExecutorTest, TestGetLastError) {
  const string& error = executor.GetLastError();
  EXPECT_TRUE(error.empty());
}

// 测试ListTables方法
TEST_F(SqlExecutorTest, TestListTables) {
  string result = executor.ListTables();
  EXPECT_TRUE(result.find("Tables") != string::npos);
  EXPECT_TRUE(result.find("no tables") != string::npos);
}

// 测试ShowTableSchema方法
TEST_F(SqlExecutorTest, TestShowTableSchema) {
  string result = executor.ShowTableSchema("test_table");
  EXPECT_TRUE(result.find("Table schema for") != string::npos);
  EXPECT_TRUE(result.find("test_table") != string::npos);
}

// 测试约束验证方法 - 基本的空实现测试
TEST_F(SqlExecutorTest, TestValidateInsertConstraints) {
  vector<string> record = {"test_value"};
  vector<sql_parser::ColumnDefinition> table_schema;
  bool result = executor.ValidateInsertConstraints("test_table", record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestValidateUpdateConstraints) {
  vector<string> old_record = {"old_value"};
  vector<string> new_record = {"new_value"};
  vector<sql_parser::ColumnDefinition> table_schema;
  bool result = executor.ValidateUpdateConstraints("test_table", old_record, new_record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestValidateDeleteConstraints) {
  vector<string> record = {"test_value"};
  vector<sql_parser::ColumnDefinition> table_schema;
  bool result = executor.ValidateDeleteConstraints("test_table", record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestCreateTableConstraints) {
  vector<sql_parser::TableConstraint> constraints;
  executor.CreateTableConstraints("test_table", constraints);
  // 测试通过意味着没有崩溃
  EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}