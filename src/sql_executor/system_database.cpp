/**
 * @file system_database.cpp
 * @brief 系统数据库（元数据目录）核心实现。
 *
 * @WHY
 * 在任何复杂的数据库管理系统（DBMS）中，除了存储用户数据之外，还需要存储关于数据库自身的“数据”，
 * 即**元数据 (Metadata)**。这些元数据包括数据库、表、列、索引、视图、用户、权限等信息。
 * 如果没有一个集中、高效且可靠的元数据管理机制，DBMS 就无法理解和操作其自身结构。
 *
 * `SystemDatabase` 的设计旨在解决以下核心问题：
 * 1.  **自托管元数据 (Self-Hosting Metadata)**: 将数据库的元数据作为普通数据存储在特殊的“系统表”中，
 *     这些系统表位于一个名为 `system` 的专用数据库内。这使得元数据本身可以像普通数据一样被查询、管理和持久化，
 *     极大地简化了数据库内部的实现和维护。
 * 2.  **结构化统一**: 提供统一的接口来访问和操作所有类型的元数据，避免了碎片化的元数据管理逻辑。
 * 3.  **动态可扩展**: 当需要添加新的数据库对象类型或元数据属性时，只需在 `system` 数据库中创建新的系统表或修改现有表结构。
 * 4.  **一致性与持久性**: 作为系统核心，其元数据必须始终保持一致并能抵抗系统故障。
 * 5.  **与外部组件集成**: 方便其他核心组件（如 `UserManager`, `QueryOptimizer` 等）查询和更新元数据。
 *
 * @WHAT
 * `SystemDatabase` 类是 SQLCC 数据库的元数据目录。它不直接存储数据，而是通过构建和执行 SQL 语句，
 * 间接操作存储在 `system` 数据库中的一系列系统表来管理所有元数据。
 *
 * 主要功能包括：
 * -   **初始化**: 检查并创建 `system` 数据库及其所有必需的系统表。
 * -   **元数据 CRUD (创建、读取、更新、删除)**: 为各种数据库对象（数据库、用户、角色、表、列、索引、约束、视图、权限、审计日志、事务、分布式集群等）提供创建、删除、更新和查询的接口。
 * -   **辅助工具**: 提供时间字符串生成、ID 生成等辅助功能。
 * -   **错误处理**: 记录并报告操作过程中遇到的错误。
 *
 * 核心系统表 (例如):
 * -   `sys_databases`: 存储所有数据库的信息。
 * -   `sys_users`: 存储所有用户的信息。
 * -   `sys_tables`: 存储所有表的信息。
 * -   `sys_privileges`: 存储所有权限授予信息。
 * -   `sys_audit_logs`: 存储审计日志。
 * -   等等，本文件定义了十几种系统表的结构和操作。
 *
 * @HOW
 * `SystemDatabase` 通过以下方式实现其功能：
 * 1.  **依赖 `DatabaseManager`**: `SystemDatabase` 持有 `DatabaseManager` 的共享指针，并通过 `DatabaseManager` 提供的底层接口（如 `CreateDatabase`, `UseDatabase`, `CreateTable`, `TableExists` 等）直接操作物理数据库。
 * 2.  **SQL 语句构造与执行**: 对于所有元数据操作，`SystemDatabase` 内部会构造相应的 `INSERT`, `UPDATE`, `DELETE`, `SELECT` SQL 语句。
 * 3.  **`SqlExecutor` 中介**: 构造的 SQL 语句通过 `SqlExecutor` 实例进行执行。`SqlExecutor` 作为更高层级的接口，能够处理 SQL 解析和执行。
 * 4.  **数据库上下文切换**: 在执行针对系统表的 SQL 语句之前，会临时切换到 `system` 数据库，操作完成后再切换回之前的数据库上下文，确保操作的隔离性和正确性。
 * 5.  **自增 ID**: 使用简单的基于时间戳的 `GenerateId` 来为每个元数据记录生成唯一 ID。
 * 6.  **错误管理**: 通过 `last_error_` 成员变量记录操作失败时的详细错误信息。
 *
 * 这种设计模式使得元数据的管理与数据本身的管理方式保持一致，并且易于维护和扩展。
 */

#include "../system_database.h"  // 使用正确的路径
#include "../sql_executor.h"
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>

