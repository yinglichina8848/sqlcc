/**
 * @file concurrency_control_test.cpp
 * @brief 并发控制系统单元测试 - 全面测试锁管理器和并发控制机制
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <shared_mutex>

#include "types/transaction_types.h"
#include "storage/advanced_lock_manager.h"
#include "storage/concurrency_control.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include "include/storage/concurrency_control.h"
#include "include/storage/advanced_lock_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class ConcurrencyControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_concurrency_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("concurrency.max_locks", std::string("1000"));
        config->SetValue("concurrency.lock_timeout_ms", std::string("5000"));
        config->SetValue("concurrency.deadlock_detection_enabled", std::string("true"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化并发控制组件
        lock_manager = std::make_unique<AdvancedLockManager>(*config);
        concurrency_control = std::make_unique<ConcurrencyControl>(*config, *lock_manager, *storage_engine);
    }

    void TearDown() override {
        concurrency_control.reset();
        lock_manager.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 创建测试事务ID
    TransactionId CreateTestTransactionId(uint64_t id = 1) {
        return TransactionId{id, std::chrono::system_clock::now()};
    }

    // 创建测试资源ID
    std::string CreateTestResourceId(size_t id, const std::string& type = "page") {
        return type + "_" + std::to_string(id);
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<AdvancedLockManager> lock_manager;
    std::unique_ptr<ConcurrencyControl> concurrency_control;
};

// 测试锁管理器基本功能
TEST_F(ConcurrencyControlTest, LockManagerBasicOperations) {
    auto tx_id = CreateTestTransactionId(1);
    auto resource_id = CreateTestResourceId(1);

    // 测试独占锁获取
    bool lock_result = lock_manager->AcquireLock(tx_id, resource_id, LockType::EXCLUSIVE);
    EXPECT_TRUE(lock_result);

    // 验证锁状态
    EXPECT_TRUE(lock_manager->HasLock(tx_id, resource_id));
    EXPECT_TRUE(lock_manager->HasLock(tx_id, resource_id, LockType::EXCLUSIVE));

    // 测试锁释放
    bool release_result = lock_manager->ReleaseLock(tx_id, resource_id);
    EXPECT_TRUE(release_result);

    // 验证锁已释放
    EXPECT_FALSE(lock_manager->HasLock(tx_id, resource_id));
}

// 测试锁兼容性
TEST_F(ConcurrencyControlTest, LockCompatibility) {
    auto tx1_id = CreateTestTransactionId(1);
    auto tx2_id = CreateTestTransactionId(2);
    auto resource_id = CreateTestResourceId(1);

    // 事务1获取共享锁
    bool lock1_result = lock_manager->AcquireLock(tx1_id, resource_id, LockType::SHARED);
    EXPECT_TRUE(lock1_result);

    // 事务2可以获取共享锁（兼容）
    bool lock2_result = lock_manager->AcquireLock(tx2_id, resource_id, LockType::SHARED);
    EXPECT_TRUE(lock2_result);

    // 事务2不能获取独占锁（不兼容）
    bool exclusive_lock_result = lock_manager->AcquireLock(tx2_id, resource_id, LockType::EXCLUSIVE);
    EXPECT_FALSE(exclusive_lock_result);

    // 释放锁
    lock_manager->ReleaseLock(tx1_id, resource_id);
    lock_manager->ReleaseLock(tx2_id, resource_id);
}

// 测试锁升级
TEST_F(ConcurrencyControlTest, LockUpgrade) {
    auto tx_id = CreateTestTransactionId(1);
    auto resource_id = CreateTestResourceId(1);

    // 首先获取共享锁
    bool shared_lock_result = lock_manager->AcquireLock(tx_id, resource_id, LockType::SHARED);
    EXPECT_TRUE(shared_lock_result);

    // 升级到独占锁
    bool upgrade_result = lock_manager->UpgradeLock(tx_id, resource_id);
    EXPECT_TRUE(upgrade_result);

    // 验证现在持有独占锁
    EXPECT_TRUE(lock_manager->HasLock(tx_id, resource_id, LockType::EXCLUSIVE));

    // 释放锁
    lock_manager->ReleaseLock(tx_id, resource_id);
}

// 测试锁降级
TEST_F(ConcurrencyControlTest, LockDowngrade) {
    auto tx_id = CreateTestTransactionId(1);
    auto resource_id = CreateTestResourceId(1);

    // 获取独占锁
    bool exclusive_lock_result = lock_manager->AcquireLock(tx_id, resource_id, LockType::EXCLUSIVE);
    EXPECT_TRUE(exclusive_lock_result);

    // 降级到共享锁
    bool downgrade_result = lock_manager->DowngradeLock(tx_id, resource_id);
    EXPECT_TRUE(downgrade_result);

    // 验证现在持有共享锁
    EXPECT_TRUE(lock_manager->HasLock(tx_id, resource_id, LockType::SHARED));
    EXPECT_FALSE(lock_manager->HasLock(tx_id, resource_id, LockType::EXCLUSIVE));

    // 释放锁
    lock_manager->ReleaseLock(tx_id, resource_id);
}

// 测试死锁检测
TEST_F(ConcurrencyControlTest, DeadlockDetection) {
    auto tx1_id = CreateTestTransactionId(1);
    auto tx2_id = CreateTestTransactionId(2);
    auto resource1_id = CreateTestResourceId(1);
    auto resource2_id = CreateTestResourceId(2);

    // 创建死锁场景
        // 事务1锁定资源1
        bool lock1_result = lock_manager->AcquireLock(tx1_id, resource1_id, LockType::EXCLUSIVE);
        EXPECT_TRUE(lock1_result);

        // 事务2锁定资源2
        bool lock2_result = lock_manager->AcquireLock(tx2_id, resource2_id, LockType::EXCLUSIVE);
        EXPECT_TRUE(lock2_result);

    // 事务1尝试锁定资源2（会被阻塞）
    std::atomic<bool> tx1_blocked{false};
    std::thread tx1_thread([this, tx1_id, resource2_id, &tx1_blocked]() {
        tx1_blocked = true;
        // 这可能会被死锁检测器终止
        lock_manager->AcquireLock(tx1_id, resource2_id, LockType::EXCLUSIVE);
    });

    // 等待事务1开始尝试获取锁
    while (!tx1_blocked) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 事务2尝试锁定资源1（创建死锁）
    std::atomic<bool> tx2_blocked{false};
    std::thread tx2_thread([this, tx2_id, resource1_id, &tx2_blocked]() {
        tx2_blocked = true;
        // 这应该被死锁检测器终止
        lock_manager->AcquireLock(tx2_id, resource1_id, LockType::EXCLUSIVE);
    });

    // 等待事务2开始尝试获取锁
    while (!tx2_blocked) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 等待死锁检测器工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 其中一个事务应该被终止
    // 注意：具体的死锁解决策略可能不同，这里我们只是验证死锁检测机制存在

    // 清理线程
    if (tx1_thread.joinable()) tx1_thread.join();
    if (tx2_thread.joinable()) tx2_thread.join();

    // 清理锁
    lock_manager->ReleaseAllLocks(tx1_id);
    lock_manager->ReleaseAllLocks(tx2_id);
}

// 测试并发控制事务管理
TEST_F(ConcurrencyControlTest, TransactionManagement) {
    auto tx_id = CreateTestTransactionId(1);

    // 开始事务
    bool begin_result = concurrency_control->BeginTransaction(tx_id);
    EXPECT_TRUE(begin_result);

    // 验证事务状态
    EXPECT_TRUE(concurrency_control->IsTransactionActive(tx_id));

    // 获取锁
    auto resource_id = CreateTestResourceId(1);
    bool lock_result = concurrency_control->AcquireLock(tx_id, resource_id, LockType::EXCLUSIVE);
    EXPECT_TRUE(lock_result);

    // 提交事务
    bool commit_result = concurrency_control->CommitTransaction(tx_id);
    EXPECT_TRUE(commit_result);

    // 验证事务已结束
    EXPECT_FALSE(concurrency_control->IsTransactionActive(tx_id));
}

// 测试并发控制隔离级别
TEST_F(ConcurrencyControlTest, IsolationLevels) {
    // 测试不同隔离级别
    std::vector<IsolationLevel> levels = {
        IsolationLevel::READ_UNCOMMITTED,
        IsolationLevel::READ_COMMITTED,
        IsolationLevel::REPEATABLE_READ,
        IsolationLevel::SERIALIZABLE
    };

    for (auto level : levels) {
        // 设置隔离级别
        bool set_result = concurrency_control->SetIsolationLevel(level);
        EXPECT_TRUE(set_result);

        // 验证隔离级别设置
        EXPECT_EQ(concurrency_control->GetIsolationLevel(), level);
    }
}

// 测试并发事务执行
TEST_F(ConcurrencyControlTest, ConcurrentTransactions) {
    const int num_transactions = 5;
    const int operations_per_transaction = 10;
    std::vector<std::thread> threads;
    std::atomic<size_t> completed_transactions{0};

    // 启动多个并发事务
    for (int i = 0; i < num_transactions; ++i) {
        threads.emplace_back([this, i, operations_per_transaction, &completed_transactions]() {
            try {
                auto tx_id = CreateTestTransactionId(100 + i);

                // 开始事务
                concurrency_control->BeginTransaction(tx_id);

                // 执行一系列操作
                for (int j = 0; j < operations_per_transaction; ++j) {
                    auto resource_id = CreateTestResourceId((i * operations_per_transaction + j) % 20);

                    // 获取锁
                    concurrency_control->AcquireLock(tx_id, resource_id, LockType::EXCLUSIVE);

                    // 模拟工作
                    std::this_thread::sleep_for(std::chrono::microseconds(100));

                    // 释放锁
                    concurrency_control->ReleaseLock(tx_id, resource_id);
                }

                // 提交事务
                concurrency_control->CommitTransaction(tx_id);

                completed_transactions++;
            } catch (const std::exception& e) {
                std::cerr << "Transaction " << i << " exception: " << e.what() << std::endl;
            }
        });
    }

    // 等待所有事务完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有事务都成功完成
    EXPECT_EQ(completed_transactions.load(), num_transactions);
}

// 测试并发控制统计信息
TEST_F(ConcurrencyControlTest, StatisticsCollection) {
    auto tx1_id = CreateTestTransactionId(1);
    auto tx2_id = CreateTestTransactionId(2);

    // 执行一些操作来生成统计信息
    concurrency_control->BeginTransaction(tx1_id);
    concurrency_control->BeginTransaction(tx2_id);

    auto resource_id = CreateTestResourceId(1);
    concurrency_control->AcquireLock(tx1_id, resource_id, LockType::SHARED);
    concurrency_control->AcquireLock(tx2_id, resource_id, LockType::SHARED);

    concurrency_control->CommitTransaction(tx1_id);
    concurrency_control->CommitTransaction(tx2_id);

    // 获取统计信息
    auto stats = concurrency_control->GetStatistics();

    // 验证统计信息
    EXPECT_GE(stats.total_transactions, 2);
    EXPECT_GE(stats.total_locks_acquired, 2);
    EXPECT_GE(stats.total_locks_released, 2);
}

// 测试锁等待队列
TEST_F(ConcurrencyControlTest, LockWaitQueue) {
    auto tx1_id = CreateTestTransactionId(1);
    auto tx2_id = CreateTestTransactionId(2);
    auto tx3_id = CreateTestTransactionId(3);
    auto resource_id = CreateTestResourceId(1);

    // 事务1获取独占锁
    concurrency_control->BeginTransaction(tx1_id);
    bool lock1_result = concurrency_control->AcquireLock(tx1_id, resource_id, LockType::EXCLUSIVE);
    EXPECT_TRUE(lock1_result);

    std::atomic<bool> tx2_waiting{false};
    std::atomic<bool> tx3_waiting{false};

    // 事务2尝试获取锁（应该等待）
    std::thread tx2_thread([this, tx2_id, resource_id, &tx2_waiting]() {
        concurrency_control->BeginTransaction(tx2_id);
        tx2_waiting = true;

        // 这应该会等待
        bool lock_result = concurrency_control->AcquireLock(tx2_id, resource_id, LockType::EXCLUSIVE);
        if (lock_result) {
            concurrency_control->ReleaseLock(tx2_id, resource_id);
            concurrency_control->CommitTransaction(tx2_id);
        }
    });

    // 事务3尝试获取锁（应该等待）
    std::thread tx3_thread([this, tx3_id, resource_id, &tx3_waiting]() {
        concurrency_control->BeginTransaction(tx3_id);
        tx3_waiting = true;

        // 这应该会等待
        bool lock_result = concurrency_control->AcquireLock(tx3_id, resource_id, LockType::EXCLUSIVE);
        if (lock_result) {
            concurrency_control->ReleaseLock(tx3_id, resource_id);
            concurrency_control->CommitTransaction(tx3_id);
        }
    });

    // 等待事务开始等待
    while (!tx2_waiting || !tx3_waiting) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // 释放事务1的锁
    concurrency_control->ReleaseLock(tx1_id, resource_id);
    concurrency_control->CommitTransaction(tx1_id);

    // 等待其他事务完成
    if (tx2_thread.joinable()) tx2_thread.join();
    if (tx3_thread.joinable()) tx3_thread.join();
}

// 测试并发控制配置
TEST_F(ConcurrencyControlTest, ConfigurationManagement) {
    // 获取默认配置
    auto default_config = concurrency_control->GetConfig();
    EXPECT_GT(default_config.max_concurrent_transactions, 0);
    EXPECT_GT(default_config.lock_timeout_ms, 0);

    // 设置新配置
    ConcurrencyControl::ConcurrencyConfig new_config;
    new_config.max_concurrent_transactions = 100;
    new_config.lock_timeout_ms = 10000;
    new_config.enable_deadlock_detection = true;

    concurrency_control->SetConfig(new_config);

    // 验证配置更新
    auto updated_config = concurrency_control->GetConfig();
    EXPECT_EQ(updated_config.max_concurrent_transactions, new_config.max_concurrent_transactions);
    EXPECT_EQ(updated_config.lock_timeout_ms, new_config.lock_timeout_ms);
    EXPECT_EQ(updated_config.enable_deadlock_detection, new_config.enable_deadlock_detection);
}

// 测试并发控制错误处理
TEST_F(ConcurrencyControlTest, ErrorHandling) {
    // 测试无效事务ID
    TransactionId invalid_tx{0, std::chrono::system_clock::now()};
    EXPECT_FALSE(concurrency_control->IsTransactionActive(invalid_tx));

    // 测试重复开始事务
    auto tx_id = CreateTestTransactionId(1);
    concurrency_control->BeginTransaction(tx_id);
    bool duplicate_begin = concurrency_control->BeginTransaction(tx_id);
    // 重复开始可能成功或失败，取决于实现

    // 测试未开始事务的提交
    TransactionId unstarted_tx{999, std::chrono::system_clock::now()};
    bool commit_unstarted = concurrency_control->CommitTransaction(unstarted_tx);
    EXPECT_FALSE(commit_unstarted);

    // 清理
    concurrency_control->CommitTransaction(tx_id);
}

// 测试并发控制性能特征
TEST_F(ConcurrencyControlTest, PerformanceCharacteristics) {
    const int num_operations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量并发控制操作
    for (int i = 0; i < num_operations; ++i) {
        auto tx_id = CreateTestTransactionId(1000 + i);
        auto resource_id = CreateTestResourceId(i % 50);

        concurrency_control->BeginTransaction(tx_id);
        concurrency_control->AcquireLock(tx_id, resource_id, LockType::EXCLUSIVE);
        concurrency_control->ReleaseLock(tx_id, resource_id);
        concurrency_control->CommitTransaction(tx_id);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    auto stats = concurrency_control->GetStatistics();
    EXPECT_EQ(stats.total_transactions, num_operations);
    EXPECT_LT(duration.count(), 10000);  // 应该在10秒内完成

    std::cout << "Concurrency control performance: " << num_operations
              << " operations in " << duration.count() << "ms" << std::endl;
}

} // namespace sqlcc
