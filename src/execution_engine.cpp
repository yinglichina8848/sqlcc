#include "../include/execution_engine.h"
#include "../include/database_manager.h"
#include "../include/user_manager.h"
#include "../include/table_storage.h"
#include "../include/storage_engine.h"
#include "../include/b_plus_tree.h"
#include <iostream>
#include <sstream>

namespace sqlcc {

// ==================== ExecutionResult ====================

ExecutionResult::ExecutionResult(bool success, const std::string& message)
    : success(success), message(message) {
}

// ==================== ExecutionEngine ====================

ExecutionEngine::ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

// ==================== DDLExecutor ====================

DDLExecutor::DDLExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager), system_db_(nullptr), user_manager_(nullptr) {
}

DDLExecutor::DDLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<SystemDatabase> system_db,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager), system_db_(system_db), user_manager_(user_manager) {
}

bool DDLExecutor::checkDDLPermission(const std::string& operation, const std::string& resource) {
    // 如果没有UserManager，默认允许（向后兼容）
    if (!user_manager_) {
        return true;
    }
    
    // TODO: 集成UserManager权限检查
    // 当前默认允许，待实现权限系统集成
    return true;
}

ExecutionResult DDLExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    switch (stmt->getType()) {
    case sql_parser::Statement::CREATE:
        return executeCreate(static_cast<sql_parser::CreateStatement*>(stmt.get()));
    case sql_parser::Statement::DROP:
        return executeDrop(static_cast<sql_parser::DropStatement*>(stmt.get()));
    case sql_parser::Statement::ALTER:
        return executeAlter(static_cast<sql_parser::AlterStatement*>(stmt.get()));
    default:
        return ExecutionResult(false, "Invalid DDL statement type");
    }
}

ExecutionResult DDLExecutor::executeCreate(sql_parser::CreateStatement* stmt) {
    switch (stmt->getObjectType()) {
    case sql_parser::CreateStatement::DATABASE:
        {
            const std::string& db_name = stmt->getObjectName();
            if (db_manager_) {
                if (db_manager_->CreateDatabase(db_name)) {
                    return ExecutionResult(true, "Database '" + db_name + "' created successfully");
                } else {
                    return ExecutionResult(false, "Failed to create database '" + db_name + "'");
                }
            }
            return ExecutionResult(false, "Database manager not available");
        }
    case sql_parser::CreateStatement::TABLE:
        {
            const std::string& table_name = stmt->getObjectName();
            if (db_manager_) {
                // 检查是否在有效数据库上下文中
                std::string current_db = db_manager_->GetCurrentDatabase();
                if (current_db.empty()) {
                    return ExecutionResult(false, "No database selected");
                }
                
                // 检查权限
                if (!checkDDLPermission("CREATE", "TABLE")) {
                    return ExecutionResult(false, "Permission denied: CREATE TABLE");
                }
                
                // 获取列定义并转换为正确的格式
                const auto& columns = stmt->getColumns();
                std::vector<std::pair<std::string, std::string>> table_columns;
                for (const auto& col : columns) {
                    table_columns.emplace_back(col.getName(), col.getType());
                }
                
                // 创建表
                if (db_manager_->CreateTable(table_name, table_columns)) {
                    // 同步元数据到SystemDatabase
                    if (system_db_) {
                        // 获取当前数据库的ID和schema名
                        // TODO: 获取实际的db_id和owner信息
                        int64_t db_id = 1;  // 临时使用默认值
                        std::string schema = "public";  // 临时使用默认schema
                        std::string owner = "root";  // 临时使用默认owner
                        
                        system_db_->CreateTableRecord(db_id, schema, table_name, owner);
                        // 记录列信息
                        // TODO: 获取创建的table_id用于列记录
                        int64_t table_id = 1;  // 临时使用，实际应该从CreateTableRecord返回
                        int ordinal = 1;
                        for (const auto& col : columns) {
                            system_db_->CreateColumnRecord(table_id, col.getName(), col.getType(), 
                                                          true, "", ordinal++);
                        }
                    }
                    return ExecutionResult(true, "Table '" + table_name + "' created successfully in database '" + current_db + "'");
                } else {
                    return ExecutionResult(false, "Failed to create table '" + table_name + "'");
                }
            }
            return ExecutionResult(false, "Database manager not available");
        }
    default:
        return ExecutionResult(false, "Unsupported CREATE object type");
    }
}

