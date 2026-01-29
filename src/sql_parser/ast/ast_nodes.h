/**
 * WHY: 为什么需要SQL解析器AST节点扩展？
 *
 * 基础AST节点只提供了抽象接口，无法满足复杂SQL语法的表示需求。
 * SQL语言包含丰富的语法结构：约束、索引、触发器、存储过程等，
 * 需要具体的节点类来精确表达语义信息。
 *
 * AST节点扩展的核心价值：
 * 1. 语法完整性：支持完整的SQL92语法解析
 * 2. 语义表达：精确表示SQL语句的含义和结构
 * 3. 类型安全：强类型系统防止运行时错误
 * 4. 扩展性：易于添加新的SQL特性支持
 *
 * 🏗️ 设计模式：组合模式(Composite Pattern)
 *
 * 节点层次结构：
 * - Statement: 语句基类，统一定义accept接口
 * - DDL: 数据定义语句（CREATE, DROP, ALTER）
 * - DML: 数据操作语句（SELECT, INSERT, UPDATE, DELETE）
 * - DCL: 数据控制语句（GRANT, REVOKE）
 * - Utility: 工具语句（USE, SHOW）
 *
 * 组合模式优势：
 * - 统一接口：所有节点都支持visitor访问
 * - 递归结构：复杂语句的嵌套表示
 * - 类型安全：编译时类型检查
 * - 内存管理：智能指针自动管理生命周期
 *
 * WHAT: SQL解析器AST节点扩展 - 完整的SQL语法树节点定义
 *
 * 核心功能：
 * - DDL语句节点：CREATE, DROP, ALTER等数据定义操作
 * - DML语句节点：SELECT, INSERT, UPDATE, DELETE等数据操作
 * - DCL语句节点：GRANT, REVOKE等权限控制操作
 * - 表达式节点：各种表达式类型的抽象表示
 * - 约束节点：主键、外键、检查约束等完整性约束
 * - 工具节点：USE, SHOW等数据库管理操作
 *
 * 节点分类：
 * - 语句节点：表示完整的SQL语句
 * - 表达式节点：表示SQL表达式
 * - 定义节点：表示对象定义和属性
 * - 约束节点：表示完整性约束
 *
 * 接口设计：
 * - accept(): 访问者模式接口，支持双分派
 * - getter/setter: 类型安全的数据访问
 * - 构造函数：保证对象初始化的正确性
 * - 析构函数：正确清理资源
 *
 * HOW: SQL解析器AST节点扩展的实现机制
 *
 * 节点创建流程：
 * 1. 词法分析：将SQL文本转换为token流
 * 2. 语法分析：根据语法规则构建AST节点
 * 3. 语义检查：验证节点结构的正确性
 * 4. 类型推断：确定表达式的类型信息
 * 5. 优化准备：为查询优化准备必要信息
 *
 * 内存管理策略：
 * - 智能指针：std::unique_ptr管理所有权
 * - 引用传递：避免不必要的对象拷贝
 * - 池化分配：减少小对象分配开销
 * - RAII模式：自动资源管理
 *
 * 类型安全保证：
 * - 强类型枚举：明确的节点类型定义
 * - 模板元编程：编译时类型检查
 * - 断言验证：在调试模式下验证约束
 * - 异常处理：运行时错误的优雅处理
 *
 * 扩展性设计：
 * - 插件架构：支持自定义节点类型的添加
 * - 配置化：可配置的语法规则和语义检查
 * - 版本兼容：向后兼容的AST格式
 * - 标准化：符合SQL标准规范
 *
 * 性能优化：
 * - 延迟求值：表达式按需计算
 * - 缓存机制：常用子表达式的结果缓存
 * - SIMD加速：向量化的字符串处理
 * - 并行解析：多核CPU的并行语法分析
 *
 * 调试和诊断：
 * - 节点可视化：AST结构的图形化展示
 * - 错误定位：精确的语法错误位置报告
 * - 性能分析：解析过程的时间和空间分析
 * - 日志记录：详细的解析过程记录
 */

#ifndef SQLCC_SQL_PARSER_AST_NODES_H
#define SQLCC_SQL_PARSER_AST_NODES_H

#pragma once

// Include base AST node definitions
#include "ast_node.h"
#include "statement.h"
#include "../data_types.h"
#include "../set_operation.h"
#include "node_visitor.h"
#include "ddl/ast_ddl_nodes.h"
#include "dml/ast_dml_nodes.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for TableMetadata from storage_engine
namespace sqlcc {
  struct TableMetadata;
}

