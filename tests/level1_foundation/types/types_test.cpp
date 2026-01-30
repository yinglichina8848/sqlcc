/**
 * @file types_test.cpp
 * @brief Types模块完整单元测试
 *
 * 测试覆盖：
 * 1. Value - 变量值类型
 * 2. DomainDefinition - 域定义
 * 3. DomainManager - 域管理器
 * 4. DomainDefinitionNode - 域定义节点
 * 5. TransactionId - 事务ID
 * 6. LockType - 锁类型枚举
 * 7. LockMode - 锁模式枚举
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

// 引入类型头文件
#include "src/types/domain_manager.h"
#include "src/types/transaction_types.h"

namespace sqlcc {
namespace test {

// ==================== Value Tests ====================

class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试默认构造
TEST_F(ValueTest, DefaultConstructor) {
    Value v;
    EXPECT_EQ(v.getType(), Value::NULL_VALUE);
}

// 测试整数构造
TEST_F(ValueTest, IntegerConstructor) {
    Value v(42);
    EXPECT_EQ(v.getType(), Value::INTEGER);
    EXPECT_EQ(v.asInteger(), 42);
}

// 测试双精度浮点构造
TEST_F(ValueTest, DoubleConstructor) {
    Value v(3.14159);
    EXPECT_EQ(v.getType(), Value::DOUBLE);
    EXPECT_DOUBLE_EQ(v.asDouble(), 3.14159);
}

// 测试字符串构造
TEST_F(ValueTest, StringConstructor) {
    Value v(std::string("Hello World"));
    EXPECT_EQ(v.getType(), Value::STRING);
    EXPECT_EQ(v.asString(), "Hello World");
}

// 测试布尔值构造
TEST_F(ValueTest, BooleanConstructor) {
    Value v1(true);
    EXPECT_EQ(v1.getType(), Value::BOOLEAN);
    EXPECT_TRUE(v1.asBoolean());

    Value v2(false);
    EXPECT_EQ(v2.getType(), Value::BOOLEAN);
    EXPECT_FALSE(v2.asBoolean());
}

// 测试toString方法
TEST_F(ValueTest, ToString) {
    Value v_int(42);
    EXPECT_EQ(v_int.toString(), "42");

    Value v_double(3.14);
    std::string double_str = v_double.toString();
    EXPECT_TRUE(double_str.find("3.14") != std::string::npos);

    Value v_str("test");
    EXPECT_EQ(v_str.toString(), "test");

    Value v_bool(true);
    EXPECT_EQ(v_bool.toString(), "true");

    Value v_null;
    EXPECT_EQ(v_null.toString(), "NULL");
}

// 测试类型转换边界情况
TEST_F(ValueTest, TypeConversionEdgeCases) {
    // 零值
    Value v_zero(0);
    EXPECT_EQ(v_zero.asInteger(), 0);

    // 负数
    Value v_neg(-42);
    EXPECT_EQ(v_neg.asInteger(), -42);

    // 大数
    Value v_large(2147483647);
    EXPECT_EQ(v_large.asInteger(), 2147483647);

    // 空字符串
    Value v_empty_str("");
    EXPECT_EQ(v_empty_str.asString(), "");
}

// ==================== DomainDefinition Tests ====================

class DomainDefinitionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试基本构造
TEST_F(DomainDefinitionTest, BasicConstructor) {
    DomainDefinition domain("AGE_DOMAIN", "INTEGER");
    EXPECT_EQ(domain.getName(), "AGE_DOMAIN");
    EXPECT_EQ(domain.getBaseType(), "INTEGER");
    EXPECT_TRUE(domain.isNullable());
    EXPECT_EQ(domain.getDefaultValue(), "");
    EXPECT_EQ(domain.getCheckConstraint(), "");
}

// 测试设置检查约束
TEST_F(DomainDefinitionTest, SetCheckConstraint) {
    DomainDefinition domain("AGE_DOMAIN", "INTEGER");
    domain.setCheckConstraint("value >= 0 AND value <= 150");
    EXPECT_EQ(domain.getCheckConstraint(), "value >= 0 AND value <= 150");
}

// 测试设置默认值
TEST_F(DomainDefinitionTest, SetDefaultValue) {
    DomainDefinition domain("STATUS_DOMAIN", "STRING");
    domain.setDefaultValue("'ACTIVE'");
    EXPECT_EQ(domain.getDefaultValue(), "'ACTIVE'");
}

// 测试设置可空性
TEST_F(DomainDefinitionTest, SetNullable) {
    DomainDefinition domain("NON_NULL_DOMAIN", "INTEGER");
    domain.setNullable(false);
    EXPECT_FALSE(domain.isNullable());

    domain.setNullable(true);
    EXPECT_TRUE(domain.isNullable());
}

// 测试值验证 - 有效值
TEST_F(DomainDefinitionTest, ValidateValue_Valid) {
    DomainDefinition domain("AGE_DOMAIN", "INTEGER");
    domain.setCheckConstraint("value >= 0 AND value <= 150");

    Value v1(25);
    EXPECT_TRUE(domain.validateValue(v1));

    Value v2(0);
    EXPECT_TRUE(domain.validateValue(v2));

    Value v3(150);
    EXPECT_TRUE(domain.validateValue(v3));
}

// 测试值验证 - 无效值
TEST_F(DomainDefinitionTest, ValidateValue_Invalid) {
    DomainDefinition domain("AGE_DOMAIN", "INTEGER");
    domain.setCheckConstraint("value >= 0 AND value <= 150");

    Value v1(-1);
    EXPECT_FALSE(domain.validateValue(v1));

    Value v2(151);
    EXPECT_FALSE(domain.validateValue(v2));
}

// 测试可空值验证
TEST_F(DomainDefinitionTest, ValidateValue_Nullable) {
    DomainDefinition domain("OPTIONAL_DOMAIN", "INTEGER");
    domain.setNullable(true);

    Value v_null;
    EXPECT_TRUE(domain.validateValue(v_null));

    Value v_int(42);
    EXPECT_TRUE(domain.validateValue(v_int));
}

// 测试非空值验证
TEST_F(DomainDefinitionTest, ValidateValue_NotNullable) {
    DomainDefinition domain("REQUIRED_DOMAIN", "INTEGER");
    domain.setNullable(false);

    Value v_null;
    EXPECT_FALSE(domain.validateValue(v_null));

    Value v_int(42);
    EXPECT_TRUE(domain.validateValue(v_int));
}

// ==================== DomainManager Tests ====================

class DomainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前获取管理器实例
        manager = &DomainManager::getInstance();
    }

    void TearDown() override {
        // 清理测试数据
        manager->dropDomain("TEST_DOMAIN1");
        manager->dropDomain("TEST_DOMAIN2");
        manager->dropDomain("AGE_DOMAIN");
    }

    DomainManager* manager;
};

// 测试单例模式
TEST_F(DomainManagerTest, SingletonPattern) {
    DomainManager& instance1 = DomainManager::getInstance();
    DomainManager& instance2 = DomainManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

// 测试创建域
TEST_F(DomainManagerTest, CreateDomain) {
    auto domain = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "INTEGER");
    domain->setCheckConstraint("value >= 0");
    EXPECT_TRUE(manager->createDomain(std::move(domain)));
    EXPECT_TRUE(manager->domainExists("TEST_DOMAIN1"));
}

// 测试创建重复域
TEST_F(DomainManagerTest, CreateDuplicateDomain) {
    auto domain1 = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "INTEGER");
    EXPECT_TRUE(manager->createDomain(std::move(domain1)));

    auto domain2 = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "STRING");
    EXPECT_FALSE(manager->createDomain(std::move(domain2)));
}

// 测试删除域
TEST_F(DomainManagerTest, DropDomain) {
    auto domain = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "INTEGER");
    manager->createDomain(std::move(domain));
    EXPECT_TRUE(manager->domainExists("TEST_DOMAIN1"));

    EXPECT_TRUE(manager->dropDomain("TEST_DOMAIN1"));
    EXPECT_FALSE(manager->domainExists("TEST_DOMAIN1"));
}

// 测试删除不存在的域
TEST_F(DomainManagerTest, DropNonExistentDomain) {
    EXPECT_FALSE(manager->dropDomain("NON_EXISTENT_DOMAIN"));
}

// 测试获取域定义
TEST_F(DomainManagerTest, GetDomain) {
    auto domain = std::make_unique<DomainDefinition>("AGE_DOMAIN", "INTEGER");
    domain->setCheckConstraint("value >= 0 AND value <= 150");
    domain->setDefaultValue("18");
    manager->createDomain(std::move(domain));

    auto retrieved = manager->getDomain("AGE_DOMAIN");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "AGE_DOMAIN");
    EXPECT_EQ(retrieved->getBaseType(), "INTEGER");
    EXPECT_EQ(retrieved->getCheckConstraint(), "value >= 0 AND value <= 150");
    EXPECT_EQ(retrieved->getDefaultValue(), "18");
}

// 测试获取不存在的域
TEST_F(DomainManagerTest, GetNonExistentDomain) {
    auto retrieved = manager->getDomain("NON_EXISTENT_DOMAIN");
    EXPECT_EQ(retrieved, nullptr);
}

// 测试检查域是否存在
TEST_F(DomainManagerTest, DomainExists) {
    auto domain = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "INTEGER");
    manager->createDomain(std::move(domain));

    EXPECT_TRUE(manager->domainExists("TEST_DOMAIN1"));
    EXPECT_FALSE(manager->domainExists("NON_EXISTENT_DOMAIN"));
}

// 测试验证域值
TEST_F(DomainManagerTest, ValidateDomainValue) {
    auto domain = std::make_unique<DomainDefinition>("AGE_DOMAIN", "INTEGER");
    domain->setCheckConstraint("value >= 0 AND value <= 150");
    manager->createDomain(std::move(domain));

    Value v1(25);
    EXPECT_TRUE(manager->validateDomainValue("AGE_DOMAIN", v1));

    Value v2(200);
    EXPECT_FALSE(manager->validateDomainValue("AGE_DOMAIN", v2));
}

// 测试验证不存在的域的值
TEST_F(DomainManagerTest, ValidateNonExistentDomainValue) {
    Value v(42);
    EXPECT_FALSE(manager->validateDomainValue("NON_EXISTENT_DOMAIN", v));
}

// 测试获取所有域名
TEST_F(DomainManagerTest, GetAllDomainNames) {
    auto domain1 = std::make_unique<DomainDefinition>("TEST_DOMAIN1", "INTEGER");
    auto domain2 = std::make_unique<DomainDefinition>("TEST_DOMAIN2", "STRING");
    manager->createDomain(std::move(domain1));
    manager->createDomain(std::move(domain2));

    auto names = manager->getAllDomainNames();
    EXPECT_EQ(names.size(), 2);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "TEST_DOMAIN1") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "TEST_DOMAIN2") != names.end());
}

// 测试获取域默认值
TEST_F(DomainManagerTest, GetDomainDefaultValue) {
    auto domain = std::make_unique<DomainDefinition>("AGE_DOMAIN", "INTEGER");
    domain->setDefaultValue("18");
    manager->createDomain(std::move(domain));

    EXPECT_EQ(manager->getDomainDefaultValue("AGE_DOMAIN"), "18");
    EXPECT_EQ(manager->getDomainDefaultValue("NON_EXISTENT_DOMAIN"), "");
}

// 测试检查域可空性
TEST_F(DomainManagerTest, IsDomainNullable) {
    auto domain1 = std::make_unique<DomainDefinition>("NULLABLE_DOMAIN", "INTEGER");
    domain1->setNullable(true);
    manager->createDomain(std::move(domain1));

    auto domain2 = std::make_unique<DomainDefinition>("NOT_NULL_DOMAIN", "INTEGER");
    domain2->setNullable(false);
    manager->createDomain(std::move(domain2));

    EXPECT_TRUE(manager->isDomainNullable("NULLABLE_DOMAIN"));
    EXPECT_FALSE(manager->isDomainNullable("NOT_NULL_DOMAIN"));
    EXPECT_FALSE(manager->isDomainNullable("NON_EXISTENT_DOMAIN"));
}

// 测试获取最后错误信息
TEST_F(DomainManagerTest, GetLastError) {
    // 尝试创建重复域
    auto domain1 = std::make_unique<DomainDefinition>("TEST_DOMAIN", "INTEGER");
    manager->createDomain(std::move(domain1));

    auto domain2 = std::make_unique<DomainDefinition>("TEST_DOMAIN", "STRING");
    bool result = manager->createDomain(std::move(domain2));

    EXPECT_FALSE(result);
    EXPECT_FALSE(manager->getLastError().empty());
}

// ==================== DomainDefinitionNode Tests ====================

class DomainDefinitionNodeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试基本构造
TEST_F(DomainDefinitionNodeTest, BasicConstructor) {
    DomainDefinitionNode node("PRICE_DOMAIN", "DECIMAL");
    EXPECT_EQ(node.getName(), "PRICE_DOMAIN");
    EXPECT_EQ(node.getBaseType(), "DECIMAL");
    EXPECT_TRUE(node.isNullable());
}

// 测试设置检查约束
TEST_F(DomainDefinitionNodeTest, SetCheckConstraint) {
    DomainDefinitionNode node("PRICE_DOMAIN", "DECIMAL");
    node.setCheckConstraint("value >= 0");
    EXPECT_EQ(node.getCheckConstraint(), "value >= 0");
}

// 测试设置默认值
TEST_F(DomainDefinitionNodeTest, SetDefaultValue) {
    DomainDefinitionNode node("STATUS_DOMAIN", "STRING");
    node.setDefaultValue("'ACTIVE'");
    EXPECT_EQ(node.getDefaultValue(), "'ACTIVE'");
}

// 测试设置可空性
TEST_F(DomainDefinitionNodeTest, SetNullable) {
    DomainDefinitionNode node("REQUIRED_DOMAIN", "INTEGER");
    node.setNullable(false);
    EXPECT_FALSE(node.isNullable());
}

// ==================== TransactionId Tests ====================

class TransactionIdTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试默认构造
TEST_F(TransactionIdTest, DefaultConstructor) {
    TransactionId tid;
    EXPECT_EQ(tid.get_id(), 0);
}

// 测试参数化构造
TEST_F(TransactionIdTest, ParameterizedConstructor) {
    auto timestamp = std::chrono::system_clock::now();
    TransactionId tid(12345, timestamp);
    EXPECT_EQ(tid.get_id(), 12345);
    EXPECT_EQ(tid.get_timestamp(), timestamp);
}

// 测试相等性比较
TEST_F(TransactionIdTest, EqualityOperators) {
    auto timestamp = std::chrono::system_clock::now();
    TransactionId tid1(100, timestamp);
    TransactionId tid2(100, timestamp);
    TransactionId tid3(200, timestamp);

    EXPECT_TRUE(tid1 == tid2);
    EXPECT_FALSE(tid1 == tid3);
    EXPECT_TRUE(tid1 != tid3);
    EXPECT_FALSE(tid1 != tid2);
}

// 测试顺序比较
TEST_F(TransactionIdTest, OrderingOperators) {
    auto timestamp1 = std::chrono::system_clock::now();
    auto timestamp2 = timestamp1 + std::chrono::seconds(1);

    TransactionId tid1(100, timestamp1);
    TransactionId tid2(100, timestamp2);

    EXPECT_TRUE(tid1 < tid2);
    EXPECT_TRUE(tid1 <= tid2);
    EXPECT_TRUE(tid2 > tid1);
    EXPECT_TRUE(tid2 >= tid1);
    EXPECT_FALSE(tid1 > tid2);
    EXPECT_FALSE(tid2 < tid1);
}

// ==================== LockType Tests ====================

class LockTypeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试LockType枚举值
TEST_F(LockTypeTest, EnumValues) {
    LockType shared = LockType::SHARED;
    LockType exclusive = LockType::EXCLUSIVE;

    EXPECT_TRUE(shared == LockType::SHARED);
    EXPECT_TRUE(exclusive == LockType::EXCLUSIVE);
    EXPECT_FALSE(shared == exclusive);
}

// ==================== LockMode Tests ====================

class LockModeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 测试LockMode枚举值
TEST_F(LockModeTest, EnumValues) {
    EXPECT_TRUE(LockMode::SHARED == LockMode::SHARED);
    EXPECT_TRUE(LockMode::EXCLUSIVE == LockMode::EXCLUSIVE);
    EXPECT_TRUE(LockMode::INTENTION_SHARED == LockMode::INTENTION_SHARED);
    EXPECT_TRUE(LockMode::INTENTION_EXCLUSIVE == LockMode::INTENTION_EXCLUSIVE);
    EXPECT_TRUE(LockMode::SHARED_INTENTION_EXCLUSIVE == LockMode::SHARED_INTENTION_EXCLUSIVE);
    EXPECT_TRUE(LockMode::UPDATE == LockMode::UPDATE);

    // 验证不同值不相等
    EXPECT_FALSE(LockMode::SHARED == LockMode::EXCLUSIVE);
    EXPECT_FALSE(LockMode::SHARED == LockMode::INTENTION_SHARED);
}

// ==================== Types集成测试 ====================

class TypesIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &DomainManager::getInstance();
    }

    void TearDown() override {
        // 清理测试数据
        manager->dropDomain("INTEGRATION_DOMAIN1");
        manager->dropDomain("INTEGRATION_DOMAIN2");
    }

    DomainManager* manager;
};

// 测试域管理的完整流程
TEST_F(TypesIntegrationTest, DomainManagementWorkflow) {
    // 1. 创建域
    auto domain = std::make_unique<DomainDefinition>("INTEGRATION_DOMAIN1", "INTEGER");
    domain->setCheckConstraint("value >= 0");
    domain->setDefaultValue("0");
    domain->setNullable(true);
    EXPECT_TRUE(manager->createDomain(std::move(domain)));

    // 2. 验证域存在
    EXPECT_TRUE(manager->domainExists("INTEGRATION_DOMAIN1"));

    // 3. 获取域定义
    auto retrieved = manager->getDomain("INTEGRATION_DOMAIN1");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "INTEGRATION_DOMAIN1");
    EXPECT_EQ(retrieved->getDefaultValue(), "0");

    // 4. 验证域值
    Value valid_value(42);
    EXPECT_TRUE(manager->validateDomainValue("INTEGRATION_DOMAIN1", valid_value));

    Value invalid_value(-1);
    EXPECT_FALSE(manager->validateDomainValue("INTEGRATION_DOMAIN1", invalid_value));

    // 5. 删除域
    EXPECT_TRUE(manager->dropDomain("INTEGRATION_DOMAIN1"));
    EXPECT_FALSE(manager->domainExists("INTEGRATION_DOMAIN1"));
}

// 测试多个域的管理
TEST_F(TypesIntegrationTest, MultipleDomainManagement) {
    auto domain1 = std::make_unique<DomainDefinition>("INTEGRATION_DOMAIN1", "INTEGER");
    domain1->setCheckConstraint("value >= 0");
    manager->createDomain(std::move(domain1));

    auto domain2 = std::make_unique<DomainDefinition>("INTEGRATION_DOMAIN2", "STRING");
    domain2->setCheckConstraint("length(value) > 0");
    manager->createDomain(std::move(domain2));

    auto names = manager->getAllDomainNames();
    EXPECT_EQ(names.size(), 2);

    // 验证不同类型的域
    Value int_val(100);
    Value str_val("Hello");

    EXPECT_TRUE(manager->validateDomainValue("INTEGRATION_DOMAIN1", int_val));
    EXPECT_FALSE(manager->validateDomainValue("INTEGRATION_DOMAIN2", int_val));

    EXPECT_TRUE(manager->validateDomainValue("INTEGRATION_DOMAIN2", str_val));
    EXPECT_FALSE(manager->validateDomainValue("INTEGRATION_DOMAIN1", str_val));
}

// 测试事务ID排序
TEST_F(TypesIntegrationTest, TransactionIdOrdering) {
    auto now = std::chrono::system_clock::now();
    auto later = now + std::chrono::seconds(1);
    auto even_later = now + std::chrono::seconds(2);

    TransactionId tid1(1, now);
    TransactionId tid2(2, later);
    TransactionId tid3(3, even_later);

    std::vector<TransactionId> transactions = {tid3, tid1, tid2};
    std::sort(transactions.begin(), transactions.end());

    EXPECT_EQ(transactions[0].get_id(), 1);
    EXPECT_EQ(transactions[1].get_id(), 2);
    EXPECT_EQ(transactions[2].get_id(), 3);
}

} // namespace test
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}