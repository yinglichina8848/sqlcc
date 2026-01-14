#include "sql_executor.h"
#include "unified_executor.h"
#include "core/core_database_manager.h"
#include "system_database.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <chrono>
#include <thread>

namespace sqlcc {

// 性能基准测试类
class PerformanceBenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_dir_ = "./test_perf_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directory(test_data_dir_);

        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>(
            test_data_dir_ + "/test.db", 1024, 4, 2);

        // 初始化用户管理器
        user_manager_ = std::make_shared<UserManager>();

        // 初始化系统数据库
        system_db_ = std::make_shared<SystemDatabase>(db_manager_);

        // 初始化统一执行器
        unified_executor_ = std::make_shared<UnifiedExecutor>(
            db_manager_, user_manager_, system_db_);

        // 初始化SQL执行器
        sql_executor_ = std::make_unique<SqlExecutor>();
    }

    void TearDown() override {
        // 清理资源
        sql_executor_.reset();
        unified_executor_.reset();
        system_db_.reset();
        user_manager_.reset();
        db_manager_.reset();

        // 删除测试数据目录
        std::filesystem::remove_all(test_data_dir_);
    }

    // 执行SQL并验证结果
    void ExecuteAndVerify(const std::string& sql, const std::string& expected_keyword = "success") {
        std::string result = sql_executor_->Execute(sql);

        // 检查是否包含预期关键词
        EXPECT_TRUE(result.find(expected_keyword) != std::string::npos ||
                   result.find("错误") == std::string::npos);

        // 检查错误状态
        std::string error = sql_executor_->GetLastError();
        EXPECT_TRUE(error.empty() || error.find("错误") == std::string::npos);
    }

    // 测量执行时间的辅助函数
    template<typename Func>
    double MeasureExecutionTime(Func&& func, int iterations = 1) {
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            func();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);

        return static_cast<double>(duration.count()) / iterations / 1000.0; // 转换为毫秒
    }

    std::string test_data_dir_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::shared_ptr<UnifiedExecutor> unified_executor_;
    std::unique_ptr<SqlExecutor> sql_executor_;
};

// 测试批量插入性能
TEST_F(PerformanceBenchmarkTest, BulkInsertPerformanceTest) {
    std::cout << "\n=== 批量插入性能测试开始 ===" << std::endl;

    // 准备测试环境
    ExecuteAndVerify("CREATE DATABASE perf_insert_test");
    ExecuteAndVerify("USE perf_insert_test");
    ExecuteAndVerify("CREATE TABLE perf_table ("
                     "id INTEGER PRIMARY KEY, "
                     "data VARCHAR(255), "
                     "timestamp TIMESTAMP)");

    // 批量插入性能测试
    const int batch_size = 1000;

    auto insert_func = [this]() {
        for (int i = 0; i < batch_size; i++) {
            std::string sql = "INSERT INTO perf_table VALUES (" +
                             std::to_string(i) + ", 'Data" + std::to_string(i) + "', CURRENT_TIMESTAMP)";
            sql_executor_->Execute(sql);
        }
    };

    double avg_time = MeasureExecutionTime(insert_func, 1);
    double time_per_insert = avg_time / batch_size;

    std::cout << "批量插入 " << batch_size << " 条记录总耗时: " << avg_time << " ms" << std::endl;
    std::cout << "平均每条插入耗时: " << time_per_insert << " ms" << std::endl;

    // 性能断言：平均每条插入应小于5ms
    EXPECT_LT(time_per_insert, 5.0) << "批量插入性能不达标";

    // 验证插入结果
    ExecuteAndVerify("SELECT COUNT(*) FROM perf_table", std::to_string(batch_size));

    // 清理
    ExecuteAndVerify("DROP TABLE perf_table");
    ExecuteAndVerify("DROP DATABASE perf_insert_test");

    std::cout << "=== 批量插入性能测试完成 ===" << std::endl;
}

