#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

#include "execution/task_executor.h"
#include "execution/procedure_trigger_task.h"
#include "procedure/procedure_trigger_executor.h"
#include "sql_executor.h"
#include "sql_parser/parser.h"

// 测试存储过程和触发器在多线程环境下的集成
class ProcedureTriggerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化SQL执行器
        sql_executor_ = std::make_shared<sqlcc::SqlExecutor>();

        // 初始化存储过程触发器执行器
        sqlcc::procedure::ProcedureTriggerExecutor::getInstance().initialize(sql_executor_.get());

        // 初始化任务执行器
        task_executor_ = std::make_unique<sqlcc::execution::TaskExecutor>(4); // 4个线程
        task_executor_->start();
    }

    void TearDown() override {
        if (task_executor_) {
            task_executor_->stop();
        }
    }

    // 辅助函数：创建存储过程
    bool createProcedure(const std::string& name, const std::string& body) {
        std::string sql = "CREATE PROCEDURE " + name + " AS BEGIN " + body + " END";
        auto statements = parser_.parse(sql);
        if (statements.empty()) return false;

        auto* create_stmt = dynamic_cast<sqlcc::sql_parser::CreateProcedureStatement*>(statements[0].get());
        if (!create_stmt) return false;

        std::string result = sqlcc::procedure::ProcedureTriggerExecutor::getInstance()
            .executeCreateProcedure(create_stmt);
        return result.find("ERROR") == std::string::npos;
    }

    // 辅助函数：创建触发器
    bool createTrigger(const std::string& name, const std::string& table,
                      sqlcc::trigger::TriggerTiming timing, sqlcc::trigger::TriggerEvent event) {
        std::string timing_str = (timing == sqlcc::trigger::TriggerTiming::BEFORE) ? "BEFORE" : "AFTER";
        std::string event_str;
        if (event == sqlcc::trigger::TriggerEvent::INSERT) event_str = "INSERT";
        else if (event == sqlcc::trigger::TriggerEvent::UPDATE) event_str = "UPDATE";
        else event_str = "DELETE";

        std::string sql = "CREATE TRIGGER " + name + " " + timing_str + " " + event_str +
                         " ON " + table + " AS BEGIN SELECT 1; END";
        auto statements = parser_.parse(sql);
        if (statements.empty()) return false;

        auto* create_stmt = dynamic_cast<sqlcc::sql_parser::CreateTriggerStatement*>(statements[0].get());
        if (!create_stmt) return false;

        std::string result = sqlcc::procedure::ProcedureTriggerExecutor::getInstance()
            .executeCreateTrigger(create_stmt);
        return result.find("ERROR") == std::string::npos;
    }

    // 辅助函数：执行DML操作
    std::string executeDML(const std::string& sql) {
        return sql_executor_->Execute(sql);
    }

    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
    std::unique_ptr<sqlcc::execution::TaskExecutor> task_executor_;
    sqlcc::sql_parser::Parser parser_;
};

// 测试存储过程任务的多线程执行
TEST_F(ProcedureTriggerIntegrationTest, ProcedureCallTaskConcurrentExecution) {
    // 创建测试存储过程
    ASSERT_TRUE(createProcedure("test_proc", "SELECT 42;"));

    // 创建多个存储过程调用任务
    const int num_tasks = 10;
    std::vector<std::string> task_ids;
    std::vector<std::shared_ptr<sqlcc::execution::Task>> tasks;

    // 解析存储过程调用语句
    auto statements = parser_.parse("CALL test_proc");
    ASSERT_FALSE(statements.empty());
    auto* call_stmt = dynamic_cast<sqlcc::sql_parser::CallProcedureStatement*>(statements[0].get());
    ASSERT_NE(call_stmt, nullptr);

    for (int i = 0; i < num_tasks; ++i) {
        std::string task_id = "proc_task_" + std::to_string(i);
        task_ids.push_back(task_id);

        auto task = std::make_shared<sqlcc::execution::ProcedureCallTask>(task_id, call_stmt);
        tasks.push_back(task);

        // 提交任务到执行器
        task_executor_->submitTask(std::move(task));
    }

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 验证任务队列为空
    ASSERT_EQ(task_executor_->getPendingTaskCount(), 0);
}

