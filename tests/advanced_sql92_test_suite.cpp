#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "execution/function_executor.h"
#include "trigger/sql_trigger_executor.h"
#include "transaction/savepoint_manager.h"
#include "types/domain_manager.h"
#include "procedure/procedure_vm.h"
#include "sql_parser/function_ast.h"
#include "trigger/trigger_manager.h"
#include "transaction/transaction_manager.h"

/**
 * @brief 高级SQL-92特性综合测试套件
 *
 * 测试以下特性：
 * 1. 存储过程和函数
 * 2. 触发器完整实现
 * 3. 事务保存点
 * 4. 用户定义类型(DOMAIN)
 */

class AdvancedSQL92Test : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试环境
        sql_executor_ = std::make_shared<SqlExecutor>();
    }

    void TearDown() override {
        // 清理测试环境
        FunctionExecutor::getInstance().unregisterFunction("test_function");
        DomainManager::getInstance().dropDomain("test_domain");
        SavepointManager::getInstance().clearTransactionSavepoints(1);
    }

    std::shared_ptr<SqlExecutor> sql_executor_;
};

// =============================================================================
// 函数测试
// =============================================================================

TEST_F(AdvancedSQL92Test, FunctionCreationAndExecution) {
    // 创建函数定义
    auto func_def = std::make_unique<FunctionDefinition>("test_function", "INTEGER");
    func_def->addParameter(FunctionParameter("x", "INTEGER"));
    func_def->addParameter(FunctionParameter("y", "INTEGER"));
    func_def->setBody("SELECT $1 + $2"); // 简单的加法函数
    func_def->addCharacteristic("DETERMINISTIC");

    // 注册函数
    ASSERT_TRUE(FunctionExecutor::getInstance().registerFunction(
        std::make_unique<SqlUserDefinedFunction>(std::move(func_def))));

    // 验证函数存在
    ASSERT_TRUE(FunctionExecutor::getInstance().functionExists("test_function"));

    // 执行函数（这里是模拟执行，实际需要SQL执行器）
    // 注意：实际执行需要完整的SQL执行环境
    auto func = FunctionExecutor::getInstance().getFunction("test_function");
    ASSERT_TRUE(func != nullptr);
    ASSERT_EQ(func->getName(), "test_function");
    ASSERT_TRUE(func->isDeterministic());
}

TEST_F(AdvancedSQL92Test, FunctionParameterValidation) {
    auto func_def = std::make_unique<FunctionDefinition>("param_test", "VARCHAR");
    func_def->addParameter(FunctionParameter("input", "VARCHAR"));

    auto func = std::make_unique<SqlUserDefinedFunction>(std::move(func_def));
    ASSERT_TRUE(FunctionExecutor::getInstance().registerFunction(std::move(func)));

    // 验证参数检查
    auto func_ptr = FunctionExecutor::getInstance().getFunction("param_test");
    ASSERT_TRUE(func_ptr != nullptr);

    // 参数数量不匹配应该失败
    std::vector<Value> wrong_args = {Value("test")}; // 缺少参数
    ASSERT_FALSE(FunctionExecutor::getInstance().validateArguments(*func_ptr, wrong_args));
}

TEST_F(AdvancedSQL92Test, FunctionCharacteristics) {
    auto func_def = std::make_unique<FunctionDefinition>("char_test", "BOOLEAN");
    func_def->addCharacteristic("DETERMINISTIC");
    func_def->addCharacteristic("CONTAINS SQL");
    func_def->addCharacteristic("READS SQL DATA");

    auto func = std::make_unique<SqlUserDefinedFunction>(std::move(func_def));
    ASSERT_TRUE(FunctionExecutor::getInstance().registerFunction(std::move(func)));

    auto func_ptr = FunctionExecutor::getInstance().getFunction("char_test");
    ASSERT_TRUE(func_ptr != nullptr);
    ASSERT_TRUE(func_ptr->isDeterministic());
    ASSERT_TRUE(func_ptr->containsSql());
    ASSERT_TRUE(func_ptr->readsSqlData());
    ASSERT_FALSE(func_ptr->modifiesSqlData());
}

