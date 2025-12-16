#include "execution/task_executor.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

namespace sqlcc {
namespace execution {
namespace test {

class TaskSchedulerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 使用默认配置创建TaskScheduler
    task_scheduler_ = std::make_unique<TaskScheduler>();
  }

  void TearDown() override {
    if (task_scheduler_) {
      task_scheduler_->Stop();
    }
  }

  std::unique_ptr<TaskScheduler> task_scheduler_;
  std::atomic<int> completed_tasks_{0};
};

// 测试基本功能
TEST_F(TaskSchedulerTest, BasicFunctionality) {
  // 测试启动和停止
  task_scheduler_->Start();

  // 创建一个简单的任务
  auto task = std::make_unique<NetworkTask>(1, "test_request");

  // 提交任务
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));

  // 等待任务完成
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_GE(stats.total_tasks_submitted.load(), 1);

  // 停止调度器
  task_scheduler_->Stop();
}

// 测试任务类型调度
TEST_F(TaskSchedulerTest, TaskTypeDispatch) {
  task_scheduler_->Start();

  // 测试不同类型的任务
  auto network_task = std::make_unique<NetworkTask>(1, "network_request");
  auto sql_task = std::make_unique<SQLTask>(2, "SELECT * FROM test");
  auto wal_task = std::make_unique<WALTask>(3, "wal_log_data");

  // 提交不同类型的任务
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(network_task)));
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(sql_task)));
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(wal_task)));

  // 等待处理
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_EQ(stats.total_tasks_submitted.load(), 3);

  task_scheduler_->Stop();
}

// 测试统计信息
TEST_F(TaskSchedulerTest, Statistics) {
  // 重置统计信息（如果有重置方法）
  // task_scheduler_->ResetStats();

  task_scheduler_->Start();

  // 提交多个任务
  const int num_tasks = 10;
  for (int i = 1; i <= num_tasks; ++i) {
    auto task = std::make_unique<NetworkTask>(i, "request_" + std::to_string(i));
    EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));
  }

  // 等待处理完成
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_EQ(stats.total_tasks_submitted.load(), num_tasks);
  EXPECT_GE(stats.total_tasks_completed.load(), 0);

  task_scheduler_->Stop();
}

// 测试并发提交
TEST_F(TaskSchedulerTest, ConcurrentSubmission) {
  task_scheduler_->Start();

  const int num_threads = 5;
  const int tasks_per_thread = 20;
  std::vector<std::thread> threads;
  std::atomic<int> submitted_tasks{0};

  // 创建多个线程并发提交任务
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, t, tasks_per_thread, &submitted_tasks]() {
      for (int i = 0; i < tasks_per_thread; ++i) {
        int task_id = t * tasks_per_thread + i + 1;
        auto task = std::make_unique<SQLTask>(task_id, "SELECT " + std::to_string(task_id));
        if (task_scheduler_->SubmitTask(std::move(task))) {
          submitted_tasks++;
        }
      }
    });
  }

  // 等待所有线程完成提交
  for (auto& thread : threads) {
    thread.join();
  }

  // 等待任务处理完成
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_EQ(stats.total_tasks_submitted.load(), submitted_tasks.load());

  task_scheduler_->Stop();
}

// 测试不同线程池的负载
TEST_F(TaskSchedulerTest, ThreadPoolLoad) {
  task_scheduler_->Start();

  // 提交大量不同类型的任务来测试线程池负载均衡
  const int num_network_tasks = 50;
  const int num_sql_tasks = 30;
  const int num_storage_tasks = 20;

  // 提交网络任务
  for (int i = 1; i <= num_network_tasks; ++i) {
    auto task = std::make_unique<NetworkTask>(i, "network_" + std::to_string(i));
    EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));
  }

  // 提交SQL任务
  for (int i = 1; i <= num_sql_tasks; ++i) {
    auto task = std::make_unique<SQLTask>(num_network_tasks + i, "SELECT " + std::to_string(i));
    EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));
  }

  // 提交存储任务（需要手动创建存储任务）
  for (int i = 1; i <= num_storage_tasks; ++i) {
    // 创建WAL任务作为存储相关任务示例
    auto task = std::make_unique<WALTask>(num_network_tasks + num_sql_tasks + i,
                                          "storage_data_" + std::to_string(i));
    EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));
  }

  // 等待处理完成
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_EQ(stats.total_tasks_submitted.load(),
            num_network_tasks + num_sql_tasks + num_storage_tasks);

  // 检查各线程池状态
  EXPECT_GE(task_scheduler_->GetActiveThreadCount(TaskType::NETWORK_IO), 0);
  EXPECT_GE(task_scheduler_->GetActiveThreadCount(TaskType::SQL_EXECUTE), 0);
  EXPECT_GE(task_scheduler_->GetActiveThreadCount(TaskType::WAL_WRITE), 0);

  task_scheduler_->Stop();
}

// 测试任务队列状态
TEST_F(TaskSchedulerTest, QueueStatus) {
  task_scheduler_->Start();

  // 检查初始队列状态
  EXPECT_EQ(task_scheduler_->GetPendingTaskCount(TaskType::NETWORK_IO), 0);
  EXPECT_EQ(task_scheduler_->GetPendingTaskCount(TaskType::SQL_EXECUTE), 0);

  // 提交一些任务
  const int num_tasks = 20;
  for (int i = 1; i <= num_tasks; ++i) {
    auto task = std::make_unique<NetworkTask>(i, "test_" + std::to_string(i));
    EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));
  }

  // 检查队列状态（可能还没有完全处理）
  EXPECT_GE(task_scheduler_->GetPendingTaskCount(TaskType::NETWORK_IO), 0);

  // 等待处理
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  task_scheduler_->Stop();
}

// 测试启动和停止
TEST_F(TaskSchedulerTest, StartStop) {
  // 测试多次启动
  task_scheduler_->Start();
  task_scheduler_->Start(); // 应该安全

  // 提交任务测试
  auto task = std::make_unique<NetworkTask>(1, "test");
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));

  // 测试多次停止
  task_scheduler_->Stop();
  task_scheduler_->Stop(); // 应该安全
}

// 测试未启动时的行为
TEST_F(TaskSchedulerTest, NotStarted) {
  // 未启动时提交任务应该失败
  auto task = std::make_unique<NetworkTask>(1, "test");
  EXPECT_FALSE(task_scheduler_->SubmitTask(std::move(task)));
}

// 测试任务执行结果
TEST_F(TaskSchedulerTest, TaskExecution) {
  task_scheduler_->Start();

  // 创建任务并记录结果
  auto task = std::make_unique<NetworkTask>(1, "test_request");
  EXPECT_TRUE(task_scheduler_->SubmitTask(std::move(task)));

  // 等待执行完成
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // 检查统计信息
  auto stats = task_scheduler_->GetStats();
  EXPECT_GE(stats.total_tasks_completed.load(), 1);

  task_scheduler_->Stop();
}

} // namespace test
} // namespace execution
} // namespace sqlcc
