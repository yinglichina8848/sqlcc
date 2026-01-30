#include "sql_parser/ast/ast_node.h"
#ifndef SQLCC_UNIFIED_QUERY_PLAN_H
#define SQLCC_UNIFIED_QUERY_PLAN_H

#include "sql_parser/ast/ast_nodes.h"
#include "core_backup_20260121_001034/user_manager.h"
#include "core_backup_20260121_001034/system_database.h"
#include "core_backup_20260121_001034/core_database_manager.h"
#include "error_handler.h"
#include "execution_engine.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * WHY: 为什么数据库系统需要统一查询计划系统？
 *
 * 数据库系统演进过程中，SQL执行器的设计从分离走向统一的关键转变：
 *
 * 分离执行器的架构痛点：
 * 1. **架构割裂导致优化失效**：
 *    - DDL执行器只处理表结构变更，无法进行跨语句优化
 *    - DML执行器只处理数据操作，索引选择策略不统一
 *    - DCL执行器只处理权限管理，安全策略无法全局协调
 *    - 各组件优化器独立工作，整体性能 suboptimal
 *
 * 2. **查询优化机会全面丢失**：
 *    - 无法进行多表关联的全局优化
 *    - 索引选择策略割裂，查询效率低下
 *    - 缓存复用率极低，重复计算严重
 *    - 执行计划质量参差不齐，用户体验差
 *
 * 3. **代码维护成本指数级增长**：
 *    - 相同逻辑在多个执行器中重复实现
 *    - 修改一处公共逻辑需要同步修改所有执行器
 *    - 调试困难，问题定位耗时费力
 *    - 新功能扩展需要全方位修改
 *
 * 4. **错误处理和用户体验极差**：
 *    - 不同执行器的错误处理策略完全不同
 *    - 错误信息格式不统一，难以理解
 *    - 异常处理逻辑重复，容易遗漏
 *    - 事务回滚策略冲突，数据一致性难以保证
 *
 * 统一查询计划系统的革命性优势：
 * - **单一优化引擎**：所有SQL语句共享全局查询优化器，最大化执行效率
 * - **统一执行模型**：标准化的SQL执行流程，消除执行器间的差异
 * - **全局资源调度**：跨语句的资源分配和缓存复用优化
 * - **一致性保证**：统一的权限检查、事务管理和错误处理
 * - **可扩展架构**：插件化设计，轻松集成新SQL特性
 * - **运维友好**：统一的监控、诊断和性能调优接口
 * - **代码质量提升**：消除重复代码，提高维护效率
 *
 * 架构设计哲学：
 * 遵循"分层架构 + 模板方法"的设计理念：
 * - **规划层**：基于统计信息和成本模型生成最优执行计划
 * - **验证层**：统一的语法语义验证和权限检查
 * - **执行层**：标准化的查询执行流程和结果处理
 * - **管理层**：统一的资源管理、监控和生命周期控制
 *
 * 🏗️ 设计模式：统一查询计划架构设计
 *
 * 1. **模板方法模式(Template Method Pattern)**：统一查询计划基类
 *    - 定义SQL执行的完整流程骨架（构建→验证→执行→清理）
 *    - 子类通过重写钩子方法实现特定SQL类型的逻辑
 *    - 保证所有查询计划遵循相同的执行流程和规范
 *    - 流程控制集中管理，子类专注业务逻辑实现
 *
 * 2. **工厂模式(Factory Pattern)**：查询计划对象工厂
 *    - 根据SQL语句类型自动创建对应的查询计划对象
 *    - 封装复杂的对象创建逻辑和依赖关系管理
 *    - 支持运行时动态注册新的查询计划类型
 *    - 提供统一的计划创建接口，简化客户端使用
 *
 * 3. **策略模式(Strategy Pattern)**：可插拔执行策略
 *    - 验证策略：不同SQL类型的语法语义验证策略
 *    - 优化策略：基于代价的查询优化算法选择
 *    - 执行策略：存储引擎访问和数据处理的策略
 *    - 缓存策略：查询结果和执行计划的缓存策略
 *
 * 4. **状态机模式(State Machine Pattern)**：查询执行状态管理
 *    - 明确的执行状态转换：PENDING→VALIDATING→EXECUTING→COMPLETED
 *    - 状态驱动的执行流程控制和错误处理
 *    - 状态变化的事件通知机制
 *    - 状态一致性的并发安全保证
 *
 * 5. **组合模式(Composite Pattern)**：查询计划的层次结构
 *    - 简单计划：单个SQL语句的执行计划
 *    - 复合计划：多语句批量执行的组合计划
 *    - 递归计划：嵌套查询和子查询的层次结构
 *    - 统一接口：所有计划类型遵循相同的执行接口
 *
 * SOLID原则在统一查询计划中的体现：
 *
 * 1. **单一职责原则(SRP)**：职责分离清晰明确
 *    - UnifiedQueryPlan：负责公共执行流程和资源管理
 *    - DDLQueryPlan：专门处理DDL语句的解析和执行
 *    - DMLQueryPlan：专注于DML语句的优化和数据操作
 *    - DCLQueryPlan：负责权限和安全策略的执行
 *    - 每个组件职责单一，功能内聚，易于理解和维护
 *
 * 2. **开闭原则(OCP)**：对扩展开放，对修改关闭
 *    - 新增SQL语句类型无需修改现有查询计划代码
 *    - 通过继承UnifiedQueryPlan轻松扩展新功能
 *    - 执行策略和优化算法可以独立扩展和替换
 *    - 插件化架构保证系统核心逻辑的稳定性
 *
 * 3. **里氏替换原则(LSP)**：子类可以完全替代父类
 *    - 任何具体查询计划都可以替代UnifiedQueryPlan使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 客户端代码无需关心具体计划类型的差异
 *    - 多态性保证了系统的灵活性和可扩展性
 *
 * 4. **接口隔离原则(ISP)**：按需提供接口，避免过度依赖
 *    - 客户端只依赖执行计划的核心执行接口
 *    - 复杂的内部管理和配置接口不暴露给外部
 *    - 不同类型的客户端可以使用不同的接口子集
 *    - 最小化接口依赖，降低耦合度
 *
 * 5. **依赖倒置原则(DIP)**：高层不依赖低层具体实现
 *    - 查询计划依赖抽象的存储和事务接口
 *    - 不依赖具体的存储引擎或数据库管理器实现
 *    - 通过依赖注入提高系统的可测试性和灵活性
 *    - 抽象层隔离了变化，提高了系统的稳定性
 *
 * WHAT: UnifiedQueryPlan - SQLCC统一查询计划系统
 *
 * 企业级数据库系统的核心执行引擎架构，整合DDL/DML/DCL执行器的公共逻辑，
 * 提供标准化的SQL语句解析、优化、执行和结果处理服务。
 *
 * 核心功能特性：
 * - **统一执行流程**：所有SQL语句遵循标准化的执行生命周期
 * - **全局查询优化**：基于成本模型的跨语句查询优化
 * - **权限和安全控制**：统一的身份验证和访问控制机制
 * - **事务管理集成**：与事务系统的深度集成和协调
 * - **错误处理统一**：一致的异常捕获、处理和报告机制
 * - **性能监控统计**：详细的执行指标收集和性能分析
 * - **资源管理优化**：智能的内存、连接和缓存资源调度
 * - **并发控制保证**：多线程环境下的线程安全和数据一致性
 *
 * 系统组件架构：
 * - **UnifiedQueryPlan**：抽象基类，定义标准执行流程接口
 * - **DDLQueryPlan**：DDL语句专用执行计划（CREATE、DROP、ALTER）
 * - **DMLQueryPlan**：DML语句专用执行计划（SELECT、INSERT、UPDATE、DELETE）
 * - **DCLQueryPlan**：DCL语句专用执行计划（GRANT、REVOKE、CREATE USER）
 * - **UtilityQueryPlan**：工具语句执行计划（USE、SHOW、DESCRIBE）
 * - **ProcedureQueryPlan**：存储过程执行计划（CALL、CREATE PROCEDURE）
 * - **TriggerQueryPlan**：触发器执行计划（CREATE TRIGGER、DROP TRIGGER）
 * - **QueryPlanFactory**：智能工厂，根据SQL类型创建对应执行计划
 *
 * 接口设计原则：
 * - **构建接口**：buildPlan() - 根据AST构建具体的执行计划
 * - **执行接口**：executePlan() - 执行构建好的查询计划
 * - **状态查询**：getStatus() - 获取当前执行状态
 * - **错误处理**：getErrorMessage() - 获取详细错误信息
 * - **统计监控**：getExecutionStats() - 获取执行性能统计
 * - **生命周期管理**：构造函数和析构函数保证资源正确管理
 *
 * HOW: 统一查询计划执行机制的技术实现
 *
 * 1. **计划构建阶段**：将SQL AST转换为可执行的查询计划
 *    - 语句类型识别：通过AST节点类型确定具体计划类型
 *    - 上下文初始化：创建执行所需的运行时环境和上下文
 *    - 依赖关系解析：解析表名、列名、函数等对象引用
 *    - 权限预检查：验证用户是否有执行该语句的基本权限
 *    - 计划步骤组装：按照执行顺序组织具体的操作步骤
 *
 * 2. **验证阶段**：全面检查执行条件和数据完整性
 *    - 语法验证：AST结构正确性和完整性检查
 *    - 语义验证：对象存在性、类型匹配、约束检查
 *    - 权限验证：基于角色的细粒度访问控制
 *    - 资源验证：检查系统资源是否满足执行要求
 *    - 业务规则验证：应用特定的业务逻辑约束
 *
 * 3. **权限检查阶段**：多层次的安全控制和审计
 *    - 用户身份认证：验证用户凭据和会话有效性
 *    - 对象级权限检查：检查对特定表的操作权限
 *    - 列级权限控制：敏感数据的细粒度访问控制
 *    - 行级安全策略：基于内容的动态权限过滤
 *    - 操作审计记录：所有权限检查操作的详细日志
 *
 * 4. **预处理阶段**：准备执行环境和优化数据结构
 *    - 查询重写：应用视图展开、子查询优化等重写规则
 *    - 统计信息收集：获取表的规模、索引选择性等优化依据
 *    - 执行计划生成：基于代价模型选择最优的访问路径
 *    - 资源预分配：预先分配执行所需的内存和临时空间
 *    - 并发控制设置：配置事务隔离级别和锁策略
 *
 * 5. **执行阶段**：实际执行查询计划并处理结果
 *    - 算子执行：按照执行计划顺序调用各个物理算子
 *    - 数据流管理：处理算子间的中间结果传递和缓存
 *    - 错误处理：执行过程中的异常捕获和恢复策略
 *    - 性能监控：实时收集执行时间、I/O操作等指标
 *    - 资源监控：跟踪内存使用、连接状态等资源消耗
 *
 * 6. **后处理阶段**：清理资源并记录执行统计信息
 *    - 结果格式化：将执行结果转换为客户端期望的格式
 *    - 元数据更新：更新系统表、统计信息和缓存
 *    - 事务提交：确保所有更改都持久化到磁盘
 *    - 操作日志记录：写入审计日志和执行历史
 *    - 资源释放：清理临时对象和释放系统资源
 *
 * 错误处理和异常管理机制：
 * - **分层异常处理**：不同抽象层次的异常处理策略
 * - **异常分类处理**：语法错误、权限错误、资源错误等分类处理
 * - **错误信息标准化**：统一的错误代码和消息格式
 * - **错误传播控制**：异常信息的向上层传递和转换
 * - **恢复策略选择**：根据错误类型选择适当的恢复机制
 * - **日志记录完整**：所有异常的详细上下文信息记录
 *
 * 性能优化和调优策略：
 * - **计划缓存复用**：相同SQL语句的执行计划缓存
 * - **参数化查询优化**：Prepared Statement减少重复解析
 * - **连接池管理**：数据库连接的复用减少建立开销
 * - **批量操作优化**：多语句批量执行减少网络往返
 * - **并行执行支持**：多核CPU的查询并行化处理
 * - **自适应优化**：根据负载情况动态调整执行策略
 * - **内存管理优化**：智能的内存分配和垃圾回收
 *
 * 扩展性和可维护性设计：
 * - **插件化架构**：支持自定义函数、类型、优化器的扩展
 * - **配置驱动设计**：通过配置文件调整执行行为参数
 * - **模块化组织**：清晰的组件边界和依赖关系管理
 * - **测试友好性**：支持单元测试和集成测试的架构设计
 * - **监控集成度**：与系统监控框架的深度集成
 * - **文档完备性**：详细的设计文档和使用指南
 *
 * 并发控制和线程安全保证：
 * - **线程安全设计**：多线程环境下的数据结构安全访问
 * - **细粒度锁管理**：最小化锁竞争范围的锁策略
 * - **死锁检测机制**：自动检测和处理死锁情况
 * - **事务隔离控制**：基于MVCC的多版本并发控制
 * - **原子操作保证**：关键状态更新的原子性操作
 * - **可扩展性支持**：水平扩展和负载均衡的架构设计
 *
 * 查询计划的生命周期管理：
 * 1. **创建阶段**：QueryPlanFactory根据SQL类型创建具体计划实例
 * 2. **初始化阶段**：注入必要的依赖关系和配置参数
 * 3. **构建阶段**：根据AST构建具体的执行步骤和数据结构
 * 4. **验证阶段**：检查所有执行条件和约束是否满足
 * 5. **执行阶段**：实际执行查询计划并产生结果
 * 6. **清理阶段**：释放资源并更新统计信息
 * 7. **销毁阶段**：对象生命周期结束，资源完全释放
 *
 * 监控和诊断支持：
 * - **执行时间统计**：各阶段的耗时详细分析
 * - **资源使用监控**：CPU、内存、I/O的消耗跟踪
 * - **缓存命中率分析**：计划缓存和数据缓存的效率评估
 * - **错误模式识别**：常见错误模式的自动识别和报告
 * - **性能趋势分析**：执行性能的历史趋势和预测
 * - **调试信息记录**：详细的执行轨迹和中间状态记录
 */

