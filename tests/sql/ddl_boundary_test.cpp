// SQLCC v1.3.2 DDL边界条件测试
// 按照SQL-92测试补全验证计划，Week 3: DDL边界条件测试补全 (15个测试用例)
// 测试目标: 验证DDL语句在各种边界条件下的正确处理
// 使用SQL解析器进行语法验证

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "sql_parser/parser.h"

// DDL边界条件测试类
class DDLBoundaryTest : public ::testing::Test {
protected:
    // 解析SQL语句
    bool parseSQL(const std::string& sql) {
        try {
            // 创建新的解析器实例
            sqlcc::sql_parser::Parser parser(sql);
            auto ast = parser.parse();
            return !ast.empty();
        } catch (const std::exception& e) {
            last_exception_ = e.what();
            return false;
        }
    }

    // 执行SQL语句 (简化为解析测试)
    void executeSQL(const std::string& sql) {
        parseSQL(sql);
    }

    std::string getLastException() const {
        return last_exception_;
    }

    bool hasException() const {
        return !last_exception_.empty();
    }

    void clearException() {
        last_exception_.clear();
    }

private:
    std::string last_exception_;
};

// Test Case 1: 表名最大长度边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_MaxTableNameLength) {
    // 测试表名最大长度 (假设最大64字符)
    std::string long_table_name(64, 'a'); // 64个'a'字符
    std::string create_sql = "CREATE TABLE " + long_table_name + " (id INT PRIMARY KEY);";

    executeSQL(create_sql);
    ASSERT_FALSE(hasException()) << "创建最大长度表名失败: " << getLastException();

    // 验证表创建成功
    executeSQL("SHOW TABLES;");
    // 这里应该包含新创建的表

    // 清理
    executeSQL("DROP TABLE " + long_table_name + ";");
}

// Test Case 2: 表名超长边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_TableNameTooLong) {
    // 测试表名超长 (假设超过65字符)
    std::string too_long_table_name(66, 'b'); // 66个'b'字符
    std::string create_sql = "CREATE TABLE " + too_long_table_name + " (id INT PRIMARY KEY);";

    executeSQL(create_sql);
    ASSERT_TRUE(hasException()) << "超长表名应该被拒绝";
    clearException();
}

// Test Case 3: 列名最大长度边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_MaxColumnNameLength) {
    // 测试列名最大长度 (假设最大32字符)
    std::string long_column_name(32, 'c'); // 32个'c'字符
    std::string create_sql = "CREATE TABLE test_table (" + long_column_name + " INT PRIMARY KEY);";

    executeSQL(create_sql);
    ASSERT_FALSE(hasException()) << "创建最大长度列名失败: " << getLastException();

    // 清理
    executeSQL("DROP TABLE test_table;");
}

// Test Case 4: 特殊字符表名测试
TEST_F(DDLBoundaryTest, DDL_Boundary_SpecialCharactersInTableName) {
    // 测试包含特殊字符的表名
    std::vector<std::string> special_names = {
        "test_table_123",  // 数字
        "test_table_abc",  // 字母
        "`test_table`",    // 反引号包围
        "test_underscore", // 下划线
    };

    for (const auto& table_name : special_names) {
        std::string create_sql = "CREATE TABLE " + table_name + " (id INT PRIMARY KEY);";
        executeSQL(create_sql);
        ASSERT_FALSE(hasException()) << "创建表名 '" << table_name << "' 失败: " << getLastException();

        // 清理
        executeSQL("DROP TABLE " + table_name + ";");
    }
}

// Test Case 5: 无效字符表名测试
TEST_F(DDLBoundaryTest, DDL_Boundary_InvalidCharactersInTableName) {
    // 测试包含无效字符的表名
    std::vector<std::string> invalid_names = {
        "test-table",      // 连字符
        "test table",      // 空格
        "test@table",      // @符号
        "test#table",      // #符号
    };

    for (const auto& table_name : invalid_names) {
        std::string create_sql = "CREATE TABLE " + table_name + " (id INT PRIMARY KEY);";
        executeSQL(create_sql);
        // 这里可能成功也可能失败，取决于解析器的实现
        // 主要测试解析器能正确处理这些情况
        clearException();
    }
}