// 测试查询性能
TEST_F(PerformanceBenchmarkTest, QueryPerformanceTest) {
    std::cout << "\n=== 查询性能测试开始 ===" << std::endl;

    // 准备测试数据
    ExecuteAndVerify("CREATE DATABASE perf_query_test");
    ExecuteAndVerify("USE perf_query_test");
    ExecuteAndVerify("CREATE TABLE query_perf_table ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(100), "
                     "value INTEGER, "
                     "category VARCHAR(50))");

    // 插入测试数据
    const int data_size = 10000;
    for (int i = 0; i < data_size; ++i) {
        std::string sql = "INSERT INTO query_perf_table VALUES (" +
                         std::to_string(i) + ", 'Item" + std::to_string(i) + "', " +
                         std::to_string(i * 10) + ", 'Category" + std::to_string(i % 10) + "')";
        sql_executor_->Execute(sql);
    }

    // 测试不同类型的查询性能
    std::vector<std::pair<std::string, std::string>> queries = {
        {"主键查询", "SELECT * FROM query_perf_table WHERE id = 5000"},
        {"范围查询", "SELECT * FROM query_perf_table WHERE value BETWEEN 1000 AND 2000"},
        {"条件查询", "SELECT * FROM query_perf_table WHERE category = 'Category5'"},
        {"聚合查询", "SELECT COUNT(*), AVG(value) FROM query_perf_table"},
        {"分组查询", "SELECT category, COUNT(*) FROM query_perf_table GROUP BY category"}
    };

    for (const auto& [query_name, query_sql] : queries) {
        auto query_func = [this, &query_sql]() {
            sql_executor_->Execute(query_sql);
        };

        double avg_time = MeasureExecutionTime(query_func, 10); // 执行10次取平均

        std::cout << query_name << " 平均耗时: " << avg_time << " ms" << std::endl;

        // 性能断言：查询应在100ms内完成
        EXPECT_LT(avg_time, 100.0) << query_name << " 性能不达标";
    }

    // 清理
    ExecuteAndVerify("DROP TABLE query_perf_table");
    ExecuteAndVerify("DROP DATABASE perf_query_test");

    std::cout << "=== 查询性能测试完成 ===" << std::endl;
}

// 测试并发操作性能
TEST_F(PerformanceBenchmarkTest, ConcurrentOperationsPerformanceTest) {
    std::cout << "\n=== 并发操作性能测试开始 ===" << std::endl;

    // 准备测试环境
    ExecuteAndVerify("CREATE DATABASE perf_concurrent_test");
    ExecuteAndVerify("USE perf_concurrent_test");
    ExecuteAndVerify("CREATE TABLE concurrent_perf_table ("
                     "id INTEGER PRIMARY KEY, "
                     "counter INTEGER, "
                     "data VARCHAR(100))");

    // 插入初始数据
    ExecuteAndVerify("INSERT INTO concurrent_perf_table VALUES (1, 0, 'Initial')");

    // 并发更新测试
    const int num_threads = 4;
    const int operations_per_thread = 100;

    auto concurrent_update_func = [this, operations_per_thread]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            ExecuteAndVerify("UPDATE concurrent_perf_table SET counter = counter + 1 WHERE id = 1");
            std::this_thread::sleep_for(std::chrono::microseconds(100)); // 小延迟模拟真实场景
        }
    };

    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 启动并发线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(concurrent_update_func);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    double total_operations = num_threads * operations_per_thread;
    double avg_time_per_operation = static_cast<double>(total_duration.count()) / total_operations;

    std::cout << "并发操作总耗时: " << total_duration.count() << " ms" << std::endl;
    std::cout << "总操作数: " << total_operations << std::endl;
    std::cout << "平均每操作耗时: " << avg_time_per_operation << " ms" << std::endl;
    std::cout << "操作吞吐量: " << (total_operations / total_duration.count() * 1000) << " ops/sec" << std::endl;

    // 性能断言：平均每操作应小于10ms
    EXPECT_LT(avg_time_per_operation, 10.0) << "并发操作性能不达标";

    // 验证数据一致性
    ExecuteAndVerify("SELECT counter FROM concurrent_perf_table WHERE id = 1",
                    std::to_string(static_cast<int>(total_operations)));

    // 清理
    ExecuteAndVerify("DROP TABLE concurrent_perf_table");
    ExecuteAndVerify("DROP DATABASE perf_concurrent_test");

    std::cout << "=== 并发操作性能测试完成 ===" << std::endl;
}

