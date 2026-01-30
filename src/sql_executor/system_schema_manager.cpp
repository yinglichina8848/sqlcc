#include "system_schema_manager.h"
#include "system_data_structures.h"
#include <sstream>
#include <iostream>

namespace sqlcc {

// 系统表名称常量
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
const std::string SYS_TABLE_AUDIT_LOGS = "sys_audit_logs";
const std::string SYS_TABLE_AUDIT_POLICIES = "sys_audit_policies";
const std::string SYS_TABLE_TRANSACTIONS = "sys_transactions";
const std::string SYS_TABLE_SAVEPOINTS = "sys_savepoints";
const std::string SYS_TABLE_CLUSTER_NODES = "sys_cluster_nodes";
const std::string SYS_TABLE_DISTRIBUTED_TRANSACTIONS = "sys_distributed_transactions";
const std::string SYS_TABLE_DISTRIBUTED_OBJECTS = "sys_distributed_objects";
const std::string SYS_TABLE_TEMPORAL_TABLES = "sys_temporal_tables";

SystemSchemaManager::SystemSchemaManager(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SystemSchemaManager::~SystemSchemaManager() {
}

bool SystemSchemaManager::CreateAllSystemTables() {
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

bool SystemSchemaManager::TableExists(const std::string& table_name) {
    return db_manager_->TableExists(table_name);
}

std::string SystemSchemaManager::GetLastError() const {
    return last_error_;
}

bool SystemSchemaManager::CreateSysDatabasesTable() {
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

bool SystemSchemaManager::CreateSysUsersTable() {
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

bool SystemSchemaManager::CreateSysRolesTable() {
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

bool SystemSchemaManager::CreateSysTablesTable() {
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

bool SystemSchemaManager::CreateSysColumnsTable() {
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

bool SystemSchemaManager::CreateSysIndexesTable() {
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

bool SystemSchemaManager::CreateSysConstraintsTable() {
    if (db_manager_->TableExists(SYS_TABLE_CONSTRAINTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"constraint_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"constraint_name", "VARCHAR(255) NOT NULL"},
        {"constraint_type", "VARCHAR(50) NOT NULL"},
        {"column_name", "VARCHAR(255) NOT NULL"},
        {"reference_table", "VARCHAR(255)"},
        {"reference_column", "VARCHAR(255)"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_CONSTRAINTS, columns)) {
        SetError("Failed to create sys_constraints table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysViewsTable() {
    if (db_manager_->TableExists(SYS_TABLE_VIEWS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"view_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"view_name", "VARCHAR(255) NOT NULL"},
        {"view_definition", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_VIEWS, columns)) {
        SetError("Failed to create sys_views table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysProceduresTable() {
    if (db_manager_->TableExists(SYS_TABLE_PROCEDURES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"procedure_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"procedure_name", "VARCHAR(255) NOT NULL"},
        {"procedure_definition", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_PROCEDURES, columns)) {
        SetError("Failed to create sys_procedures table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysTriggersTable() {
    if (db_manager_->TableExists(SYS_TABLE_TRIGGERS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"trigger_id", "BIGINT PRIMARY KEY"},
        {"db_id", "BIGINT NOT NULL"},
        {"schema_name", "VARCHAR(255) NOT NULL"},
        {"trigger_name", "VARCHAR(255) NOT NULL"},
        {"table_name", "VARCHAR(255) NOT NULL"},
        {"trigger_type", "VARCHAR(50) NOT NULL"},
        {"trigger_event", "VARCHAR(50) NOT NULL"},
        {"trigger_definition", "TEXT NOT NULL"},
        {"owner", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TRIGGERS, columns)) {
        SetError("Failed to create sys_triggers table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysPrivilegesTable() {
    if (db_manager_->TableExists(SYS_TABLE_PRIVILEGES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"privilege_id", "BIGINT PRIMARY KEY"},
        {"grantee", "VARCHAR(255) NOT NULL"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"object_name", "VARCHAR(255) NOT NULL"},
        {"privilege_type", "VARCHAR(50) NOT NULL"},
        {"grantor", "VARCHAR(255) NOT NULL"},
        {"is_grantable", "BOOLEAN DEFAULT FALSE"},
        {"granted_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_PRIVILEGES, columns)) {
        SetError("Failed to create sys_privileges table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysAuditLogsTable() {
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_LOGS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"log_id", "BIGINT PRIMARY KEY"},
        {"username", "VARCHAR(255) NOT NULL"},
        {"operation_type", "VARCHAR(50) NOT NULL"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"object_name", "VARCHAR(255) NOT NULL"},
        {"sql_statement", "TEXT"},
        {"operation_time", "TIMESTAMP NOT NULL"},
        {"success", "BOOLEAN DEFAULT TRUE"},
        {"error_message", "TEXT"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_LOGS, columns)) {
        SetError("Failed to create sys_audit_logs table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysAuditPoliciesTable() {
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_POLICIES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"policy_id", "BIGINT PRIMARY KEY"},
        {"policy_name", "VARCHAR(255) UNIQUE NOT NULL"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"object_name", "VARCHAR(255)"},
        {"operation_types", "TEXT NOT NULL"},
        {"is_enabled", "BOOLEAN DEFAULT TRUE"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_POLICIES, columns)) {
        SetError("Failed to create sys_audit_policies table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysTransactionsTable() {
    if (db_manager_->TableExists(SYS_TABLE_TRANSACTIONS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"transaction_id", "BIGINT PRIMARY KEY"},
        {"username", "VARCHAR(255) NOT NULL"},
        {"start_time", "TIMESTAMP NOT NULL"},
        {"status", "VARCHAR(50) NOT NULL"},
        {"description", "TEXT"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_transactions table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysSavepointsTable() {
    if (db_manager_->TableExists(SYS_TABLE_SAVEPOINTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"savepoint_id", "BIGINT PRIMARY KEY"},
        {"transaction_id", "BIGINT NOT NULL"},
        {"savepoint_name", "VARCHAR(255) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_SAVEPOINTS, columns)) {
        SetError("Failed to create sys_savepoints table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysClusterNodesTable() {
    if (db_manager_->TableExists(SYS_TABLE_CLUSTER_NODES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"node_id", "BIGINT PRIMARY KEY"},
        {"node_name", "VARCHAR(255) UNIQUE NOT NULL"},
        {"host_address", "VARCHAR(255) NOT NULL"},
        {"port", "INT NOT NULL"},
        {"status", "VARCHAR(50) NOT NULL"},
        {"last_heartbeat", "TIMESTAMP"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_CLUSTER_NODES, columns)) {
        SetError("Failed to create sys_cluster_nodes table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysDistributedTransactionsTable() {
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_TRANSACTIONS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"dist_tx_id", "BIGINT PRIMARY KEY"},
        {"global_tx_id", "VARCHAR(255) NOT NULL"},
        {"node_id", "BIGINT NOT NULL"},
        {"local_tx_id", "BIGINT NOT NULL"},
        {"status", "VARCHAR(50) NOT NULL"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_distributed_transactions table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysDistributedObjectsTable() {
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_OBJECTS)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"object_id", "BIGINT PRIMARY KEY"},
        {"object_name", "VARCHAR(255) NOT NULL"},
        {"object_type", "VARCHAR(50) NOT NULL"},
        {"node_id", "BIGINT NOT NULL"},
        {"is_local", "BOOLEAN DEFAULT FALSE"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_OBJECTS, columns)) {
        SetError("Failed to create sys_distributed_objects table");
        return false;
    }

    return true;
}

bool SystemSchemaManager::CreateSysTemporalTablesTable() {
    if (db_manager_->TableExists(SYS_TABLE_TEMPORAL_TABLES)) {
        return true;
    }

    std::vector<std::pair<std::string, std::string>> columns = {
        {"temporal_table_id", "BIGINT PRIMARY KEY"},
        {"table_id", "BIGINT NOT NULL"},
        {"period_start_column", "VARCHAR(255) NOT NULL"},
        {"period_end_column", "VARCHAR(255) NOT NULL"},
        {"system_versioning", "BOOLEAN DEFAULT FALSE"},
        {"history_table", "VARCHAR(255)"},
        {"created_at", "TIMESTAMP NOT NULL"}
    };

    if (!db_manager_->CreateTable(SYS_TABLE_TEMPORAL_TABLES, columns)) {
        SetError("Failed to create sys_temporal_tables table");
        return false;
    }

    return true;
}

void SystemSchemaManager::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace sqlcc