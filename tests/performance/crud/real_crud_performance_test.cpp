#include "../performance_test_base.h"
#include "crud_performance_test.h"
#include "../../include/sql_executor.h"
#include "../../include/database_manager.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <thread>
#include <fstream>

namespace sqlcc {
namespace test {

CRUDPerformanceTest::CRUDPerformanceTest(const std::string& scale) 
    : next_record_id_(1), rng_(std::random_device{}()) {
  // 根据测试规模初始化测试配置
  if (scale == "small") {
    test_configs_ = {
      {1000, 100, 1, "Small", "1000 records, 100 bytes each, 1 thread"}
    };
  } else if (scale == "medium") {
    test_configs_ = {
      {10000, 200, 2, "Medium", "10000 records, 200 bytes each, 2 threads"}
    };
  } else if (scale == "large") {
    test_configs_ = {
      {50000, 500, 4, "Large", "50000 records, 500 bytes each, 4 threads"}
    };
  } else if (scale == "xlarge") {
    test_configs_ = {
      {100000, 1000, 8, "XLarge", "100000 records, 1000 bytes each, 8 threads"}
    };
  } else {
    // 默认运行所有规模测试
    test_configs_ = {
      {1000, 100, 1, "Small", "1000 records, 100 bytes each, 1 thread"},
      {10000, 200, 2, "Medium", "10000 records, 200 bytes each, 2 threads"},
      {50000, 500, 4, "Large", "50000 records, 500 bytes each, 4 threads"},
      {100000, 1000, 8, "XLarge", "100000 records, 1000 bytes each, 8 threads"}
    };
  }
}

CRUDPerformanceTest::~CRUDPerformanceTest() {
  Cleanup();
}

void CRUDPerformanceTest::RunAllTests() {
  std::cout << "=== SQLCC Real CRUD Performance Tests ===" << std::endl;
  
  SetupTestEnvironment();
  
  for (const auto& config : test_configs_) {
    std::cout << "\n--- Running " << config.name << " Test ---" << std::endl;
    std::cout << "Description: " << config.description << std::endl;
    
    // 预填充测试数据
    PrepopulateTestData(config.data_size);
    
    // 运行各项测试
    RunInsertPerformanceTest(config);
    RunSelectPointPerformanceTest(config);
    RunSelectRangePerformanceTest(config);
    RunUpdatePerformanceTest(config);
    RunDeletePerformanceTest(config);
  }
  
  // 生成性能报告
  GeneratePerformanceReport(test_results_);
}

void CRUDPerformanceTest::Cleanup() {
  // 清理测试环境
  // 注意：由于SQL解析器暂不支持DROP TABLE语句，这里暂时不执行任何操作
  // if (sql_executor_) {
  //   sql_executor_->Execute("DROP TABLE IF EXISTS performance_test_table");
  // }
}

void CRUDPerformanceTest::SetupTestEnvironment() {
  std::cout << "Setting up test environment..." << std::endl;
  
  // 创建数据库管理器
  auto db_manager = std::make_shared<DatabaseManager>("./test_data", 1024, 16, 64);
  db_manager->Initialize();
  
  // 创建SQL执行器
  sql_executor_ = std::make_unique<SqlExecutor>(db_manager);
  
  // 创建测试表（不指定数据库，使用默认数据库）
  std::string create_table_sql = R"(
    CREATE TABLE performance_test_table (
      id INT PRIMARY KEY,
      name VARCHAR(50),
      email VARCHAR(100),
      age INT,
      salary FLOAT,
      department VARCHAR(50),
      address TEXT,
      created_at TIMESTAMP
    )
  )";
  
  sql_executor_->Execute(create_table_sql);
  