ExecutionResult DDLExecutor::executeDrop(sql_parser::DropStatement* stmt) {
    switch (stmt->getObjectType()) {
    case sql_parser::DropStatement::DATABASE:
        {
            const std::string& db_name = stmt->getObjectName();
            if (db_manager_) {
                if (db_manager_->DropDatabase(db_name)) {
                    return ExecutionResult(true, "Database '" + db_name + "' dropped successfully");
                } else {
                    return ExecutionResult(false, "Failed to drop database '" + db_name + "'");
                }
            }
            return ExecutionResult(false, "Database manager not available");
        }
    case sql_parser::DropStatement::TABLE:
        {
            const std::string& table_name = stmt->getObjectName();
            if (db_manager_) {
                // 检查是否在有效数据库上下文中
                std::string current_db = db_manager_->GetCurrentDatabase();
                if (current_db.empty()) {
                    return ExecutionResult(false, "No database selected");
                }
                
                // 检查权限
                if (!checkDDLPermission("DROP", "TABLE")) {
                    return ExecutionResult(false, "Permission denied: DROP TABLE");
                }
                
                // 删除表
                if (db_manager_->DropTable(table_name)) {
                    // 同步元数据（从系统数据库删除）
                    if (system_db_) {
                        // TODO: 获取实际的schema信息
                        std::string schema = "public";  // 临时使用默认schema
                        system_db_->DropTableRecord(schema, table_name);
                    }
                    return ExecutionResult(true, "Table '" + table_name + "' dropped successfully from database '" + current_db + "'");
                } else {
                    return ExecutionResult(false, "Failed to drop table '" + table_name + "'");
                }
            }
            return ExecutionResult(false, "Database manager not available");
        }
    default:
        return ExecutionResult(false, "Unsupported DROP object type");
    }
}

ExecutionResult DDLExecutor::executeAlter(sql_parser::AlterStatement* stmt) {
    if (stmt->getObjectType() == sql_parser::AlterStatement::DATABASE) {
        const std::string& db_name = stmt->getObjectName();
        if (db_manager_) {
            // 修改数据库的逻辑应该在这里实现
            return ExecutionResult(true, "Database '" + db_name + "' altered successfully");
        }
        return ExecutionResult(false, "Database manager not available");
    }
    return ExecutionResult(false, "Unsupported ALTER object type");
}

// ==================== DMLExecutor ====================

DMLExecutor::DMLExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager), user_manager_(nullptr) {
}

DMLExecutor::DMLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager), user_manager_(user_manager) {
}

bool DMLExecutor::checkDMLPermission(const std::string& operation, const std::string& table_name) {
    // 如果没有UserManager，默认允许（向后兼容）
    if (!user_manager_) {
        return true;
    }
    
    // TODO: 集成UserManager权限检查
    // 当前默认允许，待实现权限系统集成
    return true;
}

ExecutionResult DMLExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    switch (stmt->getType()) {
    case sql_parser::Statement::INSERT:
        return executeInsert(static_cast<sql_parser::InsertStatement*>(stmt.get()));
    case sql_parser::Statement::UPDATE:
        return executeUpdate(static_cast<sql_parser::UpdateStatement*>(stmt.get()));
    case sql_parser::Statement::DELETE:
        return executeDelete(static_cast<sql_parser::DeleteStatement*>(stmt.get()));
    default:
        return ExecutionResult(false, "Invalid DML statement type");
    }
}

ExecutionResult DMLExecutor::executeInsert(sql_parser::InsertStatement* stmt) {
    if (!db_manager_) {
        return ExecutionResult(false, "Database manager not available");
    }
    
    // 检查是否选择了数据库
    std::string current_db = db_manager_->GetCurrentDatabase();
    if (current_db.empty()) {
        return ExecutionResult(false, "No database selected");
    }
    
    // 获取表名
    const std::string& table_name = stmt->getTableName();
    
    // 检查INSERT权限
    if (!checkDMLPermission("INSERT", table_name)) {
        return ExecutionResult(false, "Permission denied: INSERT on table '" + table_name + "'");
    }
    
    // 检查表是否存在
    if (!db_manager_->TableExists(table_name)) {
        return ExecutionResult(false, "Table '" + table_name + "' does not exist");
    }
    
    // 获取存储引擎
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return ExecutionResult(false, "Storage engine not available");
    }
    
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);
    
    // 获取要插入的值
    const auto& value_rows = stmt->getValues();
    if (value_rows.empty()) {
        return ExecutionResult(false, "No values to insert");
    }
    
    int rows_inserted = 0;
    
    // 获取表元数据
    auto metadata = table_storage.GetTableMetadata(table_name);
    if (!metadata) {
        return ExecutionResult(false, "Failed to get table metadata");
    }
    
    // 处理每一行数据
    for (const auto& values : value_rows) {
        std::vector<std::string> record(values.begin(), values.end());
        
        // 验证约束
        if (!validateColumnConstraints(record, metadata, table_name)) {
            return ExecutionResult(false, "Constraint validation failed for row");
        }
        
        // 验证主键和唯一约束
        if (!checkPrimaryKeyConstraints(record, metadata, table_name)) {
            return ExecutionResult(false, "Primary key constraint violation");
        }
        if (!checkUniqueKeyConstraints(record, metadata, table_name)) {
            return ExecutionResult(false, "Unique constraint violation");
        }
        
        // 插入记录
        int32_t page_id;
        size_t offset;
        if (!table_storage.InsertRecord(table_name, values, page_id, offset)) {
            return ExecutionResult(false, "Failed to insert record into table '" + table_name + "'");
        }
        
        // 批维护索引
        maintainIndexesOnInsert(record, table_name, page_id, offset);
        
        rows_inserted++;
    }
    
    std::string message = "INSERT INTO " + table_name + " executed successfully, " + 
                         std::to_string(rows_inserted) + " row(s) inserted";
    return ExecutionResult(true, message);
}

