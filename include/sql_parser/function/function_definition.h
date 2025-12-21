/**
 * @file function_definition.h
 * @brief 函数定义类定义
 *
 * Why: 需要专门的类来管理用户定义函数的完整定义
 * What: FunctionDefinition类封装函数的名称、参数、返回值和特征
 * How: 提供函数定义的存储和管理功能
 */

#pragma once

#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 函数特征枚举
 */
enum class FunctionCharacteristic {
    DETERMINISTIC,        ///< 确定性函数
    NOT_DETERMINISTIC,    ///< 非确定性函数
    CONTAINS_SQL,         ///< 包含SQL
    READS_SQL_DATA,       ///< 读取SQL数据
    MODIFIES_SQL_DATA     ///< 修改SQL数据
};

/**
 * @brief 函数参数结构
 */
struct FunctionParameter {
    std::string name;     ///< 参数名称
    std::string type;     ///< 参数类型
    bool is_in = true;    ///< 是否为输入参数
    bool is_out = false;  ///< 是否为输出参数
    bool is_inout = false; ///< 是否为输入输出参数
};

/**
 * @brief 函数定义类
 *
 * 存储用户定义函数的完整信息，包括名称、参数、返回值、特征等
 */
class FunctionDefinition {
public:
    /**
     * @brief 构造函数
     * @param name 函数名称
     * @param return_type 返回值类型
     */
    FunctionDefinition(const std::string& name, const std::string& return_type);

    /**
     * @brief 析构函数
     */
    ~FunctionDefinition();

    /**
     * @brief 添加参数
     * @param param 函数参数
     */
    void addParameter(const FunctionParameter& param);

    /**
     * @brief 添加函数特征
     * @param characteristic 函数特征字符串
     */
    void addCharacteristic(const std::string& characteristic);

    /**
     * @brief 设置函数体
     * @param body 函数体代码
     */
    void setBody(const std::string& body);

    /**
     * @brief 设置函数语言
     * @param language 函数语言
     */
    void setLanguage(const std::string& language);

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getReturnType() const { return return_type_; }
    const std::vector<FunctionParameter>& getParameters() const { return parameters_; }
    const std::vector<std::string>& getCharacteristics() const { return characteristics_; }
    const std::string& getBody() const { return body_; }
    const std::string& getLanguage() const { return language_; }

    // 特征检查
    bool isDeterministic() const;
    bool containsSql() const;
    bool readsSqlData() const;
    bool modifiesSqlData() const;

    // 工具函数
    static std::string characteristicToString(FunctionCharacteristic characteristic);
    static FunctionCharacteristic stringToCharacteristic(const std::string& str);

private:
    std::string name_;                           ///< 函数名称
    std::string return_type_;                    ///< 返回值类型
    std::vector<FunctionParameter> parameters_;  ///< 参数列表
    std::vector<std::string> characteristics_;   ///< 函数特征列表
    std::string body_;                           ///< 函数体
    std::string language_;                       ///< 函数语言
};

} // namespace sql_parser
} // namespace sqlcc
