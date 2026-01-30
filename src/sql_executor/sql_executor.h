#include "../sql_parser/ast/ast_node.h"
#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include "../permission_validator.h"
#include "../system_database.h"
#include "../user_manager.h"
#include "../core_database_manager.h"
#include "../sql_parser/parser.h"
#include "../unified_query_plan.h"
#include "../view_manager.h"
#include <memory>
#include <string>

namespace sqlcc {



/**
 * WHY: 为什么数据库系统需要统一的SQL执行器？
 *
 * 数据库系统演进过程中，SQL执行器的设计经历了从分离到统一的关键转变：
 *
 * 分离执行器的痛点：
 * 1. 架构割裂：DDL/DML/DCL执行器相互独立，缺乏统一规划
 *    - DDL执行器只处理表结构变更
 *    - DML执行器只处理数据操作
 *    - DCL执行器只处理权限管理
 *    - 各组件优化策略不一致，整体性能 suboptimal
 *
 * 2. 优化机会丢失：查询计划割裂导致跨语句优化无法实现
 *    - 无法进行多表关联优化
 *    - 索引选择策略不统一
 *    - 缓存复用率低下
 *    - 执行计划质量参差不齐
 *
 * 3. 错误处理不一致：不同执行器错误处理策略差异巨大
 *    - 错误信息格式不统一
 *    - 异常处理逻辑重复
 *    - 事务回滚策略冲突
 *    - 用户体验极差
 *
 * 4. 维护成本高昂：代码重复和耦合导致系统脆弱
 *    - 相同逻辑在多个执行器中重复
 *    - 修改一处需要同步多处
 *    - 调试困难，问题定位复杂
 *    - 扩展新特性极为困难
 *
 * 统一执行器的革命性优势：
 * - **单一入口原则**：所有SQL语句通过统一接口，简化客户端使用
 * - **全局查询优化**：基于成本的统一查询规划，最大化执行效率
 * - **一致性保证**：统一的错误处理、事务管理和安全控制
 * - **可扩展架构**：插件化设计，轻松添加新SQL特性支持
 * - **性能最优化**：全局资源调度和智能缓存策略
 * - **运维友好**：统一的监控、诊断和性能调优接口
 *
 * 架构设计哲学：
 * 遵循"分层架构 + 流水线处理"的设计理念：
 * - **解析层**：将SQL文本转换为结构化AST语法树
 * - **规划层**：基于统计信息和成本模型生成最优执行计划
 * - **执行层**：调用存储引擎执行具体的数据操作
 * - **返回层**：格式化结果并返回给客户端应用程序
 *
 * 🏗️ 设计模式：统一执行器架构设计
 *
 * 1. 外观模式(Facade Pattern)：统一SQL执行接口
 *    - 隐藏底层复杂性（解析器、优化器、执行引擎、存储引擎）
 *    - 提供简洁一致的API接口，降低客户端使用难度
 *    - 解耦客户端代码和数据库内部实现细节
 *    - 支持透明的内部架构演进和优化
 *
 * 2. 管道模式(Pipeline Pattern)：SQL执行流水线
 *    - 解析 → 验证 → 优化 → 执行 → 返回的线性处理流程
 *    - 每个处理阶段职责单一，功能内聚，易于测试和维护
 *    - 支持插件化扩展，可以灵活插入自定义处理逻辑
 *    - 便于性能监控和瓶颈分析，每个阶段都可以独立测量
 *
 * 3. 策略模式(Strategy Pattern)：查询优化策略
 *    - 可插拔的查询优化规则，支持不同的优化算法
 *    - 运行时动态选择最优的执行策略和优化方案
 *    - 支持基于代价的优化决策和启发式优化
 *    - 允许针对特定查询类型定制专门的优化策略
 *
 * 4. 工厂模式(Factory Pattern)：执行器组件创建
 *    - 集中管理各种执行器组件的创建和生命周期
 *    - 支持依赖注入和组件配置的灵活定制
 *    - 便于单元测试和组件替换
 *    - 提供统一的组件初始化和清理接口
 *
 * SOLID原则在统一执行器中的体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - SqlExecutor只负责SQL执行的统一协调和调度
 *    - Parser专注于语法解析和AST构建
 *    - QueryPlanner负责查询优化和计划生成
 *    - ExecutionEngine专注执行计划的实际运行
 *    - 每个组件职责清晰，功能单一，易于理解和维护
 *
 * 2. 开闭原则(OCP)：
 *    - 新增SQL语句类型无需修改现有执行器代码
 *    - 新的优化策略可以通过扩展轻松集成
 *    - 执行引擎支持新的存储引擎后端扩展
 *    - 对扩展开放，对修改关闭，保证系统稳定性
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何查询优化器实现都可以替代默认优化器
 *    - 不同的存储引擎实现可以无缝替换
 *    - 各种执行器组件遵循统一的接口契约
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 客户端只依赖执行器提供的核心接口
 *    - 复杂的内部组件接口不暴露给外部使用
 *    - 按需提供接口，避免接口污染和过度耦合
 *    - 不同类型的客户端可以使用不同的接口子集
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 执行器依赖抽象的存储和事务接口
 *    - 不依赖具体的存储引擎实现细节
 *    - 通过依赖注入提高系统的可测试性和灵活性
 *    - 高层模块不依赖低层模块的具体实现
 *
 * WHAT: SqlExecutor - SQLCC统一SQL执行器
 *
 * 企业级数据库系统的核心执行引擎，整合DDL/DML/DCL执行器的公共逻辑，
 * 提供统一的SQL语句解析、优化和执行服务。
 *
 * 核心功能特性：
 * - **SQL语句全生命周期管理**：从文本解析到结果返回的完整处理流程
 * - **权限验证和安全控制**：基于角色的访问控制和安全策略执行
 * - **事务管理和并发控制**：ACID事务保证和多版本并发控制
 * - **错误处理和恢复机制**：统一的异常处理和故障恢复策略
 * - **系统数据库管理**：内置系统表和元数据的管理功能
 * - **视图管理和虚拟化**：支持复杂视图定义和查询重写
 * - **性能监控和调优**：详细的执行统计和性能指标收集
 *
 * 接口设计原则：
 * - **单一入口**：Execute()方法作为所有SQL执行的统一入口点
 * - **多格式支持**：支持SQL字符串和预解析AST的双重输入方式
 * - **批量处理**：ExecuteFile()支持文件级SQL脚本批量执行
 * - **状态查询**：GetLastError()和GetExecutionStats()提供执行状态反馈
 * - **资源管理**：构造函数和析构函数保证资源的正确初始化和清理
 *
 * HOW: SQL执行流水线的技术实现机制
 *
 * 1. 权限验证阶段：确保用户具有执行语句的必要权限
 *    - 用户身份认证和会话验证
 *    - 基于角色的权限检查和访问控制
 *    - 资源配额和限制的验证
 *    - 安全策略和审计日志的记录
 *
 * 2. 语法解析阶段：将SQL文本转换为结构化的AST表示
 *    - 词法分析：将SQL文本分解为token流
 *    - 语法分析：根据SQL语法规则构建AST树
 *    - 语义验证：检查语法正确性和语义合理性
 *    - 预处理优化：常量折叠和简单表达式化简
 *
 * 3. 查询规划阶段：基于成本模型生成最优执行计划
 *    - 统计信息收集：表的规模、索引选择性等统计数据
 *    - 代价估算：CPU代价、I/O代价、内存使用等综合评估
 *    - 计划枚举：生成多种可能的执行计划候选方案
 *    - 计划选择：基于代价模型选择最优执行计划
 *    - 计划优化：应用启发式规则进行进一步优化
 *
 * 4. 计划执行阶段：调用存储引擎执行具体的数据库操作
 *    - 执行上下文创建：准备执行所需的运行时环境
 *    - 算子执行：按照执行计划顺序调用各个算子
 *    - 数据流管理：处理算子间的中间结果传递
 *    - 资源管理：控制内存使用和临时空间分配
 *    - 并发控制：协调多线程执行和事务隔离
 *
 * 5. 结果返回阶段：格式化执行结果并返回给客户端
 *    - 结果集构造：将执行结果组织成标准格式
 *    - 数据类型转换：确保返回数据的类型正确性
 *    - 结果分页：支持大数据集的分页返回
 *    - 客户端适配：根据客户端需求调整结果格式
 *
 * 6. 清理和维护阶段：释放资源并记录执行统计信息
 *    - 资源释放：清理临时数据结构和缓存
 *    - 统计信息更新：记录执行时间、资源使用等指标
 *    - 日志记录：写入执行日志用于审计和调试
 *    - 状态重置：为下次执行做好准备
 *
 * 错误处理和异常管理机制：
 * - **异常捕获策略**：使用try-catch块捕获运行时异常
 * - **错误传播机制**：错误信息通过调用栈向上层传播
 * - **状态管理**：维护详细的错误状态信息和上下文
 * - **日志记录**：所有错误和异常写入系统日志用于诊断
 * - **恢复策略**：根据错误类型选择适当的恢复和重试机制
 * - **用户友好**：提供清晰的错误信息和解决建议
 *
 * 性能优化和调优策略：
 * - **连接池管理**：复用数据库连接减少建立开销
 * - **缓存机制**：查询结果、执行计划、元数据的智能缓存
 * - **批量处理**：支持多语句批量执行减少网络往返
 * - **异步执行**：非阻塞I/O操作提高并发处理能力
 * - **预编译优化**：Prepared Statement减少重复解析开销
 * - **并行执行**：多核CPU的并行查询处理优化
 * - **自适应调整**：根据负载情况动态调整资源分配
 *
 * 扩展性和可维护性设计：
 * - **插件架构**：支持自定义函数、类型、优化器的扩展
 * - **配置驱动**：通过配置文件调整执行器行为参数
 * - **模块化设计**：清晰的组件边界和依赖关系管理
 * - **测试友好**：支持单元测试和集成测试的接口设计
 * - **监控集成**：与系统监控框架的深度集成
 * - **向后兼容**：保证API的稳定性和兼容性
 *
 * 线程安全和并发控制保证：
 * - **互斥锁保护**：使用互斥锁保护共享状态的访问
 * - **原子操作**：无锁的数据结构和原子变量的使用
 * - **线程局部存储**：隔离线程特定的状态和缓存
 * - **事务隔离**：基于MVCC的多版本并发控制机制
 * - **死锁避免**：资源获取的有序化策略和超时机制
 * - **可扩展性**：支持水平扩展和负载均衡的架构设计
 */
class SqlExecutor {
public:
  SqlExecutor();
  // 新增：接受DatabaseManager的构造函数，用于共享数据库实例
  SqlExecutor(std::shared_ptr<DatabaseManager> db_manager);
  ~SqlExecutor();

