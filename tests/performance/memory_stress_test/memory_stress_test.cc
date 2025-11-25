#include "memory_stress_test.h"

namespace sqlcc {
namespace test {

// 实现内存压力测试中的方法
void MemoryStressTest::SetUp() {
    // 初始化SQL执行器
    sql_executor_ = new SqlExecutor();
    
    // 创建测试数据库和表
    sql_executor_->Execute("CREATE DATABASE IF NOT EXISTS memory_test_db");
    sql_executor_->Execute("USE memory_test_db");
    sql_executor_->Execute(
        "CREATE TABLE IF NOT EXISTS large_data (" 
        "id INT PRIMARY KEY, " 
        "large_text TEXT" 
        ")"
    );
}

void MemoryStressTest::TearDown() {
    // 清理测试数据
    if (sql_executor_) {
        sql_executor_->Execute("DROP DATABASE IF EXISTS memory_test_db");
        delete sql_executor_;
        sql_executor_ = nullptr;
    }
    // 基类没有TearDown方法
}

}  // namespace test
}  // namespace sqlcc
