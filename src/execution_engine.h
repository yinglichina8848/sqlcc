/**
 * @file execution_engine.h
 * @brief SQLCC执行引擎 - SQL语句解释和执行的核心组件
 *
 * 执行引擎是SQLCC数据库系统的核心执行部件，负责将解析后的SQL抽象语法树（AST）
 * 转换为具体的数据库操作序列。通过精心设计的执行器分层架构，实现DDL、DML、DCL
 * 等不同类型SQL语句的高效执行，为上层应用提供统一的数据库操作接口。
 *
 * 📚 配套教材参考：
 * - [第9章：查询执行与优化](../../textbook/《数据库系统原理与开发实践》.md#第九章查询执行与优化)
 * - [9.1 查询执行模型](../../textbook/《数据库系统原理与开发实践》.md#91-查询执行模型)
 * - [9.2 执行器架构设计](../../textbook/《数据库系统原理与开发实践》.md#92-执行器架构设计)
 * - [9.3 查询优化策略](../../textbook/《数据库系统原理与开发实践》.md#93-查询优化策略)
 *
 * WHY层 - 设计意图：
 *   执行引擎是数据库系统的"执行大脑"，将抽象的SQL语句转换为具体的物理操作序列。
 *   通过分层执行器架构，实现不同类型SQL语句的专业化处理，同时保证执行的效率、
 *   正确性和安全性，为应用层提供可靠的数据库操作服务。
 *
 * WHAT层 - 功能说明：
 *   - SQL语句执行：支持SELECT、INSERT、UPDATE、DELETE等完整SQL语法
 *   - 执行器分层：DDL执行器、DML执行器、DCL执行器、工具执行器
 *   - 权限控制：基于用户的访问控制和权限检查
 *   - 执行上下文：管理执行过程中的状态信息和临时数据
 *   - 结果处理：统一的执行结果格式化和返回机制
 *
 * HOW层 - 实现机制：
 *   - 访问者模式：通过visit方法遍历AST节点实现语句执行
 *   - 策略模式：不同类型的执行器实现不同的执行策略
 *   - 组合模式：复杂查询通过多个执行器的组合完成
 *   - 模板方法：统一的执行流程框架，子类实现具体逻辑
 *   - 工厂模式：根据语句类型创建对应的执行器实例
 *
 * 执行器架构详解：
 *   1. **ExecutionEngine基类**：定义统一的执行接口和执行上下文管理
 *   2. **DDLExecutor**：处理数据定义语言，管理数据库对象的生命周期
 *   3. **DMLExecutor**：处理数据操作语言，实现CRUD操作的核心逻辑
 *   4. **DCLExecutor**：处理数据控制语言，管理用户权限和访问控制
 *   5. **UtilityExecutor**：处理工具语句，提供数据库状态查询功能
 *
 * 执行流程设计：
 *   1. **语句接收**：从SQL解析器获取AST语句对象
 *   2. **执行器选择**：根据语句类型选择合适的执行器
 *   3. **权限检查**：验证用户是否有执行该操作的权限
 *   4. **执行准备**：初始化执行上下文和必要的数据结构
 *   5. **执行操作**：调用具体的执行逻辑处理语句
 *   6. **结果处理**：格式化执行结果返回给客户端
 *   7. **清理工作**：释放临时资源和清理执行状态
 *
 * 性能优化策略：
 *   - **查询优化**：基于索引的查询优化和执行计划选择
 *   - **批量操作**：支持批量插入、更新和删除操作
 *   - **缓存机制**：元数据缓存和查询结果缓存
 *   - **并发控制**：事务级别的并发控制和锁管理
 *   - **资源管理**：内存使用限制和垃圾回收机制
 *
 * 安全性保障：
 *   - **SQL注入防护**：参数化查询和输入验证
 *   - **权限分级**：细粒度的权限控制和访问审计
 *   - **事务隔离**：确保操作的原子性和一致性
 *   - **错误处理**：完善的异常处理和错误恢复机制
 *   - **日志记录**：详细的操作日志用于审计和调试
 *
 * 扩展性设计：
 *   - **插件架构**：支持自定义执行器和函数扩展
 *   - **分布式执行**：跨节点查询的分布式执行支持
 *   - **流式处理**：大数据集的流式处理和管道操作
 *   - **缓存集成**：与外部缓存系统的集成支持
 *   - **监控集成**：与监控系统的集成和指标收集
 *
 * 监控和诊断：
 *   - **执行统计**：查询执行时间、I/O操作、缓存命中率
 *   - **性能指标**：QPS、响应时间、并发连接数
 *   - **错误统计**：各类错误的发生频率和分布
 *   - **资源使用**：CPU、内存、磁盘I/O的使用情况
 *   - **慢查询分析**：识别和优化慢查询操作
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2025-12-24
 */

