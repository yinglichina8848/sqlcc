/**
 * @file abstract_strategy.h
 * @brief 抽象替换策略基类定义
 *
 * Why: 需要一个统一的抽象基类来定义页面替换策略的通用接口
 * What: AbstractReplaceStrategy类定义了页面替换策略的通用接口，支持多种替换算法
 * How: 提供纯虚函数供派生类实现，包含页面访问信息和统计信息的结构定义
 */

#pragma once

#include <unordered_map>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace sqlcc {

/**
 * @brief 抽象替换策略基类
 *
 * WHY层 - 设计意图：
 *   不同的数据库负载对缓存的局部性（Locality）有不同的表现。
 *   有些适合 LRU（最近最少使用），有些适合 LFU（最不经常使用），
 *   有些则需要更复杂的 ARC（自适应缓存）。
 *   抽象基类通过定义统一的 RecordAccess 和 SelectVictim 接口，
 *   使得 BufferPool 能够透明地切换各种算法，而无需修改核心逻辑。
 *
 * WHAT层 - 功能说明：
 *   定义页面替换算法的生命周期接口。
 *   管理页面元数据（PageAccessInfo），包括访问计数、时间戳、Pin 状态和脏标志。
 *   收集算法执行的统计信息（命中率、替换延迟等）。
 *
 * HOW层 - 实现机制：
 *   1. 状态追踪：使用 unordered_map 存储 page_id 到 PageAccessInfo 的映射。
 *   2. 线程安全：使用 page_info_mutex_ 保护并发的页面状态更新。
 *   3. 模板方法：派生类仅需实现核心的决策逻辑 SelectVictim()。
 *   4. 原子统计：stats_ 记录缓存表现，用于性能可视化。
 */
class AbstractReplaceStrategy {
public:
    /**
     * @brief 页面访问信息
     */
    struct PageAccessInfo {
        int32_t page_id;
        int32_t access_count;
        std::chrono::steady_clock::time_point last_access_time;
        bool is_dirty;
        int32_t pin_count;

        PageAccessInfo() : page_id(0), access_count(0), is_dirty(false), pin_count(0) {
            last_access_time = std::chrono::steady_clock::now();
        }

        explicit PageAccessInfo(int32_t id)
            : page_id(id), access_count(0), is_dirty(false), pin_count(0) {
            last_access_time = std::chrono::steady_clock::now();
        }
    };

    /**
     * @brief 策略统计信息
     */
    struct StrategyStats {
        size_t total_evictions = 0;      // 总替换次数
        size_t cache_hits = 0;           // 缓存命中次数
        size_t cache_misses = 0;         // 缓存未命中次数
        double hit_rate = 0.0;           // 命中率
        size_t dirty_evictions = 0;      // 脏页替换次数
        std::chrono::microseconds avg_eviction_time{0}; // 平均替换时间

        void UpdateHitRate() {
            size_t total = cache_hits + cache_misses;
            hit_rate = total > 0 ? static_cast<double>(cache_hits) * 100.0 / total : 0.0;
        }
    };

    /**
     * @brief 构造函数
     */
    explicit AbstractReplaceStrategy(const std::string& name);

    /**
     * @brief 析构函数
     */
    virtual ~AbstractReplaceStrategy() = default;

    /**
     * @brief 记录页面访问
     * @param page_id 页面ID
     * @param is_hit 是否命中
     * @param is_write 是否为写操作
     */
    virtual void RecordAccess(int32_t page_id, bool is_hit, bool is_write) = 0;

    /**
     * @brief 页面被钉住（引用计数增加）
     * @param page_id 页面ID
     */
    virtual void PinPage(int32_t page_id);

    /**
     * @brief 页面被释放（引用计数减少）
     * @param page_id 页面ID
     */
    virtual void UnpinPage(int32_t page_id);

    /**
     * @brief 页面被修改（标记为脏页）
     * @param page_id 页面ID
     */
    virtual void MarkDirty(int32_t page_id);

    /**
     * @brief 页面被清理（脏页状态清除）
     * @param page_id 页面ID
     */
    virtual void CleanPage(int32_t page_id);

    /**
     * @brief 选择要替换的页面
     * @return 被选中的页面ID，如果没有可替换的页面返回-1
     */
    virtual int32_t SelectVictim() = 0;

    /**
     * @brief 获取策略名称
     * @return 策略名称
     */
    const std::string& GetName() const { return name_; }

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    StrategyStats GetStats() const;

    /**
     * @brief 重置统计信息
     */
    void ResetStats();

    /**
     * @brief 获取页面访问信息
     * @param page_id 页面ID
     * @return 页面访问信息，如果页面不存在返回nullptr
     */
    PageAccessInfo* GetPageInfo(int32_t page_id);

    /**
     * @brief 移除页面信息
     * @param page_id 页面ID
     */
    virtual void RemovePage(int32_t page_id);

protected:
    /**
     * @brief 添加新页面
     * @param page_id 页面ID
     */
    virtual void AddPage(int32_t page_id);

    /**
     * @brief 页面访问信息映射
     */
    std::unordered_map<int32_t, PageAccessInfo> page_info_;

    /**
     * @brief 页面访问信息互斥锁
     */
    mutable std::mutex page_info_mutex_;

    /**
     * @brief 策略名称
     */
    const std::string name_;

    /**
     * @brief 统计信息
     */
    mutable StrategyStats stats_;

    /**
     * @brief 统计信息互斥锁
     */
    mutable std::mutex stats_mutex_;
};

} // namespace sqlcc
