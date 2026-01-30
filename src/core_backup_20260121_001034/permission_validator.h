#ifndef SQLCC_PERMISSION_VALIDATOR_H
#define SQLCC_PERMISSION_VALIDATOR_H

#include "src/core_backup_20260121_001034/user_manager.h"
#include "../error_handler.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace sqlcc {

/**
 * @brief 权限验证操作类型
 */
enum class PermissionOperation {
    CREATE_DATABASE,
    DROP_DATABASE,
    CREATE_TABLE,
    DROP_TABLE,
    ALTER_TABLE,
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE_USER,
    DROP_USER,
    GRANT,
    REVOKE,
    USE_DATABASE,
    SHOW_DATABASES,
    SHOW_TABLES
};

/**
 * @brief 权限验证结果
 */
struct PermissionResult {
    bool allowed;
    std::string message;
    ErrorInfo error_info;

    PermissionResult(bool allowed, const std::string& msg = "", const ErrorInfo& error = ErrorInfo(ErrorCode::SUCCESS, ErrorLevel::INFO, "", "", "PERMISSION"))
        : allowed(allowed), message(msg), error_info(error) {}

    static PermissionResult createAllowed() {
        return PermissionResult(true, "Permission granted");
    }

    static PermissionResult createDenied(const std::string& reason) {
        return PermissionResult(false, reason);
    }

    static PermissionResult createDeniedWithError(const ErrorInfo& error) {
        return PermissionResult(false, error.message, error);
    }
};

/**
 * @brief 权限上下文
 */
struct PermissionContext {
    std::string user;
    std::string database;
    std::string resource;
    PermissionOperation operation;

    PermissionContext(const std::string& u = "", const std::string& db = "", const std::string& res = "", PermissionOperation op = PermissionOperation::SELECT)
        : user(u), database(db), resource(res), operation(op) {}
};

/**
 * @brief 权限验证回调接口
 */
using PermissionCheckCallback = std::function<PermissionResult(const PermissionContext&)>;

/**
 * @brief 统一权限验证器 (独立设计)
 *
 * 核心模块的权限验证器，不依赖任何业务模块。
 * 通过回调函数接口实现与业务逻辑的解耦。
 */
class PermissionValidator {
public:
    PermissionValidator(std::shared_ptr<UserManager> user_manager);

    ~PermissionValidator() = default;

    /**
     * @brief 设置权限检查回调
     */
    void setPermissionCheckCallback(PermissionCheckCallback callback);

    /**
     * @brief 验证权限
     */
    PermissionResult validate(PermissionOperation operation,
                             const std::string& resource = "",
                             const std::string& current_user = "",
                             const std::string& current_database = "");

    /**
     * @brief 设置默认用户
     */
    void setDefaultUser(const std::string& user);

    /**
     * @brief 设置默认数据库
     */
    void setDefaultDatabase(const std::string& database);

    /**
     * @brief 获取权限映射
     */
    static std::string operationToPrivilege(PermissionOperation operation);

    /**
     * @brief 获取资源类型
     */
    static std::string operationToResourceType(PermissionOperation operation);

    /**
     * @brief 检查用户是否存在
     */
    bool userExists(const std::string& username) const;

    /**
     * @brief 检查用户是否为管理员
     */
    bool isAdmin(const std::string& username) const;

private:
    std::shared_ptr<UserManager> user_manager_;
    PermissionCheckCallback permission_callback_;
    std::string default_user_;
    std::string default_database_;

    // 基础权限验证（不依赖业务逻辑）
    PermissionResult validateBasicPermissions(PermissionOperation operation,
                                             const std::string& resource,
                                             const std::string& current_user,
                                             const std::string& current_database);

    // 使用回调进行扩展权限验证
    PermissionResult validateWithCallback(const PermissionContext& context);

    // 检查操作是否需要数据库上下文
    bool validateDatabaseContext(PermissionOperation operation) const;

    // 辅助方法
    std::string getCurrentUser(const std::string& user) const;
    std::string getCurrentDatabase(const std::string& database) const;
};

/**
 * @brief 权限验证辅助宏
 */
#define VALIDATE_PERMISSION(validator, operation, resource, user, database) \
    validator.validate(operation, resource, user, database)

#define VALIDATE_STATEMENT(validator, stmt, user, database) \
    validator.validateStatement(std::move(stmt), user, database)

} // namespace sqlcc

#endif // SQLCC_PERMISSION_VALIDATOR_H
