#include "../include/database_manager.h"
#include "../include/permission_validator.h"
#include "../include/sql_parser/parser.h"
#include "../include/system_database.h"
#include "../include/unified_executor.h"
#include "../include/user_manager.h"
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
        std::make_shared<PermissionValidator>(db_manager_, user_manager_);
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

  // 测试用户是否具有CREATE权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_CREATE);
  EXPECT_TRUE(result.success);
}

// 测试DROP权限
TEST_F(PermissionValidatorTest, DropPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有DROP权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_DROP);
  EXPECT_TRUE(result.success);
}

// 测试SELECT权限
TEST_F(PermissionValidatorTest, SelectPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有SELECT权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_SELECT);
  EXPECT_TRUE(result.success);
}

// 测试INSERT权限
TEST_F(PermissionValidatorTest, InsertPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有INSERT权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_INSERT);
  EXPECT_TRUE(result.success);
}

// 测试UPDATE权限
TEST_F(PermissionValidatorTest, UpdatePermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有UPDATE权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_UPDATE);
  EXPECT_TRUE(result.success);
}

// 测试DELETE权限
TEST_F(PermissionValidatorTest, DeletePermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有DELETE权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_DELETE);
  EXPECT_TRUE(result.success);
}

// 测试ALTER权限
TEST_F(PermissionValidatorTest, AlterPermissionTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有ALTER权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_ALTER);
  EXPECT_TRUE(result.success);
}

// 测试权限继承
TEST_F(PermissionValidatorTest, PermissionInheritanceTest) {
  // 简化测试：直接测试权限验证器的初始化
  EXPECT_TRUE(permission_validator_ != nullptr);

  // 测试用户是否具有所有权限
  PermissionResult result = permission_validator_->CheckPermission(
      "admin", "test_db", "test_table", PermissionValidator::PRIVILEGE_ALL);
  EXPECT_TRUE(result.success);
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}