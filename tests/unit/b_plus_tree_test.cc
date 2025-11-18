#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "b_plus_tree.h"
#include "storage_engine.h"
#include "config_manager.h"
#include "exception.h"

namespace sqlcc {
namespace test {

class BPlusTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建配置管理器
        config_manager_ = std::make_unique<ConfigManager>();
        config_manager_->LoadFromFile("/home/liying/sqlcc/config/sqlcc_test.conf");
        
        // 创建存储引擎
        storage_engine_ = std::make_unique<StorageEngine>(config_manager_.get());
        
        // 获取索引管理器
        index_manager_ = storage_engine_->GetIndexManager();
        
        // 确保索引管理器不为空
        ASSERT_NE(index_manager_, nullptr);
    }
    
    void TearDown() override {
        // 清理测试数据
        try {
            if (index_manager_) {
                // 删除测试创建的索引
                index_manager_->DropIndex("test_table", "test_index");
                index_manager_->DropIndex("test_table", "unique_index");
            }
        } catch (...) {
            // 忽略清理过程中的错误
        }
    }
    
    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<StorageEngine> storage_engine_;
    IndexManager* index_manager_;
};

// 测试创建索引
TEST_F(BPlusTreeTest, CreateIndex) {
    // 创建一个普通索引
    std::vector<std::string> columns = {"id"};
    bool success = index_manager_->CreateIndex("test_table", "test_index", columns, false);
    EXPECT_TRUE(success);
    
    // 创建一个唯一索引
    success = index_manager_->CreateIndex("test_table", "unique_index", {"name"}, true);
    EXPECT_TRUE(success);
    
    // 测试创建已存在的索引
    EXPECT_THROW(index_manager_->CreateIndex("test_table", "test_index", columns, false), Exception);
}

// 测试获取索引
TEST_F(BPlusTreeTest, GetIndex) {
    // 先创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "test_index");
    EXPECT_NE(index, nullptr);
    
    // 测试获取不存在的索引
    EXPECT_THROW(index_manager_->GetIndex("test_table", "non_existent_index"), Exception);
    EXPECT_THROW(index_manager_->GetIndex("non_existent_table", "test_index"), Exception);
}

// 测试删除索引
TEST_F(BPlusTreeTest, DropIndex) {
    // 先创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 删除索引
    bool success = index_manager_->DropIndex("test_table", "test_index");
    EXPECT_TRUE(success);
    
    // 验证索引已被删除
    EXPECT_THROW(index_manager_->GetIndex("test_table", "test_index"), Exception);
    
    // 测试删除不存在的索引
    EXPECT_FALSE(index_manager_->DropIndex("test_table", "non_existent_index"));
}

// 测试B+树索引的插入和查找功能
TEST_F(BPlusTreeTest, InsertAndSearch) {
    // 创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "test_index");
    ASSERT_NE(index, nullptr);
    
    // 插入一些键值对
    for (int i = 1; i <= 100; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        uint64_t value = i * 10;  // 使用行ID作为值
        EXPECT_TRUE(index->Insert(key, value));
    }
    
    // 测试查找
    for (int i = 1; i <= 100; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        std::vector<uint64_t> results;
        EXPECT_TRUE(index->Search(key, results));
        EXPECT_FALSE(results.empty());
        EXPECT_EQ(results[0], static_cast<uint64_t>(i * 10));
    }
    
    // 测试查找不存在的键
    std::vector<std::string> non_existent_key = {"101"};
    std::vector<uint64_t> results;
    EXPECT_FALSE(index->Search(non_existent_key, results));
    EXPECT_TRUE(results.empty());
}

// 测试B+树索引的删除功能
TEST_F(BPlusTreeTest, Delete) {
    // 创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "test_index");
    ASSERT_NE(index, nullptr);
    
    // 插入一些键值对
    for (int i = 1; i <= 100; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        uint64_t value = i * 10;
        index->Insert(key, value);
    }
    
    // 删除一些键
    for (int i = 1; i <= 50; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        EXPECT_TRUE(index->Delete(key, i * 10));
    }
    
    // 验证删除的键已不存在
    for (int i = 1; i <= 50; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        std::vector<uint64_t> results;
        EXPECT_FALSE(index->Search(key, results));
    }
    
    // 验证未删除的键仍然存在
    for (int i = 51; i <= 100; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        std::vector<uint64_t> results;
        EXPECT_TRUE(index->Search(key, results));
        EXPECT_EQ(results[0], static_cast<uint64_t>(i * 10));
    }
    
    // 测试删除不存在的键
    std::vector<std::string> non_existent_key = {"101"};
    EXPECT_FALSE(index->Delete(non_existent_key, 1010));
}

// 测试唯一索引的唯一性约束
TEST_F(BPlusTreeTest, UniqueIndexConstraint) {
    // 创建唯一索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "unique_index", columns, true);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "unique_index");
    ASSERT_NE(index, nullptr);
    
    // 插入第一个键值对
    std::vector<std::string> key = {"1"};
    EXPECT_TRUE(index->Insert(key, 10));
    
    // 尝试插入相同的键，应该失败
    EXPECT_FALSE(index->Insert(key, 20));
    
    // 删除键后应该可以重新插入
    EXPECT_TRUE(index->Delete(key, 10));
    EXPECT_TRUE(index->Insert(key, 30));
}

// 测试范围查询功能
TEST_F(BPlusTreeTest, RangeQuery) {
    // 创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "test_index");
    ASSERT_NE(index, nullptr);
    
    // 插入一些有序的键值对
    for (int i = 1; i <= 100; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        uint64_t value = i * 10;
        index->Insert(key, value);
    }
    
    // 测试范围查询 [20, 30]
    std::vector<std::string> start_key = {"20"};
    std::vector<std::string> end_key = {"30"};
    std::vector<std::pair<std::vector<std::string>, uint64_t>> range_results;
    
    EXPECT_TRUE(index->RangeQuery(start_key, end_key, range_results));
    EXPECT_EQ(range_results.size(), 11);  // 20到30共11个值
    
    // 验证结果的正确性
    int expected_id = 20;
    for (const auto& result : range_results) {
        EXPECT_EQ(result.first[0], std::to_string(expected_id));
        EXPECT_EQ(result.second, static_cast<uint64_t>(expected_id * 10));
        ++expected_id;
    }
}

// 测试并发操作（模拟）
TEST_F(BPlusTreeTest, ConcurrentOperations) {
    // 创建索引
    std::vector<std::string> columns = {"id"};
    index_manager_->CreateIndex("test_table", "test_index", columns, false);
    
    // 获取索引
    auto index = index_manager_->GetIndex("test_table", "test_index");
    ASSERT_NE(index, nullptr);
    
    // 模拟并发插入
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);
    
    // 启动多个线程同时插入
    for (int t = 0; t < 5; ++t) {
        threads.emplace_back([t, index, &success_count]() {
            for (int i = t * 100 + 1; i <= (t + 1) * 100; ++i) {
                std::vector<std::string> key = {std::to_string(i)};
                if (index->Insert(key, i * 10)) {
                    success_count++;
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有插入都成功
    EXPECT_EQ(success_count, 500);
    
    // 验证所有键都能被正确找到
    for (int i = 1; i <= 500; ++i) {
        std::vector<std::string> key = {std::to_string(i)};
        std::vector<uint64_t> results;
        EXPECT_TRUE(index->Search(key, results));
        EXPECT_EQ(results[0], static_cast<uint64_t>(i * 10));
    }
}

} // namespace test
} // namespace sqlcc

// 运行测试
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}