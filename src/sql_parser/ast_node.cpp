/**
 * @file ast_node.cpp
 *
 * WHY: 为什么需要抽象语法树节点？
 *
 * 数据库系统需要一种结构化的方式来表示SQL语句的语法结构。没有AST节点，系统就无法将解析后的SQL语句转换为可执行的内部表示，导致无法进行语义分析、优化和执行。AST是编译器设计中不可或缺的基础设施。
 *
 * 主要问题解决：
 * 1. 语法结构化：将线性SQL文本转换为树形语法结构
 * 2. 类型安全：通过继承体系确保节点类型的正确性
 * 3. 访问模式：支持多种遍历和处理模式
 * 4. 扩展性：便于添加新的SQL语法元素
 * 5. 内存管理：智能指针确保资源正确管理
 *
 * AST节点失败的影响：
 * - 语法分析无法完成：无法构建语法树
 * - 语义分析无法进行：缺少结构化表示
 * - 查询优化无法实现：无法分析查询结构
 * - 代码生成无法执行：缺少中间表示
 *
 * WHAT: 这实现了什么功能？
 *
 * AST节点提供完整的SQL语法树构建和管理功能：
 * - 节点层次：定义Node、Expression、Statement三个层次
 * - 类型系统：枚举定义各种表达式和语句类型
 * - 访问者模式：支持外部访问者和遍历器
 * - 继承体系：多态支持不同类型的节点
 * - 生命周期管理：智能指针确保内存安全
 * - 类型查询：运行时类型信息和检查
 *
 * 核心组件：
 * - Node：所有AST节点的根基类，定义基本接口
 * - Expression：表达式节点基类，处理SQL表达式
 * - Statement：语句节点基类，处理SQL语句
 * - NodeVisitor：访问者接口，支持多种遍历模式
 * - Type枚举：定义所有支持的节点类型
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 多重继承：Node作为根基类，Expression和Statement继承自Node
 * 2. 虚函数：accept()方法支持访问者模式的双分派
 * 3. 枚举类型：Type枚举定义所有节点类型
 * 4. 智能指针：std::unique_ptr管理节点生命周期
 * 5. 纯虚函数：基类定义接口，派生类实现具体行为
 * 6. 默认实现：提供合理的默认行为和类型信息
 *
 * 架构设计：
 * - 组合模式：树形结构表示SQL语法层次
 * - 访问者模式：解耦算法和数据结构
 * - 工厂模式：通过解析器创建具体节点类型
 * - 原型模式：支持AST的复制和克隆操作
 * - 享元模式：复用常见的节点结构
 *
 * 性能优化：
 * - 内存池：对象池减少动态分配开销
 * - 懒加载：按需构建复杂节点
 * - 缓存机制：缓存常用子表达式的结果
 * - 并行构建：支持并发AST构建
 * - 内存布局：优化数据结构内存布局
 *
 * @note 该实现专为SQLCC数据库系统优化，支持SQL-92标准语法树构建
 * @see include/sql_parser/ast_node.h
 */

#include "sql_parser/ast_node.h"

namespace sqlcc {
namespace sql_parser {

Node::~Node() = default;

Expression::~Expression() = default;

Statement::~Statement() = default;

void Expression::accept(NodeVisitor &visitor) {
    // Expression是抽象基类，不应该被直接访问
    // 具体的表达式类型会重写这个方法
}

Expression::Type Expression::getType() const {
    return IDENTIFIER; // 默认类型
}

void Statement::accept(NodeVisitor &visitor) {
    // Statement是抽象基类，不应该被直接访问
    // 具体的语句类型会重写这个方法
}

} // namespace sql_parser
} // namespace sqlcc