/**
 * @brief 统一查询计划状态
 */
enum class QueryPlanStatus {
    PENDING,      // 待执行
    VALIDATING,   // 验证中
    EXECUTING,    // 执行中
    COMPLETED,    // 已完成
    FAILED        // 失败
};

/**
 * @brief 统一查询计划步骤类型
 */
enum class QueryStepType {
    VALIDATION,    // 验证步骤
    PERMISSION,    // 权限检查
    PRE_PROCESS,   // 预处理
    EXECUTION,     // 执行
    POST_PROCESS,  // 后处理
    CLEANUP        // 清理
};

/**
 * @brief 查询计划步骤
 */
struct QueryStep {
    QueryStepType type;
    std::string description;
    std::function<bool()> action;
    bool required; // 是否为必需步骤
    
    QueryStep(QueryStepType t, const std::string& desc, std::function<bool()> act, bool req = true)
        : type(t), description(desc), action(act), required(req) {}
};

/**
 * @brief 统一查询计划类
 * 
 * 整合DDL/DML/DCL执行器的公共逻辑，提供统一的执行流程
 */
class UnifiedQueryPlan {
public:
    UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);
    
    ~UnifiedQueryPlan() = default;
    
    /**
     * @brief 构建查询计划
     */
    bool buildPlan(std::unique_ptr<sql_parser::Statement> stmt);
    
    /**
     * @brief 执行查询计划
     */
    ExecutionResult executePlan();
    
    /**
     * @brief 获取计划状态
     */
    QueryPlanStatus getStatus() const { return status_; }
    
    /**
     * @brief 获取错误信息
     */
    const std::string& getErrorMessage() const { return error_message_; }
    
    /**
     * @brief 获取执行统计信息
     */
    const std::string& getExecutionStats() const { return execution_stats_; }

