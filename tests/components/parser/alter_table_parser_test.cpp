#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <iostream>

#include "sql_parser/parser_new.h"
#include "sql_parser/lexer.h"
#include "../../../include/sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

class AlterTableParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// 测试ALTER TABLE ADD COLUMN语句解析
TEST_F(AlterTableParserTest, ParseAlterTableAddColumn) {
    std::string sql = "ALTER TABLE users ADD COLUMN age INT";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    
    auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
    ASSERT_NE(alter_stmt, nullptr);
    
    EXPECT_EQ(alter_stmt->getType(), Statement::ALTER);
    EXPECT_EQ(alter_stmt->getTarget(), AlterStatement::TABLE);
    EXPECT_EQ(alter_stmt->getAction(), AlterStatement::ADD_COLUMN);
    EXPECT_EQ(alter_stmt->getTableName(), "users");
    
    auto column_def = alter_stmt->getColumnDefinition();
    EXPECT_EQ(column_def.getName(), "age");
    EXPECT_EQ(column_def.getType(), "INT");
}

// 测试ALTER TABLE DROP COLUMN语句解析
TEST_F(AlterTableParserTest, ParseAlterTableDropColumn) {
    std::string sql = "ALTER TABLE users DROP COLUMN age";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    
    auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
    ASSERT_NE(alter_stmt, nullptr);
    
    EXPECT_EQ(alter_stmt->getType(), Statement::ALTER);
    EXPECT_EQ(alter_stmt->getTarget(), AlterStatement::TABLE);
    EXPECT_EQ(alter_stmt->getAction(), AlterStatement::DROP_COLUMN);
    EXPECT_EQ(alter_stmt->getTableName(), "users");
    EXPECT_EQ(alter_stmt->getColumnName(), "age");
}

// 测试ALTER TABLE MODIFY COLUMN语句解析
TEST_F(AlterTableParserTest, ParseAlterTableModifyColumn) {
    std::string sql = "ALTER TABLE users MODIFY COLUMN age BIGINT";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    
    auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
    ASSERT_NE(alter_stmt, nullptr);
    
    EXPECT_EQ(alter_stmt->getType(), Statement::ALTER);
    EXPECT_EQ(alter_stmt->getTarget(), AlterStatement::TABLE);
    EXPECT_EQ(alter_stmt->getAction(), AlterStatement::MODIFY_COLUMN);
    EXPECT_EQ(alter_stmt->getTableName(), "users");
    
    auto column_def = alter_stmt->getColumnDefinition();
    EXPECT_EQ(column_def.getName(), "age");
    EXPECT_EQ(column_def.getType(), "BIGINT");
}

// 测试ALTER TABLE RENAME TO语句解析
TEST_F(AlterTableParserTest, ParseAlterTableRenameTable) {
    std::string sql = "ALTER TABLE users RENAME TO customers";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    
    auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
    ASSERT_NE(alter_stmt, nullptr);
    
    EXPECT_EQ(alter_stmt->getType(), Statement::ALTER);
    EXPECT_EQ(alter_stmt->getTarget(), AlterStatement::TABLE);
    EXPECT_EQ(alter_stmt->getAction(), AlterStatement::RENAME_TABLE);
    EXPECT_EQ(alter_stmt->getTableName(), "users");
    EXPECT_EQ(alter_stmt->getNewTableName(), "customers");
}

// 测试带有复杂列定义的ALTER TABLE ADD COLUMN语句解析
TEST_F(AlterTableParserTest, ParseAlterTableAddColumnWithComplexDefinition) {
    std::string sql = "ALTER TABLE products ADD COLUMN description VARCHAR(255) NOT NULL DEFAULT 'N/A'";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    
    auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
    ASSERT_NE(alter_stmt, nullptr);
    
    EXPECT_EQ(alter_stmt->getType(), Statement::ALTER);
    EXPECT_EQ(alter_stmt->getTarget(), AlterStatement::TABLE);
    EXPECT_EQ(alter_stmt->getAction(), AlterStatement::ADD_COLUMN);
    EXPECT_EQ(alter_stmt->getTableName(), "products");
    
    auto column_def = alter_stmt->getColumnDefinition();
    EXPECT_EQ(column_def.getName(), "description");
    EXPECT_EQ(column_def.getType(), "VARCHAR(255)");
    // 注意：当前解析器可能不会完整解析NOT NULL和DEFAULT子句，这部分可以在后续完善
}

// 测试错误情况：无效的ALTER操作
TEST_F(AlterTableParserTest, ParseInvalidAlterOperation) {
    std::string sql = "ALTER TABLE users INVALID_ACTION column_name";
    ParserNew parser(sql);
    
    // 期望解析失败
    auto statements = parser.parse();
    EXPECT_TRUE(statements.empty());
}

// 测试错误情况：缺少必要的关键字
TEST_F(AlterTableParserTest, ParseIncompleteAlterStatement) {
    std::string sql = "ALTER TABLE users ADD";
    ParserNew parser(sql);
    
    // 期望解析失败
    auto statements = parser.parse();
    EXPECT_TRUE(statements.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}