ExecutionResult DMLExecutor::executeUpdate(sql_parser::UpdateStatement* stmt) {
    if (!db_manager_) {
        return ExecutionResult(false, "Database manager not available");
    }
    
    // 检查是否选择了数据库
    std::string current_db = db_manager_->GetCurrentDatabase();
    if (current_db.empty()) {
        return ExecutionResult(false, "No database selected");
    }
    
    // 获取表名
    const std::string& table_name = stmt->getTableName();
    
    // 检查UPDATE权限
    if (!checkDMLPermission("UPDATE", table_name)) {
        return ExecutionResult(false, "Permission denied: UPDATE on table '" + table_name + "'");
    }
    
    // 检查表是否存在
    if (!db_manager_->TableExists(table_name)) {
        return ExecutionResult(false, "Table '" + table_name + "' does not exist");
    }
    
    // 获取存储引擎
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return ExecutionResult(false, "Storage engine not available");
    }
    
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);
    
    // 扫描表找到符合WHERE条件的记录
    auto locations = table_storage.ScanTable(table_name);
    int rows_updated = 0;
    
    // 获取表元数据
    auto metadata = table_storage.GetTableMetadata(table_name);
    if (!metadata) {
        return ExecutionResult(false, "Failed to get table metadata");
    }
    
    // 获取UPDATE值
    const auto& update_values = stmt->getUpdateValues();
    
    // 处理每一条记录
    for (const auto& location : locations) {
        int32_t page_id = location.first;
        size_t offset = location.second;
        
        // 获取记录
        std::vector<std::string> record = table_storage.GetRecord(table_name, page_id, offset);
        if (record.empty()) {
            continue;
        }
        
        // 检查WHERE条件
        if (!stmt->hasWhereClause() || matchesWhereClause(record, stmt->getWhereClause(), metadata)) {
            // 混合带田的值
            std::vector<std::string> new_values = record;
            for (const auto& update_pair : update_values) {
                const std::string& column_name = update_pair.first;
                const std::string& new_value = update_pair.second;
                
                auto col_it = metadata->column_index_map.find(column_name);
                if (col_it != metadata->column_index_map.end()) {
                    int col_index = col_it->second;
                    if (col_index >= 0 && col_index < static_cast<int>(new_values.size())) {
                        new_values[col_index] = new_value;
                    }
                }
            }
            
            // 验证更新后的数据是否符合约束
            if (!validateColumnConstraints(new_values, metadata, table_name)) {
                return ExecutionResult(false, "Constraint validation failed for update");
            }
            
            // 验证主键和唯一约束
            if (!checkPrimaryKeyConstraints(new_values, metadata, table_name)) {
                return ExecutionResult(false, "Primary key constraint violation");
            }
            if (!checkUniqueKeyConstraints(new_values, metadata, table_name)) {
                return ExecutionResult(false, "Unique constraint violation");
            }
            
            // 批维护索引
            maintainIndexesOnUpdate(record, new_values, table_name, page_id, offset);
            
            // 更新记录
            if (table_storage.UpdateRecord(table_name, page_id, offset, new_values)) {
                rows_updated++;
            }
        }
    }
    
    std::string message = "UPDATE " + table_name + " executed successfully, " + 
                         std::to_string(rows_updated) + " row(s) updated";
    return ExecutionResult(true, message);
}

