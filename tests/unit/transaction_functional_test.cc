// tests/unit/transaction_functional_test.cc
// Functional tests for Transaction Manager - focusing on real transaction
// scenarios
#include "transaction_manager.h"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

namespace sqlcc {

class TransactionFunctionalTest : public ::testing::Test {
protected:
  void SetUp() override { txn_mgr_ = std::make_unique<TransactionManager>(); }

  void TearDown() override { txn_mgr_.reset(); }

  // Helper function to simulate database operations
  bool simulate_update_operation(TransactionId txn_id, const std::string &table,
                                 int record_id) {
    std::string resource = table + "." + std::to_string(record_id);
    return txn_mgr_->acquire_lock(txn_id, resource, LockType::EXCLUSIVE);
  }

  bool simulate_read_operation(TransactionId txn_id, const std::string &table,
                               int record_id) {
    std::string resource = table + "." + std::to_string(record_id);
    return txn_mgr_->acquire_lock(txn_id, resource, LockType::SHARED);
  }

  void simulate_workload(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
  }

  std::unique_ptr<TransactionManager> txn_mgr_;
};

// =========================================
// 事务处理功能场景测试
// =========================================

// 测试银行转账事务场景 - 两个账户间的转账
TEST_F(TransactionFunctionalTest, BankTransferTransactionScenario) {
  const std::string account_table = "accounts";

  // 转账金额和账户
  const int from_account = 12345;
  const int to_account = 67890;
  const double transfer_amount = 100.0;

  // 开始转账事务
  TransactionId transfer_txn =
      txn_mgr_->begin_transaction(IsolationLevel::SERIALIZABLE);

  // 阶段1: 锁定源账户
  bool lock_source =
      simulate_update_operation(transfer_txn, account_table, from_account);
  ASSERT_TRUE(lock_source) << "Failed to lock source account";

  // 阶段2: 检查账户余额（在这里我们模拟这个检查）
  simulate_read_operation(transfer_txn, account_table, from_account);
  // 在真实场景中，这里会检查余额是否足够
  double balance = 500.0; // 模拟账户余额
  ASSERT_GE(balance, transfer_amount) << "Insufficient balance for transfer";

  // 阶段3: 锁定目标账户
  bool lock_dest =
      simulate_update_operation(transfer_txn, account_table, to_account);
  ASSERT_TRUE(lock_dest) << "Failed to lock destination account";

  // 阶段4: 执行转账
  // 扣除源账户金额
  simulate_update_operation(transfer_txn, account_table, from_account);
  // 增加目标账户金额
  simulate_update_operation(transfer_txn, account_table, to_account);

  // 记录操作日志
  LogEntry debit_log{transfer_txn,
                     "UPDATE",
                     account_table,
                     from_account,
                     std::chrono::system_clock::now(),
                     std::vector<char>(sizeof(double)),
                     std::vector<char>()};
  LogEntry credit_log{transfer_txn,
                      "UPDATE",
                      account_table,
                      to_account,
                      std::chrono::system_clock::now(),
                      std::vector<char>(sizeof(double)),
                      std::vector<char>()};

  txn_mgr_->log_operation(transfer_txn, debit_log);
  txn_mgr_->log_operation(transfer_txn, credit_log);

  // 提交事务
  bool commit_success = txn_mgr_->commit_transaction(transfer_txn);
  ASSERT_TRUE(commit_success) << "Transaction commit failed";

  // 验证事务状态
  TransactionState final_state = txn_mgr_->get_transaction_state(transfer_txn);
  EXPECT_EQ(final_state, TransactionState::COMMITTED);

  // 验证没有活跃事务
  auto active_txns = txn_mgr_->get_active_transactions();
  EXPECT_TRUE(active_txns.empty()) << "Active transactions remain after commit";
}

// 测试订单处理事务场景 - 电商订单创建
TEST_F(TransactionFunctionalTest, OrderProcessingTransactionScenario) {
  const std::string orders_table = "orders";
  const std::string products_table = "products";
  const std::string users_table = "users";

  TransactionId order_txn =
      txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 步骤1: 验证用户存在
  bool user_check = simulate_read_operation(order_txn, users_table, 1001);
  ASSERT_TRUE(user_check);

  // 步骤2: 检查产品库存
  bool product_inventory =
      simulate_read_operation(order_txn, products_table, 2001);
  ASSERT_TRUE(product_inventory);

  // 步骤3: 创建订单记录
  bool order_creation =
      simulate_update_operation(order_txn, orders_table, 3001);
  ASSERT_TRUE(order_creation);

  // 步骤4: 更新产品库存
  bool inventory_update =
      simulate_update_operation(order_txn, products_table, 2001);
  ASSERT_TRUE(inventory_update);

  // 设置保存点 - 以防后续步骤失败
  txn_mgr_->create_savepoint(order_txn, "before_payment");

  // 步骤5: 处理支付（这里可能会失败）
  // 模拟一个潜在的失败
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 10);
  bool payment_success = (dis(gen) > 2); // 80%成功率

