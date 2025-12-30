#include "core/stored_procedure_manager.h"
#include "utils/config_manager.h"
#include "core/core_database_manager.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

namespace sqlcc {
namespace core {
namespace test {

class StoredProcedureManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    config_manager_ = std::make_unique<ConfigManager>();
    database_manager_ = std::make_unique<DatabaseManager>("./test_db");
    spm_ = std::make_unique<StoredProcedureManager>(*config_manager_, *database_manager_);
  }

  void TearDown() override {
    spm_.reset();
    database_manager_.reset();
    config_manager_.reset();
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<DatabaseManager> database_manager_;
  std::unique_ptr<StoredProcedureManager> spm_;
};

// 测试基本功能
TEST_F(StoredProcedureManagerTest, BasicFunctionality) {
  // 测试初始状态
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_procedures.load(), 0);
  EXPECT_EQ(stats.total_triggers.load(), 0);

  // 测试空列表
  auto procedures = spm_->ListProcedures();
  EXPECT_TRUE(procedures.empty());

  auto triggers = spm_->ListTriggers();
  EXPECT_TRUE(triggers.empty());
}

// 测试存储过程创建
TEST_F(StoredProcedureManagerTest, CreateProcedure) {
  // 创建存储过程定义
  StoredProcedure proc("test_proc", "public");
  proc.body = "SELECT * FROM users WHERE id = $1";
  proc.language = "SQL";

  // 添加参数
  proc.parameters.emplace_back("user_id", "INTEGER");

  // 创建存储过程
  EXPECT_TRUE(spm_->CreateProcedure(proc));

  // 验证统计信息
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_procedures.load(), 1);

  // 获取存储过程
  auto retrieved_proc = spm_->GetProcedure("public", "test_proc");
  ASSERT_NE(retrieved_proc, nullptr);
  EXPECT_EQ(retrieved_proc->name, "test_proc");
  EXPECT_EQ(retrieved_proc->schema, "public");
  EXPECT_EQ(retrieved_proc->parameters.size(), 1);
  EXPECT_EQ(retrieved_proc->parameters[0].name, "user_id");
}

// 测试存储过程删除
TEST_F(StoredProcedureManagerTest, DropProcedure) {
  // 创建存储过程
  StoredProcedure proc("test_proc", "public");
  proc.body = "SELECT * FROM users";
  spm_->CreateProcedure(proc);

  // 验证存在
  auto retrieved_proc = spm_->GetProcedure("public", "test_proc");
  ASSERT_NE(retrieved_proc, nullptr);

  // 删除存储过程
  EXPECT_TRUE(spm_->DropProcedure("public", "test_proc"));

  // 验证已删除
  auto deleted_proc = spm_->GetProcedure("public", "test_proc");
  EXPECT_EQ(deleted_proc, nullptr);

  // 验证统计信息
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_procedures.load(), 0);
}

// 测试触发器创建
TEST_F(StoredProcedureManagerTest, CreateTrigger) {
  // 创建触发器定义
  Trigger trigger("test_trigger", "users", "INSERT");
  trigger.body = "UPDATE statistics SET count = count + 1";
  trigger.language = "SQL";
  trigger.condition = "NEW.status = 'active'";

  // 创建触发器
  EXPECT_TRUE(spm_->CreateTrigger(trigger));

  // 验证统计信息
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_triggers.load(), 1);

  // 获取触发器列表
  auto triggers = spm_->ListTriggers();
  EXPECT_EQ(triggers.size(), 1);
  EXPECT_EQ(triggers[0]->name, "test_trigger");
  EXPECT_EQ(triggers[0]->table_name, "users");
  EXPECT_EQ(triggers[0]->event, "INSERT");
}

// 测试触发器删除
TEST_F(StoredProcedureManagerTest, DropTrigger) {
  // 创建触发器
  Trigger trigger("test_trigger", "users", "UPDATE");
  spm_->CreateTrigger(trigger);

  // 验证存在
  auto triggers_before = spm_->ListTriggers();
  EXPECT_EQ(triggers_before.size(), 1);

  // 删除触发器
  EXPECT_TRUE(spm_->DropTrigger("public", "test_trigger"));

  // 验证已删除
  auto triggers_after = spm_->ListTriggers();
  EXPECT_TRUE(triggers_after.empty());

  // 验证统计信息
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_triggers.load(), 0);
}

