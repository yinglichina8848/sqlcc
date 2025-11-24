#include "sql_executor.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace std;
using namespace sqlcc;

class SqlExecutorTest : public ::testing::Test {
protected:
  SqlExecutor executor;

  // 在每个测试用例执行前调用
  void SetUp() override {
    // 清理之前可能存在的表，确保测试环境干净
    executor.Execute("DROP TABLE IF EXISTS users");
    executor.Execute("DROP TABLE IF EXISTS products");
    executor.Execute("DROP TABLE IF EXISTS orders");
  }

  // 在每个测试用例执行后调用
  void TearDown() override {
    // 清理测试环境，删除创建的表
    executor.Execute("DROP TABLE IF EXISTS users");
    executor.Execute("DROP TABLE IF EXISTS products");
    executor.Execute("DROP TABLE IF EXISTS orders");
  }
};

// 测试构造函数
TEST_F(SqlExecutorTest, TestConstructor) {
  EXPECT_TRUE(true); // 基本的构造函数测试
}

// 测试Execute方法 - 识别基本SQL语句类型
TEST_F(SqlExecutorTest, TestExecute) {
  string result = executor.Execute("CREATE TABLE test (id INT)");
  EXPECT_TRUE(result.find("CREATE executed") != string::npos);

  result = executor.Execute("DROP TABLE test");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
}