ExecutionResult DMLExecutor::executeDelete(sql_parser::DeleteStatement* stmt) {
    if (!db_manager_) {
        return ExecutionResult(false, "Database manager not available");
    }
    
    // 检查是否选择了数据库
    std::string current_db = db_manager_->GetCurrentDatabase();
    if (current_db.empty()) {
        return ExecutionResult(false, "No database selected");
    }
    
    // 获取表名
    const std::string& table_name = stmt->getTableName();
    
    // 检查DELETE权限
    if (!checkDMLPermission("DELETE", table_name)) {
        return ExecutionResult(false, "Permission denied: DELETE on table '" + table_name + "'");
    }
    
    // 检查表是否存在
    if (!db_manager_->TableExists(table_name)) {
        return ExecutionResult(false, "Table '" + table_name + "' does not exist");
    }
    
    // 获取存储引擎
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return ExecutionResult(false, "Storage engine not available");
    }
    
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);
    
    // 扫描表找到符合WHERE条件的记录
    auto locations = table_storage.ScanTable(table_name);
    int rows_deleted = 0;
    
    // 获取表元数据
    auto metadata = table_storage.GetTableMetadata(table_name);
    if (!metadata) {
        return ExecutionResult(false, "Failed to get table metadata");
    }
    
    // 处理每一条记录
    for (const auto& location : locations) {
        int32_t page_id = location.first;
        size_t offset = location.second;
        
        // 获取记录
        std::vector<std::string> record = table_storage.GetRecord(table_name, page_id, offset);
        if (record.empty()) {
            continue;
        }
        
        // 检查WHERE条件
        if (!stmt->hasWhereClause() || matchesWhereClause(record, stmt->getWhereClause(), metadata)) {
            // 批维护索引
            maintainIndexesOnDelete(record, table_name, page_id, offset);
            
            // 删除记录
            if (table_storage.DeleteRecord(table_name, page_id, offset)) {
                rows_deleted++;
            }
        }
    }
    
    std::string message = "DELETE FROM " + table_name + " executed successfully, " + 
                         std::to_string(rows_deleted) + " row(s) deleted";
    return ExecutionResult(true, message);
}

bool DMLExecutor::compareValues(const std::string& left, const std::string& right, const std::string& op) {
    if (op == "=") {
        return left == right;
    } else if (op == "<>") {
        return left != right;
    } else if (op == "<") {
        try {
            return std::stoi(left) < std::stoi(right);
        } catch (...) {
            return left < right;
        }
    } else if (op == ">") {
        try {
            return std::stoi(left) > std::stoi(right);
        } catch (...) {
            return left > right;
        }
    } else if (op == "<=") {
        try {
            return std::stoi(left) <= std::stoi(right);
        } catch (...) {
            return left <= right;
        }
    } else if (op == ">=") {
        try {
            return std::stoi(left) >= std::stoi(right);
        } catch (...) {
            return left >= right;
        }
    } else if (op == "LIKE") {
        // TODO: 实现模式匹配
        return left.find(right) != std::string::npos; // 简化实现：仅检查是否包含
    } else if (op == "IN") {
        // TODO: 实现IN操作符
        return false; // 待实现
    } else if (op == "BETWEEN") {
        // TODO: 实现BETWEEN操作符
        return false; // 待实现
    }
    
    return false; // 未知操作符
}

bool DMLExecutor::matchesWhereClause(const std::vector<std::string>& record,
                                     const sql_parser::WhereClause& where_clause,
                                     std::shared_ptr<TableMetadata> metadata) {
    // 如果没有WHERE子句，则匹配所有记录
    if (where_clause.getColumnName().empty()) {
        return true;
    }
    
    // 获取列的值
    std::string column_value = getColumnValue(record, where_clause.getColumnName(), metadata);
    std::string condition_value = where_clause.getValue();
    std::string op = where_clause.getOp();
    
    // 使用比较是否匹配
    return compareValues(column_value, condition_value, op);
}

std::string DMLExecutor::getColumnValue(const std::vector<std::string>& record,
                                       const std::string& column_name,
                                       std::shared_ptr<TableMetadata> metadata) {
    if (!metadata) {
        return "";
    }
    
    // 查找列的索引
    auto it = metadata->column_index_map.find(column_name);
    if (it == metadata->column_index_map.end()) {
        return "";
    }
    
    int column_index = it->second;
    if (column_index < 0 || column_index >= static_cast<int>(record.size())) {
        return "";
    }
    
    return record[column_index];
}

