/**
 * @file unique_constraint.h
 * @brief 唯一约束类定义
 *
 * Why: 需要专门的类来处理唯一约束的唯一性验证
 * What: UniqueConstraint类封装唯一约束的列定义和验证
 * How: 提供唯一约束的存储和唯一性检查功能
 */

#pragma once

#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 唯一约束类
 *
 * 实现SQL UNIQUE约束，确保列值的唯一性
 */
class UniqueConstraint {
public:
    /**
     * @brief 构造函数
     * @param columns 唯一约束列名列表
     * @param name 约束名称（可选）
     */
    UniqueConstraint(const std::vector<std::string>& columns,
                    const std::string& name = "");

    /**
     * @brief 获取唯一约束列名列表
     * @return 列名列表的常量引用
     */
    const std::vector<std::string>& getColumns() const;

    /**
     * @brief 获取约束名称
     * @return 约束名称的常量引用
     */
    const std::string& getName() const;

private:
    std::vector<std::string> columns_; ///< 唯一约束列名列表
    std::string name_;                 ///< 约束名称
};

} // namespace sql_parser
} // namespace sqlcc