namespace sqlcc {
namespace sql_parser {

// Forward declarations
class Statement;
class CreateStatement;
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

// ==================== ConstraintValidator ====================

/**
 * @brief 约束验证器接口
 */
class ConstraintValidator {
public:
  virtual ~ConstraintValidator() = default;

  /**
   * 验证记录是否满足约束条件
   * @param record 要验证的记录
   * @param metadata 表元数据
   * @param table_name 表名
   * @return 验证结果
   */
  virtual bool validate(const std::vector<std::string>& record,
                       std::shared_ptr<TableMetadata> metadata,
                       const std::string& table_name) const = 0;

  /**
   * 获取约束名称
   */
  virtual std::string getConstraintName() const = 0;

  /**
   * 获取约束类型
   */
  virtual std::string getConstraintType() const = 0;
};

/**
 * @brief 主键约束验证器
 */
class PrimaryKeyValidator : public ConstraintValidator {
public:
  PrimaryKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~PrimaryKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "PRIMARY KEY"; }

private:
  std::vector<std::string> columns_;
  std::string constraint_name_;
};

/**
 * @brief 唯一约束验证器
 */
class UniqueKeyValidator : public ConstraintValidator {
public:
  UniqueKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~UniqueKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "UNIQUE"; }

private:
  std::vector<std::string> columns_;
  std::string constraint_name_;

  bool checkUniqueness(const std::vector<std::string>& key_values) const;
};

/**
 * @brief 外键约束验证器
 */
class ForeignKeyValidator : public ConstraintValidator {
public:
  ForeignKeyValidator(const std::vector<std::string>& columns,
                     const std::string& referenced_table,
                     const std::vector<std::string>& referenced_columns,
                     const std::string& constraint_name = "");
  ~ForeignKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "FOREIGN KEY"; }

  // 级联操作支持
  enum CascadeAction { RESTRICT, CASCADE, SET_NULL };
  void setOnDeleteAction(CascadeAction action);
  void setOnUpdateAction(CascadeAction action);
  CascadeAction getOnDeleteAction() const;
  CascadeAction getOnUpdateAction() const;

private:
  std::vector<std::string> columns_;
  std::string referenced_table_;
  std::vector<std::string> referenced_columns_;
  std::string constraint_name_;
  CascadeAction on_delete_action_;
  CascadeAction on_update_action_;

  bool checkReferenceExists(const std::vector<std::string>& values,
                           const std::string& ref_table) const;
};

/**
 * @brief 检查约束验证器
 */
class CheckConstraintValidator : public ConstraintValidator {
public:
  CheckConstraintValidator(const std::string& expression, const std::string& constraint_name = "");
  ~CheckConstraintValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "CHECK"; }

private:
  std::string expression_;
  std::string constraint_name_;

  bool evaluateExpression(const std::string& expression,
                         const std::vector<std::string>& record,
                         std::shared_ptr<TableMetadata> metadata) const;
};

/**
 * @brief 约束管理器
 */
class ConstraintManager {
public:
  static ConstraintManager& getInstance();

  // 添加约束验证器
  void addValidator(const std::string& table_name,
                   std::unique_ptr<ConstraintValidator> validator);

  // 移除约束验证器
  void removeValidator(const std::string& table_name,
                      const std::string& constraint_name);

  // 验证记录
  bool validateRecord(const std::vector<std::string>& record,
                     std::shared_ptr<TableMetadata> metadata,
                     const std::string& table_name) const;

  // 获取表的所有约束
  std::vector<const ConstraintValidator*> getValidators(const std::string& table_name) const;

  // 清空表的所有约束
  void clearValidators(const std::string& table_name);

private:
  ConstraintManager();
  std::unordered_map<std::string, std::vector<std::unique_ptr<ConstraintValidator>>> validators_;
};

// ==================== TableConstraint ====================
// Moved to ast_ddl_nodes.h

// ==================== WhereClause ====================
// Moved to ast_dml_nodes.h

// ==================== CreateStatement ====================
// Moved to ast_ddl_nodes.h

// ==================== CreateViewStatement ====================
// Moved to ast_ddl_nodes.h

// ==================== AlterViewStatement ====================