bool DMLExecutor::validateColumnConstraints(const std::vector<std::string>& record,
                                           std::shared_ptr<TableMetadata> metadata,
                                           const std::string& table_name) {
    if (!metadata) {
        return false;
    }
    
    // 验证每一列的约束
    for (size_t i = 0; i < metadata->columns.size(); i++) {
        const auto& col = metadata->columns[i];
        if (i >= record.size()) {
            break;
        }
        
        const std::string& value = record[i];
        
        // 1. 验证NOT NULL约束
        if (!col.nullable && value.empty()) {
            return false; // NOT NULL约束违反
        }
    }
    
    // TODO: 完整的PRIMARY KEY和UNIQUE验证需要扫描表
    // 当前简化实现，仅验证NOT NULL约束
    // 完整实现需要：
    // 1. 获取表的所有PRIMARY KEY列
    // 2. 对每个PRIMARY KEY列检查是否有重复值
    // 3. 获取表的所有UNIQUE列
    // 4. 对每个UNIQUE列检查是否有重复值
    
    return true; // 所有约束验证通过
}

bool DMLExecutor::checkUniqueConstraints(const std::vector<std::string>& record,
                                        std::shared_ptr<TableMetadata> metadata,
                                        const std::string& table_name) {
    if (!metadata || !db_manager_) {
        return true; // 不有元数据或数据库管理器时，跳过验证
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return true;
    }
    
    TableStorageManager tsm(storage_engine);
    
    // 检查主键和唯一约束的列
    for (size_t i = 0; i < metadata->columns.size() && i < record.size(); i++) {
        const auto& col = metadata->columns[i];
        const std::string& value = record[i];
        
        // 跳过空NULL值（NULL不参与唯一性检查）
        if (value.empty()) {
            continue;
        }
        
        // 检查主键和唯一约束
        if ((col.name.find("PRIMARY") != std::string::npos || col.name.find("primary") != std::string::npos) ||
            (col.name.find("UNIQUE") != std::string::npos || col.name.find("unique") != std::string::npos)) {
            
            // TODO: 扫描表中的所有记录检查重复值
            // 当前为空实现，标记TODO
            // 完整实现需要使用TableStorageManager扫描表
        }
    }
    
    return true; // 验证通过
}

bool DMLExecutor::checkPrimaryKeyConstraints(const std::vector<std::string>& record,
                                            std::shared_ptr<TableMetadata> metadata,
                                            const std::string& table_name) {
    if (!metadata || !db_manager_) {
        return true;
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return true;
    }
    
    TableStorageManager tsm(storage_engine);
    
    // 检查每一列是否是主键
    for (size_t i = 0; i < metadata->columns.size() && i < record.size(); i++) {
        const auto& col = metadata->columns[i];
        const std::string& value = record[i];
        
        // 提示：主键不能为NULL
        if (value.empty()) {
            // 主键不能为NULL，向上传递需要下NOT NULL约束
            // 这里仅处理重复性，下NOT NULL由validateColumnConstraints处理
            continue;
        }
        
        // TODO: 检查是否为主键列
        // 步骤：
        // 1. 从表的描述信息中找主键列名算法
        // 2. 仅对主键列执行重复检查
        // 3. 扫描表中的所有记录
        // 4. 比较主键值是否重复
        
        // 检查表中是否已经存在相同主键值的记录
        std::vector<std::pair<int32_t, size_t>> locations = tsm.ScanTable(table_name);
        for (const auto& loc : locations) {
            std::vector<std::string> existing_record = tsm.GetRecord(table_name, loc.first, loc.second);
            if (!existing_record.empty() && i < existing_record.size()) {
                // 比较主键值
                if (existing_record[i] == value) {
                    // 找到符合条件的记录
                    // TODO: 此処更严格的处理：需要检查是否是同一条记录
                    return false; // 主键重复
                }
            }
        }
    }
    
    return true; // 主键验证通过
}

bool DMLExecutor::checkUniqueKeyConstraints(const std::vector<std::string>& record,
                                           std::shared_ptr<TableMetadata> metadata,
                                           const std::string& table_name) {
    if (!metadata || !db_manager_) {
        return true;
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return true;
    }
    
    TableStorageManager tsm(storage_engine);
    
    // 检查每一列是否有UNIQUE约束
    for (size_t i = 0; i < metadata->columns.size() && i < record.size(); i++) {
        const auto& col = metadata->columns[i];
        const std::string& value = record[i];
        
        // UNIQUE约束允许NULL（SQL标准）
        if (value.empty()) {
            continue; // NULL不参与唯一性检查
        }
        
        // TODO: 检查是否为UNIQUE列
        // 步骤：
        // 1. 从表的描述信息中找所有UNIQUE列
        // 2. 扫描表中的所有记录
        // 3. 比较UNIQUE值是否重复
        
        // 检查表中是否已经存在相同唯一值的记录
        std::vector<std::pair<int32_t, size_t>> locations = tsm.ScanTable(table_name);
        for (const auto& loc : locations) {
            std::vector<std::string> existing_record = tsm.GetRecord(table_name, loc.first, loc.second);
            if (!existing_record.empty() && i < existing_record.size()) {
                // 比较唯一值
                if (existing_record[i] == value) {
                    // 找到符合条件的记录
                    // TODO: 此処更严格的处理：需要检查是否是同一条记录
                    return false; // 唯一约束违反
                }
            }
        }
    }
    
    return true; // UNIQUE验证通过
}