// Test Case 6: 最大列数边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_MaxColumns) {
    // 测试表的最大列数 (假设最大100列)
    std::string create_sql = "CREATE TABLE test_max_columns (";
    for (int i = 1; i <= 100; ++i) {
        create_sql += "col" + std::to_string(i) + " INT";
        if (i < 100) create_sql += ", ";
    }
    create_sql += ");";

    executeSQL(create_sql);
    ASSERT_FALSE(hasException()) << "创建100列表失败: " << getLastException();

    // 清理
    executeSQL("DROP TABLE test_max_columns;");
}

// Test Case 7: 约束组合边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_ConstraintCombination) {
    // 测试多个约束的组合
    std::string create_sql = R"(
        CREATE TABLE test_constraints (
            id INT PRIMARY KEY,
            name VARCHAR(50) NOT NULL,
            email VARCHAR(100) UNIQUE,
            age INT CHECK (age >= 0 AND age <= 150),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

    executeSQL(create_sql);
    ASSERT_FALSE(hasException()) << "创建多约束表失败: " << getLastException();

    // 清理
    executeSQL("DROP TABLE test_constraints;");
}

// Test Case 8: 数据类型边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_DataTypeBoundaries) {
    // 测试各种数据类型的边界值定义
    std::vector<std::string> data_type_tests = {
        "CREATE TABLE test_types (tiny_int TINYINT, small_int SMALLINT, normal_int INT, big_int BIGINT);",
        "CREATE TABLE test_strings (short_str VARCHAR(1), normal_str VARCHAR(255), long_str TEXT);",
        "CREATE TABLE test_decimals (small_dec DECIMAL(5,2), normal_dec DECIMAL(10,4), big_dec DECIMAL(20,8));",
        "CREATE TABLE test_times (date_col DATE, time_col TIME, datetime_col DATETIME, timestamp_col TIMESTAMP);",
        "CREATE TABLE test_booleans (bool_col BOOLEAN, true_col BOOLEAN DEFAULT TRUE, false_col BOOLEAN DEFAULT FALSE);"
    };

    for (const auto& sql : data_type_tests) {
        executeSQL(sql);
        ASSERT_FALSE(hasException()) << "数据类型测试失败: " << sql << " - " << getLastException();

        // 清理表 (从SQL中提取表名)
        size_t table_start = sql.find("test_");
        size_t table_end = sql.find(" ", table_start);
        std::string table_name = sql.substr(table_start, table_end - table_start);
        executeSQL("DROP TABLE " + table_name + ";");
    }
}

// Test Case 9: 索引名称边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_IndexNameBoundaries) {
    // 创建测试表
    executeSQL("CREATE TABLE test_index (id INT, name VARCHAR(50), email VARCHAR(100));");

    // 测试索引名称的最大长度
    std::string long_index_name(64, 'i'); // 64个'i'字符
    std::string create_index_sql = "CREATE INDEX " + long_index_name + " ON test_index (name);";

    executeSQL(create_index_sql);
    ASSERT_FALSE(hasException()) << "创建最大长度索引名失败: " << getLastException();

    // 清理
    executeSQL("DROP TABLE test_index;");
}

// Test Case 10: 外键约束边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_ForeignKeyBoundaries) {
    // 创建父表
    executeSQL("CREATE TABLE parent_table (id INT PRIMARY KEY, name VARCHAR(50));");

    // 创建包含外键的子表
    std::string create_child_sql = R"(
        CREATE TABLE child_table (
            id INT PRIMARY KEY,
            parent_id INT,
            name VARCHAR(50),
            FOREIGN KEY (parent_id) REFERENCES parent_table(id)
        );
    )";

    executeSQL(create_child_sql);
    ASSERT_FALSE(hasException()) << "创建外键表失败: " << getLastException();

    // 清理
    executeSQL("DROP TABLE child_table;");
    executeSQL("DROP TABLE parent_table;");
}