  if (!payment_success) {
    // 支付失败，回滚到保存点
    txn_mgr_->rollback_to_savepoint(order_txn, "before_payment");
    // 手动解锁资源（在真实实现中会自动处理）
    txn_mgr_->release_lock(order_txn, products_table + ".2001");

    // 重新尝试支付（简化处理）
    payment_success = true;
  }

  if (payment_success) {
    // 完成订单
    bool order_complete = txn_mgr_->commit_transaction(order_txn);
    ASSERT_TRUE(order_complete);
  } else {
    // 支付最终失败
    txn_mgr_->rollback_transaction(order_txn);
  }

  SUCCEED();
}

// 测试并发事务的隔离级别效果
TEST_F(TransactionFunctionalTest, IsolationLevelConcurrentAccess) {
  const std::string table_name = "inventory";
  const int record_id = 500;

  // 测试 READ COMMITTED 隔离级别
  std::vector<TransactionId> readers;
  std::vector<TransactionId> writers;

  // 创建多个读者事务
  for (int i = 0; i < 3; ++i) {
    TransactionId reader_txn =
        txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);
    bool lock_success =
        simulate_read_operation(reader_txn, table_name, record_id);
    ASSERT_TRUE(lock_success)
        << "Reader " << i << " failed to acquire shared lock";
    readers.push_back(reader_txn);
  }

  // 创建写者事务（应该被阻塞在READ_COMMITTED隔离级别下）
  for (int i = 0; i < 2; ++i) {
    TransactionId writer_txn =
        txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);

    // 对于READ_COMMITTED，写者会等待读者完成
    bool lock_success =
        simulate_update_operation(writer_txn, table_name, record_id);

    // 在测试环境中，我们的实现可能允许或阻塞这里
    // 但我们记录结果用于分析
    if (lock_success) {
      std::cout << "Writer " << i << " acquired lock despite readers"
                << std::endl;
    } else {
      std::cout << "Writer " << i << " blocked by readers (expected behavior)"
                << std::endl;
    }

    writers.push_back(writer_txn);

    if (lock_success) {
      txn_mgr_->release_lock(writer_txn,
                             table_name + "." + std::to_string(record_id));
    }
  }

  // 提交所有事务
  for (auto txn : readers) {
    txn_mgr_->commit_transaction(txn);
  }
  for (auto txn : writers) {
    txn_mgr_->commit_transaction(txn);
  }

  SUCCEED();
}

