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

namespace sqlcc {

// 存储过程测试类
class ProcedureTest : public ::testing::Test {
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

    // 辅助函数：执行DML操作
    std::string executeDML(const std::string& sql) {
        return sql_executor_->Execute(sql);
    }

    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
    std::unique_ptr<sqlcc::execution::TaskExecutor> task_executor_;
    sqlcc::sql_parser::Parser parser_;
};

// 测试存储过程创建和调用
TEST_F(ProcedureTest, ProcedureCreationAndCall) {
    std::cout << "\n=== 存储过程创建和调用测试开始 ===" << std::endl;

    // 创建简单的存储过程
    ASSERT_TRUE(createProcedure("simple_proc", "SELECT 42;"));

    // 解析存储过程调用语句
    auto statements = parser_.parse("CALL simple_proc");
    ASSERT_FALSE(statements.empty());
    auto* call_stmt = dynamic_cast<sqlcc::sql_parser::CallProcedureStatement*>(statements[0].get());
    ASSERT_NE(call_stmt, nullptr);

    // 执行存储过程调用
    auto task = std::make_shared<sqlcc::execution::ProcedureCallTask>("simple_proc_task", call_stmt);
    task_executor_->submitTask(std::move(task));

    // 等待执行完成
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 验证任务队列为空
    ASSERT_EQ(task_executor_->getPendingTaskCount(), 0);

    std::cout << "=== 存储过程创建和调用测试完成 ===" << std::endl;
}

// 测试存储过程参数传递
TEST_F(ProcedureTest, ProcedureWithParameters) {
    std::cout << "\n=== 存储过程参数测试开始 ===" << std::endl;

    // 创建带参数的存储过程（如果支持）
    // 这里暂时使用简单的SELECT过程来模拟

    ASSERT_TRUE(createProcedure("param_proc", "SELECT 'Parameter Test';"));

    // 调用存储过程
    auto statements = parser_.parse("CALL param_proc");
    ASSERT_FALSE(statements.empty());
    auto* call_stmt = dynamic_cast<sqlcc::sql_parser::CallProcedureStatement*>(statements[0].get());
    ASSERT_NE(call_stmt, nullptr);

    auto task = std::make_shared<sqlcc::execution::ProcedureCallTask>("param_proc_task", call_stmt);
    task_executor_->submitTask(std::move(task));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(task_executor_->getPendingTaskCount(), 0);

    std::cout << "=== 存储过程参数测试完成 ===" << std::endl;
}

// 测试存储过程错误处理
TEST_F(ProcedureTest, ProcedureErrorHandling) {
    std::cout << "\n=== 存储过程错误处理测试开始 ===" << std::endl;

    // 尝试调用不存在的存储过程
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

    std::cout << "=== 存储过程错误处理测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}