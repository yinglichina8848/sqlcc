#include "mixed_workload_test.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace sqlcc {
namespace test {

MixedWorkloadTest::MixedWorkloadTest() 
    : next_page_id_(0), rng_(std::random_device{}()) {
    
    // 初始化预定义的工作负载配置
    
    // 读写比例测试配置
    workload_configs_.push_back({0.9, 0.1, 0.0, 0.0, 1, 10000, 1, 1000, "ReadHeavy_90_10"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 1, 10000, 1, 1000, "ReadWrite_70_30"});
    workload_configs_.push_back({0.5, 0.5, 0.0, 0.0, 1, 10000, 1, 1000, "Balanced_50_50"});
    workload_configs_.push_back({0.3, 0.7, 0.0, 0.0, 1, 10000, 1, 1000, "WriteHeavy_30_70"});
    
    // 事务大小测试配置
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 1, 10000, 1, 1000, "TransactionSize_1"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 10000, 1, 1000, "TransactionSize_5"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 10, 10000, 1, 1000, "TransactionSize_10"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 20, 10000, 1, 1000, "TransactionSize_20"});
    
    // 长时间运行测试配置
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 60000, 1, 1000, "LongRunning_1min"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 300000, 1, 1000, "LongRunning_5min"});
    
    // 并发测试配置
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 10000, 1, 1000, "Concurrent_1thread"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 10000, 2, 1000, "Concurrent_2threads"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 10000, 4, 1000, "Concurrent_4threads"});
    workload_configs_.push_back({0.7, 0.3, 0.0, 0.0, 5, 10000, 8, 1000, "Concurrent_8threads"});
}

MixedWorkloadTest::~MixedWorkloadTest() {
    Cleanup();
}

void MixedWorkloadTest::RunAllTests() {
    std::cout << "\n===== Running Mixed Workload Performance Tests =====" << std::endl;
    
    RunReadWriteRatioTest();
    RunTransactionSizeTest();
    RunLongRunningStabilityTest();
    RunConcurrentWorkloadTest();
    
    // 生成综合报告
    GenerateReport(test_results_);
    
    std::cout << "\n===== All Mixed Workload Performance Tests Completed =====" << std::endl;
}

void MixedWorkloadTest::Cleanup() {
    existing_pages_.clear();
    page_dirty_flags_.clear();
    next_page_id_ = 0;
    test_results_.clear();
}

