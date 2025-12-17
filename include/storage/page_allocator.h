/**
 * @file page_allocator.h
 * @brief 页面分配器头文件 - 智能页面分配算法优化
 */

#ifndef SQLCC_PAGE_ALLOCATOR_H
#define SQLCC_PAGE_ALLOCATOR_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <deque>

namespace sqlcc {

class Page;

// 页面类型枚举
enum class PageType {
    DATA = 0,       // 数据页面
    INDEX = 1,      // 索引页面
    METADATA = 2,   // 元数据页面
    LOG = 3,        // 日志页面
    TEMP = 4        // 临时页面
};

// 页面分配策略枚举
enum class PageAllocationStrategy {
    SEQUENTIAL = 0,     // 顺序分配
    RANDOM = 1,         // 随机分配
    PREDICTIVE = 2,     // 预测性分配
    MEMORY_AWARE = 3    // 内存感知分配
};

// 页面分配统计信息
struct PageAllocationStats {
    size_t total_allocations = 0;
    size_t total_deallocations = 0;
    size_t peak_pages_used = 0;
    size_t allocation_failures = 0;
};

// 页面分配信息
struct PageAllocationInfo {
    int32_t page_id;
    PageType page_type;
    std::chrono::steady_clock::time_point allocation_time;
    std::chrono::steady_clock::time_point last_access_time;
    size_t access_count;
    bool is_dirty;
};

// 页面使用统计信息
struct PageUsageStats {
    size_t total_pages = 0;
    size_t peak_pages = 0;
    size_t available_pages = 0;
    std::unordered_map<PageType, size_t> page_type_distribution;
    double average_page_lifetime_ms = 0.0;
};

// 访问模式分析结果
struct AccessPatternAnalysis {
    size_t total_accesses = 0;
    bool has_sequential_access = false;
    bool has_predictable_access = false;
    int32_t most_accessed_page = -1;
};

// 内存统计信息
struct MemoryStats {
    size_t total_memory_mb = 0;
    size_t used_memory_mb = 0;
    size_t available_memory_mb = 0;
    double memory_usage_percent = 0.0;
};

// 页面分配器类
class PageAllocator {
public:
    PageAllocator(size_t max_pages, PageAllocationStrategy strategy = PageAllocationStrategy::SEQUENTIAL);
    ~PageAllocator();

    // 页面分配和释放
    std::shared_ptr<Page> AllocatePage(PageType type = PageType::DATA, int32_t hint_page_id = -1);
    bool DeallocatePage(int32_t page_id);
    std::shared_ptr<Page> GetPage(int32_t page_id);

    // 页面状态管理
    void MarkPageDirty(int32_t page_id);
    bool IsPageDirty(int32_t page_id) const;

    // 统计信息
    PageAllocationStats GetAllocationStats() const;
    PageUsageStats GetPageUsageStats() const;
    AccessPatternAnalysis GetAccessPatternAnalysis() const;

    // 配置管理
    void SetAllocationStrategy(PageAllocationStrategy strategy);
    void OptimizeMemoryUsage();

private:
    // 私有分配策略实现
    std::shared_ptr<Page> AllocateSequential(PageType type);
    std::shared_ptr<Page> AllocateRandom(PageType type);
    std::shared_ptr<Page> AllocatePredictive(PageType type, int32_t hint_page_id);
    std::shared_ptr<Page> AllocateMemoryAware(PageType type);

    // 页面驱逐
    bool TryEvictPages();

    // 成员变量
    size_t max_pages_;
    size_t current_pages_;
    PageAllocationStrategy allocation_strategy_;
    PageAllocationStats allocation_stats_;

    std::vector<std::shared_ptr<Page>> page_pool_;
    std::unordered_map<int32_t, PageAllocationInfo> page_info_;
    std::vector<std::chrono::milliseconds> lifetime_stats_;

    mutable std::mutex allocator_mutex_;

    // 访问模式分析器
    class AccessPatternAnalyzer {
    public:
        AccessPatternAnalyzer();
        ~AccessPatternAnalyzer() = default;

        // 记录访问模式
        void RecordAllocation(int32_t page_id, PageType type);
        void RecordAccess(int32_t page_id);
        void RecordDeallocation(int32_t page_id);

        // 预测和分析
        int32_t PredictNextPageId(int32_t current_page_id) const;
        AccessPatternAnalysis AnalyzePatterns() const;

    private:
        struct AllocationRecord {
            int32_t page_id;
            PageType page_type;
            std::chrono::steady_clock::time_point allocation_time;
        };

        struct AccessRecord {
            int32_t page_id;
            std::chrono::steady_clock::time_point access_time;
        };

        struct PairHash {
            template <class T1, class T2>
            size_t operator()(const std::pair<T1, T2>& pair) const {
                return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
            }
        };

        size_t access_window_size_;
        double sequential_threshold_;
        double predictability_threshold_;

        std::vector<AllocationRecord> allocation_history_;
        std::vector<AccessRecord> access_history_;

        mutable std::mutex pattern_mutex_;
    };
    std::unique_ptr<AccessPatternAnalyzer> access_pattern_analyzer_;

    // 内存监视器
    class MemoryMonitor {
    public:
        MemoryMonitor() = default;
        ~MemoryMonitor() = default;

        MemoryStats GetMemoryStats() const;
    };
    std::unique_ptr<MemoryMonitor> memory_monitor_;
};

} // namespace sqlcc

#endif // SQLCC_PAGE_ALLOCATOR_H