// =============================================================================
// 触发器测试
// =============================================================================

TEST_F(AdvancedSQL92Test, TriggerCreationAndRegistration) {
    // 创建触发器定义
    auto trigger_def = std::make_unique<trigger::TriggerDefinition>(
        "test_trigger",
        trigger::TriggerTiming::BEFORE,
        trigger::TriggerEvent::INSERT,
        trigger::TriggerLevel::ROW,
        "test_table"
    );

    trigger_def->setBody("UPDATE audit_log SET count = count + 1 WHERE table_name = 'test_table'");
    trigger_def->setCondition("NEW.status IS NOT NULL");

    // 注册触发器
    ASSERT_TRUE(trigger::TriggerManager::getInstance().createTrigger(std::move(trigger_def)));

    // 验证触发器存在
    auto trigger = trigger::TriggerManager::getInstance().getTrigger("test_trigger");
    ASSERT_TRUE(trigger != nullptr);
    ASSERT_EQ(trigger->getName(), "test_trigger");
    ASSERT_EQ(trigger->getTiming(), trigger::TriggerTiming::BEFORE);
    ASSERT_EQ(trigger->getEvent(), trigger::TriggerEvent::INSERT);
    ASSERT_EQ(trigger->getTableName(), "test_table");
}

TEST_F(AdvancedSQL92Test, TriggerVariableSubstitution) {
    // 创建SQL触发器执行器
    trigger::SQLTriggerExecutor executor;
    executor.setSqlExecutor(sql_executor_);

    // 创建触发器定义
    trigger::TriggerDefinition trigger_def(
        "var_test",
        trigger::TriggerTiming::BEFORE,
        trigger::TriggerEvent::UPDATE,
        trigger::TriggerLevel::ROW,
        "users"
    );

    trigger_def.setBody("INSERT INTO audit_log VALUES (:NEW.id, :OLD.email, :NEW.email, NOW())");

    // 创建测试行数据
    trigger::RowData old_row;
    old_row.columns = {"id", "email", "status"};
    old_row.values = {"1", "old@example.com", "active"};

    trigger::RowData new_row;
    new_row.columns = {"id", "email", "status"};
    new_row.values = {"1", "new@example.com", "inactive"};

    // 执行触发器（这里是测试变量替换逻辑）
    std::string processed_sql = executor.substituteTriggerVariables(
        trigger_def.getBody(), &old_row, &new_row);

    // 验证变量被正确替换
    ASSERT_NE(processed_sql.find("'1'"), std::string::npos);
    ASSERT_NE(processed_sql.find("'old@example.com'"), std::string::npos);
    ASSERT_NE(processed_sql.find("'new@example.com'"), std::string::npos);
}

TEST_F(AdvancedSQL92Test, TriggerEventHandling) {
    // 测试触发器事件触发逻辑
    trigger::RowData row_data;
    row_data.columns = {"id", "name"};
    row_data.values = {"1", "test"};

    std::vector<trigger::RowData> old_rows = {row_data};
    std::vector<trigger::RowData> new_rows = {row_data};

    // 触发BEFORE INSERT事件
    bool result = trigger::TriggerManager::getInstance().fireTriggers(
        trigger::TriggerTiming::BEFORE,
        trigger::TriggerEvent::INSERT,
        "test_table",
        old_rows,
        new_rows
    );

    // 即使没有触发器，也应该成功（无错误）
    ASSERT_TRUE(result);
}

// =============================================================================
// 事务保存点测试
// =============================================================================

TEST_F(AdvancedSQL92Test, SavepointCreationAndManagement) {
    TransactionId txn_id = 100;

    // 创建保存点
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "savepoint1"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "savepoint2"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "savepoint3"));

    // 验证保存点存在
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "savepoint1"));
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "savepoint2"));
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "savepoint3"));

    // 获取保存点
    auto savepoint = SavepointManager::getInstance().getSavepoint(txn_id, "savepoint2");
    ASSERT_TRUE(savepoint != nullptr);
    ASSERT_EQ(savepoint->getName(), "savepoint2");

    // 获取事务的所有保存点
    auto all_savepoints = SavepointManager::getInstance().getTransactionSavepoints(txn_id);
    ASSERT_EQ(all_savepoints.size(), 3);
}