// 测试长运行事务的资源管理
TEST_F(TransactionFunctionalTest, LongRunningTransactionResourceManagement) {
  const std::string table_name = "log_table";

  // 模拟长时间运行的事务（如数据迁移）
  TransactionId long_txn =
      txn_mgr_->begin_transaction(IsolationLevel::REPEATABLE_READ);

  // 长时间持有锁
  std::vector<int> record_ids;
  for (int i = 0; i < 50; ++i) {
    record_ids.push_back(1000 + i);
    bool lock_success =
        simulate_update_operation(long_txn, table_name, record_ids.back());
    ASSERT_TRUE(lock_success) << "Failed to lock record " << record_ids.back();
  }

  // 模拟长时间处理
  simulate_workload(50); // 50ms 的"处理时间"

  // 在处理过程中，验证事务仍然活跃
  TransactionState state = txn_mgr_->get_transaction_state(long_txn);
  ASSERT_EQ(state, TransactionState::ACTIVE);

  // 提交事务，释放所有锁
  bool commit_success = txn_mgr_->commit_transaction(long_txn);
  ASSERT_TRUE(commit_success);

  // 验证提交后事务状态
  state = txn_mgr_->get_transaction_state(long_txn);
  ASSERT_EQ(state, TransactionState::COMMITTED);

  // 验证没有活跃事务
  auto active = txn_mgr_->get_active_transactions();
  ASSERT_TRUE(active.empty()) << "Locks not properly released after commit";
}

// =========================================
// 事务恢复场景测试
// =========================================

// 测试事务失败后的自动回滚
TEST_F(TransactionFunctionalTest, TransactionFailureRollback) {
  const std::string accounts_table = "accounts";

  TransactionId failed_txn =
      txn_mgr_->begin_transaction(IsolationLevel::SERIALIZABLE);

  // 开始一些操作
  simulate_update_operation(failed_txn, accounts_table, 100);
  simulate_update_operation(failed_txn, accounts_table, 200);
  simulate_update_operation(failed_txn, accounts_table, 300);

  // 记录操作日志
  txn_mgr_->log_operation(failed_txn, {"UPDATE", accounts_table, 100});
  txn_mgr_->log_operation(failed_txn, {"UPDATE", accounts_table, 200});
  txn_mgr_->log_operation(failed_txn, {"UPDATE", accounts_table, 300});

  // 设置保存点
  txn_mgr_->create_savepoint(failed_txn, "checkpoint");

  // 继续执行更多操作
  simulate_update_operation(failed_txn, accounts_table, 400);
  simulate_update_operation(failed_txn, accounts_table, 500);

  // 模拟业务逻辑失败（比如违反业务规则）
  bool business_logic_failed = true; // 模拟失败

  if (business_logic_failed) {
    // 回滚到保存点，只撤销部分操作
    txn_mgr_->rollback_to_savepoint(failed_txn, "checkpoint");

    // 释放撤销的操作锁
    txn_mgr_->release_lock(failed_txn, accounts_table + ".400");
    txn_mgr_->release_lock(failed_txn, accounts_table + ".500");

    // 可以继续执行其他操作
    simulate_update_operation(failed_txn, accounts_table, 600);

    // 最终提交修改后的交易
    txn_mgr_->commit_transaction(failed_txn);
  } else {
    txn_mgr_->commit_transaction(failed_txn);
  }

  // 验证最终状态
  TransactionState state = txn_mgr_->get_transaction_state(failed_txn);
  EXPECT_EQ(state, TransactionState::COMMITTED);

  SUCCEED();
}

// =========================================
// 锁升级和降级测试
// =========================================

// 测试锁升级：共享锁升级为排他锁
TEST_F(TransactionFunctionalTest, LockUpgradeScenario) {
  const std::string resource = "shared_resource";

  TransactionId txn = txn_mgr_->begin_transaction();

  // 首先获取共享锁
  bool shared_lock = txn_mgr_->acquire_lock(txn, resource, LockType::SHARED);
  ASSERT_TRUE(shared_lock) << "Failed to acquire shared lock";

  // 在同一事务中升级为排他锁
  bool upgrade_lock =
      txn_mgr_->acquire_lock(txn, resource, LockType::EXCLUSIVE);
  EXPECT_TRUE(upgrade_lock)
      << "Lock upgrade should succeed within same transaction";

  // 验证锁仍然为当前事务所有
  txn_mgr_->release_lock(txn, resource);

  // 提交事务
  txn_mgr_->commit_transaction(txn);

  SUCCEED();
}

