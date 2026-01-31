/**
 * WHY: ExecutionContext是SQLCC的执行上下文管理类，负责SQL执行的统一状态管理。
 * 为了保证数据库系统的正确执行，需要全面测试执行上下文功能：
 * 1. 用户和数据库上下文管理
 * 2. 事务状态跟踪
 * 3. 执行统计收集
 * 4. 错误状态管理
 * 5. 资源管理
 *
 * WHAT: ExecutionContext单元测试
 * 测试覆盖：
 * - 上下文初始化
 * - 用户和数据库设置
 * - 事务状态管理
 * - 执行统计（行数、时间）
 * - 执行计划管理
 * - 错误处理
 *
 * HOW: 使用Google Test框架，测试ExecutionContext的各个功能模块。
 */

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <chrono>

#include "src/core/execution_context.h"
#include "src/core/core_database_manager.h"
#include "src/core/user_manager.h"
#include "src/exception/exception.h"
#include "src/logger/logger.h"

namespace sqlcc {
namespace test {

// ExecutionContext测试 fixture
class ExecutionContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<Logger>("ExecutionContextTest");
        context_ = std::make_unique<ExecutionContext>();
    }

    void TearDown() override {
        context_.reset();
    }

    std::shared_ptr<Logger> logger_;
    std::unique_ptr<ExecutionContext> context_;
};

// ============ 构造函数测试 ============

TEST_F(ExecutionContextTest, DefaultConstructor) {
    // 测试默认构造函数
    ExecutionContext default_context;
    
    // 验证默认值
    EXPECT_TRUE(default_context.get_current_user().empty());
    EXPECT_TRUE(default_context.get_current_database().empty());
    EXPECT_FALSE(default_context.is_transactional());
    EXPECT_FALSE(default_context.is_read_only());
    EXPECT_EQ(default_context.get_rows_affected(), 0);
    EXPECT_EQ(default_context.get_rows_returned(), 0);
    EXPECT_EQ(default_context.get_execution_time_ms(), 0);
}

TEST_F(ExecutionContextTest, ParameterizedConstructor) {
    // 测试带参数的构造函数
    ExecutionContext context("test_user", "test_database", true);
    
    EXPECT_EQ(context.get_current_user(), "test_user");
    EXPECT_EQ(context.get_current_database(), "test_database");
    EXPECT_TRUE(context.is_transactional());
    EXPECT_FALSE(context.is_read_only());
}

TEST_F(ExecutionContextTest, ParameterizedConstructor_ReadOnly) {
    // 测试带参数的构造函数 - 只读模式
    ExecutionContext context("test_user", "test_database", false);
    
    EXPECT_EQ(context.get_current_user(), "test_user");
    EXPECT_EQ(context.get_current_database(), "test_database");
    EXPECT_FALSE(context.is_transactional());
}

// ============ 用户管理测试 ============

TEST_F(ExecutionContextTest, SetAndGetCurrentUser) {
    // 测试设置和获取当前用户
    context_->set_current_user("new_user");
    
    EXPECT_EQ(context_->get_current_user(), "new_user");
}

TEST_F(ExecutionContextTest, SetCurrentUser_Empty) {
    // 测试设置空用户
    context_->set_current_user("test_user");
    context_->set_current_user("");
    
    EXPECT_TRUE(context_->get_current_user().empty());
}

TEST_F(ExecutionContextTest, SetCurrentUser_MultipleChanges) {
    // 测试多次更改用户
    context_->set_current_user("user1");
    EXPECT_EQ(context_->get_current_user(), "user1");
    
    context_->set_current_user("user2");
    EXPECT_EQ(context_->get_current_user(), "user2");
    
    context_->set_current_user("user3");
    EXPECT_EQ(context_->get_current_user(), "user3");
}

// ============ 数据库管理测试 ============

TEST_F(ExecutionContextTest, SetAndGetCurrentDatabase) {
    // 测试设置和获取当前数据库
    context_->set_current_database("new_database");
    
    EXPECT_EQ(context_->get_current_database(), "new_database");
}

TEST_F(ExecutionContextTest, SetCurrentDatabase_Empty) {
    // 测试设置空数据库
    context_->set_current_database("test_db");
    context_->set_current_database("");
    
    EXPECT_TRUE(context_->get_current_database().empty());
}

