#include "sql_executor.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace sqlcc;

class RecordTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

// 测试默认构造函数
TEST_F(RecordTest, TestDefaultConstructor) {
    Record record;
    EXPECT_EQ(record.column_values.size(), 0);
    EXPECT_EQ(record.record_id, 0);
    EXPECT_EQ(record.txn_id, 0);
    EXPECT_EQ(record.table_name, "");
}

// 测试带字段值向量的构造函数
TEST_F(RecordTest, TestVectorConstructor) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record record(fields);
    
    EXPECT_EQ(record.column_values.size(), 3);
    EXPECT_EQ(record.column_values[0], "1");
    EXPECT_EQ(record.column_values[1], "John");
    EXPECT_EQ(record.column_values[2], "john@example.com");
    EXPECT_EQ(record.record_id, 0);
    EXPECT_EQ(record.txn_id, 0);
    EXPECT_EQ(record.table_name, "");
    
    // 测试带记录ID的构造函数
    Record record_with_id(fields, 123);
    EXPECT_EQ(record_with_id.record_id, 123);
}

// 测试GetFieldCount方法
TEST_F(RecordTest, TestGetFieldCount) {
    Record empty_record;
    EXPECT_EQ(empty_record.column_values.size(), empty_record.GetFieldCount());
    EXPECT_EQ(empty_record.GetFieldCount(), 0);
    
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record record(fields);
    EXPECT_EQ(record.GetFieldCount(), 3);
}

// 测试GetField方法
TEST_F(RecordTest, TestGetField) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record record(fields);
    
    EXPECT_EQ(record.GetField(0), "1");
    EXPECT_EQ(record.GetField(1), "John");
    EXPECT_EQ(record.GetField(2), "john@example.com");
}

// 测试SetField方法
TEST_F(RecordTest, TestSetField) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record record(fields);
    
    record.SetField(1, "Jane");
    EXPECT_EQ(record.GetField(1), "Jane");
    
    record.SetField(0, "2");
    EXPECT_EQ(record.GetField(0), "2");
}

// 测试AddField方法
TEST_F(RecordTest, TestAddField) {
    Record record;
    EXPECT_EQ(record.GetFieldCount(), 0);
    
    record.AddField("First");
    EXPECT_EQ(record.GetFieldCount(), 1);
    EXPECT_EQ(record.GetField(0), "First");
    
    record.AddField("Second");
    EXPECT_EQ(record.GetFieldCount(), 2);
    EXPECT_EQ(record.GetField(1), "Second");
}

// 测试==操作符
TEST_F(RecordTest, TestEqualityOperator) {
    std::vector<std::string> fields1 = {"1", "John", "john@example.com"};
    std::vector<std::string> fields2 = {"1", "John", "john@example.com"};
    std::vector<std::string> fields3 = {"2", "Jane", "jane@example.com"};
    
    Record record1(fields1);
    Record record2(fields2);
    Record record3(fields3);
    
    EXPECT_TRUE(record1 == record2);
    EXPECT_FALSE(record1 == record3);
    
    // 自等测试
    EXPECT_TRUE(record1 == record1);
}

// 测试!=操作符
TEST_F(RecordTest, TestInequalityOperator) {
    std::vector<std::string> fields1 = {"1", "John", "john@example.com"};
    std::vector<std::string> fields2 = {"1", "John", "john@example.com"};
    std::vector<std::string> fields3 = {"2", "Jane", "jane@example.com"};
    
    Record record1(fields1);
    Record record2(fields2);
    Record record3(fields3);
    
    EXPECT_FALSE(record1 != record2);
    EXPECT_TRUE(record1 != record3);
    
    // 自等测试
    EXPECT_FALSE(record1 != record1);
}

// 测试拷贝构造函数
TEST_F(RecordTest, TestCopyConstructor) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record original_record(fields);
    original_record.record_id = 100;
    original_record.txn_id = 200;
    original_record.table_name = "users";
    
    Record copied_record(original_record);
    
    EXPECT_EQ(copied_record.GetFieldCount(), original_record.GetFieldCount());
    EXPECT_EQ(copied_record.GetField(0), original_record.GetField(0));
    EXPECT_EQ(copied_record.GetField(1), original_record.GetField(1));
    EXPECT_EQ(copied_record.GetField(2), original_record.GetField(2));
    EXPECT_EQ(copied_record.record_id, original_record.record_id);
    EXPECT_EQ(copied_record.txn_id, original_record.txn_id);
    EXPECT_EQ(copied_record.table_name, original_record.table_name);
    
    // 验证深拷贝 - 修改原对象不应影响拷贝对象
    original_record.SetField(1, "Jane");
    EXPECT_EQ(original_record.GetField(1), "Jane");
    EXPECT_EQ(copied_record.GetField(1), "John"); // 拷贝对象应该不变
}

// 测试赋值操作符
TEST_F(RecordTest, TestAssignmentOperator) {
    std::vector<std::string> fields1 = {"1", "John", "john@example.com"};
    std::vector<std::string> fields2 = {"2", "Jane", "jane@example.com"};
    
    Record record1(fields1);
    Record record2(fields2);
    
    record1 = record2;
    
    EXPECT_EQ(record1.GetFieldCount(), record2.GetFieldCount());
    EXPECT_EQ(record1.GetField(0), record2.GetField(0));
    EXPECT_EQ(record1.GetField(1), record2.GetField(1));
    EXPECT_EQ(record1.GetField(2), record2.GetField(2));
    
    // 自赋值测试
    record1 = record1;
    EXPECT_EQ(record1.GetField(0), "2");
}

// 测试移动构造函数
TEST_F(RecordTest, TestMoveConstructor) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record original_record(fields);
    original_record.record_id = 100;
    original_record.txn_id = 200;
    original_record.table_name = "users";
    
    Record moved_record(std::move(original_record));
    
    EXPECT_EQ(moved_record.GetFieldCount(), 3);
    EXPECT_EQ(moved_record.GetField(0), "1");
    EXPECT_EQ(moved_record.GetField(1), "John");
    EXPECT_EQ(moved_record.GetField(2), "john@example.com");
    EXPECT_EQ(moved_record.record_id, 100);
    EXPECT_EQ(moved_record.txn_id, 200);
    EXPECT_EQ(moved_record.table_name, "users");
    
    // 原对象应处于有效但未指定状态
    EXPECT_EQ(original_record.column_values.size(), 0);
}

// 测试移动赋值操作符
TEST_F(RecordTest, TestMoveAssignmentOperator) {
    std::vector<std::string> fields = {"1", "John", "john@example.com"};
    Record original_record(fields);
    Record record;
    
    record = std::move(original_record);
    
    EXPECT_EQ(record.GetFieldCount(), 3);
    EXPECT_EQ(record.GetField(0), "1");
    EXPECT_EQ(record.GetField(1), "John");
    EXPECT_EQ(record.GetField(2), "john@example.com");
    
    // 原对象应处于有效但未指定状态
    EXPECT_EQ(original_record.column_values.size(), 0);
}