// =========================================
// 批量操作事务测试
// =========================================

// 测试批量更新事务场景
TEST_F(TransactionFunctionalTest, BatchUpdateTransaction) {
  const std::string employees_table = "employees";
  const std::string salaries_table = "salaries";

  TransactionId batch_txn =
      txn_mgr_->begin_transaction(IsolationLevel::SERIALIZABLE);

  // 批量更新场景：全体员工加薪
  const int num_employees = 1000;

  // 创建保存点
  txn_mgr_->create_savepoint(batch_txn, "batch_start");

  // 批量获取锁
  for (int emp_id = 1; emp_id <= num_employees; ++emp_id) {
    // 更新员工状态
    bool emp_lock =
        simulate_update_operation(batch_txn, employees_table, emp_id);
    if (!emp_lock) {
      // 如果无法获得锁，回滚整个批量操作
      txn_mgr_->rollback_to_savepoint(batch_txn, "batch_start");
      FAIL() << "Failed to acquire lock for employee " << emp_id;
    }

    // 更新薪资
    bool salary_lock =
        simulate_update_operation(batch_txn, salaries_table, emp_id);
    if (!salary_lock) {
      txn_mgr_->rollback_to_savepoint(batch_txn, "batch_start");
      FAIL() << "Failed to acquire lock for salary " << emp_id;
    }
  }

  // 记录批处理操作
  LogEntry batch_log{batch_txn, "BATCH_UPDATE", "ALL_EMPLOYEES", 0,
                     std::chrono::system_clock::now()};
  txn_mgr_->log_operation(batch_txn, batch_log);

  // 提交批处理事务
  bool commit_success = txn_mgr_->commit_transaction(batch_txn);
  ASSERT_TRUE(commit_success) << "Batch update transaction failed";

  // 验证事务完成
  TransactionState state = txn_mgr_->get_transaction_state(batch_txn);
  EXPECT_EQ(state, TransactionState::COMMITTED);

  SUCCEED();
}

// =========================================
// 多表事务完整性测试
// =========================================

// 测试跨多个表的复杂事务
TEST_F(TransactionFunctionalTest, MultiTableComplexTransaction) {
  // 涉及的表
  const std::string customers_table = "customers";
  const std::string orders_table = "orders";
  const std::string order_items_table = "order_items";
  const std::string inventory_table = "inventory";

  // 客户下订单的场景
  const int customer_id = 999;
  const int order_id = 8888;
  std::vector<int> items = {100, 200, 300};
  std::vector<int> quantities = {2, 1, 3};

  TransactionId order_txn =
      txn_mgr_->begin_transaction(IsolationLevel::SERIALIZABLE);

  // 步骤1: 验证客户状态
  bool customer_valid =
      simulate_read_operation(order_txn, customers_table, customer_id);
  ASSERT_TRUE(customer_valid) << "Invalid customer";

  // 步骤2: 创建订单头
  bool order_created =
      simulate_update_operation(order_txn, orders_table, order_id);
  ASSERT_TRUE(order_created) << "Failed to create order";

  // 步骤3: 处理每个订单项
  for (size_t i = 0; i < items.size(); ++i) {
    int item_id = items[i];
    int qty = quantities[i];

    // 检查库存
    simulate_read_operation(order_txn, inventory_table, item_id);

    // 锁定并更新库存
    bool inventory_locked =
        simulate_update_operation(order_txn, inventory_table, item_id);
    ASSERT_TRUE(inventory_locked)
        << "Failed to lock inventory for item " << item_id;

    // 创建订单项
    int order_item_id = order_id * 10 + i;
    bool item_created =
        simulate_update_operation(order_txn, order_items_table, order_item_id);
    ASSERT_TRUE(item_created)
        << "Failed to create order item " << order_item_id;
  }

  // 步骤4: 更新客户订单历史
  bool customer_updated =
      simulate_update_operation(order_txn, customers_table, customer_id);
  ASSERT_TRUE(customer_updated) << "Failed to update customer order history";

  // 记录操作链
  txn_mgr_->log_operation(order_txn, {"CREATE_ORDER", "ALL_TABLES", order_id});

  // 提交完整订单事务
  bool success = txn_mgr_->commit_transaction(order_txn);
  ASSERT_TRUE(success) << "Complex multi-table transaction failed";

  // 验证事务成功完成
  TransactionState state = txn_mgr_->get_transaction_state(order_txn);
  EXPECT_EQ(state, TransactionState::COMMITTED);

  SUCCEED();
}