  /**
   * WHAT: Execute - SQL语句执行主入口
   *
   * 处理完整的SQL语句执行流程，从解析到结果返回。
   * 支持DDL、DML、DCL等多种SQL语句类型。
   *
   * HOW: SQL执行流水线
   * 1. 权限验证：检查用户执行权限
   * 2. 语法解析：将SQL转换为AST
   * 3. 查询规划：生成最优执行计划
   * 4. 计划执行：实际执行查询操作
   * 5. 结果返回：格式化并返回执行结果
   *
   * 执行流程：
   * - 单条语句：直接解析执行
   * - 多条语句：分批处理，保持事务一致性
   * - 错误处理：捕获异常，返回错误信息
   *
   * @param sql SQL语句字符串
   * @return 执行结果消息
   */
  std::string Execute(const std::string &sql);

  /**
   * WHAT: Execute - AST驱动SQL执行入口
   *
   * 直接接受解析后的AST节点，生成查询计划并执行。
   * 这是优化的执行路径，避免重复解析开销。
   *
   * HOW: AST驱动执行流程
   * 1. 验证AST节点有效性
   * 2. 创建执行上下文
   * 3. 生成查询计划
   * 4. 执行查询计划
   * 5. 返回执行结果
   *
   * @param stmt 解析后的AST语句节点
   * @return 执行结果消息
   */
  std::string Execute(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 执行文件中的SQL语句
   * @param file_path 文件路径
   * @return 执行结果消息
   */
  std::string ExecuteFile(const std::string &file_path);

  /**
   * @brief 验证语句的有效性
   * @param stmt 要验证的AST语句节点
   * @return 验证结果
   */
  bool validateStatement(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 检查语句是否需要事务支持
   * @param stmt 要检查的AST语句节点
   * @return 是否需要事务
   */
  bool requiresTransaction(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 与存储引擎集成的执行方法
   * @param stmt AST语句节点
   * @param context 执行上下文
   * @param pages_accessed 页面访问计数
   * @return 执行结果
   */
  ExecutionResult executeWithStorageEngine(
      const sqlcc::sql_parser::Statement* stmt,
      ExecutionContext& context,
      size_t& pages_accessed);

  /**
   * @brief 获取最后一次执行的错误信息
   * @return 错误信息
   */
  std::string GetLastError() const;

  /**
   * @brief 获取执行统计信息
   * @return 统计信息字符串
   */
  std::string GetExecutionStats() const;

private:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::unique_ptr<ViewManager> view_manager_;
  std::unique_ptr<PermissionValidator> permission_validator_;
  std::shared_ptr<StorageEngine> storage_engine_;
  std::shared_ptr<TransactionManager> transaction_manager_;
  std::string last_error_;
  std::string execution_stats_;
  std::string current_user_;
  std::string current_database_;

  // 组件初始化
  void initializeComponents();

  /**
   * @brief 设置错误信息
   * @param error 错误信息
   */
  void SetError(const std::string &error);

  /**
   * @brief 清除错误信息
   */
  void ClearError();

  /**
   * @brief 初始化系统数据库
   */
  bool InitializeSystemDatabase();

  /**
   * @brief 解析SQL语句
   * @param sql SQL语句
   * @return 解析后的语句对象
   */
  std::unique_ptr<sql_parser::Statement> ParseSQL(const std::string &sql);

  /**
   * @brief 创建统一查询计划
   * @param stmt 解析后的语句
   * @return 查询计划对象
   */
  std::unique_ptr<UnifiedQueryPlan>
  CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt);

  /**
   * @brief 初始化权限验证器
   */
  bool InitializePermissionValidator();

  /**
   * @brief 更新当前数据库
   * @param sql SQL语句
   */
  void UpdateCurrentDatabase(const std::string &sql);

  /**
   * @brief 去除字符串两端的空白字符
   * @param str 要处理的字符串
   */
  void TrimString(std::string &str);

  /**
   * @brief 执行语句的主要逻辑
   * @param sql SQL语句
   * @return 执行结果
   */
  std::string ExecuteStatement(const std::string& sql);

  // DDL语句处理方法
  std::string ExecuteCreateTable(const std::string& sql);
  std::string ExecuteCreateDatabase(const std::string& sql);
  std::string ExecuteDropTable(const std::string& sql);
  std::string ExecuteDropDatabase(const std::string& sql);
  std::string ExecuteAlterTable(const std::string& sql);
  std::string ExecuteCreateIndex(const std::string& sql);
  std::string ExecuteDropIndex(const std::string& sql);

  // DML语句处理方法
  std::string ExecuteInsert(const std::string& sql);
  std::string ExecuteUpdate(const std::string& sql);
  std::string ExecuteDelete(const std::string& sql);

  // DQL语句处理方法
  std::string ExecuteSelect(const std::string& sql);

  // DCL语句处理方法
  std::string ExecuteGrant(const std::string& sql);
  std::string ExecuteRevoke(const std::string& sql);
  std::string ExecuteCreateUser(const std::string& sql);
  std::string ExecuteDropUser(const std::string& sql);
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H
