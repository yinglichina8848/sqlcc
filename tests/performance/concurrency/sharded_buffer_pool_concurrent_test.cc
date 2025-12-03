#include "sharded_buffer_pool_concurrent_test.h"

// 实现SimpleLockManager的方法
bool SimpleLockManager::AcquireLock(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (locks_[key]) {
        return false; // 锁已存在
    }
    locks_[key] = true;
    return true;
}

void SimpleLockManager::ReleaseLock(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (locks_.find(key) != locks_.end()) {
        locks_[key] = false;
    }
}

bool SimpleLockManager::IsLocked(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = locks_.find(key);
    return (it != locks_.end() && it->second);
}
