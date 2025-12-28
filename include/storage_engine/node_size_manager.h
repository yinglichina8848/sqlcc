#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <mutex>

namespace sqlcc {

/**
 * @brief 节点大小统计信息
 */
struct NodeSizeStats {
    size_t total_entries = 0;
    size_t total_size = 0;
    double avg_entry_size = 0.0;
    double size_variance = 0.0;
    size_t min_entry_size = 0;
    size_t max_entry_size = 0;
    size_t recommended_node_capacity = 0;

    NodeSizeStats() = default;

    void update_stats(const std::vector<size_t>& entry_sizes);
    void calculate_recommendations();
};

/**
 * @brief 动态节点大小管理器
 *
 * 负责监控和动态调整B+树节点的大小策略，
 * 基于实际使用模式优化节点容量和内存使用。
 */
class NodeSizeManager {
public:
    /**
     * @brief 获取单例实例
     */
    static NodeSizeManager& get_instance();

    /**
     * @brief 记录节点大小信息
     * @param node_type 节点类型 ("leaf" 或 "internal")
     * @param entry_sizes 条目大小列表
     * @param current_capacity 当前节点容量
     */
    void record_node_stats(const std::string& node_type,
                          const std::vector<size_t>& entry_sizes,
                          size_t current_capacity);

    /**
     * @brief 获取推荐的节点容量
     * @param node_type 节点类型
     * @return 推荐的节点容量
     */
    size_t get_recommended_capacity(const std::string& node_type) const;

    /**
     * @brief 检查节点是否已满（动态容量）
     * @param node_type 节点类型
     * @param current_entries 当前条目数
     * @param current_size 当前总大小
     * @return 是否需要分裂
     */
    bool should_split_node(const std::string& node_type,
                          size_t current_entries,
                          size_t current_size) const;

    /**
     * @brief 获取节点大小统计信息
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
     * @brief 获取性能报告
     * @return 性能统计报告
     */
    std::string get_performance_report() const;

    // 配置参数 - 设为 public 以便外部访问
    static constexpr size_t MIN_CAPACITY = 4;      // 最小节点容量
    static constexpr size_t MAX_CAPACITY = 1024;   // 最大节点容量
    static constexpr size_t DEFAULT_CAPACITY = 64; // 默认节点容量
    static constexpr double SPLIT_THRESHOLD = 0.85; // 分裂阈值
    static constexpr size_t MIN_SAMPLES = 100;     // 最少采样次数

private:
    NodeSizeManager() = default;
    ~NodeSizeManager() = default;

    // 禁止拷贝和赋值
    NodeSizeManager(const NodeSizeManager&) = delete;
    NodeSizeManager& operator=(const NodeSizeManager&) = delete;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, NodeSizeStats> node_stats_;
    std::unordered_map<std::string, std::atomic<size_t>> sample_counts_;

    /**
     * @brief 计算动态容量
     * @param stats 统计信息
     * @return 推荐容量
     */
    size_t calculate_dynamic_capacity(const NodeSizeStats& stats) const;

    /**
     * @brief 更新统计信息
     * @param node_type 节点类型
     * @param entry_sizes 条目大小列表
     */
    void update_stats(const std::string& node_type, const std::vector<size_t>& entry_sizes);
};

} // namespace sqlcc
