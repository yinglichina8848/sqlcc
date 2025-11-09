#pragma once

#include "performance_test_base.h"
#include <vector>
#include <string>
#include <fstream>
#include <random>

namespace sqlcc {
namespace test {

/**
 * 磁盘I/O性能测试类
 * 评估磁盘管理器的I/O性能
 */
class DiskIOPerformanceTest : public PerformanceTestBase {
public:
    /**
     * 构造函数
     */
    DiskIOPerformanceTest();

    /**
     * 析构函数
     */
    ~DiskIOPerformanceTest() override;

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
     * 运行顺序读写测试
     */
    void RunSequentialReadWriteTest();

    /**
     * 运行随机读写测试
     */
    void RunRandomReadWriteTest();

    /**
     * 运行不同页面大小的测试
     */
    void RunVaryingPageSizeTest();

    /**
     * 运行并发I/O测试
     */
    void RunConcurrentIOTest();

    /**
     * 准备测试文件
     */
    void PrepareTestFile(size_t file_size_mb);

    /**
     * 清理测试文件
     */
    void CleanupTestFile();

    /**
     * 模拟页面读取操作
     */
    bool SimulatePageRead(int32_t page_id, char* buffer, size_t page_size);

    /**
     * 模拟页面写入操作
     */
    bool SimulatePageWrite(int32_t page_id, const char* buffer, size_t page_size);

    /**
     * 执行顺序读取操作
     */
    void ExecuteSequentialReads(size_t page_count, size_t page_size, 
                                std::vector<double>& latencies);

    /**
     * 执行顺序写入操作
     */
    void ExecuteSequentialWrites(size_t page_count, size_t page_size, 
                                 std::vector<double>& latencies);

    /**
     * 执行随机读取操作
     */
    void ExecuteRandomReads(size_t page_count, size_t page_size, 
                            std::vector<double>& latencies);

    /**
     * 执行随机写入操作
     */
    void ExecuteRandomWrites(size_t page_count, size_t page_size, 
                            std::vector<double>& latencies);

private:
    std::string test_file_path_;  // 测试文件路径
    size_t test_file_size_mb_;    // 测试文件大小（MB）
    std::vector<size_t> page_sizes_;  // 不同的页面大小（字节）
    size_t page_count_;           // 页面数量
    std::vector<char> test_data_;  // 测试数据
    
    // 随机数生成器
    std::mt19937 rng_;
};

}  // namespace test
}  // namespace sqlcc