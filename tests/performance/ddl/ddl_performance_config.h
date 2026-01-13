#ifndef DDL_PERFORMANCE_CONFIG_H
#define DDL_PERFORMANCE_CONFIG_H

#include <string>
#include <vector>
#include <chrono>

// DDL性能测试配置
struct DDLPerformanceConfig {
    // 数据库连接配置
    std::string db_host = "localhost";
    std::string db_port = "3306";
    std::string db_user = "test_user";
    std::string db_password = "test_password";
    std::string db_name = "ddl_performance_test";

    // 测试参数配置
    int warmup_iterations = 5;           // 预热迭代次数
    int measurement_iterations = 10;     // 测量迭代次数
    std::chrono::milliseconds operation_timeout = std::chrono::milliseconds(30000); // 操作超时时间

    // 并发测试配置
    std::vector<int> thread_counts = {1, 2, 4, 8, 16, 32}; // 测试的线程数
    int max_concurrent_operations = 100;  // 最大并发操作数

    // 数据规模配置
    std::vector<size_t> table_sizes = {1000, 10000, 100000, 1000000}; // 测试表大小

    // 性能阈值配置
    double max_p95_latency_ms = 100.0;   // P95延迟最大值
    double min_throughput_ops_sec = 10.0; // 最小吞吐量
    double max_cpu_usage_percent = 80.0; // CPU使用率上限
    double max_memory_usage_mb = 1024.0; // 内存使用上限(MB)

    // 输出配置
    std::string output_directory = "tests/performance/ddl/results";
    std::string log_directory = "tests/performance/ddl/logs";
    bool enable_detailed_logging = true;
    bool generate_html_report = true;
    bool export_csv_data = true;
};

// DDL操作类型枚举
enum class DDLOperationType {
    CREATE_TABLE,
    ALTER_TABLE_ADD_COLUMN,
    ALTER_TABLE_DROP_COLUMN,
    ALTER_TABLE_MODIFY_COLUMN,
    CREATE_INDEX,
    DROP_INDEX,
    CREATE_VIEW,
    DROP_VIEW,
    CREATE_DATABASE,
    DROP_DATABASE
};

// DDL操作配置
struct DDLOperationConfig {
    DDLOperationType operation_type;
    std::string operation_name;
    std::string sql_template;
    bool requires_cleanup;
    std::string cleanup_sql;
};

// 性能指标数据结构
struct PerformanceMetrics {
    // 时间指标
    double average_latency_ms = 0.0;
    double p50_latency_ms = 0.0;
    double p95_latency_ms = 0.0;
    double p99_latency_ms = 0.0;
    double min_latency_ms = 0.0;
    double max_latency_ms = 0.0;

    // 吞吐量指标
    double operations_per_second = 0.0;
    int total_operations = 0;
    int successful_operations = 0;
    int failed_operations = 0;

    // 资源消耗指标
    double average_cpu_usage_percent = 0.0;
    double peak_cpu_usage_percent = 0.0;
    double average_memory_usage_mb = 0.0;
    double peak_memory_usage_mb = 0.0;
    size_t disk_read_bytes = 0;
    size_t disk_write_bytes = 0;

    // 测试信息
    std::chrono::system_clock::time_point test_start_time;
    std::chrono::system_clock::time_point test_end_time;
    int thread_count = 1;
    size_t data_size = 0;
    DDLOperationType operation_type;

    // 计算派生指标
    void calculateDerivedMetrics();
    bool meetsPerformanceThresholds(const DDLPerformanceConfig& config) const;
    std::string toJsonString() const;
    std::string toCsvString() const;
};

// 并发测试配置
struct ConcurrentTestConfig {
    int thread_count;
    int operations_per_thread;
    std::chrono::milliseconds operation_interval;
    bool enable_resource_monitoring;
    std::string test_scenario_name;
};

// 测试场景配置
struct TestScenario {
    std::string scenario_name;
    DDLOperationType operation_type;
    size_t data_size;
    ConcurrentTestConfig concurrency_config;
    std::vector<std::string> setup_sql;
    std::vector<std::string> cleanup_sql;
};

// 测试结果汇总
struct TestResultSummary {
    std::string test_suite_name;
    std::chrono::system_clock::time_point test_start_time;
    std::chrono::system_clock::time_point test_end_time;
    std::vector<PerformanceMetrics> all_metrics;

    // 汇总统计
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    double average_throughput = 0.0;
    double max_latency_p95 = 0.0;

    // 生成报告
    void generateSummaryReport(const std::string& output_path);
    void generateDetailedReport(const std::string& output_path);
    void exportToCsv(const std::string& output_path);
};

#endif // DDL_PERFORMANCE_CONFIG_H
