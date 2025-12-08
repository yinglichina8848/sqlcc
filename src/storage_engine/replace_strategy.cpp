#include "storage/replace_strategy.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {

// ==================== LRUStrategy Implementation ====================

LRUStrategy::LRUStrategy(ConfigManager &config_manager, size_t max_pages)
    : AbstractReplaceStrategy(config_manager, max_pages) {
  // 初始化配置参数
  UpdateConfig();
}

void LRUStrategy::NotifyPageAccess(int32_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  // 更新统计信息
  stats_.total_accesses++;

  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    // 页面已存在，移动到LRU链表头部
    lru_list_.erase(it->second);
    lru_list_.push_front(page_id);
    it->second = lru_list_.begin();

    // 更新页面访问信息
    auto &info = page_info_[page_id];
    info.ref_count++;
    info.is_dirty = is_dirty;
    info.access_time = std::chrono::steady_clock::now();
    info.access_frequency++;

    stats_.strategy_hits++;
  } else {
    // 页面不存在，添加到LRU链表头部
    lru_list_.push_front(page_id);
    lru_map_[page_id] = lru_list_.begin();

    // 创建页面访问信息
    PageAccessInfo info(page_id);
    info.ref_count = 1;
    info.is_dirty = is_dirty;
    page_info_[page_id] = info;

    stats_.strategy_misses++;
  }
}

void LRUStrategy::NotifyPageRelease(int32_t page_id) {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    // 减少引用计数
    auto &info = page_info_[page_id];
    if (info.ref_count > 0) {
      info.ref_count--;
    }

    // 如果引用计数为0，保留在LRU链表中，可以被替换
    // 如果引用计数大于0，不能被替换
  }
}

int32_t LRUStrategy::SelectVictim() {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  // 从LRU链表尾部开始查找引用计数为0的页面
  for (auto rit = lru_list_.rbegin(); rit != lru_list_.rend(); ++rit) {
    int32_t page_id = *rit;
    auto info_it = page_info_.find(page_id);

    if (info_it != page_info_.end() && info_it->second.ref_count == 0) {
      // 找到可替换的页面
      stats_.total_evictions++;
      return page_id;
    }
  }

  // 没有找到可替换的页面
  return -1;
}

bool LRUStrategy::Contains(int32_t page_id) const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return lru_map_.find(page_id) != lru_map_.end();
}

size_t LRUStrategy::Size() const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return lru_list_.size();
}

AbstractReplaceStrategy::StrategyStats LRUStrategy::GetStats() const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return stats_;
}

void LRUStrategy::Reset() {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  lru_list_.clear();
  lru_map_.clear();
  page_info_.clear();

  stats_ = StrategyStats();
}

std::string LRUStrategy::GetStrategyName() const { return "LRU"; }

void LRUStrategy::UpdateConfig() {
  // LRU策略没有额外的配置参数
  // 未来可以添加如自适应调整等配置
}

// ==================== TwoQStrategy Implementation ====================

TwoQStrategy::TwoQStrategy(ConfigManager &config_manager, size_t max_pages)
    : AbstractReplaceStrategy(config_manager, max_pages) {
  // 初始化队列容量配置
  UpdateQueueCapacities();
}

void TwoQStrategy::UpdateQueueCapacities() {
  // 默认配置：A1in队列占25%，Am队列占50%，A1out历史队列占25%
  a1in_capacity_ =
      config_manager_.GetInt("buffer.2q.a1in_ratio", 25) * max_pages_ / 100;
  am_capacity_ =
      config_manager_.GetInt("buffer.2q.am_ratio", 50) * max_pages_ / 100;
  a1out_capacity_ =
      config_manager_.GetInt("buffer.2q.a1out_ratio", 25) * max_pages_ / 100;

  // 确保总容量不超过最大页面数
  size_t total_capacity = a1in_capacity_ + am_capacity_;
  if (total_capacity > max_pages_) {
    // 按比例缩放
    double scale = static_cast<double>(max_pages_) / total_capacity;
    a1in_capacity_ = static_cast<size_t>(a1in_capacity_ * scale);
    am_capacity_ = static_cast<size_t>(am_capacity_ * scale);
  }
}