TEST_F(ExecutionContextTest, SetCurrentDatabase_SpecialCharacters) {
    // 测试包含特殊字符的数据库名
    context_->set_current_database("my_database_v2");
    
    EXPECT_EQ(context_->get_current_database(), "my_database_v2");
}

// ============ 事务状态测试 ============

TEST_F(ExecutionContextTest, SetAndGetTransactional) {
    // 测试设置和获取事务状态
    context_->set_transactional(true);
    EXPECT_TRUE(context_->is_transactional());
    
    context_->set_transactional(false);
    EXPECT_FALSE(context_->is_transactional());
}

TEST_F(ExecutionContextTest, Transactional_DefaultFalse) {
    // 测试默认事务状态为false
    EXPECT_FALSE(context_->is_transactional());
}

TEST_F(ExecutionContextTest, SetAndGetTransactionId) {
    // 测试设置和获取事务ID
    context_->set_transaction_id("tx_12345");
    
    EXPECT_EQ(context_->get_transaction_id(), "tx_12345");
}

TEST_F(ExecutionContextTest, SetTransactionId_Empty) {
    // 测试设置空事务ID
    context_->set_transaction_id("tx_12345");
    context_->set_transaction_id("");
    
    EXPECT_TRUE(context_->get_transaction_id().empty());
}

// ============ 只读状态测试 ============

TEST_F(ExecutionContextTest, SetAndGetReadOnly) {
    // 测试设置和获取只读状态
    context_->set_read_only(true);
    EXPECT_TRUE(context_->is_read_only());
    
    context_->set_read_only(false);
    EXPECT_FALSE(context_->is_read_only());
}

TEST_F(ExecutionContextTest, ReadOnly_DefaultFalse) {
    // 测试默认只读状态为false
    EXPECT_FALSE(context_->is_read_only());
}

// ============ 影响行数测试 ============

TEST_F(ExecutionContextTest, SetAndGetRowsAffected) {
    // 测试设置和获取影响的行数
    context_->set_rows_affected(100);
    
    EXPECT_EQ(context_->get_rows_affected(), 100);
}

TEST_F(ExecutionContextTest, IncrementRowsAffected) {
    // 测试增加影响的行数
    context_->set_rows_affected(50);
    context_->increment_rows_affected();
    
    EXPECT_EQ(context_->get_rows_affected(), 51);
    
    context_->increment_rows_affected(10);
    
    EXPECT_EQ(context_->get_rows_affected(), 61);
}

TEST_F(ExecutionContextTest, IncrementRowsAffected_Default) {
    // 测试默认增加1行
    context_->set_rows_affected(0);
    context_->increment_rows_affected();
    
    EXPECT_EQ(context_->get_rows_affected(), 1);
}

TEST_F(ExecutionContextTest, RowsAffected_Zero) {
    // 测试初始行数为0
    EXPECT_EQ(context_->get_rows_affected(), 0);
}

// ============ 返回行数测试 ============

TEST_F(ExecutionContextTest, SetAndGetRowsReturned) {
    // 测试设置和获取返回的行数
    context_->set_rows_returned(50);
    
    EXPECT_EQ(context_->get_rows_returned(), 50);
}

TEST_F(ExecutionContextTest, RowsReturned_Zero) {
    // 测试初始返回行数为0
    EXPECT_EQ(context_->get_rows_returned(), 0);
}

// ============ 执行时间测试 ============

TEST_F(ExecutionContextTest, SetAndGetExecutionTime) {
    // 测试设置和获取执行时间
    context_->set_execution_time_ms(150);
    
    EXPECT_EQ(context_->get_execution_time_ms(), 150);
}

TEST_F(ExecutionContextTest, ExecutionTime_Zero) {
    // 测试初始执行时间为0
    EXPECT_EQ(context_->get_execution_time_ms(), 0);
}

TEST_F(ExecutionContextTest, ExecutionTime_LargeValue) {
    // 测试大时间值
    context_->set_execution_time_ms(86400000);  // 24小时
    
    EXPECT_EQ(context_->get_execution_time_ms(), 86400000);
}

// ============ 索引使用测试 ============

TEST_F(ExecutionContextTest, SetAndGetUsedIndex) {
    // 测试设置和获取索引使用状态
    context_->set_used_index(true);
    EXPECT_TRUE(context_->is_used_index());
    
    context_->set_used_index(false);
    EXPECT_FALSE(context_->is_used_index());
}

