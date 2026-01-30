#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../backups/core_backup_20260121_001034/core_database_manager.h"

namespace sqlcc {

/**
 * @brief 系统表结构管理器
 * 
 * 负责创建和管理所有系统表的结构定义，包括：
 * - 系统数据库表（sys_databases）
 * - 用户管理表（sys_users, sys_roles）
 * - 对象元数据表（sys_tables, sys_columns, sys_indexes, sys_constraints）
 * - 其他系统表（sys_views, sys_procedures, sys_triggers等）
 */
class SystemSchemaManager {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器指针
     */
    explicit SystemSchemaManager(std::shared_ptr<DatabaseManager> db_manager);
    
    /**
     * @brief 析构函数
     */
    ~SystemSchemaManager();
    
    /**
     * @brief 创建所有系统表
     * @return 是否创建成功
     */
    bool CreateAllSystemTables();
    
    /**
     * @brief 检查系统表是否存在
     * @param table_name 表名
     * @return 表是否存在
     */
    bool TableExists(const std::string& table_name);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

private:
    // 系统表创建方法
    bool CreateSysDatabasesTable();
    bool CreateSysUsersTable();
    bool CreateSysRolesTable();
    bool CreateSysTablesTable();
    bool CreateSysColumnsTable();
    bool CreateSysIndexesTable();
    bool CreateSysConstraintsTable();
    bool CreateSysViewsTable();
    bool CreateSysProceduresTable();
    bool CreateSysTriggersTable();
    bool CreateSysPrivilegesTable();
    bool CreateSysAuditLogsTable();
    bool CreateSysAuditPoliciesTable();
    bool CreateSysTransactionsTable();
    bool CreateSysSavepointsTable();
    bool CreateSysClusterNodesTable();
    bool CreateSysDistributedTransactionsTable();
    bool CreateSysDistributedObjectsTable();
    bool CreateSysTemporalTablesTable();
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);
    
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    std::string last_error_;                       // 最后一次错误信息
};

} // namespace sqlcc