TEST_F(AdvancedSQL92Test, SavepointRollback) {
    TransactionId txn_id = 200;

    // 创建多个保存点
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp1"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp2"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp3"));

    // 回滚到中间的保存点
    ASSERT_TRUE(SavepointManager::getInstance().rollbackToSavepoint(txn_id, "sp2"));

    // 验证只有sp1和sp2保留，sp3被删除
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "sp1"));
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "sp2"));
    ASSERT_FALSE(SavepointManager::getInstance().savepointExists(txn_id, "sp3"));
}

TEST_F(AdvancedSQL92Test, SavepointRelease) {
    TransactionId txn_id = 300;

    // 创建保存点
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp1"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp2"));
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "sp3"));

    // 释放sp2保存点
    ASSERT_TRUE(SavepointManager::getInstance().releaseSavepoint(txn_id, "sp2"));

    // 验证sp2及其之后的保存点被释放
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "sp1"));
    ASSERT_FALSE(SavepointManager::getInstance().savepointExists(txn_id, "sp2"));
    ASSERT_FALSE(SavepointManager::getInstance().savepointExists(txn_id, "sp3"));
}

TEST_F(AdvancedSQL92Test, SavepointErrorHandling) {
    TransactionId txn_id = 400;

    // 尝试释放不存在的保存点
    ASSERT_FALSE(SavepointManager::getInstance().releaseSavepoint(txn_id, "nonexistent"));
    ASSERT_EQ(SavepointManager::getInstance().getLastError(),
              "Savepoint 'nonexistent' not found for transaction 400");

    // 尝试创建重复的保存点
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "test_sp"));
    ASSERT_FALSE(SavepointManager::getInstance().createSavepoint(txn_id, "test_sp"));
    ASSERT_EQ(SavepointManager::getInstance().getLastError(),
              "Savepoint 'test_sp' already exists for transaction 400");
}

// =============================================================================
// 用户定义类型(DOMAIN)测试
// =============================================================================

TEST_F(AdvancedSQL92Test, DomainCreationAndValidation) {
    // 创建域定义
    auto domain = std::make_unique<DomainDefinition>("email_domain", "VARCHAR");
    domain->setCheckConstraint("VALUE LIKE '%@%'");
    domain->setDefaultValue("user@example.com");
    domain->setNullable(false);

    // 创建域
    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(domain)));

    // 验证域存在
    ASSERT_TRUE(DomainManager::getInstance().domainExists("email_domain"));

    // 获取域
    auto domain_ptr = DomainManager::getInstance().getDomain("email_domain");
    ASSERT_TRUE(domain_ptr != nullptr);
    ASSERT_EQ(domain_ptr->getName(), "email_domain");
    ASSERT_EQ(domain_ptr->getBaseType(), "VARCHAR");
    ASSERT_FALSE(domain_ptr->isNullable());
    ASSERT_EQ(domain_ptr->getDefaultValue(), "user@example.com");
}

TEST_F(AdvancedSQL92Test, DomainValueValidation) {
    // 创建带约束的域
    auto domain = std::make_unique<DomainDefinition>("positive_int", "INTEGER");
    domain->setCheckConstraint("VALUE > 0");
    domain->setNullable(false);

    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(domain)));

    // 验证有效值
    ASSERT_TRUE(DomainManager::getInstance().validateDomainValue("positive_int", Value(10)));
    ASSERT_TRUE(DomainManager::getInstance().validateDomainValue("positive_int", Value(1)));

    // 验证无效值
    ASSERT_FALSE(DomainManager::getInstance().validateDomainValue("positive_int", Value(0)));
    ASSERT_FALSE(DomainManager::getInstance().validateDomainValue("positive_int", Value(-5)));
}

