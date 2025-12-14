#include "core/execution_context.h"
#include "core/user_manager.h"
#include "database_manager.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <chrono>

namespace sqlcc {

// 测试夹具类
class ExecutionContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的管理器
        user_manager_ = std::make_shared<UserManager>();
        db_manager_ = std::make_shared<DatabaseManager>();
    }

    void TearDown() override {
        user_manager_.reset();
        db_manager_.reset();
    }

    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
};

// 测试ExecutionContext基本构造
TEST_F(ExecutionContextTest, BasicConstructor) {
    ExecutionContext context;
    
    // 测试默认值
    EXPECT_EQ(context.get_current_user(), "");
    EXPECT_EQ(context.get_current_database(), "");
    EXPECT_FALSE(context.is_transactional());
    EXPECT_EQ(context.get_transaction_id(), "");
    EXPECT_FALSE(context.is_read_only());
    EXPECT_EQ(context.get_rows_affected(), 0);
    EXPECT_EQ(context.get_rows_returned(), 0);
    EXPECT_EQ(context.get_execution_time_ms(), 0);
    EXPECT_FALSE(context.is_used_index());
    EXPECT_EQ(context.get_execution_plan(), "");
    EXPECT_FALSE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "");
    EXPECT_FALSE(context.is_query_optimized());
    EXPECT_EQ(context.get_optimization_rules().size(), 0);
    EXPECT_EQ(context.get_cost_estimate(), 0.0);
    EXPECT_EQ(context.get_db_manager(), nullptr);
    EXPECT_EQ(context.get_user_manager(), nullptr);
    EXPECT_EQ(context.get_system_db(), nullptr);
}

// 测试带参数的构造
TEST_F(ExecutionContextTest, ParameterizedConstructor) {
    ExecutionContext context("test_user", "test_db", true);
    
    EXPECT_EQ(context.get_current_user(), "test_user");
    EXPECT_EQ(context.get_current_database(), "test_db");
    EXPECT_TRUE(context.is_transactional());
    EXPECT_EQ(context.get_transaction_id(), "");
    EXPECT_FALSE(context.is_read_only());
}

// 测试带管理器的构造
TEST_F(ExecutionContextTest, ManagerConstructor) {
    auto user_mgr = std::make_shared<UserManager>();
    auto db_mgr = std::make_shared<DatabaseManager>();
    
    ExecutionContext context(db_mgr, user_mgr, nullptr);
    
    EXPECT_EQ(context.get_user_manager(), user_mgr);
    EXPECT_EQ(context.get_db_manager(), db_mgr);
}

// 测试用户和数据库管理
TEST_F(ExecutionContextTest, UserAndDatabaseManagement) {
    ExecutionContext context;
    
    // 测试设置用户和数据库
    context.set_current_user("test_user");
    context.set_current_database("test_database");
    
    EXPECT_EQ(context.get_current_user(), "test_user");
    EXPECT_EQ(context.get_current_database(), "test_database");
    
    // 测试修改用户和数据库
    context.set_current_user("new_user");
    context.set_current_database("new_database");
    
    EXPECT_EQ(context.get_current_user(), "new_user");
    EXPECT_EQ(context.get_current_database(), "new_database");
}

// 测试事务管理
TEST_F(ExecutionContextTest, TransactionManagement) {
    ExecutionContext context;
    
    // 初始状态检查
    EXPECT_FALSE(context.is_transactional());
    EXPECT_EQ(context.get_transaction_id(), "");
    
    // 设置事务状态
    context.set_transactional(true);
    EXPECT_TRUE(context.is_transactional());
    
    // 设置事务ID
    context.set_transaction_id("txn_12345");
    EXPECT_EQ(context.get_transaction_id(), "txn_12345");
    
    // 关闭事务
    context.set_transactional(false);
    EXPECT_FALSE(context.is_transactional());
}

// 测试执行统计信息
TEST_F(ExecutionContextTest, ExecutionStatistics) {
    ExecutionContext context;
    
    // 测试行数统计
    context.set_rows_affected(10);
    EXPECT_EQ(context.get_rows_affected(), 10);
    
    context.increment_rows_affected(5);
    EXPECT_EQ(context.get_rows_affected(), 15);
    
    context.set_rows_returned(100);
    EXPECT_EQ(context.get_rows_returned(), 100);
    
    // 测试执行时间
    context.set_execution_time_ms(150);
    EXPECT_EQ(context.get_execution_time_ms(), 150);
}

