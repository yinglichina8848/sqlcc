#ifndef SQLCC_SYSTEM_DATABASE_H
#define SQLCC_SYSTEM_DATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "database_manager.h"
#include "exception.h"

namespace sqlcc {

// 前向声明
class SqlExecutor;

// 系统数据库常量定义
const std::string SYSTEM_DB_NAME = "system";

// 系统表名定义
const std::string SYS_TABLE_DATABASES = "sys_databases";
const std::string SYS_TABLE_USERS = "sys_users";
const std::string SYS_TABLE_ROLES = "sys_roles";
const std::string SYS_TABLE_TABLES = "sys_tables";
const std::string SYS_TABLE_COLUMNS = "sys_columns";
const std::string SYS_TABLE_INDEXES = "sys_indexes";
const std::string SYS_TABLE_CONSTRAINTS = "sys_constraints";
const std::string SYS_TABLE_VIEWS = "sys_views";
const std::string SYS_TABLE_PROCEDURES = "sys_procedures";
const std::string SYS_TABLE_TRIGGERS = "sys_triggers";
const std::string SYS_TABLE_PRIVILEGES = "sys_privileges";

// 新增系统表名定义
const std::string SYS_TABLE_AUDIT_LOGS = "sys_audit_logs";
const std::string SYS_TABLE_AUDIT_POLICIES = "sys_audit_policies";
const std::string SYS_TABLE_TRANSACTIONS = "sys_transactions";
const std::string SYS_TABLE_SAVEPOINTS = "sys_savepoints";
const std::string SYS_TABLE_CLUSTER_NODES = "sys_cluster_nodes";
const std::string SYS_TABLE_DISTRIBUTED_TRANSACTIONS = "sys_distributed_transactions";
const std::string SYS_TABLE_DISTRIBUTED_OBJECTS = "sys_distributed_objects";
const std::string SYS_TABLE_TEMPORAL_TABLES = "sys_temporal_tables";

// 系统数据库数据结构定义

// 数据库信息
struct SysDatabase {
    int64_t db_id;
    std::string db_name;
    std::string owner;
    std::string created_at;
    std::string description;
};

// 用户信息
struct SysUser {
    int64_t user_id;
    std::string username;
    std::string password_hash;
    std::string role;
    std::string current_role;
    bool is_active;
    std::string created_at;
};

// 角色信息
struct SysRole {
    int64_t role_id;
    std::string role_name;
    std::string created_at;
};

// 表信息
struct SysTable {
    int64_t table_id;
    int64_t db_id;
    std::string schema_name;
    std::string table_name;
    std::string owner;
    std::string created_at;
    std::string table_type; // BASE TABLE, VIEW, etc.
};

// 列信息
struct SysColumn {
    int64_t column_id;
    int64_t table_id;
    std::string column_name;
    std::string data_type;
    bool is_nullable;
    std::string default_value;
    int ordinal_position;
};

// 索引信息
struct SysIndex {
    int64_t index_id;
    int64_t table_id;
    std::string index_name;
    std::string column_name;
    bool is_unique;
    std::string index_type;
    std::string created_at;
};

// 约束信息
struct SysConstraint {
    int64_t constraint_id;
    int64_t table_id;
    std::string constraint_name;
    std::string constraint_type; // PRIMARY KEY, FOREIGN KEY, CHECK, UNIQUE
    std::string column_name;
    std::string check_expression;
    std::string referenced_table;
    std::string referenced_column;
};

// 视图信息
struct SysView {
    int64_t view_id;
    int64_t db_id;
    std::string schema_name;
    std::string view_name;
    std::string definition;
    std::string owner;
    std::string created_at;
};

// 存储过程信息
struct SysProcedure {
    int64_t proc_id;
    int64_t db_id;
    std::string schema_name;
    std::string proc_name;
    std::string definition;
    std::string owner;
    std::string created_at;
};

// 触发器信息
struct SysTrigger {
    int64_t trigger_id;
    int64_t table_id;
    std::string trigger_name;
    std::string trigger_type; // BEFORE/AFTER, INSERT/UPDATE/DELETE
    std::string trigger_body;
    std::string owner;
    std::string created_at;
};

// 权限信息
struct SysPrivilege {
    int64_t privilege_id;
    std::string grantee_type; // USER/ROLE
    std::string grantee_name;
    std::string db_name;
    std::string table_name;
    std::string privilege; // SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER
    std::string grantor;
};

// 审计日志信息
struct SysAuditLog {
    int64_t log_id;
    std::string user_name;
    std::string operation_type;
    std::string object_type;
    std::string object_name;
    std::string operation_time;
    std::string client_ip;
    std::string session_id;
    std::string sql_text;
    int affected_rows;
    std::string execution_result;
};

// 审计策略信息
struct SysAuditPolicy {
    int64_t policy_id;
    std::string object_type;
    std::string object_name;
    std::string operation_type;
    bool is_enabled;
    std::string created_at;
    std::string updated_at;
};

// 事务信息
struct SysTransaction {
    std::string transaction_id;
    std::string session_id;
    std::string user_name;
    std::string start_time;
    std::string end_time;
    std::string status;
    std::string isolation_level;
    std::string client_ip;
};

// 保存点信息
struct SysSavepoint {
    int64_t savepoint_id;
    std::string transaction_id;
    std::string savepoint_name;
    std::string created_at;
};

// 集群节点信息
struct SysClusterNode {
    std::string node_id;
    std::string node_name;
    std::string host_address;
    int port;
    std::string status;
    std::string role;
    std::string joined_at;
    std::string last_heartbeat;
};

// 分布式事务信息
struct SysDistributedTransaction {
    std::string dt_id;
    std::string coordinator_node;
    std::string status;
    std::string created_at;
    std::string updated_at;
    int timeout_seconds;
};

// 分布式对象信息
struct SysDistributedObject {
    int64_t object_id;
    std::string object_type;
    std::string object_name;
    std::string database_name;
    std::string shard_key;
    std::string node_mapping;
    int replication_factor;
    std::string created_at;
};

// 时态表信息
struct SysTemporalTable {
    int64_t temporal_id;
    int64_t table_id;
    std::string system_time_start_column;
    std::string system_time_end_column;
    std::string period_start;
    std::string period_end;
    int retention_period_days;
    std::string created_at;
};

/**
 * 系统数据库管理器
 * 负责管理SQLCC的system数据库，存储所有元数据信息
 */
class SystemDatabase {
public:
    /**
     * 构造函数
     * @param db_manager 数据库管理器实例
     */
    SystemDatabase(std::shared_ptr<DatabaseManager> db_manager);

