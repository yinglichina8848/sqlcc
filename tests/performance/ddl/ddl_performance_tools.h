#ifndef DDL_PERFORMANCE_TOOLS_H
#define DDL_PERFORMANCE_TOOLS_H

#include "ddl_performance_config.h"
#include <functional>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

// 前向声明
class SqlExecutor;
class DatabaseManager;
class Logger;

// 性能测试工具类
class DDLPerformanceTools {
public:
    DDLPerformanceTools(const DDLPerformanceConfig& config);
    ~DDLPerformanceTools();

    // 性能测量工具
    template<typename Func>
    PerformanceMetrics measureExecutionTime(Func&& func, DDLOperationType operation_type, int thread_count = 1, size_t data_size = 0);

    // 并发测试工具
    void runConcurrentTest(int thread_count,
                          const std::function<void(int)>& task,
                          const std::string& scenario_name);

    // 资源监控工具
    ResourceMetrics monitorSystemResources();

    // 数据库连接管理
    bool setupDatabaseConnection();
    void closeDatabaseConnection();
    bool isDatabaseConnectionValid() const;

    // 测试数据管理
    bool createTestDatabase(const std::string& db_name);
    bool dropTestDatabase(const std::string& db_name);
    bool createTestTable(const std::string& table_name, size_t row_count);
    bool dropTestTable(const std::string& table_name);
    bool populateTestData(const std::string& table_name, size_t row_count);

    // DDL操作执行器
    bool executeDDL(const std::string& sql, DDLOperationType operation_type);
    bool executeDDLInTransaction(const std::string& sql, DDLOperationType operation_type);

    // 结果报告工具
    void generatePerformanceReport(const std::vector<PerformanceMetrics>& metrics,
                                 const std::string& report_path);
    void generateHtmlReport(const std::vector<PerformanceMetrics>& metrics,
                          const std::string& report_path);
    void exportCsvData(const std::vector<PerformanceMetrics>& metrics,
                     const std::string& csv_path);

    // 日志工具
    void logPerformanceData(const PerformanceMetrics& metrics);
    void logTestProgress(const std::string& message);
    void logTestError(const std::string& error_message);

private:
    DDLPerformanceConfig config_;
    std::unique_ptr<SqlExecutor> sql_executor_;
    std::unique_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<Logger> logger_;

    // 资源监控状态
    std::atomic<size_t> initial_memory_usage_;
    std::atomic<double> initial_cpu_usage_;
    std::chrono::steady_clock::time_point test_start_time_;

    // 并发控制
    std::mutex test_mutex_;
    std::condition_variable test_cv_;
    std::atomic<int> active_threads_;

    // 私有辅助方法
    void initializeResourceMonitoring();
    void collectSystemMetrics(ResourceMetrics& metrics);
    void calculatePercentiles(std::vector<double>& latencies,
                            PerformanceMetrics& metrics);
    std::string formatDuration(std::chrono::milliseconds duration);
    std::string generateReportHeader();
    std::string generateReportFooter(const TestResultSummary& summary);

    // 系统资源获取方法
    double getCurrentCpuUsage();
    size_t getCurrentMemoryUsage();
    size_t getDiskReadBytes();
    size_t getDiskWriteBytes();

    // 数据库操作辅助方法
    std::string generateCreateTableSql(const std::string& table_name, size_t row_count);
    std::string generateInsertSql(const std::string& table_name, size_t start_id, size_t count);
    std::string generateIndexSql(const std::string& table_name, const std::string& column_name);
    std::string generateAlterTableSql(const std::string& table_name, const std::string& operation);

    // 性能数据验证
    bool validatePerformanceMetrics(const PerformanceMetrics& metrics);
    bool checkPerformanceThresholds(const PerformanceMetrics& metrics);
};

// 并发测试执行器
class ConcurrentTestExecutor {
public:
    ConcurrentTestExecutor(int max_threads);
    ~ConcurrentTestExecutor();

    // 执行并发测试
    void executeConcurrentTasks(int thread_count,
                              const std::function<void(int)>& task,
                              const std::string& scenario_name);

