#include "execution/task_executor.h"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

using namespace sqlcc::execution;

int main() {
    std::cout << "Starting TaskExecutor test..." << std::endl;
    
    // 创建任务执行器
    TaskExecutor executor(4); // 4个工作线程
    executor.start();
    
    std::cout << "TaskExecutor started with " << executor.getActiveThreadCount() << " threads" << std::endl;
    
    // 创建不同类型的任务
    std::vector<std::unique_ptr<Task>> tasks;
    
    // 创建网络任务
    for (int i = 0; i < 5; ++i) {
        std::string task_id = "network_task_" + std::to_string(i);
        std::string request_data = "GET /api/data/" + std::to_string(i);
        tasks.push_back(std::make_unique<NetworkTask>(task_id, request_data));
    }
    
    // 创建SQL任务
    for (int i = 0; i < 5; ++i) {
        std::string task_id = "sql_task_" + std::to_string(i);
        std::string sql_statement = "SELECT * FROM table_" + std::to_string(i);
        auto sql_task = std::make_unique<SQLTask>(task_id, sql_statement);
        sql_task->setTransactionId(i + 1000);
        tasks.push_back(std::move(sql_task));
    }
    
    // 创建WAL任务
    for (int i = 0; i < 5; ++i) {
        std::string task_id = "wal_task_" + std::to_string(i);
        std::string log_data = "Transaction log entry " + std::to_string(i);
        auto wal_task = std::make_unique<WALTask>(task_id, log_data);
        wal_task->setFlushRequired(i % 2 == 0); // 偶数任务需要刷新
        tasks.push_back(std::move(wal_task));
    }
    
    // 创建事务任务
    for (int i = 0; i < 5; ++i) {
        std::string task_id = "txn_task_" + std::to_string(i);
        TransactionTask::Operation op;
        switch (i % 3) {
            case 0: op = TransactionTask::BEGIN; break;
            case 1: op = TransactionTask::COMMIT; break;
            case 2: op = TransactionTask::ROLLBACK; break;
        }
        tasks.push_back(std::make_unique<TransactionTask>(task_id, 2000 + i, op));
    }
    
    std::cout << "Created " << tasks.size() << " tasks" << std::endl;
    
    // 提交所有任务
    for (auto& task : tasks) {
        executor.submitTask(std::move(task));
    }
    
    std::cout << "Submitted all tasks, pending count: " << executor.getPendingTaskCount() << std::endl;
    
    // 等待一段时间让任务执行完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "After execution, pending count: " << executor.getPendingTaskCount() << std::endl;
    
    // 停止执行器
    executor.stop();
    
    std::cout << "TaskExecutor test completed successfully!" << std::endl;
    
    return 0;
}