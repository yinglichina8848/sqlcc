/**
 * WHY: 为什么需要访问者模式来处理AST节点？
 *
 * 数据库系统需要对AST树进行多种不同的操作，传统方案存在诸多问题：
 * - 操作类型固定：难以添加新的AST操作类型和算法
 * - 代码分散重复：每个操作都需要遍历所有AST节点类型
 * - 类型安全缺失：运行时类型检查容易出错
 * - 扩展性差：新增AST节点类型需要修改所有相关操作
 * - 耦合度高：AST节点类和操作逻辑紧密耦合
 *
 * 访问者模式的核心价值：
 * 1. 操作解耦：将数据结构和操作算法完全分离
 * 2. 扩展灵活：新增操作只需实现新的访问者类
 * 3. 类型安全：编译时保证所有节点类型的访问方法
 * 4. 代码复用：同一访问者可应用于不同AST树
 * 5. 维护简便：AST节点修改不影响现有操作逻辑
 *
 * 🏗️ 设计模式：访问者模式(Visitor Pattern)
 *
 * 访问者模式作为行为型模式的经典应用：
 * - 双重分派：根据访问者和被访问对象的运行时类型选择方法
 * - 接口分离：元素接口和访问者接口完全独立
 * - 开放封闭：对扩展开放，对修改封闭
 * - 层次遍历：支持深度优先和广度优先的树遍历
 * - 类型安全：编译时保证方法调用的正确性
 *
 * SOLID原则体现：
 * - 单一职责：访问者类负责单一的AST操作逻辑
 * - 开闭原则：新操作通过扩展访问者实现
 * - 里氏替换：访问者子类可以替换基类使用
 * - 接口隔离：访问者接口精确定义所需方法
 * - 依赖倒置：AST节点依赖访问者接口而非实现
 *
 * WHAT: 访问者模式系统 - 统一的AST树操作框架
 *
 * 核心功能：
 * - 多态访问：根据节点类型自动调用相应的访问方法
 * - 操作抽象：将具体操作逻辑封装在访问者类中
 * - 树遍历：支持深度优先的AST树遍历算法
 * - 类型安全：编译时保证所有节点类型都被处理
 * - 扩展机制：支持动态添加新的AST操作类型
 *
 * 系统组件：
 * - NodeVisitor：抽象访问者基类，定义访问接口
 * - 具体访问者：实现特定操作的访问者子类
 * - AST节点：接受访问者访问的元素类
 * - 遍历算法：控制访问者遍历AST树的顺序
 * - 操作结果：收集和返回访问操作的结果
 *
 * 支持的访问操作：
 * - 语法检查：验证AST树的语法正确性
 * - 语义分析：进行类型检查和语义验证
 * - 代码生成：将AST转换为执行代码或字节码
 * - 优化重写：对AST进行查询优化和重写
 * - 格式化输出：生成格式化的SQL字符串
 * - 统计分析：收集AST的统计信息和度量
 *
 * 访问者类型：
 * - 只读访问：不修改AST节点的访问操作
 * - 改写访问：可以修改或重写AST节点的访问操作
 * - 收集访问：收集AST信息并返回结果的访问操作
 * - 验证访问：检查AST约束和规则的访问操作
 * - 转换访问：将AST转换为其他表示形式的访问操作
 *
 * 接口设计：
 * - 纯虚方法：为每种AST节点类型定义纯虚访问方法
 * - 默认实现：提供默认的空实现供子类选择性重写
 * - 返回值设计：根据操作类型设计合适的返回值
 * - 参数传递：通过方法参数传递上下文和配置信息
 * - 异常处理：定义访问过程中的异常处理策略
 *
 * HOW: 访问者模式系统的实现机制
 *
 * 访问者接口定义：
 * 1. 纯虚函数：为每种具体的AST节点类型定义visit方法
 * 2. 方法命名：使用visit前缀和节点类型名称
 * 3. 参数设计：接受节点对象的引用或指针
 * 4. 返回设计：根据操作需要设计返回值类型
 * 5. 异常规范：定义可能抛出的异常类型
 *
 * AST节点接受机制：
 * 1. accept方法：在每个AST节点类中实现accept方法
 * 2. 双重分派：accept方法调用访问者的对应visit方法
 * 3. 类型匹配：根据this指针的运行时类型选择visit方法
 * 4. 递归遍历：accept方法负责遍历子节点
 * 5. 上下文传递：将访问者对象传递给子节点
 *
 * 遍历算法实现：
 * 1. 深度优先：从根节点开始递归遍历所有子节点
 * 2. 前序遍历：先访问当前节点再访问子节点
 * 3. 后序遍历：先访问子节点再访问当前节点
 * 4. 中序遍历：对二元节点采用左根右的访问顺序
 * 5. 自定义遍历：支持根据需要自定义遍历顺序
 *
 * 类型安全保证：
 * 1. 编译检查：编译器保证所有visit方法都被定义
 * 2. 运行匹配：运行时根据实际类型调用正确方法
 * 3. 向下转换：安全的dynamic_cast或static_cast
 * 4. 异常处理：处理类型转换失败的异常情况
 * 5. 断言验证：在调试模式下验证类型正确性
 *
 * 性能优化策略：
 * - 虚函数表：利用编译器的虚函数表优化方法分派
 * - 内联优化：对简单访问方法进行内联优化
 * - 缓存机制：缓存访问结果避免重复计算
 * - 批量处理：支持批量AST节点的访问操作
 * - 并发访问：允许多个访问者并行访问不同子树
 *
 * 内存管理策略：
 * - 对象生命周期：访问者对象的生命周期管理
 * - 引用传递：使用引用避免不必要的对象拷贝
 * - 智能指针：对复杂对象使用智能指针管理
 * - 栈分配：对小对象使用栈分配提高性能
 * - 内存池：使用对象池减少动态分配开销
 *
 * 错误处理机制：
 * - 类型错误：访问不存在节点类型的错误处理
 * - 状态错误：访问者状态不正确的错误处理
 * - 递归错误：递归深度过大的错误处理
 * - 资源错误：内存不足等资源错误的处理
 * - 逻辑错误：访问逻辑错误的异常处理
 *
 * 扩展性设计：
 * - 插件架构：支持动态加载新的访问者类型
 * - 配置化：访问者行为的配置化管理
 * - 组合访问：支持多个访问者的组合使用
 * - 条件访问：支持条件执行的访问逻辑
 * - 自定义访问：允许用户定义自定义的访问逻辑
 *
 * 调试和诊断：
 * - 访问跟踪：详细记录访问者的执行轨迹
 * - 状态监控：监控访问者的内部状态变化
 * - 性能分析：分析访问操作的性能瓶颈
 * - 结果验证：验证访问结果的正确性和完整性
 * - 可视化工具：可视化AST树的访问过程和结果
 */