    // 获取执行统计
    struct ExecutionStats {
        int total_threads;
        std::chrono::milliseconds total_duration;
        std::chrono::milliseconds avg_thread_duration;
        int completed_tasks;
        int failed_tasks;
    };

    ExecutionStats getExecutionStats() const;

private:
    int max_threads_;
    std::vector<std::thread> thread_pool_;
    std::mutex stats_mutex_;
    ExecutionStats stats_;

    void updateExecutionStats(int thread_id, std::chrono::milliseconds duration, bool success);
};

// 性能基准建立器
class PerformanceBaselineBuilder {
public:
    PerformanceBaselineBuilder(const DDLPerformanceConfig& config);

    // 建立性能基准
    bool establishBaseline(const std::vector<PerformanceMetrics>& metrics);

    // 验证性能回归
    bool checkPerformanceRegression(const PerformanceMetrics& current_metrics);

    // 加载基准数据
    bool loadBaselineData(const std::string& baseline_file);

    // 保存基准数据
    bool saveBaselineData(const std::string& baseline_file);

private:
    DDLPerformanceConfig config_;
    std::vector<PerformanceMetrics> baseline_metrics_;

    struct BaselineThresholds {
        double max_p95_latency_increase_percent;
        double max_throughput_decrease_percent;
        double max_cpu_increase_percent;
        double max_memory_increase_mb;
    };

    BaselineThresholds thresholds_;

    bool compareWithBaseline(const PerformanceMetrics& metrics);
    void updateBaseline(const PerformanceMetrics& metrics);
};

// 性能数据收集器
class PerformanceDataCollector {
public:
    PerformanceDataCollector(const DDLPerformanceConfig& config);

    // 开始数据收集
    void startCollection();

    // 停止数据收集
    void stopCollection();

    // 添加性能数据点
    void addDataPoint(const PerformanceMetrics& metrics);

    // 获取汇总统计
    TestResultSummary getSummaryStatistics();

    // 导出数据
    bool exportToJson(const std::string& file_path);
    bool exportToCsv(const std::string& file_path);

private:
    DDLPerformanceConfig config_;
    std::vector<PerformanceMetrics> collected_data_;
    std::mutex data_mutex_;
    std::atomic<bool> is_collecting_;
    std::chrono::system_clock::time_point collection_start_time_;

    void calculateSummaryStats(TestResultSummary& summary);
    void writeJsonReport(const std::string& file_path);
    void writeCsvReport(const std::string& file_path);
};

// 工具函数
namespace DDLPerformanceUtils {

    // 时间格式化
    std::string formatTimestamp(const std::chrono::system_clock::time_point& time);
    std::string formatDuration(std::chrono::milliseconds duration);

    // 数据格式化
    std::string formatLatency(double latency_ms);
    std::string formatThroughput(double ops_per_sec);
    std::string formatPercentage(double value);
    std::string formatDataSize(size_t bytes);

    // 文件操作
    bool ensureDirectoryExists(const std::string& directory);
    bool writeTextFile(const std::string& file_path, const std::string& content);
    std::string readTextFile(const std::string& file_path);

    // 统计计算
    double calculatePercentile(const std::vector<double>& values, double percentile);
    double calculateStandardDeviation(const std::vector<double>& values);
    double calculateAverage(const std::vector<double>& values);

    // 字符串操作
    std::string toLowerCase(const std::string& str);
    std::string trimWhitespace(const std::string& str);
    std::vector<std::string> splitString(const std::string& str, char delimiter);

    // DDL操作类型转换
    std::string ddlOperationTypeToString(DDLOperationType type);
    DDLOperationType stringToDDLOperationType(const std::string& str);

    // 性能状态判断
    bool isPerformanceWithinThresholds(const PerformanceMetrics& metrics,
                                     const DDLPerformanceConfig& config);
    std::string getPerformanceStatusString(bool within_thresholds);

} // namespace DDLPerformanceUtils

#endif // DDL_PERFORMANCE_TOOLS_H
