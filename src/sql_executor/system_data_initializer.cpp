#include "src/system_data_initializer.h"
#include "src/system_data_structures.h"
#include "src/system_database_new.h"  // 包含SYSTEM_DB_NAME常量定义
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace sqlcc {

// 默认角色和用户
const std::string DEFAULT_SUPERUSER_ROLE = "admin";
const std::string DEFAULT_SUPERUSER_NAME = "root";
const std::string DEFAULT_SUPERUSER_PASSWORD = "admin123"; // 在实际应用中应该使用更强的密码

SystemDataInitializer::SystemDataInitializer(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SystemDataInitializer::~SystemDataInitializer() {
}

bool SystemDataInitializer::InitializeDefaultData() {
    try {
        // 初始化默认角色
        if (!InitializeDefaultRoles()) {
            return false;
        }
        
        // 初始化默认用户
        if (!InitializeDefaultUsers()) {
            return false;
        }
        
        // 初始化默认权限
        if (!InitializeDefaultPrivileges()) {
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to initialize default data: ") + e.what());
        return false;
    }
}

std::string SystemDataInitializer::GetLastError() const {
    return last_error_;
}

bool SystemDataInitializer::InitializeDefaultRoles() {
    try {
        // 检查是否已有角色数据
        std::string check_sql = "SELECT COUNT(*) FROM sys_roles";
        auto result = ExecuteSelectQuery(check_sql);
        
        if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
            // 已有角色数据，跳过初始化
            return true;
        }
        
        // 创建默认角色
        std::vector<std::string> default_roles = {
            DEFAULT_SUPERUSER_ROLE,  // 超级管理员
            "dba",                   // 数据库管理员
            "developer",             // 开发者
            "readonly",              // 只读用户
            "readwrite"              // 读写用户
        };
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 为每个角色创建记录
        for (const auto& role_name : default_roles) {
            // 生成角色ID
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
                
            std::stringstream sql;
            sql << "INSERT INTO sys_roles (role_id, role_name, created_at) VALUES ("
                << timestamp << ", '" << role_name << "', '" << current_time << "')";
                
            if (!ExecuteSQL(sql.str())) {
                SetError("Failed to create default role: " + role_name);
                return false;
            }
            
            // 为下一个角色增加ID
            now += std::chrono::milliseconds(1);
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to initialize default roles: ") + e.what());
        return false;
    }
}

bool SystemDataInitializer::InitializeDefaultUsers() {
    try {
        // 检查是否已有用户数据
        std::string check_sql = "SELECT COUNT(*) FROM sys_users";
        auto result = ExecuteSelectQuery(check_sql);
        
        if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
            // 已有用户数据，跳过初始化
            return true;
        }
        
        // 创建默认超级用户
        // 在实际应用中，密码应该经过哈希处理
        std::string password_hash = DEFAULT_SUPERUSER_PASSWORD; // 简化示例，实际应该使用哈希
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成用户ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
            
        std::stringstream sql;
        sql << "INSERT INTO sys_users (user_id, username, password_hash, role, current_role, is_active, created_at) VALUES ("
            << timestamp << ", '" << DEFAULT_SUPERUSER_NAME << "', '" << password_hash << "', '"
            << DEFAULT_SUPERUSER_ROLE << "', '" << DEFAULT_SUPERUSER_ROLE << "', TRUE, '" << current_time << "')";
            
        if (!ExecuteSQL(sql.str())) {
            SetError("Failed to create default superuser");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to initialize default users: ") + e.what());
        return false;
    }
}

bool SystemDataInitializer::InitializeDefaultPrivileges() {
    try {
        // 检查是否已有权限数据
        std::string check_sql = "SELECT COUNT(*) FROM sys_privileges";
        auto result = ExecuteSelectQuery(check_sql);
        
        if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
            // 已有权限数据，跳过初始化
            return true;
        }
        
        // 为默认超级用户授予所有权限
        std::vector<std::pair<std::string, std::string>> default_privileges = {
            {"DATABASE", "ALL PRIVILEGES"},
            {"TABLE", "ALL PRIVILEGES"},
            {"VIEW", "ALL PRIVILEGES"},
            {"PROCEDURE", "ALL PRIVILEGES"},
            {"TRIGGER", "ALL PRIVILEGES"},
            {"USER", "ALL PRIVILEGES"},
            {"ROLE", "ALL PRIVILEGES"}
        };
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 为每个权限创建记录
        for (const auto& privilege : default_privileges) {
            // 生成权限ID
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
                
            std::stringstream sql;
            sql << "INSERT INTO sys_privileges (privilege_id, grantee, object_type, object_name, "
                << "privilege_type, grantor, is_grantable, granted_at) VALUES ("
                << timestamp << ", '" << DEFAULT_SUPERUSER_NAME << "', '" << privilege.first
                << "', '*', '" << privilege.second << "', '" << DEFAULT_SUPERUSER_NAME
                << "', TRUE, '" << current_time << "')";
                
            if (!ExecuteSQL(sql.str())) {
                SetError("Failed to grant default privilege: " + privilege.first + " " + privilege.second);
                return false;
            }
            
            // 为下一个权限增加ID
            now += std::chrono::milliseconds(1);
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to initialize default privileges: ") + e.what());
        return false;
    }
}

bool SystemDataInitializer::ExecuteSQL(const std::string& sql) {
    try {
        // 切换到系统数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return false;
        }
        
        // 执行SQL语句
        if (!db_manager_->ExecuteSQL(sql)) {
            SetError("Failed to execute SQL: " + sql);
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to execute SQL: ") + e.what());
        return false;
    }
}

std::vector<std::vector<std::string>> SystemDataInitializer::ExecuteSelectQuery(const std::string& sql) {
    std::vector<std::vector<std::string>> result;
    
    try {
        // 切换到系统数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return result;
        }
        
        // 执行查询
        // 注意：这里假设DatabaseManager有ExecuteQuery方法，返回查询结果
        // 实际实现可能需要根据DatabaseManager的API进行调整
        result = db_manager_->ExecuteQuery(sql);
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to execute query: ") + e.what());
        return result;
    }
}

void SystemDataInitializer::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace sqlcc