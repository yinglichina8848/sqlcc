/**
 * @file procedure_vm.h
 * @brief SQLCC存储过程虚拟机 - 数据库服务端逻辑的沙盒执行环境
 *
 * ProcedureVM (Procedure Virtual Machine) 是一个轻量级的虚拟机，
 * 专门设计用于执行存储过程。它提供了一个隔离的执行环境，支持变量管理、
 * 参数传递、控制流（IF/WHILE）和 SQL 语句的内联执行。
 *
 * 📚 配套教材参考：
 * - [第13章：查询优化与执行](../../textbook/《数据库系统原理与开发实践》.md#第十三章查询优化与执行)
 * - [13.4 存储过程的执行原理](../../textbook/《数据库系统原理与开发实践》.md#134-存储过程的执行原理)
 * - [13.5 数据库虚拟机的设计](../../textbook/《数据库系统原理与开发实践》.md#135-数据库虚拟机的设计)
 *
 * WHY层 - 设计意图：
 *   1. **逻辑封装**：为存储过程提供一个独立的、可控的运行环境，与外部数据库操作隔离。
 *   2. **安全性**：通过沙盒机制限制存储过程的行为，防止恶意代码直接访问系统资源。
 *   3. **可移植性**：提供与特定 SQL 引擎无关的执行模型，理论上可支持多种过程语言。
 *   4. **性能**：通过预编译和高效的内部指令集，优化过程体的执行速度。
 *
 * WHAT层 - 功能说明：
 *   - 存储过程解析：将过程体解析为可执行的中间代码或 AST。
 *   - 变量管理：支持过程内的局部变量声明、赋值和作用域管理。
 *   - 参数传递：处理存储过程的输入/输出参数。
 *   - 控制流：实现 IF/ELSE, WHILE 循环等编程结构。
 *   - SQL 执行：能够将过程体内的 SQL 语句提交给底层的 SqlExecutor 执行。
 *   - 异常处理：捕获过程执行中的错误，并返回错误信息。
 *
 * HOW层 - 实现机制：
 *   - **基于 AST 的解释执行**：VM 直接遍历存储过程的抽象语法树（AST），根据节点类型执行相应操作。
 *   - **ProcedureContext**：作为 VM 的执行环境，维护局部变量、参数、调用栈和 SQL 执行器句柄。
 *   - **Value 类型封装**：统一处理 INTEGER, DOUBLE, STRING, BOOLEAN, NULL 等数据类型。
 *   - **SqlExecutorInterface 依赖**：VM 通过此接口与数据库的核心 SQL 执行功能交互。
 *   - **错误堆栈**：通过 `call_stack_` 跟踪执行路径，便于错误诊断。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#include "sql_parser/ast/ast_node.h"
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>
#include "procedure_parser.h"
#include "../core/sql_executor_interface.h"

namespace sqlcc {
namespace procedure {

// 前向声明 Procedure AST 节点
class ProcedureDefinition;
class VariableDeclaration;
class AssignmentStatement;
class IfStatement;
class WhileStatement;
class CallStatement;
class ProcedureVisitor; // 如果使用访问者模式

/**
 * @class Value
 * @brief 存储过程运行时数据类型封装
 *
 * WHY:
 *   存储过程语言需要一个统一的数据类型系统，能够透明地处理 SQL 类型与过程语言
 *   内部类型的转换，并支持 NULL 值。
 *
 * WHAT:
 *   封装了 INTEGER, DOUBLE, STRING, BOOLEAN, NULL_VALUE 五种基本数据类型。
 *   提供类型安全的访问方法（如 `asInteger()`），并在类型不匹配时抛出异常。
 *
 * HOW:
 *   使用 `std::variant` (如果支持) 或手动 `union + enum` 实现类型判别与存储。
 */
class Value {
public:
    enum Type { INTEGER, DOUBLE, STRING, BOOLEAN, NULL_VALUE };

    Value() : type_(NULL_VALUE), int_value_(0), double_value_(0.0), bool_value_(false) {}
    explicit Value(int val) : type_(INTEGER), int_value_(val), double_value_(0.0), bool_value_(false) {}
    explicit Value(double val) : type_(DOUBLE), int_value_(0), double_value_(val), bool_value_(false) {}
    explicit Value(const std::string& val) : type_(STRING), int_value_(0), double_value_(0.0), bool_value_(false), string_value_(val) {}
    explicit Value(bool val) : type_(BOOLEAN), int_value_(0), double_value_(0.0), bool_value_(val) {}

