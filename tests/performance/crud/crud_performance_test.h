#ifndef CRUD_PERFORMANCE_TEST_H
#define CRUD_PERFORMANCE_TEST_H

#include "performance_test_base.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace sqlcc {
namespace test {

/**
 * @brief CRUD性能测试类
 *
 * 测试要求：插入、点查、范围扫描、更新、删除
 * 数据规模：1-10万行数据
 * 性能要求：单操作耗时<5ms (SSD)
 */

// 测试配置结构体
struct CRUDTestConfig {
  size_t data_size;
  size_t record_size;
  size_t thread_count;
  std::string name;
  std::string description;
};

class CRUDPerformanceTest : public PerformanceTestBase {
public:
  CRUDPerformanceTest();
  ~CRUDPerformanceTest() override;

  // 重写基类方法
  void RunAllTests() override;
  void Cleanup() override;

private:
  // 常量定义
  static constexpr const char *kTestDatabase = "crud_performance_test_db";
  static constexpr const char *kTestTable = "test_table";

  // 成员变量
  std::string test_db_path_;
  std::unique_ptr<sqlcc::SqlExecutor> sql_executor_;
  std::atomic<size_t> next_record_id_;
  std::vector<CRUDTestConfig> test_configs_;
  std::vector<TestResult> test_results_;
  std::mt19937 rng_;

  // 测试环境设置方法
  void SetupTestEnvironment();
  void PrepopulateTestData(size_t data_size);

  // CRUD性能测试方法
  void RunInsertPerformanceTest(const CRUDTestConfig &config);
  void RunSelectPointPerformanceTest(const CRUDTestConfig &config);
  void RunSelectRangePerformanceTest(const CRUDTestConfig &config);
  void RunUpdatePerformanceTest(const CRUDTestConfig &config);
  void RunDeletePerformanceTest(const CRUDTestConfig &config);

  // 具体操作执行方法
  bool ExecuteInsertOperation(size_t record_id, size_t record_size,
                              double &latency_ms);
  bool ExecuteSelectPointOperation(size_t record_id, double &latency_ms);
  bool ExecuteSelectRangeOperation(size_t start_id, size_t end_id,
                                   double &latency_ms);
  bool ExecuteUpdateOperation(size_t record_id, double &latency_ms);
  bool ExecuteDeleteOperation(size_t record_id, double &latency_ms);

  // 辅助方法
  std::string GenerateRandomData(size_t size);
  bool VerifyPerformanceRequirement(const std::string &operation,
                                    double avg_latency);
  void GeneratePerformanceReport(const std::vector<TestResult> &results);
};

} // namespace test
} // namespace sqlcc

#endif // CRUD_PERFORMANCE_TEST_H
