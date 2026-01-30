#include "src/system_metadata_manager.h"
#include "src/system_data_structures.h"
#include "src/system_database_new.h"  // 包含SYSTEM_DB_NAME常量定义
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace sqlcc {

// 系统表名称常量
const std::string SYS_TABLE_DATABASES = "sys_databases";
const std::string SYS_TABLE_TABLES = "sys_tables";
const std::string SYS_TABLE_COLUMNS = "sys_columns";
const std::string SYS_TABLE_INDEXES = "sys_indexes";
const std::string SYS_TABLE_CONSTRAINTS = "sys_constraints";
const std::string SYS_TABLE_VIEWS = "sys_views";


SystemMetadataManager::SystemMetadataManager(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SystemMetadataManager::~SystemMetadataManager() {
}

// 数据库元数据操作
bool SystemMetadataManager::CreateDatabaseRecord(const std::string& db_name, const std::string& owner, 
                                                const std::string& description) {
    try {
        // 检查数据库是否已存在
        if (DatabaseExists(db_name)) {
            SetError("Database already exists: " + db_name);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成数据库ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_DATABASES << " (db_id, db_name, owner, created_at, description) VALUES ("
            << timestamp << ", '" << db_name << "', '" << owner << "', '"
            << current_time << "', '" << description << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create database record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropDatabaseRecord(const std::string& db_name) {
    try {
        // 检查数据库是否存在
        if (!DatabaseExists(db_name)) {
            SetError("Database does not exist: " + db_name);
            return false;
        }
        
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_DATABASES << " WHERE db_name = '" << db_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop database record: ") + e.what());
        return false;
    }
}

SysDatabase SystemMetadataManager::GetDatabaseRecord(const std::string& db_name) {
    SysDatabase database;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT db_id, db_name, owner, created_at, description FROM "
            << SYS_TABLE_DATABASES << " WHERE db_name = '" << db_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].size() < 5) {
            SetError("Database not found: " + db_name);
            return database;
        }
        
        // 填充数据库信息
        database.db_id = std::stoll(result[0][0]);
        database.db_name = result[0][1];
        database.owner = result[0][2];
        database.created_at = result[0][3];
        database.description = result[0][4];
        
        return database;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get database record: ") + e.what());
        return database;
    }
}

std::vector<SysDatabase> SystemMetadataManager::ListDatabases() {
    std::vector<SysDatabase> databases;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT db_id, db_name, owner, created_at, description FROM "
            << SYS_TABLE_DATABASES << " ORDER BY db_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充数据库列表
        for (const auto& row : result) {
            if (row.size() >= 5) {
                SysDatabase database;
                database.db_id = std::stoll(row[0]);
                database.db_name = row[1];
                database.owner = row[2];
                database.created_at = row[3];
                database.description = row[4];
                databases.push_back(database);
            }
        }
        
        return databases;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to list databases: ") + e.what());
        return databases;
    }
}

bool SystemMetadataManager::DatabaseExists(const std::string& db_name) {
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_DATABASES << " WHERE db_name = '" << db_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check database existence: ") + e.what());
        return false;
    }
}

// 表元数据操作
bool SystemMetadataManager::CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                                              const std::string& owner, const std::string& table_type) {
    try {
        // 检查表是否已存在
        if (TableExists(schema_name, table_name)) {
            SetError("Table already exists: " + schema_name + "." + table_name);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成表ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_TABLES << " (table_id, db_id, schema_name, table_name, owner, created_at, table_type) VALUES ("
            << timestamp << ", " << db_id << ", '" << schema_name << "', '" << table_name << "', '"
            << owner << "', '" << current_time << "', '" << table_type << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create table record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropTableRecord(const std::string& schema_name, const std::string& table_name) {
    try {
        // 检查表是否存在
        if (!TableExists(schema_name, table_name)) {
            SetError("Table does not exist: " + schema_name + "." + table_name);
            return false;
        }
        
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_TABLES 
            << " WHERE schema_name = '" << schema_name << "' AND table_name = '" << table_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop table record: ") + e.what());
        return false;
    }
}

SysTable SystemMetadataManager::GetTableRecord(const std::string& schema_name, const std::string& table_name) {
    SysTable table;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT table_id, db_id, schema_name, table_name, owner, created_at, table_type FROM "
            << SYS_TABLE_TABLES << " WHERE schema_name = '" << schema_name 
            << "' AND table_name = '" << table_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].size() < 7) {
            SetError("Table not found: " + schema_name + "." + table_name);
            return table;
        }
        
        // 填充表信息
        table.table_id = std::stoll(result[0][0]);
        table.db_id = std::stoll(result[0][1]);
        table.schema_name = result[0][2];
        table.table_name = result[0][3];
        table.owner = result[0][4];
        table.created_at = result[0][5];
        table.table_type = result[0][6];
        
        return table;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get table record: ") + e.what());
        return table;
    }
}

