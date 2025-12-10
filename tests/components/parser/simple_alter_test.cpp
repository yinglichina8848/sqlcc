#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "sql_parser/parser_new.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

class SimpleAlterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// 测试ALTER TABLE ADD COLUMN语句解析
TEST_F(SimpleAlterTest, ParseAlterTableAddColumn) {
    std::string sql = "ALTER TABLE users ADD COLUMN age INT;";
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
TEST_F(SimpleAlterTest, ParseAlterTableDropColumn) {
    std::string sql = "ALTER TABLE users DROP COLUMN age;";
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}