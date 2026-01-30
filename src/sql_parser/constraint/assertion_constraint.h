/**
 * @file assertion_constraint.h
 * @brief 断言约束类定义
 *
 * Why: 需要专门的类来处理断言约束的复杂条件验证
 * What: AssertionConstraint类封装断言约束的条件表达式和验证
 * How: 提供断言约束的存储和跨表条件检查功能
 */

#pragma once

#include "../ast/ast_node.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 断言约束类
 *
 * 实现SQL ASSERTION约束，支持复杂的跨表条件验证
 */
class AssertionConstraint {
public:
    /**
     * @brief 构造函数
     * @param condition 断言条件表达式
     * @param name 约束名称（可选）
     */
    AssertionConstraint(std::unique_ptr<Expression> condition,
                       const std::string& name = "");

    /**
     * @brief 获取断言条件表达式
     * @return 条件表达式的常量指针
     */
    const Expression* getCondition() const;

    /**
     * @brief 获取约束名称
     * @return 约束名称的常量引用
     */
    const std::string& getName() const;

private:
    std::unique_ptr<Expression> condition_; ///< 断言条件表达式
    std::string name_;                      ///< 约束名称
};

} // namespace sql_parser
} // namespace sqlcc
