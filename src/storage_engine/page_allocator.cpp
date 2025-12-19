/**
 * @file page_allocator.cpp
 * @brief 页面分配器实现 - 智能页面分配算法优化
 *
 * 该文件实现了智能页面分配器，提供：
 * - 基于访问模式的预测性分配
 * - 内存使用优化
 * - 并发安全分配
 * - 页面生命周期管理
 */

#include "storage/page_allocator.h"
#include "page.h"
#include "exception.h"
#include "utils/logger.h"
#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 页面分配器实现
PageAllocator::PageAllocator(size_t max_pages, PageAllocationStrategy strategy)
    : max_pages_(max_pages),
      current_pages_(0),
      allocation_strategy_(strategy),
      access_pattern_analyzer_(std::make_unique<AccessPatternAnalyzer>()),
      memory_monitor_(std::make_unique<MemoryMonitor>()) {

    // 初始化分配统计
    allocation_stats_.total_allocations = 0;
    allocation_stats_.total_deallocations = 0;
    allocation_stats_.peak_pages_used = 0;
    allocation_stats_.allocation_failures = 0;

    // 初始化页面池
    page_pool_.reserve(max_pages_);

    SQLCC_LOG_INFO("PageAllocator initialized with max_pages=" +
                   std::to_string(max_pages_) + ", strategy=" +
                   std::to_string(static_cast<int>(strategy)));
}

PageAllocator::~PageAllocator() {
    // 清理所有页面
    std::lock_guard<std::mutex> lock(allocator_mutex_);
    for (auto& page : page_pool_) {
        if (page) {
            // 确保页面被正确清理
            page.reset();
        }
    }
    page_pool_.clear();

    std::stringstream ss;
    ss << "PageAllocator destroyed, final stats: "
       << "allocations=" << allocation_stats_.total_allocations
       << ", deallocations=" << allocation_stats_.total_deallocations
       << ", failures=" << allocation_stats_.allocation_failures;
    SQLCC_LOG_INFO(ss.str());
}

// 分配新页面
std::shared_ptr<Page> PageAllocator::AllocatePage(PageType type, int32_t hint_page_id) {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    // 检查是否超过最大页面限制
    if (current_pages_ >= max_pages_) {
        // 尝试释放一些页面
        if (!TryEvictPages()) {
            allocation_stats_.allocation_failures++;
            SQLCC_LOG_WARN("Page allocation failed: max pages limit reached (" +
                          std::to_string(max_pages_) + ")");
            return nullptr;
        }
    }

    // 根据策略选择分配方法
    std::shared_ptr<Page> page;
    switch (allocation_strategy_) {
        case PageAllocationStrategy::SEQUENTIAL:
            page = AllocateSequential(type);
            break;
        case PageAllocationStrategy::RANDOM:
            page = AllocateRandom(type);
            break;
        case PageAllocationStrategy::PREDICTIVE:
            page = AllocatePredictive(type, hint_page_id);
            break;
        case PageAllocationStrategy::MEMORY_AWARE:
            page = AllocateMemoryAware(type);
            break;
        default:
            page = AllocateSequential(type);
            break;
    }

    if (page) {
        current_pages_++;
        allocation_stats_.total_allocations++;
        allocation_stats_.peak_pages_used = std::max(allocation_stats_.peak_pages_used, current_pages_);

        // 记录页面分配信息
        PageAllocationInfo info;
        info.page_id = page->GetPageId();
        info.page_type = type;
        info.allocation_time = std::chrono::steady_clock::now();
        info.last_access_time = info.allocation_time;
        info.access_count = 0;
        info.is_dirty = false;

        page_info_[info.page_id] = info;

        // 更新访问模式分析器
        access_pattern_analyzer_->RecordAllocation(info.page_id, type);

        SQLCC_LOG_DEBUG("Page allocated: id=" + std::to_string(info.page_id) +
                       ", type=" + std::to_string(static_cast<int>(type)) +
                       ", strategy=" + std::to_string(static_cast<int>(allocation_strategy_)));
    }

    return page;
}