namespace sqlcc {

/**
 * @brief 构造SystemDatabase实例。
 * @details 初始化SystemDatabase，传入一个DatabaseManager的共享指针，用于底层数据库操作。
 * @param db_manager DatabaseManager的共享指针，提供基础数据库操作接口。
 */
SystemDatabase::SystemDatabase(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
    // TODO(#SYSDB-001): 在实际实现中，SystemDatabase可能需要更直接地与存储层交互，
    // 而不是通过SqlExecutor。当前注释指出“移除SqlExecutor依赖，使用DatabaseManager直接执行操作”，
    // 但目前大部分方法仍然依赖于通过SqlExecutor执行SQL字符串来操作元数据。
    // 这可能需要SystemDatabase内部实现一个更底层的元数据存储接口。
}
/**
 * @brief 销毁SystemDatabase实例。
 * @details 默认析构函数，负责清理SystemDatabase实例占用的资源。
 */
SystemDatabase::~SystemDatabase() {
}

/**
 * @brief 初始化系统数据库及其元数据。
 * @details 该方法是SystemDatabase的入口点，负责检查、创建system数据库，
 * 创建所有必需的系统表，并初始化默认数据。
 * @return 初始化成功返回true，否则返回false。
 */
bool SystemDatabase::Initialize() {
    try {
        // 1. 检查并创建system数据库（如果不存在）。
        if (!Exists()) {
            if (!db_manager_->CreateDatabase(SYSTEM_DB_NAME)) {
                SetError("Failed to create system database");
                return false;
            }
        }

        // 2. 切换到system数据库，以便后续操作。
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return false;
        }

        // 3. 创建所有必需的系统元数据表。
        if (!CreateSystemTables()) {
            return false;
        }

        // 4. 初始化默认数据（如默认用户、角色等）。
        if (!InitializeDefaultData()) { // TODO(#SYSDB-002): 完整实现此方法以初始化默认用户、角色等数据。
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        SetError(std::string("System database initialization failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 检查系统数据库`system`是否存在。
 * @return 如果`system`数据库存在返回true，否则返回false。
 */
bool SystemDatabase::Exists() {
    return db_manager_->DatabaseExists(SYSTEM_DB_NAME);
}
/**
 * @brief 获取当前时间的格式化字符串。
 * @details 返回当前时间，格式为"YYYY-MM-DD HH:MM:SS"。用于元数据记录中的时间戳。
 * @return 格式化后的当前时间字符串。
 */
std::string SystemDatabase::GetCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
/**
 * @brief 生成一个唯一的ID。
 * @details 当前实现使用时间戳作为ID，这在小规模测试中可能足够，但在高并发或长时间运行的生产环境中可能不足以保证唯一性。
 * @param table_name 目标表名，虽然当前未使用，但预留未来可用于基于表的ID生成策略。
 * @return 生成的唯一ID。
 */
int64_t SystemDatabase::GenerateId(const std::string& table_name) {
    // TODO(#SYSDB-003): 在高并发生产环境中，需要实现更健壮的分布式唯一ID生成器，
    // 例如，基于UUID、雪花算法（Snowflake ID）或数据库序列。
    // 简单的ID生成器，使用时间戳
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return timestamp;
}
/**
 * @brief 创建所有必需的系统元数据表。
 * @details 该方法会按顺序调用各个系统表的创建函数，确保所有元数据表在系统启动时都已存在。
 * @return 所有表创建成功返回true，任一失败则返回false。
 */
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
    
    // --- 创建新增的系统表 ---
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
/**
 * @brief 创建`sys_databases`系统元数据表。
 * @details 该表存储了所有数据库的元数据信息，如数据库ID、名称、所有者等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysDatabasesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_DATABASES)) {
        return true;
    }

    // 2. 定义`sys_databases`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"db_id", "BIGINT PRIMARY KEY"},         // 数据库的唯一ID
        {"db_name", "VARCHAR(255) UNIQUE NOT NULL"}, // 数据库名称，唯一且非空
        {"owner", "VARCHAR(255) NOT NULL"},       // 数据库所有者
        {"created_at", "TIMESTAMP NOT NULL"},     // 数据库创建时间
        {"description", "TEXT"}                   // 数据库描述
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_DATABASES, columns)) {
        SetError("Failed to create sys_databases table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_users`系统元数据表。
 * @details 该表存储了所有用户的元数据信息，如用户ID、用户名、密码哈希、角色等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysUsersTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_USERS)) {
        return true;
    }

    // 2. 定义`sys_users`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"user_id", "BIGINT PRIMARY KEY"},               // 用户的唯一ID
        {"username", "VARCHAR(255) UNIQUE NOT NULL"},     // 用户名，唯一且非空
        {"password_hash", "VARCHAR(255) NOT NULL"},       // 密码的哈希值
        {"role", "VARCHAR(255) NOT NULL"},                // 用户的主角色
        {"current_role", "VARCHAR(255)"},                 // 用户当前激活的角色（可变）
        {"is_active", "BOOLEAN DEFAULT TRUE"},            // 用户是否活跃
        {"created_at", "TIMESTAMP NOT NULL"}              // 用户创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_USERS, columns)) {
        SetError("Failed to create sys_users table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_roles`系统元数据表。
 * @details 该表存储了所有角色的元数据信息，如角色ID、名称、创建时间等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysRolesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_ROLES)) {
        return true;
    }

    // 2. 定义`sys_roles`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"role_id", "BIGINT PRIMARY KEY"},           // 角色的唯一ID
        {"role_name", "VARCHAR(255) UNIQUE NOT NULL"}, // 角色名称，唯一且非空
        {"created_at", "TIMESTAMP NOT NULL"}          // 角色创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_ROLES, columns)) {
        SetError("Failed to create sys_roles table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_tables`系统元数据表。
 * @details 该表存储了所有表的元数据信息，如表ID、所属数据库ID、表名、所有者、创建时间等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysTablesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_TABLES)) {
        return true;
    }

    // 2. 定义`sys_tables`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"table_id", "BIGINT PRIMARY KEY"},               // 表的唯一ID
        {"db_id", "BIGINT NOT NULL"},                     // 表所属数据库的ID
        {"schema_name", "VARCHAR(255) NOT NULL"},         // 表所属Schema名称
        {"table_name", "VARCHAR(255) NOT NULL"},          // 表名称
        {"owner", "VARCHAR(255) NOT NULL"},               // 表的所有者
        {"created_at", "TIMESTAMP NOT NULL"},             // 表的创建时间
        {"table_type", "VARCHAR(50) DEFAULT 'BASE TABLE'"} // 表的类型（如BASE TABLE, VIEW）
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_TABLES, columns)) {
        SetError("Failed to create sys_tables table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_columns`系统元数据表。
 * @details 该表存储了所有列的元数据信息，如列ID、所属表ID、列名、数据类型、可空性等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysColumnsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_COLUMNS)) {
        return true;
    }

    // 2. 定义`sys_columns`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"column_id", "BIGINT PRIMARY KEY"},           // 列的唯一ID
        {"table_id", "BIGINT NOT NULL"},               // 列所属表的ID
        {"column_name", "VARCHAR(255) NOT NULL"},      // 列名称
        {"data_type", "VARCHAR(100) NOT NULL"},        // 列的数据类型
        {"is_nullable", "BOOLEAN DEFAULT TRUE"},       // 列是否可为空
        {"default_value", "TEXT"},                     // 列的默认值
        {"ordinal_position", "INT NOT NULL"}           // 列在表中的顺序位置
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_COLUMNS, columns)) {
        SetError("Failed to create sys_columns table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_indexes`系统元数据表。
 * @details 该表存储了所有索引的元数据信息，如索引ID、所属表ID、索引名、列名、唯一性等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysIndexesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_INDEXES)) {
        return true;
    }

    // 2. 定义`sys_indexes`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"index_id", "BIGINT PRIMARY KEY"},           // 索引的唯一ID
        {"table_id", "BIGINT NOT NULL"},              // 索引所属表的ID
        {"index_name", "VARCHAR(255) NOT NULL"},      // 索引名称
        {"column_name", "VARCHAR(255) NOT NULL"},     // 索引包含的列名
        {"is_unique", "BOOLEAN DEFAULT FALSE"},       // 索引是否唯一
        {"index_type", "VARCHAR(50) DEFAULT 'BTREE'"},// 索引类型（例如B树、哈希）
        {"created_at", "TIMESTAMP NOT NULL"}          // 索引创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_INDEXES, columns)) {
        SetError("Failed to create sys_indexes table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_constraints`系统元数据表。
 * @details 该表存储了所有表约束的元数据信息，如约束ID、所属表ID、约束名、类型、涉及列等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysConstraintsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_CONSTRAINTS)) {
        return true;
    }

    // 2. 定义`sys_constraints`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"constraint_id", "BIGINT PRIMARY KEY"},           // 约束的唯一ID
        {"table_id", "BIGINT NOT NULL"},                  // 约束所属表的ID
        {"constraint_name", "VARCHAR(255) NOT NULL"},     // 约束名称
        {"constraint_type", "VARCHAR(50) NOT NULL"},      // 约束类型（如PRIMARY KEY, FOREIGN KEY, CHECK）
        {"column_name", "VARCHAR(255)"},                  // 约束涉及的列名（可选）
        {"check_expression", "TEXT"},                     // CHECK约束的表达式
        {"referenced_table", "VARCHAR(255)"},             // 外键约束引用的表
        {"referenced_column", "VARCHAR(255)"}             // 外键约束引用的列
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_CONSTRAINTS, columns)) {
        SetError("Failed to create sys_constraints table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_views`系统元数据表。
 * @details 该表存储了所有视图的元数据信息，如视图ID、所属数据库ID、视图名、定义语句等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysViewsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_VIEWS)) {
        return true;
    }

    // 2. 定义`sys_views`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"view_id", "BIGINT PRIMARY KEY"},               // 视图的唯一ID
        {"db_id", "BIGINT NOT NULL"},                     // 视图所属数据库的ID
        {"schema_name", "VARCHAR(255) NOT NULL"},         // 视图所属Schema名称
        {"view_name", "VARCHAR(255) NOT NULL"},           // 视图名称
        {"definition", "TEXT NOT NULL"},                  // 视图的定义SQL语句
        {"owner", "VARCHAR(255) NOT NULL"},               // 视图所有者
        {"created_at", "TIMESTAMP NOT NULL"}              // 视图创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_VIEWS, columns)) {
        SetError("Failed to create sys_views table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_procedures`系统元数据表。
 * @details 该表存储了所有存储过程的元数据信息，如过程ID、所属数据库ID、过程名、定义等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysProceduresTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_PROCEDURES)) {
        return true;
    }

    // 2. 定义`sys_procedures`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"proc_id", "BIGINT PRIMARY KEY"},               // 存储过程的唯一ID
        {"db_id", "BIGINT NOT NULL"},                     // 存储过程所属数据库的ID
        {"schema_name", "VARCHAR(255) NOT NULL"},         // 存储过程所属Schema名称
        {"proc_name", "VARCHAR(255) NOT NULL"},           // 存储过程名称
        {"definition", "TEXT NOT NULL"},                  // 存储过程的定义SQL代码
        {"owner", "VARCHAR(255) NOT NULL"},               // 存储过程所有者
        {"created_at", "TIMESTAMP NOT NULL"}              // 存储过程创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_PROCEDURES, columns)) {
        SetError("Failed to create sys_procedures table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_triggers`系统元数据表。
 * @details 该表存储了所有触发器的元数据信息，如触发器ID、所属表ID、触发器名、类型、触发器体等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysTriggersTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_TRIGGERS)) {
        return true;
    }

    // 2. 定义`sys_triggers`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"trigger_id", "BIGINT PRIMARY KEY"},           // 触发器的唯一ID
        {"table_id", "BIGINT NOT NULL"},                // 触发器所属表的ID
        {"trigger_name", "VARCHAR(255) NOT NULL"},      // 触发器名称
        {"trigger_type", "VARCHAR(100) NOT NULL"},      // 触发器类型（例如BEFORE INSERT, AFTER UPDATE）
        {"trigger_body", "TEXT NOT NULL"},              // 触发器执行的SQL或代码体
        {"owner", "VARCHAR(255) NOT NULL"},             // 触发器所有者
        {"created_at", "TIMESTAMP NOT NULL"}            // 触发器创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_TRIGGERS, columns)) {
        SetError("Failed to create sys_triggers table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_privileges`系统元数据表。
 * @details 该表存储了所有权限授予的元数据信息，如权限ID、被授予者类型/名称、作用对象和权限类型等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysPrivilegesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_PRIVILEGES)) {
        return true;
    }

    // 2. 定义`sys_privileges`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"privilege_id", "BIGINT PRIMARY KEY"},             // 权限记录的唯一ID
        {"grantee_type", "VARCHAR(10) NOT NULL"},           // 被授予者类型 (USER 或 ROLE)
        {"grantee_name", "VARCHAR(255) NOT NULL"},          // 被授予者的名称
        {"db_name", "VARCHAR(255)"},                        // 权限所属的数据库名称（可选）
        {"table_name", "VARCHAR(255)"},                     // 权限所属的表名称（可选）
        {"privilege", "VARCHAR(50) NOT NULL"},              // 具体的权限类型（如 SELECT, INSERT, ALL）
        {"grantor", "VARCHAR(255) NOT NULL"},               // 授予该权限的用户
        {"granted_at", "TIMESTAMP NOT NULL"}                // 权限授予时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_PRIVILEGES, columns)) {
        SetError("Failed to create sys_privileges table");
        return false;
    }

    return true;
}
// --- 新增系统表创建方法 ---

/**
 * @brief 创建`sys_audit_logs`系统元数据表。
 * @details 该表存储了数据库的所有审计日志记录，用于安全审计和合规性检查。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysAuditLogsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_LOGS)) {
        return true;
    }

    // 2. 定义`sys_audit_logs`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"log_id", "BIGINT PRIMARY KEY"},             // 审计日志记录的唯一ID
        {"user_name", "VARCHAR(255) NOT NULL"},       // 执行操作的用户
        {"operation_type", "VARCHAR(50) NOT NULL"},   // 操作类型 (如SELECT, INSERT, LOGIN)
        {"object_type", "VARCHAR(50)"},               // 操作对象类型 (如TABLE, DATABASE)
        {"object_name", "VARCHAR(255)"},              // 操作对象名称
        {"operation_time", "TIMESTAMP NOT NULL"},     // 操作发生时间
        {"client_ip", "VARCHAR(45)"},                 // 客户端IP地址
        {"session_id", "VARCHAR(255)"},               // 会话ID
        {"sql_text", "TEXT"},                         // 执行的SQL语句文本
        {"affected_rows", "INT"},                     // 影响的行数
        {"execution_result", "VARCHAR(20)"}           // 执行结果 (如SUCCESS, FAILED)
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_LOGS, columns)) {
        SetError("Failed to create sys_audit_logs table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_audit_policies`系统元数据表。
 * @details 该表存储了数据库的审计策略，定义了需要审计哪些操作和对象。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysAuditPoliciesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_AUDIT_POLICIES)) {
        return true;
    }

    // 2. 定义`sys_audit_policies`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"policy_id", "BIGINT PRIMARY KEY"},           // 审计策略的唯一ID
        {"object_type", "VARCHAR(50) NOT NULL"},      // 审计对象类型 (如TABLE, DATABASE, USER)
        {"object_name", "VARCHAR(255)"},              // 审计对象名称（可选，表示所有同类型对象）
        {"operation_type", "VARCHAR(50) NOT NULL"},   // 审计操作类型 (如SELECT, INSERT, ALTER)
        {"is_enabled", "BOOLEAN DEFAULT TRUE"},       // 策略是否启用
        {"created_at", "TIMESTAMP NOT NULL"},         // 策略创建时间
        {"updated_at", "TIMESTAMP NOT NULL"}          // 策略最后更新时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_AUDIT_POLICIES, columns)) {
        SetError("Failed to create sys_audit_policies table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_transactions`系统元数据表。
 * @details 该表存储了系统中所有事务的元数据信息，用于事务监控和恢复。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysTransactionsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_TRANSACTIONS)) {
        return true;
    }

    // 2. 定义`sys_transactions`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"transaction_id", "VARCHAR(255) PRIMARY KEY"}, // 事务的唯一ID
        {"session_id", "VARCHAR(255)"},                 // 事务所属会话ID
        {"user_name", "VARCHAR(255)"},                  // 事务发起用户
        {"start_time", "TIMESTAMP NOT NULL"},           // 事务开始时间
        {"end_time", "TIMESTAMP"},                      // 事务结束时间
        {"status", "VARCHAR(20) NOT NULL"},             // 事务状态 (如ACTIVE, COMMITTED, ABORTED)
        {"isolation_level", "VARCHAR(20)"},             // 事务隔离级别
        {"client_ip", "VARCHAR(45)"}                    // 客户端IP地址
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_transactions table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_savepoints`系统元数据表。
 * @details 该表存储了事务中所有保存点的元数据信息，用于事务的部分回滚。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysSavepointsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_SAVEPOINTS)) {
        return true;
    }

    // 2. 定义`sys_savepoints`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"savepoint_id", "BIGINT PRIMARY KEY"},           // 保存点的唯一ID
        {"transaction_id", "VARCHAR(255) NOT NULL"},      // 保存点所属事务的ID
        {"savepoint_name", "VARCHAR(255) NOT NULL"},      // 保存点名称
        {"created_at", "TIMESTAMP NOT NULL"}              // 保存点创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_SAVEPOINTS, columns)) {
        SetError("Failed to create sys_savepoints table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_cluster_nodes`系统元数据表。
 * @details 该表存储了数据库集群中所有节点的元数据信息，用于分布式管理和监控。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysClusterNodesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_CLUSTER_NODES)) {
        return true;
    }

    // 2. 定义`sys_cluster_nodes`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"node_id", "VARCHAR(255) PRIMARY KEY"},    // 节点的唯一ID
        {"node_name", "VARCHAR(255) NOT NULL"},     // 节点名称
        {"host_address", "VARCHAR(255) NOT NULL"},  // 节点主机地址
        {"port", "INT NOT NULL"},                   // 节点服务端口
        {"status", "VARCHAR(20) NOT NULL"},         // 节点状态 (如ONLINE, OFFLINE, RECOVERING)
        {"role", "VARCHAR(20) NOT NULL"},           // 节点角色 (如MASTER, REPLICA, WORKER)
        {"joined_at", "TIMESTAMP NOT NULL"},        // 节点加入集群时间
        {"last_heartbeat", "TIMESTAMP"}             // 最后一次心跳时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_CLUSTER_NODES, columns)) {
        SetError("Failed to create sys_cluster_nodes table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_distributed_transactions`系统元数据表。
 * @details 该表存储了分布式事务的元数据信息，用于协调跨节点事务的一致性。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysDistributedTransactionsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_TRANSACTIONS)) {
        return true;
    }

    // 2. 定义`sys_distributed_transactions`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"dt_id", "VARCHAR(255) PRIMARY KEY"},           // 分布式事务的唯一ID
        {"coordinator_node", "VARCHAR(255) NOT NULL"},  // 协调该分布式事务的节点ID
        {"status", "VARCHAR(20) NOT NULL"},             // 分布式事务状态 (如PENDING, COMMITTED, ABORTED)
        {"created_at", "TIMESTAMP NOT NULL"},           // 事务创建时间
        {"updated_at", "TIMESTAMP NOT NULL"},           // 事务最后更新时间
        {"timeout_seconds", "INT DEFAULT 30"}           // 事务超时时间（秒）
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_TRANSACTIONS, columns)) {
        SetError("Failed to create sys_distributed_transactions table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_distributed_objects`系统元数据表。
 * @details 该表存储了分布式数据库中对象的元数据信息，如分片键、节点映射等。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysDistributedObjectsTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_DISTRIBUTED_OBJECTS)) {
        return true;
    }

    // 2. 定义`sys_distributed_objects`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"object_id", "BIGINT PRIMARY KEY"},           // 分布式对象的唯一ID
        {"object_type", "VARCHAR(50) NOT NULL"},      // 对象类型 (如TABLE, INDEX)
        {"object_name", "VARCHAR(255) NOT NULL"},      // 对象名称
        {"database_name", "VARCHAR(255) NOT NULL"},    // 对象所属数据库
        {"shard_key", "VARCHAR(255)"},                  // 分布式对象的Sharding Key
        {"node_mapping", "TEXT"},                       // 对象到节点的映射信息 (如JSON格式)
        {"replication_factor", "INT DEFAULT 1"},       // 对象的复制因子
        {"created_at", "TIMESTAMP NOT NULL"}            // 对象创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_DISTRIBUTED_OBJECTS, columns)) {
        SetError("Failed to create sys_distributed_objects table");
        return false;
    }

    return true;
}
/**
 * @brief 创建`sys_temporal_tables`系统元数据表。
 * @details 该表存储了时态表的元数据信息，支持数据随时间变化的查询。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSysTemporalTablesTable() {
    // 1. 检查表是否已存在，如果存在则直接返回true。
    if (db_manager_->TableExists(SYS_TABLE_TEMPORAL_TABLES)) {
        return true;
    }

    // 2. 定义`sys_temporal_tables`表的列结构。
    std::vector<std::pair<std::string, std::string>> columns = {
        {"temporal_id", "BIGINT PRIMARY KEY"},           // 时态表的唯一ID
        {"table_id", "BIGINT NOT NULL"},                  // 关联的普通表的ID
        {"system_time_start_column", "VARCHAR(255) NOT NULL"}, // 系统时间周期开始列
        {"system_time_end_column", "VARCHAR(255) NOT NULL"},   // 系统时间周期结束列
        {"period_start", "TIMESTAMP NOT NULL"},           // 时态数据有效期的开始时间
        {"period_end", "TIMESTAMP"},                      // 时态数据有效期的结束时间
        {"retention_period_days", "INT"},                 // 数据保留期限（天）
        {"created_at", "TIMESTAMP NOT NULL"}              // 时态表创建时间
    };

    // 3. 调用DatabaseManager创建表。
    if (!db_manager_->CreateTable(SYS_TABLE_TEMPORAL_TABLES, columns)) {
        SetError("Failed to create sys_temporal_tables table");
        return false;
    }

    return true;
}
/**
 * @brief 初始化系统数据库的默认数据。
 * @details 该方法负责向`system`数据库中的`sys_users`、`sys_roles`等表插入必要的默认记录，
 * 例如创建默认的超级用户和系统角色。
 * @return 初始化成功返回true，否则返回false。
 */
bool SystemDatabase::InitializeDefaultData() {
    // TODO(#SYSDB-002): 完整实现此方法以初始化默认用户、角色等数据。
    // 这将涉及构造SQL INSERT语句并使用ExecuteSQL执行。
    // 例如，创建初始管理员账户，以及'public'等默认角色。
    // 这里需要实现默认数据的插入逻辑
    return true;
}
/**
 * @brief 执行SQL语句的辅助方法。
 * @details 该方法通过`SqlExecutor`执行给定的SQL字符串。
 * 它是`SystemDatabase`操作元数据（通过SQL语句）的底层机制。
 * @param sql 待执行的SQL语句字符串。
 * @return SQL语句执行成功返回true，否则返回false。
 */
bool SystemDatabase::ExecuteSQL(const std::string& sql) {
    try {
        // 1. 检查DatabaseManager是否可用。
        if (!db_manager_) {
            SetError("Database manager is not available");
            return false;
        }
        
        // 2. 使用SqlExecutor执行SQL语句。
        SqlExecutor executor(db_manager_); // 每次都创建新的Executor实例
        std::string result = executor.Execute(sql);
        
        // 3. 检查执行结果是否包含错误信息。
        // TODO(#SYSDB-004): SqlExecutor的Execute方法应返回结构化结果，而非字符串，以便更可靠地检查错误。
        if (result.find("Error") != std::string::npos || result.find("ERROR") != std::string::npos) {
            SetError(result);
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("ExecuteSQL exception: ") + e.what());
        return false;
    }
}
/**
 * @brief 执行SELECT查询语句的辅助方法。
 * @details 该方法通过`SqlExecutor`执行给定的SELECT SQL字符串，并返回查询结果。
 * 它是`SystemDatabase`获取元数据信息的底层机制。
 * @param sql 待执行的SELECT SQL语句字符串。
 * @return SELECT查询结果的字符串表示。如果执行失败，返回空字符串，错误信息存储在last_error_中。
 */
std::string SystemDatabase::ExecuteSelectQuery(const std::string& sql) {
    try {
        // 1. 检查DatabaseManager是否可用。
        if (!db_manager_) {
            SetError("Database manager is not available");
            return "";
        }
        
        // 2. 使用SqlExecutor执行SELECT查询。
        SqlExecutor executor(db_manager_); // 每次都创建新的Executor实例
        std::string result = executor.Execute(sql);
        
        // TODO(#SYSDB-005): SqlExecutor的Execute方法应返回结构化结果，而非字符串，以便SystemDatabase解析元数据。
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("ExecuteSelectQuery exception: ") + e.what());
        return "";
    }
}
// 数据库元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的数据库元数据记录。
 * @details 记录数据库ID、名称、所有者、创建时间等信息到`sys_databases`表。
 * @param db_name 数据库名称。
 * @param owner 数据库所有者。
 * @param description 数据库描述。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateDatabaseRecord(const std::string& db_name, const std::string& owner, const std::string& description) {
    try {
        // 1. 生成数据库ID。
        int64_t db_id = GenerateId(SYS_TABLE_DATABASES); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_DATABASES 
           << " (db_id, db_name, owner, created_at, description) VALUES ("
           << db_id << ", '"
           << db_name << "', '"
           << owner << "', '"
           << current_time << "', '"
           << description << "')";
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateDatabaseRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条数据库元数据记录。
 * @details 从`sys_databases`表删除指定数据库名称的记录。
 * @param db_name 待删除数据库的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropDatabaseRecord(const std::string& db_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_DATABASES 
           << " WHERE db_name = '" << db_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db);
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropDatabaseRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定数据库的元数据记录。
 * @param db_name 待获取数据库的名称。
 * @return 包含数据库元数据`SysDatabase`结构体。如果未找到，返回一个默认构造的`SysDatabase`。
 */
SysDatabase SystemDatabase::GetDatabaseRecord(const std::string& db_name) {
    // TODO(#SYSDB-006): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句，通过ExecuteSelectQuery执行，然后将返回的字符串结果解析为SysDatabase结构体。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)db_name; // 避免未使用参数警告
    return SysDatabase{};
}
/**
 * @brief 列出系统数据库中所有数据库的元数据记录。
 * @return 包含所有数据库元数据`SysDatabase`结构体的向量。
 */
std::vector<SysDatabase> SystemDatabase::ListDatabases() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_databases），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysDatabase结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    return std::vector<SysDatabase>();
}
/**
 * @brief 检查指定名称的数据库在系统数据库中是否存在。
 * @details 通过查询`sys_databases`表，检查是否存在`db_name`匹配的记录。
 * @param db_name 待检查数据库的名称。
 * @return 如果数据库存在返回true，否则返回false。
 */
bool SystemDatabase::DatabaseExists(const std::string& db_name) {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_DATABASES
           << " WHERE db_name = '" << db_name << "'";
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            SetError("Failed to switch to system database");
            return false;
        }
        
        std::string result = ExecuteSelectQuery(ss.str()); // TODO(#SYSDB-005): ExecuteSelectQuery返回结构化结果。
        
        if (!prev_db.empty()) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            db_manager_->UseDatabase(prev_db);
        }
        
        // 3. 解析结果，检查COUNT是否大于0。
        // TODO(#SYSDB-005): 简化解析逻辑：目前通过字符串查找'1'到'9'，在生产环境应通过结构化方式解析COUNT值。
        if (result.find("1") != std::string::npos || result.find("2") != std::string::npos || 
            result.find("3") != std::string::npos || result.find("4") != std::string::npos ||
            result.find("5") != std::string::npos || result.find("6") != std::string::npos ||
            result.find("7") != std::string::npos || result.find("8") != std::string::npos ||
            result.find("9") != std::string::npos) {
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        SetError(std::string("DatabaseExists exception: ") + e.what());
        return false;
    }
}
// 用户元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的用户元数据记录。
 * @details 记录用户ID、用户名、密码哈希、角色等信息到`sys_users`表。
 * @param username 用户名。
 * @param password_hash 密码的哈希值。
 * @param role 用户的主角色。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role) {
    try {
        // 1. 生成用户ID。
        int64_t user_id = GenerateId(SYS_TABLE_USERS); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
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
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateUserRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条用户元数据记录。
 * @details 从`sys_users`表删除指定用户名的记录。
 * @param username 待删除用户的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropUserRecord(const std::string& username) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_USERS
           << " WHERE username = '" << username << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropUserRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 更新系统数据库中的一条用户元数据记录。
 * @details 根据提供的`SysUser`结构体更新`sys_users`表中对应用户名的记录。
 * @param user 包含更新信息的用户结构体。
 * @return 更新成功返回true，否则返回false。
 */
bool SystemDatabase::UpdateUserRecord(const SysUser& user) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_USERS << " SET "
           << "password_hash = '" << user.password_hash << "', "
           << "role = '" << user.role << "', "
           << "current_role = '" << user.current_role << "', "
           << "is_active = " << (user.is_active ? "1" : "0")  // 使用1/0而不是TRUE/FALSE
           << " WHERE username = '" << user.username << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("UpdateUserRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定用户名的元数据记录。
 * @param username 待获取用户的名称。
 * @return 包含用户元数据`SysUser`结构体。如果未找到，返回一个默认构造的`SysUser`。
 */
SysUser SystemDatabase::GetUserRecord(const std::string& username) {
    // TODO(#SYSDB-006): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句，通过ExecuteSelectQuery执行，然后将返回的字符串结果解析为SysUser结构体。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)username; // 避免未使用参数警告
    return SysUser{};
}
/**
 * @brief 列出系统数据库中所有用户的元数据记录。
 * @return 包含所有用户元数据`SysUser`结构体的向量。
 */
std::vector<SysUser> SystemDatabase::ListUsers() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_users），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysUser结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    return std::vector<SysUser>();
}
/**
 * @brief 检查指定用户名的用户在系统数据库中是否存在。
 * @details 通过查询`sys_users`表，检查是否存在`username`匹配的记录。
 * @param username 待检查用户的名称。
 * @return 如果用户存在返回true，否则返回false。
 */
bool SystemDatabase::UserExists(const std::string& username) {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_USERS
           << " WHERE username = '" << username << "'";
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            SetError("Failed to switch to system database");
            return false;
        }
        
        std::string result = ExecuteSelectQuery(ss.str()); // TODO(#SYSDB-005): ExecuteSelectQuery返回结构化结果。
        
        if (!prev_db.empty()) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            db_manager_->UseDatabase(prev_db);
        }
        
        // 3. 解析结果，检查COUNT是否大于0。
        // TODO(#SYSDB-005): 简化解析逻辑：目前通过字符串查找'1'到'9'，在生产环境应通过结构化方式解析COUNT值。
        if (result.find("1") != std::string::npos || result.find("2") != std::string::npos || 
            result.find("3") != std::string::npos || result.find("4") != std::string::npos ||
            result.find("5") != std::string::npos || result.find("6") != std::string::npos ||
            result.find("7") != std::string::npos || result.find("8") != std::string::npos ||
            result.find("9") != std::string::npos) {
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        SetError(std::string("UserExists exception: ") + e.what());
        return false;
    }
}
// 角色元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的角色元数据记录。
 * @details 记录角色ID、名称、创建时间等信息到`sys_roles`表。
 * @param role_name 角色名称。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateRoleRecord(const std::string& role_name) {
    try {
        // 1. 生成角色ID。
        int64_t role_id = GenerateId(SYS_TABLE_ROLES); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_ROLES
           << " (role_id, role_name, created_at) VALUES ("
           << role_id << ", '"
           << role_name << "', '"
           << current_time << "')";
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateRoleRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条角色元数据记录。
 * @details 从`sys_roles`表删除指定角色名称的记录。
 * @param role_name 待删除角色的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropRoleRecord(const std::string& role_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_ROLES
           << " WHERE role_name = '" << role_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropRoleRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定角色的元数据记录。
 * @param role_name 待获取角色的名称。
 * @return 包含角色元数据`SysRole`结构体。如果未找到，返回一个默认构造的`SysRole`。
 */
SysRole SystemDatabase::GetRoleRecord(const std::string& role_name) {
    // TODO(#SYSDB-006): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句，通过ExecuteSelectQuery执行，然后将返回的字符串结果解析为SysRole结构体。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)role_name; // 避免未使用参数警告
    return SysRole{};
}
/**
 * @brief 列出系统数据库中所有角色的元数据记录。
 * @return 包含所有角色元数据`SysRole`结构体的向量。
 */
std::vector<SysRole> SystemDatabase::ListRoles() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_roles），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysRole结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    return std::vector<SysRole>();
}
/**
 * @brief 检查指定名称的角色在系统数据库中是否存在。
 * @details 通过查询`sys_roles`表，检查是否存在`role_name`匹配的记录。
 * @param role_name 待检查角色的名称。
 * @return 如果角色存在返回true，否则返回false。
 */
bool SystemDatabase::RoleExists(const std::string& role_name) {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_ROLES
           << " WHERE role_name = '" << role_name << "'";
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            SetError("Failed to switch to system database");
            return false;
        }
        
        std::string result = ExecuteSelectQuery(ss.str()); // TODO(#SYSDB-005): ExecuteSelectQuery返回结构化结果。
        
        if (!prev_db.empty()) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            db_manager_->UseDatabase(prev_db);
        }
        
        // 3. 解析结果，检查COUNT是否大于0。
        // TODO(#SYSDB-005): 简化解析逻辑：目前通过字符串查找'1'到'9'，在生产环境应通过结构化方式解析COUNT值。
        if (result.find("1") != std::string::npos || result.find("2") != std::string::npos || 
            result.find("3") != std::string::npos || result.find("4") != std::string::npos ||
            result.find("5") != std::string::npos || result.find("6") != std::string::npos ||
            result.find("7") != std::string::npos || result.find("8") != std::string::npos ||
            result.find("9") != std::string::npos) {
            return true;
        }
        
        return false;
    } catch (const std::exception& e) {
        SetError(std::string("RoleExists exception: ") + e.what());
        return false;
    }
}
// 表元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的表元数据记录。
 * @details 记录表ID、所属数据库ID、Schema名称、表名、所有者、表类型和创建时间到`sys_tables`表。
 * @param db_id 表所属数据库的ID。
 * @param schema_name 表所属的Schema名称。
 * @param table_name 表名称。
 * @param owner 表的所有者。
 * @param table_type 表的类型（如"BASE TABLE", "VIEW"）。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                                      const std::string& owner, const std::string& table_type) {
    try {
        // 1. 生成表ID。
        int64_t table_id = GenerateId(SYS_TABLE_TABLES); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
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
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateTableRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条表元数据记录。
 * @details 从`sys_tables`表删除指定Schema和表名的记录。
 * @param schema_name 表所属的Schema名称。
 * @param table_name 表名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropTableRecord(const std::string& schema_name, const std::string& table_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_TABLES
           << " WHERE schema_name = '" << schema_name << "'"
           << " AND table_name = '" << table_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropTableRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定表名的元数据记录。
 * @param schema_name 表所属的Schema名称。
 * @param table_name 待获取表的名称。
 * @return 包含表元数据`SysTable`结构体。如果未找到，返回一个默认构造的`SysTable`。
 */
SysTable SystemDatabase::GetTableRecord(const std::string& schema_name, const std::string& table_name) {
    // TODO(#SYSDB-006): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句，通过ExecuteSelectQuery执行，然后将返回的字符串结果解析为SysTable结构体。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)schema_name; // 避免未使用参数警告
    (void)table_name; // 避免未使用参数警告
    return SysTable{};
}
/**
 * @brief 列出系统数据库中指定数据库的所有表的元数据记录。
 * @param db_id 待列出表的数据库ID。
 * @return 包含所有表元数据`SysTable`结构体的向量。
 */
std::vector<SysTable> SystemDatabase::ListTables(int64_t db_id) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_tables WHERE db_id = ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysTable结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)db_id; // 避免未使用参数警告
    return std::vector<SysTable>();
}
/**
 * @brief 检查指定Schema和名称的表在系统数据库中是否存在。
 * @details 通过查询`sys_tables`表，检查是否存在`schema_name`和`table_name`匹配的记录。
 * @param schema_name 表所属的Schema名称。
 * @param table_name 待检查表的名称。
 * @return 如果表存在返回true，否则返回false。
 */
bool SystemDatabase::TableExists(const std::string& schema_name, const std::string& table_name) {
    try {
        // 1. 构建SELECT SQL语句，检查是否存在匹配的表记录。
        std::stringstream ss;
        ss << "SELECT table_id FROM " << SYS_TABLE_TABLES
           << " WHERE schema_name = '" << schema_name << "'"
           << " AND table_name = '" << table_name << "'";
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            SetError("Failed to switch to system database");
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果并正确解析。
        
        if (!prev_db.empty()) {
            // TODO(#SYSDB-008): 错误处理应更健壮，确保在切换数据库失败时能够恢复或抛出异常。
            db_manager_->UseDatabase(prev_db);
        }
        
        // TODO(#SYSDB-005): 这里需要解析ExecuteSQL的返回值来判断表是否存在，
        // 目前ExecuteSQL只返回boolean，不能区分“查询无结果”和“查询出错”。
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("TableExists exception: ") + e.what()); // 设置错误信息
        return false;
    }
}
// 列元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的列元数据记录。
 * @details 记录列ID、所属表ID、列名、数据类型、可空性、默认值和顺序位置到`sys_columns`表。
 * @param table_id 列所属表的ID。
 * @param column_name 列名称。
 * @param data_type 列的数据类型。
 * @param is_nullable 列是否可为空。
 * @param default_value 列的默认值。
 * @param ordinal_position 列在表中的顺序位置。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                                       bool is_nullable, const std::string& default_value, int ordinal_position) {
    try {
        // 1. 生成列ID。
        int64_t column_id = GenerateId(SYS_TABLE_COLUMNS); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        
        // 2. 构建INSERT SQL语句。
        std::stringstream ss;
        ss << "INSERT INTO " << SYS_TABLE_COLUMNS
           << " (column_id, table_id, column_name, data_type, is_nullable, default_value, ordinal_position) VALUES ("
           << column_id << ", "
           << table_id << ", '"
           << column_name << "', '"
           << data_type << "', "
           << (is_nullable ? "1" : "0") << ", '"  // 使用1/0表示BOOLEAN
           << default_value << "', "
           << ordinal_position << ")";
        
        // 3. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateColumnRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条列元数据记录。
 * @details 从`sys_columns`表删除指定表ID和列名的记录。
 * @param table_id 列所属表的ID。
 * @param column_name 待删除列的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropColumnRecord(int64_t table_id, const std::string& column_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_COLUMNS
           << " WHERE table_id = " << table_id
           << " AND column_name = '" << column_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropColumnRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定表的所有列的元数据记录。
 * @param table_id 待获取列的表ID。
 * @return 包含所有列元数据`SysColumn`结构体的向量。
 */
std::vector<SysColumn> SystemDatabase::GetTableColumns(int64_t table_id) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_columns WHERE table_id = ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysColumn结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)table_id; // 避免未使用参数警告
    return std::vector<SysColumn>();
}
/**
 * @brief 更新系统数据库中的一条列元数据记录。
 * @details 根据提供的参数更新`sys_columns`表中指定表ID和列名的记录。
 * @param table_id 列所属表的ID。
 * @param column_name 待更新列的名称。
 * @param new_data_type 列的新数据类型。
 * @param new_is_nullable 列新的可空性。
 * @param new_default_value 列新的默认值。
 * @return 更新成功返回true，否则返回false。
 */
bool SystemDatabase::UpdateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& new_data_type,
                                       bool new_is_nullable, const std::string& new_default_value) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_COLUMNS
           << " SET data_type = '" << new_data_type << "', "
           << "is_nullable = " << (new_is_nullable ? "1" : "0") << ", "
           << "default_value = '" << new_default_value << "' "
           << "WHERE table_id = " << table_id
           << " AND column_name = '" << column_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("UpdateColumnRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 重命名系统数据库中的一条列元数据记录。
 * @details 更新`sys_columns`表中指定表ID和旧列名的记录，将其名称改为新列名。
 * @param table_id 列所属表的ID。
 * @param old_column_name 列当前的名称。
 * @param new_column_name 列的新名称。
 * @return 重命名成功返回true，否则返回false。
 */
bool SystemDatabase::RenameColumnRecord(int64_t table_id, const std::string& old_column_name, const std::string& new_column_name) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_COLUMNS
           << " SET column_name = '" << new_column_name << "' "
           << "WHERE table_id = " << table_id
           << " AND column_name = '" << old_column_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RenameColumnRecord failed: ") + e.what());
        return false;
    }
}
// 索引元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的索引元数据记录。
 * @details 记录索引ID、所属表ID、索引名、列名、唯一性和类型到`sys_indexes`表。
 * @param table_id 索引所属表的ID。
 * @param index_name 索引名称。
 * @param column_name 索引包含的列名。
 * @param is_unique 索引是否唯一。
 * @param index_type 索引类型。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                                      bool is_unique, const std::string& index_type) {
    try {
        // 1. 生成索引ID。
        int64_t index_id = GenerateId(SYS_TABLE_INDEXES); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
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
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateIndexRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条索引元数据记录。
 * @details 从`sys_indexes`表删除指定表ID和索引名称的记录。
 * @param table_id 索引所属表的ID。
 * @param index_name 待删除索引的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropIndexRecord(int64_t table_id, const std::string& index_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_INDEXES
           << " WHERE table_id = " << table_id
           << " AND index_name = '" << index_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropIndexRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定表的所有索引的元数据记录。
 * @param table_id 待获取索引的表ID。
 * @return 包含所有索引元数据`SysIndex`结构体的向量。
 */
std::vector<SysIndex> SystemDatabase::GetTableIndexes(int64_t table_id) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_indexes WHERE table_id = ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysIndex结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)table_id; // 避免未使用参数警告
    return std::vector<SysIndex>();
}
/**
 * @brief 重命名系统数据库中的一条索引元数据记录。
 * @details 更新`sys_indexes`表中指定表ID和旧索引名的记录，将其名称改为新索引名。
 * @param table_id 索引所属表的ID。
 * @param old_index_name 索引当前的名称。
 * @param new_index_name 索引的新名称。
 * @return 重命名成功返回true，否则返回false。
 */
bool SystemDatabase::RenameIndexRecord(int64_t table_id, const std::string& old_index_name, const std::string& new_index_name) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_INDEXES
           << " SET index_name = '" << new_index_name << "' "
           << "WHERE table_id = " << table_id
           << " AND index_name = '" << old_index_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RenameIndexRecord failed: ") + e.what());
        return false;
    }
}
// 约束元数据操作实现
/**
 * @brief 在系统数据库中创建一条新的约束元数据记录。
 * @details 记录约束ID、所属表ID、约束名、类型、涉及列、检查表达式和引用信息到`sys_constraints`表。
 * @param table_id 约束所属表的ID。
 * @param constraint_name 约束名称。
 * @param constraint_type 约束类型（如"PRIMARY KEY", "FOREIGN KEY", "CHECK"）。
 * @param column_name 约束涉及的列名（可选）。
 * @param check_expression CHECK约束的表达式。
 * @param referenced_table 外键约束引用的表名。
 * @param referenced_column 外键约束引用的列名。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, const std::string& constraint_type,
                                           const std::string& column_name, const std::string& check_expression,
                                           const std::string& referenced_table, const std::string& referenced_column) {
    try {
        // 1. 生成约束ID。
        int64_t constraint_id = GenerateId(SYS_TABLE_CONSTRAINTS); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
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
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CreateConstraintRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条约束元数据记录。
 * @details 从`sys_constraints`表删除指定表ID和约束名称的记录。
 * @param table_id 约束所属表的ID。
 * @param constraint_name 待删除约束的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropConstraintRecord(int64_t table_id, const std::string& constraint_name) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_CONSTRAINTS
           << " WHERE table_id = " << table_id
           << " AND constraint_name = '" << constraint_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("DropConstraintRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定表的所有约束的元数据记录。
 * @param table_id 待获取约束的表ID。
 * @return 包含所有约束元数据`SysConstraint`结构体的向量。
 */
