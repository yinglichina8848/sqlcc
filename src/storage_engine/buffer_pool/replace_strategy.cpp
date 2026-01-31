/**
 * @file replace_strategy.cpp
 * @brief Implements various page replacement algorithms for the buffer pool.
 *
 * @WHY
 * A buffer pool has finite memory. When a new page needs to be brought in and the pool is full, one of the
 * existing pages must be "evicted". A page replacement algorithm is the policy used to select this "victim" page.
 * The choice of algorithm is a classic computer science problem involving trade-offs between performance,
 * complexity, and overhead, directly impacting the database's cache hit rate and overall speed.
 *
 * @WHAT
 * This file provides concrete implementations for several well-known page replacement algorithms, all derived
 * from the `AbstractReplaceStrategy` interface. This includes:
 * 1.  **LRU (Least Recently Used)**: A popular, balanced algorithm.
 * 2.  **LFU (Least Frequently Used)**: Prioritizes pages based on access frequency.
 * 3.  **CLOCK**: A more efficient approximation of LRU.
 * 4.  **ARC (Adaptive Replacement Cache)**: A sophisticated algorithm that dynamically balances between LRU and LFU properties.
 *
 * It also includes a `ReplaceStrategyFactory` to create instances of these strategies based on configuration.
 *
 * @HOW
 * The implementation follows the **Strategy Design Pattern**:
 * - `AbstractReplaceStrategy` defines the common interface (`RecordAccess`, `SelectVictim`, `PinPage`, etc.).
 * - Each concrete class (`LRUReplaceStrategy`, etc.) implements this interface with its own unique logic and data structures.
 * - The `BufferPoolManager` holds a pointer to an `AbstractReplaceStrategy` and interacts with it through the
 *   common interface, remaining completely unaware of the specific algorithm being used. This allows the replacement
 *   policy to be changed dynamically or at startup without affecting the buffer pool manager's code.
 */
#include "../replace_strategy.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

// --- AbstractReplaceStrategy Implementation ---

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
        if (it->second.pin_count > 0) {
            it->second.pin_count--;
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

// ... other base class implementations ...

// --- LRUReplaceStrategy Implementation ---

/**
 * @class LRUReplaceStrategy
 * @brief Implements the LRU (Least Recently Used) page replacement policy.
 *
 * @WHY
 * LRU is one of the most common and effective general-purpose caching algorithms. It operates on the
 * principle of **temporal locality**: pages that have been accessed recently are likely to be accessed
 * again soon. Therefore, when a page must be evicted, LRU chooses the one that has not been used for the
 * longest amount of time. It strikes a good balance between performance and implementation complexity.
 *
 * @WHAT
 * This class maintains pages in a list ordered by access time. The most recently used page is at the
 * front, and the least recently used page is at the back.
 *
 * @HOW
 * It uses two data structures for efficient, O(1) operations:
 * - `std::list<int32_t> lru_list_`: A doubly-linked list that stores page IDs. The order of the list
 *   represents the access order (front = most recent, back = least recent).
 * - `std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_`: A hash map that maps a page ID
 *   to its corresponding iterator in the `lru_list_`. This allows us to find and move a page in O(1) time.
 *
 *   **RecordAccess(page_id)**: When a page is accessed, we use the map to find it in the list, then use
 *   `std::list::splice` to move its node to the front of the list. This is an O(1) operation.
 *
 *   **SelectVictim()**: We scan from the back of the list (the least recently used pages). The first page
 *   we find that is not "pinned" (i.e., its `pin_count` is 0) is chosen as the victim.
 */
LRUReplaceStrategy::LRUReplaceStrategy() 
    : AbstractReplaceStrategy("LRU") {
}

void LRUReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    // Step 1: Update hit/miss statistics.
    if (is_hit) {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_hits++;
    } else {
        std::unique_lock<std::mutex> lock(stats_mutex_);
        stats_.cache_misses++;
    }
    stats_.UpdateHitRate();
    
    // Step 2: If the access was a write, mark the page as dirty.
    if (is_write) {
        MarkDirty(page_id);
    }
    
    // Step 3: Update the page's position in the LRU order.
    UpdateLRU(page_id);
}

