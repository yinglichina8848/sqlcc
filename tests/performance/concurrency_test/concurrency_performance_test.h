#pragma once

#include "../performance_test_base.h"
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <random>
#include <memory>
#include "../../include/buffer_pool.h"
#include "../../include/disk_manager.h"
#include "../../include/config_manager.h"

namespace sqlcc {
namespace test {

// 简单的屏障实现，因为有些编译器不支持std::barrier
class SimpleBarrier {
public:
    explicit SimpleBarrier(int count) : count_(count), current_(0) {}
    
    void Wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        int current = current_;
        if (++current == count_) {
            current_ = 0;
            condition_.notify_all();
        } else {
            while (current_ != 0) {
                condition_.wait(lock);
            }
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
    int current_;
};

/**
 * 并发性能测试类
 * 测试系统在并发访问下的性能表现
 */
class ConcurrencyPerformanceTest : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    ConcurrencyPerformanceTest();

    /**
     * 析构函数
     */
    ~ConcurrencyPerformanceTest() override;

    /**
     * 运行所有测试
     */
    void RunAllTests() override;

protected:
    /**
     * 并发读取测试
     */
    TestResult TestConcurrentReads();

    /**
     * 并发写入测试
     */
    TestResult TestConcurrentWrites();

    /**
     * 混合读写测试
     */
    TestResult TestMixedReadWrite();

    /**
     * 锁竞争测试
     */
    TestResult TestLockContention();

    /**
     * 模拟读取操作
     */
    void SimulateReadOperation(int thread_id, int operation_id);

    /**
     * 模拟写入操作
     */
    void SimulateWriteOperation(int thread_id, int operation_id);

    /**
     * 模拟锁竞争操作
     */
    void SimulateLockOperation(int thread_id, int operation_id, std::mutex& mutex);

    /**
     * 清理测试环境
     */
    void Cleanup() override;

private:
    /**
     * 读取工作线程函数
     */
    void ReadWorkerThread(int thread_id, size_t operations, 
                         std::atomic<size_t>& completed_ops,
                         std::vector<double>& latencies);

    /**
     * 写入工作线程函数
     */
    void WriteWorkerThread(int thread_id, size_t operations,
                          std::atomic<size_t>& completed_ops,
                          std::vector<double>& latencies);

    /**
     * 混合读写工作线程函数
     */
    void MixedWorkerThread(int thread_id, size_t operations,
                          std::atomic<size_t>& completed_ops,
                          std::vector<double>& latencies,
                          double read_ratio);

    /**
     * 锁竞争工作线程函数
     */
    void LockContentionWorkerThread(int thread_id, size_t operations,
                                   std::atomic<size_t>& completed_ops,
                                   std::vector<double>& latencies,
                                   std::mutex& mutex);

    /**
     * 生成测试数据
     */
    void GenerateTestData();

    /**
     * 执行页面读取操作
     */
    bool ExecutePageRead(int32_t page_id);

    /**
     * 执行页面写入操作
     */
    bool ExecutePageWrite(int32_t page_id, const std::string& data);

private:
    // 常量定义
    static constexpr size_t kDefaultThreadCount = 8;
    static constexpr size_t kOperationsPerThread = 1000;
    static constexpr size_t kDataSize = 10000;
    static constexpr size_t kWorkingSetSize = 100;
    static constexpr size_t kLockCount = 100;

    // 测试环境
    std::unique_ptr<sqlcc::BufferPool> buffer_pool_;
    std::unique_ptr<sqlcc::DiskManager> disk_manager_;
    std::string test_db_file_ = "./test_concurrency.db";

    // 测试数据
    std::vector<int32_t> test_data_;
    std::vector<std::string> string_data_;
    std::vector<std::unique_ptr<std::mutex>> lock_table_;

    // 同步原语
    SimpleBarrier* start_barrier_;
    std::atomic<bool> test_running_;

    // 随机数生成器
    std::mt19937 rng_;
    std::uniform_int_distribution<int32_t> page_id_dist_;
    std::uniform_real_distribution<double> random_dist_;
};

}  // namespace test
}  // namespace sqlcc