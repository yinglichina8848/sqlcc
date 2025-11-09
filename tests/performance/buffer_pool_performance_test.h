#pragma once

#include "performance_test_base.h"
#include <vector>
#include <random>
#include <memory>

namespace sqlcc {
namespace test {

/**
 * 缓冲池性能测试类
 * 测试不同缓冲池大小和替换策略下的性能表现
 */
class BufferPoolPerformanceTest : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    BufferPoolPerformanceTest();

    /**
     * 析构函数
     */
    ~BufferPoolPerformanceTest() override;

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
     * 运行缓存命中率测试
     */
    void RunCacheHitRateTest();

    /**
     * 运行LRU效率测试
     */
    void RunLRUEfficiencyTest();

    /**
     * 运行访问模式测试
     */
    void RunAccessPatternTest();

    /**
     * 运行缓冲池大小扩展性测试
     */
    void RunPoolSizeScalabilityTest();

    /**
     * 设置指定大小的缓冲池
     */
    void SetupBufferPool(size_t pool_size);

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
     * 执行页面访问操作并记录延迟
     */
    void ExecutePageAccesses(const std::vector<int32_t>& page_ids, 
                             std::vector<double>& latencies, 
                             size_t& hit_count);

    /**
     * 计算缓存命中率
     */
    double CalculateHitRate(size_t hit_count, size_t total_accesses) const;

    /**
     * 模拟页面访问
     */
    bool SimulatePageAccess(int32_t page_id);

private:
    // 缓冲池相关成员
    size_t current_pool_size_;
    std::vector<int32_t> buffer_pool_;
    std::vector<bool> dirty_flags_;
    std::vector<std::chrono::high_resolution_clock::time_point> access_times_;
    
    // LRU相关成员
    std::vector<int32_t> lru_list_;
    
    // 测试参数
    std::vector<size_t> pool_sizes_ = {32, 64, 128, 256};  // 不同的缓冲池大小
    size_t access_count_ = 10000;  // 每次测试的访问次数
    size_t working_set_size_ = 1000;  // 工作集大小
    
    // 随机数生成器
    std::mt19937 rng_;
};

}  // namespace test
}  // namespace sqlcc