/**
 * @file create_function.h
 * @brief 创建函数语句头文件
 */

#ifndef SQLCC_SQL_PARSER_CREATE_FUNCTION_H
#define SQLCC_SQL_PARSER_CREATE_FUNCTION_H

#include <string>
#include <vector>
#include <memory>

#include "sql_parser/ast_node.h"
#include "sql_parser/function/function_parameter.h"
#include "sql_parser/datatype.h"

namespace sqlcc {

namespace sql_parser {

// 创建函数语句
class CreateFunctionStatement : public DDLStatement {
public:
    CreateFunctionStatement(const std::string& function_name,
                           const std::vector<FunctionParameter>& parameters,
                           DataType return_type,
                           const std::string& function_body,
                           bool is_replace = false);

    const std::string& get_function_name() const;
    const std::vector<FunctionParameter>& get_parameters() const;
    DataType get_return_type() const;
    const std::string& get_function_body() const;
    bool is_replace() const;

    void accept(ASTVisitor& visitor) override;
    std::string to_string() const override;

private:
    std::string function_name_;
    std::vector<FunctionParameter> parameters_;
    DataType return_type_;
    std::string function_body_;
    bool is_replace_;
};

} // namespace sql_parser

} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_CREATE_FUNCTION_H