#ifndef SQLCC_SQL_PARSER_NODE_VISITOR_H
#define SQLCC_SQL_PARSER_NODE_VISITOR_H

#include "token.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// Forward declarations for all AST node classes
class CreateStatement;
class CreateViewStatement;
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
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
class CommitStatement;
class RollbackStatement;
class BinaryExpression;
class IdentifierExpression;
class StringLiteralExpression;
class NumericLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;
class CreateProcedureStatement;
class CallProcedureStatement;
class DropProcedureStatement;
class CreateTriggerStatement;
class DropTriggerStatement;
class AlterTriggerStatement;
class AlterViewStatement;
class DropViewStatement;
class Expression;
class SetOperation;
class CompositeSelectStatement;
class WindowFunction;
class WindowSpecification;
class WithRecursiveClause;

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  // 为每种具体的AST节点类型提供visit方法
  virtual void visit(CreateStatement &node) = 0;
  virtual void visit(CreateViewStatement &node) = 0;
  virtual void visit(SelectStatement &node) = 0;
  virtual void visit(InsertStatement &node) = 0;
  virtual void visit(UpdateStatement &node) = 0;
  virtual void visit(DeleteStatement &node) = 0;
  virtual void visit(DropStatement &node) = 0;
  virtual void visit(AlterStatement &node) = 0;
  virtual void visit(UseStatement &node) = 0;
  virtual void visit(CreateIndexStatement &node) = 0;
  virtual void visit(DropIndexStatement &node) = 0;
  virtual void visit(CreateUserStatement &node) = 0;
  virtual void visit(DropUserStatement &node) = 0;
  virtual void visit(GrantStatement &node) = 0;
  virtual void visit(RevokeStatement &node) = 0;
  virtual void visit(ShowStatement &node) = 0;
  virtual void visit(CommitStatement &node) = 0;
  virtual void visit(RollbackStatement &node) = 0;
  virtual void visit(CreateProcedureStatement &node) = 0;
  virtual void visit(CallProcedureStatement &node) = 0;
  virtual void visit(DropProcedureStatement &node) = 0;
  virtual void visit(CreateTriggerStatement &node) = 0;
  virtual void visit(DropTriggerStatement &node) = 0;
  virtual void visit(AlterTriggerStatement &node) = 0;
  virtual void visit(AlterViewStatement &node) = 0;
  virtual void visit(DropViewStatement &node) = 0;

  // 表达式访问方法
  virtual void visit(BinaryExpression &node) = 0;
  virtual void visit(IdentifierExpression &node) = 0;
  virtual void visit(StringLiteralExpression &node) = 0;
  virtual void visit(NumericLiteralExpression &node) = 0;
  virtual void visit(BooleanLiteralExpression &node) = 0;
  virtual void visit(NullLiteralExpression &node) = 0;

  // 集合操作访问方法
  virtual void visit(SetOperation &node) = 0;
  virtual void visit(CompositeSelectStatement &node) = 0;
  
  // 窗口函数访问方法
  virtual void visit(WindowFunction &node) = 0;
  virtual void visit(WindowSpecification &node) = 0;
  virtual void visit(WithRecursiveClause &node) = 0;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_NODE_VISITOR_H
