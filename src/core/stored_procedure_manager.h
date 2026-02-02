/**
 * @file stored_procedure_manager.h
 * @brief SQLCC存储过程与触发器管理器 - 数据库服务端编程逻辑核心
 *
 * StoredProcedureManager 是数据库系统支持服务端编程（Server-Side Programming）
 * 的核心组件。它管理着所有的存储过程（Stored Procedure）和触发器（Trigger），
 * 允许用户将复杂的业务逻辑封装在数据库内部执行，从而减少网络往返，提高性能，
 * 并保证数据完整性。
 *
 * 📚 配套教材参考：
 * - [第4章：SQL数据操作](../../textbook/《数据库系统原理与开发实践》.md#第四章sql数据操作)
 * - [4.4 存储过程与函数](../../textbook/《数据库系统原理与开发实践》.md#44-存储过程与函数)
 * - [4.5 触发器](../../textbook/《数据库系统原理与开发实践》.md#45-触发器)
 *
 * WHY层 - 设计意图：
 *   1. **逻辑封装**：将业务逻辑下沉到数据库层，提供统一的 API 接口，实现业务与数据解耦。
 *   2. **性能优化**：减少客户端与服务器之间的网络通信（例如，一个过程调用代替多次 SQL 交互）。
 *   3. **数据完整性**：通过触发器自动执行约束检查、审计日志记录或级联更新，防止逻辑漏洞。
 *   4. **安全性**：通过授予执行存储过程的权限而不是直接访问表的权限，实现细粒度的访问控制。
 *
 * WHAT层 - 功能说明：
 *   - 过程管理：Create/Drop/Get 存储过程，支持参数定义和 SQL 体。
 *   - 触发器管理：Create/Drop/Enable/Disable 触发器，支持 BEFORE/AFTER/INSTEAD OF 时机。
 *   - 执行引擎：提供 `ExecuteProcedure` 接口，解析参数并调用 SqlExecutor 执行过程体。
 *   - 事件分发：提供 `ExecuteTrigger` 接口，在 INSERT/UPDATE/DELETE 操作前后自动触发相关逻辑。
 *   - 元数据同步：将过程和触发器的定义持久化到系统表。
 *
 * HOW层 - 实现机制：
 *   - **内存缓存**：使用 `unordered_map` 缓存编译好的过程和触发器对象，加速查找和执行。
 *   - **参数绑定**：在执行时，将传入的实参绑定到过程体中的形参变量。
 *   - **触发器链**：同一事件可能有多个触发器，管理器负责按定义顺序依次执行。
 *   - **递归控制**：虽然本类不直接处理，但通常需配合执行器防止触发器无限递归。
 *   - **原子性**：过程和触发器的执行通常在调用者的事务上下文中运行，保证 ACID 属性。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

#include "../utils/config_manager.h"
#include "sql_parser/ast/ast_nodes.h"

namespace sqlcc {

class DatabaseManager;
class SqlExecutor;

/**
 * @struct ProcedureParameter
 * @brief 存储过程参数元数据
 */
struct ProcedureParameter {
  std::string name;           ///< 参数名（如 @p_id）
  std::string type;           ///< 数据类型（INT, VARCHAR...）
  bool is_output;             ///< 是否为输出参数 (OUT/INOUT)
  std::string default_value;  ///< 默认值表达式

  ProcedureParameter(const std::string& n, const std::string& t, bool out = false, const std::string& def = "")
      : name(n), type(t), is_output(out), default_value(def) {}
};

/**
 * @struct StoredProcedure
 * @brief 存储过程定义对象
 */
struct StoredProcedure {
  std::string name;           ///< 过程名
  std::string schema;         ///< 所属模式
  std::vector<ProcedureParameter> parameters; ///< 参数列表
  std::string body;           ///< 过程体（SQL语句块）
  std::string language;       ///< 实现语言 (SQL, PL/SQL, Java...)
  std::string owner;          ///< 所有者
  std::chrono::system_clock::time_point created_at;
  std::chrono::system_clock::time_point modified_at;

  StoredProcedure(const std::string& n, const std::string& s = "public")
      : name(n), schema(s), language("SQL") {
    created_at = modified_at = std::chrono::system_clock::now();
  }
};

/**
 * @struct Trigger
 * @brief 触发器定义对象
 */
struct Trigger {
  std::string name;           ///< 触发器名
  std::string table_name;     ///< 绑定的表名
  std::string event;          ///< 触发事件 (INSERT, UPDATE, DELETE)
  std::string timing;         ///< 触发时机 (BEFORE, AFTER, INSTEAD OF)
  std::vector<std::string> columns;  ///< 关注的列（仅 UPDATE OF col）
  std::string condition;      ///< 触发条件 (WHEN 子句)
  std::string body;           ///< 执行体
  std::string language;       ///< 实现语言
  bool enabled;               ///< 启用状态
  std::string owner;          ///< 所有者
  std::chrono::system_clock::time_point created_at;

