#ifndef INDEX_PERFORMANCE_TEST_H
#define INDEX_PERFORMANCE_TEST_H

#include "performance_test_base.h"
#include "storage_engine.h"
#include "config_manager.h"
#include "b_plus_tree.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

namespace sqlcc {
namespace test {
namespace performance {

class IndexPerformanceTest : public PerformanceTestBase {
public:
    IndexPerformanceTest();
    virtual ~IndexPerformanceTest();
    
    virtual void SetUp();
    virtual void TearDown();
    virtual void RunTests();
    
    virtual std::string GetOutputDirectory() const;
    virtual void CleanOutputDirectory();
    
private:
    // 测试辅助方法
    void CreateTestTable();
    bool InsertData(const std::string& key, const std::string& value);
    std::string FindData(const std::string& key, bool use_index);
    std::vector<std::string> RangeQuery(const std::string& lower, const std::string& upper, bool use_index);
    
    // 各个性能测试方法
    void TestSequentialInserts();
    void TestRandomLookups();
    void TestRangeQueries();
    void TestMixedWorkload();
    void TestIndexSizeGrowth();
    
    // 报告生成方法
    void GenerateReport();
    std::string GetCurrentTimestamp() const;
    
    // 测试数据
    std::string test_table_name_;
    std::string db_file_path_;
    ConfigManager config_manager_;
    std::unique_ptr<StorageEngine> storage_engine_;
    IndexManager* index_manager_;
    std::vector<TestResult> results_;
    std::unordered_map<std::string, std::string> data_store_; // 用于存储测试数据
};

} // namespace performance
} // namespace test
} // namespace sqlcc

#endif // INDEX_PERFORMANCE_TEST_H