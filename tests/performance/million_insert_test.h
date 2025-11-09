#pragma once

#include "performance_test_base.h"
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <future>
#include <fstream>
#include <sys/stat.h>

namespace sqlcc {
namespace test {

/**
 * 100万INSERT操作性能测试类
 * 测试存储引擎在大规模数据插入场景下的性能和磁盘文件膨胀率
 */
class MillionInsertTest : public PerformanceTestBase {
public:
    /**
     * INSERT测试配置
     */
    struct InsertTestConfig {
        size_t insert_count;       // 插入记录数
        size_t thread_count;       // 线程数
        size_t record_size;        // 记录大小（字节）
        bool measure_file_size;    // 是否测量文件大小
        std::string name;          // 测试名称
    };

    /**
     * 构造函数
     */
    MillionInsertTest();

    /**
     * 析构函数
     */
    ~MillionInsertTest() override;

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
     * 运行单线程100万INSERT测试
     */
    void RunSingleThreadTest();

    /**
     * 运行多线程100万INSERT测试
     */
    void RunMultiThreadTest();

    /**
     * 运行不同线程数的扩展性测试
     */
    void RunScalabilityTest();

    /**
     * 执行INSERT测试
     */
    void ExecuteInsertTest(const InsertTestConfig& config);

    /**
     * 工作线程函数
     */
    void WorkerThread(size_t thread_id, const InsertTestConfig& config, 
                     std::vector<double>& latencies, 
                     std::atomic<size_t>& operations_completed);

    /**
     * 模拟记录插入操作
     */
    bool SimulateRecordInsert(size_t record_id, size_t record_size);

    /**
     * 获取数据库文件大小
     */
    size_t GetDatabaseFileSize();

    /**
     * 计算理论文件大小
     */
    size_t CalculateTheoreticalFileSize(size_t record_count, size_t record_size);

    /**
     * 计算文件膨胀率
     */
    double CalculateFileExpansionRatio(size_t actual_size, size_t theoretical_size);

    /**
     * 初始化测试环境
     */
    void SetupTestEnvironment();

    /**
     * 清理测试数据文件
     */
    void CleanupTestData();

private:
    // 测试环境状态
    std::string test_db_path_;      // 测试数据库文件路径
    size_t next_record_id_;          // 下一个可用的记录ID
    
    // 随机数生成器
    std::mt19937 rng_;
    
    // 预定义的测试配置
    std::vector<InsertTestConfig> test_configs_;
    
    // 测试结果
    std::vector<TestResult> test_results_;
};

}  // namespace test
}  // namespace sqlcc