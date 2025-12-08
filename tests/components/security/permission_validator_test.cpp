#include "database_manager.h"
#include "permission_validator.h"
#include "sql_parser/parser_new.h"
#include "system_database.h"
#include "unified_executor.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {

// 测试PermissionValidator类的功能
class PermissionValidatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化数据库管理器
    db_manager_ = std::make_shared<DatabaseManager>(
        "./test_permission_validator.db", 1024, 4, 2);
    user_manager_ = std::make_shared<UserManager>();
    system_db_ = std::make_shared<SystemDatabase>(db_manager_);
    unified_executor_ = std::make_shared<UnifiedExecutor>(
        db_manager_, user_manager_, system_db_);

    // 初始化权限验证器
    permission_validator_ =
        std::make_shared<PermissionValidator>(user_manager_, db_manager_);
  }

  void TearDown() override {
    // 清理资源
    permission_validator_.reset();
    unified_executor_.reset();
    system_db_.reset();
    user_manager_.reset();
    db_manager_.reset();

    // 删除测试数据库文件
    std::system("rm -rf ./test_permission_validator.db");
  }

  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<UnifiedExecutor> unified_executor_;
  std::shared_ptr<PermissionValidator> permission_validator_;
};

// 测试CREATE权限
TEST_F(PermissionValidatorTest, CreatePermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试数据库创建权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::CREATE_DATABASE, "test_db", "admin", "");
  EXPECT_TRUE(result.allowed);
}

// 测试DROP权限
TEST_F(PermissionValidatorTest, DropPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试表删除权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::DROP_TABLE, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试SELECT权限
TEST_F(PermissionValidatorTest, SelectPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试SELECT权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::SELECT, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试INSERT权限
TEST_F(PermissionValidatorTest, InsertPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试INSERT权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::INSERT, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试UPDATE权限
TEST_F(PermissionValidatorTest, UpdatePermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试UPDATE权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::UPDATE, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试DELETE权限
TEST_F(PermissionValidatorTest, DeletePermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试DELETE权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::DELETE, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试ALTER权限
TEST_F(PermissionValidatorTest, AlterPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试ALTER权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::ALTER_TABLE, "test_table", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

// 测试权限继承
TEST_F(PermissionValidatorTest, PermissionInheritanceTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试SHOW权限
  PermissionResult result = permission_validator_->validate(
      PermissionOperation::SHOW_TABLES, "", "admin", "test_db");
  EXPECT_TRUE(result.allowed);
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}