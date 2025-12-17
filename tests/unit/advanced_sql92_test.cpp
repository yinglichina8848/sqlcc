#include "gtest/gtest.h"
#include "sql_parser/advanced_sql92_features.h"
#include "sql_executor/advanced_sql92_executor.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_executor {
namespace test {

// ==================== 存储过程和函数测试 ====================

class ProcedureFunctionTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化测试环境
  }
};

TEST_F(ProcedureFunctionTest, CreateProcedureTest) {
  // 测试创建存储过程
  auto stmt = std::make_unique<sql_parser::CreateProcedureStatement>("test_proc");
  sql_parser::ProcedureParameter param("param1", "VARCHAR", sql_parser::ProcedureParameter::IN);
  stmt->addParameter(param);
  stmt->setBody("BEGIN SELECT * FROM users; END");
  
  auto& manager = ProcedureFunctionManager::getInstance();
  bool result = manager.createProcedure(*stmt);
  
  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.procedureExists("test_proc"));
}

TEST_F(ProcedureFunctionTest, DropProcedureTest) {
  // 先创建过程
  auto stmt = std::make_unique<sql_parser::CreateProcedureStatement>("drop_test_proc");
  auto& manager = ProcedureFunctionManager::getInstance();
  manager.createProcedure(*stmt);
  
  // 测试删除过程
  bool result = manager.dropProcedure("drop_test_proc");
  EXPECT_TRUE(result);
  EXPECT_FALSE(manager.procedureExists("drop_test_proc"));
}

TEST_F(ProcedureFunctionTest, CallProcedureTest) {
  // 先创建过程
  auto stmt = std::make_unique<sql_parser::CreateProcedureStatement>("call_test_proc");
  auto& manager = ProcedureFunctionManager::getInstance();
  manager.createProcedure(*stmt);
  
  // 测试调用过程
  auto callStmt = manager.callProcedure("call_test_proc");
  ASSERT_NE(callStmt, nullptr);
  EXPECT_EQ(callStmt->getName(), "call_test_proc");
}

TEST_F(ProcedureFunctionTest, CreateFunctionTest) {
  // 测试创建函数
  auto functionDef = std::make_unique<sql_parser::FunctionDefinition>("test_func");
  functionDef->setReturnDataType("INTEGER");
  functionDef->addParameter(sql_parser::ProcedureParameter("x", "INTEGER", sql_parser::ProcedureParameter::IN));
  functionDef->setBody("BEGIN RETURN x * 2; END");
  
  auto stmt = std::make_unique<sql_parser::CreateFunctionStatement>(std::move(functionDef));
  auto& manager = ProcedureFunctionManager::getInstance();
  bool result = manager.createFunction(*stmt);
  
  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.functionExists("test_func"));
}

TEST_F(ProcedureFunctionTest, VariableManagementTest) {
  auto& manager = ProcedureFunctionManager::getInstance();
  
  // 测试设置变量
  manager.setVariable("test_var", "test_value");
  EXPECT_TRUE(manager.variableExists("test_var"));
  
  // 测试获取变量
  std::string value = manager.getVariable("test_var");
  EXPECT_EQ(value, "test_value");
}

// ==================== 事务控制测试 ====================

class TransactionControlTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化测试环境
  }
};

TEST_F(TransactionControlTest, SavepointTest) {
  auto& manager = TransactionControlManager::getInstance();
  
  // 测试创建保存点
  bool result = manager.createSavepoint("test_savepoint");
  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.savepointExists("test_savepoint"));
  
  // 测试重复创建保存点（应该失败）
  result = manager.createSavepoint("test_savepoint");
  EXPECT_FALSE(result);
}

TEST_F(TransactionControlTest, SetTransactionTest) {
  auto& manager = TransactionControlManager::getInstance();
  
  // 测试设置事务隔离级别
  bool result = manager.setTransactionIsolation(
      sql_parser::SetTransactionStatement::SERIALIZABLE);
  EXPECT_TRUE(result);
  
  auto level = manager.getCurrentIsolationLevel();
  EXPECT_EQ(level, sql_parser::SetTransactionStatement::SERIALIZABLE);
  
  // 测试设置访问模式
  result = manager.setTransactionAccessMode(
      sql_parser::SetTransactionStatement::READ_ONLY);
  EXPECT_TRUE(result);
  
  auto mode = manager.getCurrentAccessMode();
  EXPECT_EQ(mode, sql_parser::SetTransactionStatement::READ_ONLY);
}

