#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../backups/core_backup_20260121_001034/core_database_manager.h"

namespace sqlcc {

/**
 * @brief 系统数据初始化器
 * 
 * 负责初始化系统数据库的默认数据，包括：
 * - 默认超级用户和角色
 * - 系统默认配置
 * - 基础权限设置
 */
class SystemDataInitializer {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器指针
     */
    explicit SystemDataInitializer(std::shared_ptr<DatabaseManager> db_manager);
    
    /**
     * @brief 析构函数
     */
    ~SystemDataInitializer();
    
    /**
     * @brief 初始化所有默认数据
     * @return 是否初始化成功
     */
    bool InitializeDefaultData();
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

private:
    /**
     * @brief 初始化默认角色
     * @return 是否初始化成功
     */
    bool InitializeDefaultRoles();
    
    /**
     * @brief 初始化默认用户
     * @return 是否初始化成功
     */
    bool InitializeDefaultUsers();
    
    /**
     * @brief 初始化默认权限
     * @return 是否初始化成功
     */
    bool InitializeDefaultPrivileges();
    
    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @return 是否执行成功
     */
    bool ExecuteSQL(const std::string& sql);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);
    
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    std::string last_error_;                       // 最后一次错误信息
};

} // namespace sqlcc