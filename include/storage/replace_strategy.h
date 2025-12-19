#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>
#include <list>
#include <mutex>
#include <string>

namespace sqlcc {

/**
 * @brief 抽象替换策略基类
 * 
 * 定义了页面替换策略的通用接口，支持多种替换算法。
 * 派生类需要实现具体的替换逻辑。
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
        
        PageAccessInfo(int32_t id) 
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

/**
 * @brief LRU（最近最少使用）替换策略
 */
class LRUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LRUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LRUReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief LRU链表：最近使用的页面在头部
     */
    std::list<int32_t> lru_list_;

    /**
     * @brief 页面ID到LRU链表迭代器的映射
     */
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;

    /**
     * @brief 更新LRU列表
     * @param page_id 页面ID
     */
    void UpdateLRU(int32_t page_id);
};

/**
 * @brief LFU（最不经常使用）替换策略
 */
class LFUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LFUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LFUReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief 访问频率列表：频率低的页面在头部
     */
    std::list<int32_t> frequency_list_;

    /**
     * @brief 页面ID到频率列表迭代器的映射
     */
    std::unordered_map<int32_t, std::list<int32_t>::iterator> frequency_map_;
};

/**
 * @brief CLOCK（时钟）替换策略
 */
class ClockReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    ClockReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~ClockReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief 页面引用位
     */
    std::unordered_map<int32_t, bool> reference_bits_;

    /**
     * @brief 时钟指针
     */
    std::list<int32_t>::iterator clock_hand_;
    std::list<int32_t> clock_list_;
};

/**
 * @brief 替换策略工厂
 */
class ReplaceStrategyFactory {
public:
    /**
     * @brief 策略类型枚举
     */
    enum class StrategyType {
        LRU,     // 最近最少使用
        LFU,     // 最不经常使用
        CLOCK,   // 时钟算法
        ARC,     // 自适应替换缓存
        FIFO     // 先进先出
    };

    /**
     * @brief 创建替换策略实例
     * @param type 策略类型
     * @return 策略实例
     */
    static std::unique_ptr<AbstractReplaceStrategy> CreateStrategy(StrategyType type);

    /**
     * @brief 获取策略类型对应的名称
     * @param type 策略类型
     * @return 策略名称
     */
    static std::string GetStrategyName(StrategyType type);

    /**
     * @brief 根据名称获取策略类型
     * @param name 策略名称
     * @return 策略类型
     */
    static StrategyType GetStrategyType(const std::string& name);
};

/**
 * @brief ARC（自适应替换缓存）替换策略
 * 
 * 结合了LRU和LFU的优点，自适应地在两种策略之间切换。
 */
class ARCReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     * @param p 初始T1和T2的大小参数（默认：总缓存大小的1/32）
     * @param total_size 总缓存大小
     */
    ARCReplaceStrategy(size_t p, size_t total_size);

    /**
     * @brief 析构函数
     */
    ~ARCReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief T1列表：最近访问的页面
     */
    std::list<int32_t> t1_list_;

    /**
     * @brief T2列表：经常访问的页面
     */
    std::list<int32_t> t2_list_;

    /**
     * @brief B1列表：最近被替换出T1的页面
     */
    std::list<int32_t> b1_list_;

    /**
     * @brief B2列表：最近被替换出T2的页面
     */
    std::list<int32_t> b2_list_;

    /**
     * @brief P值：T1的目标大小
     */
    size_t p_;

    /**
     * @brief 总缓存大小
     */
    const size_t total_size_;

    /**
     * @brief 获取迭代器的工具函数
     */
    std::list<int32_t>::iterator GetIterator(const std::string& list_name, int32_t page_id);
};

} // namespace sqlcc
