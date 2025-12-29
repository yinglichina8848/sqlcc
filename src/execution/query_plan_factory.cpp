#include "execution/query_plan_factory.h"
#include "execution/unified_query_plan.h"

namespace sqlcc {

std::unique_ptr<UnifiedQueryPlan> QueryPlanFactory::createPlan(
    std::unique_ptr<sql_parser::Statement> stmt,
    std::shared_ptr<DatabaseManager> db_manager,
    std::shared_ptr<UserManager> user_manager,
    std::shared_ptr<SystemDatabase> system_db) {
    // 创建一个基本的查询计划
    auto plan = std::make_unique<UnifiedQueryPlan>(db_manager, user_manager, system_db);

    // 构建查询计划
    if (plan->buildPlan(std::move(stmt))) {
        return plan;
    }

    return nullptr; // 构建失败
}

} // namespace sqlcc
