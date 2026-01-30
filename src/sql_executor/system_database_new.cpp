#include "core_backup_20260121_001034/system_database.h"
#include "system_schema_manager.h"
#include "system_data_initializer.h"
#include "system_permission_manager.h"
#include "system_metadata_manager.h"
#include "system_data_structures.h"
#include <iostream>
#include <memory>

namespace sqlcc {

SystemDatabase::SystemDatabase(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager), is_initialized_(false) {
}

SystemDatabase::~SystemDatabase() {
}

bool SystemDatabase::Initialize() {
    try {
        // 检查系统数据库是否已存在
        if (!Exists()) {
            // 创建系统数据库
            if (!db_manager_->CreateDatabase(SYSTEM_DB_NAME)) {
                last_error_ = "Failed to create system database";
                return false;
            }
        }
        
        // 切换到系统数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            last_error_ = "Failed to use system database";
            return false;
        }
        
        // 创建系统表管理器
        schema_manager_ = std::make_unique<SystemSchemaManager>(db_manager_);
        
        // 创建所有系统表
        if (!schema_manager_->CreateAllSystemTables()) {
            last_error_ = "Failed to create system tables: " + schema_manager_->GetLastError();
            return false;
        }
        
        // 创建数据初始化器
        data_initializer_ = std::make_unique<SystemDataInitializer>(db_manager_);
        
        // 初始化默认数据
        if (!data_initializer_->InitializeDefaultData()) {
            last_error_ = "Failed to initialize default data: " + data_initializer_->GetLastError();
            return false;
        }
        
        // 创建权限管理器
        permission_manager_ = std::make_unique<SystemPermissionManager>(db_manager_);
        
        // 创建元数据管理器
        metadata_manager_ = std::make_unique<SystemMetadataManager>(db_manager_);
        
        is_initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        last_error_ = std::string("Failed to initialize system database: ") + e.what();
        return false;
    }
}

bool SystemDatabase::IsInitialized() const {
    return is_initialized_;
}

std::shared_ptr<DatabaseManager> SystemDatabase::GetDatabaseManager() const {
    return db_manager_;
}

std::string SystemDatabase::GetLastError() const {
    return last_error_;
}

// 数据库操作
bool SystemDatabase::CreateDatabase(const std::string& db_name, const std::string& owner, const std::string& description) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用元数据管理器创建数据库记录
    if (!metadata_manager_->CreateDatabaseRecord(db_name, owner, description)) {
        last_error_ = metadata_manager_->GetLastError();
        return false;
    }
    
    // 创建物理数据库
    if (!db_manager_->CreateDatabase(db_name)) {
        last_error_ = "Failed to create physical database: " + db_name;
        return false;
    }
    
    return true;
}

bool SystemDatabase::DropDatabase(const std::string& db_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 不能删除系统数据库
    if (db_name == SYSTEM_DB_NAME) {
        last_error_ = "Cannot drop system database";
        return false;
    }
    
    // 删除物理数据库
    if (!db_manager_->DropDatabase(db_name)) {
        last_error_ = "Failed to drop physical database: " + db_name;
        return false;
    }
    
    // 使用元数据管理器删除数据库记录
    if (!metadata_manager_->DropDatabaseRecord(db_name)) {
        last_error_ = metadata_manager_->GetLastError();
        return false;
    }
    
    return true;
}

