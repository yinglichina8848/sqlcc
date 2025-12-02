#ifndef SQLCC_UNIFIED_QUERY_PLAN_H
#define SQLCC_UNIFIED_QUERY_PLAN_H

#include "sql_parser/ast_nodes.h"
#include "user_manager.h"
#include "system_database.h"
#include "database_manager.h"
#include "error_handler.h"
#include "execution_engine.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

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