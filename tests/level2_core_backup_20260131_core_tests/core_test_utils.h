/**
 * @file core_test_utils.h
 * @brief Core module test utilities and helper functions.
 *
 * WHY: 提供核心模块测试的通用工具类和辅助函数，
 * 减少测试代码重复，提高测试可维护性。
 *
 * WHAT: 核心模块测试工具
 *
 * HOW: 使用单例模式和工厂模式提供测试辅助功能。
 */

#ifndef SQLCC_CORE_TEST_UTILS_H
#define SQLCC_CORE_TEST_UTILS_H

#include <memory>
#include <string>
#include <functional>

#include "src/core/user_manager.h"
#include "src/core/permission_validator.h"
#include "src/core/execution_context.h"
#include "src/logger/logger.h"

namespace sqlcc {
namespace test {
namespace core_test_utils {

/**
 * @brief 获取共享的测试日志器
 */
std::shared_ptr<Logger> GetTestLogger(const std::string& test_name);

/**
 * @brief 创建带有测试用户的UserManager
 */
std::unique_ptr<UserManager> CreateTestUserManager(
    std::shared_ptr<Logger> logger,
    const std::string& admin_password = "admin_pass",
    const std::string& user_password = "user_pass",
    const std::string& guest_password = "guest_pass"
);

/**
 * @brief 创建PermissionValidator并设置默认回调
 */
std::shared_ptr<PermissionValidator> CreateTestPermissionValidator(
    std::unique_ptr<UserManager> user_manager,
    std::function<bool(const PermissionContext&)> allow_callback = nullptr
);

/**
 * @brief 创建测试ExecutionContext
 */
std::unique_ptr<ExecutionContext> CreateTestExecutionContext(
    const std::string& user = "test_user",
    const std::string& database = "test_database",
    bool transactional = false
);

/**
 * @brief 测试用户数据生成器
 */
struct TestUserData {
    std::string username;
    std::string password;
    UserRole role;
    
    static TestUserData CreateAdmin(const std::string& name = "admin") {
        return {name, "admin_pass", UserRole::ADMIN};
    }
    
    static TestUserData CreateNormalUser(const std::string& name = "user") {
        return {name, "user_pass", UserRole::USER};
    }
    
    static TestUserData CreateGuest(const std::string& name = "guest") {
        return {name, "guest_pass", UserRole::GUEST};
    }
};

/**
 * @brief 生成测试用的唯一用户名
 */
std::string GenerateUniqueUsername(const std::string& prefix = "test_user");

/**
 * @brief 生成测试用的唯一数据库名
 */
std::string GenerateUniqueDatabaseName(const std::string& prefix = "test_db");

}  // namespace core_test_utils
}  // namespace test
}  // namespace sqlcc

#endif  // SQLCC_CORE_TEST_UTILS_H