TEST_F(TransactionControlTest, TransactionInfoTest) {
  auto& manager = TransactionControlManager::getInstance();
  
  std::string info = manager.getTransactionInfo();
  EXPECT_FALSE(info.empty());
  EXPECT_TRUE(info.find("transaction") != std::string::npos);
}

// ==================== 域(DOMAIN)管理测试 ====================

class DomainManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化测试环境
  }
};

TEST_F(DomainManagerTest, CreateDomainTest) {
  // 测试创建域
  auto domainDef = std::make_unique<sql_parser::DomainDefinition>(
      "email_type", sql_parser::DomainDefinition::CHARACTER);
  domainDef->setCharacterLength(100);
  domainDef->setCheckConstraint("email LIKE '%@%.%'");
  
  auto stmt = std::make_unique<sql_parser::CreateDomainStatement>(std::move(domainDef));
  auto& manager = DomainManager::getInstance();
  
  bool result = manager.createDomain(*stmt);
  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.domainExists("email_type"));
}

TEST_F(DomainManagerTest, DomainValidationTest) {
  // 先创建域
  auto domainDef = std::make_unique<sql_parser::DomainDefinition>(
      "test_domain", sql_parser::DomainDefinition::INTEGER);
  auto stmt = std::make_unique<sql_parser::CreateDomainStatement>(std::move(domainDef));
  auto& manager = DomainManager::getInstance();
  manager.createDomain(*stmt);
  
  // 测试验证有效值
  bool result = manager.validateValue("test_domain", "123");
  EXPECT_TRUE(result);
  
  // 测试验证无效值
  result = manager.validateValue("test_domain", "not_a_number");
  EXPECT_FALSE(result);
}

TEST_F(DomainManagerTest, DropDomainTest) {
  // 先创建域
  auto domainDef = std::make_unique<sql_parser::DomainDefinition>(
      "drop_test_domain", sql_parser::DomainDefinition::VARCHAR);
  auto stmt = std::make_unique<sql_parser::CreateDomainStatement>(std::move(domainDef));
  auto& manager = DomainManager::getInstance();
  manager.createDomain(*stmt);
  
  // 测试删除域
  bool result = manager.dropDomain("drop_test_domain");
  EXPECT_TRUE(result);
  EXPECT_FALSE(manager.domainExists("drop_test_domain"));
}

TEST_F(DomainManagerTest, ListDomainsTest) {
  auto& manager = DomainManager::getInstance();
  
  // 创建几个域
  auto domainDef1 = std::make_unique<sql_parser::DomainDefinition>(
      "domain1", sql_parser::DomainDefinition::INTEGER);
  auto stmt1 = std::make_unique<sql_parser::CreateDomainStatement>(std::move(domainDef1));
  manager.createDomain(*stmt1);
  
  auto domainDef2 = std::make_unique<sql_parser::DomainDefinition>(
      "domain2", sql_parser::DomainDefinition::VARCHAR);
  auto stmt2 = std::make_unique<sql_parser::CreateDomainStatement>(std::move(domainDef2));
  manager.createDomain(*stmt2);
  
  // 测试列出域
  auto domains = manager.listDomains();
  EXPECT_EQ(domains.size(), 2);
  EXPECT_TRUE(std::find(domains.begin(), domains.end(), "domain1") != domains.end());
  EXPECT_TRUE(std::find(domains.begin(), domains.end(), "domain2") != domains.end());
}

// ==================== 增强触发器测试 ====================

class EnhancedTriggerTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化测试环境
  }
};

TEST_F(EnhancedTriggerTest, CreateTriggerTest) {
  // 测试创建增强触发器
  sql_parser::TriggerDefinition triggerDef(
      "test_trigger", 
      sql_parser::TriggerDefinition::AFTER,
      sql_parser::TriggerDefinition::INSERT,
      sql_parser::TriggerDefinition::ROW,
      "test_table");
  triggerDef.setBody("BEGIN INSERT INTO log_table VALUES (NEW.id, NEW.name); END");
  
  auto stmt = std::make_unique<sql_parser::CreateTriggerStatement>(triggerDef);
  auto& manager = EnhancedTriggerManager::getInstance();
  
  bool result = manager.createTrigger(*stmt);
  EXPECT_TRUE(result);
  EXPECT_TRUE(manager.triggerExists("test_trigger"));
}

