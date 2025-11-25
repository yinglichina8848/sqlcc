#pragma once

#include "performance_test_base.h"
#include "sql_executor.h"
#include "logger.h"
#include <vector>
#include <string>
#include <iomanip>
#include <random>
#include <thread>
#include <atomic>
#include <future>
#include <fstream>
#include <sys/stat.h>
#include <memory>

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
    
    /**
     * 初始化测试环境和表结构
     */
    // 已在后面实现，删除重复声明

private:
    /**
     * 运行单线程100万INSERT测试
     */
    void RunSingleThreadTest() {
        std::stringstream info_ss;
        info_ss << "Starting single-thread insert test with real SQL execution...";
        SQLCC_LOG_INFO(info_ss.str());
        
        // 重置计数器
        records_inserted_ = 0;
        
        // 记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 执行插入操作
        const int kInsertCount = 100000;  // 插入10万条记录
        std::vector<double> latencies;
        std::atomic<size_t> completed_ops(0);
        
        // 为单线程测试创建配置
        InsertTestConfig config;
        config.name = "SingleThreadConfig";
        config.insert_count = kInsertCount;
        config.thread_count = 1;
        config.record_size = 250;  // 平均250字节记录
        
        WorkerThread(0, config, latencies, completed_ops);
        
        // 记录结束时间
        auto end_time = std::chrono::high_resolution_clock::now();
        
        // 计算总时间（秒）
        double total_time_s = std::chrono::duration<double>
                            (end_time - start_time).count();
        
        // 计算QPS（每秒查询数）
        double qps = kInsertCount / total_time_s;
        
        // 计算平均延迟
        double avg_latency = 0;
        for (double l : latencies) {
            avg_latency += l;
        }
        avg_latency /= latencies.size();
        
        std::stringstream info_ss_end;
        info_ss_end << "Single-thread insert test completed:";
        SQLCC_LOG_INFO(info_ss_end.str());
        std::stringstream info_ss1;
        info_ss1 << "  - Total time: " << std::fixed << std::setprecision(2) << total_time_s << " seconds";
        SQLCC_LOG_INFO(info_ss1.str());
        
        std::stringstream info_ss2;
        info_ss2 << "  - Records inserted: " << records_inserted_;
        SQLCC_LOG_INFO(info_ss2.str());
        
        std::stringstream info_ss3;
        info_ss3 << "  - QPS: " << std::fixed << std::setprecision(2) << qps;
        SQLCC_LOG_INFO(info_ss3.str());
        
        std::stringstream info_ss4;
        info_ss4 << "  - Average latency: " << std::fixed << std::setprecision(4) << avg_latency << " ms";
        SQLCC_LOG_INFO(info_ss4.str());
        
        // 保存测试结果
        TestResult result;
        result.test_name = "SingleThreadInsert";
        // 根据PerformanceTestBase::TestResult的实际成员修改
        // 移除不存在的成员赋值
        result.avg_latency = avg_latency;
        result.throughput = qps;
        test_results_.push_back(result);
    }

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
                 std::atomic<size_t>& operations_completed) {
    std::stringstream info_ss;
    info_ss << "Worker thread " << thread_id << " started: config.name = " << config.name.c_str() << ", record_size = " << config.record_size;
    SQLCC_LOG_INFO(info_ss.str());
    
    // 为每个线程创建独立的随机数生成器
    std::mt19937 thread_rng(rng_() + thread_id);
    
    // 计算每个线程需要插入的记录数
    size_t records_per_thread = config.insert_count / config.thread_count;
    size_t start_id = thread_id * records_per_thread;
    size_t end_id = (thread_id == config.thread_count - 1) ? 
                    config.insert_count : (thread_id + 1) * records_per_thread;
    size_t thread_insert_count = end_id - start_id;
    
    // 记录成功插入的记录数
    size_t successful_inserts = 0;
    
    // 执行插入操作
    for (size_t i = 0; i < thread_insert_count; ++i) {
        // 生成记录ID
        size_t record_id = start_id + i;
        
        // 记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 执行真实SQL插入
        bool success = ExecuteRealRecordInsert(record_id, config.record_size);
        
        // 记录结束时间
        auto end_time = std::chrono::high_resolution_clock::now();
        
        // 计算延迟（毫秒）
        double latency_ms = std::chrono::duration<double, std::milli>
                          (end_time - start_time).count();
        
        // 记录延迟
        latencies.push_back(latency_ms);
        
        // 如果成功，增加计数
        if (success) {
            successful_inserts++;
        }
        
        // 增加已完成操作计数
        operations_completed++;
    }
    
    std::stringstream info_ss_end;
    info_ss_end << "Worker thread " << thread_id << " completed: " << successful_inserts << " successful inserts out of " << thread_insert_count << " attempts";
    SQLCC_LOG_INFO(info_ss_end.str());
}

    /**
     * 执行真实记录插入操作（SQL INSERT）
     */
    // 已在后面实现，删除重复声明
    
    // 记录插入计数器
    std::atomic<size_t> records_inserted_;
    
    /**
     * 初始化测试环境和表结构
     */
    void SetupTestEnvironment() {
        // 创建测试数据库目录
        mkdir(test_db_path_.c_str(), 0755);
        
        // 初始化SQL执行器
        sql_executor_ = std::make_unique<sqlcc::SqlExecutor>();
        
        // 创建测试数据库
        std::string create_db_sql = "CREATE DATABASE " + std::string(kTestDatabase);
        sql_executor_->Execute(create_db_sql);
        
        // 使用测试数据库
        std::string use_db_sql = "USE " + std::string(kTestDatabase);
        sql_executor_->Execute(use_db_sql);
        
        // 创建测试表
        std::string create_table_sql = "CREATE TABLE " + std::string(kTestTable) + 
                                      " (id INTEGER PRIMARY KEY, data TEXT)";
        sql_executor_->Execute(create_table_sql);
        
        // 重置记录ID
        next_record_id_ = 0;
        
        std::stringstream info_ss;
        info_ss << "Test environment set up: DB path = " << test_db_path_.c_str() << ", Database = " << kTestDatabase << ", Table = " << kTestTable;
        SQLCC_LOG_INFO(info_ss.str());
    }
    
    /**
     * 执行真实记录插入操作（SQL INSERT）
     */
    bool ExecuteRealRecordInsert(size_t record_id, size_t record_size) {
        try {
            // 生成随机数据
            std::string data(record_size, '0');
            for (size_t i = 0; i < record_size; ++i) {
                data[i] = 'A' + (rng_() % 26);
            }
            
            // 构建SQL插入语句
            std::string insert_sql = "INSERT INTO " + std::string(kTestTable) + 
                                    " (id, data) VALUES (" + 
                                    std::to_string(record_id) + ", '" + 
                                    data + "')";
            
            // 执行真实的SQL插入
            std::string result = sql_executor_->Execute(insert_sql);
            if (result.empty() || !sql_executor_->GetLastError().empty()) {
                std::stringstream error_ss;
                error_ss << "Failed to execute SQL insert for record " << record_id << ": " << sql_executor_->GetLastError().c_str();
                SQLCC_LOG_ERROR(error_ss.str());
                return false;
            }
            
            // 更新计数器
            records_inserted_++;
            
            return true;
        } catch (const std::exception& e) {
            std::stringstream error_ss;
            error_ss << "Exception during record insert: " << e.what();
            SQLCC_LOG_ERROR(error_ss.str());
            return false;
        }
    }

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
     * 初始化测试环境和表结构
     */
    // 已在前面实现，删除重复声明

    /**
     * 清理测试数据文件
     */
    void CleanupTestData() {
        // 如果SQL执行器存在，先删除测试数据库
        if (sql_executor_) {
            std::string drop_db_sql = "DROP DATABASE " + std::string(kTestDatabase);
            sql_executor_->Execute(drop_db_sql);
            std::stringstream info_ss;
            info_ss << "Test database dropped: " << kTestDatabase;
            SQLCC_LOG_INFO(info_ss.str());
        }
        
        // 删除测试数据库文件
        if (!test_db_path_.empty()) {
            std::string cmd = "rm -rf " + test_db_path_;
            system(cmd.c_str());
            std::stringstream info_ss;
            info_ss << "Test data cleaned up: " << test_db_path_.c_str();
            SQLCC_LOG_INFO(info_ss.str());
        }
    }

private:
    // 常量定义
    static constexpr const char* kTestDatabase = "million_insert_test_db";
    static constexpr const char* kTestTable = "insert_test_data";
    
    // SQL执行器
    std::unique_ptr<sqlcc::SqlExecutor> sql_executor_;
    
    // 测试环境状态
    std::string test_db_path_;      // 测试数据库文件路径
    std::atomic<size_t> next_record_id_;          // 下一个可用的记录ID
    
    // 随机数生成器
    std::mt19937 rng_;
    
    // 预定义的测试配置
    std::vector<InsertTestConfig> test_configs_;
    
    // 测试结果
    std::vector<TestResult> test_results_;
};

}  // namespace test
}  // namespace sqlcc