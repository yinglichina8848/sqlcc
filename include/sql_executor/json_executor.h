#ifndef SQLCC_SQL_EXECUTOR_JSON_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_JSON_EXECUTOR_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace sqlcc {
namespace sql_executor {

/**
 * @brief JSON执行器
 *
 * 负责执行JSON相关的SQL操作，包括：
 * - JSON查询和提取
 * - JSON路径操作
 * - JSON聚合函数
 * - JSON验证和转换
 */
class JSONExecutor {
public:
    /**
     * @brief 构造函数
     */
    JSONExecutor();

    /**
     * @brief 析构函数
     */
    ~JSONExecutor();

    /**
     * @brief 执行JSON查询操作
     * @param json_data JSON数据字符串
     * @param json_path JSON路径表达式
     * @return 查询结果
     */
    std::string ExecuteJSONQuery(const std::string& json_data,
                                const std::string& json_path);

    /**
     * @brief 执行JSON值提取操作
     * @param json_data JSON数据字符串
     * @param json_path JSON路径表达式
     * @return 提取的值
     */
    std::string ExecuteJSONValue(const std::string& json_data,
                                const std::string& json_path);

    /**
     * @brief 执行JSON数组聚合操作
     * @param json_values JSON值数组
     * @return 聚合后的JSON数组
     */
    std::string ExecuteJSONArrayAgg(const std::vector<std::string>& json_values);

    /**
     * @brief 执行JSON对象聚合操作
     * @param json_objects JSON对象数组
     * @return 聚合后的JSON对象
     */
    std::string ExecuteJSONObjectAgg(const std::vector<std::pair<std::string, std::string>>& json_objects);

    /**
     * @brief 执行JSON修改操作
     * @param json_data 原始JSON数据
     * @param json_path 修改路径
     * @param new_value 新值
     * @return 修改后的JSON数据
     */
    std::string ExecuteJSONModify(const std::string& json_data,
                                 const std::string& json_path,
                                 const std::string& new_value);

    /**
     * @brief 验证JSON数据格式
     * @param json_data JSON数据字符串
     * @return 是否为有效JSON
     */
    bool ValidateJSON(const std::string& json_data);

    /**
     * @brief 格式化JSON数据（美化输出）
     * @param json_data JSON数据字符串
     * @return 格式化后的JSON字符串
     */
    std::string FormatJSON(const std::string& json_data);

    /**
     * @brief 压缩JSON数据（去除空白）
     * @param json_data JSON数据字符串
     * @return 压缩后的JSON字符串
     */
    std::string MinifyJSON(const std::string& json_data);

    /**
     * @brief 计算JSON数据长度
     * @param json_data JSON数据字符串
     * @return JSON长度
     */
    size_t GetJSONLength(const std::string& json_data);

    /**
     * @brief 检查JSON类型
     * @param json_data JSON数据字符串
     * @return JSON类型字符串
     */
    std::string GetJSONType(const std::string& json_data);

private:
    // 私有辅助方法
    bool IsValidJSONPath(const std::string& json_path);
    std::string EscapeJSON(const std::string& input);
    std::string UnescapeJSON(const std::string& input);
};

} // namespace sql_executor
} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_JSON_EXECUTOR_H