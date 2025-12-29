#ifndef SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H
#define SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H

#include <memory>
#include <string>
#include <vector>

// 添加对Statement类完整定义的引用
#include "sql_parser/ast_node.h"

namespace sqlcc {

class DatabaseManager;
class UserManager;
class SystemDatabase;

/**
 * @brief 统一查询计划 - 执行SQL语句的抽象接口
 *
 * 提供统一的查询计划执行接口，支持不同的SQL语句类型
 */
class UnifiedQueryPlan {
public:
    /**
     * @brief 构造函数
     */
    UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);

    /**
     * @brief 析构函数
     */
    virtual ~UnifiedQueryPlan();

    /**
     * @brief 构建查询计划
     * @param stmt SQL语句AST
     * @return 是否构建成功
     */
    virtual bool buildPlan(std::unique_ptr<sql_parser::Statement> stmt);

    /**
     * @brief 执行查询计划
     * @return 执行结果
     */
    virtual std::string executePlan();

    /**
     * @brief 获取计划类型
     * @return 计划类型字符串
     */
    virtual std::string getPlanType() const;

protected:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::unique_ptr<sql_parser::Statement> stmt_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H