  Trigger(const std::string& n, const std::string& table, const std::string& evt)
      : name(n), table_name(table), event(evt), timing("AFTER"),
        language("SQL"), enabled(true) {
    created_at = std::chrono::system_clock::now();
  }
};

/**
 * @class StoredProcedureManager
 * @brief 存储过程与触发器管理中心
 */
class StoredProcedureManager {
public:
  /**
   * @brief 运行时统计信息
   */
  struct SPMStats {
    std::atomic<size_t> total_procedures{0};
    std::atomic<size_t> total_triggers{0};
    std::atomic<size_t> procedure_executions{0};
    std::atomic<size_t> trigger_executions{0};
    std::atomic<size_t> failed_executions{0};

    SPMStats() = default;
  };

  /**
   * @brief 构造函数
   */
  StoredProcedureManager(ConfigManager& config_manager, DatabaseManager& database_manager);

  ~StoredProcedureManager();

  // --- 存储过程管理 ---

  /**
   * @brief 注册新的存储过程
   * 
   * 将过程定义保存到内存和持久化存储中。
   * @return true 若成功
   */
  bool CreateProcedure(const StoredProcedure& procedure);

  /**
   * @brief 删除存储过程
   */
  bool DropProcedure(const std::string& schema, const std::string& name);

  /**
   * @brief 查找存储过程定义
   */
  std::shared_ptr<StoredProcedure> GetProcedure(const std::string& schema, const std::string& name);

  /**
   * @brief 执行存储过程
   * 
   * HOW:
   * 1. 查找过程定义。
   * 2. 绑定输入参数。
   * 3. 在当前或新的执行上下文中运行过程体。
   * 4. 收集输出参数和结果集。
   * 
   * @param parameters 实参映射表
   * @param executor 执行上下文的 SQL 执行器
   */
  std::vector<std::vector<std::string>> ExecuteProcedure(
      const std::string& schema, const std::string& name,
      const std::unordered_map<std::string, std::string>& parameters,
      SqlExecutor& executor);

  // --- 触发器管理 ---

  /**
   * @brief 注册新触发器
   */
  bool CreateTrigger(const Trigger& trigger);

  bool DropTrigger(const std::string& schema, const std::string& name);

  /**
   * @brief 动态启用/禁用触发器
   * 
   * 用于在数据导入（Bulk Load）或维护期间临时关闭触发器。
   */
  bool SetTriggerEnabled(const std::string& schema, const std::string& name, bool enabled);

  /**
   * @brief 触发事件处理逻辑
   * 
   * 当 SQL 执行器进行 DML 操作时调用。
   * @param event_type INSERT/UPDATE/DELETE
   * @param old_row 更新前的行数据 (OLD.*)
   * @param new_row 更新后的行数据 (NEW.*)
   */
  bool ExecuteTrigger(const Trigger& trigger, const std::string& event_type,
                     const std::vector<std::string>& old_row,
                     const std::vector<std::string>& new_row,
                     SqlExecutor& executor);

  /**
   * @brief 获取表的相关触发器列表
   * 
   * 用于 DML 执行器在操作前预加载相关触发器。
   */
  std::vector<std::shared_ptr<Trigger>> GetTriggersForTable(
      const std::string& table_name, const std::string& event_type);

  // --- 统计与查询 ---

  SPMStats GetStats() const;
  void ResetStats();

  std::vector<std::shared_ptr<StoredProcedure>> ListProcedures(const std::string& schema = "");
  std::vector<std::shared_ptr<Trigger>> ListTriggers(const std::string& schema = "");

private:
  bool ValidateProcedure(const StoredProcedure& procedure);
  bool ValidateTrigger(const Trigger& trigger);
  std::vector<ProcedureParameter> ParseProcedureParameters(const std::string& param_str);
  bool CompileProcedure(StoredProcedure& procedure);

  ConfigManager& config_manager_;
  DatabaseManager& database_manager_;

  // 元数据缓存
  std::unordered_map<std::string, std::shared_ptr<StoredProcedure>> procedures_;
  std::unordered_map<std::string, std::shared_ptr<Trigger>> triggers_;
  
  // 索引：表名 -> 触发器列表，加速 DML 时的查找
  std::unordered_map<std::string, std::vector<std::shared_ptr<Trigger>>> table_triggers_;

  mutable std::mutex procedures_mutex_;
  mutable std::mutex triggers_mutex_;
  mutable std::mutex stats_mutex_;
  SPMStats stats_;
};

} // namespace sqlcc

