#include "long_term_stability_test.h"
#include "sql_executor.h"

namespace sqlcc {
namespace test {

// 实现稳定性测试中的方法
void LongTermStabilityTest::SetUp() {
    // 直接初始化SQL执行器
    sql_executor_ = new SqlExecutor();
    
    // 创建测试数据库和表
    sql_executor_->Execute("CREATE DATABASE IF NOT EXISTS stability_test_db");
    sql_executor_->Execute("USE stability_test_db");
    sql_executor_->Execute(
        "CREATE TABLE IF NOT EXISTS stability_data (" 
        "id INT PRIMARY KEY, " 
        "counter INT, " 
        "status VARCHAR(50), " 
        "last_updated TIMESTAMP" 
        ")"
    );
}

void LongTermStabilityTest::TearDown() {
    // 清理测试数据
    if (sql_executor_) {
        sql_executor_->Execute("DROP DATABASE IF EXISTS stability_test_db");
        delete sql_executor_;
        sql_executor_ = nullptr;
    }
}

} // namespace test
} // namespace sqlcc
