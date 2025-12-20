#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "utils/config_manager.h"
#include <memory>

namespace sqlcc {
namespace test {

// ConfigManager基础测试
class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // ConfigManager是单例模式，使用GetInstance获取实例
        config_manager_ = &ConfigManager::GetInstance();
    }

    void TearDown() override {
        // 单例模式不需要清理
    }

    ConfigManager* config_manager_;
};

// 测试SetValue和GetString
TEST_F(ConfigManagerTest, SetValueAndGetString) {
    // 设置字符串值
    EXPECT_TRUE(config_manager_->SetValue("test_key", std::string("test_value")));

    // 获取字符串值
    EXPECT_EQ(config_manager_->GetString("test_key"), "test_value");
    EXPECT_EQ(config_manager_->GetString("nonexistent_key", "default"), "default");
}

// 测试GetInt
TEST_F(ConfigManagerTest, GetInt) {
    // 设置整数值
    EXPECT_TRUE(config_manager_->SetValue("int_key", 42));

    // 获取整数值
    EXPECT_EQ(config_manager_->GetInt("int_key"), 42);
    EXPECT_EQ(config_manager_->GetInt("nonexistent_key", 100), 100);

    // 测试字符串转整数
    EXPECT_TRUE(config_manager_->SetValue("string_int_key", std::string("123")));
    EXPECT_EQ(config_manager_->GetInt("string_int_key"), 123);
}

// 测试GetBool
TEST_F(ConfigManagerTest, GetBool) {
    // 设置布尔值
    EXPECT_TRUE(config_manager_->SetValue("bool_key", true));

    // 获取布尔值
    EXPECT_TRUE(config_manager_->GetBool("bool_key"));
    EXPECT_FALSE(config_manager_->GetBool("nonexistent_key", false));

    // 测试字符串转布尔
    EXPECT_TRUE(config_manager_->SetValue("string_true_key", std::string("true")));
    EXPECT_TRUE(config_manager_->SetValue("string_false_key", std::string("false")));
    EXPECT_TRUE(config_manager_->SetValue("string_1_key", std::string("1")));
    EXPECT_TRUE(config_manager_->SetValue("string_0_key", std::string("0")));

    EXPECT_TRUE(config_manager_->GetBool("string_true_key"));
    EXPECT_FALSE(config_manager_->GetBool("string_false_key"));
    EXPECT_TRUE(config_manager_->GetBool("string_1_key"));
    EXPECT_FALSE(config_manager_->GetBool("string_0_key"));
}

// 测试GetDouble
TEST_F(ConfigManagerTest, GetDouble) {
    // 设置浮点数值
    EXPECT_TRUE(config_manager_->SetValue("double_key", 3.14159));

    // 获取浮点数值
    EXPECT_DOUBLE_EQ(config_manager_->GetDouble("double_key"), 3.14159);
    EXPECT_DOUBLE_EQ(config_manager_->GetDouble("nonexistent_key", 2.71828), 2.71828);

    // 测试字符串转浮点数
    EXPECT_TRUE(config_manager_->SetValue("string_double_key", std::string("2.71828")));
    EXPECT_DOUBLE_EQ(config_manager_->GetDouble("string_double_key"), 2.71828);
}

// 测试HasKey
TEST_F(ConfigManagerTest, HasKey) {
    // 测试不存在的键
    EXPECT_FALSE(config_manager_->HasKey("nonexistent_key"));

    // 设置值后测试存在
    EXPECT_TRUE(config_manager_->SetValue("existing_key", std::string("value")));
    EXPECT_TRUE(config_manager_->HasKey("existing_key"));
}

// 测试类型覆盖
TEST_F(ConfigManagerTest, TypeCoverage) {
    // 测试各种类型的值设置和获取
    EXPECT_TRUE(config_manager_->SetValue("int_val", 42));
    EXPECT_TRUE(config_manager_->SetValue("double_val", 3.14));
    EXPECT_TRUE(config_manager_->SetValue("bool_val", true));
    EXPECT_TRUE(config_manager_->SetValue("string_val", std::string("hello")));

    EXPECT_EQ(config_manager_->GetInt("int_val"), 42);
    EXPECT_DOUBLE_EQ(config_manager_->GetDouble("double_val"), 3.14);
    EXPECT_TRUE(config_manager_->GetBool("bool_val"));
    EXPECT_EQ(config_manager_->GetString("string_val"), "hello");
}

// 测试默认值行为
TEST_F(ConfigManagerTest, DefaultValues) {
    // 测试各种类型的默认值
    EXPECT_EQ(config_manager_->GetString("missing_string", "default_string"), "default_string");
    EXPECT_EQ(config_manager_->GetInt("missing_int", 42), 42);
    EXPECT_DOUBLE_EQ(config_manager_->GetDouble("missing_double", 3.14), 3.14);
    EXPECT_TRUE(config_manager_->GetBool("missing_bool", true));
    EXPECT_FALSE(config_manager_->GetBool("missing_bool", false));
}

// 测试错误处理
TEST_F(ConfigManagerTest, ErrorHandling) {
    // 测试类型不匹配的情况
    EXPECT_TRUE(config_manager_->SetValue("int_as_string", 123));

    // 尝试以错误类型获取值
    EXPECT_EQ(config_manager_->GetString("int_as_string"), "123");  // 应该能转换
    EXPECT_EQ(config_manager_->GetInt("int_as_string"), 123);       // 应该能转换

    // 测试无效的字符串转数字
    EXPECT_TRUE(config_manager_->SetValue("invalid_int", std::string("not_a_number")));
    EXPECT_EQ(config_manager_->GetInt("invalid_int", 999), 999);  // 应该返回默认值
}

} // namespace test
} // namespace sqlcc
