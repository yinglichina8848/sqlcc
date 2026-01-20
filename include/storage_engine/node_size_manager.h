#ifndef STORAGE_ENGINE_NODE_SIZE_MANAGER_H
#define STORAGE_ENGINE_NODE_SIZE_MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>
#include <memory>

namespace sqlcc {

class StorageEngine;

/**
 * @brief 节点大小统计信息
 *
 * 用于收集和分析B+树节点的大小统计信息，
 * 基于这些统计信息动态调整节点容量
 */
struct NodeSizeStats {
    size_t total_entries = 0;        // 总条目数
    size_t total_size = 0;           // 总大小（字节）
    size_t avg_entry_size = 0;       // 平均条目大小
    size_t min_entry_size = 0;       // 最小条目大小
    size_t max_entry_size = 0;       // 最大条目大小
    double size_variance = 0.0;      // 大小方差
    size_t recommended_node_capacity = 0;  // 推荐的节点容量

    /**
     * @brief 更新统计信息
     * @param entry_sizes 条目大小向量
     */
    void update_stats(const std::vector<size_t>& entry_sizes);

    /**
     * @brief 计算推荐容量
     */
    void calculate_recommendations();
};

/**
 * @brief 节点大小管理器
 *
 * 动态管理B+树节点大小，基于运行时统计信息
 * 优化节点分裂和合并策略
 */
class NodeSizeManager {
public:
    // 常量定义
    static constexpr size_t MIN_CAPACITY = 4;      // 最小节点容量
    static constexpr size_t MAX_CAPACITY = 4096;   // 最大节点容量
    static constexpr size_t DEFAULT_CAPACITY = 256; // 默认节点容量
    static constexpr size_t MIN_SAMPLES = 10;      // 最小采样数
    static constexpr double SPLIT_THRESHOLD = 0.8; // 分裂阈值

    /**
     * @brief 获取单例实例
     * @return NodeSizeManager实例
     */
    static NodeSizeManager& get_instance();

    /**
     * @brief 记录节点统计信息
     * @param node_type 节点类型
     * @param entry_sizes 条目大小向量
     * @param current_capacity 当前容量
     */
    void record_node_stats(const std::string& node_type,
                          const std::vector<size_t>& entry_sizes,
                          size_t current_capacity);

    /**
     * @brief 获取推荐容量
     * @param node_type 节点类型
     * @return 推荐的节点容量
     */
    size_t get_recommended_capacity(const std::string& node_type) const;

    /**
     * @brief 判断是否需要分裂节点
     * @param node_type 节点类型
     * @param current_entries 当前条目数
     * @param current_size 当前大小
     * @return 是否需要分裂
     */
    bool should_split_node(const std::string& node_type,
                          size_t current_entries,
                          size_t current_size) const;

    /**
     * @brief 获取统计信息
     * @param node_type 节点类型
     * @return 统计信息
     */
    NodeSizeStats get_stats(const std::string& node_type) const;

    /**
     * @brief 重置统计信息
     * @param node_type 节点类型
     */
    void reset_stats(const std::string& node_type);

    /**
     * @brief 生成性能报告
     * @return 性能报告字符串
     */
    std::string get_performance_report() const;

private:
    NodeSizeManager() = default;
    ~NodeSizeManager() = default;
    NodeSizeManager(const NodeSizeManager&) = delete;
    NodeSizeManager& operator=(const NodeSizeManager&) = delete;

    /**
     * @brief 计算动态容量
     * @param stats 统计信息
     * @return 动态计算的容量
     */
    size_t calculate_dynamic_capacity(const NodeSizeStats& stats) const;

    /**
     * @brief 更新统计信息
     * @param node_type 节点类型
     * @param entry_sizes 条目大小向量
     */
    void update_stats(const std::string& node_type,
                     const std::vector<size_t>& entry_sizes);

    // 成员变量
    mutable std::mutex mutex_;  // 互斥锁
    std::unordered_map<std::string, NodeSizeStats> node_stats_;  // 节点统计信息
    std::unordered_map<std::string, std::atomic<size_t>> sample_counts_;  // 采样计数
};

} // namespace sqlcc

#endif // STORAGE_ENGINE_NODE_SIZE_MANAGER_H