    /**
     * 析构函数
     */
    ~SystemDatabase();

    /**
     * 初始化系统数据库
     * 创建system数据库和所有系统表
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * 检查系统数据库是否存在
     * @return 是否存在
     */
    bool Exists();

    /**
     * 获取数据库管理器
     * @return 数据库管理器指针
     */
    std::shared_ptr<DatabaseManager> GetDatabaseManager() {
        return db_manager_;
    }

    // 数据库元数据操作
    bool CreateDatabaseRecord(const std::string& db_name, const std::string& owner, const std::string& description = "");
    bool DropDatabaseRecord(const std::string& db_name);
    SysDatabase GetDatabaseRecord(const std::string& db_name);
    std::vector<SysDatabase> ListDatabases();
    bool DatabaseExists(const std::string& db_name);

    // 用户元数据操作
    bool CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role);
    bool DropUserRecord(const std::string& username);
    bool UpdateUserRecord(const SysUser& user);
    SysUser GetUserRecord(const std::string& username);
    std::vector<SysUser> ListUsers();
    bool UserExists(const std::string& username);

    // 角色元数据操作
    bool CreateRoleRecord(const std::string& role_name);
    bool DropRoleRecord(const std::string& role_name);
    SysRole GetRoleRecord(const std::string& role_name);
    std::vector<SysRole> ListRoles();
    bool RoleExists(const std::string& role_name);