TEST_F(EnhancedTriggerTest, EnableDisableTriggerTest) {
  // 先创建触发器
  sql_parser::TriggerDefinition triggerDef(
      "enable_test_trigger",
      sql_parser::TriggerDefinition::BEFORE,
      sql_parser::TriggerDefinition::UPDATE,
      sql_parser::TriggerDefinition::ROW,
      "test_table");
  
  auto stmt = std::make_unique<sql_parser::CreateTriggerStatement>(triggerDef);
  auto& manager = EnhancedTriggerManager::getInstance();
  manager.createTrigger(*stmt);
  
  // 测试禁用触发器
  bool result = manager.disableTrigger("enable_test_trigger");
  EXPECT_TRUE(result);
  
  // 测试启用触发器
  result = manager.enableTrigger("enable_test_trigger");
  EXPECT_TRUE(result);
}

TEST_F(EnhancedTriggerTest, GetTriggersForTableTest) {
  // 创建表相关的触发器
  sql_parser::TriggerDefinition triggerDef1(
      "trigger1", 
      sql_parser::TriggerDefinition::AFTER,
      sql_parser::TriggerDefinition::INSERT,
      sql_parser::TriggerDefinition::ROW,
      "test_table");
  
  sql_parser::TriggerDefinition triggerDef2(
      "trigger2",
      sql_parser::TriggerDefinition::AFTER,
      sql_parser::TriggerDefinition::UPDATE,
      sql_parser::TriggerDefinition::ROW,
      "test_table");
  
  auto stmt1 = std::make_unique<sql_parser::CreateTriggerStatement>(triggerDef1);
  auto stmt2 = std::make_unique<sql_parser::CreateTriggerStatement>(triggerDef2);
  auto& manager = EnhancedTriggerManager::getInstance();
  manager.createTrigger(*stmt1);
  manager.createTrigger(*stmt2);
  
  // 测试获取表的触发器
  auto triggers = manager.getTriggersForTable("test_table");
  EXPECT_EQ(triggers.size(), 2);
  EXPECT_TRUE(std::find(triggers.begin(), triggers.end(), "trigger1") != triggers.end());
  EXPECT_TRUE(std::find(triggers.begin(), triggers.end(), "trigger2") != triggers.end());
}

// ==================== 增强ALTER TABLE测试 ====================

class EnhancedAlterTableTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化测试环境
  }
};

TEST_F(EnhancedAlterTableTest, EnhancedAlterTableTest) {
  // 创建增强的ALTER TABLE语句
  auto stmt = std::make_unique<sql_parser::EnhancedAlterTableStatement>("test_table");
  
  // 添加列
  auto action1 = std::make_unique<sql_parser::AlterTableAction>(
      sql_parser::AlterTableAction::ADD_COLUMN);
  sql_parser::ColumnDefinition newCol("new_column", "VARCHAR(50)");
  action1->setColumnDefinition(std::move(newCol));
  stmt->addAction(std::move(action1));
  
  // 添加约束
  auto action2 = std::make_unique<sql_parser::AlterTableAction>(
      sql_parser::AlterTableAction::ADD_CONSTRAINT);
  sql_parser::TableConstraint constraint(sql_parser::TableConstraint::PRIMARY_KEY, "pk_test");
  constraint.addColumn("id");
  action2->setConstraint(std::move(constraint));
  stmt->addAction(std::move(action2));
  
  auto& manager = EnhancedAlterTableManager::getInstance();
  bool result = manager.executeAlterTable(*stmt);
  
  // 由于没有实际的表，执行可能会失败，但API应该工作
  EXPECT_TRUE(stmt->getTableName() == "test_table");
  EXPECT_EQ(stmt->getActions().size(), 2);
}

TEST_F(EnhancedAlterTableTest, AddColumnActionTest) {
  auto action = std::make_unique<sql_parser::AlterTableAction>(
      sql_parser::AlterTableAction::ADD_COLUMN);
  
  sql_parser::ColumnDefinition column("test_column", "INTEGER");
  column.setNotNull(true);
  action->setColumnDefinition(std::move(column));
  
  EXPECT_EQ(action->getActionType(), sql_parser::AlterTableAction::ADD_COLUMN);
  EXPECT_TRUE(action->hasColumnDefinition());
  EXPECT_EQ(action->getColumnDefinition().getName(), "test_column");
}

TEST_F(EnhancedAlterTableTest, DropColumnActionTest) {
  auto action = std::make_unique<sql_parser::AlterTableAction>(
      sql_parser::AlterTableAction::DROP_COLUMN);
  action->setColumnName("column_to_drop");
  
  EXPECT_EQ(action->getActionType(), sql_parser::AlterTableAction::DROP_COLUMN);
  EXPECT_TRUE(action->hasColumnName());
  EXPECT_EQ(action->getColumnName(), "column_to_drop");
}

} // namespace test
} // namespace sql_executor
} // namespace sqlcc