private:
    // 公共验证方法
    bool validateStatement();
    bool validateDatabaseContext();
    bool validateTableExistence(const std::string& table_name);
    bool validateColumnExistence(const std::string& table_name, const std::string& column_name);
    
    // 权限检查方法
    bool checkPermission(const std::string& operation, const std::string& resource);
    bool checkDatabasePermission(const std::string& operation);
    bool checkTablePermission(const std::string& operation, const std::string& table_name);
    
    // 公共预处理方法
    bool preProcessStatement();
    bool resolveObjectReferences();
    bool prepareExecutionContext();
    
    // 公共后处理方法
    bool postProcessStatement();
    bool updateSystemMetadata();
    bool logOperation();
    
protected:
    // 错误处理
    void setError(const std::string& error);
    void clearError();
    
    // 执行器特定方法（由子类实现）
    virtual bool buildSpecificPlan() = 0;
    virtual ExecutionResult executeSpecificPlan() = 0;
    
protected:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::unique_ptr<sql_parser::Statement> statement_;
    
    std::vector<QueryStep> steps_;
    QueryPlanStatus status_;
    std::string error_message_;
    std::string execution_stats_;
    
    // 执行上下文
    std::string current_database_;
    std::string current_user_;
    std::string operation_type_;
    std::string target_object_;
};

