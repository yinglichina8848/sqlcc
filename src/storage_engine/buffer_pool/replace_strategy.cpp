#include "../replace_strategy.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

// AbstractReplaceStrategy implementation
AbstractReplaceStrategy::AbstractReplaceStrategy(const std::string& name) 
    : name_(name) {
}

void AbstractReplaceStrategy::PinPage(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    auto it = page_info_.find(page_id);
    if (it != page_info_.end()) {
        it->second.pin_count++;
    }
}

void AbstractReplaceStrategy::UnpinPage(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    auto it = page_info_.find(page_id);
    if (it != page_info_.end()) {
        it->second.pin_count--;
        if (it->second.pin_count < 0) {
            it->second.pin_count = 0;
        }
    }
}

void AbstractReplaceStrategy::MarkDirty(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    auto it = page_info_.find(page_id);
    if (it != page_info_.end()) {
        it->second.is_dirty = true;
    }
}

void AbstractReplaceStrategy::CleanPage(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    auto it = page_info_.find(page_id);
    if (it != page_info_.end()) {
        it->second.is_dirty = false;
    }
}

AbstractReplaceStrategy::StrategyStats AbstractReplaceStrategy::GetStats() const {
    std::unique_lock<std::mutex> lock(stats_mutex_);
    return stats_;
}

void AbstractReplaceStrategy::ResetStats() {
    std::unique_lock<std::mutex> lock(stats_mutex_);
    stats_ = StrategyStats{};
}

AbstractReplaceStrategy::PageAccessInfo* AbstractReplaceStrategy::GetPageInfo(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    auto it = page_info_.find(page_id);
    return (it != page_info_.end()) ? &it->second : nullptr;
}

void AbstractReplaceStrategy::RemovePage(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    page_info_.erase(page_id);
}

void AbstractReplaceStrategy::AddPage(int32_t page_id) {
    std::unique_lock<std::mutex> lock(page_info_mutex_);
    if (page_info_.find(page_id) == page_info_.end()) {
        page_info_.emplace(page_id, PageAccessInfo(page_id));
    }
}

// LRUReplaceStrategy implementation
LRUReplaceStrategy::LRUReplaceStrategy() 
    : AbstractReplaceStrategy("LRU") {
}

void LRUReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    if (is_hit) {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_hits++;
        stats_.UpdateHitRate();
    } else {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_misses++;
        stats_.UpdateHitRate();
    }
    
    if (is_write) {
        MarkDirty(page_id);
    }
    
    UpdateLRU(page_id);
}

int32_t LRUReplaceStrategy::SelectVictim() {
    // Select the least recently used page that is not pinned
    for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
        int32_t page_id = *it;
        auto page_info = GetPageInfo(page_id);
        if (page_info && page_info->pin_count == 0) {
            return page_id;
        }
    }
    return -1; // No page can be evicted
}

void LRUReplaceStrategy::UpdateLRU(int32_t page_id) {
    auto it = lru_map_.find(page_id);
    if (it != lru_map_.end()) {
        // Move existing page to front
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
    } else {
        // Add new page to front
        lru_list_.push_front(page_id);
        lru_map_[page_id] = lru_list_.begin();
    }
    
    // Update access time
    auto page_info = GetPageInfo(page_id);
    if (page_info) {
        page_info->access_count++;
        page_info->last_access_time = std::chrono::steady_clock::now();
    }
}

// LFUReplaceStrategy implementation
LFUReplaceStrategy::LFUReplaceStrategy() 
    : AbstractReplaceStrategy("LFU") {
}

void LFUReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    if (is_hit) {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_hits++;
        stats_.UpdateHitRate();
    } else {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_misses++;
        stats_.UpdateHitRate();
    }
    
    if (is_write) {
        MarkDirty(page_id);
    }
    
    auto page_info = GetPageInfo(page_id);
    if (page_info) {
        page_info->access_count++;
        page_info->last_access_time = std::chrono::steady_clock::now();
    }
}

int32_t LFUReplaceStrategy::SelectVictim() {
    // Select the least frequently used page that is not pinned
    for (auto it = frequency_list_.begin(); it != frequency_list_.end(); ++it) {
        int32_t page_id = *it;
        auto page_info = GetPageInfo(page_id);
        if (page_info && page_info->pin_count == 0) {
            return page_id;
        }
    }
    return -1; // No page can be evicted
}

// ClockReplaceStrategy implementation
ClockReplaceStrategy::ClockReplaceStrategy() 
    : AbstractReplaceStrategy("CLOCK") {
}

void ClockReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    if (is_hit) {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_hits++;
        stats_.UpdateHitRate();
    } else {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_misses++;
        stats_.UpdateHitRate();
    }
    
    if (is_write) {
        MarkDirty(page_id);
    }
    
    // Set reference bit
    reference_bits_[page_id] = true;
    
    auto page_info = GetPageInfo(page_id);
    if (page_info) {
        page_info->access_count++;
        page_info->last_access_time = std::chrono::steady_clock::now();
    }
}

int32_t ClockReplaceStrategy::SelectVictim() {
    // CLOCK algorithm: scan through pages, clearing reference bits
    // until finding a page with reference bit = 0
    if (clock_list_.empty()) {
        return -1;
    }
    
    size_t max_scans = clock_list_.size();
    while (max_scans > 0) {
        int32_t page_id = *clock_hand_;
        
        if (reference_bits_[page_id]) {
            // Page was referenced, clear bit and move to next
            reference_bits_[page_id] = false;
            ++clock_hand_;
            if (clock_hand_ == clock_list_.end()) {
                clock_hand_ = clock_list_.begin();
            }
            max_scans--;
        } else {
            // Page was not referenced, select as victim
            auto page_info = GetPageInfo(page_id);
            if (page_info && page_info->pin_count == 0) {
                return page_id;
            } else {
                // Page is pinned, move to next
                ++clock_hand_;
                if (clock_hand_ == clock_list_.end()) {
                    clock_hand_ = clock_list_.begin();
                }
                max_scans--;
            }
        }
    }
    
    return -1; // No suitable page found
}

