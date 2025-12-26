/**
#include "sql_parser/data_types.h"
 * @file function_parameter.h
 * @brief 函数参数定义头文件
 */

#ifndef SQLCC_SQL_PARSER_FUNCTION_PARAMETER_H
#define SQLCC_SQL_PARSER_FUNCTION_PARAMETER_H

#include <string>
#include <memory>

#include "sql_parser/datatype.h"

namespace sqlcc {

namespace sql_parser {

// 函数参数方向
enum class ParameterMode {
    IN,      // 输入参数
    OUT,     // 输出参数
    INOUT    // 输入输出参数
};

// 函数参数定义
class FunctionParameter {
public:
    FunctionParameter(const std::string& name, DataType data_type,
                     ParameterMode mode = ParameterMode::IN,
                     const std::string& default_value = "");

    const std::string& get_name() const;
    DataType get_data_type() const;
    ParameterMode get_mode() const;
    const std::string& get_default_value() const;
    bool has_default_value() const;

    void set_default_value(const std::string& value);

    std::string to_string() const;

private:
    std::string name_;
    DataType data_type_;
    ParameterMode mode_;
    std::string default_value_;
};

} // namespace sql_parser

} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_FUNCTION_PARAMETER_H
