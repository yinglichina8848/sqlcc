/**
 * @file statistics_collector.h
 * @brief 缓冲池统计收集器类定义
 *
 * Why: 需要一个专门的统计收集器来跟踪缓冲池的性能指标
 * What: StatisticsCollector类收集和报告缓冲池的各种性能统计信息
 * How: 使用原子操作和互斥锁来确保并发安全的数据收集
 */

#pragma once

#include <atomic>
#include <unordered_map>
#include <cstdint>
#include <mutex>

namespace sqlcc {
namespace storage {

/**
 * @brief 缓冲池统计收集器
 *
 * 收集缓冲池的各种性能统计信息，包括命中率、访问次数、替换次数等。
 * 提供线程安全的统计数据收集和报告功能。
 */
class StatisticsCollector {
public:
    /**
     * @brief 构造函数
     */
    StatisticsCollector();

    /**
     * @brief 析构函数
     */
    ~StatisticsCollector();

    /**
     * @brief 记录页面访问
     * @param page_id 页面ID
     */
    void RecordPageAccess(int32_t page_id);

    /**
     * @brief 记录页面命中
     */
    void RecordPageHit();

    /**
     * @brief 记录页面未命中
     */
    void RecordPageMiss();

    /**
     * @brief 记录页面替换
     */
    void RecordPageReplacement();

    /**
     * @brief 记录页面刷新（写回磁盘）
     */
    void RecordPageFlush();

    /**
     * @brief 获取总访问次数
     * @return 总访问次数
     */
    uint64_t GetTotalAccesses() const;

    /**
     * @brief 获取总命中次数
     * @return 总命中次数
     */
    uint64_t GetTotalHits() const;

    /**
     * @brief 获取命中率
     * @return 命中率（0.0-1.0）
     */
    double GetHitRate() const;

    /**
     * @brief 获取页面访问频率统计
     * @return 页面ID到访问次数的映射
     */
    std::unordered_map<int32_t, uint64_t> GetAccessFrequency() const;

    /**
     * @brief 获取替换次数
     * @return 替换次数
     */
    uint64_t GetReplacementCount() const;

    /**
     * @brief 获取刷新次数
     * @return 刷新次数
     */
    uint64_t GetFlushCount() const;

    /**
     * @brief 重置所有统计信息
     */
    void Reset();

    /**
     * @brief 获取统计信息的字符串表示
     * @return 统计信息字符串
     */
    std::string GetStatisticsString() const;

private:
    // 基本统计计数器
    std::atomic<uint64_t> total_accesses_;    ///< 总访问次数
    std::atomic<uint64_t> total_hits_;        ///< 总命中次数
    std::atomic<uint64_t> replacement_count_; ///< 替换次数
    std::atomic<uint64_t> flush_count_;       ///< 刷新次数

    // 页面访问频率统计
    mutable std::mutex access_stats_mutex_;   ///< 访问统计互斥锁
    std::unordered_map<int32_t, uint64_t> access_frequency_; ///< 页面访问频率
};

} // namespace storage
} // namespace sqlcc
