#include "database_manager.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace sqlcc;

class DatabaseManagerTest : public ::testing::Test {
protected:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::string db_path_ = "./test_db";

  void SetUp() override {
    // 在每个测试前创建DatabaseManager实例
    db_manager_ = std::make_shared<DatabaseManager>(db_path_, 1024, 4, 4);
  }

  void TearDown() override {
    // 在每个测试后关闭DatabaseManager
    if (db_manager_) {
      db_manager_->Close();
    }
  }
};

// 测试构造函数和析构函数
TEST_F(DatabaseManagerTest, ConstructorDestructor) {
  // 构造函数在SetUp中调用，析构函数在TearDown中调用
  EXPECT_TRUE(db_manager_ != nullptr);
}

// 测试数据库管理功能
TEST_F(DatabaseManagerTest, DatabaseManagement) {
  // 测试创建数据库
  EXPECT_TRUE(db_manager_->CreateDatabase("test_db1"));
  EXPECT_TRUE(db_manager_->CreateDatabase("test_db2"));

  // 测试创建已存在的数据库
  EXPECT_FALSE(db_manager_->CreateDatabase("test_db1"));

  // 测试列出数据库
  std::vector<std::string> databases = db_manager_->ListDatabases();
  EXPECT_GE(databases.size(), 2);

  // 测试数据库是否存在
  EXPECT_TRUE(db_manager_->DatabaseExists("test_db1"));
  EXPECT_FALSE(db_manager_->DatabaseExists("non_existent_db"));

  // 测试使用数据库
  EXPECT_TRUE(db_manager_->UseDatabase("test_db1"));
  EXPECT_EQ(db_manager_->GetCurrentDatabase(), "test_db1");

  // 测试使用不存在的数据库
  EXPECT_FALSE(db_manager_->UseDatabase("non_existent_db"));

  // 测试删除数据库
  EXPECT_TRUE(db_manager_->DropDatabase("test_db2"));
  EXPECT_FALSE(db_manager_->DatabaseExists("test_db2"));

  // 测试删除不存在的数据库
  EXPECT_FALSE(db_manager_->DropDatabase("non_existent_db"));

  // 测试删除当前使用的数据库
  EXPECT_TRUE(db_manager_->DropDatabase("test_db1"));
  EXPECT_EQ(db_manager_->GetCurrentDatabase(), "");
}

// 测试表管理功能
TEST_F(DatabaseManagerTest, TableManagement) {
  // 首先创建并使用数据库
  EXPECT_TRUE(db_manager_->CreateDatabase("test_db"));
  EXPECT_TRUE(db_manager_->UseDatabase("test_db"));

  // 测试创建表
  std::vector<std::pair<std::string, std::string>> columns = {
      {"id", "INT"}, {"name", "VARCHAR(50)"}, {"age", "INT"}};
  EXPECT_TRUE(db_manager_->CreateTable("test_table1", columns));
  EXPECT_TRUE(db_manager_->CreateTable("test_table2", columns));

  // 测试创建已存在的表
  EXPECT_FALSE(db_manager_->CreateTable("test_table1", columns));

  // 测试表是否存在
  EXPECT_TRUE(db_manager_->TableExists("test_table1"));
  EXPECT_FALSE(db_manager_->TableExists("non_existent_table"));

  // 测试列出表
  std::vector<std::string> tables = db_manager_->ListTables();
  EXPECT_EQ(tables.size(), 2);

  // 测试删除表
  EXPECT_TRUE(db_manager_->DropTable("test_table1"));
  EXPECT_FALSE(db_manager_->TableExists("test_table1"));

  // 测试删除不存在的表
  EXPECT_FALSE(db_manager_->DropTable("non_existent_table"));

  // 测试在未选择数据库时创建表
  db_manager_->DropDatabase("test_db");
  EXPECT_FALSE(db_manager_->CreateTable("test_table", columns));
}

// 测试事务管理功能
TEST_F(DatabaseManagerTest, TransactionManagement) {
  // 测试开始事务
  TransactionId txn_id1 =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);
  EXPECT_NE(txn_id1, 0);

  TransactionId txn_id2 =
      db_manager_->BeginTransaction(IsolationLevel::REPEATABLE_READ);
  EXPECT_NE(txn_id2, 0);
  EXPECT_NE(txn_id1, txn_id2);

  // 测试提交事务
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id1));
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id2));

  // 测试回滚事务
  TransactionId txn_id3 =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);
  EXPECT_TRUE(db_manager_->RollbackTransaction(txn_id3));

  // 测试提交已回滚的事务
  EXPECT_FALSE(db_manager_->CommitTransaction(txn_id3));

  // 测试回滚已提交的事务
  EXPECT_FALSE(db_manager_->RollbackTransaction(txn_id1));
}

// 测试页面读写功能
TEST_F(DatabaseManagerTest, PageReadWrite) {
  // 测试开始事务
  TransactionId txn_id =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);

  // 测试读取页面（这里我们不实际测试读取内容，因为页面可能不存在）
  Page *page = nullptr;
  EXPECT_FALSE(db_manager_->ReadPage(txn_id, 1, &page));

  // 测试提交事务
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id));
}