std::vector<SysTable> SystemMetadataManager::ListTables(int64_t db_id) {
    std::vector<SysTable> tables;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT table_id, db_id, schema_name, table_name, owner, created_at, table_type FROM "
            << SYS_TABLE_TABLES << " WHERE db_id = " << db_id << " ORDER BY schema_name, table_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充表列表
        for (const auto& row : result) {
            if (row.size() >= 7) {
                SysTable table;
                table.table_id = std::stoll(row[0]);
                table.db_id = std::stoll(row[1]);
                table.schema_name = row[2];
                table.table_name = row[3];
                table.owner = row[4];
                table.created_at = row[5];
                table.table_type = row[6];
                tables.push_back(table);
            }
        }
        
        return tables;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to list tables: ") + e.what());
        return tables;
    }
}

bool SystemMetadataManager::TableExists(const std::string& schema_name, const std::string& table_name) {
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_TABLES 
            << " WHERE schema_name = '" << schema_name << "' AND table_name = '" << table_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check table existence: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::RenameTableRecord(const std::string& schema_name, const std::string& old_table_name, 
                                              const std::string& new_table_name) {
    try {
        // 检查表是否存在
        if (!TableExists(schema_name, old_table_name)) {
            SetError("Table does not exist: " + schema_name + "." + old_table_name);
            return false;
        }
        
        // 检查新表名是否已存在
        if (TableExists(schema_name, new_table_name)) {
            SetError("Table already exists: " + schema_name + "." + new_table_name);
            return false;
        }
        
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_TABLES << " SET table_name = '" << new_table_name
            << "' WHERE schema_name = '" << schema_name << "' AND table_name = '" << old_table_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to rename table record: ") + e.what());
        return false;
    }
}

// 列元数据操作
bool SystemMetadataManager::CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                                              bool is_nullable, const std::string& default_value, int ordinal_position) {
    try {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成列ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_COLUMNS << " (column_id, table_id, column_name, data_type, is_nullable, default_value, ordinal_position) VALUES ("
            << timestamp << ", " << table_id << ", '" << column_name << "', '" << data_type << "', "
            << (is_nullable ? "TRUE" : "FALSE") << ", '" << default_value << "', " << ordinal_position << ")";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create column record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropColumnRecord(int64_t table_id, const std::string& column_name) {
    try {
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_COLUMNS 
            << " WHERE table_id = " << table_id << " AND column_name = '" << column_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop column record: ") + e.what());
        return false;
    }
}

std::vector<SysColumn> SystemMetadataManager::GetTableColumns(int64_t table_id) {
    std::vector<SysColumn> columns;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT column_id, table_id, column_name, data_type, is_nullable, default_value, ordinal_position FROM "
            << SYS_TABLE_COLUMNS << " WHERE table_id = " << table_id << " ORDER BY ordinal_position";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充列列表
        for (const auto& row : result) {
            if (row.size() >= 7) {
                SysColumn column;
                column.column_id = std::stoll(row[0]);
                column.table_id = std::stoll(row[1]);
                column.column_name = row[2];
                column.data_type = row[3];
                column.is_nullable = (row[4] == "TRUE" || row[4] == "true" || row[4] == "1");
                column.default_value = row[5];
                column.ordinal_position = std::stoi(row[6]);
                columns.push_back(column);
            }
        }
        
        return columns;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get table columns: ") + e.what());
        return columns;
    }
}

bool SystemMetadataManager::UpdateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                                               bool is_nullable, const std::string& default_value) {
    try {
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_COLUMNS << " SET "
            << "data_type = '" << data_type << "', "
            << "is_nullable = " << (is_nullable ? "TRUE" : "FALSE") << ", "
            << "default_value = '" << default_value << "'"
            << " WHERE table_id = " << table_id << " AND column_name = '" << column_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to update column record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::RenameColumnRecord(int64_t table_id, const std::string& old_column_name, 
                                               const std::string& new_column_name) {
    try {
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_COLUMNS << " SET column_name = '" << new_column_name
            << "' WHERE table_id = " << table_id << " AND column_name = '" << old_column_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to rename column record: ") + e.what());
        return false;
    }
}

// 索引元数据操作
bool SystemMetadataManager::CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                                             bool is_unique, const std::string& index_type) {
    try {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成索引ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_INDEXES << " (index_id, table_id, index_name, column_name, is_unique, index_type, created_at) VALUES ("
            << timestamp << ", " << table_id << ", '" << index_name << "', '" << column_name << "', "
            << (is_unique ? "TRUE" : "FALSE") << ", '" << index_type << "', '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create index record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropIndexRecord(int64_t table_id, const std::string& index_name) {
    try {
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_INDEXES 
            << " WHERE table_id = " << table_id << " AND index_name = '" << index_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop index record: ") + e.what());
        return false;
    }
}

std::vector<SysIndex> SystemMetadataManager::GetTableIndexes(int64_t table_id) {
    std::vector<SysIndex> indexes;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT index_id, table_id, index_name, column_name, is_unique, index_type, created_at FROM "
            << SYS_TABLE_INDEXES << " WHERE table_id = " << table_id << " ORDER BY index_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充索引列表
        for (const auto& row : result) {
            if (row.size() >= 7) {
                SysIndex index;
                index.index_id = std::stoll(row[0]);
                index.table_id = std::stoll(row[1]);
                index.index_name = row[2];
                index.column_name = row[3];
                index.is_unique = (row[4] == "TRUE" || row[4] == "true" || row[4] == "1");
                index.index_type = row[5];
                index.created_at = row[6];
                indexes.push_back(index);
            }
        }
        
        return indexes;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get table indexes: ") + e.what());
        return indexes;
    }
}

bool SystemMetadataManager::RenameIndexRecord(int64_t table_id, const std::string& old_index_name, 
                                              const std::string& new_index_name) {
    try {
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_INDEXES << " SET index_name = '" << new_index_name
            << "' WHERE table_id = " << table_id << " AND index_name = '" << old_index_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to rename index record: ") + e.what());
        return false;
    }
}

// 约束元数据操作
bool SystemMetadataManager::CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, 
                                                   const std::string& constraint_type, const std::string& column_name,
                                                   const std::string& reference_table, const std::string& reference_column) {
    try {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成约束ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_CONSTRAINTS << " (constraint_id, table_id, constraint_name, constraint_type, column_name, reference_table, reference_column, created_at) VALUES ("
            << timestamp << ", " << table_id << ", '" << constraint_name << "', '" << constraint_type << "', '"
            << column_name << "', '" << reference_table << "', '" << reference_column << "', '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create constraint record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropConstraintRecord(int64_t table_id, const std::string& constraint_name) {
    try {
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_CONSTRAINTS 
            << " WHERE table_id = " << table_id << " AND constraint_name = '" << constraint_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop constraint record: ") + e.what());
        return false;
    }
}

std::vector<SysConstraint> SystemMetadataManager::GetTableConstraints(int64_t table_id) {
    std::vector<SysConstraint> constraints;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT constraint_id, table_id, constraint_name, constraint_type, column_name, reference_table, reference_column, created_at FROM "
            << SYS_TABLE_CONSTRAINTS << " WHERE table_id = " << table_id << " ORDER BY constraint_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充约束列表
        for (const auto& row : result) {
            if (row.size() >= 8) {
                SysConstraint constraint;
                constraint.constraint_id = std::stoll(row[0]);
                constraint.table_id = std::stoll(row[1]);
                constraint.constraint_name = row[2];
                constraint.constraint_type = row[3];
                constraint.column_name = row[4];
                constraint.reference_table = row[5];
                constraint.reference_column = row[6];
                constraint.created_at = row[7];
                constraints.push_back(constraint);
            }
        }
        
        return constraints;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get table constraints: ") + e.what());
        return constraints;
    }
}