// 释放页面
bool PageAllocator::DeallocatePage(int32_t page_id) {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    // 查找页面信息
    auto info_it = page_info_.find(page_id);
    if (info_it == page_info_.end()) {
        SQLCC_LOG_WARN("Attempted to deallocate unknown page: " + std::to_string(page_id));
        return false;
    }

    // 从页面池中移除
    auto page_it = std::find_if(page_pool_.begin(), page_pool_.end(),
                               [page_id](const std::shared_ptr<Page>& page) {
                                   return page && page->GetPageId() == page_id;
                               });

    if (page_it != page_pool_.end()) {
        // 更新统计信息
        allocation_stats_.total_deallocations++;
        current_pages_--;

        // 计算页面生命周期
        auto now = std::chrono::steady_clock::now();
        auto lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - info_it->second.allocation_time);

        // 记录页面生命周期统计
        lifetime_stats_.emplace_back(lifetime);

        // 清理页面信息
        page_info_.erase(info_it);
        page_pool_.erase(page_it);

        // 更新访问模式分析器
        access_pattern_analyzer_->RecordDeallocation(page_id);

        SQLCC_LOG_DEBUG("Page deallocated: id=" + std::to_string(page_id) +
                       ", lifetime=" + std::to_string(lifetime.count()) + "ms");

        return true;
    }

    return false;
}

// 获取页面
std::shared_ptr<Page> PageAllocator::GetPage(int32_t page_id) {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    // 更新访问统计
    auto info_it = page_info_.find(page_id);
    if (info_it != page_info_.end()) {
        info_it->second.last_access_time = std::chrono::steady_clock::now();
        info_it->second.access_count++;
    }

    // 查找页面
    auto page_it = std::find_if(page_pool_.begin(), page_pool_.end(),
                               [page_id](const std::shared_ptr<Page>& page) {
                                   return page && page->GetPageId() == page_id;
                               });

    if (page_it != page_pool_.end()) {
        // 更新访问模式分析器
        access_pattern_analyzer_->RecordAccess(page_id);
        return *page_it;
    }

    return nullptr;
}

// 顺序分配策略
std::shared_ptr<Page> PageAllocator::AllocateSequential(PageType type) {
    (void)type; // 避免未使用参数警告
    // 使用递增的页面ID
    static int32_t next_page_id = 0;
    int32_t page_id = next_page_id++;

    auto page = std::make_shared<Page>(page_id);
    page_pool_.push_back(page);

    return page;
}

// 随机分配策略
std::shared_ptr<Page> PageAllocator::AllocateRandom(PageType type) {
    (void)type; // 避免未使用参数警告
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int32_t> dis(0, INT32_MAX);

    int32_t page_id = dis(gen);

    // 确保页面ID不重复
    while (page_info_.find(page_id) != page_info_.end()) {
        page_id = dis(gen);
    }

    auto page = std::make_shared<Page>(page_id);
    page_pool_.push_back(page);

    return page;
}

// 预测性分配策略
std::shared_ptr<Page> PageAllocator::AllocatePredictive(PageType type, int32_t hint_page_id) {
    // 基于访问模式预测最佳页面ID
    int32_t predicted_page_id = access_pattern_analyzer_->PredictNextPageId(hint_page_id);

    if (predicted_page_id == -1 || page_info_.find(predicted_page_id) != page_info_.end()) {
        // 预测失败或页面已存在，使用顺序分配作为fallback
        return AllocateSequential(type);
    }

    auto page = std::make_shared<Page>(predicted_page_id);
    page_pool_.push_back(page);

    return page;
}

// 内存感知分配策略
std::shared_ptr<Page> PageAllocator::AllocateMemoryAware(PageType type) {
    // 检查内存使用情况
    MemoryStats mem_stats = memory_monitor_->GetMemoryStats();

    // 如果内存使用率高，使用更保守的分配策略
    if (mem_stats.memory_usage_percent > 80.0) {
        // 内存紧张时优先释放一些页面
        if (!TryEvictPages()) {
            SQLCC_LOG_WARN("Memory usage high (" +
                          std::to_string(mem_stats.memory_usage_percent) +
                          "%), allocation may fail");
        }
    }

    // 根据页面类型和内存情况选择分配策略
    if (type == PageType::INDEX && mem_stats.available_memory_mb < 100) {
        // 索引页面在内存不足时优先释放
        return nullptr;
    }

    return AllocateSequential(type);
}