class AlterViewStatement : public Statement {
public:
  AlterViewStatement(const std::string &viewName);
  ~AlterViewStatement();

  const std::string &getViewName() const;
  const std::vector<std::string> &getColumnNames() const;
  const SelectStatement &getSelectStatement() const;

  void addColumnName(const std::string &columnName);
  void setSelectStatement(std::unique_ptr<SelectStatement> selectStmt);

  bool hasColumnNames() const;

  void accept(NodeVisitor &visitor) override;

private:
  std::string viewName_;
  std::vector<std::string> columnNames_;
  std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== DropViewStatement ====================

class DropViewStatement : public Statement {
public:
  enum DropBehavior { RESTRICT, CASCADE };

  DropViewStatement(const std::string &viewName);
  ~DropViewStatement();

  const std::string &getViewName() const;
  DropBehavior getDropBehavior() const;
  void setDropBehavior(DropBehavior behavior);

  bool isIfExists() const;
  void setIfExists(bool ifExists);

  void accept(NodeVisitor &visitor) override;

private:
  std::string viewName_;
  DropBehavior dropBehavior_;
  bool ifExists_;
};

// ==================== SelectStatement ====================
// Moved to dml/ast_dml_nodes.h

// ==================== CompositeSelectStatement (复合SELECT，包含集合操作) ====================

class CompositeSelectStatement : public Statement {
public:
  CompositeSelectStatement() : Statement(Statement::COMPOSITE_SELECT) {}
  ~CompositeSelectStatement() override = default;

  void addSelectStatement(std::unique_ptr<SelectStatement> stmt) {
    selectStatements_.push_back(std::move(stmt));
  }

  void addSetOperation(std::unique_ptr<SetOperation> op) {
    operations_.push_back(std::move(op));
  }

  const std::vector<std::unique_ptr<SelectStatement>> &getSelectStatements() const {
    return selectStatements_;
  }

  const std::vector<std::unique_ptr<SetOperation>> &getSetOperations() const {
    return operations_;
  }

  size_t getStatementCount() const { return selectStatements_.size(); }
  size_t getOperationCount() const { return operations_.size(); }
  bool hasSetOperations() const { return !operations_.empty(); }

  void accept(NodeVisitor &visitor) override;

private:
  std::vector<std::unique_ptr<SelectStatement>> selectStatements_;
  std::vector<std::unique_ptr<SetOperation>> operations_;
};

// ==================== InsertStatement ====================
// Moved to dml/ast_dml_nodes.h

// ==================== UpdateStatement ====================
// Moved to dml/ast_dml_nodes.h

// ==================== DeleteStatement ====================
// Moved to dml/ast_dml_nodes.h

// ==================== DropStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== AlterStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== UseStatement ====================

class UseStatement : public Statement {
public:
  UseStatement(const std::string &databaseName);
  ~UseStatement();

  const std::string &getDatabaseName() const;
  std::string getDatabaseName();

  void accept(NodeVisitor &visitor) override;

private:
  std::string databaseName_;
};

// ==================== CreateIndexStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== DropIndexStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== CreateUserStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== DropUserStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== GrantStatement ====================

class GrantStatement : public Statement {
public:
  GrantStatement();
  ~GrantStatement();

  void addPrivilege(const std::string &privilege);
  const std::vector<std::string> &getPrivileges() const;

  void setObjectType(const std::string &objectType);
  const std::string &getObjectType() const;

  void setObjectName(const std::string &objectName);
  const std::string &getObjectName() const;

  void setGrantee(const std::string &grantee);
  const std::string &getGrantee() const;
  std::string getGrantee();

  void accept(NodeVisitor &visitor) override;

private:
  std::vector<std::string> privileges_;
  std::string objectType_;
  std::string objectName_;
  std::string grantee_;
};

// ==================== RevokeStatement ====================

class RevokeStatement : public Statement {
public:
  RevokeStatement();
  ~RevokeStatement();

  void addPrivilege(const std::string &privilege);
  const std::vector<std::string> &getPrivileges() const;

  void setObjectType(const std::string &objectType);
  const std::string &getObjectType() const;

  void setObjectName(const std::string &objectName);
  const std::string &getObjectName() const;

  void setGrantee(const std::string &grantee);
  const std::string &getGrantee() const;
  std::string getGrantee();