void TwoQStrategy::NotifyPageAccess(int32_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  // 更新统计信息
  stats_.total_accesses++;

  // 检查页面是否在Am队列中
  auto am_it = am_map_.find(page_id);
  if (am_it != am_map_.end()) {
    // 页面在Am队列中，移动到Am队列头部（LRU更新）
    am_list_.erase(am_it->second);
    am_list_.push_front(page_id);
    am_it->second = am_list_.begin();

    // 更新页面访问信息
    auto &info = page_info_[page_id];
    info.ref_count++;
    info.is_dirty = is_dirty;
    info.access_time = std::chrono::steady_clock::now();
    info.access_frequency++;

    stats_.strategy_hits++;
    return;
  }

  // 检查页面是否在A1in队列中
  auto a1in_it = a1in_map_.find(page_id);
  if (a1in_it != a1in_map_.end()) {
    // 页面在A1in队列中，移动到Am队列
    RemoveFromA1inQueue(page_id);
    MoveToAmQueue(page_id);

    // 更新页面访问信息
    auto &info = page_info_[page_id];
    info.ref_count++;
    info.is_dirty = is_dirty;
    info.access_time = std::chrono::steady_clock::now();
    info.access_frequency++;

    stats_.strategy_hits++;
    return;
  }

  // 检查页面是否在A1out历史队列中
  auto a1out_it = a1out_map_.find(page_id);
  if (a1out_it != a1out_map_.end()) {
    // 页面在A1out历史队列中，直接添加到Am队列
    a1out_history_.erase(a1out_it->second);
    a1out_map_.erase(page_id);
    MoveToAmQueue(page_id);

    // 创建页面访问信息
    PageAccessInfo info(page_id);
    info.ref_count = 1;
    info.is_dirty = is_dirty;
    info.access_frequency = 2; // 第二次访问
    page_info_[page_id] = info;

    stats_.strategy_hits++;
    return;
  }

  // 页面是新页面，添加到A1in队列
  stats_.strategy_misses++;

  // 如果A1in队列已满，从尾部移除一个页面到A1out历史队列
  if (a1in_list_.size() >= a1in_capacity_) {
    int32_t victim_page_id = a1in_list_.back();
    RemoveFromA1inQueue(victim_page_id);

    // 添加到A1out历史队列
    a1out_history_.push_front(victim_page_id);
    a1out_map_[victim_page_id] = a1out_history_.begin();

    // 如果A1out历史队列已满，从尾部移除最旧的记录
    if (a1out_history_.size() > a1out_capacity_) {
      int32_t old_page_id = a1out_history_.back();
      a1out_history_.pop_back();
      a1out_map_.erase(old_page_id);
    }
  }

  // 添加页面到A1in队列头部
  a1in_list_.push_front(page_id);
  a1in_map_[page_id] = a1in_list_.begin();

  // 创建页面访问信息
  PageAccessInfo info(page_id);
  info.ref_count = 1;
  info.is_dirty = is_dirty;
  page_info_[page_id] = info;
}

void TwoQStrategy::NotifyPageRelease(int32_t page_id) {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  auto info_it = page_info_.find(page_id);
  if (info_it != page_info_.end()) {
    // 减少引用计数
    if (info_it->second.ref_count > 0) {
      info_it->second.ref_count--;
    }
  }
}