// 测试执行计划管理
TEST_F(ExecutionContextTest, ExecutionPlanManagement) {
    ExecutionContext context;
    
    // 测试执行计划设置和获取
    context.set_execution_plan("Full Table Scan");
    EXPECT_EQ(context.get_execution_plan(), "Full Table Scan");
    
    // 测试索引使用标记
    context.set_used_index(true);
    EXPECT_TRUE(context.is_used_index());
    
    context.set_used_index(false);
    EXPECT_FALSE(context.is_used_index());
    
    // 测试计划详情
    context.set_plan_details("Using index idx_name");
    EXPECT_EQ(context.get_plan_details(), "Using index idx_name");
    
    // 测试优化计划
    context.set_optimized_plan("Index Range Scan");
    EXPECT_EQ(context.get_optimized_plan(), "Index Range Scan");
    
    // 测试查询优化状态
    context.set_query_optimized(true);
    EXPECT_TRUE(context.is_query_optimized());
    
    // 测试优化规则
    std::vector<std::string> rules = {"rule1", "rule2"};
    context.set_optimization_rules(rules);
    EXPECT_EQ(context.get_optimization_rules().size(), 2);
    EXPECT_EQ(context.get_optimization_rules()[0], "rule1");
    EXPECT_EQ(context.get_optimization_rules()[1], "rule2");
    
    // 测试索引信息
    context.set_index_info("Used primary key");
    EXPECT_EQ(context.get_index_info(), "Used primary key");
    
    // 测试成本估算
    context.set_cost_estimate(45.5);
    EXPECT_EQ(context.get_cost_estimate(), 45.5);
}

// 测试只读模式
TEST_F(ExecutionContextTest, ReadOnlyMode) {
    ExecutionContext context;
    
    // 初始状态
    EXPECT_FALSE(context.is_read_only());
    
    // 设置只读模式
    context.set_read_only(true);
    EXPECT_TRUE(context.is_read_only());
    
    // 关闭只读模式
    context.set_read_only(false);
    EXPECT_FALSE(context.is_read_only());
}

// 测试错误处理
TEST_F(ExecutionContextTest, ErrorHandling) {
    ExecutionContext context;
    
    // 初始状态
    EXPECT_FALSE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "");
    
    // 设置错误
    context.set_error(true, "Test error message");
    EXPECT_TRUE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "Test error message");
    
    // 清除错误
    context.clear_error();
    EXPECT_FALSE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "");
    
    // 设置新的错误消息
    context.set_error(false, "Another error");
    EXPECT_FALSE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "Another error");
}

// 测试管理器设置和获取
TEST_F(ExecutionContextTest, ManagerSetting) {
    ExecutionContext context;
    
    // 初始状态
    EXPECT_EQ(context.get_db_manager(), nullptr);
    EXPECT_EQ(context.get_user_manager(), nullptr);
    EXPECT_EQ(context.get_system_db(), nullptr);
    
    // 设置管理器
    context.set_db_manager(db_manager_);
    context.set_user_manager(user_manager_);
    
    // 验证设置
    EXPECT_EQ(context.get_db_manager(), db_manager_);
    EXPECT_EQ(context.get_user_manager(), user_manager_);
}

// 测试上下文重置
TEST_F(ExecutionContextTest, ContextReset) {
    ExecutionContext context;
    
    // 设置各种状态
    context.set_current_user("test_user");
    context.set_current_database("test_db");
    context.set_transactional(true);
    context.set_rows_affected(100);
    context.set_error(true, "Some error");
    
    // 重置上下文
    context.reset();
    
    // 验证重置效果
    EXPECT_EQ(context.get_current_user(), "");
    EXPECT_EQ(context.get_current_database(), "");
    EXPECT_FALSE(context.is_transactional());
    EXPECT_EQ(context.get_rows_affected(), 0);
    EXPECT_FALSE(context.has_error());
    EXPECT_EQ(context.get_error_message(), "");
}

// 测试上下文复制
TEST_F(ExecutionContextTest, ContextCloning) {
    ExecutionContext context;
    
    // 设置各种状态
    context.set_current_user("test_user");
    context.set_current_database("test_db");
    context.set_transactional(true);
    context.set_rows_affected(100);
    context.set_error(true, "Some error");
    
    // 复制上下文
    auto cloned_context = context.clone();
    
    // 验证复制的内容
    EXPECT_EQ(cloned_context->get_current_user(), "test_user");
    EXPECT_EQ(cloned_context->get_current_database(), "test_db");
    EXPECT_TRUE(cloned_context->is_transactional());
    EXPECT_EQ(cloned_context->get_rows_affected(), 100);
    EXPECT_TRUE(cloned_context->has_error());
    EXPECT_EQ(cloned_context->get_error_message(), "Some error");
    
    // 验证独立性（修改原上下文不影响克隆）
    context.reset();
    EXPECT_EQ(context.get_current_user(), "");
    EXPECT_EQ(cloned_context->get_current_user(), "test_user");
}