// ReplaceStrategyFactory implementation
std::unique_ptr<AbstractReplaceStrategy> ReplaceStrategyFactory::CreateStrategy(StrategyType type) {
    switch (type) {
        case StrategyType::LRU:
            return std::make_unique<LRUReplaceStrategy>();
        case StrategyType::LFU:
            return std::make_unique<LFUReplaceStrategy>();
        case StrategyType::CLOCK:
            return std::make_unique<ClockReplaceStrategy>();
        case StrategyType::ARC:
            return std::make_unique<ARCReplaceStrategy>(1024, 1024); // Default sizes
        case StrategyType::FIFO:
            // For simplicity, use LRU as FIFO approximation
            return std::make_unique<LRUReplaceStrategy>();
        default:
            return std::make_unique<LRUReplaceStrategy>();
    }
}

std::string ReplaceStrategyFactory::GetStrategyName(StrategyType type) {
    switch (type) {
        case StrategyType::LRU: return "LRU";
        case StrategyType::LFU: return "LFU";
        case StrategyType::CLOCK: return "CLOCK";
        case StrategyType::ARC: return "ARC";
        case StrategyType::FIFO: return "FIFO";
        default: return "Unknown";
    }
}

ReplaceStrategyFactory::StrategyType ReplaceStrategyFactory::GetStrategyType(const std::string& name) {
    if (name == "LRU") return StrategyType::LRU;
    if (name == "LFU") return StrategyType::LFU;
    if (name == "CLOCK") return StrategyType::CLOCK;
    if (name == "ARC") return StrategyType::ARC;
    if (name == "FIFO") return StrategyType::FIFO;
    return StrategyType::LRU; // Default
}

// ARCReplaceStrategy implementation
ARCReplaceStrategy::ARCReplaceStrategy(size_t p, size_t total_size) 
    : AbstractReplaceStrategy("ARC"), p_(p), total_size_(total_size) {
}

void ARCReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    if (is_hit) {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_hits++;
        stats_.UpdateHitRate();
    } else {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_misses++;
        stats_.UpdateHitRate();
    }
    
    if (is_write) {
        MarkDirty(page_id);
    }
    
    // Check if page is in T1, T2, B1, or B2
    auto in_t1 = std::find(t1_list_.begin(), t1_list_.end(), page_id);
    auto in_t2 = std::find(t2_list_.begin(), t2_list_.end(), page_id);
    auto in_b1 = std::find(b1_list_.begin(), b1_list_.end(), page_id);
    auto in_b2 = std::find(b2_list_.begin(), b2_list_.end(), page_id);
    
    if (in_t1 != t1_list_.end()) {
        // Hit in T1, move to T2
        t1_list_.erase(in_t1);
        t2_list_.push_front(page_id);
    } else if (in_t2 != t2_list_.end()) {
        // Hit in T2, move to front
        t2_list_.erase(in_t2);
        t2_list_.push_front(page_id);
    } else if (in_b1 != b1_list_.end()) {
        // Hit in B1, increase p and move to T2
        p_ = std::min(p_ + std::max((size_t)1, b1_list_.size() / b2_list_.size()), total_size_);
        b1_list_.erase(in_b1);
        t2_list_.push_front(page_id);
    } else if (in_b2 != b2_list_.end()) {
        // Hit in B2, increase p and move to T2
        p_ = std::min(p_ + std::max((size_t)1, b2_list_.size() / b1_list_.size()), total_size_);
        b2_list_.erase(in_b2);
        t2_list_.push_front(page_id);
    } else {
        // Cache miss, add to T1
        t1_list_.push_front(page_id);
        AddPage(page_id);
    }
}

int32_t ARCReplaceStrategy::SelectVictim() {
    // ARC replacement logic
    if (!t1_list_.empty() && (t2_list_.size() == p_ || (t1_list_.size() > p_ && !t2_list_.empty()))) {
        // Replace from T1 or B1
        if (!t1_list_.empty()) {
            int32_t victim = t1_list_.back();
            t1_list_.pop_back();
            b1_list_.push_front(victim);
            if (b1_list_.size() > total_size_) {
                b1_list_.pop_back();
            }
            return victim;
        }
    } else {
        // Replace from T2 or B2
        if (!t2_list_.empty()) {
            int32_t victim = t2_list_.back();
            t2_list_.pop_back();
            b2_list_.push_front(victim);
            if (b2_list_.size() > total_size_) {
                b2_list_.pop_back();
            }
            return victim;
        }
    }
    
    return -1; // No page can be evicted
}

std::list<int32_t>::iterator ARCReplaceStrategy::GetIterator(const std::string& list_name, int32_t page_id) {
    std::list<int32_t>* list_ptr = nullptr;
    if (list_name == "T1") list_ptr = &t1_list_;
    else if (list_name == "T2") list_ptr = &t2_list_;
    else if (list_name == "B1") list_ptr = &b1_list_;
    else if (list_name == "B2") list_ptr = &b2_list_;
    
    if (list_ptr) {
        return std::find(list_ptr->begin(), list_ptr->end(), page_id);
    }
    
    return std::list<int32_t>::iterator();
}

} // namespace sqlcc
