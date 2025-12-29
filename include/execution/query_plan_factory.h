#ifndef SQLCC_EXECUTION_QUERY_PLAN_FACTORY_H
#define SQLCC_EXECUTION_QUERY_PLAN_FACTORY_H

#include <memory>
#include <string>
#include <unordered_map>

namespace sqlcc {

class DatabaseManager;
class UserManager;
class SystemDatabase;
class UnifiedQueryPlan;

namespace sql_parser {
class Statement;
} // namespace sql_parser

/**
 * @brief 查询计划工厂 - 负责创建各种查询计划
 *
 * 根据SQL语句类型创建相应的查询计划对象
 */
class QueryPlanFactory {
public:
    /**
     * @brief 创建查询计划
     * @param stmt SQL语句AST
     * @param db_manager 数据库管理器
     * @param user_manager 用户管理器
     * @param system_db 系统数据库
     * @return 查询计划对象
     */
    static std::unique_ptr<UnifiedQueryPlan> createPlan(
        std::unique_ptr<sql_parser::Statement> stmt,
        std::shared_ptr<DatabaseManager> db_manager,
        std::shared_ptr<UserManager> user_manager,
        std::shared_ptr<SystemDatabase> system_db);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_QUERY_PLAN_FACTORY_H