    // 表元数据操作
    bool CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                          const std::string& owner, const std::string& table_type = "BASE TABLE");
    bool DropTableRecord(const std::string& schema_name, const std::string& table_name);
    SysTable GetTableRecord(const std::string& schema_name, const std::string& table_name);
    std::vector<SysTable> ListTables(int64_t db_id);
    bool TableExists(const std::string& schema_name, const std::string& table_name);

    // 列元数据操作
    bool CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                           bool is_nullable, const std::string& default_value, int ordinal_position);
    bool DropColumnRecord(int64_t table_id, const std::string& column_name);
    std::vector<SysColumn> GetTableColumns(int64_t table_id);

    // 索引元数据操作
    bool CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                          bool is_unique, const std::string& index_type);
    bool DropIndexRecord(int64_t table_id, const std::string& index_name);
    std::vector<SysIndex> GetTableIndexes(int64_t table_id);

    // 约束元数据操作
    bool CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, const std::string& constraint_type,
                               const std::string& column_name, const std::string& check_expression = "",
                               const std::string& referenced_table = "", const std::string& referenced_column = "");
    bool DropConstraintRecord(int64_t table_id, const std::string& constraint_name);
    std::vector<SysConstraint> GetTableConstraints(int64_t table_id);

    // 视图元数据操作
    bool CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                         const std::string& definition, const std::string& owner);
    bool DropViewRecord(const std::string& schema_name, const std::string& view_name);
    SysView GetViewRecord(const std::string& schema_name, const std::string& view_name);
    std::vector<SysView> ListViews(int64_t db_id);

    // 权限元数据操作
    bool GrantPrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                             const std::string& db_name, const std::string& table_name,
                             const std::string& privilege, const std::string& grantor);
    bool RevokePrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                              const std::string& db_name, const std::string& table_name,
                              const std::string& privilege);
    std::vector<SysPrivilege> GetUserPrivileges(const std::string& username);

    // 审计功能
    bool CreateAuditLog(const std::string& user_name, const std::string& operation_type,
                       const std::string& object_type, const std::string& object_name,
                       const std::string& client_ip, const std::string& session_id,
                       const std::string& sql_text, int affected_rows,
                       const std::string& execution_result);
    bool CreateAuditPolicy(const std::string& object_type, const std::string& object_name,
                          const std::string& operation_type, bool is_enabled);
    std::vector<SysAuditLog> GetAuditLogs(time_t start_time, time_t end_time);
    std::vector<SysAuditPolicy> GetAuditPolicies();

    // 事务功能
    bool CreateTransactionRecord(const std::string& transaction_id, const std::string& session_id,
                               const std::string& user_name, const std::string& client_ip,
                               const std::string& isolation_level);
    bool UpdateTransactionStatus(const std::string& transaction_id, const std::string& status,
                                time_t end_time);
    bool CreateSavepointRecord(const std::string& transaction_id, const std::string& savepoint_name);
    std::vector<SysTransaction> GetActiveTransactions();

    // 分布式功能
    bool RegisterClusterNode(const std::string& node_id, const std::string& node_name,
                           const std::string& host_address, int port,
                           const std::string& role);
    bool UpdateNodeStatus(const std::string& node_id, const std::string& status,
                         time_t last_heartbeat);
    bool CreateDistributedTransaction(const std::string& dt_id, const std::string& coordinator_node);
    bool UpdateDistributedTransactionStatus(const std::string& dt_id, const std::string& status);
    bool RegisterDistributedObject(int64_t object_id, const std::string& object_type,
                                  const std::string& object_name, const std::string& database_name,
                                  const std::string& shard_key, const std::string& node_mapping,
                                  int replication_factor);
    std::vector<SysClusterNode> GetClusterNodes();
    std::vector<SysDistributedTransaction> GetActiveDistributedTransactions();

    // 工具方法
    std::string GetCurrentTimeString() const;
    int64_t GenerateId(const std::string& table_name);

    /**
     * 获取最后一次错误信息
     * @return 错误信息
     */
    const std::string& GetLastError() const {
        return last_error_;
    }

private:
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    std::shared_ptr<SqlExecutor> sql_executor_;    // SQL执行器
    std::string last_error_;                       // 最后一次错误信息

    // 系统表创建方法
    bool CreateSystemTables();
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

    // 新增系统表创建方法
    bool CreateSysAuditLogsTable();
    bool CreateSysAuditPoliciesTable();
    bool CreateSysTransactionsTable();
    bool CreateSysSavepointsTable();
    bool CreateSysClusterNodesTable();
    bool CreateSysDistributedTransactionsTable();
    bool CreateSysDistributedObjectsTable();
    bool CreateSysTemporalTablesTable();

    // 初始化默认数据
    bool InitializeDefaultData();

    // 执行SQL语句的辅助方法
    bool ExecuteSQL(const std::string& sql);

    void SetError(const std::string& error) {
        last_error_ = error;
    }
};

} // namespace sqlcc

#endif // SQLCC_SYSTEM_DATABASE_H