  std::cout << "Test environment setup complete." << std::endl;
}

void CRUDPerformanceTest::PrepopulateTestData(size_t data_size) {
  std::cout << "Prepopulating test data (" << data_size << " records)..." << std::endl;
  
  // 批量插入数据以提高效率
  const size_t BATCH_SIZE = 1000;
  size_t batches = (data_size + BATCH_SIZE - 1) / BATCH_SIZE;
  
  for (size_t batch = 0; batch < batches; batch++) {
    size_t current_batch_size = std::min(BATCH_SIZE, data_size - batch * BATCH_SIZE);
    
    std::string sql = "INSERT INTO performance_test_table VALUES ";
    std::vector<std::string> value_strings;
    
    for (size_t i = 0; i < current_batch_size; i++) {
      size_t id = batch * BATCH_SIZE + i + 1;
      std::string name = "User" + std::to_string(id);
      std::string email = "user" + std::to_string(id) + "@example.com";
      int age = 20 + (id % 50);
      float salary = 30000.0f + (id % 70000);
      std::string department = "Department" + std::to_string(id % 10);
      std::string address = "Address " + std::to_string(id) + ", City, Country";
      std::string created_at = "2025-01-01 10:00:00";
      
      std::string value_str = "(" +
          std::to_string(id) + ", '" +
          name + "', '" +
          email + "', " +
          std::to_string(age) + ", " +
          std::to_string(salary) + ", '" +
          department + "', '" +
          address + "', '" +
          created_at + "')";
      
      value_strings.push_back(value_str);
    }
    
    sql += std::accumulate(value_strings.begin() + 1, value_strings.end(), value_strings[0],
                          [](const std::string& a, const std::string& b) {
                              return a + ", " + b;
                          });
    
    sql_executor_->Execute(sql);
  }
  
  next_record_id_.store(data_size + 1);
  std::cout << "Test data prepopulation complete." << std::endl;
}

void CRUDPerformanceTest::RunInsertPerformanceTest(const CRUDTestConfig &config) {
  // 静默执行测试，避免控制台输出影响性能
  auto start_time = std::chrono::high_resolution_clock::now();
  size_t operations_completed = 0;
  std::vector<double> latencies;
  
  // 插入新记录
  for (size_t i = 0; i < config.data_size / 10; i++) {  // 插入现有数据的10%
    auto op_start = std::chrono::high_resolution_clock::now();
    
    size_t id = next_record_id_.fetch_add(1);
    std::string name = "NewUser" + std::to_string(id);
    std::string email = "newuser" + std::to_string(id) + "@example.com";
    int age = 25 + (id % 40);
    float salary = 35000.0f + (id % 60000);
    std::string department = "NewDepartment" + std::to_string(id % 5);
    std::string address = "New Address " + std::to_string(id) + ", City, Country";
    std::string created_at = "2025-01-02 10:00:00";
    
    std::string sql = "INSERT INTO performance_test_table VALUES (" +
        std::to_string(id) + ", '" +
        name + "', '" +
        email + "', " +
        std::to_string(age) + ", " +
        std::to_string(salary) + ", '" +
        department + "', '" +
        address + "', '" +
        created_at + "')";
    
    sql_executor_->Execute(sql);
    
    auto op_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
    latencies.push_back(duration.count() / 1000.0);  // 转换为毫秒
    
    operations_completed++;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  
  // 计算统计数据
  double avg_latency = 0, p95_latency = 0, p99_latency = 0;
  if (!latencies.empty()) {
    std::sort(latencies.begin(), latencies.end());
    avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    p95_latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
  }
  
  double throughput = (operations_completed * 1000.0) / total_duration.count();
  
  TestResult result;
  result.test_scale = config.name;  // 设置测试规模
  result.test_name = "INSERT_" + config.name;
  result.duration = total_duration;
  result.operations_completed = operations_completed;
  result.throughput = throughput;
  result.avg_latency = avg_latency;
  result.p95_latency = p95_latency;
  result.p99_latency = p99_latency;
  
  test_results_.push_back(result);
}

void CRUDPerformanceTest::RunSelectPointPerformanceTest(const CRUDTestConfig &config) {
  // 创建输出文件流
  std::ofstream output_file("crud_performance_output.txt", std::ios::app);
  output_file << "Running SELECT (point query) performance test..." << std::endl;
  output_file.close();
  
  auto start_time = std::chrono::high_resolution_clock::now();
  size_t operations_completed = 0;
  std::vector<double> latencies;
  
  std::uniform_int_distribution<size_t> id_dist(1, next_record_id_.load() - 1);
  
  // 执行点查询
  for (size_t i = 0; i < 1000; i++) {  // 执行1000次查询
    auto op_start = std::chrono::high_resolution_clock::now();
    
    size_t id = id_dist(rng_);
    std::string sql = "SELECT * FROM performance_test_table WHERE id = " + std::to_string(id);
    
    sql_executor_->Execute(sql);
    
    auto op_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
    latencies.push_back(duration.count() / 1000.0);  // 转换为毫秒
    
    operations_completed++;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  
  // 计算统计数据
  double avg_latency = 0, p95_latency = 0, p99_latency = 0;
  if (!latencies.empty()) {
    std::sort(latencies.begin(), latencies.end());
    avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    p95_latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
  }
  
  double throughput = (operations_completed * 1000.0) / total_duration.count();
  
  TestResult result;
  result.test_scale = config.name;  // 设置测试规模
  result.test_name = "SELECT_POINT_" + config.name;
  result.duration = total_duration;
  result.operations_completed = operations_completed;
  result.throughput = throughput;
  result.avg_latency = avg_latency;
  result.p95_latency = p95_latency;
  result.p99_latency = p99_latency;
  
  test_results_.push_back(result);
}

void CRUDPerformanceTest::RunSelectRangePerformanceTest(const CRUDTestConfig &config) {
  // 静默执行测试，避免控制台输出影响性能
  auto start_time = std::chrono::high_resolution_clock::now();
  size_t operations_completed = 0;
  std::vector<double> latencies;
  
  std::uniform_int_distribution<size_t> id_dist(1, next_record_id_.load() - 1000);
  
  // 执行范围查询
  for (size_t i = 0; i < 100; i++) {  // 执行100次查询
    auto op_start = std::chrono::high_resolution_clock::now();
    
    size_t start_id = id_dist(rng_);
    size_t end_id = start_id + 100;
    std::string sql = "SELECT * FROM performance_test_table WHERE id BETWEEN " + 
                     std::to_string(start_id) + " AND " + std::to_string(end_id);
    
    sql_executor_->Execute(sql);
    
    auto op_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
    latencies.push_back(duration.count() / 1000.0);  // 转换为毫秒
    
    operations_completed++;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  
  // 计算统计数据
  double avg_latency = 0, p95_latency = 0, p99_latency = 0;
  if (!latencies.empty()) {
    std::sort(latencies.begin(), latencies.end());
    avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    p95_latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
  }
  
  double throughput = (operations_completed * 1000.0) / total_duration.count();
  
  TestResult result;
  result.test_scale = config.name;  // 设置测试规模
  result.test_name = "SELECT_RANGE_" + config.name;
  result.duration = total_duration;
  result.operations_completed = operations_completed;
  result.throughput = throughput;
  result.avg_latency = avg_latency;
  result.p95_latency = p95_latency;
  result.p99_latency = p99_latency;
  
  test_results_.push_back(result);
}

void CRUDPerformanceTest::RunUpdatePerformanceTest(const CRUDTestConfig &config) {
  // 静默执行测试，避免控制台输出影响性能
  auto start_time = std::chrono::high_resolution_clock::now();
  size_t operations_completed = 0;
  std::vector<double> latencies;
  
  std::uniform_int_distribution<size_t> id_dist(1, next_record_id_.load() - 1);
  
  // 执行更新操作
  for (size_t i = 0; i < 1000; i++) {  // 执行1000次更新
    auto op_start = std::chrono::high_resolution_clock::now();
    
    size_t id = id_dist(rng_);
    float new_salary = 40000.0f + (id % 50000);
    std::string sql = "UPDATE performance_test_table SET salary = " + 
                     std::to_string(new_salary) + " WHERE id = " + std::to_string(id);
    
    sql_executor_->Execute(sql);
    
    auto op_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
    latencies.push_back(duration.count() / 1000.0);  // 转换为毫秒
    
    operations_completed++;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  
  // 计算统计数据
  double avg_latency = 0, p95_latency = 0, p99_latency = 0;
  if (!latencies.empty()) {
    std::sort(latencies.begin(), latencies.end());
    avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    p95_latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
  }
  
  double throughput = (operations_completed * 1000.0) / total_duration.count();
  
  TestResult result;
  result.test_scale = config.name;  // 设置测试规模
  result.test_name = "UPDATE_" + config.name;
  result.duration = total_duration;
  result.operations_completed = operations_completed;
  result.throughput = throughput;
  result.avg_latency = avg_latency;
  result.p95_latency = p95_latency;
  result.p99_latency = p99_latency;
  
  test_results_.push_back(result);
}

void CRUDPerformanceTest::RunDeletePerformanceTest(const CRUDTestConfig &config) {
  // 静默执行测试，避免控制台输出影响性能
  auto start_time = std::chrono::high_resolution_clock::now();
  size_t operations_completed = 0;
  std::vector<double> latencies;
  
  std::uniform_int_distribution<size_t> id_dist(next_record_id_.load(), next_record_id_.load() + 999);
  
  // 执行删除操作（删除新插入的记录）
  for (size_t i = 0; i < 500; i++) {  // 执行500次删除
    auto op_start = std::chrono::high_resolution_clock::now();
    
    size_t id = id_dist(rng_);
    std::string sql = "DELETE FROM performance_test_table WHERE id = " + std::to_string(id);
    
    sql_executor_->Execute(sql);
    
    auto op_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
    latencies.push_back(duration.count() / 1000.0);  // 转换为毫秒
    
    operations_completed++;
  }
  
  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  
  // 计算统计数据
  double avg_latency = 0, p95_latency = 0, p99_latency = 0;
  if (!latencies.empty()) {
    std::sort(latencies.begin(), latencies.end());
    avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    p95_latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
  }
  
  double throughput = (operations_completed * 1000.0) / total_duration.count();
  
  TestResult result;
  result.test_scale = config.name;  // 设置测试规模
  result.test_name = "DELETE_" + config.name;
  result.duration = total_duration;
  result.operations_completed = operations_completed;
  result.throughput = throughput;
  result.avg_latency = avg_latency;
  result.p95_latency = p95_latency;
  result.p99_latency = p99_latency;
  
  test_results_.push_back(result);
  PrintResult(result);
}

std::string CRUDPerformanceTest::GenerateRandomData(size_t size) {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  
  std::string result;
  result.reserve(size);
  
  std::uniform_int_distribution<size_t> dist(0, sizeof(alphanum) - 2);
  
  for (size_t i = 0; i < size; ++i) {
    result += alphanum[dist(rng_)];
  }
  
  return result;
}

bool CRUDPerformanceTest::VerifyPerformanceRequirement(const std::string &operation,
                                                     double avg_latency) {
  // 简单的性能要求验证
  if (operation == "INSERT" && avg_latency > 5.0) {
    return false;
  } else if (operation == "SELECT_POINT" && avg_latency > 2.0) {
    return false;
  } else if (operation == "SELECT_RANGE" && avg_latency > 10.0) {
    return false;
  } else if (operation == "UPDATE" && avg_latency > 3.0) {
    return false;
  } else if (operation == "DELETE" && avg_latency > 3.0) {
    return false;
  }
  
  return true;
}

void CRUDPerformanceTest::GeneratePerformanceReport(const std::vector<TestResult> &results) {
  std::cout << "\n=== Performance Test Report ===" << std::endl;
  
  // 创建报告文件
  std::ofstream report_file("crud_performance_report.txt");
  
  report_file << "SQLCC CRUD Performance Test Report\n";
  report_file << "==================================\n\n";
  
  for (const auto& result : results) {
    std::cout << "\nTest: " << result.test_name << std::endl;
    std::cout << "  Duration: " << result.duration.count() << " ms" << std::endl;
    std::cout << "  Operations: " << result.operations_completed << std::endl;
    std::cout << "  Throughput: " << result.throughput << " ops/sec" << std::endl;
    std::cout << "  Avg Latency: " << result.avg_latency << " ms" << std::endl;
    std::cout << "  95th Percentile: " << result.p95_latency << " ms" << std::endl;
    std::cout << "  99th Percentile: " << result.p99_latency << " ms" << std::endl;
    
    report_file << "Test: " << result.test_name << "\n";
    report_file << "  Duration: " << result.duration.count() << " ms\n";
    report_file << "  Operations: " << result.operations_completed << "\n";
    report_file << "  Throughput: " << result.throughput << " ops/sec\n";
    report_file << "  Avg Latency: " << result.avg_latency << " ms\n";
    report_file << "  95th Percentile: " << result.p95_latency << " ms\n";
    report_file << "  99th Percentile: " << result.p99_latency << " ms\n\n";
  }
  
  report_file.close();
  std::cout << "\nDetailed report saved to crud_performance_report.txt" << std::endl;
}

} // namespace test
} // namespace sqlcc