TEST_F(ExecutionContextTest, UsedIndex_DefaultFalse) {
    // 测试默认不使用索引
    EXPECT_FALSE(context_->is_used_index());
}

// ============ 执行计划测试 ============

TEST_F(ExecutionContextTest, SetAndGetExecutionPlan) {
    // 测试设置和获取执行计划
    context_->set_execution_plan("TABLE_SCAN");
    
    EXPECT_EQ(context_->get_execution_plan(), "TABLE_SCAN");
}

TEST_F(ExecutionContextTest, ExecutionPlan_Empty) {
    // 测试初始执行计划为空
    EXPECT_TRUE(context_->get_execution_plan().empty());
}

TEST_F(ExecutionContextTest, SetAndGetPlanDetails) {
    // 测试设置和获取执行计划详情
    context_->set_plan_details("Index Scan on idx_table (cost=10.5)");
    
    EXPECT_EQ(context_->get_plan_details(), "Index Scan on idx_table (cost=10.5)");
}

TEST_F(ExecutionContextTest, PlanDetails_Empty) {
    // 测试初始计划详情为空
    EXPECT_TRUE(context_->get_plan_details().empty());
}

// ============ 优化计划测试 ============

TEST_F(ExecutionContextTest, SetAndGetOptimizedPlan) {
    // 测试设置和获取优化后的执行计划
    context_->set_optimized_plan("OPTIMIZED_PLAN");
    
    EXPECT_EQ(context_->get_optimized_plan(), "OPTIMIZED_PLAN");
}

TEST_F(ExecutionContextTest, SetAndGetQueryOptimized) {
    // 测试设置和获取查询优化状态
    context_->set_query_optimized(true);
    EXPECT_TRUE(context_->is_query_optimized());
    
    context_->set_query_optimized(false);
    EXPECT_FALSE(context_->is_query_optimized());
}

TEST_F(ExecutionContextTest, QueryOptimized_DefaultFalse) {
    // 测试默认查询未优化
    EXPECT_FALSE(context_->is_query_optimized());
}

TEST_F(ExecutionContextTest, SetAndGetOptimizationRules) {
    // 测试设置和获取优化规则
    std::vector<std::string> rules = {"rule1", "rule2", "rule3"};
    context_->set_optimization_rules(rules);
    
    const auto& stored_rules = context_->get_optimization_rules();
    EXPECT_EQ(stored_rules.size(), 3);
    EXPECT_EQ(stored_rules[0], "rule1");
    EXPECT_EQ(stored_rules[1], "rule2");
    EXPECT_EQ(stored_rules[2], "rule3");
}

TEST_F(ExecutionContextTest, OptimizationRules_Empty) {
    // 测试初始优化规则为空
    EXPECT_TRUE(context_->get_optimization_rules().empty());
}

// ============ 索引信息测试 ============

TEST_F(ExecutionContextTest, SetAndGetIndexInfo) {
    // 测试设置和获取索引信息
    context_->set_index_info("Using index: PRIMARY_KEY");
    
    EXPECT_EQ(context_->get_index_info(), "Using index: PRIMARY_KEY");
}

TEST_F(ExecutionContextTest, IndexInfo_Empty) {
    // 测试初始索引信息为空
    EXPECT_TRUE(context_->get_index_info().empty());
}

// ============ 成本估算测试 ============

TEST_F(ExecutionContextTest, SetAndGetCostEstimate) {
    // 测试设置和获取成本估算
    context_->set_cost_estimate(10.5);
    
    EXPECT_DOUBLE_EQ(context_->get_cost_estimate(), 10.5);
}

TEST_F(ExecutionContextTest, CostEstimate_Zero) {
    // 测试初始成本为0
    EXPECT_DOUBLE_EQ(context_->get_cost_estimate(), 0.0);
}

TEST_F(ExecutionContextTest, CostEstimate_Negative) {
    // 测试负成本估算
    context_->set_cost_estimate(-5.0);
    
    EXPECT_DOUBLE_EQ(context_->get_cost_estimate(), -5.0);
}

// ============ 错误处理测试 ============

TEST_F(ExecutionContextTest, SetAndGetErrorCode) {
    // 测试设置和获取错误码
    context_->set_error_code(ErrorCode::SYNTAX_ERROR);
    
    EXPECT_EQ(context_->get_error_code(), ErrorCode::SYNTAX_ERROR);
}