#include "src/sql_parser/ast/ast_node.h"
#ifndef SQLCC_EXECUTION_ENGINE_H
#define SQLCC_EXECUTION_ENGINE_H

#include "src/core/execution_context.h" // 包含ExecutionContext定义
#include "src/core/execution_result.h"  // 包含完整的ExecutionResult定义
#include "src/core/system_database.h"
#include "src/core/user_manager.h"
#include "src/sql_parser/ast/ast_nodes.h"
#include "src/storage_engine/b_plus_tree.h"
#include "src/storage_engine/table_storage.h"
#include "src/storage_engine/storage_engine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

/**
 * @brief 执行引擎接口 - SQL语句执行的核心抽象
 *
 * WHY层 - 设计意图：
 *   ExecutionEngine定义了SQL语句执行的统一接口，抽象了不同类型语句的执行逻辑。
 *   通过接口分离和多态设计，实现执行逻辑的可扩展性和可维护性，为不同类型的
 *   SQL语句提供一致的执行入口和结果处理机制。
 *
 * WHAT层 - 接口定义：
 *   - execute方法：统一的SQL语句执行入口
 *   - 执行上下文管理：维护执行过程中的状态信息
 *   - 数据库管理器集成：与底层存储引擎的接口
 *   - 结果格式化：统一的执行结果处理和返回
 *
 * HOW层 - 接口实现：
 *   - 纯虚函数定义：确保子类必须实现核心执行逻辑
 *   - 默认参数处理：提供灵活的执行选项配置
 *   - 智能指针管理：自动资源管理和内存安全
 *   - 异常安全：完善的异常处理和资源清理
 *
 * 接口设计原则：
 *   - **单一职责**：每个执行器专注于一种类型的SQL语句
 *   - **开闭原则**：支持新类型执行器的扩展而不修改现有代码
 *   - **依赖倒置**：通过接口依赖而不是具体实现
 *   - **里氏替换**：所有子类都可以替换父类使用
 */
class ExecutionEngine {
protected:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<ExecutionContext> execution_context_; // 执行上下文

public:
  ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager);
  virtual ~ExecutionEngine() = default;

  /**
   * 执行SQL语句
   */
  virtual ExecutionResult
  execute(std::unique_ptr<sql_parser::Statement> stmt) = 0;

  /**
   * 执行SQL语句，带执行上下文
   * 默认实现使用默认执行上下文
   */
  virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  std::shared_ptr<ExecutionContext> context) {
    // 默认实现：调用无上下文版本
    return execute(std::move(stmt));
  }

  /**
   * 设置执行上下文
   */
  virtual void set_execution_context(std::shared_ptr<ExecutionContext> context);

  /**
   * 获取执行上下文
   */
  virtual std::shared_ptr<ExecutionContext> get_execution_context() const;
};

/**
 * @brief DDL执行器 - 处理数据定义语言
 *
 * WHY层 - 设计意图：
 *   DDLExecutor专门处理数据定义语言（CREATE、DROP、ALTER等），负责数据库对象的
 *   生命周期管理。通过集中的DDL处理逻辑，确保数据结构变更的原子性和一致性，
 *   为数据库模式演化提供可靠的基础设施。
 *
 * WHAT层 - DDL操作支持：
 *   - CREATE TABLE：创建新表结构和约束
 *   - DROP TABLE：删除表及其相关对象
 *   - ALTER TABLE：修改表结构和属性
 *   - CREATE INDEX：创建索引以优化查询性能
 *   - DROP INDEX：删除不再需要的索引
 *
 * HOW层 - DDL执行策略：
 *   - 元数据管理：更新系统表和元数据缓存
 *   - 依赖检查：确保操作不会破坏数据完整性
 *   - 权限验证：检查用户是否有DDL操作权限
 *   - 事务保证：DDL操作的事务性保障
 *   - 回滚支持：失败时的自动清理机制
 */