// Test Case 11: 默认值边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_DefaultValueBoundaries) {
    // 测试各种默认值的边界情况
    std::vector<std::string> default_tests = {
        "CREATE TABLE test_defaults1 (id INT DEFAULT 0, name VARCHAR(50) DEFAULT 'default_name');",
        "CREATE TABLE test_defaults2 (active BOOLEAN DEFAULT TRUE, count INT DEFAULT 100);",
        "CREATE TABLE test_defaults3 (created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);",
        "CREATE TABLE test_defaults4 (score DECIMAL(5,2) DEFAULT 0.00);"
    };

    for (const auto& sql : default_tests) {
        executeSQL(sql);
        ASSERT_FALSE(hasException()) << "默认值测试失败: " << sql << " - " << getLastException();

        // 清理表
        size_t table_start = sql.find("test_defaults");
        size_t table_end = sql.find(" ", table_start);
        std::string table_name = sql.substr(table_start, table_end - table_start);
        executeSQL("DROP TABLE " + table_name + ";");
    }
}

// Test Case 12: ALTER TABLE边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_AlterTableBoundaries) {
    // 创建基础表
    executeSQL("CREATE TABLE test_alter (id INT PRIMARY KEY, name VARCHAR(50));");

    // 测试ALTER TABLE的各种边界情况
    std::vector<std::string> alter_tests = {
        "ALTER TABLE test_alter ADD COLUMN email VARCHAR(100);",
        "ALTER TABLE test_alter ADD COLUMN age INT DEFAULT 18;",
        "ALTER TABLE test_alter MODIFY COLUMN name VARCHAR(100);",
        "ALTER TABLE test_alter ADD CONSTRAINT ck_age CHECK (age >= 0);",
        "ALTER TABLE test_alter DROP COLUMN email;"
    };

    for (const auto& sql : alter_tests) {
        executeSQL(sql);
        ASSERT_FALSE(hasException()) << "ALTER TABLE测试失败: " << sql << " - " << getLastException();
    }

    // 清理
    executeSQL("DROP TABLE test_alter;");
}

// Test Case 13: 视图名称边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_ViewNameBoundaries) {
    // 创建基础表
    executeSQL("CREATE TABLE base_table (id INT, name VARCHAR(50));");

    // 测试视图名称的最大长度
    std::string long_view_name(64, 'v'); // 64个'v'字符
    std::string create_view_sql = "CREATE VIEW " + long_view_name + " AS SELECT * FROM base_table;";

    executeSQL(create_view_sql);
    ASSERT_FALSE(hasException()) << "创建最大长度视图名失败: " << getLastException();

    // 清理
    executeSQL("DROP VIEW " + long_view_name + ";");
    executeSQL("DROP TABLE base_table;");
}

// Test Case 14: 数据库名称边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_DatabaseNameBoundaries) {
    // 测试数据库名称的最大长度
    std::string long_db_name(32, 'd'); // 32个'd'字符
    std::string create_db_sql = "CREATE DATABASE " + long_db_name + ";";

    executeSQL(create_db_sql);
    ASSERT_FALSE(hasException()) << "创建最大长度数据库名失败: " << getLastException();

    // 切换到新数据库并创建表
    executeSQL("USE " + long_db_name + ";");
    executeSQL("CREATE TABLE test_table (id INT);");

    // 清理
    executeSQL("DROP TABLE test_table;");
    executeSQL("USE test_db;");
    executeSQL("DROP DATABASE " + long_db_name + ";");
}

// Test Case 15: 复合约束边界测试
TEST_F(DDLBoundaryTest, DDL_Boundary_CompositeConstraints) {
    // 测试复合约束的边界情况
    std::string create_sql = R"(
        CREATE TABLE test_composite (
            id INT,
            name VARCHAR(50),
            email VARCHAR(100),
            age INT,
            PRIMARY KEY (id),
            UNIQUE (email),
            CHECK (age >= 18 AND age <= 100),
            CHECK (name IS NOT NULL AND LENGTH(name) > 0),
            UNIQUE (name, email)
        );
    )";

    executeSQL(create_sql);
    ASSERT_FALSE(hasException()) << "创建复合约束表失败: " << getLastException();

    // 测试违反约束的情况
    executeSQL("INSERT INTO test_composite VALUES (1, '', 'test@example.com', 25);");
    // 这里可能成功或失败，取决于约束检查的严格程度

    // 清理
    executeSQL("DROP TABLE test_composite;");
}