TEST_F(ExecutionContextTest, ErrorCode_Success) {
    // 测试初始错误码为SUCCESS
    EXPECT_EQ(context_->get_error_code(), ErrorCode::SUCCESS);
}

TEST_F(ExecutionContextTest, SetAndGetErrorMessage) {
    // 测试设置和获取错误消息
    context_->set_error_message("Test error message");
    
    EXPECT_EQ(context_->get_error_message(), "Test error message");
}

TEST_F(ExecutionContextTest, ErrorMessage_Empty) {
    // 测试初始错误消息为空
    EXPECT_TRUE(context_->get_error_message().empty());
}

TEST_F(ExecutionContextTest, SetAndGetErrorLevel) {
    // 测试设置和获取错误级别
    context_->set_error_level(ErrorLevel::ERROR);
    
    EXPECT_EQ(context_->get_error_level(), ErrorLevel::ERROR);
}

TEST_F(ExecutionContextTest, ErrorLevel_Info) {
    // 测试初始错误级别为INFO
    EXPECT_EQ(context_->get_error_level(), ErrorLevel::INFO);
}

TEST_F(ExecutionContextTest, SetAndGetErrorContext) {
    // 测试设置和获取错误上下文
    context_->set_error_context("Table 'test_table' not found");
    
    EXPECT_EQ(context_->get_error_context(), "Table 'test_table' not found");
}

TEST_F(ExecutionContextTest, ErrorContext_Empty) {
    // 测试初始错误上下文为空
    EXPECT_TRUE(context_->get_error_context().empty());
}

TEST_F(ExecutionContextTest, ClearError) {
    // 测试清除错误状态
    context_->set_error_code(ErrorCode::PERMISSION_DENIED);
    context_->set_error_message("Access denied");
    context_->set_error_level(ErrorLevel::ERROR);
    
    context_->clear_error();
    
    EXPECT_EQ(context_->get_error_code(), ErrorCode::SUCCESS);
    EXPECT_TRUE(context_->get_error_message().empty());
    EXPECT_EQ(context_->get_error_level(), ErrorLevel::INFO);
}

TEST_F(ExecutionContextTest, HasError_NoError) {
    // 测试无错误状态
    EXPECT_FALSE(context_->has_error());
}

TEST_F(ExecutionContextTest, HasError_WithError) {
    // 测试有错误状态
    context_->set_error_code(ErrorCode::SYNTAX_ERROR);
    
    EXPECT_TRUE(context_->has_error());
}

// ============ 警告信息测试 ============

TEST_F(ExecutionContextTest, SetAndGetWarningMessage) {
    // 测试设置和获取警告消息
    context_->set_warning_message("Warning: Data truncation");
    
    EXPECT_EQ(context_->get_warning_message(), "Warning: Data truncation");
}

TEST_F(ExecutionContextTest, WarningMessage_Empty) {
    // 测试初始警告消息为空
    EXPECT_TRUE(context_->get_warning_message().empty());
}

TEST_F(ExecutionContextTest, AddWarning) {
    // 测试添加警告
    context_->add_warning("Warning 1");
    context_->add_warning("Warning 2");
    
    const auto& warnings = context_->get_warnings();
    EXPECT_EQ(warnings.size(), 2);
    EXPECT_EQ(warnings[0], "Warning 1");
    EXPECT_EQ(warnings[1], "Warning 2");
}

TEST_F(ExecutionContextTest, Warnings_Empty) {
    // 测试初始警告列表为空
    EXPECT_TRUE(context_->get_warnings().empty());
}

TEST_F(ExecutionContextTest, ClearWarnings) {
    // 测试清除警告
    context_->add_warning("Warning 1");
    context_->add_warning("Warning 2");
    
    context_->clear_warnings();
    
    EXPECT_TRUE(context_->get_warnings().empty());
}

// ============ 自定义属性测试 ============

TEST_F(ExecutionContextTest, SetAndGetCustomProperty) {
    // 测试设置和获取自定义属性
    context_->set_custom_property("key1", "value1");
    
    EXPECT_EQ(context_->get_custom_property("key1"), "value1");
}

TEST_F(ExecutionContextTest, CustomProperty_NotFound) {
    // 测试获取不存在的自定义属性
    EXPECT_TRUE(context_->get_custom_property("nonexistent").empty());
}