void DMLExecutor::maintainIndexesOnInsert(const std::vector<std::string>& record,
                                         const std::string& table_name,
                                         int32_t page_id, size_t offset) {
    // 实现索引维护逻辑
    // 步骤：
    // 1. 获取表的所有索引
    // 2. 为每个索引提取对应列的值
    // 3. 今IndexEntry制作并插入索引
    
    if (!db_manager_) {
        return; // 不能空
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return;
    }
    
    // TODO: Extract all indexes information for the table
    // This information can be queried from the system database's index table
    // Requirement: SystemDatabase::GetIndexesForTable() method
    
    // Current implementation is a framework
    // Complete implementation requires integration with IndexManager API
    
    // Example code (needs actual implementation):
    // auto indexes = db_manager_->GetSystemDatabase()->GetIndexesForTable(table_name);
    // for (const auto& index_info : indexes) {
    //     std::string key_value = record[index_info.column_index];
    //     IndexEntry entry(key_value, page_id, offset);
    //     index_manager->Insert(index_info.index_name, entry);
    // }
}

void DMLExecutor::maintainIndexesOnUpdate(const std::vector<std::string>& old_record,
                                          const std::vector<std::string>& new_record,
                                          const std::string& table_name,
                                          int32_t page_id, size_t offset) {
    // 实现索引更新逻辑
    // 步骤：
    // 1. 获取表的所有索引
    // 2. 为每个索引：
    //    a. 删除旧记录的索引条目
    //    b. 插入新记录的索引条目
    
    if (!db_manager_) {
        return;
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return;
    }
    
    // TODO: 提取表的所有索引信息
    // 归约需要SystemDatabase::GetIndexesForTable()方法
    
    // 当前为框架实现
    // 完整实现需要部上下IndexManager的API
    
    // 示例代码（需要实际实现）:
    // auto indexes = db_manager_->GetSystemDatabase()->GetIndexesForTable(table_name);
    // for (const auto& index_info : indexes) {
    //     // 删除旧值
    //     std::string old_key_value = old_record[index_info.column_index];
    //     IndexEntry old_entry(old_key_value, page_id, offset);
    //     index_manager->Remove(index_info.index_name, old_entry);
    //     
    //     // 插入新值
    //     std::string new_key_value = new_record[index_info.column_index];
    //     IndexEntry new_entry(new_key_value, page_id, offset);
    //     index_manager->Insert(index_info.index_name, new_entry);
    // }
}

void DMLExecutor::maintainIndexesOnDelete(const std::vector<std::string>& record,
                                          const std::string& table_name,
                                          int32_t page_id, size_t offset) {
    // 实现索引删除逻辑
    // 步骤：
    // 1. 获取表的所有索引
    // 2. 为每个索引删除对应记录的索引条目
    
    if (!db_manager_) {
        return;
    }
    
    auto storage_engine = db_manager_->GetStorageEngine();
    if (!storage_engine) {
        return;
    }
    
    // TODO: 提取表的所有索引信息
    // 归约需要SystemDatabase::GetIndexesForTable()方法
    
    // 当前为框架实现
    // 完整实现需要部上下IndexManager的API
    
    // 示例代码（需要实际实现）:
    // auto indexes = db_manager_->GetSystemDatabase()->GetIndexesForTable(table_name);
    // for (const auto& index_info : indexes) {
    //     std::string key_value = record[index_info.column_index];
    //     IndexEntry entry(key_value, page_id, offset);
    //     index_manager->Remove(index_info.index_name, entry);
    // }
}

// ==================== DCLExecutor ====================

DCLExecutor::DCLExecutor(std::shared_ptr<DatabaseManager> db_manager, std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager), user_manager_(user_manager) {
}

