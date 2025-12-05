#include "database_manager.h"
#include "execution_context.h"
#include "sql_parser/parser.h"
#include "system_database.h"
#include "unified_executor.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {

// 测试ExecutionContext类的功能
class ExecutionContextTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化数据库管理器
    db_manager_ = std::make_shared<DatabaseManager>(
        "./test_execution_context.db", 1024, 4, 2);
    user_manager_ = std::make_shared<UserManager>();
    system_db_ = std::make_shared<SystemDatabase>(db_manager_);
    unified_executor_ = std::make_shared<UnifiedExecutor>(
        db_manager_, user_manager_, system_db_);

    // 初始化执行上下文
    execution_context_ = std::make_shared<ExecutionContext>(
        db_manager_, user_manager_, system_db_);
  }

  void TearDown() override {
    // 清理资源
    execution_context_.reset();
    unified_executor_.reset();
    system_db_.reset();
    user_manager_.reset();
    db_manager_.reset();

    // 删除测试数据库文件
    std::system("rm -rf ./test_execution_context.db");
  }

  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<UnifiedExecutor> unified_executor_;
  std::shared_ptr<ExecutionContext> execution_context_;
};

// 测试执行上下文初始化
TEST_F(ExecutionContextTest, InitializationTest) {
  // 测试执行上下文是否正确初始化
  EXPECT_TRUE(execution_context_ != nullptr);
  EXPECT_EQ(execution_context_->get_db_manager(), db_manager_);
  EXPECT_EQ(execution_context_->get_user_manager(), user_manager_);
  EXPECT_EQ(execution_context_->get_system_db(), system_db_);
}

// 测试执行统计信息
TEST_F(ExecutionContextTest, ExecutionStatsTest) {
  // 测试执行统计信息的设置和获取
  execution_context_->set_records_affected(100);
  EXPECT_EQ(execution_context_->get_records_affected(), 100);

  execution_context_->set_execution_time_ms(500);
  EXPECT_EQ(execution_context_->get_execution_time_ms(), 500);

  execution_context_->set_used_index(true);
  EXPECT_TRUE(execution_context_->get_used_index());

  execution_context_->set_execution_plan("test_execution_plan");
  EXPECT_EQ(execution_context_->get_execution_plan(), "test_execution_plan");
}

// 测试执行计划管理
TEST_F(ExecutionContextTest, ExecutionPlanTest) {
  // 测试执行计划的设置和获取
  execution_context_->set_plan_details("test_plan_details");
  EXPECT_EQ(execution_context_->get_plan_details(), "test_plan_details");

  execution_context_->set_optimized_plan("test_optimized_plan");
  EXPECT_EQ(execution_context_->get_optimized_plan(), "test_optimized_plan");

  execution_context_->set_query_optimized(true);
  EXPECT_TRUE(execution_context_->get_query_optimized());

  execution_context_->set_cost_estimate(10.5);
  EXPECT_EQ(execution_context_->get_cost_estimate(), 10.5);
}

// 测试事务状态管理
TEST_F(ExecutionContextTest, TransactionStatusTest) {
  // 测试事务状态的设置和获取
  execution_context_->set_in_transaction(true);
  EXPECT_TRUE(execution_context_->get_in_transaction());

  execution_context_->set_transaction_id(12345);
  EXPECT_EQ(execution_context_->get_transaction_id(), 12345);
}

// 测试权限验证器集成
TEST_F(ExecutionContextTest, PermissionValidatorTest) {
  // 测试权限验证器的设置和获取
  auto permission_validator =
      std::make_shared<PermissionValidator>(db_manager_, user_manager_);
  execution_context_->set_permission_validator(permission_validator);
  EXPECT_EQ(execution_context_->get_permission_validator(),
            permission_validator);
}

// 测试优化规则
TEST_F(ExecutionContextTest, OptimizationRulesTest) {
  // 测试优化规则的设置和获取
  std::vector<std::string> rules = {"rule1", "rule2", "rule3"};
  execution_context_->set_optimization_rules(rules);
  EXPECT_EQ(execution_context_->get_optimization_rules(), rules);
}

// 测试索引信息
TEST_F(ExecutionContextTest, IndexInfoTest) {
  // 测试索引信息的设置和获取
  execution_context_->set_index_info("test_index");
  EXPECT_EQ(execution_context_->get_index_info(), "test_index");
}

// 测试当前用户和数据库
TEST_F(ExecutionContextTest, CurrentUserAndDatabaseTest) {
  // 测试当前用户和数据库的设置和获取
  execution_context_->set_current_user("test_user");
  EXPECT_EQ(execution_context_->get_current_user(), "test_user");

  execution_context_->set_current_database("test_db");
  EXPECT_EQ(execution_context_->get_current_database(), "test_db");
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}