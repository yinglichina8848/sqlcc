#include "sql_parser/ast_nodes.h"
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "core/execution_result.h"
#include "sql_parser/load_data_ast.h"
#include "storage_engine.h"
#include "sql_executor.h"

namespace sqlcc {

class LoadDataExecutor {
public:
    LoadDataExecutor(std::shared_ptr<StorageEngine> storage_engine,
                    std::shared_ptr<SqlExecutor> sql_executor);
    ~LoadDataExecutor();

    // 执行LOAD DATA语句
    ExecutionResult execute(const sql_parser::LoadDataStatement& stmt);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<SqlExecutor> sql_executor_;

    // 文件处理
    bool openFile(const std::string& filename, bool is_local, std::ifstream& file);
    bool readLine(std::ifstream& file, std::string& line);
    void closeFile(std::ifstream& file);

    // 数据解析
    std::vector<std::string> parseFields(const std::string& line, const sql_parser::LoadDataStatement& stmt);
    std::string unescapeField(const std::string& field, char escape_char);
    std::string removeEnclosure(const std::string& field, const std::string& enclosure, bool optionally);

    // 数据转换和验证
    bool validateAndConvertRow(const std::vector<std::string>& raw_fields,
                              std::vector<std::string>& processed_row,
                              const sql_parser::LoadDataStatement& stmt,
                              std::shared_ptr<TableMetadata> table_meta);
    bool applySetExpressions(std::vector<std::string>& row,
                           const sql_parser::LoadDataStatement& stmt);
    std::string evaluateSetExpression(const std::string& expression,
                                    const std::vector<std::string>& row,
                                    const std::vector<TableColumn>& columns);
    std::string evaluateArithmeticExpression(const std::string& expr);
    bool validateConstraints(const std::vector<std::string>& row,
                           std::shared_ptr<TableMetadata> table_meta);
    bool validateDataType(const std::string& value, const std::string& type);

    // 批量插入
    bool insertRow(const std::vector<std::string>& row,
                  const sql_parser::LoadDataStatement& stmt,
                  std::shared_ptr<TableMetadata> table_meta);

    // 错误处理和统计
    struct LoadStats {
        size_t total_lines = 0;
        size_t skipped_lines = 0;
        size_t inserted_rows = 0;
        size_t failed_rows = 0;
        size_t warnings = 0;
        std::vector<std::string> errors;
    };

    LoadStats stats_;

    // 辅助方法
    std::shared_ptr<TableMetadata> getTableMetadata(const std::string& table_name);
    bool checkTableExists(const std::string& table_name);
    bool checkFilePermissions(const std::string& filename, bool is_local);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
};

} // namespace sqlcc