// 尝试驱逐页面
bool PageAllocator::TryEvictPages() {
    // 使用LRU策略驱逐页面
    std::vector<int32_t> evict_candidates;

    // 选择最久未访问的页面
    for (const auto& pair : page_info_) {
        const PageAllocationInfo& info = pair.second;
        if (info.access_count == 0) {
            // 从未访问的页面优先驱逐
            evict_candidates.push_back(pair.first);
        }
    }

    // 如果没有从未访问的页面，选择最久未访问的
    if (evict_candidates.empty()) {
        auto oldest_it = std::min_element(
            page_info_.begin(), page_info_.end(),
            [](const auto& a, const auto& b) {
                return a.second.last_access_time < b.second.last_access_time;
            });

        if (oldest_it != page_info_.end()) {
            evict_candidates.push_back(oldest_it->first);
        }
    }

    // 驱逐候选页面
    size_t evicted_count = 0;
    for (int32_t page_id : evict_candidates) {
        if (DeallocatePage(page_id)) {
            evicted_count++;
            if (current_pages_ < max_pages_ * 0.9) { // 驱逐到90%以下即可
                break;
            }
        }
    }

    return evicted_count > 0;
}

// 标记页面为脏页
void PageAllocator::MarkPageDirty(int32_t page_id) {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    auto it = page_info_.find(page_id);
    if (it != page_info_.end()) {
        it->second.is_dirty = true;
    }
}

// 检查页面是否为脏页
bool PageAllocator::IsPageDirty(int32_t page_id) const {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    auto it = page_info_.find(page_id);
    return it != page_info_.end() && it->second.is_dirty;
}

// 获取分配统计信息
PageAllocationStats PageAllocator::GetAllocationStats() const {
    std::lock_guard<std::mutex> lock(allocator_mutex_);
    return allocation_stats_;
}

// 获取页面使用情况
PageUsageStats PageAllocator::GetPageUsageStats() const {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    PageUsageStats stats;
    stats.total_pages = current_pages_;
    stats.peak_pages = allocation_stats_.peak_pages_used;
    stats.available_pages = max_pages_ - current_pages_;

    // 计算页面类型分布
    for (const auto& pair : page_info_) {
        stats.page_type_distribution[pair.second.page_type]++;
    }

    // 计算平均页面生命周期
    if (!lifetime_stats_.empty()) {
        auto total_lifetime = std::accumulate(
            lifetime_stats_.begin(), lifetime_stats_.end(),
            std::chrono::milliseconds(0));
        stats.average_page_lifetime_ms =
            total_lifetime.count() / lifetime_stats_.size();
    }

    return stats;
}

// 获取访问模式分析结果
AccessPatternAnalysis PageAllocator::GetAccessPatternAnalysis() const {
    return access_pattern_analyzer_->AnalyzePatterns();
}

// 设置分配策略
void PageAllocator::SetAllocationStrategy(PageAllocationStrategy strategy) {
    std::lock_guard<std::mutex> lock(allocator_mutex_);
    allocation_strategy_ = strategy;

    SQLCC_LOG_INFO("Page allocation strategy changed to: " +
                   std::to_string(static_cast<int>(strategy)));
}

// 优化内存使用
void PageAllocator::OptimizeMemoryUsage() {
    std::lock_guard<std::mutex> lock(allocator_mutex_);

    // 分析访问模式
    AccessPatternAnalysis analysis = access_pattern_analyzer_->AnalyzePatterns();

    // 根据分析结果调整策略
    if (analysis.has_sequential_access) {
        // 顺序访问模式，使用顺序分配
        allocation_strategy_ = PageAllocationStrategy::SEQUENTIAL;
    } else if (analysis.has_predictable_access) {
        // 可预测访问模式，使用预测性分配
        allocation_strategy_ = PageAllocationStrategy::PREDICTIVE;
    } else {
        // 随机访问模式，使用内存感知分配
        allocation_strategy_ = PageAllocationStrategy::MEMORY_AWARE;
    }

    // 主动驱逐不常用的页面
    size_t target_pages = max_pages_ * 0.8; // 目标使用80%的容量
    while (current_pages_ > target_pages && TryEvictPages()) {
        // 继续驱逐直到达到目标
    }

    SQLCC_LOG_INFO("Memory optimization completed: strategy=" +
                   std::to_string(static_cast<int>(allocation_strategy_)) +
                   ", pages_after=" + std::to_string(current_pages_));
}

// 访问模式分析器实现
PageAllocator::AccessPatternAnalyzer::AccessPatternAnalyzer()
    : access_window_size_(1000),
      sequential_threshold_(0.8),
      predictability_threshold_(0.6) {
}

void PageAllocator::AccessPatternAnalyzer::RecordAllocation(int32_t page_id, PageType type) {
    std::lock_guard<std::mutex> lock(pattern_mutex_);

    AllocationRecord record;
    record.page_id = page_id;
    record.page_type = type;
    record.allocation_time = std::chrono::steady_clock::now();

    allocation_history_.push_back(record);

    // 限制历史记录大小
    if (allocation_history_.size() > access_window_size_) {
        allocation_history_.erase(allocation_history_.begin());
    }
}