std::vector<SysConstraint> SystemDatabase::GetTableConstraints(int64_t table_id) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_constraints WHERE table_id = ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysConstraint结构体向量。
    // 这需要QueryExecutor支持返回结构化数据或SystemDatabase内部实现解析逻辑。
    (void)table_id; // 避免未使用参数警告
    return std::vector<SysConstraint>();
}
/**
 * @brief 重命名系统数据库中的一条约束元数据记录。
 * @details 更新`sys_constraints`表中指定表ID和旧约束名的记录，将其名称改为新约束名。
 * @param table_id 约束所属表的ID。
 * @param old_constraint_name 约束当前的名称。
 * @param new_constraint_name 约束的新名称。
 * @return 重命名成功返回true，否则返回false。
 */
bool SystemDatabase::RenameConstraintRecord(int64_t table_id, const std::string& old_constraint_name, const std::string& new_constraint_name) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_CONSTRAINTS
           << " SET constraint_name = '" << new_constraint_name << "' "
           << "WHERE table_id = " << table_id
           << " AND constraint_name = '" << old_constraint_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RenameConstraintRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 重命名系统数据库中的一条表元数据记录。
 * @details 更新`sys_tables`表中指定Schema和旧表名的记录，将其名称改为新表名。
 * @param schema_name 表所属的Schema名称。
 * @param old_table_name 表当前的名称。
 * @param new_table_name 表的新名称。
 * @return 重命名成功返回true，否则返回false。
 */
