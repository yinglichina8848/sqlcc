#include "transaction_manager.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace sqlcc;

class TransactionManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<TransactionManager> txn_manager_;

  void SetUp() override {
    // 在每个测试前创建TransactionManager实例
    txn_manager_ = std::make_shared<TransactionManager>();
  }

  void TearDown() override {
    // 在每个测试后清理
    txn_manager_.reset();
  }
};

// 测试事务的基本生命周期：开始、提交
TEST_F(TransactionManagerTest, TransactionBasicLifecycle) {
  // 测试开始事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  EXPECT_NE(txn_id, 0);

  // 测试获取事务状态
  TransactionState state = txn_manager_->get_transaction_state(txn_id);
  EXPECT_EQ(state, TransactionState::ACTIVE);

  // 测试提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));

  // 测试获取已提交事务的状态
  state = txn_manager_->get_transaction_state(txn_id);
  EXPECT_EQ(state, TransactionState::COMMITTED);
}

// 测试事务的基本生命周期：开始、回滚
TEST_F(TransactionManagerTest, TransactionRollback) {
  // 测试开始事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  EXPECT_NE(txn_id, 0);

  // 测试回滚事务
  EXPECT_TRUE(txn_manager_->rollback_transaction(txn_id));

  // 测试获取已回滚事务的状态
  TransactionState state = txn_manager_->get_transaction_state(txn_id);
  EXPECT_EQ(state, TransactionState::ABORTED);
}

// 测试保存点功能
TEST_F(TransactionManagerTest, SavepointFunctionality) {
  // 测试开始事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  EXPECT_NE(txn_id, 0);

  // 测试创建保存点
  EXPECT_TRUE(txn_manager_->create_savepoint(txn_id, "savepoint1"));

  // 测试回滚到保存点
  EXPECT_TRUE(txn_manager_->rollback_to_savepoint(txn_id, "savepoint1"));

  // 测试提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));
}

// 测试锁的基本功能
TEST_F(TransactionManagerTest, LockBasicFunctionality) {
  // 测试开始事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  EXPECT_NE(txn_id, 0);

  // 测试获取共享锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource1", LockType::SHARED));

  // 测试获取排他锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource2", LockType::EXCLUSIVE));

  // 测试释放锁
  txn_manager_->release_lock(txn_id, "resource1");
  txn_manager_->release_lock(txn_id, "resource2");

  // 测试提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));
}

// 测试锁的兼容性
TEST_F(TransactionManagerTest, LockCompatibility) {
  // 测试开始两个事务
  TransactionId txn_id1 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 测试事务1获取共享锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id1, "resource1", LockType::SHARED));

  // 测试事务2也能获取同一个资源的共享锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id2, "resource1", LockType::SHARED));

  // 测试事务1获取排他锁失败，因为资源已被共享锁占用
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id1, "resource1", LockType::EXCLUSIVE));

  // 释放事务1和事务2的共享锁
  txn_manager_->release_lock(txn_id1, "resource1");
  txn_manager_->release_lock(txn_id2, "resource1");

  // 测试事务1获取排他锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id1, "resource1", LockType::EXCLUSIVE));

  // 测试事务2获取共享锁失败，因为资源已被排他锁占用
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id2, "resource1", LockType::SHARED));

  // 测试事务2获取排他锁失败，因为资源已被排他锁占用
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id2, "resource1", LockType::EXCLUSIVE));

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id1));
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id2));
}

// 测试活跃事务列表
TEST_F(TransactionManagerTest, ActiveTransactions) {
  // 初始状态下应该没有活跃事务
  std::vector<TransactionId> active_txns =
      txn_manager_->get_active_transactions();
  EXPECT_EQ(active_txns.size(), 0);

  // 开始两个事务
  TransactionId txn_id1 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 应该有两个活跃事务
  active_txns = txn_manager_->get_active_transactions();
  EXPECT_GE(active_txns.size(), 2);

  // 提交一个事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id1));

  // 应该只有一个活跃事务
  active_txns = txn_manager_->get_active_transactions();
  EXPECT_GE(active_txns.size(), 1);

  // 提交另一个事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id2));

  // 应该没有活跃事务
  active_txns = txn_manager_->get_active_transactions();
  EXPECT_EQ(active_txns.size(), 0);
}

