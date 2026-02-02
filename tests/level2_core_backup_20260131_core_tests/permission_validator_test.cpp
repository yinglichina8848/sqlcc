/**
 * WHY: PermissionValidator是SQLCC的权限验证框架，负责解耦和可扩展的权限检查。
 * 为了保证数据库系统的安全性，需要全面测试权限验证功能：
 * 1. 基本权限检查（管理员权限覆盖）
 * 2. 回调机制验证
 * 3. 数据库上下文验证
 * 4. 用户存在性检查
 * 5. 权限操作到权限字符串的映射
 *
 * WHAT: PermissionValidator单元测试
 * 测试覆盖：
 * - 基本权限验证流程
 * - 回调注册和调用
 * - 默认用户和数据库设置
 * - 管理员权限覆盖
 * - 操作到权限字符串映射
 *
 * HOW: 使用Google Test框架，模拟UserManager依赖，
 * 测试PermissionValidator的核心验证逻辑。
 */

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <functional>

#include "src/core/permission_validator.h"
#include "src/core/user_manager.h"
#include "src/exception/exception.h"
#include "src/logger/logger.h"

namespace sqlcc {
namespace test {

// PermissionValidator测试 fixture
class PermissionValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<Logger>("PermissionValidatorTest");
        user_manager_ = std::make_unique<UserManager>(logger_);
        user_manager_->Initialize("");
        
        // 创建测试用户
        user_manager_->CreateUser("admin_user", "admin_pass", UserRole::ADMIN);
        user_manager_->CreateUser("normal_user", "user_pass", UserRole::USER);
        user_manager_->CreateUser("guest_user", "guest_pass", UserRole::GUEST);
        
        validator_ = std::make_shared<PermissionValidator>(user_manager_);
    }

    void TearDown() override {
        if (user_manager_) {
            user_manager_->Shutdown();
        }
    }

    std::shared_ptr<Logger> logger_;
    std::unique_ptr<UserManager> user_manager_;
    std::shared_ptr<PermissionValidator> validator_;
};

// ============ 基本验证测试 ============

TEST_F(PermissionValidatorTest, Validate_AdminHasAllPermissions) {
    // 测试管理员拥有所有权限
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "any_table",
        "admin_user",
        "any_database"
    );
    
    EXPECT_TRUE(result.allowed);
    EXPECT_EQ(result.message, "Permission granted");
}

TEST_F(PermissionValidatorTest, Validate_NoCallbackReturnsDeny) {
    // 测试未注册回调时返回拒绝
    PermissionValidator new_validator(user_manager_);
    
    auto result = new_validator.validate(
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        "test_database"
    );
    
    // 没有回调且非管理员，应该拒绝
    EXPECT_FALSE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_CallbackAllows) {
    // 测试回调允许访问
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createAllowed();
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        "test_database"
    );
    
    EXPECT_TRUE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_CallbackDenies) {
    // 测试回调拒绝访问
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createDenied("Access denied by callback");
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        "test_database"
    );
    
    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.message, "Access denied by callback");
}

// ============ 默认用户和数据库测试 ============

TEST_F(PermissionValidatorTest, SetAndGetDefaultUser) {
    // 测试默认用户设置
    validator_->setDefaultUser("normal_user");
    
    // 验证时不指定用户应该使用默认用户
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            if (context.user == "normal_user") {
                return PermissionResult::createAllowed();
            }
            return PermissionResult::createDenied("Wrong user");
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "",  // 不指定用户
        "test_database"
    );
    
    EXPECT_TRUE(result.allowed);
}

TEST_F(PermissionValidatorTest, SetAndGetDefaultDatabase) {
    // 测试默认数据库设置
    validator_->setDefaultDatabase("default_db");
    
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            if (context.database == "default_db") {
                return PermissionResult::createAllowed();
            }
            return PermissionResult::createDenied("Wrong database");
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        ""  // 不指定数据库
    );
    
    EXPECT_TRUE(result.allowed);
}

// ============ 用户存在性测试 ============

TEST_F(PermissionValidatorTest, UserExists_ExistingUser) {
    // 测试存在的用户
    EXPECT_TRUE(validator_->userExists("admin_user"));
    EXPECT_TRUE(validator_->userExists("normal_user"));
    EXPECT_TRUE(validator_->userExists("guest_user"));
}

