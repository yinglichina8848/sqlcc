/**
 * @file core_test_utils.cpp
 * @brief Core module test utilities implementation.
 */

#include "core_test_utils.h"
#include <chrono>
#include <random>

namespace sqlcc {
namespace test {
namespace core_test_utils {

static std::shared_ptr<Logger> shared_logger_ = nullptr;

std::shared_ptr<Logger> GetTestLogger(const std::string& test_name) {
    if (!shared_logger_) {
        shared_logger_ = std::make_shared<Logger>("CoreTest_" + test_name);
    }
    return shared_logger_;
}

std::unique_ptr<UserManager> CreateTestUserManager(
    std::shared_ptr<Logger> logger,
    const std::string& admin_password,
    const std::string& user_password,
    const std::string& guest_password
) {
    auto user_manager = std::make_unique<UserManager>(logger);
    user_manager->Initialize("");
    
    // 创建测试用户
    user_manager->CreateUser("admin_test", admin_password, UserRole::ADMIN);
    user_manager->CreateUser("normal_test", user_password, UserRole::USER);
    user_manager->CreateUser("guest_test", guest_password, UserRole::GUEST);
    
    return user_manager;
}

std::shared_ptr<PermissionValidator> CreateTestPermissionValidator(
    std::unique_ptr<UserManager> user_manager,
    std::function<bool(const PermissionContext&)> allow_callback
) {
    auto validator = std::make_shared<PermissionValidator>(std::move(user_manager));
    
    if (allow_callback) {
        validator->setPermissionCheckCallback(
            [allow_callback](const PermissionContext& context) -> PermissionResult {
                if (allow_callback(context)) {
                    return PermissionResult::createAllowed();
                }
                return PermissionResult::createDenied("Test denial");
            }
        );
    }
    
    return validator;
}

std::unique_ptr<ExecutionContext> CreateTestExecutionContext(
    const std::string& user,
    const std::string& database,
    bool transactional
) {
    return std::make_unique<ExecutionContext>(user, database, transactional);
}

std::string GenerateUniqueUsername(const std::string& prefix) {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return prefix + "_" + std::to_string(ms);
}

std::string GenerateUniqueDatabaseName(const std::string& prefix) {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return prefix + "_" + std::to_string(ms);
}

}  // namespace core_test_utils
}  // namespace test
}  // namespace sqlcc
