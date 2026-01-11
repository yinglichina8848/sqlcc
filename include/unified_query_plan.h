#include "sql_parser/ast_node.h"
#ifndef SQLCC_UNIFIED_QUERY_PLAN_H
#define SQLCC_UNIFIED_QUERY_PLAN_H

#include "sql_parser/ast_nodes.h"
#include "core/user_manager.h"
#include "core/system_database.h"
#include "core/core_database_manager.h"
#include "error_handler.h"
#include "execution_engine.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * WHY: 为什么需要统一查询计划系统？
 *
 * 传统数据库系统中DDL/DML/DCL执行器分离导致：
 * - 查询优化机会丢失：不同执行器无法共享优化策略
 * - 代码重复严重：权限检查、验证逻辑在各执行器中重复
 * - 错误处理不一致：不同执行器的错误处理方式各异
 * - 维护成本高：修改公共逻辑需要同时修改多个执行器
 * - 可扩展性差：添加新功能需要修改所有执行器
 *
 * 统一查询计划系统的优势：
 * 1. 统一优化：所有SQL语句共享查询优化引擎
 * 2. 代码复用：公共逻辑提取到基类中
 * 3. 一致性保证：统一的错误处理和权限检查
 * 4. 可维护性：修改公共逻辑只需在一处进行
 * 5. 可扩展性：新功能通过继承轻松扩展
 *
 * 架构设计：
 * - 基类：定义公共执行流程和接口
 * - 派生类：实现特定SQL类型的执行逻辑
 * - 工厂模式：根据语句类型创建对应的计划对象
 * - 策略模式：可插拔的优化和执行策略
 *
 * 🏗️ 设计模式：统一查询计划架构设计
 *
 * 设计模式应用：
 * 1. 模板方法模式(Template Method Pattern)：统一查询计划基类
 *    - 定义执行流程的骨架
 *    - 子类实现特定步骤
 *    - 保证执行流程的一致性
 *
 * 2. 工厂模式(Factory Pattern)：查询计划工厂
 *    - 根据SQL语句类型创建对应计划
 *    - 封装对象创建逻辑
 *    - 支持扩展新的计划类型
 *
 * 3. 策略模式(Strategy Pattern)：可插拔执行策略
 *    - 验证策略：不同SQL的验证逻辑
 *    - 优化策略：不同的查询优化算法
 *    - 执行策略：不同的执行方式
 *
 * 4. 状态机模式(State Machine Pattern)：查询执行状态管理
 *    - 明确的执行状态转换
 *    - 状态驱动的执行流程
 *    - 错误状态的统一处理
 *
 * SOLID原则体现：
 * - 单一职责：每个计划类负责一种SQL类型
 * - 开闭原则：新计划类型通过继承实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: 统一查询计划系统 - 整合DDL/DML/DCL执行器公共逻辑
 *
 * 核心功能：
 * - 统一SQL执行流程：所有SQL语句遵循相同执行模式
 * - 公共逻辑提取：权限检查、验证、预处理等公共功能
 * - 错误处理统一：一致的错误捕获和报告机制
 * - 状态管理：执行状态跟踪和生命周期管理
 * - 扩展框架：支持新SQL类型的轻松集成
 *
 * 系统组件：
 * - UnifiedQueryPlan：基类，定义公共执行流程
 * - DDLQueryPlan：DDL语句执行计划
 * - DMLQueryPlan：DML语句执行计划
 * - DCLQueryPlan：DCL语句执行计划
 * - UtilityQueryPlan：工具语句执行计划
 * - ProcedureQueryPlan：存储过程执行计划
 * - TriggerQueryPlan：触发器执行计划
 * - QueryPlanFactory：计划对象工厂
 *
 * 接口设计：
 * - buildPlan(): 构建查询执行计划
 * - executePlan(): 执行查询计划
 * - getStatus(): 获取执行状态
 * - getErrorMessage(): 获取错误信息
 * - getExecutionStats(): 获取执行统计
 *
 * HOW: 统一查询计划执行机制
 *
 * 执行流程：
 * 1. 计划构建阶段：根据AST构建执行计划
 *    - 语句类型识别：确定具体计划类型
 *    - 上下文准备：初始化执行环境
 *    - 计划构建：组装执行步骤
 *
 * 2. 验证阶段：检查执行条件
 *    - 语法验证：AST结构正确性检查
 *    - 语义验证：对象存在性和权限检查
 *    - 约束验证：业务规则和完整性约束
 *
 * 3. 权限检查阶段：验证用户权限
 *    - 用户身份验证：确认用户身份
 *    - 权限验证：检查操作权限
 *    - 角色检查：角色-based访问控制
 *
 * 4. 预处理阶段：准备执行环境
 *    - 对象引用解析：解析表名、列名等
 *    - 上下文设置：设置执行上下文
 *    - 资源分配：分配所需资源
 *
 * 5. 执行阶段：实际执行查询
 *    - 步骤执行：按顺序执行计划步骤
 *    - 结果处理：处理执行结果
 *    - 状态更新：更新执行状态
 *
 * 6. 后处理阶段：清理和记录
 *    - 元数据更新：更新系统元数据
 *    - 操作日志：记录执行日志
 *    - 资源释放：释放占用的资源
 *
 * 错误处理机制：
 * - 分层错误处理：不同层次的错误处理策略
 * - 错误传播：错误信息向上层传递
 * - 回滚机制：执行失败时的状态回滚
 * - 日志记录：错误信息详细记录
 *
 * 性能优化策略：
 * - 计划缓存：复用相同的查询计划
 * - 连接池：复用数据库连接
 * - 批量处理：减少网络往返
 * - 异步执行：非阻塞操作支持
 *
 * 扩展性设计：
 * - 插件架构：支持自定义计划类型
 * - 配置化：可配置的执行策略
 * - 事件机制：执行生命周期事件通知
 * - 监控集成：性能监控和统计
 *
 * 并发控制：
 * - 线程安全：多线程环境下的安全保证
 * - 锁管理：细粒度的锁控制
 * - 死锁检测：自动死锁检测和处理
 * - 隔离级别：事务隔离级别控制
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
    std::vector<std::unique_ptr<sql_parser::Expression>> arguments_;
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
