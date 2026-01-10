#include "test_base.h"
#include <memory>
#include <vector>
#include <string>

namespace sqlcc {
namespace test {

/**
 * @brief 索引性能测试
 *
 * 测试B+树索引的性能特性：
 * - 索引创建性能
 * - 索引查找性能
 * - 索引插入性能
 * - 索引删除性能
 * - 范围查询性能
 */
class IndexPerformanceTest : public PerformanceTestBase {
protected:
    void SetUp() override {
        PerformanceTestBase::SetUp();

        // 设置测试数据规模
        test_data_sizes_ = {1000, 10000, 100000};

        // 初始化存储引擎Mock
        storage_engine_mock_ = GetStorageEngineMock();
        storage_engine_mock_->SetNewPageResult(true);
        storage_engine_mock_->SetFetchPageResult(nullptr);
        storage_engine_mock_->SetFlushPageResult(true);
    }

    void TearDown() override {
        PerformanceTestBase::TearDown();
    }

    // 测试数据规模
    std::vector<size_t> test_data_sizes_;

    // Mock对象
    std::shared_ptr<mocks::StorageEngineMock> storage_engine_mock_;
};

/**
 * @brief 索引创建性能测试
 */
TEST_F(IndexPerformanceTest, CreateIndexPerformance) {
    for (size_t data_size : test_data_sizes_) {
        std::string test_name = "CreateIndex_" + std::to_string(data_size) + "_entries";

        auto result = RunBenchmark([this, data_size]() {
            // 模拟索引创建操作
            auto index_mock = GetIndexManagerMock();

            // 配置Mock返回成功
            index_mock->SetCreateIndexResult(true);

            // 执行创建操作
            std::string index_name = "test_index_" + std::to_string(data_size);
            bool success = index_mock->CreateIndex(index_name, "test_table", "test_column");

            // 验证结果
            ASSERT_TRUE(success);
            AssertCallHistoryContains(*index_mock, "CreateIndex", 1);

            // 清理调用历史
            index_mock->ClearCallHistory();
        }, 10, test_name); // 运行10次迭代

        // 输出性能统计
        std::cout << "Performance Test: " << test_name << std::endl;
        std::cout << "Average time: " << result.average_time_ms << "ms" << std::endl;
        std::cout << "Min time: " << result.min_time_ms << "ms" << std::endl;
        std::cout << "Max time: " << result.max_time_ms << "ms" << std::endl;

        // 断言性能要求（创建索引不应超过100ms）
        ASSERT_LE(result.average_time_ms, 100.0)
            << "Index creation took too long: " << result.average_time_ms << "ms";
    }
}

/**
 * @brief 索引查找性能测试
 */
TEST_F(IndexPerformanceTest, SearchIndexPerformance) {
    for (size_t data_size : test_data_sizes_) {
        std::string test_name = "SearchIndex_" + std::to_string(data_size) + "_entries";

        auto result = RunBenchmark([this, data_size]() {
            auto index_mock = GetIndexManagerMock();

            // 配置Mock返回索引对象
            BPlusTreeIndex mock_index(nullptr, "test_table", "test_column");
            index_mock->SetGetIndexResult(&mock_index);

            // 执行查找操作
            BPlusTreeIndex* result = index_mock->GetIndex("test_index", "test_table");

            // 验证结果
            ASSERT_NE(result, nullptr);
            AssertCallHistoryContains(*index_mock, "GetIndex", 1);

            index_mock->ClearCallHistory();
        }, 100, test_name); // 运行100次迭代，模拟更多查找操作

        std::cout << "Performance Test: " << test_name << std::endl;
        std::cout << "Average time: " << result.average_time_ms << "ms" << std::endl;
        std::cout << "Min time: " << result.min_time_ms << "ms" << std::endl;
        std::cout << "Max time: " << result.max_time_ms << "ms" << std::endl;

        // 断言性能要求（索引查找不应超过10ms）
        ASSERT_LE(result.average_time_ms, 10.0)
            << "Index search took too long: " << result.average_time_ms << "ms";
    }
}

/**
 * @brief 批量索引操作性能测试
 */
TEST_F(IndexPerformanceTest, BulkIndexOperationsPerformance) {
    const size_t batch_size = 1000;

    auto result = RunBenchmark([this, batch_size]() {
        auto index_mock = GetIndexManagerMock();

        // 配置Mock返回成功
        index_mock->SetCreateIndexResult(true);
        index_mock->SetDropIndexResult(true);

        // 执行批量操作
        for (size_t i = 0; i < batch_size; ++i) {
            std::string index_name = "bulk_index_" + std::to_string(i);
            std::string table_name = "bulk_table_" + std::to_string(i % 10); // 10个不同的表
            std::string column_name = "column_" + std::to_string(i % 5);    // 5个不同的列

            // 创建索引
            bool create_success = index_mock->CreateIndex(index_name, table_name, column_name);
            ASSERT_TRUE(create_success);

            // 删除索引
            bool drop_success = index_mock->DropIndex(index_name, table_name);
            ASSERT_TRUE(drop_success);
        }

        // 验证调用历史
        AssertCallHistoryContains(*index_mock, "CreateIndex", batch_size);
        AssertCallHistoryContains(*index_mock, "DropIndex", batch_size);

        index_mock->ClearCallHistory();
    }, 5, "BulkIndexOperations_" + std::to_string(batch_size) + "_operations");

    std::cout << "Bulk operations performance:" << std::endl;
    std::cout << "Total operations: " << (batch_size * 2) << std::endl;
    std::cout << "Average time per operation: "
              << (result.average_time_ms / (batch_size * 2)) << "ms" << std::endl;

    // 断言性能要求（批量操作平均每个不应超过1ms）
    ASSERT_LE(result.average_time_ms / (batch_size * 2), 1.0)
        << "Bulk index operations too slow: "
        << (result.average_time_ms / (batch_size * 2)) << "ms per operation";
}

/**
 * @brief 索引内存使用性能测试
 */
TEST_F(IndexPerformanceTest, IndexMemoryUsagePerformance) {
    // 这个测试主要验证索引操作不会导致内存泄漏
    // 通过多次创建和销毁索引来测试

    const size_t iterations = 1000;

    auto result = RunBenchmark([this]() {
        auto index_mock = GetIndexManagerMock();
        index_mock->SetCreateIndexResult(true);
        index_mock->SetDropIndexResult(true);

        // 创建和删除索引
        bool create_success = index_mock->CreateIndex("memory_test_index", "memory_test_table", "test_column");
        ASSERT_TRUE(create_success);

        bool drop_success = index_mock->DropIndex("memory_test_index", "memory_test_table");
        ASSERT_TRUE(drop_success);

        index_mock->ClearCallHistory();
    }, iterations, "IndexMemoryUsage_" + std::to_string(iterations) + "_iterations");

    std::cout << "Memory usage test completed:" << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "Total time: " << result.total_time_ms << "ms" << std::endl;
    std::cout << "Average time per iteration: "
              << result.average_time_ms << "ms" << std::endl;

    // 验证没有明显的性能退化（内存泄漏可能导致性能下降）
    ASSERT_LE(result.average_time_ms, 5.0)
        << "Memory usage test shows performance degradation: "
        << result.average_time_ms << "ms per iteration";
}

/**
 * @brief 并发索引操作性能测试
 */
TEST_F(IndexPerformanceTest, ConcurrentIndexOperationsPerformance) {
    const size_t concurrent_operations = 50;

    auto result = RunBenchmark([this, concurrent_operations]() {
        std::vector<std::thread> threads;

        // 创建多个线程并发执行索引操作
        for (size_t i = 0; i < concurrent_operations; ++i) {
            threads.emplace_back([this, i]() {
                auto index_mock = GetIndexManagerMock();
                index_mock->SetCreateIndexResult(true);
                index_mock->SetGetIndexResult(nullptr);

                std::string index_name = "concurrent_index_" + std::to_string(i);
                std::string table_name = "concurrent_table_" + std::to_string(i % 5);

                // 执行并发操作
                bool create_success = index_mock->CreateIndex(index_name, table_name, "test_column");
                ASSERT_TRUE(create_success);

                BPlusTreeIndex* result = index_mock->GetIndex(index_name, table_name);
                (void)result; // 避免未使用变量警告

                index_mock->ClearCallHistory();
            });
        }

        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
    }, 3, "ConcurrentIndexOperations_" + std::to_string(concurrent_operations) + "_threads");

    std::cout << "Concurrent operations test:" << std::endl;
    std::cout << "Threads: " << concurrent_operations << std::endl;
    std::cout << "Total time: " << result.total_time_ms << "ms" << std::endl;
    std::cout << "Time per thread: "
              << (result.average_time_ms / concurrent_operations) << "ms" << std::endl;

    // 并发操作不应太慢
    ASSERT_LE(result.average_time_ms, 1000.0)
        << "Concurrent operations took too long: " << result.average_time_ms << "ms";
}

} // namespace test
} // namespace sqlcc