TEST_F(ExecutionContextTest, CustomProperty_Overwrite) {
    // 测试覆盖自定义属性
    context_->set_custom_property("key1", "value1");
    context_->set_custom_property("key1", "value2");
    
    EXPECT_EQ(context_->get_custom_property("key1"), "value2");
}

TEST_F(ExecutionContextTest, HasCustomProperty) {
    // 测试检查自定义属性是否存在
    context_->set_custom_property("key1", "value1");
    
    EXPECT_TRUE(context_->has_custom_property("key1"));
    EXPECT_FALSE(context_->has_custom_property("nonexistent"));
}

TEST_F(ExecutionContextTest, RemoveCustomProperty) {
    // 测试移除自定义属性
    context_->set_custom_property("key1", "value1");
    context_->remove_custom_property("key1");
    
    EXPECT_FALSE(context_->has_custom_property("key1"));
}

// ============ 重置测试 ============

TEST_F(ExecutionContextTest, Reset_AllFields) {
    // 测试重置所有字段
    context_->set_current_user("test_user");
    context_->set_current_database("test_db");
    context_->set_transactional(true);
    context_->set_rows_affected(100);
    context_->set_error_code(ErrorCode::SYNTAX_ERROR);
    context_->add_warning("Test warning");
    
    context_->reset();
    
    EXPECT_TRUE(context_->get_current_user().empty());
    EXPECT_TRUE(context_->get_current_database().empty());
    EXPECT_FALSE(context_->is_transactional());
    EXPECT_EQ(context_->get_rows_affected(), 0);
    EXPECT_EQ(context_->get_error_code(), ErrorCode::SUCCESS);
    EXPECT_TRUE(context_->get_warnings().empty());
}

// ============ 资源管理测试 ============

TEST_F(ExecutionContextTest, SetAndGetMemoryEstimate) {
    // 测试设置和获取内存估算
    context_->set_memory_estimate(1024);
    
    EXPECT_EQ(context_->get_memory_estimate(), 1024);
}

TEST_F(ExecutionContextTest, MemoryEstimate_Zero) {
    // 测试初始内存估算为0
    EXPECT_EQ(context_->get_memory_estimate(), 0);
}

TEST_F(ExecutionContextTest, SetAndGetDiskEstimate) {
    // 测试设置和获取磁盘估算
    context_->set_disk_estimate(2048);
    
    EXPECT_EQ(context_->get_disk_estimate(), 2048);
}

TEST_F(ExecutionContextTest, DiskEstimate_Zero) {
    // 测试初始磁盘估算为0
    EXPECT_EQ(context_->get_disk_estimate(), 0);
}

// ============ 并发控制测试 ============

TEST_F(ExecutionContextTest, SetAndGetLockType) {
    // 测试设置和获取锁类型
    context_->set_lock_type(LockType::EXCLUSIVE);
    
    EXPECT_EQ(context_->get_lock_type(), LockType::EXCLUSIVE);
}

TEST_F(ExecutionContextTest, LockType_None) {
    // 测试初始锁类型为NONE
    EXPECT_EQ(context_->get_lock_type(), LockType::NONE);
}

TEST_F(ExecutionContextTest, SetAndGetLockTimeoutMs) {
    // 测试设置和获取锁超时
    context_->set_lock_timeout_ms(5000);
    
    EXPECT_EQ(context_->get_lock_timeout_ms(), 5000);
}

TEST_F(ExecutionContextTest, LockTimeoutMs_Default) {
    // 测试默认锁超时
    EXPECT_EQ(context_->get_lock_timeout_ms(), 30000);  // 默认30秒
}

// ============ 隔离级别测试 ============

TEST_F(ExecutionContextTest, SetAndGetIsolationLevel) {
    // 测试设置和获取隔离级别
    context_->set_isolation_level(IsolationLevel::REPEATABLE_READ);
    
    EXPECT_EQ(context_->get_isolation_level(), IsolationLevel::REPEATABLE_READ);
}

TEST_F(ExecutionContextTest, IsolationLevel_Default) {
    // 测试默认隔离级别
    EXPECT_EQ(context_->get_isolation_level(), IsolationLevel::READ_COMMITTED);
}

}  // namespace test
}  // namespace sqlcc