void MixedWorkloadTest::RunReadWriteRatioTest() {
    std::cout << "\n--- Running Read/Write Ratio Test ---" << std::endl;
    
    // 测试不同的读写比例
    std::vector<WorkloadConfig> rw_configs = {
        {0.9, 0.1, 0.0, 0.0, 1, 10000, 1, 1000, "ReadHeavy_90_10"},
        {0.7, 0.3, 0.0, 0.0, 1, 10000, 1, 1000, "ReadWrite_70_30"},
        {0.5, 0.5, 0.0, 0.0, 1, 10000, 1, 1000, "Balanced_50_50"},
        {0.3, 0.7, 0.0, 0.0, 1, 10000, 1, 1000, "WriteHeavy_30_70"}
    };
    
    for (const auto& config : rw_configs) {
        ExecuteWorkload(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "mixed_workload_read_write_ratio.csv");
}

void MixedWorkloadTest::RunTransactionSizeTest() {
    std::cout << "\n--- Running Transaction Size Test ---" << std::endl;
    
    // 测试不同的事务大小
    std::vector<WorkloadConfig> tx_configs = {
        {0.7, 0.3, 0.0, 0.0, 1, 10000, 1, 1000, "TransactionSize_1"},
        {0.7, 0.3, 0.0, 0.0, 5, 10000, 1, 1000, "TransactionSize_5"},
        {0.7, 0.3, 0.0, 0.0, 10, 10000, 1, 1000, "TransactionSize_10"},
        {0.7, 0.3, 0.0, 0.0, 20, 10000, 1, 1000, "TransactionSize_20"}
    };
    
    for (const auto& config : tx_configs) {
        ExecuteWorkload(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "mixed_workload_transaction_size.csv");
}

void MixedWorkloadTest::RunLongRunningStabilityTest() {
    std::cout << "\n--- Running Long Running Stability Test ---" << std::endl;
    
    // 测试长时间运行的稳定性
    std::vector<WorkloadConfig> long_configs = {
        {0.7, 0.3, 0.0, 0.0, 5, 60000, 1, 1000, "LongRunning_1min"},
        {0.7, 0.3, 0.0, 0.0, 5, 300000, 1, 1000, "LongRunning_5min"}
    };
    
    for (const auto& config : long_configs) {
        ExecuteWorkload(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "mixed_workload_long_running.csv");
}

void MixedWorkloadTest::RunConcurrentWorkloadTest() {
    std::cout << "\n--- Running Concurrent Workload Test ---" << std::endl;
    
    // 测试不同线程数下的并发性能
    std::vector<WorkloadConfig> concurrent_configs = {
        {0.7, 0.3, 0.0, 0.0, 5, 10000, 1, 1000, "Concurrent_1thread"},
        {0.7, 0.3, 0.0, 0.0, 5, 10000, 2, 1000, "Concurrent_2threads"},
        {0.7, 0.3, 0.0, 0.0, 5, 10000, 4, 1000, "Concurrent_4threads"},
        {0.7, 0.3, 0.0, 0.0, 5, 10000, 8, 1000, "Concurrent_8threads"}
    };
    
    for (const auto& config : concurrent_configs) {
        ExecuteWorkload(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "mixed_workload_concurrent.csv");
}

void MixedWorkloadTest::ExecuteWorkload(const WorkloadConfig& config) {
    std::cout << "Running workload: " << config.name << std::endl;
    
    // 初始化测试环境
    SetupTestEnvironment(config.working_set_size);
    
    // 生成操作序列
    std::vector<OperationType> operations;
    GenerateOperationSequence(config, operations);
    
    // 启动工作线程
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(config.thread_count);
    std::atomic<size_t> total_operations(0);
    
    auto start_time = GetCurrentTime();
    
    // 创建并启动线程
    for (size_t i = 0; i < config.thread_count; ++i) {
        threads.emplace_back([this, i, &config, &thread_latencies, &total_operations]() {
            WorkerThread(i, config, thread_latencies[i], total_operations);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = GetCurrentTime();
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    TestResult result;
    result.test_name = config.name;
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Read Ratio"] = std::to_string(config.read_ratio * 100) + "%";
    result.custom_metrics["Write Ratio"] = std::to_string(config.write_ratio * 100) + "%";
    result.custom_metrics["Transaction Size"] = std::to_string(config.transaction_size);
    result.custom_metrics["Thread Count"] = std::to_string(config.thread_count);
    result.custom_metrics["Working Set Size"] = std::to_string(config.working_set_size);
    
    test_results_.push_back(result);
    PrintResult(result);
}

void MixedWorkloadTest::WorkerThread(size_t thread_id, const WorkloadConfig& config, 
                                     std::vector<double>& latencies, 
                                     std::atomic<size_t>& operations_completed) {
    // 生成操作序列
    std::vector<OperationType> operations;
    GenerateOperationSequence(config, operations);
    
    // 计算每个线程应该执行的操作数
    size_t ops_per_thread = operations.size() / config.thread_count;
    size_t start_op = thread_id * ops_per_thread;
    size_t end_op = (thread_id == config.thread_count - 1) ? operations.size() : start_op + ops_per_thread;
    
    // 执行操作
    for (size_t i = start_op; i < end_op; ++i) {
        auto op_start = GetCurrentTime();
        bool success = ExecuteOperation(operations[i], thread_id);
        auto op_end = GetCurrentTime();
        
        if (success) {
            auto duration = CalculateDuration(op_start, op_end);
            latencies.push_back(static_cast<double>(duration.count()));
            operations_completed.fetch_add(1);
        }
    }
}

void MixedWorkloadTest::GenerateOperationSequence(const WorkloadConfig& config, 
                                                  std::vector<OperationType>& operations) {
    operations.clear();
    
    // 计算总操作数
    size_t total_ops = static_cast<size_t>(config.duration_ms / 10);  // 假设每个操作平均10ms
    
    // 计算每种操作的数量
    size_t read_ops = static_cast<size_t>(total_ops * config.read_ratio);
    size_t write_ops = static_cast<size_t>(total_ops * config.write_ratio);
    size_t create_ops = static_cast<size_t>(total_ops * config.create_ratio);
    size_t delete_ops = static_cast<size_t>(total_ops * config.delete_ratio);
    
    // 填充操作序列
    operations.reserve(total_ops);
    
    for (size_t i = 0; i < read_ops; ++i) {
        operations.push_back(OperationType::READ);
    }
    
    for (size_t i = 0; i < write_ops; ++i) {
        operations.push_back(OperationType::WRITE);
    }
    
    for (size_t i = 0; i < create_ops; ++i) {
        operations.push_back(OperationType::CREATE);
    }
    
    for (size_t i = 0; i < delete_ops; ++i) {
        operations.push_back(OperationType::DELETE);
    }
    
    // 随机打乱操作序列
    std::shuffle(operations.begin(), operations.end(), rng_);
}

bool MixedWorkloadTest::ExecuteOperation(OperationType op, size_t thread_id) {
    (void)thread_id; // 避免未使用参数警告
    switch (op) {
        case OperationType::READ:
            if (existing_pages_.empty()) {
                return false;
            }
            {
                std::uniform_int_distribution<size_t> dist(0, existing_pages_.size() - 1);
                int32_t page_id = existing_pages_[dist(rng_)];
                return SimulatePageRead(page_id);
            }
            
        case OperationType::WRITE:
            if (existing_pages_.empty()) {
                return false;
            }
            {
                std::uniform_int_distribution<size_t> dist(0, existing_pages_.size() - 1);
                int32_t page_id = existing_pages_[dist(rng_)];
                return SimulatePageWrite(page_id);
            }
            
        case OperationType::CREATE:
            return SimulatePageCreate();
            
        case OperationType::DELETE:
            if (existing_pages_.empty()) {
                return false;
            }
            {
                std::uniform_int_distribution<size_t> dist(0, existing_pages_.size() - 1);
                size_t index = dist(rng_);
                int32_t page_id = existing_pages_[index];
                existing_pages_.erase(existing_pages_.begin() + index);
                return SimulatePageDelete(page_id);
            }
    }
    
    return false;
}

bool MixedWorkloadTest::SimulatePageRead(int32_t page_id) {
    (void)page_id; // 避免未使用参数警告
    
    // 模拟页面读取操作
    // 在实际实现中，这里会调用存储引擎的页面读取接口
    // 这里我们只是模拟延迟
    
    // 模拟读取延迟（1-5ms）
    std::uniform_int_distribution<int> dist(1, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_)));
    
    return true;
}

bool MixedWorkloadTest::SimulatePageWrite(int32_t page_id) {
    // 模拟页面写入操作
    // 在实际实现中，这里会调用存储引擎的页面写入接口
    // 这里我们只是模拟延迟
    
    // 模拟写入延迟（2-8ms）
    std::uniform_int_distribution<int> dist(2, 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_)));
    
    // 更新脏标记
    auto it = std::find(existing_pages_.begin(), existing_pages_.end(), page_id);
    if (it != existing_pages_.end()) {
        size_t index = std::distance(existing_pages_.begin(), it);
        if (index < page_dirty_flags_.size()) {
            page_dirty_flags_[index] = true;
        }
    }
    
    return true;
}

bool MixedWorkloadTest::SimulatePageCreate() {
    // 模拟页面创建操作
    // 在实际实现中，这里会调用存储引擎的页面创建接口
    // 这里我们只是模拟延迟
    
    // 模拟创建延迟（5-15ms）
    std::uniform_int_distribution<int> dist(5, 15);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_)));
    
    // 添加新页面到现有页面列表
    int32_t new_page_id = next_page_id_++;
    existing_pages_.push_back(new_page_id);
    page_dirty_flags_.push_back(true);  // 新创建的页面是脏的
    
    return true;
}

bool MixedWorkloadTest::SimulatePageDelete(int32_t page_id) {
    (void)page_id; // 避免未使用参数警告
    
    // 模拟页面删除操作
    // 在实际实现中，这里会调用存储引擎的页面删除接口
    // 这里我们只是模拟延迟
    
    // 模拟删除延迟（3-10ms）
    std::uniform_int_distribution<int> dist(3, 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng_)));
    
    return true;
}

void MixedWorkloadTest::SetupTestEnvironment(size_t working_set_size) {
    // 清理现有状态
    existing_pages_.clear();
    page_dirty_flags_.clear();
    next_page_id_ = 0;
    
    // 创建初始工作集
    for (size_t i = 0; i < working_set_size; ++i) {
        existing_pages_.push_back(next_page_id_++);
        page_dirty_flags_.push_back(false);
    }
}

}  // namespace test
}  // namespace sqlcc