ExecutionResult DCLExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    switch (stmt->getType()) {
    case sql_parser::Statement::CREATE_USER:
        return executeCreateUser(static_cast<sql_parser::CreateUserStatement*>(stmt.get()));
    case sql_parser::Statement::DROP_USER:
        return executeDropUser(static_cast<sql_parser::DropUserStatement*>(stmt.get()));
    case sql_parser::Statement::GRANT:
        return executeGrant(static_cast<sql_parser::GrantStatement*>(stmt.get()));
    case sql_parser::Statement::REVOKE:
        return executeRevoke(static_cast<sql_parser::RevokeStatement*>(stmt.get()));
    default:
        return ExecutionResult(false, "Invalid DCL statement type");
    }
}

ExecutionResult DCLExecutor::executeCreateUser(sql_parser::CreateUserStatement* stmt) {
    // 创建用户的逻辑应该在这里实现
    if (user_manager_) {
        // 这里应该调用user_manager_的方法来创建用户
        return ExecutionResult(true, "User '" + stmt->getUsername() + "' created successfully");
    }
    return ExecutionResult(false, "User manager not available");
}

ExecutionResult DCLExecutor::executeDropUser(sql_parser::DropUserStatement* stmt) {
    // 删除用户的逻辑应该在这里实现
    if (user_manager_) {
        // 这里应该调用user_manager_的方法来删除用户
        return ExecutionResult(true, "User '" + stmt->getUsername() + "' dropped successfully");
    }
    return ExecutionResult(false, "User manager not available");
}

ExecutionResult DCLExecutor::executeGrant(sql_parser::GrantStatement* stmt) {
    // 授权的逻辑应该在这里实现
    if (user_manager_) {
        const std::string& grantee = stmt->getGrantee();
        // 这里应该调用user_manager_的方法来授予权限
        return ExecutionResult(true, "Privileges granted to user '" + grantee + "' successfully");
    }
    return ExecutionResult(false, "User manager not available");
}

ExecutionResult DCLExecutor::executeRevoke(sql_parser::RevokeStatement* stmt) {
    // 撤销权限的逻辑应该在这里实现
    if (user_manager_) {
        const std::string& grantee = stmt->getGrantee();
        // 这里应该调用user_manager_的方法来撤销权限
        return ExecutionResult(true, "Privileges revoked from user '" + grantee + "' successfully");
    }
    return ExecutionResult(false, "User manager not available");
}

// ==================== UtilityExecutor ====================

UtilityExecutor::UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager), system_db_(nullptr) {
}

UtilityExecutor::UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager, std::shared_ptr<SystemDatabase> system_db)
    : ExecutionEngine(db_manager), system_db_(system_db) {
}

ExecutionResult UtilityExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    if (stmt->getType() == sql_parser::Statement::USE) {
        sql_parser::UseStatement* use_stmt = static_cast<sql_parser::UseStatement*>(stmt.get());
        if (db_manager_) {
            if (db_manager_->UseDatabase(use_stmt->getDatabaseName())) {
                return ExecutionResult(true, "Database changed to '" + use_stmt->getDatabaseName() + "'");
            } else {
                return ExecutionResult(false, "Failed to change database to '" + use_stmt->getDatabaseName() + "'");
            }
        }
        return ExecutionResult(false, "Database manager not available");
    }
    else if (stmt->getType() == sql_parser::Statement::SHOW) {
        sql_parser::ShowStatement* show_stmt = static_cast<sql_parser::ShowStatement*>(stmt.get());
        return executeShow(show_stmt);
    }
    return ExecutionResult(false, "Invalid utility statement type");
}

