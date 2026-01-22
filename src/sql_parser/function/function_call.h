/**
 * @file function_call.h
 * @brief 函数调用相关类定义
 *
 * Why: 需要专门的类来处理函数调用的表达式和语句形式
 * What: FunctionCallExpression和FunctionCallStatement类封装函数调用逻辑
 * How: 提供函数调用的参数传递和执行控制
 */

#pragma once

#include "../ast/ast_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 函数调用表达式
 *
 * 表示SQL中的函数调用表达式，如func(arg1, arg2)
 */
class FunctionCallExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param function_name 函数名
     */
    explicit FunctionCallExpression(const std::string& function_name);

    /**
     * @brief 析构函数
     */
    ~FunctionCallExpression() override;

    /**
     * @brief 添加函数参数
     * @param argument 参数表达式
     */
    void addArgument(std::unique_ptr<Expression> argument);

    /**
     * @brief 获取函数名
     * @return 函数名常量引用
     */
    const std::string& getFunctionName() const { return function_name_; }

    /**
     * @brief 获取参数列表
     * @return 参数表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return arguments_; }

private:
    std::string function_name_;                           ///< 函数名称
    std::vector<std::unique_ptr<Expression>> arguments_; ///< 参数列表
};

/**
 * @brief 函数调用语句
 *
 * 表示SQL中的函数调用语句，如CALL func(arg1, arg2)
 */
class FunctionCallStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param function_name 函数名
     */
    explicit FunctionCallStatement(const std::string& function_name);

    /**
     * @brief 析构函数
     */
    ~FunctionCallStatement() override;

    /**
     * @brief 添加函数参数
     * @param argument 参数表达式
     */
    void addArgument(std::unique_ptr<Expression> argument);

    /**
     * @brief 获取函数名
     * @return 函数名常量引用
     */
    const std::string& getFunctionName() const { return function_name_; }

    /**
     * @brief 获取参数列表
     * @return 参数表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return arguments_; }

private:
    std::string function_name_;                           ///< 函数名称
    std::vector<std::unique_ptr<Expression>> arguments_; ///< 参数列表
};

} // namespace sql_parser
} // namespace sqlcc
