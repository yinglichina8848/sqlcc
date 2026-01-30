/**
 * WHY: 为什么需要AST节点的前向声明？
 *
 * 数据库系统的SQL解析器包含大量的AST节点类，这些类之间存在复杂的依赖关系：
 * - 执行层需要引用解析层的AST节点类型
 * - 解析层内部各个AST节点之间相互引用
 * - 头文件包含关系形成循环依赖，编译失败
 * - 编译时间过长，增量编译效率低下
 * - 模块化程度差，难以独立测试和维护
 *
 * 前向声明的核心价值：
 * 1. 打破循环依赖：使用前向声明避免头文件循环包含
 * 2. 降低编译耦合：减少不必要的头文件依赖关系
 * 3. 提高编译速度：减少头文件解析和宏展开时间
 * 4. 增强模块化：各模块可以独立编译和测试
 * 5. 简化依赖管理：清晰的模块间依赖关系图
 *
 * 🏗️ 设计模式：前向声明模式(Forward Declaration Pattern)
 *
 * 前向声明作为C++编译优化模式的经典应用：
 * - 最小化包含：只声明需要的类型，不包含完整定义
 * - 延迟绑定：将类型定义和使用分离，提高灵活性
 * - 接口抽象：通过指针和引用使用类型，隐藏实现细节
 * - 编译防火墙：防止头文件变更影响过多编译单元
 * - 增量编译：支持更高效的增量编译和链接
 *
 * SOLID原则体现：
 * - 单一职责：前向声明文件专注于打破循环依赖
 * - 开闭原则：新增AST节点只需在此文件添加声明
 * - 里氏替换：前向声明的类型可被具体实现替换
 * - 接口隔离：精确声明需要的类型，避免过度包含
 * - 依赖倒置：高层模块依赖前向声明而非具体实现
 *
 * WHAT: AST节点前向声明系统 - 循环依赖解决框架
 *
 * 核心功能：
 * - 节点类型声明：为所有AST节点类提供前向声明
 * - 依赖关系管理：管理AST节点之间的引用关系
 * - 编译优化：优化大型项目的编译时间和依赖
 * - 模块解耦：减少模块间的紧耦合关系
 * - 接口定义：定义各模块间的接口契约
 *
 * 系统组件：
 * - 语句节点：各种SQL语句的AST节点声明
 * - 表达式节点：SQL表达式的AST节点声明
 * - 约束节点：表约束和列约束的节点声明
 * - 函数节点：窗口函数和聚合函数的节点声明
 * - 其他节点：复合查询、连接等特殊节点的声明
 *
 * 前向声明策略：
 * - 基类优先：先声明基类，再声明派生类
 * - 按功能分组：将相关节点声明分组组织
 * - 最小化暴露：只声明必要的类型和关系
 * - 版本控制：声明版本随代码库演化更新
 * - 文档同步：声明与实际定义保持同步
 *
 * 包含策略：
 * - 执行层包含：执行引擎包含此文件而非ast_nodes.h
 * - 解析层保留：解析器内部仍可包含完整定义
 * - 条件包含：根据编译单元需求选择性包含
 * - 头文件卫士：使用头文件卫士防止重复包含
 * - 包含顺序：控制包含顺序避免隐藏依赖
 *
 * 接口设计：
 * - 声明完整性：包含所有必要的类型声明
 * - 命名空间：使用正确的命名空间限定
 * - 注释文档：为重要声明提供文档注释
 * - 版本标记：标识声明的版本和变更历史
 * - 兼容性保证：保证声明的向后兼容性
 *
 * HOW: AST前向声明系统的实现机制
 *
 * 声明组织策略：
 * 1. 基类声明：优先声明Statement、Expression等基类
 * 2. 分类组织：按功能将声明分组，语句、表达式、约束等
 * 3. 依赖顺序：确保基类声明在派生类之前
 * 4. 命名规范：使用一致的命名约定和注释风格
 * 5. 版本管理：为声明添加版本信息和变更记录
 *
 * 循环依赖解决：
 * 1. 识别循环：分析头文件包含图找出循环依赖
 * 2. 前向声明：对循环依赖的类型使用前向声明
 * 3. 接口抽象：通过纯虚函数和抽象接口减少耦合
 * 4. PIMPL模式：对复杂类使用指针到实现的模式
 * 5. 工厂模式：使用工厂函数创建具体对象实例
 *
 * 编译优化实现：
 * 1. 减少包含：使用前向声明减少不必要的头文件包含
 * 2. 条件编译：根据需要选择性包含头文件
 * 3. 预编译头：利用预编译头文件加速编译
 * 4. 模块化编译：将代码组织为独立编译的模块
 * 5. 并行编译：支持多核并行编译优化
 *
 * 依赖管理机制：
 * 1. 依赖分析：使用工具分析代码依赖关系
 * 2. 声明同步：保持前向声明与实际定义同步
 * 3. 版本控制：对声明变更进行版本控制
 * 4. 兼容性检查：检查声明变更的兼容性影响
 * 5. 重构支持：支持大规模代码重构的声明调整
 *
 * 错误检测机制：
 * 1. 声明完整性：检查所有必要的类型都已声明
 * 2. 拼写检查：验证声明的拼写和语法正确性
 * 3. 依赖一致性：确保声明与使用保持一致
 * 4. 编译验证：通过编译验证声明的正确性
 * 5. 静态分析：使用静态分析工具检查潜在问题
 *
 * 扩展性设计：
 * 1. 声明生成：使用代码生成工具自动生成声明
 * 2. 元数据驱动：基于元数据自动管理声明
 * 3. 配置化管理：通过配置文件管理声明策略
 * 4. 插件扩展：支持插件化的声明扩展机制
 * 5. 标准库集成：与标准库的前向声明机制集成
 *
 * 调试和诊断：
 * 1. 依赖可视化：可视化代码依赖关系图
 * 2. 编译时间分析：分析编译时间的瓶颈点
 * 3. 包含分析：分析头文件包含的频率和影响
 * 4. 错误追踪：追踪编译错误到具体的声明问题
 * 5. 性能监控：监控声明策略对编译性能的影响
 */

#include "ast_node.h"
#include "ast_nodes.h"
#ifndef SQLCC_SQL_PARSER_AST_FWD_H
#define SQLCC_SQL_PARSER_AST_FWD_H

// Forward declarations for AST nodes to break circular dependencies
// This file should be included by execution layer instead of ast_nodes.h

namespace sqlcc {
namespace sql_parser {

// Base classes
class Statement;
class Expression;
// NodeVisitor is defined in ast namespace, not here

// Statement types
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class CreateStatement;
class DropStatement;
class AlterStatement;
class UseStatement;
class CreateIndexStatement;
class DropIndexStatement;
class CreateUserStatement;
class DropUserStatement;
class GrantStatement;
class RevokeStatement;
class ShowStatement;
class CreateViewStatement;
class AlterViewStatement;
class DropViewStatement;
class CreateProcedureStatement;
class CallProcedureStatement;
class DropProcedureStatement;
class CreateTriggerStatement;
class DropTriggerStatement;
class AlterTriggerStatement;

// Expression types
class IdentifierExpression;
class StringLiteralExpression;
class NumericLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;

// Other types
class ColumnDefinition;
class TableConstraint;
class WhereClause;
class JoinClause;
class CompositeSelectStatement;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_FWD_H