ExecutionResult UtilityExecutor::executeShow(sql_parser::ShowStatement* stmt) {
    if (!db_manager_) {
        return ExecutionResult(false, "Database manager not available");
    }
    
    switch (stmt->getShowType()) {
    case sql_parser::ShowStatement::DATABASES: {
        auto databases = db_manager_->ListDatabases();
        return ExecutionResult(true, formatDatabases(databases));
    }
    
    case sql_parser::ShowStatement::TABLES: {
        // 如果有FROM子句，先切换数据库
        std::string current_db = db_manager_->GetCurrentDatabase();
        if (stmt->hasFromDatabase()) {
            if (!db_manager_->UseDatabase(stmt->getFromDatabase())) {
                return ExecutionResult(false, "Database '" + stmt->getFromDatabase() + "' does not exist");
            }
        }
        
        if (db_manager_->GetCurrentDatabase().empty()) {
            return ExecutionResult(false, "No database selected");
        }
        
        auto tables = db_manager_->ListTables();
        std::string result = formatTables(tables);
        
        // 如果之前有数据库，切换回去
        if (stmt->hasFromDatabase() && !current_db.empty()) {
            db_manager_->UseDatabase(current_db);
        }
        
        return ExecutionResult(true, result);
    }
    
    case sql_parser::ShowStatement::CREATE_TABLE: {
        std::string current_db = db_manager_->GetCurrentDatabase();
        if (current_db.empty()) {
            return ExecutionResult(false, "No database selected");
        }
        
        std::string table_name = stmt->getTargetObject();
        if (!db_manager_->TableExists(table_name)) {
            return ExecutionResult(false, "Table '" + table_name + "' does not exist");
        }
        
        // TODO: 从元数据获取表结构并生成CREATE TABLE语句
        return ExecutionResult(true, "CREATE TABLE " + table_name + " (...)\n[Table structure not yet implemented]");
    }
    
    case sql_parser::ShowStatement::COLUMNS: {
        std::string current_db = db_manager_->GetCurrentDatabase();
        if (current_db.empty()) {
            return ExecutionResult(false, "No database selected");
        }
        
        std::string table_name = stmt->getTargetObject();
        if (!db_manager_->TableExists(table_name)) {
            return ExecutionResult(false, "Table '" + table_name + "' does not exist");
        }
        
        // TODO: 从system数据库获取列信息
        return ExecutionResult(true, "Columns for table '" + table_name + "':\n[Column information not yet implemented]");
    }
    
    case sql_parser::ShowStatement::INDEXES: {
        std::string current_db = db_manager_->GetCurrentDatabase();
        if (current_db.empty()) {
            return ExecutionResult(false, "No database selected");
        }
        
        std::string table_name = stmt->getTargetObject();
        if (!db_manager_->TableExists(table_name)) {
            return ExecutionResult(false, "Table '" + table_name + "' does not exist");
        }
        
        // TODO: 从system数据库获取索引信息
        return ExecutionResult(true, "Indexes for table '" + table_name + "':\n[Index information not yet implemented]");
    }
    
    case sql_parser::ShowStatement::GRANTS: {
        std::string user_name = stmt->getTargetObject();
        
        if (!system_db_) {
            return ExecutionResult(false, "System database not available");
        }
        
        // 从SystemDatabase获取用户权限信息
        auto privileges = system_db_->GetUserPrivileges(user_name);
        
        if (privileges.empty()) {
            return ExecutionResult(true, "No grants found for user '" + user_name + "'");
        }
        
        // 格式化输出
        std::string result = "Grants for user '" + user_name + "':\n";
        result += "+--------------------+--------------------+--------------------+--------------------+\n";
        result += "| Database           | Table              | Privilege          | Grantor            |\n";
        result += "+--------------------+--------------------+--------------------+--------------------+\n";
        
        for (const auto& priv : privileges) {
            auto pad = [](const std::string& str, size_t width) -> std::string {
                if (str.length() >= width) return str.substr(0, width);
                return str + std::string(width - str.length(), ' ');
            };
            
            result += "| " + pad(priv.db_name, 18);
            result += " | " + pad(priv.table_name, 18);
            result += " | " + pad(priv.privilege, 18);
            result += " | " + pad(priv.grantor, 18);
            result += " |\n";
        }
        
        result += "+--------------------+--------------------+--------------------+--------------------+\n";
        result += std::to_string(privileges.size()) + " grant(s) found";
        
        return ExecutionResult(true, result);
    }
    
    default:
        return ExecutionResult(false, "Unsupported SHOW command");
    }
}

std::string UtilityExecutor::formatDatabases(const std::vector<std::string>& databases) {
    if (databases.empty()) {
        return "No databases found";
    }
    
    std::string result = "Databases:\n";
    result += "+--------------------------+\n";
    result += "| Database                 |\n";
    result += "+--------------------------+\n";
    
    for (const auto& db : databases) {
        result += "| " + db;
        // 填充空格到固定宽度
        size_t padding = 25 - db.length();
        result += std::string(padding, ' ');
        result += "|\n";
    }
    
    result += "+--------------------------+\n";
    result += std::to_string(databases.size()) + " database(s) found";
    
    return result;
}

std::string UtilityExecutor::formatTables(const std::vector<std::string>& tables) {
    if (tables.empty()) {
        return "No tables found";
    }
    
    std::string result = "Tables:\n";
    result += "+--------------------------+\n";
    result += "| Table                    |\n";
    result += "+--------------------------+\n";
    
    for (const auto& table : tables) {
        result += "| " + table;
        // 填充空格到固定宽度
        size_t padding = 25 - table.length();
        result += std::string(padding, ' ');
        result += "|\n";
    }
    
    result += "+--------------------------+\n";
    result += std::to_string(tables.size()) + " table(s) found";
    
    return result;
}

} // namespace sqlcc