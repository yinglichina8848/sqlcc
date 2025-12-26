#include "execution/task_executor.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace sqlcc {

// 测试TaskExecutor类
class TaskExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

// 测试TaskExecutor基本构造
TEST_F(TaskExecutorTest, BasicConstructor) {
    auto executor = std::make_unique<TaskExecutor>();
    EXPECT_NE(executor, nullptr);
}

// 测试TaskExecutor初始化
TEST_F(TaskExecutorTest, Initialize) {
    TaskExecutor executor;
    bool result = executor.initialize(2); // 使用2个线程进行测试
    EXPECT_TRUE(result);
}

// 测试TaskExecutor析构
TEST_F(TaskExecutorTest, Destructor) {
    {
        auto executor = std::make_unique<TaskExecutor>();
        EXPECT_NE(executor, nullptr);
    }
    // 验证对象正确析构
    SUCCEED();
}

// 测试任务提交
TEST_F(TaskExecutorTest, SubmitTask) {
    TaskExecutor executor;
    executor.initialize(2);
    
    // 由于Task是抽象类，我们测试函数提交接口
    int task_id = executor.submitTask(
        [](ExecutionContext& context) {
            // 空任务
        },
        "test task",
        TaskPriority::NORMAL
    );
    
    EXPECT_GE(task_id, 0);
}

// 测试任务管理功能
TEST_F(TaskExecutorTest, TaskManagement) {
    TaskExecutor executor;
    executor.initialize(2);
    
    int task_id = executor.submitTask(
        [](ExecutionContext& context) {
            // 空任务
        },
        "test task",
        TaskPriority::NORMAL
    );
    
    TaskStatus status = executor.getTaskStatus(task_id);
    // 任务可能已经完成，所以接受多种状态
    EXPECT_TRUE(status == TaskStatus::PENDING || 
                status == TaskStatus::RUNNING || 
                status == TaskStatus::COMPLETED);
}

// 测试关闭功能
TEST_F(TaskExecutorTest, Shutdown) {
    TaskExecutor executor;
    executor.initialize(2);
    executor.shutdown();
    SUCCEED(); // 验证没有崩溃
}

} // namespace sqlcc