  void accept(NodeVisitor &visitor) override;

private:
  std::vector<std::string> privileges_;
  std::string objectType_;
  std::string objectName_;
  std::string grantee_;
};

// ==================== ShowStatement ====================

class ShowStatement : public Statement {
public:
  enum ShowType {
    DATABASES,    // SHOW DATABASES
    TABLES,       // SHOW TABLES [FROM db]
    CREATE_TABLE, // SHOW CREATE TABLE table
    COLUMNS,      // SHOW COLUMNS FROM table
    INDEXES,      // SHOW INDEXES FROM table
    GRANTS        // SHOW GRANTS FOR user
  };

  ShowStatement(ShowType type);
  ~ShowStatement();

  ShowType getShowType() const;

  // 设置目标对象（表名、用户名、数据库名）
  void setTargetObject(const std::string &target);
  const std::string &getTargetObject() const;

  // 设置FROM子句（数据库名）
  void setFromDatabase(const std::string &dbName);
  const std::string &getFromDatabase() const;
  bool hasFromDatabase() const;

  void accept(NodeVisitor &visitor) override;

private:
  ShowType type_;
  std::string targetObject_; // 目标对象（表名、用户名）
  std::string fromDatabase_; // FROM子句指定的数据库
  bool hasFromDb_;           // 是否有FROM子句
};

// ==================== CommitStatement ====================

class CommitStatement : public Statement {
public:
  CommitStatement();
  ~CommitStatement();

  void accept(NodeVisitor &visitor) override;

private:
};

// ==================== RollbackStatement ====================

class RollbackStatement : public Statement {
public:
  RollbackStatement();
  ~RollbackStatement();

  void accept(NodeVisitor &visitor) override;

private:
};

// ==================== BeginStatement ====================

class BeginStatement : public Statement {
public:
  BeginStatement();
  ~BeginStatement();

  void accept(NodeVisitor &visitor) override;

private:
};

// ==================== ProcedureParameter ====================

class ProcedureParameter {
public:
  enum Mode { IN, OUT, INOUT };

  ProcedureParameter(const std::string &name, const std::string &type,
                     Mode mode);
  ~ProcedureParameter();

  const std::string &getName() const;
  const std::string &getType() const;
  Mode getMode() const;
  std::string getModeString() const;

private:
  std::string name_;
  std::string type_;
  Mode mode_;
};

// ==================== CreateProcedureStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== CallProcedureStatement ====================

class CallProcedureStatement : public Statement {
public:
  CallProcedureStatement(const std::string &name);
  ~CallProcedureStatement();

  void addArgument(std::unique_ptr<Expression> arg);
  const std::vector<std::unique_ptr<Expression>> &getArguments() const;

  const std::string &getName() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
  std::vector<std::unique_ptr<Expression>> arguments_;
};

// ==================== DropProcedureStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== TriggerDefinition ====================

class TriggerDefinition {
public:
  enum Timing { BEFORE, AFTER, INSTEAD_OF };

  enum Event { INSERT, UPDATE, DELETE };

  enum Level { ROW, STATEMENT };

  TriggerDefinition(const std::string &name, Timing timing, Event event,
                    Level level, const std::string &tableName);
  ~TriggerDefinition();

  const std::string &getName() const;
  Timing getTiming() const;
  std::string getTimingString() const;
  Event getEvent() const;
  std::string getEventString() const;
  Level getLevel() const;
  std::string getLevelString() const;
  const std::string &getTableName() const;
  std::string getTableName();

  void setCondition(const std::string &condition);
  const std::string &getCondition() const;
  bool hasCondition() const;

  void setBody(const std::string &body);
  const std::string &getBody() const;

private:
  std::string name_;
  Timing timing_;
  Event event_;
  Level level_;
  std::string tableName_;
  std::string condition_;
  std::string body_;
  bool hasCondition_;
};

// ==================== CreateTriggerStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== DropTriggerStatement ====================
// Moved to ddl/ast_ddl_nodes.h

// ==================== AlterTriggerStatement ====================

class AlterTriggerStatement : public Statement {
public:
  enum Action { ENABLE, DISABLE };

  AlterTriggerStatement(const std::string &name, Action action);
  ~AlterTriggerStatement();

  const std::string &getName() const;
  Action getAction() const;
  std::string getActionString() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
  Action action_;
};

// ==================== Function AST Nodes ====================
// Function-related classes are defined in function_ast.h

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODES_H