#include <gtest/gtest.h>
#include "storage_engine/node_size_manager.h"
#include <vector>
#include <thread>
#include <chrono>

namespace sqlcc {

class NodeSizeManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 重置管理器状态
        NodeSizeManager::get_instance().reset_stats("leaf");
        NodeSizeManager::get_instance().reset_stats("internal");
        NodeSizeManager::get_instance().reset_stats("test_node");
    }

    void TearDown() override {
        // 清理测试数据
        NodeSizeManager::get_instance().reset_stats("leaf");
        NodeSizeManager::get_instance().reset_stats("internal");
        NodeSizeManager::get_instance().reset_stats("test_node");
    }
};

// 测试基本功能
TEST_F(NodeSizeManagerTest, BasicFunctionality) {
    auto& manager = NodeSizeManager::get_instance();

    // 测试获取推荐容量（初始状态应返回默认值）
    size_t capacity = manager.get_recommended_capacity("leaf");
    EXPECT_GT(capacity, 0); // 应该返回一个正的容量值

    // 测试统计信息获取
    NodeSizeStats stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 0);
    EXPECT_EQ(stats.total_size, 0);
}

// 测试统计数据更新
TEST_F(NodeSizeManagerTest, StatsUpdate) {
    auto& manager = NodeSizeManager::get_instance();

    // 模拟一些条目大小数据
    std::vector<size_t> entry_sizes = {50, 75, 100, 25, 150};

    // 记录统计信息
    manager.record_node_stats("leaf", entry_sizes, 64);

    // 验证统计信息
    NodeSizeStats stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 5);
    EXPECT_EQ(stats.total_size, 400); // 50+75+100+25+150
    EXPECT_DOUBLE_EQ(stats.avg_entry_size, 80.0); // 400/5
    EXPECT_EQ(stats.min_entry_size, 25);
    EXPECT_EQ(stats.max_entry_size, 150);

    // 验证推荐容量已更新
    size_t recommended = manager.get_recommended_capacity("leaf");
    EXPECT_GT(recommended, 0); // 应该大于0
    EXPECT_LT(recommended, 10000); // 应该在合理范围内
}

// 测试分裂判断逻辑
TEST_F(NodeSizeManagerTest, ShouldSplitLogic) {
    auto& manager = NodeSizeManager::get_instance();

    // 初始状态（无统计数据）应使用默认逻辑
    bool should_split = manager.should_split_node("leaf", 100, 40000); // 假设页面大小为4096*10
    EXPECT_TRUE(should_split); // 超过默认阈值

    // 添加统计数据后测试
    std::vector<size_t> entry_sizes = {80, 80, 80, 80, 80}; // 平均80字节
    manager.record_node_stats("leaf", entry_sizes, 64);

    // 现在应该使用动态逻辑
    should_split = manager.should_split_node("leaf", 50, 4000);
    // 根据计算逻辑判断结果
    EXPECT_FALSE(should_split); // 应该不会分裂
}

// 测试性能报告
TEST_F(NodeSizeManagerTest, PerformanceReport) {
    auto& manager = NodeSizeManager::get_instance();

    // 添加一些测试数据
    std::vector<size_t> entry_sizes1 = {50, 100, 75};
    manager.record_node_stats("leaf", entry_sizes1, 64);

    std::vector<size_t> entry_sizes2 = {200, 150, 180};
    manager.record_node_stats("internal", entry_sizes2, 128);

    // 获取性能报告
    std::string report = manager.get_performance_report();

    // 验证报告包含必要信息
    EXPECT_NE(report.find("Node Type: leaf"), std::string::npos);
    EXPECT_NE(report.find("Node Type: internal"), std::string::npos);
    EXPECT_NE(report.find("Samples Collected"), std::string::npos);
    EXPECT_NE(report.find("Average Entry Size"), std::string::npos);
    EXPECT_NE(report.find("Recommended Capacity"), std::string::npos);
}

// 测试边界情况
TEST_F(NodeSizeManagerTest, EdgeCases) {
    auto& manager = NodeSizeManager::get_instance();

    // 空条目列表
    std::vector<size_t> empty_sizes;
    manager.record_node_stats("leaf", empty_sizes, 64);

    NodeSizeStats stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 0);
    EXPECT_EQ(stats.total_size, 0);

    // 单个条目
    std::vector<size_t> single_size = {100};
    manager.record_node_stats("leaf", single_size, 64);

    stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 1);
    EXPECT_EQ(stats.total_size, 100);
    EXPECT_DOUBLE_EQ(stats.avg_entry_size, 100.0);
    EXPECT_EQ(stats.min_entry_size, 100);
    EXPECT_EQ(stats.max_entry_size, 100);

    // 大小差异很大的条目
    std::vector<size_t> varied_sizes = {10, 1000, 50, 500};
    manager.record_node_stats("leaf", varied_sizes, 64);

    stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.min_entry_size, 10);
    EXPECT_EQ(stats.max_entry_size, 1000);
    EXPECT_GT(stats.size_variance, 0); // 应该有方差
}

