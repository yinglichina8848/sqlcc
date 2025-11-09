#pragma once

#include "performance_test_base.h"
#include <vector>
#include <random>
#include <memory>
#include "../../include/buffer_pool.h"
#include "../../include/disk_manager.h"

namespace sqlcc {
namespace test {

/**
 * 批量读取和预取机制性能测试类
 * 测试批量读取和预取机制的性能表现
 */
class BatchPrefetchPerformanceTest : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    BatchPrefetchPerformanceTest();

    /**
     * 析构函数
     */
    ~BatchPrefetchPerformanceTest() override;

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
     * 运行单个页面读取性能测试
     */
    void RunSinglePageReadTest();

    /**
     * 运行批量页面读取性能测试
     */
    void RunBatchPageReadTest();

    /**
     * 运行单页预取性能测试
     */
    void RunSinglePagePrefetchTest();

    /**
     * 运行批量预取性能测试
     */
    void RunBatchPrefetchTest();

    /**
     * 运行混合访问模式测试
     */
    void RunMixedAccessPatternTest();

    /**
     * 运行不同批量大小性能测试
     */
    void RunVaryingBatchSizeTest();

    /**
     * 设置测试环境
     */
    void SetupTestEnvironment();

    /**
     * 生成顺序访问序列
     */
    void GenerateSequentialAccess(std::vector<int32_t>& page_ids, size_t count);

    /**
     * 生成随机访问序列
     */
    void GenerateRandomAccess(std::vector<int32_t>& page_ids, size_t count, size_t max_page_id);

    /**
     * 生成具有局部性的访问序列
     */
    void GenerateLocalityAccess(std::vector<int32_t>& page_ids, size_t count, size_t working_set);

    /**
     * 执行单个页面访问操作并记录延迟
     */
    void ExecuteSinglePageAccesses(const std::vector<int32_t>& page_ids, 
                                   std::vector<double>& latencies);

    /**
     * 执行批量页面访问操作并记录延迟
     */
    void ExecuteBatchPageAccesses(const std::vector<int32_t>& page_ids, 
                                  std::vector<double>& latencies, 
                                  size_t batch_size);

    /**
     * 执行预取操作并记录延迟
     */
    void ExecutePrefetchOperations(const std::vector<int32_t>& page_ids, 
                                   std::vector<double>& latencies);

    /**
     * 执行批量预取操作并记录延迟
     */
    void ExecuteBatchPrefetchOperations(const std::vector<int32_t>& page_ids, 
                                       std::vector<double>& latencies, 
                                       size_t batch_size);

private:
    // 测试环境
    std::unique_ptr<sqlcc::BufferPool> buffer_pool_;
    std::unique_ptr<sqlcc::DiskManager> disk_manager_;
    std::string test_db_file_ = "./test_batch_prefetch.db";
    
    // 测试参数
    size_t pool_size_ = 128;  // 缓冲池大小
    size_t access_count_ = 10000;  // 每次测试的访问次数
    size_t working_set_size_ = 1000;  // 工作集大小
    std::vector<size_t> batch_sizes_ = {1, 4, 8, 16, 32, 64};  // 不同的批量大小
    
    // 随机数生成器
    std::mt19937 rng_;
};

}  // namespace test
}  // namespace sqlcc