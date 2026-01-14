#include "sql_parser/data_types.h"
#include <gtest/gtest.h>
#include <string>
#include <chrono>

using namespace sqlcc::sql_parser;

class DataTypesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// 测试DecimalValue基本功能
TEST_F(DataTypesTest, DecimalValue_BasicOperations) {
    DecimalValue d1(static_cast<int64_t>(12345), 5, 2);  // 123.45
    DecimalValue d2("67.89");

    // 测试字符串表示
    EXPECT_EQ(d1.toString(), "123.45");
    EXPECT_EQ(d2.toString(), "67.89");

    // 测试加法
    DecimalValue sum = d1 + d2;
    EXPECT_EQ(sum.toString(), "191.34");

    // 测试减法
    DecimalValue diff = d1 - d2;
    EXPECT_EQ(diff.toString(), "55.56");

    // 测试比较
    EXPECT_TRUE(d1 > d2);
    EXPECT_TRUE(d2 < d1);
    EXPECT_TRUE(d1 != d2);
}

// 测试DateTimeValue基本功能
TEST_F(DataTypesTest, DateTimeValue_BasicOperations) {
    DateTimeValue dt1("2023-12-15 10:30:45");
    DateTimeValue dt2("2023-12-15 09:15:30");

    // 测试字符串表示
    EXPECT_EQ(dt1.toString(), "2023-12-15 10:30:45");

    // 测试比较
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt2 < dt1);

    // 测试日期组件提取
    EXPECT_EQ(dt1.getYear(), 2023);
    EXPECT_EQ(dt1.getMonth(), 12);
    EXPECT_EQ(dt1.getDay(), 15);
    EXPECT_EQ(dt1.getHour(), 10);
    EXPECT_EQ(dt1.getMinute(), 30);
    EXPECT_EQ(dt1.getSecond(), 45);
}

// 测试DateTimeValue静态方法
TEST_F(DataTypesTest, DateTimeValue_StaticMethods) {
    DateTimeValue now = DateTimeValue::now();
    DateTimeValue today = DateTimeValue::today();

    // now应该晚于today
    EXPECT_TRUE(now >= today);

    // today的时分秒应该都是0
    EXPECT_EQ(today.getHour(), 0);
    EXPECT_EQ(today.getMinute(), 0);
    EXPECT_EQ(today.getSecond(), 0);
}

// 测试DataValue基本功能
TEST_F(DataTypesTest, DataValue_BasicOperations) {
    // 逐个测试不同类型的DataValue
    {
        DataValue int_val(static_cast<int64_t>(42));
        EXPECT_EQ(int_val.getType(), DataType::BIGINT);
        EXPECT_EQ(int_val.asInt64(), 42);
    }

    {
        DataValue double_val(3.14);
        EXPECT_EQ(double_val.getType(), DataType::DOUBLE);
        EXPECT_DOUBLE_EQ(double_val.asDouble(), 3.14);
    }

    {
        DataValue string_val(std::string("hello"));
        std::cout << "String val type: " << static_cast<int>(string_val.getType()) << std::endl;
        std::cout << "String val content: " << string_val.asString() << std::endl;
        EXPECT_EQ(string_val.getType(), DataType::VARCHAR);
        EXPECT_EQ(string_val.asString(), "hello");
    }

    {
        DataValue bool_val(true);
        EXPECT_EQ(bool_val.getType(), DataType::BOOLEAN);
        EXPECT_TRUE(bool_val.asBool());
    }

    {
        DataValue decimal_val(DecimalValue("12.34"));
        EXPECT_EQ(decimal_val.getType(), DataType::DECIMAL);
        EXPECT_EQ(decimal_val.asDecimal().toString(), "12.34");
    }

    {
        DataValue datetime_val(DateTimeValue("2023-12-15"));
        EXPECT_EQ(datetime_val.getType(), DataType::TIMESTAMP);
    }
}

// 测试DataValue序列化
TEST_F(DataTypesTest, DataValue_Serialization) {
    DataValue original(DecimalValue("123.45"));
    std::string serialized = original.serialize();
    DataValue deserialized = DataValue::deserialize(serialized, DataType::DECIMAL);

    EXPECT_EQ(original.asDecimal(), deserialized.asDecimal());
}

// 测试DataTypeManager
TEST_F(DataTypesTest, DataTypeManager_BasicOperations) {
    DataTypeManager& mgr = DataTypeManager::getInstance();

    // 测试类型名称转换
    EXPECT_EQ(mgr.getTypeFromName("INT"), DataType::INTEGER);
    EXPECT_EQ(mgr.getTypeFromName("BIGINT"), DataType::BIGINT);
    EXPECT_EQ(mgr.getTypeFromName("DECIMAL"), DataType::DECIMAL);
    EXPECT_EQ(mgr.getTypeFromName("TIMESTAMP"), DataType::TIMESTAMP);

    // 测试类型信息
    const DataTypeInfo* int_info = mgr.getTypeInfo(DataType::INTEGER);
    ASSERT_NE(int_info, nullptr);
    EXPECT_EQ(int_info->name, "INT");
    EXPECT_TRUE(int_info->isNumeric);

    const DataTypeInfo* decimal_info = mgr.getTypeInfo(DataType::DECIMAL);
    ASSERT_NE(decimal_info, nullptr);
    EXPECT_EQ(decimal_info->name, "DECIMAL");
    EXPECT_TRUE(decimal_info->isNumeric);

    const DataTypeInfo* timestamp_info = mgr.getTypeInfo(DataType::TIMESTAMP);
    ASSERT_NE(timestamp_info, nullptr);
    EXPECT_EQ(timestamp_info->name, "TIMESTAMP");
    EXPECT_TRUE(timestamp_info->isTemporal);
}

// 测试数据类型转换
TEST_F(DataTypesTest, DataTypeManager_Conversions) {
    DataTypeManager& mgr = DataTypeManager::getInstance();

    // 测试可转换性
    EXPECT_TRUE(mgr.canConvert(DataType::BIGINT, DataType::DOUBLE));
    EXPECT_TRUE(mgr.canConvert(DataType::VARCHAR, DataType::BIGINT));
    EXPECT_TRUE(mgr.canConvert(DataType::TIMESTAMP, DataType::VARCHAR));

    // 测试实际转换
    DataValue int_val(static_cast<int64_t>(42));
    DataValue converted = mgr.convertValue(int_val, DataType::DOUBLE);
    EXPECT_DOUBLE_EQ(converted.asDouble(), 42.0);
}

// 测试DECIMAL类型解析
TEST_F(DataTypesTest, DataTypeManager_ParseDecimal) {
    DataTypeManager& mgr = DataTypeManager::getInstance();

    int precision, scale;
    bool result = mgr.parseDecimalType("DECIMAL(10,2)", precision, scale);
    EXPECT_TRUE(result);
    EXPECT_EQ(precision, 10);
    EXPECT_EQ(scale, 2);

    // 测试默认值
    result = mgr.parseDecimalType("DECIMAL(15)", precision, scale);
    EXPECT_TRUE(result);
    EXPECT_EQ(precision, 15);
    EXPECT_EQ(scale, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