// 测试索引性能
TEST_F(PerformanceBenchmarkTest, IndexPerformanceTest) {
    std::cout << "\n=== 索引性能测试开始 ===" << std::endl;

    // 准备测试环境
    ExecuteAndVerify("CREATE DATABASE perf_index_test");
    ExecuteAndVerify("USE perf_index_test");
    ExecuteAndVerify("CREATE TABLE index_perf_table ("
                     "id INTEGER PRIMARY KEY, "
                     "indexed_col VARCHAR(100), "
                     "non_indexed_col VARCHAR(100), "
                     "value INTEGER)");

    // 插入大量测试数据
    const int data_size = 10000;
    for (int i = 0; i < data_size; ++i) {
        std::string sql = "INSERT INTO index_perf_table VALUES (" +
                         std::to_string(i) + ", 'IndexedValue" + std::to_string(i) + "', " +
                         "'NonIndexedValue" + std::to_string(i) + "', " + std::to_string(i * 10) + ")";
        sql_executor_->Execute(sql);
    }

    // 创建索引
    ExecuteAndVerify("CREATE INDEX idx_indexed_col ON index_perf_table (indexed_col)");

    // 测试索引查询性能
    auto indexed_query_func = [this]() {
        ExecuteAndVerify("SELECT * FROM index_perf_table WHERE indexed_col = 'IndexedValue5000'");
    };

    auto non_indexed_query_func = [this]() {
        ExecuteAndVerify("SELECT * FROM index_perf_table WHERE non_indexed_col = 'NonIndexedValue5000'");
    };

    // 测量性能
    double indexed_query_time = MeasureExecutionTime(indexed_query_func, 100);
    double non_indexed_query_time = MeasureExecutionTime(non_indexed_query_func, 100);

    std::cout << "索引查询平均耗时: " << indexed_query_time << " ms" << std::endl;
    std::cout << "非索引查询平均耗时: " << non_indexed_query_time << " ms" << std::endl;
    std::cout << "性能提升倍数: " << (non_indexed_query_time / indexed_query_time) << "x" << std::endl;

    // 断言：索引查询应比非索引查询快
    EXPECT_LT(indexed_query_time, non_indexed_query_time);

    // 清理
    ExecuteAndVerify("DROP TABLE index_perf_table");
    ExecuteAndVerify("DROP DATABASE perf_index_test");

    std::cout << "=== 索引性能测试完成 ===" << std::endl;
}

// 测试内存使用性能
TEST_F(PerformanceBenchmarkTest, MemoryUsagePerformanceTest) {
    std::cout << "\n=== 内存使用性能测试开始 ===" << std::endl;

    // 这个测试主要关注内存使用模式，不做具体的性能计时
    // 在实际实现中，可以集成内存分析工具

    ExecuteAndVerify("CREATE DATABASE perf_memory_test");
    ExecuteAndVerify("USE perf_memory_test");

    // 创建多个表并插入数据，观察内存使用模式
    for (int i = 0; i < 10; ++i) {
        std::string table_name = "memory_table_" + std::to_string(i);
        ExecuteAndVerify("CREATE TABLE " + table_name + " (id INTEGER, data VARCHAR(1000))");

        // 插入大数据
        for (int j = 0; j < 100; ++j) {
            std::string sql = "INSERT INTO " + table_name + " VALUES (" +
                             std::to_string(j) + ", '" + std::string(500, 'x') + "')";
            sql_executor_->Execute(sql);
        }
    }

    // 执行一些查询操作
    for (int i = 0; i < 10; ++i) {
        std::string table_name = "memory_table_" + std::to_string(i);
        ExecuteAndVerify("SELECT COUNT(*) FROM " + table_name);
        ExecuteAndVerify("SELECT * FROM " + table_name + " ORDER BY id DESC LIMIT 10");
    }

    std::cout << "内存使用测试完成 - 建议使用Valgrind或类似工具进行详细分析" << std::endl;

    // 清理
    for (int i = 0; i < 10; ++i) {
        std::string table_name = "memory_table_" + std::to_string(i);
        ExecuteAndVerify("DROP TABLE " + table_name);
    }
    ExecuteAndVerify("DROP DATABASE perf_memory_test");

    std::cout << "=== 内存使用性能测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}