void PageAllocator::AccessPatternAnalyzer::RecordAccess(int32_t page_id) {
    std::lock_guard<std::mutex> lock(pattern_mutex_);

    AccessRecord record;
    record.page_id = page_id;
    record.access_time = std::chrono::steady_clock::now();

    access_history_.push_back(record);

    // 限制历史记录大小
    if (access_history_.size() > access_window_size_) {
        access_history_.erase(access_history_.begin());
    }
}

void PageAllocator::AccessPatternAnalyzer::RecordDeallocation(int32_t page_id) {
    std::lock_guard<std::mutex> lock(pattern_mutex_);

    // 从历史记录中移除
    allocation_history_.erase(
        std::remove_if(allocation_history_.begin(), allocation_history_.end(),
                      [page_id](const AllocationRecord& record) {
                          return record.page_id == page_id;
                      }),
        allocation_history_.end());
}

int32_t PageAllocator::AccessPatternAnalyzer::PredictNextPageId(int32_t current_page_id) const {
    std::lock_guard<std::mutex> lock(pattern_mutex_);

    if (access_history_.size() < 2) {
        return -1; // 没有足够的历史数据
    }

    // 查找当前页面的访问记录
    auto current_it = std::find_if(
        access_history_.rbegin(), access_history_.rend(),
        [current_page_id](const AccessRecord& record) {
            return record.page_id == current_page_id;
        });

    if (current_it == access_history_.rend()) {
        return -1; // 当前页面没有访问记录
    }

    // 查找下一个访问的页面
    auto next_it = current_it;
    ++next_it;

    if (next_it != access_history_.rend()) {
        return next_it->page_id;
    }

    return -1; // 没有后续访问记录
}

AccessPatternAnalysis PageAllocator::AccessPatternAnalyzer::AnalyzePatterns() const {
    std::lock_guard<std::mutex> lock(pattern_mutex_);

    AccessPatternAnalysis analysis;
    analysis.total_accesses = access_history_.size();

    if (analysis.total_accesses < 3) {
        return analysis; // 数据不足
    }

    // 分析顺序访问模式
    size_t sequential_count = 0;
    for (size_t i = 1; i < access_history_.size(); ++i) {
        int32_t prev_id = access_history_[i-1].page_id;
        int32_t curr_id = access_history_[i].page_id;

        if (curr_id == prev_id + 1 || curr_id == prev_id - 1) {
            sequential_count++;
        }
    }

    analysis.has_sequential_access = 
        static_cast<double>(sequential_count) / (analysis.total_accesses - 1) > sequential_threshold_;

    // 分析可预测访问模式
    std::unordered_map<std::pair<int32_t, int32_t>, size_t, PairHash> transitions;

    for (size_t i = 1; i < access_history_.size(); ++i) {
        std::pair<int32_t, int32_t> transition = {
            access_history_[i-1].page_id,
            access_history_[i].page_id
        };
        transitions[transition]++;
    }

    size_t predictable_transitions = 0;
    for (const auto& pair : transitions) {
        if (pair.second > 1) { // 多次出现的转换
            predictable_transitions += pair.second;
        }
    }

    analysis.has_predictable_access = 
        static_cast<double>(predictable_transitions) / analysis.total_accesses > predictability_threshold_;

    // 计算访问频率
    std::unordered_map<int32_t, size_t> page_access_counts;
    for (const auto& record : access_history_) {
        page_access_counts[record.page_id]++;
    }

    analysis.most_accessed_page = -1;
    size_t max_accesses = 0;
    for (const auto& pair : page_access_counts) {
        if (pair.second > max_accesses) {
            max_accesses = pair.second;
            analysis.most_accessed_page = pair.first;
        }
    }

    return analysis;
}

// 内存监视器实现
MemoryStats PageAllocator::MemoryMonitor::GetMemoryStats() const {
    MemoryStats stats;

    // 在实际实现中，这里应该获取系统的内存信息
    // 现在使用模拟数据
    stats.total_memory_mb = 8192; // 8GB
    stats.used_memory_mb = 4096;  // 4GB
    stats.available_memory_mb = stats.total_memory_mb - stats.used_memory_mb;
    stats.memory_usage_percent = (stats.used_memory_mb * 100.0) / stats.total_memory_mb;

    return stats;
}

} // namespace sqlcc
