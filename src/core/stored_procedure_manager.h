#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

#include "src/utils/config_manager.h"
#include "sql_parser/ast_nodes.h"

namespace sqlcc {

class DatabaseManager;
class SqlExecutor;

/**
 * @brief 存储过程参数信息
 */
struct ProcedureParameter {
  std::string name;
  std::string type;
  bool is_output;
  std::string default_value;

  ProcedureParameter(const std::string& n, const std::string& t, bool out = false, const std::string& def = "")
      : name(n), type(t), is_output(out), default_value(def) {}
};

/**
 * @brief 存储过程定义
 */
struct StoredProcedure {
  std::string name;
  std::string schema;
  std::vector<ProcedureParameter> parameters;
  std::string body;  // SQL语句体
  std::string language;  // 过程语言 (SQL, PL/SQL等)
  std::string owner;
  std::chrono::system_clock::time_point created_at;
  std::chrono::system_clock::time_point modified_at;

  StoredProcedure(const std::string& n, const std::string& s = "public")
      : name(n), schema(s), language("SQL") {
    created_at = modified_at = std::chrono::system_clock::now();
  }
};

/**
 * @brief 触发器定义
 */
struct Trigger {
  std::string name;
  std::string table_name;
  std::string event;  // INSERT, UPDATE, DELETE
  std::string timing; // BEFORE, AFTER, INSTEAD OF
  std::vector<std::string> columns;  // 对于列级触发器
  std::string condition; // WHEN条件
  std::string body;     // 触发器执行体
  std::string language; // 触发器语言
  bool enabled;
  std::string owner;
  std::chrono::system_clock::time_point created_at;

  Trigger(const std::string& n, const std::string& table, const std::string& evt)
      : name(n), table_name(table), event(evt), timing("AFTER"),
        language("SQL"), enabled(true) {
    created_at = std::chrono::system_clock::now();
  }
};

/**
 * @brief 存储过程和触发器管理器
 *
 * 负责存储过程和触发器的创建、修改、删除和执行
 */
class StoredProcedureManager {
public:
  /**
   * @brief 统计信息
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
   * @param config_manager 配置管理器引用
   * @param database_manager 数据库管理器引用
   */
  StoredProcedureManager(ConfigManager& config_manager, DatabaseManager& database_manager);

  /**
   * @brief 析构函数
   */
  ~StoredProcedureManager();

  /**
   * @brief 创建存储过程
   * @param procedure 存储过程定义
   * @return 是否成功
   */
  bool CreateProcedure(const StoredProcedure& procedure);

  /**
   * @brief 删除存储过程
   * @param schema 模式名
   * @param name 过程名
   * @return 是否成功
   */
  bool DropProcedure(const std::string& schema, const std::string& name);

  /**
   * @brief 获取存储过程定义
   * @param schema 模式名
   * @param name 过程名
   * @return 存储过程定义，如果不存在返回nullptr
   */
  std::shared_ptr<StoredProcedure> GetProcedure(const std::string& schema, const std::string& name);

  /**
   * @brief 执行存储过程
   * @param schema 模式名
   * @param name 过程名
   * @param parameters 参数值
   * @param executor SQL执行器
   * @return 执行结果
   */
  std::vector<std::vector<std::string>> ExecuteProcedure(
      const std::string& schema, const std::string& name,
      const std::unordered_map<std::string, std::string>& parameters,
      SqlExecutor& executor);

  /**
   * @brief 创建触发器
   * @param trigger 触发器定义
   * @return 是否成功
   */
  bool CreateTrigger(const Trigger& trigger);

  /**
   * @brief 删除触发器
   * @param schema 模式名
   * @param name 触发器名
   * @return 是否成功
   */
  bool DropTrigger(const std::string& schema, const std::string& name);

  /**
   * @brief 启用/禁用触发器
   * @param schema 模式名
   * @param name 触发器名
   * @param enabled 是否启用
   * @return 是否成功
   */
  bool SetTriggerEnabled(const std::string& schema, const std::string& name, bool enabled);

  /**
   * @brief 执行触发器
   * @param trigger 触发器定义
   * @param event_type 事件类型
   * @param old_row 旧行数据（用于UPDATE和DELETE）
   * @param new_row 新行数据（用于INSERT和UPDATE）
   * @param executor SQL执行器
   * @return 是否成功
   */
  bool ExecuteTrigger(const Trigger& trigger, const std::string& event_type,
                     const std::vector<std::string>& old_row,
                     const std::vector<std::string>& new_row,
                     SqlExecutor& executor);

  /**
   * @brief 获取指定表的触发器
   * @param table_name 表名
   * @param event_type 事件类型
   * @return 触发器列表
   */
  std::vector<std::shared_ptr<Trigger>> GetTriggersForTable(
      const std::string& table_name, const std::string& event_type);

  /**
   * @brief 获取统计信息
   * @return 统计信息
   */
  SPMStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 列出所有存储过程
   * @param schema 模式名（可选）
   * @return 存储过程列表
   */
  std::vector<std::shared_ptr<StoredProcedure>> ListProcedures(const std::string& schema = "");

  /**
   * @brief 列出所有触发器
   * @param schema 模式名（可选）
   * @return 触发器列表
   */
  std::vector<std::shared_ptr<Trigger>> ListTriggers(const std::string& schema = "");

private:
  /**
   * @brief 验证存储过程定义
   * @param procedure 存储过程定义
   * @return 是否有效
   */
  bool ValidateProcedure(const StoredProcedure& procedure);

  /**
   * @brief 验证触发器定义
   * @param trigger 触发器定义
   * @return 是否有效
   */
  bool ValidateTrigger(const Trigger& trigger);

  /**
   * @brief 解析存储过程参数
   * @param param_str 参数字符串
   * @return 参数列表
   */
  std::vector<ProcedureParameter> ParseProcedureParameters(const std::string& param_str);

  /**
   * @brief 编译存储过程体
   * @param procedure 存储过程定义
   * @return 编译后的执行计划
   */
  bool CompileProcedure(StoredProcedure& procedure);

  // 配置和依赖
  ConfigManager& config_manager_;
  DatabaseManager& database_manager_;

  // 数据存储
  std::unordered_map<std::string, std::shared_ptr<StoredProcedure>> procedures_;
  std::unordered_map<std::string, std::shared_ptr<Trigger>> triggers_;
  std::unordered_map<std::string, std::vector<std::shared_ptr<Trigger>>> table_triggers_; // 表->触发器映射

  // 并发控制
  mutable std::mutex procedures_mutex_;
  mutable std::mutex triggers_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  SPMStats stats_;
};

} // namespace sqlcc