// 测试DML功能 - 真实的CRUD操作
TEST_F(SqlExecutorTest, TestRealCRUDOperations) {
  // 1. 创建表（DDL操作）
  string result = executor.Execute("CREATE TABLE users (id INT, name "
                                   "VARCHAR(50), age INT, email VARCHAR(100))");
  EXPECT_TRUE(result.find("CREATE executed") != string::npos);

  // 2. 插入数据（DML - INSERT操作）
  result = executor.Execute(
      "INSERT INTO users VALUES (1, '张三', 28, 'zhangsan@example.com')");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
  EXPECT_TRUE(result.find("1 row(s) affected") != string::npos);

  // 插入多行数据
  result = executor.Execute("INSERT INTO users VALUES 
                              (2, '李四', 32, 'lisi@example.com'), 
                              (3, '王五', 45, 'wangwu@example.com'),
                              (4, '赵六', 25, 'zhaoliu@example.com')");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
  EXPECT_TRUE(result.find("3 row(s) affected") != string::npos);
  
  // 3. 查询数据（DML - SELECT操作）并验证数据正确性
  result = executor.Execute("SELECT * FROM users WHERE id = 1");
  EXPECT_TRUE(result.find("Query OK") != string::npos || result.find("1 row(s) in set") != string::npos);
  EXPECT_TRUE(result.find("张三") != string::npos);
  EXPECT_TRUE(result.find("28") != string::npos);
  
  // 4. 更新数据（DML - UPDATE操作）
  result = executor.Execute("UPDATE users SET age = 29, email = 'zhangsan_new@example.com' WHERE id = 1");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
  EXPECT_TRUE(result.find("1 row(s) affected") != string::npos);
  
  // 验证更新后的数据
  result = executor.Execute("SELECT age, email FROM users WHERE id = 1");
  EXPECT_TRUE(result.find("29") != string::npos);
  EXPECT_TRUE(result.find("zhangsan_new@example.com") != string::npos);
  
  // 5. 删除数据（DML - DELETE操作）
  result = executor.Execute("DELETE FROM users WHERE id = 4");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
  EXPECT_TRUE(result.find("1 row(s) affected") != string::npos);
  
  // 验证删除后的数据
  result = executor.Execute("SELECT * FROM users WHERE id = 4");
  EXPECT_TRUE(result.find("Empty set") != string::npos);
  
  // 6. 查询所有剩余数据，验证总记录数
  result = executor.Execute("SELECT * FROM users");
  EXPECT_TRUE(result.find("3 row(s) in set") != string::npos);
}

// 测试更复杂的查询和多表操作
TEST_F(SqlExecutorTest, TestComplexQueries) {
  // 创建两个关联表
  executor.Execute("CREATE TABLE products (id INT, name VARCHAR(50), price "
                   "DECIMAL(10,2), category VARCHAR(50))");
  executor.Execute("CREATE TABLE orders (order_id INT, product_id INT, "
                   "quantity INT, customer_name VARCHAR(50))");

  // 插入真实的产品数据
  executor.Execute("INSERT INTO products VALUES 
                    (1, '笔记本电脑', 5999.00, '电子产品'),
                    (2, '智能手机', 3999.00, '电子产品'),
                    (3, '机械键盘', 299.00, '外设'),
                    (4, '无线鼠标', 99.00, '外设'),
                    (5, '显示器', 1499.00, '电子产品')");
  
  // 插入真实的订单数据
  executor.Execute("INSERT INTO orders VALUES 
                    (101, 1, 1, '张三'),
                    (102, 3, 2, '李四'),
                    (103, 2, 1, '王五'),
                    (104, 4, 3, '赵六'),
                    (105, 1, 2, '钱七')");
  
  // 测试条件查询
  string result = executor.Execute("SELECT * FROM products");
  EXPECT_TRUE(result.find("笔记本电脑") != string::npos);
  EXPECT_TRUE(result.find("显示器") != string::npos);
  
  // 测试订单统计查询
  result = executor.Execute("SELECT * FROM orders WHERE quantity > 1");
  
  // 测试删除操作后查询
  executor.Execute("DELETE FROM orders WHERE order_id = 102");
  result = executor.Execute("SELECT * FROM orders WHERE customer_name = '李四'");
}

// 测试ExecuteFile方法
TEST_F(SqlExecutorTest, TestExecuteFile) {
  string result = executor.ExecuteFile("test.sql");
  EXPECT_TRUE(result.find("Executing file") != string::npos);
}

// 测试GetLastError方法
TEST_F(SqlExecutorTest, TestGetLastError) {
  const string &error = executor.GetLastError();
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
  bool result =
      executor.ValidateInsertConstraints("test_table", record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestValidateUpdateConstraints) {
  vector<string> old_record = {"old_value"};
  vector<string> new_record = {"new_value"};
  vector<sql_parser::ColumnDefinition> table_schema;
  bool result = executor.ValidateUpdateConstraints("test_table", old_record,
                                                   new_record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestValidateDeleteConstraints) {
  vector<string> record = {"test_value"};
  vector<sql_parser::ColumnDefinition> table_schema;
  bool result =
      executor.ValidateDeleteConstraints("test_table", record, table_schema);
  EXPECT_TRUE(result);
}

TEST_F(SqlExecutorTest, TestCreateTableConstraints) {
  vector<sql_parser::TableConstraint> constraints;
  executor.CreateTableConstraints("test_table", constraints);
  // 测试通过意味着没有崩溃
  EXPECT_TRUE(true);
}

// 测试DCL命令功能 - 用户和权限管理
TEST_F(SqlExecutorTest, TestDCLCommands) {
  // 1. 测试CREATE USER命令
  string result = executor.Execute("CREATE USER test_user IDENTIFIED BY 'password123'");
  EXPECT_TRUE(result.find("User created successfully") != string::npos);

  // 2. 测试GRANT命令
  result = executor.Execute("GRANT SELECT, INSERT ON users TO test_user");
  EXPECT_TRUE(result.find("Privilege granted successfully") != string::npos);

  // 3. 测试REVOKE命令
  result = executor.Execute("REVOKE INSERT ON users FROM test_user");
  EXPECT_TRUE(result.find("Privilege revoked successfully") != string::npos);

  // 4. 测试DROP USER命令
  result = executor.Execute("DROP USER test_user");
  EXPECT_TRUE(result.find("User dropped successfully") != string::npos);
}

// 测试更多DDL命令功能
TEST_F(SqlExecutorTest, TestAdvancedDDLCommands) {
  // 1. 测试CREATE TABLE带约束和默认值
  string result = executor.Execute(
      "CREATE TABLE test_constraints (" 
      "id INT PRIMARY KEY, " 
      "name VARCHAR(50) NOT NULL, " 
      "email VARCHAR(100) UNIQUE, " 
      "status VARCHAR(20) DEFAULT 'active', " 
      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)"
  );
  EXPECT_TRUE(result.find("CREATE executed") != string::npos);
  
  // 2. 测试ALTER TABLE命令
  // 添加列
  result = executor.Execute("ALTER TABLE test_constraints ADD COLUMN age INT");
  EXPECT_TRUE(result.find("ALTER executed successfully") != string::npos);
  
  // 修改列
  result = executor.Execute("ALTER TABLE test_constraints MODIFY COLUMN age INT NOT NULL");
  EXPECT_TRUE(result.find("ALTER executed successfully") != string::npos);
  
  // 删除列
  result = executor.Execute("ALTER TABLE test_constraints DROP COLUMN age");
  EXPECT_TRUE(result.find("ALTER executed successfully") != string::npos);
  
  // 3. 测试CREATE INDEX
  result = executor.Execute("CREATE INDEX idx_name ON test_constraints(name)");
  EXPECT_TRUE(result.find("CREATE INDEX executed") != string::npos);
  
  // 4. 测试CREATE VIEW
  result = executor.Execute(
      "CREATE VIEW test_view AS SELECT id, name FROM test_constraints"
  );
  EXPECT_TRUE(result.find("CREATE VIEW executed") != string::npos);
  
  // 5. 测试SHOW TABLES
  result = executor.Execute("SHOW TABLES");
  EXPECT_TRUE(result.find("Tables in database") != string::npos);
  EXPECT_TRUE(result.find("test_constraints") != string::npos);
  
  // 6. 测试SHOW CREATE TABLE
  result = executor.Execute("SHOW CREATE TABLE test_constraints");
  EXPECT_TRUE(result.find("CREATE TABLE test_constraints") != string::npos);
  
  // 7. 测试DROP命令
  result = executor.Execute("DROP VIEW test_view");
  EXPECT_TRUE(result.find("DROP VIEW executed") != string::npos);
  
  result = executor.Execute("DROP TABLE test_constraints");
  EXPECT_TRUE(result.find("Query OK") != string::npos);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}