bool SystemDatabase::RenameTableRecord(const std::string& schema_name, const std::string& old_table_name, const std::string& new_table_name) {
    try {
        // 1. 构建UPDATE SQL语句。
        std::stringstream ss;
        ss << "UPDATE " << SYS_TABLE_TABLES
           << " SET table_name = '" << new_table_name << "' "
           << "WHERE schema_name = '" << schema_name << "' "
           << "AND table_name = '" << old_table_name << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RenameTableRecord failed: ") + e.what());
        return false;
    }
}
// --- 视图元数据操作实现 ---
/**
 * @brief 在系统数据库中创建一条新的视图元数据记录。
 * @param db_id 视图所属数据库的ID。
 * @param schema_name 视图所属的Schema名称。
 * @param view_name 视图名称。
 * @param definition 视图的定义SQL语句。
 * @param owner 视图所有者。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                                     const std::string& definition, const std::string& owner) {
    // TODO(#SYSDB-009): 需要实现视图记录创建。
    // 这将涉及生成view_id，获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)db_id; // 避免未使用参数警告
    (void)schema_name; // 避免未使用参数警告
    (void)view_name; // 避免未使用参数警告
    (void)definition; // 避免未使用参数警告
    (void)owner; // 避免未使用参数警告
    return true;
}
/**
 * @brief 从系统数据库中删除一条视图元数据记录。
 * @param schema_name 视图所属的Schema名称。
 * @param view_name 待删除视图的名称。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::DropViewRecord(const std::string& schema_name, const std::string& view_name) {
    // TODO(#SYSDB-010): 需要实现视图记录删除。
    // 这将涉及构建DELETE SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)schema_name; // 避免未使用参数警告
    (void)view_name; // 避免未使用参数警告
    return true;
}
/**
 * @brief 从系统数据库中获取指定视图的元数据记录。
 * @param schema_name 视图所属的Schema名称。
 * @param view_name 待获取视图的名称。
 * @return 包含视图元数据`SysView`结构体。如果未找到，返回一个默认构造的`SysView`。
 */