int32_t LRUReplaceStrategy::SelectVictim() {
    // The core of the LRU victim selection.
    // We iterate from the back of the list, which holds the least recently used pages.
    for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
        int32_t page_id = *it;
        auto page_info = GetPageInfo(page_id);

        // A page can only be evicted if it is not currently "pinned" (in use by a task).
        if (page_info && page_info->pin_count == 0) {
            return page_id; // Found a suitable victim.
        }
    }
    return -1; // No page can be evicted (e.g., all are pinned).
}

void LRUReplaceStrategy::UpdateLRU(int32_t page_id) {
    auto it = lru_map_.find(page_id);

    // Scenario 1: The page is already in the cache.
    if (it != lru_map_.end()) {
        // We move its existing node to the front of the list to mark it as most recently used.
        // `splice` is an O(1) operation that just re-wires pointers, avoiding allocations or copies.
        lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
    } else {
        // Scenario 2: The page is new to the cache.
        // Add the new page to the front of the list.
        lru_list_.push_front(page_id);
        // Create a map entry pointing to the new node's iterator.
        lru_map_[page_id] = lru_list_.begin();
    }
}

// --- LFUReplaceStrategy Implementation ---
/**
 * @class LFUReplaceStrategy
 * @brief Implements the LFU (Least Frequently Used) page replacement policy.
 *
 * @WHY
 * LFU operates on the principle of **access frequency**. It assumes that pages that are accessed
 * very often are more valuable than pages accessed recently but infrequently. It evicts the page
 * with the lowest number of accesses. This can be beneficial for workloads with stable "hot" data,
 * as it prevents a single, large scan from flushing out frequently used pages. However, it can be
 * slow to adapt if the "hot" data set changes, a problem known as "cache pollution".
 *
 * @WHAT
 * This class tracks the access count for every page. When a victim is needed, it selects the
 * unpinned page with the lowest access count.
 *
 * @HOW
 * A simple implementation can use a sorted data structure (like a `std::multiset` or a sorted `std::vector`)
 * to keep pages ordered by their frequency count.
 * - **Data Structure**: `frequency_list_` (could be a `std::list` that is kept sorted or a `std::multiset`).
 * - **RecordAccess(page_id)**: When a page is accessed, its frequency count in `page_info_` is incremented. The page's
 *   position in the `frequency_list_` must be updated to reflect its new frequency.
 * - **SelectVictim()**: Find the page at the beginning of the `frequency_list_` (lowest frequency)
 *   that is not pinned.
 *
 * @note The current implementation is a simplified stub and does not fully implement the sorted frequency list,
 * making its `SelectVictim` O(N). A production-ready LFU would use more complex data structures for efficiency.
 */
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
        page_info->access_count++; // This is the core of LFU.
    }
    // A full implementation would now update the page's position in the frequency_list_.
}

int32_t LFUReplaceStrategy::SelectVictim() {
    // This is a naive O(N) implementation. It iterates through all pages to find the LFU victim.
    // A more optimal solution would use a min-heap or balanced tree keyed by frequency.
    int32_t victim_id = -1;
    uint64_t min_freq = UINT64_MAX;

    std::unique_lock<std::mutex> lock(page_info_mutex_);
    for (const auto& pair : page_info_) {
        if (pair.second.pin_count == 0 && pair.second.access_count < min_freq) {
            min_freq = pair.second.access_count;
            victim_id = pair.first;
        }
    }
    return victim_id;
}