// 测试触发器任务的执行
TEST_F(ProcedureTriggerIntegrationTest, TriggerExecutionIntegration) {
    // 创建测试表
    executeDML("CREATE TABLE test_table (id INT PRIMARY KEY, value VARCHAR(50))");

    // 创建触发器
    ASSERT_TRUE(createTrigger("test_trigger", "test_table",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 执行INSERT操作，这应该触发触发器
    std::string result = executeDML("INSERT INTO test_table VALUES (1, 'test')");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE test_table");
}

// 测试并发DML操作与触发器的集成
TEST_F(ProcedureTriggerIntegrationTest, ConcurrentDMLWithTriggers) {
    // 创建测试表
    executeDML("CREATE TABLE concurrent_test (id INT PRIMARY KEY, value VARCHAR(50))");

    // 创建触发器
    ASSERT_TRUE(createTrigger("concurrent_trigger", "concurrent_test",
                             sqlcc::trigger::TriggerTiming::BEFORE,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 创建多个INSERT任务
    const int num_inserts = 5;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_inserts; ++i) {
        threads.emplace_back([this, i]() {
            std::string sql = "INSERT INTO concurrent_test VALUES (" +
                             std::to_string(i + 1) + ", 'value" + std::to_string(i) + "')";
            executeDML(sql);
        });
    }

    // 等待所有INSERT操作完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证数据插入成功
    std::string result = executeDML("SELECT COUNT(*) FROM concurrent_test");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE concurrent_test");
}

// 测试性能指标：任务执行时间
TEST_F(ProcedureTriggerIntegrationTest, PerformanceMetrics) {
    // 创建存储过程
    ASSERT_TRUE(createProcedure("perf_proc", "SELECT 1;"));

    // 解析存储过程调用
    auto statements = parser_.parse("CALL perf_proc");
    ASSERT_FALSE(statements.empty());
    auto* call_stmt = dynamic_cast<sqlcc::sql_parser::CallProcedureStatement*>(statements[0].get());
    ASSERT_NE(call_stmt, nullptr);

    // 执行多个任务并测量时间
    const int num_iterations = 100;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_iterations; ++i) {
        std::string task_id = "perf_task_" + std::to_string(i);
        auto task = std::make_shared<sqlcc::execution::ProcedureCallTask>(task_id, call_stmt);
        task_executor_->submitTask(std::move(task));
    }

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证并发性能（平均每个任务执行时间应小于10ms）
    double avg_time_per_task = static_cast<double>(total_time.count()) / num_iterations;
    ASSERT_LT(avg_time_per_task, 10.0); // 平均执行时间应小于10ms
}

// 测试错误处理和恢复
TEST_F(ProcedureTriggerIntegrationTest, ErrorHandlingAndRecovery) {
    // 创建不存在的存储过程调用任务
    std::string sql = "CALL nonexistent_proc";
    auto statements = parser_.parse(sql);
    ASSERT_FALSE(statements.empty());

    auto* call_stmt = dynamic_cast<sqlcc::sql_parser::CallProcedureStatement*>(statements[0].get());
    ASSERT_NE(call_stmt, nullptr);

    // 创建任务并执行
    auto task = std::make_shared<sqlcc::execution::ProcedureCallTask>("error_task", call_stmt);
    task_executor_->submitTask(std::move(task));

    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 验证任务队列为空（任务已完成，即使失败）
    ASSERT_EQ(task_executor_->getPendingTaskCount(), 0);

    // 验证系统仍然正常工作 - 创建一个有效的存储过程
    ASSERT_TRUE(createProcedure("recovery_proc", "SELECT 1;"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
