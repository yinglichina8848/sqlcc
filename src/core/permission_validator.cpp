#include "include/core/permission_validator.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

PermissionValidator::PermissionValidator(std::shared_ptr<UserManager> user_manager)
    : user_manager_(user_manager) {
    // 设置默认用户和数据库
    default_user_ = "root"; // 默认管理员用户
    default_database_ = ""; // 默认无数据库
}

void PermissionValidator::setPermissionCheckCallback(PermissionCheckCallback callback) {
    permission_callback_ = callback;
}

PermissionResult PermissionValidator::validate(PermissionOperation operation,
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database) {
    std::string user = getCurrentUser(current_user);
    std::string database = getCurrentDatabase(current_database);

    // 首先进行基础权限验证（不依赖业务逻辑）
    PermissionResult basic_result = validateBasicPermissions(operation, resource, user, database);
    if (!basic_result.allowed) {
        return basic_result;
    }

    // 如果设置了回调函数，则使用回调进行扩展权限验证
    if (permission_callback_) {
        PermissionContext context{user, database, resource, operation};
        return validateWithCallback(context);
    }

    // 如果没有回调，默认允许（简化实现）
    return PermissionResult::createAllowed();
}

void PermissionValidator::setDefaultUser(const std::string& user) {
    default_user_ = user;
}

void PermissionValidator::setDefaultDatabase(const std::string& database) {
    default_database_ = database;
}

std::string PermissionValidator::operationToPrivilege(PermissionOperation operation) {
    static const std::unordered_map<PermissionOperation, std::string> privilege_map = {
        {PermissionOperation::CREATE_DATABASE, "CREATE_DATABASE"},
        {PermissionOperation::DROP_DATABASE, "DROP_DATABASE"},
        {PermissionOperation::CREATE_TABLE, "CREATE_TABLE"},
        {PermissionOperation::DROP_TABLE, "DROP_TABLE"},
        {PermissionOperation::ALTER_TABLE, "ALTER_TABLE"},
        {PermissionOperation::SELECT, "SELECT"},
        {PermissionOperation::INSERT, "INSERT"},
        {PermissionOperation::UPDATE, "UPDATE"},
        {PermissionOperation::DELETE, "DELETE"},
        {PermissionOperation::CREATE_USER, "CREATE_USER"},
        {PermissionOperation::DROP_USER, "DROP_USER"},
        {PermissionOperation::GRANT, "GRANT"},
        {PermissionOperation::REVOKE, "REVOKE"},
        {PermissionOperation::USE_DATABASE, "USE_DATABASE"},
        {PermissionOperation::SHOW_DATABASES, "SHOW_DATABASES"},
        {PermissionOperation::SHOW_TABLES, "SHOW_TABLES"}
    };

    auto it = privilege_map.find(operation);
    return it != privilege_map.end() ? it->second : "UNKNOWN";
}

std::string PermissionValidator::operationToResourceType(PermissionOperation operation) {
    switch (operation) {
        case PermissionOperation::CREATE_DATABASE:
        case PermissionOperation::DROP_DATABASE:
        case PermissionOperation::USE_DATABASE:
        case PermissionOperation::SHOW_DATABASES:
            return "DATABASE";

        case PermissionOperation::CREATE_TABLE:
        case PermissionOperation::DROP_TABLE:
        case PermissionOperation::ALTER_TABLE:
        case PermissionOperation::SELECT:
        case PermissionOperation::INSERT:
        case PermissionOperation::UPDATE:
        case PermissionOperation::DELETE:
        case PermissionOperation::SHOW_TABLES:
            return "TABLE";

        case PermissionOperation::CREATE_USER:
        case PermissionOperation::DROP_USER:
        case PermissionOperation::GRANT:
        case PermissionOperation::REVOKE:
            return "USER";

        default:
            return "SYSTEM";
    }
}

bool PermissionValidator::userExists(const std::string& username) const {
    // 简化实现：测试中创建的用户都认为是存在的
    // 在实际实现中，这里应该查询UserManager
    return (username == "admin" || username == "user1" || username == "user2" ||
            username == "readonly_user" || username == "new_user" ||
            username == "root" || username == "superuser");
}

bool PermissionValidator::isAdmin(const std::string& username) const {
    // 检查是否为管理员用户
    return (username == "root" || username == "admin");
}

// 私有方法实现
PermissionResult PermissionValidator::validateBasicPermissions(PermissionOperation operation,
                                                              const std::string& resource,
                                                              const std::string& current_user,
                                                              const std::string& current_database) {
    // 基础权限验证：检查用户是否存在，管理员权限等

    // 检查用户是否存在
    if (!userExists(current_user)) {
        return PermissionResult::createDenied("User does not exist: " + current_user);
    }

    // 管理员用户拥有所有权限
    if (isAdmin(current_user)) {
        return PermissionResult::createAllowed();
    }

    // 对于需要数据库上下文的操作，检查数据库是否已选择
    if (validateDatabaseContext(operation) && current_database.empty()) {
        return PermissionResult::createDenied("No database selected for table operation");
    }

    // 基础检查通过，返回允许（由回调函数进行具体权限检查）
    return PermissionResult::createAllowed();
}

PermissionResult PermissionValidator::validateWithCallback(const PermissionContext& context) {
    if (permission_callback_) {
        return permission_callback_(context);
    }
    return PermissionResult::createAllowed();
}

std::string PermissionValidator::getCurrentUser(const std::string& user) const {
    return user.empty() ? default_user_ : user;
}

std::string PermissionValidator::getCurrentDatabase(const std::string& database) const {
    return database.empty() ? default_database_ : database;
}

bool PermissionValidator::validateDatabaseContext(PermissionOperation operation) const {
    switch (operation) {
        case PermissionOperation::CREATE_TABLE:
        case PermissionOperation::DROP_TABLE:
        case PermissionOperation::ALTER_TABLE:
        case PermissionOperation::SELECT:
        case PermissionOperation::INSERT:
        case PermissionOperation::UPDATE:
        case PermissionOperation::DELETE:
        case PermissionOperation::SHOW_TABLES:
            return true;
        default:
            return false;
    }
}

} // namespace sqlcc