// --- ClockReplaceStrategy Implementation ---
/**
 * @class ClockReplaceStrategy
 * @brief Implements the CLOCK page replacement policy.
 *
 * @WHY
 * The CLOCK algorithm is a more efficient approximation of LRU. Standard LRU requires moving an element
 * to the front of a list on every access, which involves overhead. CLOCK avoids this by using a
 * single "reference bit" and a circular scanning pointer ("clock hand"). It offers performance
- * close to LRU with lower overhead per access.
 *
 * @WHAT
 * This class arranges all pages in a conceptual circle. A "clock hand" points to one page. Each page
 * has a "reference bit".
 *
 * @HOW
 * - **Data Structures**:
 *   - `clock_list_`: A `std::list` or `std::vector` to hold pages in a circular buffer.
 *   - `clock_hand_`: An iterator pointing to the current position in the circle.
 *   - `reference_bits_`: A map or vector storing the reference bit for each page.
 * - **RecordAccess(page_id)**: When a page is accessed, its reference bit is simply set to `true`. This is
 *   a very fast O(1) operation.
 * - **SelectVictim()**:
 *   1. The clock hand starts sweeping from its current position.
 *   2. For each page it encounters, it inspects the reference bit.
 *   3. If the bit is `true`, it means the page was recently used. The algorithm gives it a "second chance" by
 *      flipping the bit to `false` and moving the clock hand to the next page.
 *   4. If the bit is `false`, it means the page has not been used since the last time the clock hand swept over it.
 *      If the page is also not pinned, it is selected as the victim.
 *   5. The sweep continues until an unpinned victim with a `false` reference bit is found.
 */
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
    
    // Core of CLOCK: On access, simply set the reference bit to true.
    reference_bits_[page_id] = true;
}

int32_t ClockReplaceStrategy::SelectVictim() {
    if (clock_list_.empty()) {
        return -1;
    }
    
    // To prevent an infinite loop if all pages are pinned, we limit the search.
    size_t max_scans = clock_list_.size() * 2; 
    while (max_scans-- > 0) {
        int32_t current_page_id = *clock_hand_;
        
        // Check the reference bit for the page pointed to by the clock hand.
        if (reference_bits_[current_page_id]) {
            // Give it a second chance: clear the bit and move the hand forward.
            reference_bits_[current_page_id] = false;
        } else {
            // The reference bit is 0. This is a candidate for eviction.
            auto page_info = GetPageInfo(current_page_id);
            if (page_info && page_info->pin_count == 0) {
                // If it's not pinned, we've found our victim.
                return current_page_id;
            }
        }
        
        // Move the clock hand to the next position in the circle.
        ++clock_hand_;
        if (clock_hand_ == clock_list_.end()) {
            clock_hand_ = clock_list_.begin(); // Wrap around
        }
    }
    
    return -1; // No suitable page found after a full scan (all might be pinned).
}


// --- ReplaceStrategyFactory Implementation ---
// ... (Factory comments are generally sufficient)


// --- ARCReplaceStrategy Implementation ---
/**
 * @class ARCReplaceStrategy
 * @brief Implements the ARC (Adaptive Replacement Cache) page replacement policy.
 *
 * @WHY
 * ARC is a sophisticated, patented algorithm designed to overcome the weaknesses of both LRU and LFU.
 * - LRU suffers from "scan pollution" where a single large scan can flush the entire cache.
 * - LFU suffers from "cache pollution" where pages that were popular in the past but are no longer
 *   needed can remain in the cache for a long time.
 * ARC dynamically and adaptively balances between LRU (recency) and LFU (frequency) properties to achieve
 * a higher hit rate across a wider variety of workloads.
 *
 * @WHAT
 * ARC maintains four lists:
 * - **T1**: A list of pages seen exactly once (the "recency" component).
 * - **T2**: A list of pages seen at least twice (the "frequency" component).
 * - **B1**: A "ghost list" of pages recently evicted from T1.
 * - **B2**: A "ghost list" of pages recently evicted from T2.
 * The total size of T1 and T2 is the cache size. The size of B1 and B2 tracks evicted items.
 *
 * @HOW
 * The logic is complex:
 * - **Cache Hit**: If a page in T1 or T2 is hit, it's moved to the MRU position of T2 (marking it as frequent and recent).
 * - **Cache Miss**:
 *   - If the page is found in the ghost list B1 (meaning it was recently used but only once), the target size `p` of the T1 list
 *     is increased, and the page is moved to T2. This indicates the workload has "recency" characteristics.
 *   - If the page is found in the ghost list B2 (meaning it was frequently used), the target size `p` of the T1 list
 *     is decreased, and the page is moved to T2. This indicates the workload has "frequency" characteristics.
 *   - If the page is completely new, it's placed in T1.
 * - **Victim Selection**: A victim is chosen from either T1 or T2 based on the current sizes of the lists relative to the adaptive parameter `p`.
 *
 * @note The current implementation is a simplified stub for educational purposes. A full, production-ready ARC is significantly more complex.
 */