TEST_F(PermissionValidatorTest, UserExists_NonExistingUser) {
    // 测试不存在的用户
    EXPECT_FALSE(validator_->userExists("nonexistent_user"));
    EXPECT_FALSE(validator_->userExists(""));
}

TEST_F(PermissionValidatorTest, UserExists_EmptyString) {
    // 测试空字符串
    EXPECT_FALSE(validator_->userExists(""));
}

// ============ 管理员检查测试 ============

TEST_F(PermissionValidatorTest, IsAdmin_AdminUser) {
    // 测试管理员用户
    EXPECT_TRUE(validator_->isAdmin("admin_user"));
}

TEST_F(PermissionValidatorTest, IsAdmin_NormalUser) {
    // 测试普通用户
    EXPECT_FALSE(validator_->isAdmin("normal_user"));
}

TEST_F(PermissionValidatorTest, IsAdmin_GuestUser) {
    // 测试访客用户
    EXPECT_FALSE(validator_->isAdmin("guest_user"));
}

TEST_F(PermissionValidatorTest, IsAdmin_NonExistingUser) {
    // 测试不存在的用户
    EXPECT_FALSE(validator_->isAdmin("nonexistent"));
}

// ============ 操作到权限字符串映射测试 ============

TEST_F(PermissionValidatorTest, OperationToPrivilege_Select) {
    // 测试SELECT操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::SELECT),
        "SELECT"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_Insert) {
    // 测试INSERT操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::INSERT),
        "INSERT"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_Update) {
    // 测试UPDATE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::UPDATE),
        "UPDATE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_Delete) {
    // 测试DELETE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::DELETE),
        "DELETE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_CreateDatabase) {
    // 测试CREATE_DATABASE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::CREATE_DATABASE),
        "CREATE DATABASE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_DropDatabase) {
    // 测试DROP_DATABASE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::DROP_DATABASE),
        "DROP DATABASE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_CreateTable) {
    // 测试CREATE_TABLE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::CREATE_TABLE),
        "CREATE TABLE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_DropTable) {
    // 测试DROP_TABLE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::DROP_TABLE),
        "DROP TABLE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_AlterTable) {
    // 测试ALTER_TABLE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::ALTER_TABLE),
        "ALTER TABLE"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_CreateUser) {
    // 测试CREATE_USER操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::CREATE_USER),
        "CREATE USER"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_DropUser) {
    // 测试DROP_USER操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::DROP_USER),
        "DROP USER"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_Grant) {
    // 测试GRANT操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::GRANT),
        "GRANT"
    );
}

TEST_F(PermissionValidatorTest, OperationToPrivilege_Revoke) {
    // 测试REVOKE操作映射
    EXPECT_EQ(
        PermissionValidator::operationToPrivilege(PermissionOperation::REVOKE),
        "REVOKE"
    );
}

// ============ 操作到资源类型映射测试 ============

TEST_F(PermissionValidatorTest, OperationToResourceType_DatabaseOperations) {
    // 测试数据库操作的资源类型
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::CREATE_DATABASE),
        "DATABASE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::DROP_DATABASE),
        "DATABASE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::USE_DATABASE),
        "DATABASE"
    );
}

TEST_F(PermissionValidatorTest, OperationToResourceType_TableOperations) {
    // 测试表操作的资源类型
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::CREATE_TABLE),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::DROP_TABLE),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::ALTER_TABLE),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::SELECT),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::INSERT),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::UPDATE),
        "TABLE"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::DELETE),
        "TABLE"
    );
}

TEST_F(PermissionValidatorTest, OperationToResourceType_UserOperations) {
    // 测试用户操作的资源类型
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::CREATE_USER),
        "USER"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::DROP_USER),
        "USER"
    );
}

TEST_F(PermissionValidatorTest, OperationToResourceType_SystemOperations) {
    // 测试系统操作的资源类型
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::GRANT),
        "SYSTEM"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::REVOKE),
        "SYSTEM"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::SHOW_DATABASES),
        "SYSTEM"
    );
    EXPECT_EQ(
        PermissionValidator::operationToResourceType(PermissionOperation::SHOW_TABLES),
        "SYSTEM"
    );
}

// ============ 上下文验证测试 ============

