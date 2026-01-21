/**
 * @file primary_key_constraint.h
 * @brief 主键约束类定义
 *
 * Why: 需要专门的类来处理主键约束的唯一性和非空性验证
 * What: PrimaryKeyConstraint类封装主键约束的列定义和验证
 * How: 提供主键约束的存储和验证功能
 */

#pragma once

#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 主键约束类
 *
 * 实现SQL PRIMARY KEY约束，确保列值的唯一性和非空性
 */
class PrimaryKeyConstraint {
public:
    /**
     * @brief 构造函数
     * @param columns 主键列名列表
     * @param name 约束名称（可选）
     */
    PrimaryKeyConstraint(const std::vector<std::string>& columns,
                        const std::string& name = "");

    /**
     * @brief 获取主键列名列表
     * @return 列名列表的常量引用
     */
    const std::vector<std::string>& getColumns() const;

    /**
     * @brief 获取约束名称
     * @return 约束名称的常量引用
     */
    const std::string& getName() const;

private:
    std::vector<std::string> columns_; ///< 主键列名列表
    std::string name_;                 ///< 约束名称
};

} // namespace sql_parser
} // namespace sqlcc