// 测试触发器启用/禁用
TEST_F(StoredProcedureManagerTest, SetTriggerEnabled) {
  // 创建触发器
  Trigger trigger("test_trigger", "users", "DELETE");
  trigger.enabled = true;
  spm_->CreateTrigger(trigger);

  // 禁用触发器
  EXPECT_TRUE(spm_->SetTriggerEnabled("public", "test_trigger", false));

  // 验证状态
  auto triggers = spm_->ListTriggers();
  ASSERT_EQ(triggers.size(), 1);
  EXPECT_FALSE(triggers[0]->enabled);

  // 重新启用
  EXPECT_TRUE(spm_->SetTriggerEnabled("public", "test_trigger", true));
  triggers = spm_->ListTriggers();
  ASSERT_EQ(triggers.size(), 1);
  EXPECT_TRUE(triggers[0]->enabled);
}

// 测试获取表触发器
TEST_F(StoredProcedureManagerTest, GetTriggersForTable) {
  // 创建多个触发器
  Trigger insert_trigger("insert_trigger", "users", "INSERT");
  Trigger update_trigger("update_trigger", "users", "UPDATE");
  Trigger delete_trigger("delete_trigger", "products", "DELETE");

  spm_->CreateTrigger(insert_trigger);
  spm_->CreateTrigger(update_trigger);
  spm_->CreateTrigger(delete_trigger);

  // 获取users表的INSERT触发器
  auto insert_triggers = spm_->GetTriggersForTable("users", "INSERT");
  EXPECT_EQ(insert_triggers.size(), 1);
  EXPECT_EQ(insert_triggers[0]->name, "insert_trigger");

  // 获取users表的所有触发器
  auto all_user_triggers = spm_->GetTriggersForTable("users", "");
  EXPECT_EQ(all_user_triggers.size(), 2);

  // 获取products表的触发器
  auto product_triggers = spm_->GetTriggersForTable("products", "DELETE");
  EXPECT_EQ(product_triggers.size(), 1);
  EXPECT_EQ(product_triggers[0]->name, "delete_trigger");
}

// 测试参数化存储过程
TEST_F(StoredProcedureManagerTest, ParameterizedProcedure) {
  // 创建带参数的存储过程
  StoredProcedure proc("get_user", "public");
  proc.body = "SELECT * FROM users WHERE id = $1 AND status = $2";
  proc.parameters.emplace_back("user_id", "INTEGER");
  proc.parameters.emplace_back("user_status", "VARCHAR");

  EXPECT_TRUE(spm_->CreateProcedure(proc));

  // 验证参数
  auto retrieved_proc = spm_->GetProcedure("public", "get_user");
  ASSERT_NE(retrieved_proc, nullptr);
  EXPECT_EQ(retrieved_proc->parameters.size(), 2);
  EXPECT_EQ(retrieved_proc->parameters[0].type, "INTEGER");
  EXPECT_EQ(retrieved_proc->parameters[1].type, "VARCHAR");
}

// 测试存储过程列表
TEST_F(StoredProcedureManagerTest, ListProcedures) {
  // 创建多个存储过程
  StoredProcedure proc1("proc1", "public");
  StoredProcedure proc2("proc2", "public");
  StoredProcedure proc3("proc3", "schema1");

  spm_->CreateProcedure(proc1);
  spm_->CreateProcedure(proc2);
  spm_->CreateProcedure(proc3);

  // 获取所有存储过程
  auto all_procedures = spm_->ListProcedures();
  EXPECT_EQ(all_procedures.size(), 3);

  // 获取public模式下的存储过程
  auto public_procedures = spm_->ListProcedures("public");
  EXPECT_EQ(public_procedures.size(), 2);

  // 获取schema1模式下的存储过程
  auto schema1_procedures = spm_->ListProcedures("schema1");
  EXPECT_EQ(schema1_procedures.size(), 1);
}

// 测试触发器列表
TEST_F(StoredProcedureManagerTest, ListTriggers) {
  // 创建多个触发器
  Trigger trigger1("trigger1", "users", "INSERT");
  Trigger trigger2("trigger2", "users", "UPDATE");
  Trigger trigger3("trigger3", "products", "DELETE");

  spm_->CreateTrigger(trigger1);
  spm_->CreateTrigger(trigger2);
  spm_->CreateTrigger(trigger3);

  // 获取所有触发器
  auto all_triggers = spm_->ListTriggers();
  EXPECT_EQ(all_triggers.size(), 3);

  // 获取public模式下的触发器
  auto public_triggers = spm_->ListTriggers("public");
  EXPECT_EQ(public_triggers.size(), 3); // 都在public模式下
}

