#pragma once

#include "performance_test_base.h"
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <future>

namespace sqlcc {
namespace test {

/**
 * 混合工作负载性能测试类
 * 模拟真实数据库工作负载，评估系统整体性能
 */
class MixedWorkloadTest : public PerformanceTestBase {
public:
    /**
     * 工作负载配置
     */
    struct WorkloadConfig {
        double read_ratio;          // 读操作比例 (0.0-1.0)
        double write_ratio;         // 写操作比例 (0.0-1.0)
        double create_ratio;        // 创建操作比例 (0.0-1.0)
        double delete_ratio;        // 删除操作比例 (0.0-1.0)
        size_t transaction_size;    // 事务大小（页面数）
        size_t duration_ms;         // 测试持续时间（毫秒）
        size_t thread_count;        // 线程数
        size_t working_set_size;    // 工作集大小（页面数）
        std::string name;           // 测试名称
    };

    /**
     * 事务操作类型
     */
    enum class OperationType {
        READ,
        WRITE,
        CREATE,
        DELETE
    };

    /**
     * 构造函数
     */
    MixedWorkloadTest();

    /**
     * 析构函数
     */
    ~MixedWorkloadTest() override;

    /**
     * 运行所有测试
     */
    void RunAllTests() override;

protected:
    /**
     * 清理测试环境
     */
    void Cleanup() override;

private:
    /**
     * 运行读写比例测试
     */
    void RunReadWriteRatioTest();

    /**
     * 运行事务大小测试
     */
    void RunTransactionSizeTest();

    /**
     * 运行长时间稳定性测试
     */
    void RunLongRunningStabilityTest();

    /**
     * 运行并发工作负载测试
     */
    void RunConcurrentWorkloadTest();

    /**
     * 执行工作负载
     */
    void ExecuteWorkload(const WorkloadConfig& config);

    /**
     * 工作线程函数
     */
    void WorkerThread(size_t thread_id, const WorkloadConfig& config, 
                     std::vector<double>& latencies, 
                     std::atomic<size_t>& operations_completed);

    /**
     * 生成操作序列
     */
    void GenerateOperationSequence(const WorkloadConfig& config, 
                                 std::vector<OperationType>& operations);

    /**
     * 执行单个操作
     */
    bool ExecuteOperation(OperationType op, size_t thread_id);

    /**
     * 模拟页面读取操作
     */
    bool SimulatePageRead(int32_t page_id);

    /**
     * 模拟页面写入操作
     */
    bool SimulatePageWrite(int32_t page_id);

    /**
     * 模拟页面创建操作
     */
    bool SimulatePageCreate();

    /**
     * 模拟页面删除操作
     */
    bool SimulatePageDelete(int32_t page_id);

    /**
     * 初始化测试环境
     */
    void SetupTestEnvironment(size_t working_set_size);

private:
    // 测试环境状态
    std::vector<int32_t> existing_pages_;  // 存在的页面ID
    std::vector<bool> page_dirty_flags_;   // 页面脏标记
    size_t next_page_id_;                  // 下一个可用的页面ID
    
    // 随机数生成器
    std::mt19937 rng_;
    
    // 预定义的工作负载配置
    std::vector<WorkloadConfig> workload_configs_;
    
    // 测试结果
    std::vector<TestResult> test_results_;
};

}  // namespace test
}  // namespace sqlcc