class DDLExecutor : public ExecutionEngine {
public:
  DDLExecutor(std::shared_ptr<DatabaseManager> db_manager);
  DDLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<SystemDatabase> system_db,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult
  execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeCreate(std::unique_ptr<sqlcc::sql_parser::CreateStatement> stmt);
  ExecutionResult executeDrop(std::unique_ptr<sqlcc::sql_parser::DropStatement> stmt);
  ExecutionResult executeAlter(std::unique_ptr<sqlcc::sql_parser::AlterStatement> stmt);
  ExecutionResult
  executeCreateIndex(std::unique_ptr<sqlcc::sql_parser::CreateIndexStatement> stmt);
  ExecutionResult executeDropIndex(std::unique_ptr<sqlcc::sql_parser::DropIndexStatement> stmt);

  // 权限检查
  bool checkDDLPermission(const std::string &operation,
                          const std::string &resource);

  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief DML执行器 - 处理数据操作语言
 *
 * WHY层 - 设计意图：
 *   DMLExecutor是数据库系统中最核心的执行器，处理所有数据操作语言（SELECT、INSERT、
 *   UPDATE、DELETE）。通过优化的查询执行算法和索引利用策略，实现高性能的数据
 *   操作，同时保证数据一致性和事务隔离性。
 *
 * WHAT层 - DML操作支持：
 *   - SELECT：复杂查询支持，包括JOIN、多表查询、子查询
 *   - INSERT：单行和批量数据插入操作
 *   - UPDATE：基于条件的批量数据更新
 *   - DELETE：基于条件的批量数据删除
 *   - 索引优化：自动选择最优的索引访问路径
 *
 * HOW层 - DML执行优化：
 *   - 查询规划：基于成本的执行计划选择
 *   - 索引利用：B+树索引的优化访问算法
 *   - 连接算法：嵌套循环、哈希连接、排序合并连接
 *   - 谓词下推：将WHERE条件尽可能下推到存储层
 *   - 结果缓存：查询结果的智能缓存机制
 *   - 批量处理：减少系统调用的批量操作优化
 */
class DMLExecutor : public ExecutionEngine {
public:
  DMLExecutor(std::shared_ptr<DatabaseManager> db_manager);
  DMLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult
  execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;

  // 公开的辅助方法（用于WHERE条件评估，可以被外部访问）
  bool compareValues(const std::string &left, const std::string &right,
                     const std::string &op);

public:
  // 索引优化查询方法 (设为公开以便测试)
  std::vector<std::pair<int32_t, size_t>>
  optimizeQueryWithIndex(const std::string &table_name,
                         const sql_parser::WhereClause &where_clause,
                         TableStorageManager &table_storage, bool &used_index,
                         std::string &index_info);

private:
  ExecutionResult executeInsert(std::unique_ptr<sqlcc::sql_parser::InsertStatement> stmt);
  ExecutionResult executeUpdate(std::unique_ptr<sqlcc::sql_parser::UpdateStatement> stmt);
  ExecutionResult executeDelete(std::unique_ptr<sqlcc::sql_parser::DeleteStatement> stmt);

  // 权限检查
  bool checkDMLPermission(const std::string &operation,
                          const std::string &table_name);

  // 辅助方法
  bool matchesWhereClause(const std::vector<std::string> &record,
                          const sqlcc::sql_parser::WhereClause &where_clause,
                          std::shared_ptr<TableMetadata> metadata);
  std::string getColumnValue(const std::vector<std::string> &record,
                             const std::string &column_name,
                             std::shared_ptr<TableMetadata> metadata);

