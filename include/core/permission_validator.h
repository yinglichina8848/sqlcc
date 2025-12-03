#ifndef SQLCC_PERMISSION_VALIDATOR_H
#define SQLCC_PERMISSION_VALIDATOR_H

#include "user_manager.h"
#include "database_manager.h"
#include "error_handler.h"
#include "sql_parser/ast_nodes.h"
#include <memory>
#include <string>
#include <unordered_map>

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
 * @brief 统一权限验证器
 * 
 * 提供统一的权限验证接口，解决各执行器权限检查逻辑不一致的问题
 */
class PermissionValidator {
public:
    PermissionValidator(std::shared_ptr<UserManager> user_manager,
                       std::shared_ptr<DatabaseManager> db_manager);
    
    ~PermissionValidator() = default;
    
    /**
     * @brief 验证权限
     */
    PermissionResult validate(PermissionOperation operation, 
                             const std::string& resource = "",
                             const std::string& current_user = "",
                             const std::string& current_database = "");
    
    /**
     * @brief 验证SQL语句权限
     */
    PermissionResult validateStatement(std::unique_ptr<sql_parser::Statement> stmt,
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

private:
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::string default_user_;
    std::string default_database_;
    
    // 权限验证方法
    PermissionResult validateDatabaseOperation(PermissionOperation operation, 
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database);
    
    PermissionResult validateTableOperation(PermissionOperation operation, 
                                           const std::string& resource,
                                           const std::string& current_user,
                                           const std::string& current_database);
    
    PermissionResult validateUserOperation(PermissionOperation operation, 
                                          const std::string& resource,
                                          const std::string& current_user,
                                          const std::string& current_database);
    
    PermissionResult validateUtilityOperation(PermissionOperation operation, 
                                             const std::string& resource,
                                             const std::string& current_user,
                                             const std::string& current_database);
    
    // 辅助方法
    std::string getCurrentUser(const std::string& user) const;
    std::string getCurrentDatabase(const std::string& database) const;
    bool hasDatabaseContext(PermissionOperation operation) const;
    
    // 权限检查
    bool checkUserPermission(const std::string& user, 
                            const std::string& database, 
                            const std::string& resource, 
                            const std::string& privilege);
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