#include "execution/unified_query_plan.h"

namespace sqlcc {

UnifiedQueryPlan::UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                 std::shared_ptr<UserManager> user_manager,
                                 std::shared_ptr<SystemDatabase> system_db)
    : db_manager_(db_manager), user_manager_(user_manager), system_db_(system_db) {}

UnifiedQueryPlan::~UnifiedQueryPlan() {}

bool UnifiedQueryPlan::buildPlan(std::unique_ptr<sql_parser::Statement> stmt) {
    stmt_ = std::move(stmt);
    return true;
}

std::string UnifiedQueryPlan::executePlan() {
    // 简化的执行逻辑
    return "Query executed successfully";
}

std::string UnifiedQueryPlan::getPlanType() const {
    return "UnifiedQueryPlan";
}

} // namespace sqlcc
