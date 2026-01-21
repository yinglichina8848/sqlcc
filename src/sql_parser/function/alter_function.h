/**
#include "data_types.h"
 * @file alter_function.h
 * @brief 修改函数语句头文件
 */

#ifndef SQLCC_SQL_PARSER_ALTER_FUNCTION_H
#define SQLCC_SQL_PARSER_ALTER_FUNCTION_H

#include <string>
#include <vector>
#include <memory>

#include "ast_node.h"
#include "function/function_parameter.h"
#include "datatype.h"

namespace sqlcc {

namespace sql_parser {

// 修改函数语句
class AlterFunctionStatement : public DDLStatement {
public:
    // 修改操作类型
    enum class AlterType {
        RENAME,           // 重命名函数
        SET_SCHEMA,       // 更改模式
        OWNER_TO,         // 更改所有者
        SET_DEFAULT,      // 设置默认值
        DROP_DEFAULT,     // 删除默认值
        RENAME_ATTRIBUTE, // 重命名属性
        ADD_PARAMETER,    // 添加参数
        DROP_PARAMETER,   // 删除参数
        ALTER_PARAMETER   // 修改参数
    };

    AlterFunctionStatement(const std::string& function_name,
                          AlterType alter_type,
                          const std::string& new_value = "",
                          const FunctionParameter& parameter = FunctionParameter("", DataType::INTEGER));

    const std::string& get_function_name() const;
    AlterType get_alter_type() const;
    const std::string& get_new_value() const;
    const FunctionParameter& get_parameter() const;

    void accept(ASTVisitor& visitor) override;
    std::string to_string() const override;

private:
    std::string function_name_;
    AlterType alter_type_;
    std::string new_value_;
    FunctionParameter parameter_;
};

} // namespace sql_parser

} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_ALTER_FUNCTION_H
