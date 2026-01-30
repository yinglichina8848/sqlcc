#include "src/storage_engine/node_size_manager.h"
#include "src/utils/logger.h"
#include "src/page/page.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace sqlcc {

void NodeSizeStats::update_stats(const std::vector<size_t>& entry_sizes) {
    if (entry_sizes.empty()) {
        return;
    }

    total_entries = entry_sizes.size();
    total_size = 0;
    min_entry_size = *std::min_element(entry_sizes.begin(), entry_sizes.end());
    max_entry_size = *std::max_element(entry_sizes.begin(), entry_sizes.end());

    // 计算平均值和方差
    double sum = 0.0;
    for (size_t size : entry_sizes) {
        total_size += size;
        sum += size;
    }
    avg_entry_size = sum / total_entries;

    // 计算方差
    double variance_sum = 0.0;
    for (size_t size : entry_sizes) {
        double diff = size - avg_entry_size;
        variance_sum += diff * diff;
    }
    size_variance = variance_sum / total_entries;

    calculate_recommendations();
}

void NodeSizeStats::calculate_recommendations() {
    // 基于统计信息计算推荐的节点容量
    // 使用标准差来确定容量范围
    double std_dev = std::sqrt(size_variance);

    // 推荐容量 = 平均条目大小 * 目标节点大小 / 平均条目大小
    // 目标节点大小基于页面大小和填充因子
    size_t target_node_bytes = PAGE_SIZE * 0.8; // 80%填充因子
    double avg_size_with_overhead = avg_entry_size + 32; // 考虑元数据开销

    if (avg_size_with_overhead > 0 && avg_entry_size > 0) {
        recommended_node_capacity = static_cast<size_t>(
            target_node_bytes / avg_size_with_overhead
        );

        // 考虑方差调整：如果方差很大，稍微降低容量以避免溢出
        if (std_dev > avg_entry_size * 0.5) { // 方差较大
            recommended_node_capacity = static_cast<size_t>(
                recommended_node_capacity * 0.9
            );
        }

        // 限制在合理范围内
        recommended_node_capacity = std::max(NodeSizeManager::MIN_CAPACITY,
            std::min(NodeSizeManager::MAX_CAPACITY, recommended_node_capacity));
    } else {
        recommended_node_capacity = NodeSizeManager::DEFAULT_CAPACITY;
    }
}

NodeSizeManager& NodeSizeManager::get_instance() {
    static NodeSizeManager instance;
    return instance;
}

void NodeSizeManager::record_node_stats(const std::string& node_type,
                                       const std::vector<size_t>& entry_sizes,
                                       size_t current_capacity) {
    std::lock_guard<std::mutex> lock(mutex_);
    update_stats(node_type, entry_sizes);
}

size_t NodeSizeManager::get_recommended_capacity(const std::string& node_type) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = node_stats_.find(node_type);
    if (it != node_stats_.end()) {
        size_t samples = sample_counts_.at(node_type).load();
        if (samples >= NodeSizeManager::MIN_SAMPLES) {
            return it->second.recommended_node_capacity;
        }
    }

    return NodeSizeManager::DEFAULT_CAPACITY;
}

bool NodeSizeManager::should_split_node(const std::string& node_type,
                                       size_t current_entries,
                                       size_t current_size) const {
    std::lock_guard<std::mutex> lock(mutex_);

    // 获取推荐容量
    size_t recommended_capacity = get_recommended_capacity(node_type);

    // 如果有足够的统计数据，使用动态容量检查
    auto it = node_stats_.find(node_type);
    if (it != node_stats_.end() && sample_counts_.at(node_type).load() >= NodeSizeManager::MIN_SAMPLES) {
        const NodeSizeStats& stats = it->second;

        // 基于平均条目大小估算容量
        double estimated_capacity = static_cast<double>(PAGE_SIZE * SPLIT_THRESHOLD) /
                                   (stats.avg_entry_size + 32); // 32字节元数据开销

        size_t effective_capacity = std::max(recommended_capacity,
            static_cast<size_t>(estimated_capacity));

        return current_entries >= effective_capacity;
    }

    // 默认策略：基于固定阈值
    return current_size >= (PAGE_SIZE * SPLIT_THRESHOLD);
}