  // WHERE条件评估辅助方法
  // TODO: 支持AND/OR组合条件
  // TODO: 支持IN操作符
  // TODO: 支持BETWEEN操作符
  // TODO: 支持LIKE模式匹配

  // 约束验证方法
  bool validateColumnConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);
  bool checkUniqueConstraints(const std::vector<std::string> &record,
                              std::shared_ptr<TableMetadata> metadata,
                              const std::string &table_name);
  bool checkPrimaryKeyConstraints(const std::vector<std::string> &record,
                                  std::shared_ptr<TableMetadata> metadata,
                                  const std::string &table_name);
  bool checkUniqueKeyConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);

  // 索引维护方法
  void maintainIndexesOnInsert(const std::vector<std::string> &record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset);
  void maintainIndexesOnUpdate(const std::vector<std::string> &old_record,
                               const std::vector<std::string> &new_record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset);
  void maintainIndexesOnDelete(const std::vector<std::string> &record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset);

  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief DCL执行器 - 处理数据控制语言
 *
 * WHY层 - 设计意图：
 *   DCLExecutor处理数据控制语言（GRANT、REVOKE、CREATE USER、DROP USER等），
 *   负责数据库系统的安全和访问控制。通过精细的权限管理机制，确保用户只能
 *   访问被授权的数据库对象和执行被允许的操作。
 *
 * WHAT层 - DCL操作支持：
 *   - CREATE USER：创建新的数据库用户账户
 *   - DROP USER：删除数据库用户账户
 *   - GRANT：为用户或角色授予权限
 *   - REVOKE：撤销用户或角色的权限
 *   - 角色管理：数据库角色的创建和管理
 *
 * HOW层 - 权限管理机制：
 *   - 访问控制列表：基于用户的权限检查
 *   - 角色层次结构：支持角色继承和权限聚合
 *   - 权限粒度：对象级、操作级、列级的权限控制
 *   - 审计日志：权限变更的完整审计记录
 *   - 权限缓存：权限检查结果的高效缓存
 */
class DCLExecutor : public ExecutionEngine {
public:
  DCLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeCreateUser(std::unique_ptr<sql_parser::CreateUserStatement> stmt);
  ExecutionResult executeDropUser(std::unique_ptr<sql_parser::DropUserStatement> stmt);
  ExecutionResult executeGrant(std::unique_ptr<sql_parser::GrantStatement> stmt);
  ExecutionResult executeRevoke(std::unique_ptr<sql_parser::RevokeStatement> stmt);

  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief 工具执行器 - 处理USE, SHOW等语句
 *
 * WHY层 - 设计意图：
 *   UtilityExecutor处理数据库管理工具语句（USE、SHOW、DESCRIBE等），提供
 *   数据库状态查询和元数据访问功能。通过统一的管理接口，为数据库管理员和
 *   应用开发人员提供便捷的数据库信息获取途径。
 *
 * WHAT层 - 工具操作支持：
 *   - USE：切换当前数据库上下文
 *   - SHOW DATABASES：列出所有数据库
 *   - SHOW TABLES：列出指定数据库的表
 *   - SHOW COLUMNS：显示表结构信息
 *   - DESCRIBE：表的详细结构描述
 *   - 系统状态查询：数据库运行状态信息
 *
 * HOW层 - 元数据访问策略：
 *   - 系统表查询：从系统表获取元数据信息
 *   - 缓存优化：元数据信息的智能缓存
 *   - 权限检查：确保用户只能查看被授权的信息
 *   - 格式化输出：用户友好的结果格式化
 *   - 分页支持：大数据集的分页查询机制
 */
class UtilityExecutor : public ExecutionEngine {
public:
  UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager);
  UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager,
                  std::shared_ptr<SystemDatabase> system_db);

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeShow(std::unique_ptr<sql_parser::ShowStatement> stmt);
  std::string formatDatabases(const std::vector<std::string> &databases);
  std::string formatTables(const std::vector<std::string> &tables);

  std::shared_ptr<SystemDatabase> system_db_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_ENGINE_H