    Type getType() const { return type_; }
    int asInteger() const;
    double asDouble() const;
    const std::string& asString() const;
    bool asBoolean() const;
    std::string toString() const;

private:
    Type type_;
    int int_value_;
    double double_value_;
    bool bool_value_;
    std::string string_value_;
};

/**
 * @class ProcedureContext
 * @brief 存储过程执行上下文 - 维护过程的运行时状态
 *
 * WHY:
 *   每个存储过程的执行都需要一个独立的上下文来管理其局部变量、参数、
 *   返回值和当前的执行环境（如 SqlExecutor 句柄）。
 *
 * WHAT:
 *   - 变量/参数管理：存储过程内的局部变量和传递的参数。
 *   - 返回值：存储过程的最终返回值。
 *   - 调用栈：用于追踪过程嵌套调用，便于错误回溯。
 *   - SQL 执行器：提供执行 SQL 语句的能力。
 */
class ProcedureContext {
public:
    explicit ProcedureContext(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor);
    ~ProcedureContext();

    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;

    void setParameter(const std::string& name, const Value& value);
    Value getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;

    void setReturnValue(const Value& value);
    const Value& getReturnValue() const;

    void pushCallStack(const std::string& procedure_name);
    void popCallStack();
    const std::vector<std::string>& getCallStack() const;

private:
    std::unordered_map<std::string, Value> variables_;
    std::unordered_map<std::string, Value> parameters_;
    Value return_value_;
    std::vector<std::string> call_stack_;
    std::unique_ptr<sqlcc::core::SqlExecutorInterface> sql_executor_;
};

/**
 * @class ProcedureVM
 * @brief 存储过程虚拟机 - 解释执行存储过程 AST
 *
 * WHY:
 *   提供一个高效、安全的运行时环境，将存储过程的 AST 转化为实际的数据操作和控制流。
 *   它是 `StoredProcedureManager` 的底层执行单元。
 *
 * WHAT:
 *   - AST 遍历执行：核心 `execute` 方法遍历过程的 AST。
 *   - 语句解释器：针对变量声明、赋值、IF/WHILE、SQL 调用等语句提供执行逻辑。
 *   - 表达式求值：能够解析和计算过程语言中的表达式。
 *   - 错误管理：记录并返回执行过程中发生的错误。
 *
 * HOW:
 *   - 递归解释：`executeStatement` 递归调用各种 `executeXXX` 方法。
 *   - 表达式求值器：`evaluateExpression` 使用简化的规则或委托给专门的表达式解析器。
 *   - SQL 委托：通过 `sql_executor_` 调用数据库的 SQL 执行功能。
 */
class ProcedureVM {
public:
    explicit ProcedureVM(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor);
    ~ProcedureVM();

    /**
     * @brief 执行存储过程的主入口
     * @param procedure 存储过程的 AST 定义
     * @param context 存储过程执行上下文
     * @return 执行是否成功
     */
    bool execute(ProcedureDefinition* procedure, ProcedureContext& context);

    const std::string& getLastError() const;

private:
    bool executeStatement(Statement* statement, ProcedureContext& context);
    bool executeVariableDeclaration(VariableDeclaration* decl, ProcedureContext& context);
    bool executeAssignment(AssignmentStatement* assign, ProcedureContext& context);
    bool executeIfStatement(IfStatement* if_stmt, ProcedureContext& context);
    bool executeWhileStatement(WhileStatement* while_stmt, ProcedureContext& context);
    bool executeCallStatement(CallStatement* call_stmt, ProcedureContext& context);

    Value evaluateExpression(const std::string& expression, ProcedureContext& context);
    Value evaluateBinaryExpression(const std::string& left_str, const std::string& op,
                                  const std::string& right_str, ProcedureContext& context);
    bool evaluateCondition(const std::string& condition, ProcedureContext& context);

    bool executeSql(const std::string& sql, ProcedureContext& context);

    std::vector<std::string> splitExpression(const std::string& expr);
    std::string trim(const std::string& str) const;
    void setError(const std::string& error);

    std::unique_ptr<sqlcc::core::SqlExecutorInterface> sql_executor_;
    std::string last_error_;
};

} // namespace procedure
} // namespace sqlcc
