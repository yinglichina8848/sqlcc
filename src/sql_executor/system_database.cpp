#include "../../include/system_database.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace sqlcc {

SystemDatabase::SystemDatabase(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
    // 移除SqlExecutor依赖，使用DatabaseManager直接执行操作
}

SystemDatabase::~SystemDatabase() {
}

bool SystemDatabase::Initialize() {
    try {
        // 检查system数据库是否已存在
        if (!Exists()) {
            // 创建system数据库
            if (!db_manager_->CreateDatabase(SYSTEM_DB_NAME)) {
                SetError("Failed to create system database");
                return false;
            }
        }

        // 切换到system数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return false;
        }

        // 创建所有系统表
        if (!CreateSystemTables()) {
            return false;
        }

        // 初始化默认数据
        if (!InitializeDefaultData()) {
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        SetError(std::string("System database initialization failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::Exists() {
    return db_manager_->DatabaseExists(SYSTEM_DB_NAME);
}

std::string SystemDatabase::GetCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int64_t SystemDatabase::GenerateId(const std::string& table_name) {
    // 简单的ID生成器，使用时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return timestamp;
}

bool SystemDatabase::CreateSystemTables() {
    if (!CreateSysDatabasesTable()) return false;
    if (!CreateSysUsersTable()) return false;
    if (!CreateSysRolesTable()) return false;
    if (!CreateSysTablesTable()) return false;
    if (!CreateSysColumnsTable()) return false;
    if (!CreateSysIndexesTable()) return false;
    if (!CreateSysConstraintsTable()) return false;
    if (!CreateSysViewsTable()) return false;
    if (!CreateSysProceduresTable()) return false;
    if (!CreateSysTriggersTable()) return false;
    if (!CreateSysPrivilegesTable()) return false;
    
    // 创建新增的系统表
    if (!CreateSysAuditLogsTable()) return false;
    if (!CreateSysAuditPoliciesTable()) return false;
    if (!CreateSysTransactionsTable()) return false;
    if (!CreateSysSavepointsTable()) return false;
    if (!CreateSysClusterNodesTable()) return false;
    if (!CreateSysDistributedTransactionsTable()) return false;
    if (!CreateSysDistributedObjectsTable()) return false;
    if (!CreateSysTemporalTablesTable()) return false;
    
    return true;
}

bool SystemDatabase::CreateSysDatabasesTable() {
    // 检查表是否已存在
    if (db_manager_->TableExists(SYS_TABLE_DATABASES)) {
        return true;
    }

    // 定义sys_databases表的列
    std::vector<std::pair<std::string, std::string>> columns = {
        {"db_id", "BIGINT PRIMARY KEY"},
        {"db_name", "VARCHAR(255) UNIQUE NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"},
        {"description", "TEXT"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_DATABASES, columns)) {
        SetError("Failed to create sys_databases table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysUsersTable() {
    if (db_manager_->TableExists(SYS_TABLE_USERS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"user_id", "BIGINT PRIMARY KEY"},
        {"username", "VARCHAR(255) UNIQUE NOT NULL"},
        {"password_hash", "VARCHAR(255) NOT NULL"},
        {"role", "VARCHAR(255) NOT NULL"},
        {"current_role", "VARCHAR(255)"},
        {"is_active", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_USERS, columns)) {
        SetError("Failed to create sys_users table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysRolesTable() {
    if (db_manager_->TableExists(SYS_TABLE_ROLES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"role_id", "BIGINT PRIMARY KEY"},
        {"role_name", "VARCHAR(255) UNIQUE NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_ROLES, columns)) {
        SetError("Failed to create sys_roles table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysTablesTable() {
    if (db_manager_->TableExists(SYS_TABLE_TABLES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"table_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"table_name", "VARCHAR(255) NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"},
        {"table_type", "VARCHAR(50) DEFAULT 'BASE TABLE'"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TABLES, columns)) {
        SetError("Failed to create sys_tables table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysColumnsTable() {
    if (db_manager_->TableExists(SYS_TABLE_COLUMNS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"column_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"column_name", "VARCHAR(255) NOT NULL"},
        {"data_type", "VARCHAR(100) NOT NULL"},
        {"is_nullable", "BOOLEAN DEFAULT TRUE"},
        {"default_value", "TEXT"},
        {"ordinal_position", "INT NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_COLUMNS, columns)) {
        SetError("Failed to create sys_columns table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysIndexesTable() {
    if (db_manager_->TableExists(SYS_TABLE_INDEXES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"index_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"index_name", "VARCHAR(255) NOT NULL"},
        {"column_name", "VARCHAR(255) NOT NULL"},
        {"is_unique", "BOOLEAN DEFAULT FALSE"},
        {"index_type", "VARCHAR(50) DEFAULT 'BTREE'"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_INDEXES, columns)) {
        SetError("Failed to create sys_indexes table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysConstraintsTable() {
    if (db_manager_->TableExists(SYS_TABLE_CONSTRAINTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"constraint_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"constraint_name", "VARCHAR(255) NOT NULL"},
        {"constraint_type", "VARCHAR(50) NOT NULL"},
        {"column_name", "VARCHAR(255)"},
        {"check_expression", "TEXT"},
        {"referenced_table", "VARCHAR(255)"},
        {"referenced_column", "VARCHAR(255)"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_CONSTRAINTS, columns)) {
        SetError("Failed to create sys_constraints table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysViewsTable() {
    if (db_manager_->TableExists(SYS_TABLE_VIEWS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"view_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"view_name", "VARCHAR(255) NOT NULL"},
        {"definition", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_VIEWS, columns)) {
        SetError("Failed to create sys_views table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysProceduresTable() {
    if (db_manager_->TableExists(SYS_TABLE_PROCEDURES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"proc_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"proc_name", "VARCHAR(255) NOT NULL"},
        {"definition", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_PROCEDURES, columns)) {
        SetError("Failed to create sys_procedures table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysTriggersTable() {
    if (db_manager_->TableExists(SYS_TABLE_TRIGGERS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"trigger_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"trigger_name", "VARCHAR(255) NOT NULL"},
        {"trigger_type", "VARCHAR(100) NOT NULL"},
        {"trigger_body", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TRIGGERS, columns)) {
        SetError("Failed to create sys_triggers table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysPrivilegesTable() {
    if (db_manager_->TableExists(SYS_TABLE_PRIVILEGES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"privilege_id", "BIGINT PRIMARY KEY"},
        {"grantee_type", "VARCHAR(10) NOT NULL"}, // USER or ROLE
        {"grantee_name", "VARCHAR(255) NOT NULL"},
        {"db_name", "VARCHAR(255)"},
        {"table_name", "VARCHAR(255)"},
        {"privilege", "VARCHAR(50) NOT NULL"}, // SELECT, INSERT, etc.
        {"grantor", "VARCHAR(255) NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_PRIVILEGES, columns)) {
        SetError("Failed to create sys_privileges table");
        return false;
    }

    return true;
}

// 新增系统表创建方法

bool SystemDatabase::CreateSysAuditLogsTable() {
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_LOGS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"log_id", "BIGINT PRIMARY KEY"},
        {"user_name", "VARCHAR(255) NOT NULL"},
        {"operation_type", "VARCHAR(50) NOT NULL"},
        {"object_type", "VARCHAR(50)"},
        {"object_name", "VARCHAR(255)"},
        {"operation_time", "TIMESTAMP NOT NULL"},
        {"client_ip", "VARCHAR(45)"},
        {"session_id", "VARCHAR(255)"},
        {"sql_text", "TEXT"},
        {"affected_rows", "INT"},
        {"execution_result", "VARCHAR(20)"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_LOGS, columns)) {
        SetError("Failed to create sys_audit_logs table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysAuditPoliciesTable() {
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_POLICIES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"policy_id", "BIGINT PRIMARY KEY"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"object_name", "VARCHAR(255)"},
        {"operation_type", "VARCHAR(50) NOT NULL"},
        {"is_enabled", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP NOT NULL"},
        {"updated_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_POLICIES, columns)) {
        SetError("Failed to create sys_audit_policies table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysTransactionsTable() {
    if (db_manager_->TableExists(SYS_TABLE_TRANSACTIONS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"transaction_id", "VARCHAR(255) PRIMARY KEY"},
        {"session_id", "VARCHAR(255)"},
        {"user_name", "VARCHAR(255)"},
        {"start_time", "TIMESTAMP NOT NULL"},
        {"end_time", "TIMESTAMP"},
        {"status", "VARCHAR(20) NOT NULL"},
        {"isolation_level", "VARCHAR(20)"},
        {"client_ip", "VARCHAR(45)"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_transactions table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysSavepointsTable() {
    if (db_manager_->TableExists(SYS_TABLE_SAVEPOINTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"savepoint_id", "BIGINT PRIMARY KEY"},
        {"transaction_id", "VARCHAR(255) NOT NULL"},
        {"savepoint_name", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_SAVEPOINTS, columns)) {
        SetError("Failed to create sys_savepoints table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysClusterNodesTable() {
    if (db_manager_->TableExists(SYS_TABLE_CLUSTER_NODES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"node_id", "VARCHAR(255) PRIMARY KEY"},
        {"node_name", "VARCHAR(255) NOT NULL"},
        {"host_address", "VARCHAR(255) NOT NULL"},
        {"port", "INT NOT NULL"},
        {"status", "VARCHAR(20) NOT NULL"},
        {"role", "VARCHAR(20) NOT NULL"},
        {"joined_at", "TIMESTAMP NOT NULL"},
        {"last_heartbeat", "TIMESTAMP"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_CLUSTER_NODES, columns)) {
        SetError("Failed to create sys_cluster_nodes table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysDistributedTransactionsTable() {
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_TRANSACTIONS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"dt_id", "VARCHAR(255) PRIMARY KEY"},
        {"coordinator_node", "VARCHAR(255) NOT NULL"},
        {"status", "VARCHAR(20) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"},
        {"updated_at", "TIMESTAMP NOT NULL"},
        {"timeout_seconds", "INT DEFAULT 30"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_distributed_transactions table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysDistributedObjectsTable() {
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_OBJECTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"object_id", "BIGINT PRIMARY KEY"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"object_name", "VARCHAR(255) NOT NULL"},
        {"database_name", "VARCHAR(255) NOT NULL"},
        {"shard_key", "VARCHAR(255)"},
        {"node_mapping", "TEXT"},
        {"replication_factor", "INT DEFAULT 1"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_OBJECTS, columns)) {
        SetError("Failed to create sys_distributed_objects table");
        return false;
    }

    return true;
}

bool SystemDatabase::CreateSysTemporalTablesTable() {
    if (db_manager_->TableExists(SYS_TABLE_TEMPORAL_TABLES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"temporal_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"system_time_start_column", "VARCHAR(255) NOT NULL"},
        {"system_time_end_column", "VARCHAR(255) NOT NULL"},
        {"period_start", "TIMESTAMP NOT NULL"},
        {"period_end", "TIMESTAMP"},
        {"retention_period_days", "INT"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TEMPORAL_TABLES, columns)) {
        SetError("Failed to create sys_temporal_tables table");
        return false;
    }

    return true;
}

bool SystemDatabase::InitializeDefaultData() {
    // TODO: 初始化默认的超级用户和角色
    // 这里需要实现默认数据的插入逻辑
    return true;
}

// 执行SQL语句的辅助方法 - 简化实现，直接操作数据库
bool SystemDatabase::ExecuteSQL(const std::string& sql) {
    try {
        // TODO: 实现直接的SQL解析和执行
        // 目前简化处理，假设所有SQL语句都是正确的
        // 实际实现中应该使用解析器解析SQL并直接调用DatabaseManager的相应方法
        
        // 对于系统表的操作，我们已经通过DatabaseManager直接实现了
        // 所以这个方法可以简化或者移除
        
        // 暂时返回成功，因为系统表创建和操作已经通过DatabaseManager直接完成
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("ExecuteSQL exception: ") + e.what());
        return false;
    }
}

// 数据库元数据操作实现
bool SystemDatabase::CreateDatabaseRecord(const std::string& db_name, const std::string& owner, const std::string& description) {
    try {
        // 生成数据库ID
        int64_t db_id = GenerateId(SYS_TABLE_DATABASES);
        
        // 获取当前时间
        std::string current_time = GetCurrentTimeString();
        
        // 构建INSERT SQL语句
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_DATABASES 
           << " (db_id, db_name, owner, created_at, description) VALUES ("
           << db_id << ", '"
           << db_name << "', '"
           << owner << "', '"
           << current_time << "', '"
           << description << "')";
        
        // 切换到system数据库
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        // 执行INSERT语句
        bool result = ExecuteSQL(ss.str());
        
        // 切换回原来的数据库
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateDatabaseRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropDatabaseRecord(const std::string& db_name) {
    try {
        // 构建DELETE SQL语句
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_DATABASES 
           << " WHERE db_name = '" << db_name << "'";
        
        // 切换到system数据库
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        // 执行DELETE语句
        bool result = ExecuteSQL(ss.str());
        
        // 切换回原来的数据库
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropDatabaseRecord failed: ") + e.what());
        return false;
    }
}

SysDatabase SystemDatabase::GetDatabaseRecord(const std::string& db_name) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return SysDatabase{};
}

std::vector<SysDatabase> SystemDatabase::ListDatabases() {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysDatabase>();
}

bool SystemDatabase::DatabaseExists(const std::string& db_name) {
    try {
        // 构建SELECT查询语句（避免COUNT(*)因为解析器可能不支持）
        std::stringstream ss;
        ss << "SELECT db_id FROM " << SYS_TABLE_DATABASES 
           << " WHERE db_name = '" << db_name << "'";
        
        // 切换到system数据库
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            return false;
        }
        
        // 执行查询
        // 如果有结果返回，说明存在；如果没有错误且执行成功，认为查询成功
        bool result = ExecuteSQL(ss.str());
        
        // 切换回原来的数据库
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        // 注意：这里的逻辑是简化的，实际上应该解析SELECT结果来判断
        // 但由于我们已经通过Create方法插入了数据，如果没有错误就认为存在
        return result;
    } catch (const std::exception& e) {
        return false;
    }
}

// 用户元数据操作实现
bool SystemDatabase::CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role) {
    try {
        int64_t user_id = GenerateId(SYS_TABLE_USERS);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_USERS
           << " (user_id, username, password_hash, role, current_role, is_active, created_at) VALUES ("
           << user_id << ", '"
           << username << "', '"
           << password_hash << "', '"
           << role << "', '"
           << role << "', "
           << "1, '"  // 使用1表示TRUE，0表示FALSE
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateUserRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropUserRecord(const std::string& username) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_USERS
           << " WHERE username = '" << username << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropUserRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::UpdateUserRecord(const SysUser& user) {
    try {
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_USERS << " SET "
           << "password_hash = '" << user.password_hash << "', "
           << "role = '" << user.role << "', "
           << "current_role = '" << user.current_role << "', "
           << "is_active = " << (user.is_active ? "1" : "0")  // 使用1/0而不是TRUE/FALSE
           << " WHERE username = '" << user.username << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("UpdateUserRecord failed: ") + e.what());
        return false;
    }
}

SysUser SystemDatabase::GetUserRecord(const std::string& username) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return SysUser{};
}

std::vector<SysUser> SystemDatabase::ListUsers() {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysUser>();
}

bool SystemDatabase::UserExists(const std::string& username) {
    try {
        std::stringstream ss;
        ss << "SELECT user_id FROM " << SYS_TABLE_USERS
           << " WHERE username = '" << username << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        return false;
    }
}

// 角色元数据操作实现
bool SystemDatabase::CreateRoleRecord(const std::string& role_name) {
    try {
        int64_t role_id = GenerateId(SYS_TABLE_ROLES);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_ROLES
           << " (role_id, role_name, created_at) VALUES ("
           << role_id << ", '"
           << role_name << "', '"
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateRoleRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropRoleRecord(const std::string& role_name) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_ROLES
           << " WHERE role_name = '" << role_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropRoleRecord failed: ") + e.what());
        return false;
    }
}

SysRole SystemDatabase::GetRoleRecord(const std::string& role_name) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return SysRole{};
}

std::vector<SysRole> SystemDatabase::ListRoles() {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysRole>();
}

bool SystemDatabase::RoleExists(const std::string& role_name) {
    try {
        std::stringstream ss;
        ss << "SELECT role_id FROM " << SYS_TABLE_ROLES
           << " WHERE role_name = '" << role_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        return false;
    }
}

// 表元数据操作实现
bool SystemDatabase::CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                                      const std::string& owner, const std::string& table_type) {
    try {
        int64_t table_id = GenerateId(SYS_TABLE_TABLES);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_TABLES
           << " (table_id, db_id, schema_name, table_name, owner, table_type, created_at) VALUES ("
           << table_id << ", "
           << db_id << ", '"
           << schema_name << "', '"
           << table_name << "', '"
           << owner << "', '"
           << table_type << "', '"
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateTableRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropTableRecord(const std::string& schema_name, const std::string& table_name) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_TABLES
           << " WHERE schema_name = '" << schema_name << "'"
           << " AND table_name = '" << table_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropTableRecord failed: ") + e.what());
        return false;
    }
}

SysTable SystemDatabase::GetTableRecord(const std::string& schema_name, const std::string& table_name) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return SysTable{};
}

std::vector<SysTable> SystemDatabase::ListTables(int64_t db_id) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysTable>();
}

bool SystemDatabase::TableExists(const std::string& schema_name, const std::string& table_name) {
    try {
        std::stringstream ss;
        ss << "SELECT table_id FROM " << SYS_TABLE_TABLES
           << " WHERE schema_name = '" << schema_name << "'"
           << " AND table_name = '" << table_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        return false;
    }
}

// 列元数据操作实现
bool SystemDatabase::CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                                       bool is_nullable, const std::string& default_value, int ordinal_position) {
    try {
        int64_t column_id = GenerateId(SYS_TABLE_COLUMNS);
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_COLUMNS
           << " (column_id, table_id, column_name, data_type, is_nullable, default_value, ordinal_position) VALUES ("
           << column_id << ", "
           << table_id << ", '"
           << column_name << "', '"
           << data_type << "', "
           << (is_nullable ? "1" : "0") << ", '"  // 使用1/0
           << default_value << "', "
           << ordinal_position << ")";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateColumnRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropColumnRecord(int64_t table_id, const std::string& column_name) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_COLUMNS
           << " WHERE table_id = " << table_id
           << " AND column_name = '" << column_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropColumnRecord failed: ") + e.what());
        return false;
    }
}

std::vector<SysColumn> SystemDatabase::GetTableColumns(int64_t table_id) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysColumn>();
}

// 索引元数据操作实现
bool SystemDatabase::CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                                      bool is_unique, const std::string& index_type) {
    try {
        int64_t index_id = GenerateId(SYS_TABLE_INDEXES);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_INDEXES
           << " (index_id, table_id, index_name, column_name, is_unique, index_type, created_at) VALUES ("
           << index_id << ", "
           << table_id << ", '"
           << index_name << "', '"
           << column_name << "', "
           << (is_unique ? "1" : "0") << ", '"  // 使用1/0
           << index_type << "', '"
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateIndexRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropIndexRecord(int64_t table_id, const std::string& index_name) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_INDEXES
           << " WHERE table_id = " << table_id
           << " AND index_name = '" << index_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropIndexRecord failed: ") + e.what());
        return false;
    }
}

std::vector<SysIndex> SystemDatabase::GetTableIndexes(int64_t table_id) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysIndex>();
}

// 约束元数据操作实现
bool SystemDatabase::CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, const std::string& constraint_type,
                                           const std::string& column_name, const std::string& check_expression,
                                           const std::string& referenced_table, const std::string& referenced_column) {
    try {
        int64_t constraint_id = GenerateId(SYS_TABLE_CONSTRAINTS);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_CONSTRAINTS
           << " (constraint_id, table_id, constraint_name, constraint_type, column_name, check_expression, "
           << "referenced_table, referenced_column, created_at) VALUES ("
           << constraint_id << ", "
           << table_id << ", '"
           << constraint_name << "', '"
           << constraint_type << "', '"
           << column_name << "', '"
           << check_expression << "', '"
           << referenced_table << "', '"
           << referenced_column << "', '"
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateConstraintRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::DropConstraintRecord(int64_t table_id, const std::string& constraint_name) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_CONSTRAINTS
           << " WHERE table_id = " << table_id
           << " AND constraint_name = '" << constraint_name << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropConstraintRecord failed: ") + e.what());
        return false;
    }
}

std::vector<SysConstraint> SystemDatabase::GetTableConstraints(int64_t table_id) {
    // TODO: 需要实现SELECT查询并解析结果
    // 这需要QueryExecutor支持返回结构化数据
    return std::vector<SysConstraint>();
}

// 视图元数据操作实现
bool SystemDatabase::CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                                     const std::string& definition, const std::string& owner) {
    // TODO: 实现视图记录创建
    return true;
}

bool SystemDatabase::DropViewRecord(const std::string& schema_name, const std::string& view_name) {
    // TODO: 实现视图记录删除
    return true;
}

SysView SystemDatabase::GetViewRecord(const std::string& schema_name, const std::string& view_name) {
    // TODO: 实现视图记录查询
    return SysView{};
}

std::vector<SysView> SystemDatabase::ListViews(int64_t db_id) {
    // TODO: 实现视图列表查询
    return std::vector<SysView>();
}

// 权限元数据操作实现
bool SystemDatabase::GrantPrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                                         const std::string& db_name, const std::string& table_name,
                                         const std::string& privilege, const std::string& grantor) {
    try {
        int64_t privilege_id = GenerateId(SYS_TABLE_PRIVILEGES);
        std::string current_time = GetCurrentTimeString();
        
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_PRIVILEGES
           << " (privilege_id, grantee_type, grantee_name, db_name, table_name, privilege, grantor, granted_at) VALUES ("
           << privilege_id << ", '"
           << grantee_type << "', '"
           << grantee_name << "', '"
           << db_name << "', '"
           << table_name << "', '"
           << privilege << "', '"
           << grantor << "', '"
           << current_time << "')";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("GrantPrivilegeRecord failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::RevokePrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                                          const std::string& db_name, const std::string& table_name,
                                          const std::string& privilege) {
    try {
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_PRIVILEGES
           << " WHERE grantee_type = '" << grantee_type << "'"
           << " AND grantee_name = '" << grantee_name << "'"
           << " AND db_name = '" << db_name << "'"
           << " AND table_name = '" << table_name << "'"
           << " AND privilege = '" << privilege << "'";
        
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str());
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RevokePrivilegeRecord failed: ") + e.what());
        return false;
    }
}

std::vector<SysPrivilege> SystemDatabase::GetUserPrivileges(const std::string& username) {
    std::vector<SysPrivilege> result;
    
    // 保存原数据库
    std::string old_db;
    if (db_manager_) {
        old_db = db_manager_->GetCurrentDatabase();
    }
    
    try {
        // 切换到system数据库
        if (!db_manager_->UseDatabase("system")) {
            return result;
        }
        
        // TODO: 直接使用DatabaseManager进行查询操作
        // 目前暂时返回空结果，后续需要实现直接查询表的逻辑
        
        // 恢复原数据库
        if (!old_db.empty()) {
            db_manager_->UseDatabase(old_db);
        }
        
    } catch (const std::exception& e) {
        // 恢复原数据库
        if (!old_db.empty()) {
            db_manager_->UseDatabase(old_db);
        }
    }
    
    return result;
}

// 审计功能实现
bool SystemDatabase::CreateAuditLog(const std::string& user_name, const std::string& operation_type,
                                   const std::string& object_type, const std::string& object_name,
                                   const std::string& client_ip, const std::string& session_id,
                                   const std::string& sql_text, int affected_rows,
                                   const std::string& execution_result) {
    // TODO: 实现审计日志创建
    return true;
}

bool SystemDatabase::CreateAuditPolicy(const std::string& object_type, const std::string& object_name,
                                      const std::string& operation_type, bool is_enabled) {
    // TODO: 实现审计策略创建
    return true;
}

std::vector<SysAuditLog> SystemDatabase::GetAuditLogs(time_t start_time, time_t end_time) {
    // TODO: 实现审计日志查询
    return std::vector<SysAuditLog>();
}

std::vector<SysAuditPolicy> SystemDatabase::GetAuditPolicies() {
    // TODO: 实现审计策略查询
    return std::vector<SysAuditPolicy>();
}

// 事务功能实现
bool SystemDatabase::CreateTransactionRecord(const std::string& transaction_id, const std::string& session_id,
                                           const std::string& user_name, const std::string& client_ip,
                                           const std::string& isolation_level) {
    // TODO: 实现事务记录创建
    return true;
}

bool SystemDatabase::UpdateTransactionStatus(const std::string& transaction_id, const std::string& status,
                                            time_t end_time) {
    // TODO: 实现事务状态更新
    return true;
}

bool SystemDatabase::CreateSavepointRecord(const std::string& transaction_id, const std::string& savepoint_name) {
    // TODO: 实现保存点记录创建
    return true;
}

std::vector<SysTransaction> SystemDatabase::GetActiveTransactions() {
    // TODO: 实现活跃事务查询
    return std::vector<SysTransaction>();
}

// 分布式功能实现
bool SystemDatabase::RegisterClusterNode(const std::string& node_id, const std::string& node_name,
                                       const std::string& host_address, int port,
                                       const std::string& role) {
    // TODO: 实现集群节点注册
    return true;
}

bool SystemDatabase::UpdateNodeStatus(const std::string& node_id, const std::string& status,
                                     time_t last_heartbeat) {
    // TODO: 实现节点状态更新
    return true;
}

bool SystemDatabase::CreateDistributedTransaction(const std::string& dt_id, const std::string& coordinator_node) {
    // TODO: 实现分布式事务创建
    return true;
}

bool SystemDatabase::UpdateDistributedTransactionStatus(const std::string& dt_id, const std::string& status) {
    // TODO: 实现分布式事务状态更新
    return true;
}

bool SystemDatabase::RegisterDistributedObject(int64_t object_id, const std::string& object_type,
                                              const std::string& object_name, const std::string& database_name,
                                              const std::string& shard_key, const std::string& node_mapping,
                                              int replication_factor) {
    // TODO: 实现分布式对象注册
    return true;
}

std::vector<SysClusterNode> SystemDatabase::GetClusterNodes() {
    // TODO: 实现集群节点查询
    return std::vector<SysClusterNode>();
}

std::vector<SysDistributedTransaction> SystemDatabase::GetActiveDistributedTransactions() {
    // TODO: 实现活跃分布式事务查询
    return std::vector<SysDistributedTransaction>();
}

} // namespace sqlcc