/**
 * @brief DDL查询计划
 */
class DDLQueryPlan : public UnifiedQueryPlan {
public:
    DDLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                 std::shared_ptr<UserManager> user_manager,
                 std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // DDL特定方法
    bool buildCreatePlan();
    bool buildDropPlan();
    bool buildAlterPlan();
    
    ExecutionResult executeCreatePlan();
    ExecutionResult executeDropPlan();
    ExecutionResult executeAlterPlan();
};

/**
 * @brief DML查询计划
 */
class DMLQueryPlan : public UnifiedQueryPlan {
public:
    DMLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                 std::shared_ptr<UserManager> user_manager,
                 std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // DML特定方法
    bool buildSelectPlan();
    bool buildInsertPlan();
    bool buildUpdatePlan();
    bool buildDeletePlan();
    
    ExecutionResult executeSelectPlan();
    ExecutionResult executeInsertPlan();
    ExecutionResult executeUpdatePlan();
    ExecutionResult executeDeletePlan();
    
    // DML特定上下文
    std::string table_name_;
    std::vector<std::string> affected_columns_;
    std::vector<std::vector<std::string>> values_;
    std::shared_ptr<sql_parser::WhereClause> where_clause_;
};

/**
 * @brief DCL查询计划
 */
class DCLQueryPlan : public UnifiedQueryPlan {
public:
    DCLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                 std::shared_ptr<UserManager> user_manager,
                 std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // DCL特定方法
    bool buildCreateUserPlan();
    bool buildDropUserPlan();
    bool buildGrantPlan();
    bool buildRevokePlan();
    
