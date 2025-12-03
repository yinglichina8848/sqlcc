#include "crud_performance_test.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace sqlcc {
namespace test {

CRUDPerformanceTest::CRUDPerformanceTest() 
    : test_db_path_("/tmp/crud_performance_test"),
      next_record_id_(0),
      rng_(std::random_device{}()) {
    
    // 定义不同数据规模的测试配置
    test_configs_ = {
        {1000, 1, 250, "1K_Data", "1千行数据测试"},
        {10000, 1, 250, "10K_Data", "1万行数据测试"},
        {50000, 1, 250, "50K_Data", "5万行数据测试"},
        {100000, 1, 250, "100K_Data", "10万行数据测试"}
    };
    
    std::stringstream info_ss;
    info_ss << "CRUDPerformanceTest initialized with " << test_configs_.size() << " test configurations";
    SQLCC_LOG_INFO(info_ss.str());
}

CRUDPerformanceTest::~CRUDPerformanceTest() {
    Cleanup();
}

void CRUDPerformanceTest::RunAllTests() {
    std::stringstream info_ss;
    info_ss << "Starting CRUD performance tests with " << test_configs_.size() << " configurations";
    SQLCC_LOG_INFO(info_ss.str());
    
    for (const auto& config : test_configs_) {
        std::stringstream config_ss;
        config_ss << "Testing configuration: " << config.name << " (" << config.description << ")";
        SQLCC_LOG_INFO(config_ss.str());
        
        // 初始化测试环境
        SetupTestEnvironment();
        
        // 预填充测试数据
        PrepopulateTestData(config.data_size);
        
        // 运行各种CRUD操作测试
        RunInsertPerformanceTest(config);
        RunSelectPointPerformanceTest(config);
        RunSelectRangePerformanceTest(config);
        RunUpdatePerformanceTest(config);
        RunDeletePerformanceTest(config);
        
        // 清理当前测试环境
        Cleanup();
    }
    
    // 生成最终性能报告
    GeneratePerformanceReport(test_results_);
}

void CRUDPerformanceTest::Cleanup() {
    if (sql_executor_) {
        try {
            std::string drop_db_sql = "DROP DATABASE IF EXISTS " + std::string(kTestDatabase);
            sql_executor_->Execute(drop_db_sql);
            std::stringstream info_ss;
            info_ss << "Test database dropped: " << kTestDatabase;
            SQLCC_LOG_INFO(info_ss.str());
        } catch (const std::exception& e) {
            std::stringstream error_ss;
            error_ss << "Error dropping test database: " << e.what();
            SQLCC_LOG_ERROR(error_ss.str());
        }
    }
    
    // 删除测试数据库文件
    if (!test_db_path_.empty()) {
        std::string cmd = "rm -rf " + test_db_path_;
        system(cmd.c_str());
        std::stringstream info_ss;
        info_ss << "Test data cleaned up: " << test_db_path_.c_str();
        SQLCC_LOG_INFO(info_ss.str());
    }
    
    sql_executor_.reset();
}

void CRUDPerformanceTest::SetupTestEnvironment() {
    // 创建测试数据库目录
    mkdir(test_db_path_.c_str(), 0755);
    
    // 初始化SQL执行器
    sql_executor_ = std::make_unique<sqlcc::SqlExecutor>();
    
    // 创建测试数据库
    std::string create_db_sql = "CREATE DATABASE IF NOT EXISTS " + std::string(kTestDatabase);
    sql_executor_->Execute(create_db_sql);
    
    // 使用测试数据库
    std::string use_db_sql = "USE " + std::string(kTestDatabase);
    sql_executor_->Execute(use_db_sql);
    
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE IF NOT EXISTS " + std::string(kTestTable) + 
                                  " (id INTEGER PRIMARY KEY, name TEXT, age INTEGER, data TEXT)";
    sql_executor_->Execute(create_table_sql);
    
    // 重置记录ID
    next_record_id_ = 0;
    
    std::stringstream info_ss;
    info_ss << "Test environment set up: DB path = " << test_db_path_.c_str() 
            << ", Database = " << kTestDatabase << ", Table = " << kTestTable;
    SQLCC_LOG_INFO(info_ss.str());
}

void CRUDPerformanceTest::PrepopulateTestData(size_t data_size) {
    std::stringstream info_ss;
    info_ss << "Prepopulating " << data_size << " test records";
    SQLCC_LOG_INFO(info_ss.str());
    
    size_t batch_size = 1000; // 批量插入大小
    size_t completed = 0;
    
    for (size_t i = 0; i < data_size; i += batch_size) {
        size_t current_batch = std::min(batch_size, data_size - i);
        
        std::string batch_sql = "INSERT INTO " + std::string(kTestTable) + " (id, name, age, data) VALUES ";
        
        for (size_t j = 0; j < current_batch; j++) {
            size_t record_id = i + j;
            std::string name = "User_" + std::to_string(record_id);
            int age = 20 + (rng_() % 50);
            std::string data = GenerateRandomData(200);
            
            batch_sql += "(" + std::to_string(record_id) + ", '" + name + "', " + 
                         std::to_string(age) + ", '" + data + "')";
            
            if (j < current_batch - 1) {
                batch_sql += ", ";
            }
        }
        
        try {
            sql_executor_->Execute(batch_sql);
            completed += current_batch;
            
            if (completed % 10000 == 0) {
                std::stringstream progress_ss;
                progress_ss << "Prepopulated " << completed << " records";
                SQLCC_LOG_INFO(progress_ss.str());
            }
        } catch (const std::exception& e) {
            std::stringstream error_ss;
            error_ss << "Error during batch insert: " << e.what();
            SQLCC_LOG_ERROR(error_ss.str());
        }
    }
    
    next_record_id_ = data_size;
    
    std::stringstream complete_ss;
    complete_ss << "Prepopulation completed: " << completed << " records inserted";
    SQLCC_LOG_INFO(complete_ss.str());
}

void CRUDPerformanceTest::RunInsertPerformanceTest(const CRUDTestConfig& config) {
    std::stringstream info_ss;
    info_ss << "Starting INSERT performance test with " << config.data_size << " operations";
    SQLCC_LOG_INFO(info_ss.str());
    
    std::vector<double> latencies;
    size_t test_operations = std::min(config.data_size, static_cast<size_t>(1000)); // 测试1000次插入
    size_t successful_operations = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < test_operations; i++) {
        double latency_ms = 0;
        size_t record_id = next_record_id_.fetch_add(1);
        
        if (ExecuteInsertOperation(record_id, config.record_size, latency_ms)) {
            latencies.push_back(latency_ms);
            successful_operations++;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 计算性能指标
    double avg_latency = 0;
    if (!latencies.empty()) {
        for (double latency : latencies) {
            avg_latency += latency;
        }
        avg_latency /= latencies.size();
    }
    
    double throughput = (successful_operations * 1000.0) / duration.count();
    
    // 保存测试结果
    TestResult result;
    result.test_name = config.name + "_INSERT";
    result.avg_latency = avg_latency;
    result.throughput = throughput;
    result.operations_completed = successful_operations;
    result.duration = duration;
    
    // 验证性能要求
    bool requirement_met = VerifyPerformanceRequirement("INSERT", avg_latency);
    result.custom_metrics["requirement_met"] = requirement_met ? "YES" : "NO";
    result.custom_metrics["data_size"] = std::to_string(config.data_size);
    
    test_results_.push_back(result);
    
    std::stringstream result_ss;
    result_ss << "INSERT test completed: " << successful_operations << " operations, "
              << "avg latency = " << std::fixed << std::setprecision(3) << avg_latency << "ms, "
              << "throughput = " << std::fixed << std::setprecision(2) << throughput << " ops/sec, "
              << "requirement met = " << (requirement_met ? "YES" : "NO");
    SQLCC_LOG_INFO(result_ss.str());
}

void CRUDPerformanceTest::RunSelectPointPerformanceTest(const CRUDTestConfig& config) {
    std::stringstream info_ss;
    info_ss << "Starting SELECT point query performance test with " << config.data_size << " operations";
    SQLCC_LOG_INFO(info_ss.str());
    
    std::vector<double> latencies;
    size_t test_operations = std::min(config.data_size, static_cast<size_t>(1000));
    size_t successful_operations = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < test_operations; i++) {
        double latency_ms = 0;
        size_t record_id = rng_() % config.data_size; // 随机选择记录
        
        if (ExecuteSelectPointOperation(record_id, latency_ms)) {
            latencies.push_back(latency_ms);
            successful_operations++;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 计算性能指标
    double avg_latency = 0;
    if (!latencies.empty()) {
        for (double latency : latencies) {
            avg_latency += latency;
        }
        avg_latency /= latencies.size();
    }
    
    double throughput = (successful_operations * 1000.0) / duration.count();
    
    // 保存测试结果
    TestResult result;
    result.test_name = config.name + "_SELECT_POINT";
    result.avg_latency = avg_latency;
    result.throughput = throughput;
    result.operations_completed = successful_operations;
    result.duration = duration;
    
    // 验证性能要求
    bool requirement_met = VerifyPerformanceRequirement("SELECT_POINT", avg_latency);
    result.custom_metrics["requirement_met"] = requirement_met ? "YES" : "NO";
    result.custom_metrics["data_size"] = std::to_string(config.data_size);
    
    test_results_.push_back(result);
    
    std::stringstream result_ss;
    result_ss << "SELECT point test completed: " << successful_operations << " operations, "
              << "avg latency = " << std::fixed << std::setprecision(3) << avg_latency << "ms, "
              << "throughput = " << std::fixed << std::setprecision(2) << throughput << " ops/sec, "
              << "requirement met = " << (requirement_met ? "YES" : "NO");
    SQLCC_LOG_INFO(result_ss.str());
}

void CRUDPerformanceTest::RunSelectRangePerformanceTest(const CRUDTestConfig& config) {
    std::stringstream info_ss;
    info_ss << "Starting SELECT range query performance test with " << config.data_size << " operations";
    SQLCC_LOG_INFO(info_ss.str());
    
    std::vector<double> latencies;
    size_t test_operations = std::min(config.data_size / 10, static_cast<size_t>(100)); // 减少范围查询次数
    size_t successful_operations = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < test_operations; i++) {
        double latency_ms = 0;
        size_t start_id = rng_() % (config.data_size - 100);
        size_t end_id = start_id + 100; // 查询100条记录的范围
        
        if (ExecuteSelectRangeOperation(start_id, end_id, latency_ms)) {
            latencies.push_back(latency_ms);
            successful_operations++;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 计算性能指标
    double avg_latency = 0;
    if (!latencies.empty()) {
        for (double latency : latencies) {
            avg_latency += latency;
        }
        avg_latency /= latencies.size();
    }
    
    double throughput = (successful_operations * 1000.0) / duration.count();
    
    // 保存测试结果
    TestResult result;
    result.test_name = config.name + "_SELECT_RANGE";
    result.avg_latency = avg_latency;
    result.throughput = throughput;
    result.operations_completed = successful_operations;
    result.duration = duration;
    
    // 验证性能要求
    bool requirement_met = VerifyPerformanceRequirement("SELECT_RANGE", avg_latency);
    result.custom_metrics["requirement_met"] = requirement_met ? "YES" : "NO";
    result.custom_metrics["data_size"] = std::to_string(config.data_size);
    
    test_results_.push_back(result);
    
    std::stringstream result_ss;
    result_ss << "SELECT range test completed: " << successful_operations << " operations, "
              << "avg latency = " << std::fixed << std::setprecision(3) << avg_latency << "ms, "
              << "throughput = " << std::fixed << std::setprecision(2) << throughput << " ops/sec, "
              << "requirement met = " << (requirement_met ? "YES" : "NO");
    SQLCC_LOG_INFO(result_ss.str());
}

void CRUDPerformanceTest::RunUpdatePerformanceTest(const CRUDTestConfig& config) {
    std::stringstream info_ss;
    info_ss << "Starting UPDATE performance test with " << config.data_size << " operations";
    SQLCC_LOG_INFO(info_ss.str());
    
    std::vector<double> latencies;
    size_t test_operations = std::min(config.data_size, static_cast<size_t>(1000));
    size_t successful_operations = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < test_operations; i++) {
        double latency_ms = 0;
        size_t record_id = rng_() % config.data_size;
        
        if (ExecuteUpdateOperation(record_id, latency_ms)) {
            latencies.push_back(latency_ms);
            successful_operations++;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 计算性能指标
    double avg_latency = 0;
    if (!latencies.empty()) {
        for (double latency : latencies) {
            avg_latency += latency;
        }
        avg_latency /= latencies.size();
    }
    
    double throughput = (successful_operations * 1000.0) / duration.count();
    
    // 保存测试结果
    TestResult result;
    result.test_name = config.name + "_UPDATE";
    result.avg_latency = avg_latency;
    result.throughput = throughput;
    result.operations_completed = successful_operations;
    result.duration = duration;
    
    // 验证性能要求
    bool requirement_met = VerifyPerformanceRequirement("UPDATE", avg_latency);
    result.custom_metrics["requirement_met"] = requirement_met ? "YES" : "NO";
    result.custom_metrics["data_size"] = std::to_string(config.data_size);
    
    test_results_.push_back(result);
    
    std::stringstream result_ss;
    result_ss << "UPDATE test completed: " << successful_operations << " operations, "
              << "avg latency = " << std::fixed << std::setprecision(3) << avg_latency << "ms, "
              << "throughput = " << std::fixed << std::setprecision(2) << throughput << " ops/sec, "
              << "requirement met = " << (requirement_met ? "YES" : "NO");
    SQLCC_LOG_INFO(result_ss.str());
}

void CRUDPerformanceTest::RunDeletePerformanceTest(const CRUDTestConfig& config) {
    std::stringstream info_ss;
    info_ss << "Starting DELETE performance test with " << config.data_size << " operations";
    SQLCC_LOG_INFO(info_ss.str());
    
    std::vector<double> latencies;
    size_t test_operations = std::min(config.data_size, static_cast<size_t>(1000));
    size_t successful_operations = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < test_operations; i++) {
        double latency_ms = 0;
        size_t record_id = rng_() % config.data_size;
        
        if (ExecuteDeleteOperation(record_id, latency_ms)) {
            latencies.push_back(latency_ms);
            successful_operations++;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 计算性能指标
    double avg_latency = 0;
    if (!latencies.empty()) {
        for (double latency : latencies) {
            avg_latency += latency;
        }
        avg_latency /= latencies.size();
    }
    
    double throughput = (successful_operations * 1000.0) / duration.count();
    
    // 保存测试结果
    TestResult result;
    result.test_name = config.name + "_DELETE";
    result.avg_latency = avg_latency;
    result.throughput = throughput;
    result.operations_completed = successful_operations;
    result.duration = duration;
    
    // 验证性能要求
    bool requirement_met = VerifyPerformanceRequirement("DELETE", avg_latency);
    result.custom_metrics["requirement_met"] = requirement_met ? "YES" : "NO";
    result.custom_metrics["data_size"] = std::to_string(config.data_size);
    
    test_results_.push_back(result);
    
    std::stringstream result_ss;
    result_ss << "DELETE test completed: " << successful_operations << " operations, "
              << "avg latency = " << std::fixed << std::setprecision(3) << avg_latency << "ms, "
              << "throughput = " << std::fixed << std::setprecision(2) << throughput << " ops/sec, "
              << "requirement met = " << (requirement_met ? "YES" : "NO");
    SQLCC_LOG_INFO(result_ss.str());
}

bool CRUDPerformanceTest::ExecuteInsertOperation(size_t record_id, size_t record_size, double& latency_ms) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string name = "TestUser_" + std::to_string(record_id);
        int age = 20 + (rng_() % 50);
        std::string data = GenerateRandomData(record_size);
        
        std::string insert_sql = "INSERT INTO " + std::string(kTestTable) + 
                                " (id, name, age, data) VALUES (" + 
                                std::to_string(record_id) + ", '" + 
                                name + "', " + std::to_string(age) + ", '" + 
                                data + "')";
        
        std::string result = sql_executor_->Execute(insert_sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return result.empty() || sql_executor_->GetLastError().empty();
    } catch (const std::exception& e) {
        std::stringstream error_ss;
        error_ss << "Exception during INSERT operation: " << e.what();
        SQLCC_LOG_ERROR(error_ss.str());
        return false;
    }
}

bool CRUDPerformanceTest::ExecuteSelectPointOperation(size_t record_id, double& latency_ms) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
                                " WHERE id = " + std::to_string(record_id);
        
        std::string result = sql_executor_->Execute(select_sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return result.empty() || sql_executor_->GetLastError().empty();
    } catch (const std::exception& e) {
        std::stringstream error_ss;
        error_ss << "Exception during SELECT point operation: " << e.what();
        SQLCC_LOG_ERROR(error_ss.str());
        return false;
    }
}

bool CRUDPerformanceTest::ExecuteSelectRangeOperation(size_t start_id, size_t end_id, double& latency_ms) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string select_sql = "SELECT * FROM " + std::string(kTestTable) + 
                                " WHERE id >= " + std::to_string(start_id) + 
                                " AND id <= " + std::to_string(end_id);
        
        std::string result = sql_executor_->Execute(select_sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return result.empty() || sql_executor_->GetLastError().empty();
    } catch (const std::exception& e) {
        std::stringstream error_ss;
        error_ss << "Exception during SELECT range operation: " << e.what();
        SQLCC_LOG_ERROR(error_ss.str());
        return false;
    }
}

bool CRUDPerformanceTest::ExecuteUpdateOperation(size_t record_id, double& latency_ms) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string new_name = "UpdatedUser_" + std::to_string(record_id);
        int new_age = 30 + (rng_() % 40);
        
        std::string update_sql = "UPDATE " + std::string(kTestTable) + 
                                " SET name = '" + new_name + "', age = " + 
                                std::to_string(new_age) + " WHERE id = " + 
                                std::to_string(record_id);
        
        std::string result = sql_executor_->Execute(update_sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return result.empty() || sql_executor_->GetLastError().empty();
    } catch (const std::exception& e) {
        std::stringstream error_ss;
        error_ss << "Exception during UPDATE operation: " << e.what();
        SQLCC_LOG_ERROR(error_ss.str());
        return false;
    }
}

bool CRUDPerformanceTest::ExecuteDeleteOperation(size_t record_id, double& latency_ms) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::string delete_sql = "DELETE FROM " + std::string(kTestTable) + 
                                " WHERE id = " + std::to_string(record_id);
        
        std::string result = sql_executor_->Execute(delete_sql);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        latency_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return result.empty() || sql_executor_->GetLastError().empty();
    } catch (const std::exception& e) {
        std::stringstream error_ss;
        error_ss << "Exception during DELETE operation: " << e.what();
        SQLCC_LOG_ERROR(error_ss.str());
        return false;
    }
}

