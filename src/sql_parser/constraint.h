/**
 * WHY: 为什么需要专门的SQL约束系统？
 *
 * 数据库约束是保证数据完整性的核心机制，传统数据库系统存在诸多约束处理问题：
 * - 约束定义分散：不同组件重复实现约束逻辑
 * - 约束检查不一致：运行时和DDL时的约束验证不统一
 * - 级联操作复杂：外键约束的级联删除、更新逻辑混乱
 * - 约束命名冲突：全局约束命名空间缺乏管理
 * - 约束延迟检查：事务中约束检查时机控制不当
 *
 * SQL约束系统的核心价值：
 * 1. 完整性保证：多层次约束检查确保数据一致性
 * 2. 统一接口：标准化的约束定义、验证和错误处理
 * 3. 级联操作支持：完整的外键约束级联操作机制
 * 4. 事务控制：灵活的约束检查时机控制
 * 5. 性能优化：高效的约束验证算法和索引利用
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern)
 *
 * 约束作为策略模式的应用：
 * - 统一接口：所有约束类型实现统一的验证接口
 * - 运行时选择：根据约束类型动态选择验证策略
 * - 扩展性：新约束类型通过实现策略接口轻松添加
 * - 组合使用：多个约束可以组合应用于同一数据
 * - 测试友好：策略模式便于单元测试和模拟
 *
 * SOLID原则体现：
 * - 单一职责：每个约束类负责一种约束类型的验证
 * - 开闭原则：新约束类型通过扩展现有类实现
 * - 里氏替换：约束子类可以替换基类使用
 * - 接口隔离：约束接口精确定义所需方法
 * - 依赖倒置：高层模块依赖约束接口而非实现
 *
 * WHAT: SQL约束系统 - 完整的数据库完整性约束框架
 *
 * 核心功能：
 * - 外键约束：支持多列外键、级联操作、延迟检查
 * - 检查约束：基于表达式的复杂业务规则验证
 * - 主键约束：唯一标识记录的主键定义和验证
 * - 唯一约束：非主键列的唯一性保证
 * - 非空约束：列级非空性强制约束
 * - 断言约束：表间完整性约束的复杂规则
 *
 * 系统组件：
 * - ForeignKeyConstraint：外键约束的完整实现
 * - CheckConstraint：检查约束的表达式验证
 * - PrimaryKeyConstraint：主键约束的多列支持
 * - UniqueConstraint：唯一约束的索引优化
 * - NotNullConstraint：非空约束的简单验证
 * - AssertionConstraint：断言约束的复杂逻辑
 *
 * 级联操作支持：
 * - RESTRICT：拒绝违反约束的操作
 * - CASCADE：级联删除或更新相关记录
 * - SET_NULL：将外键列设置为NULL
 * - SET_DEFAULT：将外键列设置为默认值
 * - NO_ACTION：标准SQL的NO ACTION行为
 *
 * 约束检查时机：
 * - NOT_DEFERRABLE：立即检查约束
 * - DEFERRABLE：可延迟检查约束
 * - INITIALLY_DEFERRED：事务开始时延迟检查
 * - INITIALLY_IMMEDIATE：事务开始时立即检查
 *
 * 接口设计：
 * - 构造函数：约束对象的完整初始化
 * - getter方法：约束属性的安全访问
 * - 验证方法：约束条件的实际检查逻辑
 * - 命名支持：约束的自定义命名
 *
 * HOW: SQL约束系统的实现机制
 *
 * 约束创建流程：
 * 1. DDL解析：解析CREATE TABLE或ALTER TABLE语句中的约束定义
 * 2. 语法验证：检查约束语法和语义的正确性
 * 3. 依赖分析：分析约束间的依赖关系和执行顺序
 * 4. 对象创建：构造相应的约束对象并设置属性
 * 5. 注册存储：将约束注册到表元数据中
 *
 * 约束验证流程：
 * 1. 触发时机：根据操作类型和约束类型确定检查时机
 * 2. 条件判断：检查约束的适用条件和延迟状态
 * 3. 验证执行：调用相应的约束验证逻辑
 * 4. 错误处理：约束违反时的异常抛出和错误信息
 * 5. 级联处理：外键约束的级联操作执行
 *
 * 外键约束实现：
 * 1. 引用完整性：确保外键值在引用表中存在
 * 2. 级联操作：根据定义执行相应的级联行为
 * 3. 延迟检查：支持事务内的延迟约束检查
 * 4. 多列支持：复合外键的多列完整性保证
 * 5. 索引优化：利用索引加速外键查找操作
 *
 * 检查约束实现：
 * 1. 表达式解析：将字符串表达式解析为AST
 * 2. 类型检查：验证表达式中涉及的列和类型
 * 3. 求值执行：在具体数据上下文下计算表达式
 * 4. 结果判断：根据表达式结果确定约束是否满足
 * 5. 错误报告：提供详细的约束违反信息
 *
 * 约束管理系统：
 * 1. 约束注册：将约束关联到具体的表和列
 * 2. 约束查找：根据表名快速查找相关约束
 * 3. 约束执行：按依赖顺序执行约束验证
 * 4. 约束清理：事务提交或回滚时的约束状态清理
 * 5. 约束缓存：常用约束的验证结果缓存
 *
 * 性能优化策略：
 * - 索引利用：约束验证充分利用数据库索引
 * - 批量验证：将多个约束验证合并执行
 * - 延迟验证：非关键约束的延迟执行
 * - 缓存机制：约束元数据的缓存加速
 * - 并行验证：独立约束的并行验证执行
 *
 * 错误处理机制：
 * - 约束违反：详细的约束违反错误信息
 * - 级联失败：级联操作执行失败的处理
 * - 死锁避免：外键约束的死锁检测和避免
 * - 事务回滚：约束违反时的自动事务回滚
 * - 用户提示：清晰的约束错误信息提示
 *
 * 扩展性设计：
 * - 插件架构：支持自定义约束类型的加载
 * - 配置化：约束行为的配置化管理
 * - 多语言支持：不同SQL方言的约束语法支持
 * - 标准化：严格遵循SQL标准的约束规范
 * - 向后兼容：保持与现有约束系统的兼容性
 *
 * 调试和诊断：
 * - 约束可视化：约束关系和依赖的图形化展示
 * - 验证日志：详细的约束验证过程记录
 * - 性能监控：约束验证的性能统计和分析
 * - 测试工具：约束逻辑的单元测试和集成测试
 * - 诊断工具：约束问题的自动化诊断和修复建议
 */

