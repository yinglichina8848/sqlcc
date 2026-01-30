#ifndef SQLCC_SQL_PARSER_JSON_PARSER_H
#define SQLCC_SQL_PARSER_JSON_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "src/sql_parser/json.h"

namespace sqlcc {
namespace sql_parser {

/**
 * @brief JSON解析器
 *
 * 负责解析JSON格式的SQL表达式，包括：
 * - JSON路径表达式解析
 * - JSON函数参数解析
 * - JSON数据验证
 * - JSON语法错误处理
 */
class JSONParser {
public:
    /**
     * @brief 构造函数
     */
    JSONParser();

    /**
     * @brief 析构函数
     */
    ~JSONParser();

    /**
     * @brief 解析JSON路径表达式
     * @param json_path JSON路径字符串（如 "$.name", "$.items[0]"）
     * @return 解析是否成功
     */
    bool ParseJSONPath(const std::string& json_path);

    /**
     * @brief 解析JSON查询函数表达式
     * @param sql_expression SQL表达式（如 "JSON_QUERY(data, '$.name')"）
     * @return 解析是否成功
     */
    bool ParseJSONQueryExpression(const std::string& sql_expression);

    /**
     * @brief 解析JSON值提取表达式
     * @param sql_expression SQL表达式（如 "JSON_VALUE(data, '$.age')"）
     * @return 解析是否成功
     */
    bool ParseJSONValueExpression(const std::string& sql_expression);

    /**
     * @brief 解析JSON数组聚合表达式
     * @param sql_expression SQL表达式（如 "JSON_ARRAYAGG(column)"）
     * @return 解析是否成功
     */
    bool ParseJSONArrayAggExpression(const std::string& sql_expression);

    /**
     * @brief 解析JSON对象聚合表达式
     * @param sql_expression SQL表达式（如 "JSON_OBJECTAGG(key, value)"）
     * @return 解析是否成功
     */
    bool ParseJSONObjectAggExpression(const std::string& sql_expression);

    /**
     * @brief 验证JSON数据格式
     * @param json_data JSON字符串数据
     * @return 是否为有效JSON
     */
    bool ValidateJSONData(const std::string& json_data);

    /**
     * @brief 获取解析后的路径段
     * @return 路径段列表
     */
    std::vector<std::string> GetPathSegments() const;

    /**
     * @brief 获取解析后的函数参数
     * @return 参数列表
     */
    std::vector<std::string> GetFunctionArguments() const;

    /**
     * @brief 获取解析错误信息
     * @return 错误信息
     */
    std::string GetErrorMessage() const;

    /**
     * @brief 检查是否有解析错误
     * @return 是否有错误
     */
    bool HasError() const;

    /**
     * @brief 重置解析器状态
     */
    void Reset();

    /**
     * @brief 解析JSON路径中的数组索引
     * @param path_segment 路径段（如 "items[0]"）
     * @return 索引列表
     */
    std::vector<size_t> ParseArrayIndices(const std::string& path_segment);

    /**
     * @brief 验证JSON路径语法
     * @param json_path JSON路径字符串
     * @return 是否语法正确
     */
    bool ValidateJSONPathSyntax(const std::string& json_path);

    /**
     * @brief 提取JSON路径中的属性名
     * @param json_path JSON路径字符串
     * @return 属性名列表
     */
    std::vector<std::string> ExtractPropertyNames(const std::string& json_path);

private:
    std::vector<std::string> path_segments_;
    std::vector<std::string> function_args_;
    std::string error_message_;
    bool has_error_;

    // 私有辅助方法
    bool IsValidPathCharacter(char c);
    bool IsValidPropertyName(const std::string& name);
    std::string TrimQuotes(const std::string& str);
    std::vector<std::string> SplitPath(const std::string& path);
    bool ParseFunctionCall(const std::string& expression, const std::string& function_name);
    void SetError(const std::string& message);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_JSON_PARSER_H