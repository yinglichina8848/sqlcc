#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core_backup_20260121_001034/core_database_manager.h"

namespace sqlcc {

// 前向声明系统数据结构
struct SysDatabase;
struct SysTable;
struct SysColumn;
struct SysIndex;
struct SysConstraint;
struct SysView;

/**
 * @brief 系统元数据管理器
 * 
 * 负责管理数据库对象的元数据，包括：
 * - 数据库元数据（创建、删除、查询）
 * - 表元数据（创建、删除、查询）
 * - 列元数据（创建、删除、更新、查询）
 * - 索引元数据（创建、删除、更新、查询）
 * - 约束元数据（创建、删除、更新、查询）
 * - 视图元数据（创建、删除、查询）
 */
class SystemMetadataManager {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器指针
     */
    explicit SystemMetadataManager(std::shared_ptr<DatabaseManager> db_manager);
    
    /**
     * @brief 析构函数
     */
    ~SystemMetadataManager();
    
    // 数据库元数据操作
    /**
     * @brief 创建数据库记录
     * @param db_name 数据库名
     * @param owner 所有者
     * @param description 描述
     * @return 是否创建成功
     */
    bool CreateDatabaseRecord(const std::string& db_name, const std::string& owner, 
                             const std::string& description = "");
    
    /**
     * @brief 删除数据库记录
     * @param db_name 数据库名
     * @return 是否删除成功
     */
    bool DropDatabaseRecord(const std::string& db_name);
    
    /**
     * @brief 获取数据库记录
     * @param db_name 数据库名
     * @return 数据库信息
     */
    SysDatabase GetDatabaseRecord(const std::string& db_name);
    
    /**
     * @brief 列出所有数据库
     * @return 数据库列表
     */
    std::vector<SysDatabase> ListDatabases();
    
    /**
     * @brief 检查数据库是否存在
     * @param db_name 数据库名
     * @return 数据库是否存在
     */
    bool DatabaseExists(const std::string& db_name);
    
    // 表元数据操作
    /**
     * @brief 创建表记录
     * @param db_id 数据库ID
     * @param schema_name 模式名
     * @param table_name 表名
     * @param owner 所有者
     * @param table_type 表类型
     * @return 是否创建成功
     */
    bool CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                         const std::string& owner, const std::string& table_type = "BASE TABLE");
    
    /**
     * @brief 删除表记录
     * @param schema_name 模式名
     * @param table_name 表名
     * @return 是否删除成功
     */
    bool DropTableRecord(const std::string& schema_name, const std::string& table_name);
    
    /**
     * @brief 获取表记录
     * @param schema_name 模式名
     * @param table_name 表名
     * @return 表信息
     */
    SysTable GetTableRecord(const std::string& schema_name, const std::string& table_name);
    
    /**
     * @brief 列出数据库中的所有表
     * @param db_id 数据库ID
     * @return 表列表
     */
    std::vector<SysTable> ListTables(int64_t db_id);
    
    /**
     * @brief 检查表是否存在
     * @param schema_name 模式名
     * @param table_name 表名
     * @return 表是否存在
     */
    bool TableExists(const std::string& schema_name, const std::string& table_name);
    
    /**
     * @brief 重命名表记录
     * @param schema_name 模式名
     * @param old_table_name 旧表名
     * @param new_table_name 新表名
     * @return 是否重命名成功
     */
    bool RenameTableRecord(const std::string& schema_name, const std::string& old_table_name, 
                         const std::string& new_table_name);
    