bool SystemMetadataManager::RenameConstraintRecord(int64_t table_id, const std::string& old_constraint_name, 
                                                   const std::string& new_constraint_name) {
    try {
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_CONSTRAINTS << " SET constraint_name = '" << new_constraint_name
            << "' WHERE table_id = " << table_id << " AND constraint_name = '" << old_constraint_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to rename constraint record: ") + e.what());
        return false;
    }
}

// 视图元数据操作
bool SystemMetadataManager::CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                                             const std::string& view_definition, const std::string& owner) {
    try {
        // 检查视图是否已存在
        if (ViewExists(schema_name, view_name)) {
            SetError("View already exists: " + schema_name + "." + view_name);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成视图ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_VIEWS << " (view_id, db_id, schema_name, view_name, view_definition, owner, created_at) VALUES ("
            << timestamp << ", " << db_id << ", '" << schema_name << "', '" << view_name << "', '"
            << view_definition << "', '" << owner << "', '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create view record: ") + e.what());
        return false;
    }
}

bool SystemMetadataManager::DropViewRecord(const std::string& schema_name, const std::string& view_name) {
    try {
        // 检查视图是否存在
        if (!ViewExists(schema_name, view_name)) {
            SetError("View does not exist: " + schema_name + "." + view_name);
            return false;
        }
        
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_VIEWS 
            << " WHERE schema_name = '" << schema_name << "' AND view_name = '" << view_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop view record: ") + e.what());
        return false;
    }
}

