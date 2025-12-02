#include "permission_validator.h"
#include "sql_parser/ast_nodes.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

PermissionValidator::PermissionValidator(std::shared_ptr<UserManager> user_manager,
                                       std::shared_ptr<DatabaseManager> db_manager)
    : user_manager_(user_manager), db_manager_(db_manager) {
    // 设置默认用户和数据库
    default_user_ = "root"; // 默认管理员用户
    default_database_ = ""; // 默认无数据库
}

PermissionResult PermissionValidator::validate(PermissionOperation operation, 
                                              const std::string& resource,
                                              const std::string& current_user,
                                              const std::string& current_database) {
    std::string user = getCurrentUser(current_user);
    std::string database = getCurrentDatabase(current_database);
    
    // 检查用户是否存在
    // TODO: 需要实现用户存在性检查
    // 临时跳过用户存在性验证
    
    // 根据操作类型进行验证
    switch (operation) {
        case PermissionOperation::CREATE_DATABASE:
        case PermissionOperation::DROP_DATABASE:
        case PermissionOperation::USE_DATABASE:
        case PermissionOperation::SHOW_DATABASES:
            return validateDatabaseOperation(operation, resource, user, database);
            
        case PermissionOperation::CREATE_TABLE:
        case PermissionOperation::DROP_TABLE:
        case PermissionOperation::ALTER_TABLE:
        case PermissionOperation::SELECT:
        case PermissionOperation::INSERT:
        case PermissionOperation::UPDATE:
        case PermissionOperation::DELETE:
        case PermissionOperation::SHOW_TABLES:
            return validateTableOperation(operation, resource, user, database);
            
        case PermissionOperation::CREATE_USER:
        case PermissionOperation::DROP_USER:
        case PermissionOperation::GRANT:
        case PermissionOperation::REVOKE:
            return validateUserOperation(operation, resource, user, database);
            
        default:
            return validateUtilityOperation(operation, resource, user, database);
    }
}

PermissionResult PermissionValidator::validateStatement(std::unique_ptr<sql_parser::Statement> stmt,
                                                       const std::string& current_user,
                                                       const std::string& current_database) {
    std::string user = getCurrentUser(current_user);
    std::string database = getCurrentDatabase(current_database);
    
    // 根据语句类型进行权限验证
    if (auto create_stmt = dynamic_cast<sql_parser::CreateStatement*>(stmt.get())) {
        if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
            return validate(PermissionOperation::CREATE_DATABASE, create_stmt->getObjectName(), user, database);
        } else if (create_stmt->getObjectType() == sql_parser::CreateStatement::TABLE) {
            return validate(PermissionOperation::CREATE_TABLE, create_stmt->getObjectName(), user, database);
        }
    }
    else if (auto drop_stmt = dynamic_cast<sql_parser::DropStatement*>(stmt.get())) {
        if (drop_stmt->getObjectType() == sql_parser::DropStatement::DATABASE) {
            return validate(PermissionOperation::DROP_DATABASE, drop_stmt->getObjectName(), user, database);
        } else if (drop_stmt->getObjectType() == sql_parser::DropStatement::TABLE) {
            return validate(PermissionOperation::DROP_TABLE, drop_stmt->getObjectName(), user, database);
        }
    }
    else if (auto use_stmt = dynamic_cast<sql_parser::UseStatement*>(stmt.get())) {
        return validate(PermissionOperation::USE_DATABASE, use_stmt->getDatabaseName(), user, database);
    }
    else if (auto select_stmt = dynamic_cast<sql_parser::SelectStatement*>(stmt.get())) {
        // 验证SELECT权限，需要检查所有涉及的表
        // TODO: SelectStatement需要添加tables字段
        // 临时跳过表级权限验证
        return PermissionResult::createAllowed();
    }
    else if (auto insert_stmt = dynamic_cast<sql_parser::InsertStatement*>(stmt.get())) {
        return validate(PermissionOperation::INSERT, insert_stmt->getTableName(), user, database);
    }
    else if (auto update_stmt = dynamic_cast<sql_parser::UpdateStatement*>(stmt.get())) {
        return validate(PermissionOperation::UPDATE, update_stmt->getTableName(), user, database);
    }
    else if (auto delete_stmt = dynamic_cast<sql_parser::DeleteStatement*>(stmt.get())) {
        return validate(PermissionOperation::DELETE, delete_stmt->getTableName(), user, database);
    }
    // TODO: 需要实现ShowStatement类
    // 临时跳过SHOW语句的权限验证
    else if (auto create_user_stmt = dynamic_cast<sql_parser::CreateUserStatement*>(stmt.get())) {
        return validate(PermissionOperation::CREATE_USER, create_user_stmt->getUsername(), user, database);
    }
    else if (auto drop_user_stmt = dynamic_cast<sql_parser::DropUserStatement*>(stmt.get())) {
        return validate(PermissionOperation::DROP_USER, drop_user_stmt->getUsername(), user, database);
    }
    else if (auto grant_stmt = dynamic_cast<sql_parser::GrantStatement*>(stmt.get())) {
        return validate(PermissionOperation::GRANT, grant_stmt->getGrantee(), user, database);
    }
    else if (auto revoke_stmt = dynamic_cast<sql_parser::RevokeStatement*>(stmt.get())) {
        return validate(PermissionOperation::REVOKE, revoke_stmt->getGrantee(), user, database);
    }
    
    // 对于其他语句类型，默认允许执行
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