// 用户操作
bool SystemDatabase::CreateUser(const std::string& username, const std::string& password, const std::string& email, const std::string& role) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器创建用户
    if (!permission_manager_->CreateUserRecord(username, password, email, role)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

bool SystemDatabase::DropUser(const std::string& username) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器删除用户
    if (!permission_manager_->DropUserRecord(username)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

bool SystemDatabase::UpdateUser(const std::string& username, const std::string& password, const std::string& email) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器更新用户
    if (!permission_manager_->UpdateUserRecord(username, password, email)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

// 角色操作
bool SystemDatabase::CreateRole(const std::string& role_name, const std::string& description) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器创建角色
    if (!permission_manager_->CreateRoleRecord(role_name, description)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

bool SystemDatabase::DropRole(const std::string& role_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器删除角色
    if (!permission_manager_->DropRoleRecord(role_name)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

// 权限操作
bool SystemDatabase::GrantPrivilege(const std::string& username, const std::string& object_type, 
                                   const std::string& object_name, const std::string& privilege) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器授予权限
    if (!permission_manager_->GrantPrivilegeRecord(username, object_type, object_name, privilege)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

bool SystemDatabase::RevokePrivilege(const std::string& username, const std::string& object_type, 
                                    const std::string& object_name, const std::string& privilege) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器撤销权限
    if (!permission_manager_->RevokePrivilegeRecord(username, object_type, object_name, privilege)) {
        last_error_ = permission_manager_->GetLastError();
        return false;
    }
    
    return true;
}

// 查询操作
SysUser SystemDatabase::GetUser(const std::string& username) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return SysUser();
    }
    
    // 使用权限管理器获取用户
    return permission_manager_->GetUserRecord(username);
}

SysRole SystemDatabase::GetRole(const std::string& role_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return SysRole();
    }
    
    // 使用权限管理器获取角色
    return permission_manager_->GetRoleRecord(role_name);
}

SysDatabase SystemDatabase::GetDatabase(const std::string& db_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return SysDatabase();
    }
    
    // 使用元数据管理器获取数据库
    return metadata_manager_->GetDatabaseRecord(db_name);
}

std::vector<SysUser> SystemDatabase::ListUsers() {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return std::vector<SysUser>();
    }
    
    // 使用权限管理器列出用户
    return permission_manager_->ListUsers();
}

std::vector<SysRole> SystemDatabase::ListRoles() {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return std::vector<SysRole>();
    }
    
    // 使用权限管理器列出角色
    return permission_manager_->ListRoles();
}

std::vector<SysDatabase> SystemDatabase::ListDatabases() {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return std::vector<SysDatabase>();
    }
    
    // 使用元数据管理器列出数据库
    return metadata_manager_->ListDatabases();
}

std::vector<SysTable> SystemDatabase::ListTables(const std::string& db_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return std::vector<SysTable>();
    }
    
    // 获取数据库ID
    SysDatabase db = metadata_manager_->GetDatabaseRecord(db_name);
    if (db.db_id == 0) {
        last_error_ = "Database not found: " + db_name;
        return std::vector<SysTable>();
    }
    
    // 使用元数据管理器列出表
    return metadata_manager_->ListTables(db.db_id);
}

std::vector<SysView> SystemDatabase::ListViews(const std::string& db_name) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return std::vector<SysView>();
    }
    
    // 获取数据库ID
    SysDatabase db = metadata_manager_->GetDatabaseRecord(db_name);
    if (db.db_id == 0) {
        last_error_ = "Database not found: " + db_name;
        return std::vector<SysView>();
    }
    
    // 使用元数据管理器列出视图
    return metadata_manager_->ListViews(db.db_id);
}

// 验证操作
bool SystemDatabase::ValidateUser(const std::string& username, const std::string& password) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器验证用户
    SysUser user = permission_manager_->GetUserRecord(username);
    if (user.user_id == 0) {
        last_error_ = "User not found: " + username;
        return false;
    }
    
    // 验证密码
    return user.password == password; // 注意：实际应用中应该使用加密密码
}

bool SystemDatabase::HasPrivilege(const std::string& username, const std::string& object_type, 
                                 const std::string& object_name, const std::string& privilege) {
    if (!is_initialized_) {
        last_error_ = "System database is not initialized";
        return false;
    }
    
    // 使用权限管理器检查权限
    return permission_manager_->HasPrivilege(username, object_type, object_name, privilege);
}

// 获取管理器
SystemSchemaManager* SystemDatabase::GetSchemaManager() const {
    return schema_manager_.get();
}

SystemDataInitializer* SystemDatabase::GetDataInitializer() const {
    return data_initializer_.get();
}

SystemPermissionManager* SystemDatabase::GetPermissionManager() const {
    return permission_manager_.get();
}

SystemMetadataManager* SystemDatabase::GetMetadataManager() const {
    return metadata_manager_.get();
}

// 私有方法
bool SystemDatabase::Exists() {
    // 检查系统数据库是否存在
    return db_manager_->DatabaseExists(SYSTEM_DB_NAME);
}

std::string SystemDatabase::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

uint64_t SystemDatabase::GenerateId() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

} // namespace sqlcc