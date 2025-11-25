#include "index_constraint_benchmark.h"
#include "sql_executor.h"

namespace sqlcc {
namespace test {

void IndexConstraintBenchmark::RunAllTests() {
    // 实现索引约束基准测试
    SqlExecutor executor;
    
    // 创建测试数据库和表
    executor.Execute("CREATE DATABASE IF NOT EXISTS constraint_benchmark_db");
    executor.Execute("USE constraint_benchmark_db");
    executor.Execute(
        "CREATE TABLE IF NOT EXISTS benchmark_data (" 
        "id INT PRIMARY KEY, " 
        "constraint_col INT, " 
        "INDEX idx_constraint (constraint_col)" 
        ")"
    );
    
    // 测试完成后清理
    executor.Execute("DROP DATABASE IF EXISTS constraint_benchmark_db");
}

} // namespace test
} // namespace sqlcc