std::string CRUDPerformanceTest::GenerateRandomData(size_t size) {
    std::string data;
    data.reserve(size);
    
    for (size_t i = 0; i < size; i++) {
        data += 'A' + (rng_() % 26);
    }
    
    return data;
}

bool CRUDPerformanceTest::VerifyPerformanceRequirement(const std::string& operation, double avg_latency) {
    // 根据README.md要求，单操作耗时<5ms (SSD)
    const double requirement_threshold = 5.0; // 5ms
    
    bool met = avg_latency < requirement_threshold;
    
    std::stringstream requirement_ss;
    requirement_ss << "Performance requirement for " << operation << ": "
                   << std::fixed << std::setprecision(3) << avg_latency << "ms < " 
                   << requirement_threshold << "ms = " << (met ? "PASS" : "FAIL");
    SQLCC_LOG_INFO(requirement_ss.str());
    
    return met;
}

void CRUDPerformanceTest::GeneratePerformanceReport(const std::vector<TestResult>& results) {
    std::stringstream report_ss;
    report_ss << "\n" << std::string(80, '=') << "\n";
    report_ss << "CRUD PERFORMANCE TEST REPORT\n";
    report_ss << std::string(80, '=') << "\n\n";
    
    report_ss << "Test Summary:\n";
    report_ss << std::string(40, '-') << "\n";
    
    size_t total_tests = results.size();
    size_t passed_tests = 0;
    
    for (const auto& result : results) {
        bool passed = result.custom_metrics.find("requirement_met") != result.custom_metrics.end() &&
                     result.custom_metrics.at("requirement_met") == "YES";
        
        if (passed) passed_tests++;
        
        report_ss << "Test: " << result.test_name << "\n";
        report_ss << "  Data Size: " << result.custom_metrics.at("data_size") << " records\n";
        report_ss << "  Avg Latency: " << std::fixed << std::setprecision(3) << result.avg_latency << " ms\n";
        report_ss << "  Throughput: " << std::fixed << std::setprecision(2) << result.throughput << " ops/sec\n";
        report_ss << "  Operations: " << result.operations_completed << "\n";
        report_ss << "  Duration: " << result.duration.count() << " ms\n";
        report_ss << "  Requirement Met: " << (passed ? "YES" : "NO") << "\n\n";
    }
    
    report_ss << "Overall Results:\n";
    report_ss << std::string(40, '-') << "\n";
    report_ss << "Total Tests: " << total_tests << "\n";
    report_ss << "Passed Tests: " << passed_tests << "\n";
    report_ss << "Failed Tests: " << (total_tests - passed_tests) << "\n";
    report_ss << "Success Rate: " << std::fixed << std::setprecision(1) 
              << (static_cast<double>(passed_tests) / total_tests * 100) << "%\n";
    
    report_ss << "\n" << std::string(80, '=') << "\n";
    
    // 输出报告到控制台
    std::cout << report_ss.str();
    
    // 保存报告到文件
    std::ofstream report_file("crud_performance_report.txt");
    if (report_file.is_open()) {
        report_file << report_ss.str();
        report_file.close();
        std::stringstream file_ss;
        file_ss << "Performance report saved to: crud_performance_report.txt";
        SQLCC_LOG_INFO(file_ss.str());
    }
}

}  // namespace test
}  // namespace sqlcc