    ExecutionResult executeCreateUserPlan();
    ExecutionResult executeDropUserPlan();
    ExecutionResult executeGrantPlan();
    ExecutionResult executeRevokePlan();
    
    // DCL特定上下文
    std::string grantee_;
    std::string grantor_;
    std::vector<std::string> privileges_;
    std::string object_type_;
    std::string object_name_;
};

/**
 * @brief 工具查询计划
 */
class UtilityQueryPlan : public UnifiedQueryPlan {
public:
    UtilityQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // 工具特定方法
    bool buildUsePlan();
    bool buildShowPlan();
    
    ExecutionResult executeUsePlan();
    ExecutionResult executeShowPlan();
    
    // USE语句特定上下文
    std::string target_database_;
};

/**
 * @brief 存储过程查询计划
 */
class ProcedureQueryPlan : public UnifiedQueryPlan {
public:
    ProcedureQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                      std::shared_ptr<UserManager> user_manager,
                      std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // 存储过程特定方法
    bool buildCreateProcedurePlan();
    bool buildCallProcedurePlan();
    bool buildDropProcedurePlan();
    
    ExecutionResult executeCreateProcedurePlan();
    ExecutionResult executeCallProcedurePlan();
    ExecutionResult executeDropProcedurePlan();
    
    // 存储过程特定上下文
    std::string procedure_name_;
    std::vector<sql_parser::ProcedureParameter> parameters_;
    std::string procedure_body_;
    std::vector<std::unique_ptr<sql_parser::ast::Expression>> arguments_;
};

/**
 * @brief 触发器查询计划
 */
class TriggerQueryPlan : public UnifiedQueryPlan {
public:
    TriggerQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);
    
protected:
    bool buildSpecificPlan() override;
    ExecutionResult executeSpecificPlan() override;
    
private:
    // 触发器特定方法
    bool buildCreateTriggerPlan();
    bool buildDropTriggerPlan();
    bool buildAlterTriggerPlan();
    
    ExecutionResult executeCreateTriggerPlan();
    ExecutionResult executeDropTriggerPlan();
    ExecutionResult executeAlterTriggerPlan();
    
    // 触发器特定上下文
    std::string trigger_name_;
    sql_parser::TriggerDefinition trigger_def_;
    sql_parser::AlterTriggerStatement::Action alter_action_;
};

/**
 * @brief 查询计划工厂
 */
class QueryPlanFactory {
public:
    static std::unique_ptr<UnifiedQueryPlan> createPlan(
        std::unique_ptr<sql_parser::Statement> stmt,
        std::shared_ptr<DatabaseManager> db_manager,
        std::shared_ptr<UserManager> user_manager,
        std::shared_ptr<SystemDatabase> system_db);
};

} // namespace sqlcc

#endif // SQLCC_UNIFIED_QUERY_PLAN_H
