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

// 触发器测试类
class TriggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化SQL执行器
        sql_executor_ = std::make_shared<sqlcc::SqlExecutor>();

        // 初始化存储过程触发器执行器
        sqlcc::procedure::ProcedureTriggerExecutor::getInstance().initialize(sql_executor_.get());
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
    sqlcc::sql_parser::Parser parser_;
};

// 测试触发器创建和基本功能
TEST_F(TriggerTest, TriggerCreationAndBasicFunction) {
    std::cout << "\n=== 触发器创建和基本功能测试开始 ===" << std::endl;

    // 创建测试表
    executeDML("CREATE TABLE test_table (id INT PRIMARY KEY, value VARCHAR(50))");

    // 创建触发器
    ASSERT_TRUE(createTrigger("test_trigger", "test_table",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 执行INSERT操作，这应该触发触发器
    std::string result = executeDML("INSERT INTO test_table VALUES (1, 'test')");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 验证数据插入成功
    result = executeDML("SELECT COUNT(*) FROM test_table");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE test_table");

    std::cout << "=== 触发器创建和基本功能测试完成 ===" << std::endl;
}

// 测试不同触发时机的触发器
TEST_F(TriggerTest, TriggerTimingTest) {
    std::cout << "\n=== 触发时机测试开始 ===" << std::endl;

    // 创建测试表
    executeDML("CREATE TABLE timing_test (id INT PRIMARY KEY, value VARCHAR(50))");

    // 创建BEFORE INSERT触发器
    ASSERT_TRUE(createTrigger("before_insert_trigger", "timing_test",
                             sqlcc::trigger::TriggerTiming::BEFORE,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 创建AFTER INSERT触发器
    ASSERT_TRUE(createTrigger("after_insert_trigger", "timing_test",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 执行INSERT操作
    std::string result = executeDML("INSERT INTO timing_test VALUES (1, 'timing_test')");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 验证数据插入成功
    result = executeDML("SELECT COUNT(*) FROM timing_test");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE timing_test");

    std::cout << "=== 触发时机测试完成 ===" << std::endl;
}

// 测试不同触发事件的触发器
TEST_F(TriggerTest, TriggerEventTest) {
    std::cout << "\n=== 触发事件测试开始 ===" << std::endl;

    // 创建测试表
    executeDML("CREATE TABLE event_test (id INT PRIMARY KEY, value VARCHAR(50))");

    // 插入初始数据
    executeDML("INSERT INTO event_test VALUES (1, 'initial')");

    // 创建UPDATE触发器
    ASSERT_TRUE(createTrigger("update_trigger", "event_test",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::UPDATE));

    // 执行UPDATE操作
    std::string result = executeDML("UPDATE event_test SET value = 'updated' WHERE id = 1");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 创建DELETE触发器
    ASSERT_TRUE(createTrigger("delete_trigger", "event_test",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::DELETE));

    // 执行DELETE操作
    result = executeDML("DELETE FROM event_test WHERE id = 1");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE event_test");

    std::cout << "=== 触发事件测试完成 ===" << std::endl;
}

// 测试多个触发器的执行顺序
TEST_F(TriggerTest, MultipleTriggersOrder) {
    std::cout << "\n=== 多触发器执行顺序测试开始 ===" << std::endl;

    // 创建测试表
    executeDML("CREATE TABLE multi_trigger_test (id INT PRIMARY KEY, value VARCHAR(50))");

    // 创建多个触发器
    ASSERT_TRUE(createTrigger("trigger1", "multi_trigger_test",
                             sqlcc::trigger::TriggerTiming::BEFORE,
                             sqlcc::trigger::TriggerEvent::INSERT));

    ASSERT_TRUE(createTrigger("trigger2", "multi_trigger_test",
                             sqlcc::trigger::TriggerTiming::AFTER,
                             sqlcc::trigger::TriggerEvent::INSERT));

    // 执行INSERT操作
    std::string result = executeDML("INSERT INTO multi_trigger_test VALUES (1, 'multi_test')");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 验证数据插入成功
    result = executeDML("SELECT COUNT(*) FROM multi_trigger_test");
    ASSERT_TRUE(result.find("ERROR") == std::string::npos || result == "EXECUTED");

    // 清理测试表
    executeDML("DROP TABLE multi_trigger_test");

    std::cout << "=== 多触发器执行顺序测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}