ARCReplaceStrategy::ARCReplaceStrategy(size_t p, size_t total_size) 
    : AbstractReplaceStrategy("ARC"), p_(p), total_size_(total_size) {
}

void ARCReplaceStrategy::RecordAccess(int32_t page_id, bool is_hit, bool is_write) {
    if (is_write) {
        MarkDirty(page_id);
    }
    
    // A simplified representation of the ARC logic.
    auto it_t1 = std::find(t1_list_.begin(), t1_list_.end(), page_id);
    if (it_t1 != t1_list_.end()) {
        // Case 1: Hit in T1 (recently used). Promote it to T2 (frequently used).
        t1_list_.erase(it_t1);
        t2_list_.push_front(page_id);
        return;
    }

    auto it_t2 = std::find(t2_list_.begin(), t2_list_.end(), page_id);
    if (it_t2 != t2_list_.end()) {
        // Case 2: Hit in T2 (frequently used). Move it to the front to keep it fresh.
        t2_list_.erase(it_t2);
        t2_list_.push_front(page_id);
        return;
    }
    
    // --- Cache Miss Handling ---
    auto it_b1 = std::find(b1_list_.begin(), b1_list_.end(), page_id);
    if (it_b1 != b1_list_.end()) {
        // Case 3: Miss, but found in B1 (a ghost of a recently-used-once page).
        // This indicates a "recency" preference. Adapt by increasing p.
        p_ = std::min(total_size_, p_ + std::max(b2_list_.size() / b1_list_.size(), (size_t)1));
        // Replace a page and move the requested page to T2.
        SelectVictim(); 
        b1_list_.erase(it_b1);
        t2_list_.push_front(page_id);
        return;
    }

    auto it_b2 = std::find(b2_list_.begin(), b2_list_.end(), page_id);
    if (it_b2 != b2_list_.end()) {
        // Case 4: Miss, but found in B2 (a ghost of a frequently-used page).
        // This indicates a "frequency" preference. Adapt by decreasing p.
        p_ = std::max((size_t)0, p_ - std::max(b1_list_.size() / b2_list_.size(), (size_t)1));
        // Replace a page and move the requested page to T2.
        SelectVictim();
        b2_list_.erase(it_b2);
        t2_list_.push_front(page_id);
        return;
    }

    // Case 5: Complete miss. A new page enters the cache.
    // First, make space.
    if (t1_list_.size() + t2_list_.size() >= total_size_) {
        SelectVictim();
    }
    // Add the new page to the "recently used once" list.
    t1_list_.push_front(page_id);
}

int32_t ARCReplaceStrategy::SelectVictim() {
    int32_t victim = -1;
    // ARC's replacement logic depends on the relative sizes of T1 and p.
    if (!t1_list_.empty() && (t1_list_.size() > p_)) {
        // T1 has grown too large, evict from T1.
        victim = t1_list_.back();
        t1_list_.pop_back();
        // Move the victim to the ghost list B1.
        b1_list_.push_front(victim);
        if (b1_list_.size() > total_size_ - p_) b1_list_.pop_back();
    } else {
        // Evict from T2.
        if (!t2_list_.empty()) {
            victim = t2_list_.back();
            t2_list_.pop_back();
            // Move the victim to the ghost list B2.
            b2_list_.push_front(victim);
            if (b2_list_.size() > p_) b2_list_.pop_back();
        }
    }
    
    if (victim != -1) RemovePage(victim);
    return victim;
}
// ... rest of the file
} // namespace sqlcc