// 测试并发访问（基本测试）
TEST_F(NodeSizeManagerTest, ConcurrentAccess) {
    auto& manager = NodeSizeManager::get_instance();

    // 在多个"线程"中模拟并发访问
    // 注意：这只是基本测试，实际并发测试需要更复杂的设置
    std::vector<size_t> sizes1 = {50, 60, 70};
    std::vector<size_t> sizes2 = {80, 90, 100};

    manager.record_node_stats("leaf", sizes1, 64);
    manager.record_node_stats("leaf", sizes2, 64);

    NodeSizeStats stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 6); // 两个记录的条目总数
    EXPECT_EQ(stats.total_size, 450); // 50+60+70+80+90+100
}

// 测试重置功能
TEST_F(NodeSizeManagerTest, ResetFunctionality) {
    auto& manager = NodeSizeManager::get_instance();

    // 添加数据
    std::vector<size_t> entry_sizes = {50, 100, 75};
    manager.record_node_stats("leaf", entry_sizes, 64);

    // 验证数据存在
    NodeSizeStats stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 3);

    // 重置统计信息
    manager.reset_stats("leaf");

    // 验证数据已被清除
    stats = manager.get_stats("leaf");
    EXPECT_EQ(stats.total_entries, 0);
    EXPECT_EQ(stats.total_size, 0);

    // 推荐容量应该回到默认值
    size_t capacity = manager.get_recommended_capacity("leaf");
    EXPECT_GT(capacity, 0); // 应该返回一个正的容量值
}

// 测试学习过程（需要足够样本才能激活动态调整）
TEST_F(NodeSizeManagerTest, LearningProcess) {
    auto& manager = NodeSizeManager::get_instance();

    // 初始状态应该返回默认值
    size_t initial_capacity = manager.get_recommended_capacity("test_node");
    EXPECT_EQ(initial_capacity, NodeSizeManager::DEFAULT_CAPACITY);

    // 添加少量样本，还不够激活动态调整
    for (int i = 0; i < NodeSizeManager::MIN_SAMPLES - 1; ++i) {
        std::vector<size_t> sizes = {100};
        manager.record_node_stats("test_node", sizes, 64);
    }

    // 仍然应该返回默认值
    size_t learning_capacity = manager.get_recommended_capacity("test_node");
    EXPECT_EQ(learning_capacity, NodeSizeManager::DEFAULT_CAPACITY);

    // 添加最后一个样本，达到最小采样次数
    std::vector<size_t> final_sizes = {100};
    manager.record_node_stats("test_node", final_sizes, 64);

    // 现在应该使用动态计算的值
    size_t active_capacity = manager.get_recommended_capacity("test_node");
    EXPECT_NE(active_capacity, NodeSizeManager::DEFAULT_CAPACITY); // 应该不再是默认值
}

// 测试容量范围限制
TEST_F(NodeSizeManagerTest, CapacityBounds) {
    auto& manager = NodeSizeManager::get_instance();

    // 测试最小容量边界：非常小的条目
    std::vector<size_t> tiny_entries = {1, 1, 1, 1, 1};
    manager.record_node_stats("tiny_test", tiny_entries, 64);

    size_t tiny_capacity = manager.get_recommended_capacity("tiny_test");
    EXPECT_GE(tiny_capacity, NodeSizeManager::MIN_CAPACITY);
    EXPECT_LE(tiny_capacity, NodeSizeManager::MAX_CAPACITY);

    // 测试最大容量边界：非常大的条目
    std::vector<size_t> huge_entries = {10000, 10000, 10000, 10000, 10000};
    manager.record_node_stats("huge_test", huge_entries, 64);

    size_t huge_capacity = manager.get_recommended_capacity("huge_test");
    EXPECT_GE(huge_capacity, NodeSizeManager::MIN_CAPACITY);
    EXPECT_LE(huge_capacity, NodeSizeManager::MAX_CAPACITY);
}

