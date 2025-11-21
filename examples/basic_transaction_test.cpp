// basic_transaction_test.cpp - 基本的非并发事务管理器测试
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

// 基本的类型定义
using TransactionId = uint64_t;

enum class TransactionState { ACTIVE, COMMITTED, ABORTED };
enum class IsolationLevel {
  READ_UNCOMMITTED,
  READ_COMMITTED,
  REPEATABLE_READ,
  SERIALIZABLE
};

// Transaction 类
struct Transaction {
  TransactionId id;
  TransactionState state;
  IsolationLevel isolation_level;
};

// TransactionManager 类 (简化版本，没有并发锁)
class TransactionManager {
private:
  std::vector<std::unique_ptr<Transaction>> transactions_;
  TransactionId next_id_ = 1;

public:
  TransactionManager() = default;

  TransactionId
  begin_transaction(IsolationLevel level = IsolationLevel::READ_COMMITTED) {
    auto txn = std::make_unique<Transaction>();
    txn->id = next_id_++;
    txn->state = TransactionState::ACTIVE;
    txn->isolation_level = level;

    transactions_.emplace_back(std::move(txn));
    return txn->id;
  }

  bool commit_transaction(TransactionId txn_id) {
    for (auto &txn : transactions_) {
      if (txn->id == txn_id && txn->state == TransactionState::ACTIVE) {
        txn->state = TransactionState::COMMITTED;
        return true;
      }
    }
    return false;
  }

  bool rollback_transaction(TransactionId txn_id) {
    for (auto &txn : transactions_) {
      if (txn->id == txn_id && txn->state == TransactionState::ACTIVE) {
        txn->state = TransactionState::ABORTED;
        return true;
      }
    }
    return false;
  }

  TransactionState get_transaction_state(TransactionId txn_id) {
    for (const auto &txn : transactions_) {
      if (txn->id == txn_id) {
        return txn->state;
      }
    }
    return TransactionState::ABORTED; // Default for not found
  }

  std::vector<TransactionId> get_active_transactions() {
    std::vector<TransactionId> active;
    for (const auto &txn : transactions_) {
      if (txn->state == TransactionState::ACTIVE) {
        active.push_back(txn->id);
      }
    }
    return active;
  }

  std::vector<TransactionId> get_all_transactions() {
    std::vector<TransactionId> all;
    for (const auto &txn : transactions_) {
      all.push_back(txn->id);
    }
    return all;
  }
};

std::string state_to_string(TransactionState state) {
  switch (state) {
  case TransactionState::ACTIVE:
    return "ACTIVE";
  case TransactionState::COMMITTED:
    return "COMMITTED";
  case TransactionState::ABORTED:
    return "ABORTED";
  default:
    return "UNKNOWN";
  }
}

int main() {
  std::cout << "🧪 基础事务管理器测试" << std::endl;
  std::cout << "======================" << std::endl << std::endl;

  TransactionManager txn_mgr;
  int tests_passed = 0;
  int total_tests = 4;

  // 测试1: 创建事务
  std::cout << "测试1: 创建事务" << std::endl;
  TransactionId txn1 = txn_mgr.begin_transaction();
  std::cout << "   事务ID: " << txn1 << std::endl;
  if (txn1 > 0) {
    tests_passed++;
    std::cout << "   ✅ 通过" << std::endl;
  } else {
    std::cout << "   ❌ 失败" << std::endl;
  }

  // 测试2: 检查事务状态
  std::cout << "测试2: 检查事务状态" << std::endl;
  TransactionState state = txn_mgr.get_transaction_state(txn1);
  std::cout << "   状态: " << state_to_string(state) << std::endl;
  if (state == TransactionState::ACTIVE) {
    tests_passed++;
    std::cout << "   ✅ 通过" << std::endl;
  } else {
    std::cout << "   ❌ 失败" << std::endl;
  }

  // 测试3: 提交事务
  std::cout << "测试3: 提交事务" << std::endl;
  if (txn_mgr.commit_transaction(txn1)) {
    state = txn_mgr.get_transaction_state(txn1);
    if (state == TransactionState::COMMITTED) {
      tests_passed++;
      std::cout << "   ✅ 通过 (状态: " << state_to_string(state) << ")"
                << std::endl;
    } else {
      std::cout << "   ❌ 提交成功但状态不对" << std::endl;
    }
  } else {
    std::cout << "   ❌ 提交失败" << std::endl;
  }

  // 测试4: 创建和回滚新事务
  std::cout << "测试4: 创建和回滚新事务" << std::endl;
  TransactionId txn2 = txn_mgr.begin_transaction();
  std::cout << "   新事务ID: " << txn2 << std::endl;

  if (txn_mgr.rollback_transaction(txn2)) {
    state = txn_mgr.get_transaction_state(txn2);
    if (state == TransactionState::ABORTED) {
      tests_passed++;
      std::cout << "   ✅ 通过 (状态: " << state_to_string(state) << ")"
                << std::endl;
    } else {
      std::cout << "   ❌ 回滚成功但状态不对" << std::endl;
    }
  } else {
    std::cout << "   ❌ 回滚失败" << std::endl;
  }

  // 显示总结
  std::cout << std::endl;
  std::cout << "======================" << std::endl;
  std::cout << "测试结果: " << tests_passed << "/" << total_tests << " 通过"
            << std::endl;

  auto all_txns = txn_mgr.get_all_transactions();
  auto active_txns = txn_mgr.get_active_transactions();
  std::cout << "总事务数: " << all_txns.size() << std::endl;
  std::cout << "活跃事务数: " << active_txns.size() << std::endl;

  if (tests_passed == total_tests) {
    std::cout << std::endl << "🎉 所有事务基本功能测试通过!" << std::endl;
    std::cout << "事务管理器核心算法正常工作。" << std::endl;
    return 0;
  } else {
    std::cout << std::endl << "❌ 部分测试失败" << std::endl;
    return 1;
  }
}