// 测试事务状态查询
TEST_F(TransactionManagerTest, TransactionStateQuery) {
  // 开始一个事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 事务应该是活跃状态
  EXPECT_EQ(txn_manager_->get_transaction_state(txn_id),
            TransactionState::ACTIVE);

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));

  // 事务应该是已提交状态
  EXPECT_EQ(txn_manager_->get_transaction_state(txn_id),
            TransactionState::COMMITTED);

  // 开始另一个事务
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 回滚事务
  EXPECT_TRUE(txn_manager_->rollback_transaction(txn_id2));

  // 事务应该是已中止状态
  EXPECT_EQ(txn_manager_->get_transaction_state(txn_id2),
            TransactionState::ABORTED);

  // 查询不存在的事务，应该返回ABORTED状态
  EXPECT_EQ(txn_manager_->get_transaction_state(9999),
            TransactionState::ABORTED);
}

// 测试死锁检测
TEST_F(TransactionManagerTest, DeadlockDetection) {
  // 开始两个事务
  TransactionId txn_id1 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 事务1获取resource1的排他锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id1, "resource1", LockType::EXCLUSIVE));

  // 事务2获取resource2的排他锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id2, "resource2", LockType::EXCLUSIVE));

  // 事务1尝试获取resource2的排他锁，应该失败
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id1, "resource2", LockType::EXCLUSIVE));

  // 事务2尝试获取resource1的排他锁，应该失败
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id2, "resource1", LockType::EXCLUSIVE));

  // 检测死锁
  EXPECT_TRUE(txn_manager_->detect_deadlock(txn_id1));

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id1));
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id2));
}

// 测试日志记录
TEST_F(TransactionManagerTest, LogOperation) {
  // 开始一个事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 创建一个日志条目
  LogEntry log_entry;
  log_entry.operation = "INSERT";
  log_entry.table_name = "users";
  log_entry.txn_id = txn_id;

  // 记录日志
  txn_manager_->log_operation(txn_id, log_entry);

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));
}

// 测试释放所有锁
TEST_F(TransactionManagerTest, ReleaseAllLocks) {
  // 开始一个事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 获取多个资源的锁
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource1", LockType::EXCLUSIVE));
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource2", LockType::EXCLUSIVE));
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource3", LockType::EXCLUSIVE));

  // 回滚事务，应该释放所有锁
  EXPECT_TRUE(txn_manager_->rollback_transaction(txn_id));

  // 开始另一个事务
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 应该能够获取之前被锁定的资源
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id2, "resource1", LockType::EXCLUSIVE));
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id2, "resource2", LockType::EXCLUSIVE));
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id2, "resource3", LockType::EXCLUSIVE));

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id2));
}

// 测试不同隔离级别
TEST_F(TransactionManagerTest, DifferentIsolationLevels) {
  // 测试不同隔离级别的事务
  TransactionId txn_id1 =
      txn_manager_->begin_transaction(IsolationLevel::READ_UNCOMMITTED);
  TransactionId txn_id2 =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);
  TransactionId txn_id3 =
      txn_manager_->begin_transaction(IsolationLevel::REPEATABLE_READ);
  TransactionId txn_id4 =
      txn_manager_->begin_transaction(IsolationLevel::SERIALIZABLE);

  // 所有事务都应该成功开始
  EXPECT_NE(txn_id1, 0);
  EXPECT_NE(txn_id2, 0);
  EXPECT_NE(txn_id3, 0);
  EXPECT_NE(txn_id4, 0);

  // 提交所有事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id1));
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id2));
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id3));
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id4));
}

// 测试多次获取同一资源的锁
TEST_F(TransactionManagerTest, MultipleLockAcquisitions) {
  // 开始一个事务
  TransactionId txn_id =
      txn_manager_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 多次获取同一资源的共享锁，应该成功
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource1", LockType::SHARED));
  EXPECT_TRUE(
      txn_manager_->acquire_lock(txn_id, "resource1", LockType::SHARED));

  // 获取同一资源的排他锁，应该失败
  EXPECT_FALSE(
      txn_manager_->acquire_lock(txn_id, "resource1", LockType::EXCLUSIVE));

  // 释放锁
  txn_manager_->release_lock(txn_id, "resource1");

  // 提交事务
  EXPECT_TRUE(txn_manager_->commit_transaction(txn_id));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