    // 列元数据操作
    /**
     * @brief 创建列记录
     * @param table_id 表ID
     * @param column_name 列名
     * @param data_type 数据类型
     * @param is_nullable 是否可为空
     * @param default_value 默认值
     * @param ordinal_position 位置
     * @return 是否创建成功
     */
    bool CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                           bool is_nullable, const std::string& default_value, int ordinal_position);
    
    /**
     * @brief 删除列记录
     * @param table_id 表ID
     * @param column_name 列名
     * @return 是否删除成功
     */
    bool DropColumnRecord(int64_t table_id, const std::string& column_name);
    
    /**
     * @brief 获取表的列信息
     * @param table_id 表ID
     * @return 列列表
     */
    std::vector<SysColumn> GetTableColumns(int64_t table_id);
    
    /**
     * @brief 更新列记录
     * @param table_id 表ID
     * @param column_name 列名
     * @param data_type 数据类型
     * @param is_nullable 是否可为空
     * @param default_value 默认值
     * @return 是否更新成功
     */
    bool UpdateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                          bool is_nullable, const std::string& default_value);
    
    /**
     * @brief 重命名列记录
     * @param table_id 表ID
     * @param old_column_name 旧列名
     * @param new_column_name 新列名
     * @return 是否重命名成功
     */
    bool RenameColumnRecord(int64_t table_id, const std::string& old_column_name, 
                           const std::string& new_column_name);
    
    // 索引元数据操作
    /**
     * @brief 创建索引记录
     * @param table_id 表ID
     * @param index_name 索引名
     * @param column_name 列名
     * @param is_unique 是否唯一
     * @param index_type 索引类型
     * @return 是否创建成功
     */
    bool CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                          bool is_unique, const std::string& index_type);
    
    /**
     * @brief 删除索引记录
     * @param table_id 表ID
     * @param index_name 索引名
     * @return 是否删除成功
     */
    bool DropIndexRecord(int64_t table_id, const std::string& index_name);
    
    /**
     * @brief 获取表的索引信息
     * @param table_id 表ID
     * @return 索引列表
     */
    std::vector<SysIndex> GetTableIndexes(int64_t table_id);
    
    /**
     * @brief 重命名索引记录
     * @param table_id 表ID
     * @param old_index_name 旧索引名
     * @param new_index_name 新索引名
     * @return 是否重命名成功
     */
    bool RenameIndexRecord(int64_t table_id, const std::string& old_index_name, 
                          const std::string& new_index_name);
    
    // 约束元数据操作
    /**
     * @brief 创建约束记录
     * @param table_id 表ID
     * @param constraint_name 约束名
     * @param constraint_type 约束类型
     * @param column_name 列名
     * @param reference_table 引用表
     * @param reference_column 引用列
     * @return 是否创建成功
     */
    bool CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, 
                               const std::string& constraint_type, const std::string& column_name,
                               const std::string& reference_table = "", const std::string& reference_column = "");
    
    /**
     * @brief 删除约束记录
     * @param table_id 表ID
     * @param constraint_name 约束名
     * @return 是否删除成功
     */
    bool DropConstraintRecord(int64_t table_id, const std::string& constraint_name);
    
    /**
     * @brief 获取表的约束信息
     * @param table_id 表ID
     * @return 约束列表
     */
    std::vector<SysConstraint> GetTableConstraints(int64_t table_id);
    
    /**
     * @brief 重命名约束记录
     * @param table_id 表ID
     * @param old_constraint_name 旧约束名
     * @param new_constraint_name 新约束名
     * @return 是否重命名成功
     */
    bool RenameConstraintRecord(int64_t table_id, const std::string& old_constraint_name, 
                               const std::string& new_constraint_name);
    
    // 视图元数据操作
    /**
     * @brief 创建视图记录
     * @param db_id 数据库ID
     * @param schema_name 模式名
     * @param view_name 视图名
     * @param view_definition 视图定义
     * @param owner 所有者
     * @return 是否创建成功
     */
    bool CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                         const std::string& view_definition, const std::string& owner);
    
    /**
     * @brief 删除视图记录
     * @param schema_name 模式名
     * @param view_name 视图名
     * @return 是否删除成功
     */
    bool DropViewRecord(const std::string& schema_name, const std::string& view_name);
    
    /**
     * @brief 获取视图记录
     * @param schema_name 模式名
     * @param view_name 视图名
     * @return 视图信息
     */
    SysView GetViewRecord(const std::string& schema_name, const std::string& view_name);
    
    /**
     * @brief 列出数据库中的所有视图
     * @param db_id 数据库ID
     * @return 视图列表
     */
    std::vector<SysView> ListViews(int64_t db_id);
    
    /**
     * @brief 检查视图是否存在
     * @param schema_name 模式名
     * @param view_name 视图名
     * @return 视图是否存在
     */
    bool ViewExists(const std::string& schema_name, const std::string& view_name);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

private:
    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @return 是否执行成功
     */
    bool ExecuteSQL(const std::string& sql);
    
    /**
     * @brief 执行查询并返回结果
     * @param sql SQL查询语句
     * @return 查询结果
     */
    std::vector<std::vector<std::string>> ExecuteSelectQuery(const std::string& sql);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);
    
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    std::string last_error_;                       // 最后一次错误信息
};

} // namespace sqlcc