SysView SystemDatabase::GetViewRecord(const std::string& schema_name, const std::string& view_name) {
    // TODO(#SYSDB-006): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句，通过ExecuteSelectQuery执行，然后将返回的字符串结果解析为SysView结构体。
    (void)schema_name; // 避免未使用参数警告
    (void)view_name; // 避免未使用参数警告
    return SysView{};
}
/**
 * @brief 列出系统数据库中指定数据库的所有视图的元数据记录。
 * @param db_id 待列出视图的数据库ID。
 * @return 包含所有视图元数据`SysView`结构体的向量。
 */
std::vector<SysView> SystemDatabase::ListViews(int64_t db_id) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_views WHERE db_id = ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysView结构体向量。
    (void)db_id; // 避免未使用参数警告
    return std::vector<SysView>();
}
// --- 权限元数据操作实现 ---
/**
 * @brief 在系统数据库中创建一条新的权限授予记录。
 * @details 记录权限ID、被授予者类型/名称、作用对象和权限类型等信息到`sys_privileges`表。
 * @param grantee_type 被授予者类型（USER或ROLE）。
 * @param grantee_name 被授予者名称。
 * @param db_name 权限所属的数据库名称。
 * @param table_name 权限所属的表名称。
 * @param privilege 具体的权限类型。
 * @param grantor 授予权限的用户。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::GrantPrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                                         const std::string& db_name, const std::string& table_name,
                                         const std::string& privilege, const std::string& grantor) {
    try {
        // 1. 生成权限ID。
        int64_t privilege_id = GenerateId(SYS_TABLE_PRIVILEGES); // TODO(#SYSDB-003): GenerateId在生产环境需要更健壮。
        // 2. 获取当前时间字符串。
        std::string current_time = GetCurrentTimeString();
        
        // 3. 构建INSERT SQL语句。
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
        
        // 4. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("GrantPrivilegeRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中删除一条权限授予记录。
 * @details 从`sys_privileges`表删除指定被授予者类型/名称、作用对象和权限类型的记录。
 * @param grantee_type 被授予者类型。
 * @param grantee_name 被授予者名称。
 * @param db_name 权限所属的数据库名称。
 * @param table_name 权限所属的表名称。
 * @param privilege 具体的权限类型。
 * @return 删除成功返回true，否则返回false。
 */