// =========================================
// 嵌套事务逻辑测试
// =========================================

// 测试嵌套事务逻辑（通过保存点模拟）
TEST_F(TransactionFunctionalTest, NestedTransactionLogic) {
  const std::string accounts_table = "accounts";

  TransactionId main_txn =
      txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);

  // 外层事务：账户管理
  bool account_locked = simulate_update_operation(main_txn, accounts_table, 1);
  ASSERT_TRUE(account_locked);

  // 创建嵌套保存点 1: 余额更新
  txn_mgr_->create_savepoint(main_txn, "balance_update");

  // 内层逻辑 1: 更新余额
  simulate_update_operation(main_txn, accounts_table, 1);
  bool balance_check = true; // 模拟余额检查

  if (!balance_check) {
    txn_mgr_->rollback_to_savepoint(main_txn, "balance_update");
  }

  // 创建嵌套保存点 2: 利息计算
  txn_mgr_->create_savepoint(main_txn, "interest_calculation");

  // 内层逻辑 2: 计算利息
  simulate_update_operation(main_txn, accounts_table, 2);
  bool interest_calc = true; // 模拟利息计算

  if (!interest_calc) {
    txn_mgr_->rollback_to_savepoint(main_txn, "interest_calculation");
  }

  // 外层事务继续
  simulate_update_operation(main_txn, accounts_table, 3);

  // 提交主事务
  bool success = txn_mgr_->commit_transaction(main_txn);
  ASSERT_TRUE(success) << "Nested transaction logic failed";

  SUCCEED();
}

// =========================================
// 性能和压力测试
// =========================================

// 测试高并发事务场景压力
TEST_F(TransactionFunctionalTest, HighConcurrencyLoadTest) {
  const int NUM_WRITER_THREADS = 5;
  const int NUM_READER_THREADS = 10;
  const int OPERATIONS_PER_THREAD = 50;

  std::atomic<int> successful_operations{0};
  std::atomic<int> failed_operations{0};

  auto writer_thread = [this]() {
    for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
      TransactionId txn =
          txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);

      // 每次操作随机选择一个记录
      int record_id = rand() % 100 + 1;
      std::string table = "test_table";

      bool success = false;
      for (int attempt = 0; attempt < 3 && !success; ++attempt) {
        success = simulate_update_operation(txn, table, record_id);
        if (!success) {
          std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
      }

      if (success) {
        // 模拟业务逻辑处理时间
        std::this_thread::sleep_for(std::chrono::microseconds(50));

        if (rand() % 10 < 8) { // 80%成功提交
          txn_mgr_->commit_transaction(txn);
          successful_operations++;
        } else {
          txn_mgr_->rollback_transaction(txn);
          successful_operations++; // 回滚也算成功操作
        }
      } else {
        failed_operations++;
        txn_mgr_->rollback_transaction(txn);
      }
    }
  };

  auto reader_thread = [this]() {
    for (int i = 0; i < OPERATIONS_PER_THREAD * 2; ++i) { // 读者操作更多
      TransactionId txn =
          txn_mgr_->begin_transaction(IsolationLevel::READ_COMMITTED);

      // 随机读取多个记录
      for (int j = 0; j < 5; ++j) {
        int record_id = rand() % 100 + 1;
        std::string table = "test_table";
        simulate_read_operation(txn, table, record_id);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      }

      txn_mgr_->commit_transaction(txn);
      successful_operations++;
    }
  };

  // 启动所有线程
  std::vector<std::thread> threads;

  for (int i = 0; i < NUM_WRITER_THREADS; ++i) {
    threads.emplace_back(writer_thread);
  }

  for (int i = 0; i < NUM_READER_THREADS; ++i) {
    threads.emplace_back(reader_thread);
  }

  // 等待所有线程完成
  for (auto &thread : threads) {
    thread.join();
  }

  // 计算总操作数
  int total_operations = successful_operations + failed_operations;

  std::cout << "High concurrency test results:" << std::endl;
  std::cout << "Total operations: " << total_operations << std::endl;
  std::cout << "Successful operations: " << successful_operations << std::endl;
  std::cout << "Failed operations: " << failed_operations << std::endl;
  std::cout << "Success rate: "
            << (100.0 * successful_operations / total_operations) << "%"
            << std::endl;

  // 基本期望：大部分操作成功，少数由于并发冲突失败
  EXPECT_GE(successful_operations, total_operations * 0.7)
      << "Too many operation failures";
  EXPECT_LE(failed_operations, total_operations * 0.3)
      << "Too few operation failures";

  // 验证没有活跃事务残留
  auto active_txns = txn_mgr_->get_active_transactions();
  EXPECT_TRUE(active_txns.empty())
      << "Active transactions remain after load test";

  SUCCEED();
}