// 测试字符串表示
TEST_F(ExecutionContextTest, StringRepresentation) {
    ExecutionContext context;
    
    context.set_current_user("test_user");
    context.set_current_database("test_db");
    context.set_transactional(true);
    context.set_rows_affected(50);
    
    std::string str_repr = context.to_string();
    
    // 验证字符串包含关键信息
    EXPECT_NE(str_repr.find("test_user"), std::string::npos);
    EXPECT_NE(str_repr.find("test_db"), std::string::npos);
    EXPECT_NE(str_repr.find("50"), std::string::npos);
}

// 测试边界条件
TEST_F(ExecutionContextTest, BoundaryConditions) {
    ExecutionContext context;
    
    // 测试空字符串
    context.set_current_user("");
    context.set_current_database("");
    EXPECT_EQ(context.get_current_user(), "");
    EXPECT_EQ(context.get_current_database(), "");
    
    // 测试极长的字符串
    std::string long_string(1000, 'a');
    context.set_current_user(long_string);
    EXPECT_EQ(context.get_current_user(), long_string);
    
    // 测试负数（如果设置方法允许）
    // context.set_rows_affected(-1); // 如果方法不检查负数，可能导致问题
    
    // 测试零值
    context.set_cost_estimate(0.0);
    EXPECT_EQ(context.get_cost_estimate(), 0.0);
}

// 测试线程安全性（基础测试）
TEST_F(ExecutionContextTest, BasicThreadSafety) {
    ExecutionContext context;
    
    // 简单的并发访问测试
    std::atomic<int> success_count{0};
    const int num_threads = 10;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&context, &success_count, i]() {
            try {
                context.set_current_user("user_" + std::to_string(i));
                context.set_rows_affected(i);
                success_count.fetch_add(1);
            } catch (...) {
                // 捕获可能的异常
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有线程都成功执行
    EXPECT_EQ(success_count.load(), num_threads);
}

// 测试RAII模式（内存安全）
TEST_F(ExecutionContextTest, RAIIAndMemorySafety) {
    // 测试智能指针的正确使用
    {
        ExecutionContext context;
        context.set_db_manager(db_manager_);
        context.set_user_manager(user_manager_);
        
        // 确保管理器引用计数正确
        EXPECT_TRUE(db_manager_);
        EXPECT_TRUE(user_manager_);
    }
    
    // 上下文销毁后，外部管理器引用仍然有效
    EXPECT_TRUE(db_manager_);
    EXPECT_TRUE(user_manager_);
}

// 测试复杂场景
TEST_F(ExecutionContextTest, ComplexScenario) {
    ExecutionContext context;
    
    // 模拟完整的查询执行流程
    context.set_current_user("admin");
    context.set_current_database("production");
    context.set_read_only(true);
    
    // 模拟查询执行
    context.set_execution_plan("Index Scan on orders");
    context.set_used_index(true);
    context.set_query_optimized(true);
    context.set_cost_estimate(25.3);
    
    std::vector<std::string> rules = {
        "Index Selection",
        "Predicate Pushdown",
        "Join Reordering"
    };
    context.set_optimization_rules(rules);
    
    // 模拟执行结果
    context.set_rows_affected(1500);
    context.set_rows_returned(1500);
    context.set_execution_time_ms(45);
    context.set_index_info("Used idx_orders_date");
    
    // 验证所有设置都正确
    EXPECT_EQ(context.get_current_user(), "admin");
    EXPECT_EQ(context.get_current_database(), "production");
    EXPECT_TRUE(context.is_read_only());
    EXPECT_EQ(context.get_execution_plan(), "Index Scan on orders");
    EXPECT_TRUE(context.is_used_index());
    EXPECT_TRUE(context.is_query_optimized());
    EXPECT_EQ(context.get_cost_estimate(), 25.3);
    EXPECT_EQ(context.get_optimization_rules().size(), 3);
    EXPECT_EQ(context.get_rows_affected(), 1500);
    EXPECT_EQ(context.get_rows_returned(), 1500);
    EXPECT_EQ(context.get_execution_time_ms(), 45);
    EXPECT_EQ(context.get_index_info(), "Used idx_orders_date");
    
    // 生成调试信息
    std::string debug_info = context.to_string();
    EXPECT_NE(debug_info.find("admin"), std::string::npos);
    EXPECT_NE(debug_info.find("production"), std::string::npos);
    EXPECT_NE(debug_info.find("1500"), std::string::npos);
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}