bool SystemDatabase::RevokePrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                                          const std::string& db_name, const std::string& table_name,
                                          const std::string& privilege) {
    try {
        // 1. 构建DELETE SQL语句。
        std::stringstream ss;
        ss << "DELETE FROM " << SYS_TABLE_PRIVILEGES
           << " WHERE grantee_type = '" << grantee_type << "'"
           << " AND grantee_name = '" << grantee_name << "'"
           << " AND db_name = '" << db_name << "'"
           << " AND table_name = '" << table_name << "'"
           << " AND privilege = '" << privilege << "'";
        
        // 2. 切换到system数据库执行操作，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("RevokePrivilegeRecord failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 从系统数据库中获取指定用户的所有权限元数据记录。
 * @param username 待获取权限的用户名称。
 * @return 包含用户权限`SysPrivilege`结构体的向量。
 */
std::vector<SysPrivilege> SystemDatabase::GetUserPrivileges(const std::string& username) {
    std::vector<SysPrivilege> result;
    
    // 保存原数据库
    std::string old_db;
    if (db_manager_) {
        old_db = db_manager_->GetCurrentDatabase();
    }
    
    try {
        // 切换到system数据库
        if (!db_manager_->UseDatabase("system")) { // TODO(#SYSDB-008): 错误处理应更健壮。
            // TODO(#SYSDB-011): "system"数据库名称应使用常量 SYSTEM_DB_NAME。
            SetError("Failed to switch to system database for GetUserPrivileges");
            return result;
        }
        
        // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
        // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_privileges WHERE grantee_name = ...），通过ExecuteSelectQuery执行，
        // 然后将返回的字符串结果解析为SysPrivilege结构体向量。
        // 目前暂时返回空结果，后续需要实现直接查询表的逻辑
        
        // 恢复原数据库
        if (!old_db.empty()) {
            db_manager_->UseDatabase(old_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
    } catch (const std::exception& e) {
        SetError(std::string("GetUserPrivileges exception: ") + e.what());
        // 恢复原数据库
        if (!old_db.empty()) {
            db_manager_->UseDatabase(old_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
    }
    
    return result;
}
// --- 审计功能实现 ---
/**
 * @brief 在系统数据库中创建一条新的审计日志记录。
 * @details 记录用户操作的详细信息到`sys_audit_logs`表，用于安全审计。
 * @param user_name 执行操作的用户。
 * @param operation_type 操作类型（如"SELECT", "INSERT", "LOGIN"）。
 * @param object_type 操作对象类型（如"TABLE", "DATABASE"）。
 * @param object_name 操作对象名称。
 * @param client_ip 客户端IP地址。
 * @param session_id 会话ID。
 * @param sql_text 执行的SQL语句文本。
 * @param affected_rows 影响的行数。
 * @param execution_result 执行结果（如"SUCCESS", "FAILED"）。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateAuditLog(const std::string& user_name, const std::string& operation_type,
                                   const std::string& object_type, const std::string& object_name,
                                   const std::string& client_ip, const std::string& session_id,
                                   const std::string& sql_text, int affected_rows,
                                   const std::string& execution_result) {
    // TODO(#SYSDB-012): 需要实现审计日志创建。
    // 这将涉及生成log_id，获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)user_name; // 避免未使用参数警告
    (void)operation_type; // 避免未使用参数警告
    (void)object_type; // 避免未使用参数警告
    (void)object_name; // 避免未使用参数警告
    (void)client_ip; // 避免未使用参数警告
    (void)session_id; // 避免未使用参数警告
    (void)sql_text; // 避免未使用参数警告
    (void)affected_rows; // 避免未使用参数警告
    (void)execution_result; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中创建一条新的审计策略记录。
 * @details 记录审计策略的详细信息到`sys_audit_policies`表。
 * @param object_type 审计对象类型。
 * @param object_name 审计对象名称。
 * @param operation_type 审计操作类型。
 * @param is_enabled 策略是否启用。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateAuditPolicy(const std::string& object_type, const std::string& object_name,
                                      const std::string& operation_type, bool is_enabled) {
    // TODO(#SYSDB-013): 需要实现审计策略创建。
    // 这将涉及生成policy_id，获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)object_type; // 避免未使用参数警告
    (void)object_name; // 避免未使用参数警告
    (void)operation_type; // 避免未使用参数警告
    (void)is_enabled; // 避免未使用参数警告
    return true;
}
/**
 * @brief 从系统数据库中查询审计日志记录。
 * @param start_time 查询的起始时间。
 * @param end_time 查询的结束时间。
 * @return 包含审计日志`SysAuditLog`结构体的向量。
 */
std::vector<SysAuditLog> SystemDatabase::GetAuditLogs(time_t start_time, time_t end_time) {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_audit_logs WHERE operation_time BETWEEN ...），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysAuditLog结构体向量。
    (void)start_time; // 避免未使用参数警告
    (void)end_time; // 避免未使用参数警告
    return std::vector<SysAuditLog>();
}
/**
 * @brief 从系统数据库中查询所有审计策略记录。
 * @return 包含审计策略`SysAuditPolicy`结构体的向量。
 */
std::vector<SysAuditPolicy> SystemDatabase::GetAuditPolicies() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_audit_policies），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysAuditPolicy结构体向量。
    return std::vector<SysAuditPolicy>();
}
// --- 事务元数据操作实现 ---
/**
 * @brief 在系统数据库中创建一条新的事务元数据记录。
 * @param transaction_id 事务的唯一ID。
 * @param session_id 事务所属会话的ID。
 * @param user_name 事务发起用户的名称。
 * @param client_ip 客户端IP地址。
 * @param isolation_level 事务的隔离级别。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateTransactionRecord(const std::string& transaction_id, const std::string& session_id,
                                           const std::string& user_name, const std::string& client_ip,
                                           const std::string& isolation_level) {
    // TODO(#SYSDB-014): 需要实现事务记录创建。
    // 这将涉及获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)transaction_id; // 避免未使用参数警告
    (void)session_id; // 避免未使用参数警告
    (void)user_name; // 避免未使用参数警告
    (void)client_ip; // 避免未使用参数警告
    (void)isolation_level; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中更新一条事务元数据记录的状态。
 * @param transaction_id 待更新事务的ID。
 * @param status 事务的新状态（如"COMMITTED", "ABORTED"）。
 * @param end_time 事务结束时间。
 * @return 更新成功返回true，否则返回false。
 */
bool SystemDatabase::UpdateTransactionStatus(const std::string& transaction_id, const std::string& status,
                                            time_t end_time) {
    // TODO(#SYSDB-015): 需要实现事务状态更新。
    // 这将涉及构建UPDATE SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)transaction_id; // 避免未使用参数警告
    (void)status; // 避免未使用参数警告
    (void)end_time; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中创建一条新的保存点元数据记录。
 * @param transaction_id 保存点所属事务的ID。
 * @param savepoint_name 保存点的名称。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateSavepointRecord(const std::string& transaction_id, const std::string& savepoint_name) {
    // TODO(#SYSDB-016): 需要实现保存点记录创建。
    // 这将涉及生成savepoint_id，获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)transaction_id; // 避免未使用参数警告
    (void)savepoint_name; // 避免未使用参数警告
    return true;
}
/**
 * @brief 从系统数据库中查询所有活跃事务的元数据记录。
 * @return 包含活跃事务`SysTransaction`结构体的向量。
 */
std::vector<SysTransaction> SystemDatabase::GetActiveTransactions() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_transactions WHERE status = 'ACTIVE'），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysTransaction结构体向量。
    return std::vector<SysTransaction>();
}
// --- 分布式元数据操作实现 ---
/**
 * @brief 在系统数据库中注册一个新的集群节点。
 * @param node_id 节点的唯一ID。
 * @param node_name 节点名称。
 * @param host_address 节点的主机地址。
 * @param port 节点的端口。
 * @param role 节点在集群中的角色。
 * @return 注册成功返回true，否则返回false。
 */
bool SystemDatabase::RegisterClusterNode(const std::string& node_id, const std::string& node_name,
                                       const std::string& host_address, int port,
                                       const std::string& role) {
    // TODO(#SYSDB-017): 需要实现集群节点注册。
    // 这将涉及获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)node_id; // 避免未使用参数警告
    (void)node_name; // 避免未使用参数警告
    (void)host_address; // 避免未使用参数警告
    (void)port; // 避免未使用参数警告
    (void)role; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中更新集群节点的元数据记录。
 * @param node_id 待更新节点的唯一ID。
 * @param status 节点的新状态。
 * @param last_heartbeat 最后一次心跳时间。
 * @return 更新成功返回true，否则返回false。
 */
bool SystemDatabase::UpdateNodeStatus(const std::string& node_id, const std::string& status,
                                     time_t last_heartbeat) {
    // TODO(#SYSDB-018): 需要实现节点状态更新。
    // 这将涉及构建UPDATE SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)node_id; // 避免未使用参数警告
    (void)status; // 避免未使用参数警告
    (void)last_heartbeat; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中创建一条新的分布式事务元数据记录。
 * @param dt_id 分布式事务的唯一ID。
 * @param coordinator_node 协调该分布式事务的节点ID。
 * @return 创建成功返回true，否则返回false。
 */
bool SystemDatabase::CreateDistributedTransaction(const std::string& dt_id, const std::string& coordinator_node) {
    // TODO(#SYSDB-019): 需要实现分布式事务创建。
    // 这将涉及获取当前时间，构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)dt_id; // 避免未使用参数警告
    (void)coordinator_node; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中更新分布式事务的状态。
 * @param dt_id 待更新分布式事务的唯一ID。
 * @param status 事务的新状态。
 * @return 更新成功返回true，否则返回false。
 */
bool SystemDatabase::UpdateDistributedTransactionStatus(const std::string& dt_id, const std::string& status) {
    // TODO(#SYSDB-020): 需要实现分布式事务状态更新。
    // 这将涉及构建UPDATE SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)dt_id; // 避免未使用参数警告
    (void)status; // 避免未使用参数警告
    return true;
}
/**
 * @brief 在系统数据库中注册一个新的分布式对象。
 * @param object_id 对象的唯一ID。
 * @param object_type 对象类型。
 * @param object_name 对象名称。
 * @param database_name 对象所属数据库名称。
 * @param shard_key 分片键。
 * @param node_mapping 节点映射信息。
 * @param replication_factor 复制因子。
 * @return 注册成功返回true，否则返回false。
 */
bool SystemDatabase::RegisterDistributedObject(int64_t object_id, const std::string& object_type,
                                              const std::string& object_name, const std::string& database_name,
                                              const std::string& shard_key, const std::string& node_mapping,
                                              int replication_factor) {
    // TODO(#SYSDB-021): 需要实现分布式对象注册。
    // 这将涉及构建INSERT SQL语句，并执行SQL。
    // 还需要处理数据库上下文切换和错误管理。
    (void)object_id; // 避免未使用参数警告
    (void)object_type; // 避免未使用参数警告
    (void)object_name; // 避免未使用参数警告
    (void)database_name; // 避免未使用参数警告
    (void)shard_key; // 避免未使用参数警告
    (void)node_mapping; // 避免未使用参数警告
    (void)replication_factor; // 避免未使用参数警告
    return true;
}
/**
 * @brief 从系统数据库中查询所有集群节点的元数据记录。
 * @return 包含所有集群节点`SysClusterNode`结构体的向量。
 */
std::vector<SysClusterNode> SystemDatabase::GetClusterNodes() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_cluster_nodes），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysClusterNode结构体向量。
    return std::vector<SysClusterNode>();
}
/**
 * @brief 从系统数据库中查询所有活跃分布式事务的元数据记录。
 * @return 包含活跃分布式事务`SysDistributedTransaction`结构体的向量。
 */
std::vector<SysDistributedTransaction> SystemDatabase::GetActiveDistributedTransactions() {
    // TODO(#SYSDB-007): 需要实现SELECT查询并解析结果。
    // 这将涉及构建SELECT SQL语句（SELECT * FROM sys_distributed_transactions WHERE status = 'ACTIVE'），通过ExecuteSelectQuery执行，
    // 然后将返回的字符串结果解析为SysDistributedTransaction结构体向量。
    return std::vector<SysDistributedTransaction>();
}
// --- 元数据一致性检查实现 ---
/**
 * @brief 检查系统数据库的基本一致性。
 * @details 该方法目前仅检查`sys_databases`表是否存在。
 * @return 如果检查通过返回true，否则返回false。
 */
bool SystemDatabase::CheckDatabaseConsistency() {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句，检查`sys_databases`表是否存在。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_DATABASES;
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CheckDatabaseConsistency failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 检查指定表的元数据一致性。
 * @details 该方法目前仅检查`sys_tables`表中指定Schema和名称的表记录是否存在。
 * @param schema_name 表所属的Schema名称。
 * @param table_name 待检查表的名称。
 * @return 如果检查通过返回true，否则返回false。
 */
bool SystemDatabase::CheckTableConsistency(const std::string& schema_name, const std::string& table_name) {
    try {
        // 1. 构建SELECT SQL语句，检查`sys_tables`表中指定的表是否存在。
        std::stringstream ss;
        ss << "SELECT table_id FROM " << SYS_TABLE_TABLES
           << " WHERE schema_name = '" << schema_name << "'"
           << " AND table_name = '" << table_name << "'";
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CheckTableConsistency failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 检查指定表的列元数据一致性。
 * @details 该方法目前仅检查`sys_columns`表中是否存在指定表ID的列记录。
 * @param table_id 待检查列的表ID。
 * @return 如果检查通过返回true，否则返回false。
 */
bool SystemDatabase::CheckColumnConsistency(int64_t table_id) {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句，检查`sys_columns`表中是否存在指定表的列。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_COLUMNS
           << " WHERE table_id = " << table_id;
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CheckColumnConsistency failed: ") + e.what());
        return false;
    }
}
/**
 * @brief 检查指定表的索引元数据一致性。
 * @details 该方法目前仅检查`sys_indexes`表中是否存在指定表ID的索引记录。
 * @param table_id 待检查索引的表ID。
 * @return 如果检查通过返回true，否则返回false。
 */
bool SystemDatabase::CheckIndexConsistency(int64_t table_id) {
    try {
        // 1. 构建SELECT COUNT(*) SQL语句，检查`sys_indexes`表中是否存在指定表的索引。
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_INDEXES
           << " WHERE table_id = " << table_id;
        
        // 2. 切换到system数据库执行查询，然后切换回原数据库。
        std::string prev_db = db_manager_->GetCurrentDatabase();
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to switch to system database"); // TODO(#SYSDB-008): 错误处理应更健壮。
            return false;
        }
        
        bool result = ExecuteSQL(ss.str()); // TODO(#SYSDB-004): ExecuteSQL返回结构化结果。
        
        if (!prev_db.empty()) {
            db_manager_->UseDatabase(prev_db); // TODO(#SYSDB-008): 错误处理应更健壮。
        }
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("CheckIndexConsistency failed: ") + e.what());
        return false;
    }
}
bool SystemDatabase::CheckConstraintConsistency(int64_t table_id) {
    try {
        // 检查指定表的约束在sys_constraints中是否存在且结构正确
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_CONSTRAINTS
           << " WHERE table_id = " << table_id;
        
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
        SetError(std::string("CheckConstraintConsistency failed: ") + e.what());
        return false;
    }
}

bool SystemDatabase::CheckPrivilegeConsistency(const std::string& grantee_name) {
    try {
        // 检查指定用户的权限在sys_privileges中是否存在且结构正确
        std::stringstream ss;
        ss << "SELECT COUNT(*) FROM " << SYS_TABLE_PRIVILEGES
           << " WHERE grantee_name = '" << grantee_name << "'";
        
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
        SetError(std::string("CheckPrivilegeConsistency failed: ") + e.what());
        return false;
    }
}

} // namespace sqlcc