// 测试统计信息
TEST_F(StoredProcedureManagerTest, Statistics) {
  // 重置统计信息
  spm_->ResetStats();
  auto initial_stats = spm_->GetStats();
  EXPECT_EQ(initial_stats.total_procedures.load(), 0);
  EXPECT_EQ(initial_stats.total_triggers.load(), 0);

  // 创建存储过程和触发器
  StoredProcedure proc("test_proc", "public");
  spm_->CreateProcedure(proc);

  Trigger trigger("test_trigger", "users", "INSERT");
  spm_->CreateTrigger(trigger);

  // 验证统计信息更新
  auto final_stats = spm_->GetStats();
  EXPECT_EQ(final_stats.total_procedures.load(), 1);
  EXPECT_EQ(final_stats.total_triggers.load(), 1);
}

// 测试并发访问
TEST_F(StoredProcedureManagerTest, ConcurrentAccess) {
  const int num_threads = 5;
  const int procedures_per_thread = 10;

  // 创建多个线程并发创建存储过程
  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, t, procedures_per_thread]() {
      for (int i = 0; i < procedures_per_thread; ++i) {
        StoredProcedure proc("proc_" + std::to_string(t) + "_" + std::to_string(i), "public");
        proc.body = "SELECT " + std::to_string(i);
        spm_->CreateProcedure(proc);
      }
    });
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
    thread.join();
  }

  // 验证总存储过程数
  auto procedures = spm_->ListProcedures();
  EXPECT_EQ(procedures.size(), num_threads * procedures_per_thread);

  // 验证统计信息
  auto stats = spm_->GetStats();
  EXPECT_EQ(stats.total_procedures.load(), num_threads * procedures_per_thread);
}

// 测试边界条件
TEST_F(StoredProcedureManagerTest, EdgeCases) {
  // 测试空名称
  StoredProcedure empty_proc("", "public");
  EXPECT_FALSE(spm_->CreateProcedure(empty_proc));

  // 测试不存在的存储过程删除
  EXPECT_FALSE(spm_->DropProcedure("public", "nonexistent"));

  // 测试不存在的触发器删除
  EXPECT_FALSE(spm_->DropTrigger("public", "nonexistent"));

  // 测试获取不存在的存储过程
  auto nonexistent_proc = spm_->GetProcedure("public", "nonexistent");
  EXPECT_EQ(nonexistent_proc, nullptr);

  // 测试重复创建（应该允许覆盖）
  StoredProcedure proc("test_proc", "public");
  proc.body = "SELECT 1";
  EXPECT_TRUE(spm_->CreateProcedure(proc));

  StoredProcedure proc2("test_proc", "public");
  proc2.body = "SELECT 2";
  EXPECT_TRUE(spm_->CreateProcedure(proc2));

  // 验证被覆盖
  auto retrieved_proc = spm_->GetProcedure("public", "test_proc");
  ASSERT_NE(retrieved_proc, nullptr);
  EXPECT_EQ(retrieved_proc->body, "SELECT 2");
}

// 测试复杂触发器条件
TEST_F(StoredProcedureManagerTest, ComplexTrigger) {
  // 创建复杂触发器
  Trigger trigger("audit_trigger", "users", "UPDATE");
  trigger.timing = "AFTER";
  trigger.columns = {"email", "status"};
  trigger.condition = "OLD.email != NEW.email OR OLD.status != NEW.status";
  trigger.body = R"(
    INSERT INTO audit_log (table_name, operation, old_values, new_values, timestamp)
    VALUES ('users', 'UPDATE',
            json_build_object('email', OLD.email, 'status', OLD.status),
            json_build_object('email', NEW.email, 'status', NEW.status),
            NOW())
  )";
  trigger.language = "SQL";

  EXPECT_TRUE(spm_->CreateTrigger(trigger));

  // 验证触发器属性
  auto triggers = spm_->ListTriggers();
  ASSERT_EQ(triggers.size(), 1);
  auto& t = *triggers[0];
  EXPECT_EQ(t.columns.size(), 2);
  EXPECT_FALSE(t.condition.empty());
  EXPECT_FALSE(t.body.empty());
}

} // namespace test
} // namespace core
} // namespace sqlcc