// 测试方差对容量的影响
TEST_F(NodeSizeManagerTest, VarianceImpact) {
    auto& manager = NodeSizeManager::get_instance();

    // 低方差数据（均匀分布）
    std::vector<size_t> uniform_sizes = {95, 100, 105, 100, 105};
    manager.record_node_stats("uniform", uniform_sizes, 64);

    // 高方差数据（差异很大）
    std::vector<size_t> varied_sizes = {10, 200, 50, 500, 25};
    manager.record_node_stats("varied", varied_sizes, 64);

    NodeSizeStats uniform_stats = manager.get_stats("uniform");
    NodeSizeStats varied_stats = manager.get_stats("varied");

    // 验证方差计算
    EXPECT_LT(uniform_stats.size_variance, varied_stats.size_variance);

    // 高方差应该导致更保守的容量建议
    size_t uniform_capacity = manager.get_recommended_capacity("uniform");
    size_t varied_capacity = manager.get_recommended_capacity("varied");

    // 高方差数据可能得到更小的容量建议（为了避免溢出）
    EXPECT_LE(varied_capacity, uniform_capacity + 50); // 允许一些差异
}

// 测试性能报告的详细程度
TEST_F(NodeSizeManagerTest, PerformanceReportDetail) {
    auto& manager = NodeSizeManager::get_instance();

    // 添加不同类型的节点数据
    std::vector<size_t> leaf_sizes = {80, 90, 85, 95, 88};
    manager.record_node_stats("leaf", leaf_sizes, 64);

    std::vector<size_t> internal_sizes = {150, 160, 155, 165, 158};
    manager.record_node_stats("internal", internal_sizes, 128);

    std::string report = manager.get_performance_report();

    // 验证报告包含所有必要信息
    EXPECT_NE(report.find("=== Node Size Manager Performance Report ==="), std::string::npos);
    EXPECT_NE(report.find("Node Type: leaf"), std::string::npos);
    EXPECT_NE(report.find("Node Type: internal"), std::string::npos);
    EXPECT_NE(report.find("Status: ACTIVE"), std::string::npos);

    // 验证数值信息
    EXPECT_NE(report.find("80.0"), std::string::npos); // leaf平均值
    EXPECT_NE(report.find("158.0"), std::string::npos); // internal平均值
}

// 测试大规模数据处理
TEST_F(NodeSizeManagerTest, LargeScaleData) {
    auto& manager = NodeSizeManager::get_instance();

    // 生成大规模测试数据
    std::vector<size_t> large_sizes;
    for (int i = 0; i < 1000; ++i) {
        large_sizes.push_back(50 + (i % 50)); // 50-99之间的值
    }

    // 分批记录数据（模拟多次操作）
    for (int batch = 0; batch < 10; ++batch) {
        std::vector<size_t> batch_data(large_sizes.begin() + batch * 100,
                                      large_sizes.begin() + (batch + 1) * 100);
        manager.record_node_stats("large_test", batch_data, 64);
    }

    // 验证大规模数据处理
    NodeSizeStats stats = manager.get_stats("large_test");
    EXPECT_EQ(stats.total_entries, 1000);
    EXPECT_EQ(stats.total_size, 75000); // 1000 * 75 (平均值)

    // 验证容量计算仍然有效
    size_t capacity = manager.get_recommended_capacity("large_test");
    EXPECT_GE(capacity, NodeSizeManager::MIN_CAPACITY);
    EXPECT_LE(capacity, NodeSizeManager::MAX_CAPACITY);
}

// 测试异常情况处理
TEST_F(NodeSizeManagerTest, ExceptionHandling) {
    auto& manager = NodeSizeManager::get_instance();

    // 测试不存在的节点类型
    NodeSizeStats empty_stats = manager.get_stats("nonexistent");
    EXPECT_EQ(empty_stats.total_entries, 0);

    size_t default_capacity = manager.get_recommended_capacity("nonexistent");
    EXPECT_EQ(default_capacity, NodeSizeManager::DEFAULT_CAPACITY);

    // 测试分裂判断（无统计数据的情况）
    bool should_split = manager.should_split_node("nonexistent", 100, 5000);
    EXPECT_TRUE(should_split); // 应该使用默认策略
}

// 测试多线程安全性（基本验证）
TEST_F(NodeSizeManagerTest, ThreadSafetyBasic) {
    auto& manager = NodeSizeManager::get_instance();

    // 在主线程中进行一些操作
    std::vector<size_t> sizes = {100, 110, 105};
    manager.record_node_stats("thread_test", sizes, 64);

    // 验证基本功能仍然正常
    NodeSizeStats stats = manager.get_stats("thread_test");
    EXPECT_EQ(stats.total_entries, 3);
    EXPECT_EQ(stats.total_size, 315);
}

} // namespace sqlcc
