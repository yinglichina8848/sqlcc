/**
 * @file check_constraint.h
 * @brief 检查约束类定义
 *
 * Why: 需要专门的类来处理检查约束的条件验证逻辑
 * What: CheckConstraint类封装检查约束的条件表达式和验证
 * How: 存储和验证条件表达式，确保数据完整性
 */

#pragma once

#include "../ast/ast_node.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 检查约束类
 *
 * 实现SQL CHECK约束，支持复杂的条件表达式验证
 */
class CheckConstraint {
public:
    /**
     * @brief 构造函数
     * @param condition 检查条件表达式
     * @param name 约束名称（可选）
     */
    CheckConstraint(std::unique_ptr<Expression> condition,
                   const std::string& name = "");

    /**
     * @brief 获取检查条件表达式
     * @return 条件表达式的常量指针
     */
    const Expression* getCondition() const;

    /**
     * @brief 获取约束名称
     * @return 约束名称的常量引用
     */
    const std::string& getName() const;

private:
    std::unique_ptr<Expression> condition_; ///< 检查条件表达式
    std::string name_;                      ///< 约束名称
};

} // namespace sql_parser
} // namespace sqlcc
