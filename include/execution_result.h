#ifndef SQLCC_EXECUTION_RESULT_H
#define SQLCC_EXECUTION_RESULT_H

#include "wal_manager.h" // 包含Value的定义
#include <optional>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief 列元数据结构体
 * 用于存储列的元数据信息
 */
struct ColumnMeta {
  std::string name;          // 列名
  std::string data_type;     // 数据类型
  bool is_nullable;          // 是否允许为NULL
  bool is_primary_key;       // 是否为主键
  bool is_unique_key;        // 是否为唯一键
  std::string default_value; // 默认值
};

/**
 * @brief 行结构体
 * 用于存储查询结果中的一行数据
 */
struct Row {
  std::vector<Value> values; // 行中的各个列值
};

/**
 * @brief 执行结果结构体
 * 用于表示SQL查询的执行结果
 */
struct ExecutionResult {
    enum Status { SUCCESS, FAILURE };
    
    // 结果集数据
    std::vector<Row> rows;
    
    // 列元数据
    std::vector<ColumnMeta> column_metadata;
    
    // 执行状态
    bool success;        // 执行是否成功
    std::string message; // 执行消息
    
    // 构造函数
    ExecutionResult(bool success = true, const std::string &message = "")
        : success(success), message(message) {}
    
    // 方法
    void add_row(const Row &row) { rows.push_back(row); }
    
    size_t row_count() const { return rows.size(); }
    
    bool is_empty() const { return rows.empty(); }
    
    bool has_error() const { return !success; }
    
    // 兼容旧版接口
    Status getStatus() const { return success ? SUCCESS : FAILURE; }
    
    const std::string& getMessage() const { return message; }
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_RESULT_H