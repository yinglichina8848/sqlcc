#include "sql_executor.h"
#include <gtest/gtest.h>
#include <string>

namespace sqlcc {

class UnsupportedCommandsTest : public ::testing::Test {
protected:
  SqlExecutor executor;
};

// DCL命令测试
TEST_F(UnsupportedCommandsTest, AlterUserCommand) {
  std::string result =
      executor.Execute("ALTER USER test_user IDENTIFIED BY new_password;");
  EXPECT_EQ(result, "ERROR: Command not supported: ALTER USER");
}

TEST_F(UnsupportedCommandsTest, DropUserCommand) {
  std::string result = executor.Execute("DROP USER test_user;");
  EXPECT_EQ(result, "ERROR: Command not supported: DROP USER");
}

TEST_F(UnsupportedCommandsTest, CreateRoleCommand) {
  std::string result = executor.Execute("CREATE ROLE admin;");
  EXPECT_EQ(result, "ERROR: Command not supported: CREATE ROLE");
}

TEST_F(UnsupportedCommandsTest, DropRoleCommand) {
  std::string result = executor.Execute("DROP ROLE admin;");
  EXPECT_EQ(result, "ERROR: Command not supported: DROP ROLE");
}

TEST_F(UnsupportedCommandsTest, AlterRoleCommand) {
  std::string result =
      executor.Execute("ALTER ROLE admin SET password = 'new_pass';");
  EXPECT_EQ(result, "ERROR: Command not supported: ALTER ROLE");
}

TEST_F(UnsupportedCommandsTest, SetRoleCommand) {
  std::string result = executor.Execute("SET ROLE admin;");
  EXPECT_EQ(result, "ERROR: Command not supported: SET ROLE");
}

// DDL命令测试
TEST_F(UnsupportedCommandsTest, CreateViewCommand) {
  std::string result =
      executor.Execute("CREATE VIEW v1 AS SELECT * FROM users;");
  EXPECT_EQ(result, "ERROR: Command not supported: CREATE VIEW");
}

TEST_F(UnsupportedCommandsTest, DropViewCommand) {
  std::string result = executor.Execute("DROP VIEW v1;");
  EXPECT_EQ(result, "ERROR: Command not supported: DROP VIEW");
}

TEST_F(UnsupportedCommandsTest, AlterViewCommand) {
  std::string result =
      executor.Execute("ALTER VIEW v1 AS SELECT id FROM users;");
  EXPECT_EQ(result, "ERROR: Command not supported: ALTER VIEW");
}

TEST_F(UnsupportedCommandsTest, CreateSchemaCommand) {
  std::string result = executor.Execute("CREATE SCHEMA test_schema;");
  EXPECT_EQ(result, "ERROR: Command not supported: CREATE SCHEMA");
}

TEST_F(UnsupportedCommandsTest, DropSchemaCommand) {
  std::string result = executor.Execute("DROP SCHEMA test_schema;");
  EXPECT_EQ(result, "ERROR: Command not supported: DROP SCHEMA");
}

TEST_F(UnsupportedCommandsTest, AlterSchemaCommand) {
  std::string result =
      executor.Execute("ALTER SCHEMA test_schema RENAME TO new_schema;");
  EXPECT_EQ(result, "ERROR: Command not supported: ALTER SCHEMA");
}

TEST_F(UnsupportedCommandsTest, TruncateTableCommand) {
  std::string result = executor.Execute("TRUNCATE TABLE users;");
  EXPECT_EQ(result, "ERROR: Command not supported: TRUNCATE TABLE");
}

TEST_F(UnsupportedCommandsTest, RenameTableCommand) {
  std::string result = executor.Execute("RENAME TABLE users TO new_users;");
  EXPECT_EQ(result, "ERROR: Command not supported: RENAME TABLE");
}

// 支持的命令应该正常执行
TEST_F(UnsupportedCommandsTest, CreateUserCommand) {
  std::string result =
      executor.Execute("CREATE USER test_user IDENTIFIED BY password;");
  EXPECT_NE(result, "ERROR: Command not supported: CREATE USER");
}

TEST_F(UnsupportedCommandsTest, GrantCommand) {
  std::string result = executor.Execute("GRANT SELECT ON users TO test_user;");
  EXPECT_NE(result, "ERROR: Command not supported: GRANT");
}

TEST_F(UnsupportedCommandsTest, RevokeCommand) {
  std::string result =
      executor.Execute("REVOKE SELECT ON users FROM test_user;");
  EXPECT_NE(result, "ERROR: Command not supported: REVOKE");
}

TEST_F(UnsupportedCommandsTest, CreateTableCommand) {
  std::string result =
      executor.Execute("CREATE TABLE test_table (id INT, name VARCHAR(50));");
  EXPECT_EQ(result, "Query OK, 1 row affected");
}

TEST_F(UnsupportedCommandsTest, DropTableCommand) {
  std::string result = executor.Execute("DROP TABLE test_table;");
  EXPECT_EQ(result, "Query OK, 1 row affected");
}

TEST_F(UnsupportedCommandsTest, CreateIndexCommand) {
  std::string result =
      executor.Execute("CREATE INDEX idx_test ON test_table(id);");
  EXPECT_EQ(result, "Query OK, 1 row affected");
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}