int32_t TwoQStrategy::SelectVictim() {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  // 首先尝试从A1in队列选择受害者
  for (auto rit = a1in_list_.rbegin(); rit != a1in_list_.rend(); ++rit) {
    int32_t page_id = *rit;
    auto info_it = page_info_.find(page_id);

    if (info_it != page_info_.end() && info_it->second.ref_count == 0) {
      stats_.total_evictions++;
      return page_id;
    }
  }

  // 如果A1in队列没有可替换的页面，尝试从Am队列选择受害者
  for (auto rit = am_list_.rbegin(); rit != am_list_.rend(); ++rit) {
    int32_t page_id = *rit;
    auto info_it = page_info_.find(page_id);

    if (info_it != page_info_.end() && info_it->second.ref_count == 0) {
      stats_.total_evictions++;
      return page_id;
    }
  }

  // 没有找到可替换的页面
  return -1;
}

bool TwoQStrategy::Contains(int32_t page_id) const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return a1in_map_.find(page_id) != a1in_map_.end() ||
         am_map_.find(page_id) != am_map_.end();
}

size_t TwoQStrategy::Size() const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return a1in_list_.size() + am_list_.size();
}

AbstractReplaceStrategy::StrategyStats TwoQStrategy::GetStats() const {
  std::lock_guard<std::mutex> lock(strategy_mutex_);
  return stats_;
}

void TwoQStrategy::Reset() {
  std::lock_guard<std::mutex> lock(strategy_mutex_);

  a1in_list_.clear();
  a1in_map_.clear();
  am_list_.clear();
  am_map_.clear();
  a1out_history_.clear();
  a1out_map_.clear();
  page_info_.clear();

  stats_ = StrategyStats();
}

std::string TwoQStrategy::GetStrategyName() const { return "2Q"; }

void TwoQStrategy::UpdateConfig() { UpdateQueueCapacities(); }

void TwoQStrategy::MoveToAmQueue(int32_t page_id) {
  // 如果Am队列已满，从尾部移除一个页面
  if (am_list_.size() >= am_capacity_) {
    int32_t victim_page_id = am_list_.back();
    am_list_.pop_back();
    am_map_.erase(victim_page_id);
  }

  // 添加页面到Am队列头部
  am_list_.push_front(page_id);
  am_map_[page_id] = am_list_.begin();
}

void TwoQStrategy::RemoveFromA1inQueue(int32_t page_id) {
  auto it = a1in_map_.find(page_id);
  if (it != a1in_map_.end()) {
    a1in_list_.erase(it->second);
    a1in_map_.erase(page_id);
  }
}

// ==================== ReplaceStrategyFactory Implementation
// ====================

std::unique_ptr<AbstractReplaceStrategy> ReplaceStrategyFactory::CreateStrategy(
    StrategyType type, ConfigManager &config_manager, size_t max_pages) {

  switch (type) {
  case StrategyType::LRU:
    return std::make_unique<LRUStrategy>(config_manager, max_pages);

  case StrategyType::TWO_Q:
    return std::make_unique<TwoQStrategy>(config_manager, max_pages);

  case StrategyType::LFU:
    // 未来实现
    throw NotImplementedException("LFU strategy not yet implemented");

  case StrategyType::ARC:
    // 未来实现
    throw NotImplementedException("ARC strategy not yet implemented");

  default:
    throw IllegalArgumentException("Unknown replace strategy type");
  }
}

ReplaceStrategyFactory::StrategyType
ReplaceStrategyFactory::ParseStrategyType(const std::string &strategy_name) {
  std::string name = strategy_name;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  if (name == "lru") {
    return StrategyType::LRU;
  } else if (name == "2q" || name == "two_q" || name == "twoq") {
    return StrategyType::TWO_Q;
  } else if (name == "lfu") {
    return StrategyType::LFU;
  } else if (name == "arc") {
    return StrategyType::ARC;
  } else {
    // 默认使用LRU
    return StrategyType::LRU;
  }
}

std::string ReplaceStrategyFactory::GetStrategyName(StrategyType type) {
  switch (type) {
  case StrategyType::LRU:
    return "LRU";
  case StrategyType::TWO_Q:
    return "2Q";
  case StrategyType::LFU:
    return "LFU";
  case StrategyType::ARC:
    return "ARC";
  default:
    return "Unknown";
  }
}

} // namespace sqlcc