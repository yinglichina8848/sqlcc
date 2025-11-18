#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <string>
#include <chrono>

// 简单的锁管理器实现，用于测试锁设计的核心逻辑
class SimpleLockManager {
public:
    SimpleLockManager() = default;
    
    bool AcquireLock(const std::string& key) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = locks_.find(key);
        if (it != locks_.end() && it->second) {
            return false; // 锁已被占用
        }
        locks_[key] = true;
        return true;
    }
    
    void ReleaseLock(const std::string& key) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = locks_.find(key);
        if (it != locks_.end()) {
            locks_[key] = false;
        }
    }
    
    bool IsLocked(const std::string& key) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = locks_.find(key);
        return (it != locks_.end() && it->second);
    }
    
private:
    std::mutex mutex_;
    std::unordered_map<std::string, bool> locks_;
};

// 简化的锁设计测试
class LockTest : public ::testing::Test {
protected:
    SimpleLockManager lock_manager;
};

// 测试基本的锁获取和释放功能
TEST_F(LockTest, BasicLockAcquireRelease) {
    std::string key = "test_key_1";
    
    // 获取锁
    EXPECT_TRUE(lock_manager.AcquireLock(key));
    
    // 检查锁状态
    EXPECT_TRUE(lock_manager.IsLocked(key));
    
    // 释放锁
    lock_manager.ReleaseLock(key);
    
    // 再次检查锁状态
    EXPECT_FALSE(lock_manager.IsLocked(key));
}

// 测试同一key的并发锁获取（应该是互斥的）
TEST_F(LockTest, ConcurrentLocking) {
    std::string key = "concurrent_test_key";
    std::atomic<bool> first_acquired(false);
    std::atomic<bool> second_acquired(false);
    std::atomic<bool> first_released(false);
    std::mutex wait_mutex;
    std::unique_lock<std::mutex> wait_lock(wait_mutex);
    
    // 第一个线程获取锁
    std::thread t1([&]() {
        lock_manager.AcquireLock(key);
        first_acquired = true;
        // 持有锁一段时间
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        first_released = true;
        lock_manager.ReleaseLock(key);
    });
    
    // 等待第一个线程获取锁
    while (!first_acquired) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 第二个线程尝试获取同一个key的锁（应该失败，因为我们的SimpleLockManager不会阻塞）
    std::thread t2([&]() {
        // 尝试获取锁（应该失败）
        bool acquired = lock_manager.AcquireLock(key);
        if (!acquired) {
            // 等待第一个线程释放锁
            while (!first_released) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            // 再次尝试获取锁
            acquired = lock_manager.AcquireLock(key);
        }
        second_acquired = acquired;
        if (acquired) {
            lock_manager.ReleaseLock(key);
        }
    });
    
    // 等待两个线程完成
    t1.join();
    t2.join();
    
    // 确保第二个线程最终获取到了锁
    EXPECT_TRUE(second_acquired);
    // 最终锁应该被释放
    EXPECT_FALSE(lock_manager.IsLocked(key));
}

// 测试不同key的并发锁获取（应该可以并行）
TEST_F(LockTest, DifferentKeysParallel) {
    std::string key1 = "key_1";
    std::string key2 = "key_2";
    std::atomic<int> acquired_count(0);
    
    auto thread_func = [&](const std::string& key) {
        lock_manager.AcquireLock(key);
        acquired_count++;
        // 持有锁一段时间
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        lock_manager.ReleaseLock(key);
    };
    
    // 创建两个线程，分别获取不同key的锁
    std::thread t1(thread_func, key1);
    std::thread t2(thread_func, key2);
    
    // 等待两个线程都获取锁
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // 检查两个线程是否都成功获取了锁
    EXPECT_EQ(acquired_count, 2);
    
    // 等待线程完成
    t1.join();
    t2.join();
    
    // 最终两个锁都应该被释放
    EXPECT_FALSE(lock_manager.IsLocked(key1));
    EXPECT_FALSE(lock_manager.IsLocked(key2));
}

// 测试大量并发的锁操作
TEST_F(LockTest, HighConcurrency) {
    const int NUM_THREADS = 32;
    const int OPERATIONS_PER_THREAD = 100;
    std::atomic<int> success_count(0);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread([&, i]() {
            for (int j = 0; j < OPERATIONS_PER_THREAD; j++) {
                std::string key = "thread_" + std::to_string(i) + "_key_" + std::to_string(j % 10);
                
                // 重试直到获取锁成功
                while (!lock_manager.AcquireLock(key)) {
                    std::this_thread::yield();
                }
                
                // 模拟一些操作
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                lock_manager.ReleaseLock(key);
                
                success_count++;
            }
        }));
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 检查所有操作是否成功完成
    EXPECT_EQ(success_count, NUM_THREADS * OPERATIONS_PER_THREAD);
}

// 添加锁冲突测试
TEST_F(LockTest, LockContentionTest) {
    const int NUM_THREADS = 16;
    const int ITERATIONS = 100;
    
    // 使用相同的key来创建锁冲突
    std::string hot_key = "hot_key_for_contention";
    std::atomic<int> successful_operations(0);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread([&]() {
            for (int j = 0; j < ITERATIONS; j++) {
                // 重试直到获取锁成功
                while (!lock_manager.AcquireLock(hot_key)) {
                    std::this_thread::yield();
                }
                
                // 模拟临界区操作
                std::this_thread::sleep_for(std::chrono::microseconds(5));
                lock_manager.ReleaseLock(hot_key);
                successful_operations++;
            }
        }));
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 检查是否所有操作都成功了
    EXPECT_EQ(successful_operations, NUM_THREADS * ITERATIONS);
}

// 添加简单的哈希分布测试
TEST_F(LockTest, HashDistributionTest) {
    const int NUM_KEYS = 1000;
    const int BUCKET_COUNT = 16;
    std::vector<int> key_distribution(BUCKET_COUNT, 0);
    
    // 统计不同key的哈希分布
    for (int i = 0; i < NUM_KEYS; i++) {
        std::string key = "test_key_" + std::to_string(i);
        // 计算key的哈希值并分布到不同的桶中
        std::hash<std::string> hasher;
        size_t hash_value = hasher(key);
        size_t bucket_index = hash_value % BUCKET_COUNT;
        
        // 记录分布
        key_distribution[bucket_index]++;
        
        // 测试锁操作
        lock_manager.AcquireLock(key);
        lock_manager.ReleaseLock(key);
    }
    
    // 打印分布信息
    std::cout << "Key distribution across hash buckets:" << std::endl;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        std::cout << "Bucket " << i << ": " << key_distribution[i] << " keys" << std::endl;
    }
    
    // 检查分布是否相对均衡（没有桶为空）
    for (int count : key_distribution) {
        EXPECT_GT(count, 0);
    }
}

// 主函数由测试框架提供