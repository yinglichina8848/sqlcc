#include "database_manager.h"
#include "sql_parser/parser.h"
#include "system_database.h"
#include "unified_executor.h"
#include "user_manager.h"
#include <gtest/gtest.h>

using namespace sqlcc;

class PermissionValidationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 创建测试数据库管理器
    db_manager_ = std::make_shared<DatabaseManager>();

    // 创建测试用户管理器，使用唯一临时数据路径
    temp_data_path_ =
        "./test_data/" +
        std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count());
    user_manager_ = std::make_shared<UserManager>(temp_data_path_);

    // 创建测试系统数据库
    system_db_ = std::make_shared<SystemDatabase>(db_manager_);

    // 创建测试执行上下文
    context_ = std::make_shared<ExecutionContext>(db_manager_, user_manager_,
                                                  system_db_);

    // 初始化用户和权限
    setupTestUsersAndPermissions();
  }

  void TearDown() override {
    // 清理临时数据目录
    std::filesystem::remove_all(temp_data_path_);
  }

  void setupTestUsersAndPermissions() {
    // 创建测试用户
    bool admin_created = user_manager_->CreateUser("admin_user", "password",
                                                   UserManager::ROLE_SUPERUSER);
    EXPECT_TRUE(admin_created)
        << "Failed to create admin_user: " << user_manager_->GetLastError();

    bool normal_created = user_manager_->CreateUser("normal_user", "password",
                                                    UserManager::ROLE_USER);
    EXPECT_TRUE(normal_created)
        << "Failed to create normal_user: " << user_manager_->GetLastError();

    // 为normal_user授予SELECT权限
    user_manager_->GrantPrivilege("normal_user", "test_db", "test_table",
                                  UserManager::PRIVILEGE_SELECT);

    // 为normal_user授予INSERT权限
    user_manager_->GrantPrivilege("normal_user", "test_db", "test_table",
                                  UserManager::PRIVILEGE_INSERT);

    // 不为normal_user授予DROP权限
    // user_manager_->GrantPrivilege("normal_user", "test_db", "test_table",
    // UserManager::PRIVILEGE_DROP);
  }

  // 辅助方法：解析SQL语句
  std::vector<std::unique_ptr<sql_parser::Statement>>
  parseSQL(const std::string &sql) {
    sql_parser::Parser parser(sql);
    return parser.parseStatements();
  }

  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<ExecutionContext> context_;
  std::string temp_data_path_;
};

TEST_F(PermissionValidationTest, AdminUserHasAllPermissions) {
  // 设置上下文为admin用户
  context_->current_user = "admin_user";
  context_->current_database = "test_db";

  // 创建测试语句：DROP TABLE
  auto stmt = parseSQL("DROP TABLE test_table");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DDLExecutionStrategy strategy;

  // 验证admin用户有DROP权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, NormalUserHasSelectPermission) {
  // 设置上下文为normal_user用户
  context_->current_user = "normal_user";
  context_->current_database = "test_db";

  // 创建测试语句：SELECT
  auto stmt = parseSQL("SELECT * FROM test_table");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DMLExecutionStrategy strategy;

  // 验证normal_user用户有SELECT权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, NormalUserHasInsertPermission) {
  // 设置上下文为normal_user用户
  context_->current_user = "normal_user";
  context_->current_database = "test_db";

  // 创建测试语句：INSERT
  auto stmt = parseSQL("INSERT INTO test_table (id, name) VALUES (1, 'test')");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DMLExecutionStrategy strategy;

  // 验证normal_user用户有INSERT权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, NormalUserNoDropPermission) {
  // 设置上下文为normal_user用户
  context_->current_user = "normal_user";
  context_->current_database = "test_db";

  // 创建测试语句：DROP TABLE
  auto stmt = parseSQL("DROP TABLE test_table");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DDLExecutionStrategy strategy;

  // 验证normal_user用户没有DROP权限
  EXPECT_FALSE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, UnknownUserHasNoPermissions) {
  // 设置上下文为未知用户
  context_->current_user = "unknown_user";
  context_->current_database = "test_db";

  // 创建测试语句：SELECT
  auto stmt = parseSQL("SELECT * FROM test_table");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DMLExecutionStrategy strategy;

  // 验证未知用户没有SELECT权限
  EXPECT_FALSE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, CheckCreatePermission) {
  // 设置上下文为admin用户
  context_->current_user = "admin_user";
  context_->current_database = "test_db";

  // 创建测试语句：CREATE TABLE
  auto stmt = parseSQL(
      "CREATE TABLE test_table (id INT PRIMARY KEY, name VARCHAR(255))");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DDLExecutionStrategy strategy;

  // 验证admin用户有CREATE权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));

  // 切换到normal_user用户
  context_->current_user = "normal_user";

  // 验证normal_user用户没有CREATE权限
  EXPECT_FALSE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, CheckUpdatePermission) {
  // 设置上下文为normal_user用户
  context_->current_user = "normal_user";
  context_->current_database = "test_db";

  // 创建测试语句：UPDATE
  auto stmt = parseSQL("UPDATE test_table SET name = 'new_name' WHERE id = 1");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DMLExecutionStrategy strategy;

  // 验证normal_user用户没有UPDATE权限（未授予）
  EXPECT_FALSE(strategy.checkPermission(stmt[0].get(), *context_));

  // 授予UPDATE权限
  user_manager_->GrantPrivilege("normal_user", "test_db", "test_table",
                                UserManager::PRIVILEGE_UPDATE);

  // 验证normal_user用户现在有UPDATE权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));
}

TEST_F(PermissionValidationTest, CheckDeletePermission) {
  // 设置上下文为normal_user用户
  context_->current_user = "normal_user";
  context_->current_database = "test_db";

  // 创建测试语句：DELETE
  auto stmt = parseSQL("DELETE FROM test_table WHERE id = 1");
  ASSERT_FALSE(stmt.empty());

  // 创建执行策略
  DMLExecutionStrategy strategy;

  // 验证normal_user用户没有DELETE权限（未授予）
  EXPECT_FALSE(strategy.checkPermission(stmt[0].get(), *context_));

  // 授予DELETE权限
  user_manager_->GrantPrivilege("normal_user", "test_db", "test_table",
                                UserManager::PRIVILEGE_DELETE);

  // 验证normal_user用户现在有DELETE权限
  EXPECT_TRUE(strategy.checkPermission(stmt[0].get(), *context_));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}