// 私有方法实现
PermissionResult PermissionValidator::validateDatabaseOperation(PermissionOperation operation, 
                                                               const std::string& resource,
                                                               const std::string& current_user,
                                                               const std::string& current_database) {
    std::string privilege = operationToPrivilege(operation);
    
    // 检查用户权限
    if (!checkUserPermission(current_user, current_database, resource, privilege)) {
        std::stringstream ss;
        ss << "User '" << current_user << "' lacks " << privilege 
           << " permission on " << operationToResourceType(operation);
        if (!resource.empty()) {
            ss << " '" << resource << "'";
        }
        
        return PermissionResult::createDeniedWithError(
            ErrorHandler::getInstance().createError(
                ErrorCode::PERMISSION_DENIED, 
                ErrorLevel::ERROR, 
                ss.str(),
                "PERMISSION",
                "PermissionValidator"
            )
        );
    }
    
    // 特定操作验证
    switch (operation) {
        case PermissionOperation::CREATE_DATABASE:
            if (!resource.empty() && db_manager_->DatabaseExists(resource)) {
                return PermissionResult::createDeniedWithError(
                    ErrorHandler::getInstance().createError(
                        ErrorCode::DATABASE_ALREADY_EXISTS, 
                        ErrorLevel::ERROR, 
                        "Database '" + resource + "' already exists",
                        "PERMISSION",
                        "PermissionValidator"
                    )
                );
            }
            break;
            
        case PermissionOperation::DROP_DATABASE:
            if (!resource.empty() && !db_manager_->DatabaseExists(resource)) {
                return PermissionResult::createDeniedWithError(
                    ErrorHandler::getInstance().createError(
                        ErrorCode::DATABASE_NOT_EXIST, 
                        ErrorLevel::ERROR, 
                        "Database '" + resource + "' does not exist",
                        "PERMISSION",
                        "PermissionValidator"
                    )
                );
            }
            break;
            
        case PermissionOperation::USE_DATABASE:
            if (!resource.empty() && !db_manager_->DatabaseExists(resource)) {
                return PermissionResult::createDeniedWithError(
                    ErrorHandler::getInstance().createError(
                        ErrorCode::DATABASE_NOT_EXIST, 
                        ErrorLevel::ERROR, 
                        "Database '" + resource + "' does not exist",
                        "PERMISSION",
                        "PermissionValidator"
                    )
                );
            }
            break;
            
        default:
            break;
    }
    
    return PermissionResult::createAllowed();
}