TEST_F(PermissionValidatorTest, Validate_DatabaseContext_Required) {
    // 测试某些操作需要数据库上下文
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            if (context.database.empty()) {
                return PermissionResult::createDenied("Database context required");
            }
            return PermissionResult::createAllowed();
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        ""  // 空数据库
    );
    
    EXPECT_FALSE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_DatabaseContext_NotRequired) {
    // 测试某些操作不需要数据库上下文
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createAllowed();
        }
    );
    
    // SHOW_DATABASES 不需要数据库上下文
    auto result = validator_->validate(
        PermissionOperation::SHOW_DATABASES,
        "",
        "normal_user",
        ""  // 空数据库
    );
    
    EXPECT_TRUE(result.allowed);
}

// ============ PermissionContext测试 ============

TEST_F(PermissionValidatorTest, PermissionContext_Constructor) {
    // 测试PermissionContext构造函数
    PermissionContext context("user1", "db1", "table1", PermissionOperation::SELECT);
    
    EXPECT_EQ(context.user, "user1");
    EXPECT_EQ(context.database, "db1");
    EXPECT_EQ(context.resource, "table1");
    EXPECT_EQ(context.operation, PermissionOperation::SELECT);
}

TEST_F(PermissionValidatorTest, PermissionContext_DefaultValues) {
    // 测试PermissionContext默认值
    PermissionContext context;
    
    EXPECT_EQ(context.user, "");
    EXPECT_EQ(context.database, "");
    EXPECT_EQ(context.resource, "");
    EXPECT_EQ(context.operation, PermissionOperation::SELECT);
}

// ============ PermissionResult测试 ============

TEST_F(PermissionValidatorTest, PermissionResult_CreateAllowed) {
    // 测试创建允许结果
    auto result = PermissionResult::createAllowed();
    
    EXPECT_TRUE(result.allowed);
    EXPECT_EQ(result.message, "Permission granted");
}

TEST_F(PermissionValidatorTest, PermissionResult_CreateDenied) {
    // 测试创建拒绝结果
    auto result = PermissionResult::createDenied("Test reason");
    
    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.message, "Test reason");
}

TEST_F(PermissionValidatorTest, PermissionResult_CustomMessage) {
    // 测试自定义消息
    ErrorInfo error(ErrorCode::PERMISSION_DENIED, ErrorLevel::ERROR, "Custom error", "Detail", "PERMISSION");
    auto result = PermissionResult::createDeniedWithError(error);
    
    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.message, "Custom error");
    EXPECT_EQ(result.error_info.error_code, ErrorCode::PERMISSION_DENIED);
}

// ============ 边界条件测试 ============

TEST_F(PermissionValidatorTest, Validate_EmptyResource) {
    // 测试空资源名
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createAllowed();
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SHOW_DATABASES,
        "",  // 空资源
        "normal_user",
        ""
    );
    
    EXPECT_TRUE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_EmptyUser) {
    // 测试空用户名
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createDenied("User required");
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "",  // 空用户
        "test_database"
    );
    
    EXPECT_FALSE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_NonExistingUser) {
    // 测试不存在的用户
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            if (!userExists(context.user)) {
                return PermissionResult::createDenied("User not found");
            }
            return PermissionResult::createAllowed();
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "nonexistent_user",
        "test_database"
    );
    
    EXPECT_FALSE(result.allowed);
}

TEST_F(PermissionValidatorTest, Validate_CallbackExceptionHandling) {
    // 测试回调异常处理
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            throw std::runtime_error("Callback exception");
        }
    );
    
    auto result = validator_->validate(
        PermissionOperation::SELECT,
        "test_table",
        "admin_user",  // 管理员应该能绕过回调异常
        "test_database"
    );
    
    // 管理员应该能访问，不受回调异常影响
    EXPECT_TRUE(result.allowed);
}

// ============ 宏测试 ============

TEST_F(PermissionValidatorTest, ValidatePermission_Macro) {
    // 测试VALIDATE_PERMISSION宏
    validator_->setPermissionCheckCallback(
        [](const PermissionContext& context) -> PermissionResult {
            return PermissionResult::createAllowed();
        }
    );
    
    auto result = VALIDATE_PERMISSION(
        *validator_,
        PermissionOperation::SELECT,
        "test_table",
        "normal_user",
        "test_database"
    );
    
    EXPECT_TRUE(result.allowed);
}

}  // namespace test
}  // namespace sqlcc
