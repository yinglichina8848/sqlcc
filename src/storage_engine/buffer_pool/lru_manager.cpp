/**
 * @file lru_manager.cpp
 * @brief LRU缓存管理器实现
 */

#include "include/storage_engine/buffer_pool/lru_manager.h"
#include "include/utils/logger.h"

namespace sqlcc {
namespace storage {

LRUManager::LRUManager() {
  SQLCC_LOG_DEBUG("LRUManager initialized");
}

LRUManager::~LRUManager() {
  Clear();
  SQLCC_LOG_DEBUG("LRUManager destroyed");
}

void LRUManager::Access(int32_t page_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 查找页面在LRU链表中的位置
  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    // 从当前位置移除
    lru_list_.erase(it->second);
    // 添加到链表头部
    lru_list_.push_front(page_id);
    // 更新映射
    lru_map_[page_id] = lru_list_.begin();
  }
}

void LRUManager::Add(int32_t page_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查是否已经存在
  if (lru_map_.find(page_id) != lru_map_.end()) {
    // 如果已经存在，先移除再添加
    Remove(page_id);
  }

  // 添加到链表头部
  lru_list_.push_front(page_id);
  lru_map_[page_id] = lru_list_.begin();
}

void LRUManager::Remove(int32_t page_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 查找页面位置
  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    // 从链表中移除
    lru_list_.erase(it->second);
    // 从映射中移除
    lru_map_.erase(it);
  }
}

int32_t LRUManager::GetLeastRecentlyUsed() const {
  std::lock_guard<std::mutex> lock(mutex_);

  if (lru_list_.empty()) {
    return -1;
  }

  // 返回链表尾部的页面ID（最少使用的）
  return lru_list_.back();
}

bool LRUManager::Contains(int32_t page_id) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return lru_map_.find(page_id) != lru_map_.end();
}

void LRUManager::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  lru_list_.clear();
  lru_map_.clear();
}

size_t LRUManager::Size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return lru_list_.size();
}

} // namespace storage
} // namespace sqlcc