// 测试锁管理功能
TEST_F(DatabaseManagerTest, LockManagement) {
  // 测试开始事务
  TransactionId txn_id =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);

  // 测试加锁
  EXPECT_TRUE(db_manager_->LockKey(txn_id, "test_key"));

  // 测试解锁
  EXPECT_TRUE(db_manager_->UnlockKey(txn_id, "test_key"));

  // 测试提交事务
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id));
}

// 测试关闭数据库
TEST_F(DatabaseManagerTest, CloseDatabase) {
  // 测试关闭数据库
  EXPECT_TRUE(db_manager_->Close());

  // 测试在已关闭的数据库上执行操作
  EXPECT_FALSE(db_manager_->CreateDatabase("test_db"));

  // 测试再次关闭已关闭的数据库
  EXPECT_TRUE(db_manager_->Close());
}

// 测试刷新所有页面
TEST_F(DatabaseManagerTest, FlushAllPages) {
  // 测试刷新所有页面
  EXPECT_TRUE(db_manager_->FlushAllPages());
}

// 测试在关闭数据库后调用各种方法的行为
TEST_F(DatabaseManagerTest, OperationsAfterClose) {
  // 关闭数据库
  EXPECT_TRUE(db_manager_->Close());

  // 测试在关闭后创建数据库
  EXPECT_FALSE(db_manager_->CreateDatabase("test_db"));

  // 测试在关闭后列出数据库
  std::vector<std::string> databases = db_manager_->ListDatabases();
  EXPECT_TRUE(databases.empty());

  // 测试在关闭后检查数据库是否存在
  EXPECT_FALSE(db_manager_->DatabaseExists("test_db"));

  // 测试在关闭后开始事务
  EXPECT_THROW(db_manager_->BeginTransaction(), std::exception);

  // 测试在关闭后刷新所有页面
  EXPECT_THROW(db_manager_->FlushAllPages(), std::exception);
}

// 测试在未选择数据库时调用表管理方法的行为
TEST_F(DatabaseManagerTest, TableOperationsWithoutDatabase) {
  // 测试在未选择数据库时创建表
  std::vector<std::pair<std::string, std::string>> columns = {
      {"id", "INT"}, {"name", "VARCHAR(50)"}};
  EXPECT_FALSE(db_manager_->CreateTable("test_table", columns));

  // 测试在未选择数据库时删除表
  EXPECT_FALSE(db_manager_->DropTable("test_table"));

  // 测试在未选择数据库时检查表是否存在
  EXPECT_FALSE(db_manager_->TableExists("test_table"));

  // 测试在未选择数据库时列出表
  std::vector<std::string> tables = db_manager_->ListTables();
  EXPECT_TRUE(tables.empty());
}

// 测试GetCurrentDatabase方法在未选择数据库时的行为
TEST_F(DatabaseManagerTest, GetCurrentDatabaseWithoutSelection) {
  // 初始状态下未选择数据库
  EXPECT_TRUE(db_manager_->GetCurrentDatabase().empty());

  // 创建并选择数据库
  EXPECT_TRUE(db_manager_->CreateDatabase("test_db"));
  EXPECT_TRUE(db_manager_->UseDatabase("test_db"));
  EXPECT_EQ(db_manager_->GetCurrentDatabase(), "test_db");

  // 删除当前数据库后，GetCurrentDatabase应返回空字符串
  EXPECT_TRUE(db_manager_->DropDatabase("test_db"));
  EXPECT_TRUE(db_manager_->GetCurrentDatabase().empty());
}

// 测试WritePage方法的锁机制
TEST_F(DatabaseManagerTest, WritePageLockMechanism) {
  // 开始事务
  TransactionId txn_id =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);

  // 测试WritePage方法是否正确获取和释放锁
  Page *page = nullptr;
  // 注意：这里我们不实际测试写入内容，因为页面可能不存在
  EXPECT_FALSE(db_manager_->WritePage(txn_id, 1, page));

  // 提交事务
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id));
}

// 测试多次调用Close方法
TEST_F(DatabaseManagerTest, MultipleCloseCalls) {
  // 第一次关闭
  EXPECT_TRUE(db_manager_->Close());

  // 第二次关闭应该也返回true
  EXPECT_TRUE(db_manager_->Close());

  // 第三次关闭应该也返回true
  EXPECT_TRUE(db_manager_->Close());
}

// 测试事务状态检查
TEST_F(DatabaseManagerTest, TransactionStateChecks) {
  // 开始事务
  TransactionId txn_id =
      db_manager_->BeginTransaction(IsolationLevel::READ_COMMITTED);

  // 提交事务
  EXPECT_TRUE(db_manager_->CommitTransaction(txn_id));

  // 测试在已提交事务上调用ReadPage
  Page *page = nullptr;
  EXPECT_FALSE(db_manager_->ReadPage(txn_id, 1, &page));

  // 测试在已提交事务上调用WritePage
  EXPECT_FALSE(db_manager_->WritePage(txn_id, 1, page));

  // 测试在已提交事务上调用LockKey
  // 注意：根据实际实现，LockKey在已提交事务上不会抛出异常，而是返回false
  EXPECT_FALSE(db_manager_->LockKey(txn_id, "test_key"));

  // 测试在已提交事务上调用UnlockKey
  // 注意：根据实际实现，UnlockKey在已提交事务上不会抛出异常，而是返回true
  EXPECT_TRUE(db_manager_->UnlockKey(txn_id, "test_key"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