#ifndef SQLCC_SQL_PARSER_CONSTRAINT_H
#define SQLCC_SQL_PARSER_CONSTRAINT_H

#include "ast/ast_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * 外键约束类
 */
class ForeignKeyConstraint {
public:
  // 级联操作类型
  enum CascadeAction {
    RESTRICT,
    CASCADE,
    SET_NULL,
    SET_DEFAULT,
    NO_ACTION
  };

  // 约束检查时机
  enum DeferrableMode {
    NOT_DEFERRABLE,     // 立即检查 (默认)
    DEFERRABLE,         // 可延迟检查
    INITIALLY_DEFERRED, // 初始延迟检查
    INITIALLY_IMMEDIATE // 初始立即检查
  };

  ForeignKeyConstraint(const std::vector<std::string> &columns,
                       const std::string &referenced_table,
                       const std::vector<std::string> &referenced_columns,
                       const std::string &name = "",
                       CascadeAction on_delete = RESTRICT,
                       CascadeAction on_update = RESTRICT,
                       DeferrableMode deferrable = NOT_DEFERRABLE);

  const std::vector<std::string> &getColumns() const;
  const std::string &getReferencedTable() const;
  const std::vector<std::string> &getReferencedColumns() const;
  const std::string &getName() const;
  CascadeAction getOnDeleteAction() const;
  CascadeAction getOnUpdateAction() const;
  DeferrableMode getDeferrableMode() const;

private:
  std::vector<std::string> columns_;
  std::string referenced_table_;
  std::vector<std::string> referenced_columns_;
  std::string name_;
  CascadeAction on_delete_;
  CascadeAction on_update_;
  DeferrableMode deferrable_;
};

/**
 * 检查约束类
 */
class CheckConstraint {
public:
  CheckConstraint(std::unique_ptr<Expression> condition,
                  const std::string &name = "");

  const Expression *getCondition() const;
  const std::string &getName() const;

private:
  std::unique_ptr<Expression> condition_;
  std::string name_;
};

/**
 * 主键约束类
 */
class PrimaryKeyConstraint {
public:
  PrimaryKeyConstraint(const std::vector<std::string> &columns,
                       const std::string &name = "");

  const std::vector<std::string> &getColumns() const;
  const std::string &getName() const;

private:
  std::vector<std::string> columns_;
  std::string name_;
};

/**
 * 唯一约束类
 */
class UniqueConstraint {
public:
  UniqueConstraint(const std::vector<std::string> &columns,
                   const std::string &name = "");

  const std::vector<std::string> &getColumns() const;
  const std::string &getName() const;

private:
  std::vector<std::string> columns_;
  std::string name_;
};

/**
 * 空值约束类
 */
class NotNullConstraint {
public:
  NotNullConstraint(const std::string &column, const std::string &name = "");

  const std::string &getColumn() const;
  const std::string &getName() const;

private:
  std::string column_;
  std::string name_;
};

/**
 * 断言约束类 (表间约束)
 */
class AssertionConstraint {
public:
  AssertionConstraint(std::unique_ptr<Expression> condition,
                      const std::string &name = "");

  const Expression *getCondition() const;
  const std::string &getName() const;

private:
  std::unique_ptr<Expression> condition_;
  std::string name_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_CONSTRAINT_H