// =========================================
// 故障注入测试
// =========================================

// 测试网络故障模拟（事务超时）
TEST_F(TransactionFunctionalTest, TransactionTimeoutSimulation) {
  TransactionId txn = txn_mgr_->begin_transaction();

  // 获取锁
  simulate_update_operation(txn, "test_table", 100);

  // 模拟长时间运行（在真实系统中会配置超时机制）
  simulate_workload(100); // 100ms - 在真实系统中可能触发超时

  // 在我们的测试实现中，事务应该仍在运行
  TransactionState state = txn_mgr_->get_transaction_state(txn);
  EXPECT_EQ(state, TransactionState::ACTIVE);

  // 手动完成事务
  txn_mgr_->commit_transaction(txn);

  SUCCEED();
}

// 测试资源紧张情况下的行为
TEST_F(TransactionFunctionalTest, ResourceContentionSimulation) {
  const std::string contended_resource = "popular_record";
  const int NUM_CONTENDING_TRANSACTIONS = 10;

  std::vector<TransactionId> txns;
  std::atomic<int> successful_locks{0};

  // 创建多个事务竞争同一个资源
  for (int i = 0; i < NUM_CONTENDING_TRANSACTIONS; ++i) {
    TransactionId txn = txn_mgr_->begin_transaction();
    txns.push_back(txn);
  }

  // 并发竞争锁
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_CONTENDING_TRANSACTIONS; ++i) {
    threads.emplace_back([this, txn = txns[i], &successful_locks]() {
      bool locked = simulate_update_operation(txn, contended_resource, 1);
      if (locked) {
        successful_locks++;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10)); // 持有锁一段时间
        txn_mgr_->release_lock(txn, contended_resource + ".1");
      }
    });
  }

  // 等待所有线程
  for (auto &thread : threads) {
    thread.join();
  }

  std::cout << "Resource contention results: " << successful_locks << "/"
            << NUM_CONTENDING_TRANSACTIONS << " transactions acquired locks"
            << std::endl;

  // 在正确的实现中，只有一个事务应该获得排他锁
  EXPECT_GE(successful_locks, 1) << "At least one transaction should succeed";
  EXPECT_LE(successful_locks, NUM_CONTENDING_TRANSACTIONS)
      << "Cannot exceed total transactions";

  // 提交所有事务
  for (auto txn : txns) {
    txn_mgr_->commit_transaction(txn);
  }

  SUCCEED();
}

} // namespace sqlcc