NodeSizeStats NodeSizeManager::get_stats(const std::string& node_type) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = node_stats_.find(node_type);
    if (it != node_stats_.end()) {
        return it->second;
    }

    return NodeSizeStats{};
}

void NodeSizeManager::reset_stats(const std::string& node_type) {
    std::lock_guard<std::mutex> lock(mutex_);

    node_stats_.erase(node_type);
    sample_counts_[node_type] = 0;
}

std::string NodeSizeManager::get_performance_report() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::stringstream ss;

    ss << "=== Node Size Manager Performance Report ===\n\n";

    for (const auto& [node_type, stats] : node_stats_) {
        size_t samples = sample_counts_.at(node_type).load();

        ss << "Node Type: " << node_type << "\n";
        ss << "  Samples Collected: " << samples << "\n";
        ss << "  Total Entries: " << stats.total_entries << "\n";
        ss << "  Total Size: " << stats.total_size << " bytes\n";
        ss << std::fixed << std::setprecision(2);
        ss << "  Average Entry Size: " << stats.avg_entry_size << " bytes\n";
        ss << "  Size Variance: " << stats.size_variance << "\n";
        ss << "  Min/Max Entry Size: " << stats.min_entry_size
           << "/" << stats.max_entry_size << " bytes\n";
        ss << "  Recommended Capacity: " << stats.recommended_node_capacity << "\n";

    if (samples >= NodeSizeManager::MIN_SAMPLES) {
        ss << "  Status: ACTIVE (using dynamic sizing)\n";
    } else {
        ss << "  Status: LEARNING (using default sizing)\n";
    }

        ss << "\n";
    }

    if (node_stats_.empty()) {
        ss << "No statistics collected yet.\n";
    }

    return ss.str();
}

size_t NodeSizeManager::calculate_dynamic_capacity(const NodeSizeStats& stats) const {
    // 基于统计信息计算动态容量
    if (stats.total_entries == 0) {
        return NodeSizeManager::DEFAULT_CAPACITY;
    }

    // 目标页面利用率
    const double target_utilization = 0.8;

    // 考虑元数据开销（键长度、页面ID、偏移量等）
    const size_t metadata_overhead_per_entry = 20; // 估算值

    // 计算每个条目的平均总大小
    double avg_total_size = stats.avg_entry_size + metadata_overhead_per_entry;

    if (avg_total_size <= 0) {
        return NodeSizeManager::DEFAULT_CAPACITY;
    }

    // 计算推荐容量
    double recommended = (PAGE_SIZE * target_utilization) / avg_total_size;

    // 考虑方差调整容量（如果方差很大，稍微降低容量以避免溢出）
    double std_dev = std::sqrt(stats.size_variance);
    double variance_factor = 1.0 - std::min(0.2, std_dev / stats.avg_entry_size);

    recommended *= variance_factor;

    // 确保在合理范围内
    size_t result = static_cast<size_t>(recommended);
    result = std::max(NodeSizeManager::MIN_CAPACITY, std::min(NodeSizeManager::MAX_CAPACITY, result));

    return result;
}

void NodeSizeManager::update_stats(const std::string& node_type,
                                  const std::vector<size_t>& entry_sizes) {
    // 更新统计信息
    auto& stats = node_stats_[node_type];
    stats.update_stats(entry_sizes);

    // 更新采样计数
    sample_counts_[node_type]++;

    // 重新计算推荐容量
    stats.recommended_node_capacity = calculate_dynamic_capacity(stats);

    SQLCC_LOG_DEBUG("Updated stats for node type '" + node_type +
                   "': samples=" + std::to_string(sample_counts_[node_type].load()) +
                   ", recommended_capacity=" + std::to_string(stats.recommended_node_capacity));
}

} // namespace sqlcc
