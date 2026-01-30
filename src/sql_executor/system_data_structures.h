#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace sqlcc {

/**
 * @brief 系统数据库记录
 */
struct SysDatabase {
    int64_t db_id;              // 数据库ID
    std::string db_name;         // 数据库名称
    std::string owner;           // 所有者
    std::string created_at;      // 创建时间
    std::string description;     // 描述
};

/**
 * @brief 系统用户记录
 */
struct SysUser {
    int64_t user_id;             // 用户ID
    std::string username;        // 用户名
    std::string password_hash;    // 密码哈希
    std::string role;             // 角色
    std::string current_role;     // 当前角色
    bool is_active;               // 是否激活
    std::string created_at;       // 创建时间
};

/**
 * @brief 系统角色记录
 */
struct SysRole {
    int64_t role_id;             // 角色ID
    std::string role_name;        // 角色名称
    std::string created_at;       // 创建时间
};

/**
 * @brief 系统表记录
 */
struct SysTable {
    int64_t table_id;             // 表ID
    int64_t db_id;                // 数据库ID
    std::string schema_name;      // 模式名称
    std::string table_name;       // 表名称
    std::string owner;            // 所有者
    std::string created_at;       // 创建时间
    std::string table_type;       // 表类型
};

/**
 * @brief 系统列记录
 */
struct SysColumn {
    int64_t column_id;            // 列ID
    int64_t table_id;             // 表ID
    std::string column_name;       // 列名称
    std::string data_type;         // 数据类型
    bool is_nullable;             // 是否可为空
    std::string default_value;    // 默认值
    int ordinal_position;         // 位置
};

/**
 * @brief 系统索引记录
 */
struct SysIndex {
    int64_t index_id;             // 索引ID
    int64_t table_id;             // 表ID
    std::string index_name;        // 索引名称
    std::string column_name;       // 列名称
    bool is_unique;               // 是否唯一
    std::string index_type;       // 索引类型
    std::string created_at;       // 创建时间
};

/**
 * @brief 系统约束记录
 */
struct SysConstraint {
    int64_t constraint_id;        // 约束ID
    int64_t table_id;             // 表ID
    std::string constraint_name;   // 约束名称
    std::string constraint_type;   // 约束类型
    std::string column_name;       // 列名称
    std::string reference_table;   // 引用表
    std::string reference_column;  // 引用列
    std::string created_at;        // 创建时间
};

/**
 * @brief 系统视图记录
 */
struct SysView {
    int64_t view_id;               // 视图ID
    int64_t db_id;                 // 数据库ID
    std::string schema_name;       // 模式名称
    std::string view_name;         // 视图名称
    std::string view_definition;   // 视图定义
    std::string owner;             // 所有者
    std::string created_at;        // 创建时间
};

/**
 * @brief 系统权限记录
 */
struct SysPrivilege {
    int64_t privilege_id;          // 权限ID
    std::string grantee;           // 接收者
    std::string object_type;       // 对象类型
    std::string object_name;       // 对象名称
    std::string privilege_type;    // 权限类型
    std::string grantor;           // 授权者
    bool is_grantable;            // 是否可授权
    std::string granted_at;        // 授权时间
};

/**
 * @brief 系统审计日志记录
 */
struct SysAuditLog {
    int64_t log_id;               // 日志ID
    std::string username;         // 用户名
    std::string operation_type;   // 操作类型
    std::string object_type;      // 对象类型
    std::string object_name;      // 对象名称
    std::string sql_statement;    // SQL语句
    std::string operation_time;   // 操作时间
    bool success;                 // 是否成功
    std::string error_message;    // 错误信息
};

/**
 * @brief 系统事务记录
 */
struct SysTransaction {
    int64_t transaction_id;       // 事务ID
    std::string username;         // 用户名
    std::string start_time;       // 开始时间
    std::string status;           // 状态
    std::string description;      // 描述
};

/**
 * @brief 系统保存点记录
 */
struct SysSavepoint {
    int64_t savepoint_id;         // 保存点ID
    int64_t transaction_id;       // 事务ID
    std::string savepoint_name;   // 保存点名称
    std::string created_at;       // 创建时间
};

/**
 * @brief 系统集群节点记录
 */
struct SysClusterNode {
    int64_t node_id;              // 节点ID
    std::string node_name;        // 节点名称
    std::string host_address;     // 主机地址
    int port;                     // 端口
    std::string status;           // 状态
    std::string last_heartbeat;   // 最后心跳时间
};

} // namespace sqlcc