TEST_F(AdvancedSQL92Test, DomainManagement) {
    // 创建多个域
    auto domain1 = std::make_unique<DomainDefinition>("domain1", "INTEGER");
    auto domain2 = std::make_unique<DomainDefinition>("domain2", "VARCHAR");

    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(domain1)));
    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(domain2)));

    // 获取所有域名
    auto domains = DomainManager::getInstance().getAllDomainNames();
    ASSERT_EQ(domains.size(), 2);
    ASSERT_NE(std::find(domains.begin(), domains.end(), "domain1"), domains.end());
    ASSERT_NE(std::find(domains.begin(), domains.end(), "domain2"), domains.end());

    // 删除域
    ASSERT_TRUE(DomainManager::getInstance().dropDomain("domain1"));
    ASSERT_FALSE(DomainManager::getInstance().domainExists("domain1"));
    ASSERT_TRUE(DomainManager::getInstance().domainExists("domain2"));
}

TEST_F(AdvancedSQL92Test, DomainErrorHandling) {
    // 尝试创建重复域
    auto domain1 = std::make_unique<DomainDefinition>("duplicate_domain", "INTEGER");
    auto domain2 = std::make_unique<DomainDefinition>("duplicate_domain", "VARCHAR");

    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(domain1)));
    ASSERT_FALSE(DomainManager::getInstance().createDomain(std::move(domain2)));
    ASSERT_EQ(DomainManager::getInstance().getLastError(),
              "Domain 'duplicate_domain' already exists");

    // 尝试删除不存在的域
    ASSERT_FALSE(DomainManager::getInstance().dropDomain("nonexistent"));
    ASSERT_EQ(DomainManager::getInstance().getLastError(),
              "Domain 'nonexistent' does not exist");

    // 尝试验证不存在域的值
    ASSERT_FALSE(DomainManager::getInstance().validateDomainValue("nonexistent", Value(1)));
    ASSERT_EQ(DomainManager::getInstance().getLastError(),
              "Domain 'nonexistent' does not exist");
}

// =============================================================================
// 集成测试
// =============================================================================

TEST_F(AdvancedSQL92Test, ComprehensiveAdvancedSQL92Integration) {
    // 1. 创建用户定义类型
    auto email_domain = std::make_unique<DomainDefinition>("email_type", "VARCHAR");
    email_domain->setCheckConstraint("VALUE LIKE '%@%'");
    ASSERT_TRUE(DomainManager::getInstance().createDomain(std::move(email_domain)));

    // 2. 创建函数
    auto func_def = std::make_unique<FunctionDefinition>("validate_email", "BOOLEAN");
    func_def->addParameter(FunctionParameter("email", "VARCHAR"));
    func_def->setBody("SELECT $1 LIKE '%@%'");
    func_def->addCharacteristic("DETERMINISTIC");

    ASSERT_TRUE(FunctionExecutor::getInstance().registerFunction(
        std::make_unique<SqlUserDefinedFunction>(std::move(func_def))));

    // 3. 创建触发器
    auto trigger_def = std::make_unique<trigger::TriggerDefinition>(
        "email_validation_trigger",
        trigger::TriggerTiming::BEFORE,
        trigger::TriggerEvent::INSERT,
        trigger::TriggerLevel::ROW,
        "users"
    );

    trigger_def->setBody("SELECT validate_email(:NEW.email)");
    trigger_def->setCondition(":NEW.email IS NOT NULL");

    ASSERT_TRUE(trigger::TriggerManager::getInstance().createTrigger(std::move(trigger_def)));

    // 4. 创建事务保存点
    TransactionId txn_id = 999;
    ASSERT_TRUE(SavepointManager::getInstance().createSavepoint(txn_id, "before_insert"));

    // 验证所有组件都正常工作
    ASSERT_TRUE(DomainManager::getInstance().domainExists("email_type"));
    ASSERT_TRUE(FunctionExecutor::getInstance().functionExists("validate_email"));
    ASSERT_TRUE(trigger::TriggerManager::getInstance().getTrigger("email_validation_trigger") != nullptr);
    ASSERT_TRUE(SavepointManager::getInstance().savepointExists(txn_id, "before_insert"));

    // 清理
    SavepointManager::getInstance().clearTransactionSavepoints(txn_id);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
