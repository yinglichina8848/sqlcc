#pragma once

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <string>
#include <chrono>

/**
 * 分片缓冲池并发测试头文件
 * 包含测试类和辅助函数的声明
 */

// 简单的锁管理器实现声明，用于测试锁设计的核心逻辑
class SimpleLockManager {
public:
    SimpleLockManager() = default;

    bool AcquireLock(const std::string& key);
    void ReleaseLock(const std::string& key);
    bool IsLocked(const std::string& key);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, bool> locks_;
};

// 简化的锁设计测试类声明
class LockTest : public ::testing::Test {
protected:
    SimpleLockManager lock_manager;
};
