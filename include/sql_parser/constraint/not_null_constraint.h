/**
 * @file not_null_constraint.h
 * @brief 非空约束类定义
 *
 * Why: 需要专门的类来处理非空约束的空值验证
 * What: NotNullConstraint类封装非空约束的列定义和验证
 * How: 提供非空约束的存储和空值检查功能
 */

#pragma once

#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 非空约束类
 *
 * 实现SQL NOT NULL约束，确保列值不为空
 */
class NotNullConstraint {
public:
    /**
     * @brief 构造函数
     * @param column 非空约束列名
     * @param name 约束名称（可选）
     */
    NotNullConstraint(const std::string& column, const std::string& name = "");

    /**
     * @brief 获取非空约束列名
     * @return 列名的常量引用
     */
    const std::string& getColumn() const;

    /**
     * @brief 获取约束名称
     * @return 约束名称的常量引用
     */
    const std::string& getName() const;

private:
    std::string column_; ///< 非空约束列名
    std::string name_;   ///< 约束名称
};

} // namespace sql_parser
} // namespace sqlcc
