#include "index_performance_test.h"

// 实现索引性能测试中的方法
namespace sqlcc {
namespace test {
namespace performance {

void IndexPerformanceTest::SetUp() {
    // 初始化测试环境
    // 注意：不使用sql_executor_，因为类中没有定义这个成员变量
    test_table_name_ = "index_test_data";
    db_file_path_ = GetOutputDirectory() + "/index_test.db";
}

void IndexPerformanceTest::TearDown() {
    // 清理测试数据
    CleanOutputDirectory();
    // 基类中没有TearDown方法，移除调用
}

} // namespace performance
} // namespace test
} // namespace sqlcc
