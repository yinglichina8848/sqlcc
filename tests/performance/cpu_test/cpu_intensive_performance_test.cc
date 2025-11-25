#include "cpu_intensive_performance_test.h"

namespace sqlcc {
namespace test {

// 实现CPU密集型性能测试中的方法
void CpuIntensivePerformanceTest::SetUp() {
    // 基类无SetUp方法，直接初始化SQL执行器
    sql_executor_ = new SqlExecutor();
    
    // 创建测试数据库和表
    sql_executor_->Execute("CREATE DATABASE IF NOT EXISTS cpu_test_db");
    sql_executor_->Execute("USE cpu_test_db");
    sql_executor_->Execute(
        "CREATE TABLE IF NOT EXISTS test_data (" 
        "id INT PRIMARY KEY, " 
        "data1 INT, " 
        "data2 INT, " 
        "data3 DOUBLE" 
        ")"
    );
}

void CpuIntensivePerformanceTest::TearDown() {
    // 清理测试数据
    if (sql_executor_) {
        sql_executor_->Execute("DROP DATABASE IF EXISTS cpu_test_db");
        delete sql_executor_;
        sql_executor_ = nullptr;
    }
    // 基类无TearDown方法
}

} // namespace test
} // namespace sqlcc