SysView SystemMetadataManager::GetViewRecord(const std::string& schema_name, const std::string& view_name) {
    SysView view;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT view_id, db_id, schema_name, view_name, view_definition, owner, created_at FROM "
            << SYS_TABLE_VIEWS << " WHERE schema_name = '" << schema_name 
            << "' AND view_name = '" << view_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].size() < 7) {
            SetError("View not found: " + schema_name + "." + view_name);
            return view;
        }
        
        // 填充视图信息
        view.view_id = std::stoll(result[0][0]);
        view.db_id = std::stoll(result[0][1]);
        view.schema_name = result[0][2];
        view.view_name = result[0][3];
        view.view_definition = result[0][4];
        view.owner = result[0][5];
        view.created_at = result[0][6];
        
        return view;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get view record: ") + e.what());
        return view;
    }
}

std::vector<SysView> SystemMetadataManager::ListViews(int64_t db_id) {
    std::vector<SysView> views;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT view_id, db_id, schema_name, view_name, view_definition, owner, created_at FROM "
            << SYS_TABLE_VIEWS << " WHERE db_id = " << db_id << " ORDER BY schema_name, view_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充视图列表
        for (const auto& row : result) {
            if (row.size() >= 7) {
                SysView view;
                view.view_id = std::stoll(row[0]);
                view.db_id = std::stoll(row[1]);
                view.schema_name = row[2];
                view.view_name = row[3];
                view.view_definition = row[4];
                view.owner = row[5];
                view.created_at = row[6];
                views.push_back(view);
            }
        }
        
        return views;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to list views: ") + e.what());
        return views;
    }
}

bool SystemMetadataManager::ViewExists(const std::string& schema_name, const std::string& view_name) {
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_VIEWS 
            << " WHERE schema_name = '" << schema_name << "' AND view_name = '" << view_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check view existence: ") + e.what());
        return false;
    }
}

std::string SystemMetadataManager::GetLastError() const {
    return last_error_;
}

bool SystemMetadataManager::ExecuteSQL(const std::string& sql) {
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

std::vector<std::vector<std::string>> SystemMetadataManager::ExecuteSelectQuery(const std::string& sql) {
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

void SystemMetadataManager::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace sqlcc