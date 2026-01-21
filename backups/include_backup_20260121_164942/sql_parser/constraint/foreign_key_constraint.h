/**
 * @file foreign_key_constraint.h
 * @brief 外键约束类定义
 *
 * Why: 需要专门的类来处理外键约束的复杂逻辑
 * What: ForeignKeyConstraint类封装外键约束的完整属性和行为
 * How: 提供外键约束的存储、验证和级联操作管理
 */

#pragma once

#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 外键约束类
 *
 * 实现SQL外键约束，支持级联操作和延迟检查
 */
class ForeignKeyConstraint {
public:
    /**
     * @brief 级联操作类型
     */
    enum CascadeAction {
        RESTRICT,    ///< 限制删除/更新
        CASCADE,     ///< 级联删除/更新
        SET_NULL,    ///< 设置为空
        SET_DEFAULT, ///< 设置为默认值
        NO_ACTION    ///< 无操作
    };

    /**
     * @brief 约束检查时机
     */
    enum DeferrableMode {
        NOT_DEFERRABLE,      ///< 立即检查 (默认)
        DEFERRABLE,          ///< 可延迟检查
        INITIALLY_DEFERRED,  ///< 初始延迟检查
        INITIALLY_IMMEDIATE  ///< 初始立即检查
    };

    /**
     * @brief 构造函数
     * @param columns 本表列名列表
     * @param referenced_table 被引用表名
     * @param referenced_columns 被引用列名列表
     * @param name 约束名称（可选）
     * @param on_delete 删除时级联操作
     * @param on_update 更新时级联操作
     * @param deferrable 延迟检查模式
     */
    ForeignKeyConstraint(const std::vector<std::string>& columns,
                        const std::string& referenced_table,
                        const std::vector<std::string>& referenced_columns,
                        const std::string& name = "",
                        CascadeAction on_delete = RESTRICT,
                        CascadeAction on_update = RESTRICT,
                        DeferrableMode deferrable = NOT_DEFERRABLE);

    /**
     * @brief 获取本表列名列表
     */
    const std::vector<std::string>& getColumns() const;

    /**
     * @brief 获取被引用表名
     */
    const std::string& getReferencedTable() const;

    /**
     * @brief 获取被引用列名列表
     */
    const std::vector<std::string>& getReferencedColumns() const;

    /**
     * @brief 获取约束名称
     */
    const std::string& getName() const;

    /**
     * @brief 获取删除时级联操作
     */
    CascadeAction getOnDeleteAction() const;

    /**
     * @brief 获取更新时级联操作
     */
    CascadeAction getOnUpdateAction() const;

    /**
     * @brief 获取延迟检查模式
     */
    DeferrableMode getDeferrableMode() const;

private:
    std::vector<std::string> columns_;           ///< 本表列名列表
    std::string referenced_table_;               ///< 被引用表名
    std::vector<std::string> referenced_columns_; ///< 被引用列名列表
    std::string name_;                           ///< 约束名称
    CascadeAction on_delete_;                    ///< 删除时级联操作
    CascadeAction on_update_;                    ///< 更新时级联操作
    DeferrableMode deferrable_;                  ///< 延迟检查模式
};

} // namespace sql_parser
} // namespace sqlcc