PermissionResult PermissionValidator::validateTableOperation(PermissionOperation operation, 
                                                            const std::string& resource,
                                                            const std::string& current_user,
                                                            const std::string& current_database) {
    std::string privilege = operationToPrivilege(operation);
    
    // 检查数据库上下文
    if (current_database.empty() && hasDatabaseContext(operation)) {
        return PermissionResult::createDeniedWithError(
            ErrorHandler::getInstance().createError(
                ErrorCode::INVALID_PARAMETER, 
                ErrorLevel::ERROR, 
                "No database selected",
                "PERMISSION",
                "PermissionValidator"
            )
        );
    }
    
    // 检查用户权限
    if (!checkUserPermission(current_user, current_database, resource, privilege)) {
        std::stringstream ss;
        ss << "User '" << current_user << "' lacks " << privilege 
           << " permission on table '" << resource << "' in database '" << current_database << "'";
        
        return PermissionResult::createDeniedWithError(
            ErrorHandler::getInstance().createError(
                ErrorCode::PERMISSION_DENIED, 
                ErrorLevel::ERROR, 
                ss.str(),
                "PERMISSION",
                "PermissionValidator"
            )
        );
    }
    
    // 特定操作验证
    switch (operation) {
        case PermissionOperation::CREATE_TABLE:
            if (!resource.empty() && db_manager_->TableExists(resource)) {
                return PermissionResult::createDeniedWithError(
                    ErrorHandler::getInstance().createError(
                        ErrorCode::TABLE_ALREADY_EXISTS, 
                        ErrorLevel::ERROR, 
                        "Table '" + resource + "' already exists",
                        "PERMISSION",
                        "PermissionValidator"
                    )
                );
            }
            break;
            
        case PermissionOperation::DROP_TABLE:
            if (!resource.empty() && !db_manager_->TableExists(resource)) {
                return PermissionResult::createDeniedWithError(
                    ErrorHandler::getInstance().createError(
                        ErrorCode::TABLE_NOT_EXIST, 
                        ErrorLevel::ERROR, 
                        "Table '" + resource + "' does not exist",
                        "PERMISSION",
                        "PermissionValidator"
                    )
                );
            }
            break;
            
        default:
            break;
    }
    
    return PermissionResult::createAllowed();
}

PermissionResult PermissionValidator::validateUserOperation(PermissionOperation operation, 
                                                           const std::string& resource,
                                                           const std::string& current_user,
                                                           const std::string& current_database) {
    std::string privilege = operationToPrivilege(operation);
    
    // 检查用户权限
    if (!checkUserPermission(current_user, current_database, resource, privilege)) {
        std::stringstream ss;
        ss << "User '" << current_user << "' lacks " << privilege 
           << " permission on user '" << resource << "'";
        
        return PermissionResult::createDeniedWithError(
            ErrorHandler::getInstance().createError(
                ErrorCode::PERMISSION_DENIED, 
                ErrorLevel::ERROR, 
                ss.str(),
                "PERMISSION",
                "PermissionValidator"
            )
        );
    }
    
    // 特定操作验证
    switch (operation) {
        case PermissionOperation::CREATE_USER:
            // TODO: 需要实现用户存在性检查
            // 临时跳过用户存在性验证
            break;
            
        case PermissionOperation::DROP_USER:
            // TODO: 需要实现用户存在性检查
            // 临时跳过用户存在性验证
            break;
            
        default:
            break;
    }
    
    return PermissionResult::createAllowed();
}

PermissionResult PermissionValidator::validateUtilityOperation(PermissionOperation operation, 
                                                              const std::string& resource,
                                                              const std::string& current_user,
                                                              const std::string& current_database) {
    // 工具操作通常不需要特殊权限验证
    return PermissionResult::createAllowed();
}

std::string PermissionValidator::getCurrentUser(const std::string& user) const {
    return user.empty() ? default_user_ : user;
}

std::string PermissionValidator::getCurrentDatabase(const std::string& database) const {
    return database.empty() ? default_database_ : database;
}

bool PermissionValidator::hasDatabaseContext(PermissionOperation operation) const {
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

bool PermissionValidator::checkUserPermission(const std::string& user, 
                                             const std::string& database, 
                                             const std::string& resource, 
                                             const std::string& privilege) {
    // 使用UserManager的权限检查功能
    
    // 管理员用户拥有所有权限
    if (user == "root" || user == "admin") {
        return true;
    }
    
    // 检查用户是否拥有特定权限
    // 使用UserManager的CheckPermission方法
    // 注意：这里需要将权限字符串映射到UserManager的权限常量
    std::string actual_privilege = privilege;
    if (privilege == "CREATE_DATABASE" || privilege == "CREATE_TABLE") {
        actual_privilege = UserManager::PRIVILEGE_CREATE;
    } else if (privilege == "DROP_DATABASE" || privilege == "DROP_TABLE") {
        actual_privilege = UserManager::PRIVILEGE_DROP;
    } else if (privilege == "ALTER_TABLE") {
        actual_privilege = UserManager::PRIVILEGE_ALTER;
    } else if (privilege == "SELECT") {
        actual_privilege = UserManager::PRIVILEGE_SELECT;
    } else if (privilege == "INSERT") {
        actual_privilege = UserManager::PRIVILEGE_INSERT;
    } else if (privilege == "UPDATE") {
        actual_privilege = UserManager::PRIVILEGE_UPDATE;
    } else if (privilege == "DELETE") {
        actual_privilege = UserManager::PRIVILEGE_DELETE;
    }
    
    return user_manager_->CheckPermission(user, database, resource, actual_privilege);
}

} // namespace sqlcc