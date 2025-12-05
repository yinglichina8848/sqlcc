#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {
namespace sql_parser {

// 测试辅助结构
class ASTNodesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试初始化
    }

    void TearDown() override {
        // 测试清理
    }
};

// 测试TableReference
TEST_F(ASTNodesTest, TableReferenceTest) {
    TableReference table("users");

    EXPECT_EQ(table.getName(), "users");
    EXPECT_EQ(table.getAlias(), "");
    EXPECT_FALSE(table.hasAlias());

    table.setAlias("u");
    EXPECT_EQ(table.getAlias(), "u");
    EXPECT_TRUE(table.hasAlias());
}

// 测试ColumnDefinition
TEST_F(ASTNodesTest, ColumnDefinitionTest) {
    ColumnDefinition col("id", "INTEGER");

    EXPECT_EQ(col.getName(), "id");
    EXPECT_EQ(col.getType(), "INTEGER");
    EXPECT_TRUE(col.isNullable());
    EXPECT_FALSE(col.isPrimaryKey());
    EXPECT_FALSE(col.isUnique());
    EXPECT_FALSE(col.isForeignKey());

    col.setPrimaryKey(true);
    col.setNullable(false);
    col.setUnique(true);

    EXPECT_TRUE(col.isPrimaryKey());
    EXPECT_FALSE(col.isNullable());
    EXPECT_TRUE(col.isUnique());
}

// 测试SelectItem
TEST_F(ASTNodesTest, SelectItemTest) {
    // 使用简化的测试，避免抽象类问题
    SelectItem item(nullptr);  // 使用nullptr作为占位符

    EXPECT_FALSE(item.hasAlias());

    item.setAlias("user_name");
    EXPECT_TRUE(item.hasAlias());
    EXPECT_EQ(item.getAlias(), "user_name");
}

// 测试CreateStatement
TEST_F(ASTNodesTest, CreateStatementTest) {
    CreateStatement create_stmt(CreateStatement::TABLE);
    create_stmt.setTableName("users");

    ColumnDefinition col1("id", "INTEGER");
    col1.setPrimaryKey(true);
    ColumnDefinition col2("name", "VARCHAR");

    create_stmt.addColumn(std::move(col1));
    create_stmt.addColumn(std::move(col2));

    EXPECT_EQ(create_stmt.getTarget(), CreateStatement::TABLE);
    EXPECT_EQ(create_stmt.getTableName(), "users");
    EXPECT_EQ(create_stmt.getColumns().size(), 2);
    EXPECT_EQ(create_stmt.getColumns()[0].getName(), "id");
    EXPECT_EQ(create_stmt.getColumns()[1].getName(), "name");
}

// 测试SelectStatement
TEST_F(ASTNodesTest, SelectStatementTest) {
    SelectStatement select_stmt;

    // 添加SELECT项 - 使用简化的方式
    SelectItem item1(nullptr);
    SelectItem item2(nullptr);

    select_stmt.addSelectItem(std::move(item1));
    select_stmt.addSelectItem(std::move(item2));

    // 添加FROM表
    TableReference table("users");
    select_stmt.addFromTable(table);

    EXPECT_EQ(select_stmt.getSelectItems().size(), 2);
    EXPECT_EQ(select_stmt.getFromTables().size(), 1);
    EXPECT_EQ(select_stmt.getFromTables()[0].getName(), "users");
    EXPECT_FALSE(select_stmt.isDistinct());
    EXPECT_EQ(select_stmt.getLimit(), -1);
}

// 测试InsertStatement
TEST_F(ASTNodesTest, InsertStatementTest) {
    InsertStatement insert_stmt;
    insert_stmt.setTableName("users");

    insert_stmt.addColumn("id");
    insert_stmt.addColumn("name");

    EXPECT_EQ(insert_stmt.getTableName(), "users");
    EXPECT_EQ(insert_stmt.getColumns().size(), 2);
}

// 测试UpdateStatement
TEST_F(ASTNodesTest, UpdateStatementTest) {
    UpdateStatement update_stmt;
    update_stmt.setTableName("users");

    // 简化测试，避免复杂的表达式构造
    EXPECT_EQ(update_stmt.getTableName(), "users");
    EXPECT_EQ(update_stmt.getSetItems().size(), 0);  // 初始为空
    EXPECT_TRUE(update_stmt.getWhereClause() == nullptr);
}

// 测试DeleteStatement
TEST_F(ASTNodesTest, DeleteStatementTest) {
    DeleteStatement delete_stmt;
    delete_stmt.setTableName("users");

    EXPECT_EQ(delete_stmt.getTableName(), "users");
    EXPECT_TRUE(delete_stmt.getWhereClause() == nullptr);
}

// 测试DropStatement
TEST_F(ASTNodesTest, DropStatementTest) {
    DropStatement drop_stmt(DropStatement::TABLE);
    drop_stmt.setTableName("users");
    drop_stmt.setIfExists(true);

    EXPECT_EQ(drop_stmt.getTarget(), DropStatement::TABLE);
    EXPECT_EQ(drop_stmt.getTableName(), "users");
    EXPECT_TRUE(drop_stmt.isIfExists());
}

// 测试表约束
TEST_F(ASTNodesTest, TableConstraintsTest) {
    // 主键约束
    PrimaryKeyConstraint pk_constraint;
    pk_constraint.addColumn("id");
    pk_constraint.addColumn("name");

    EXPECT_EQ(pk_constraint.getType(), TableConstraint::PRIMARY_KEY);
    EXPECT_EQ(pk_constraint.getColumns().size(), 2);

    // 唯一约束
    UniqueConstraint unique_constraint;
    unique_constraint.addColumn("email");

    EXPECT_EQ(unique_constraint.getType(), TableConstraint::UNIQUE);
    EXPECT_EQ(unique_constraint.getColumns().size(), 1);

    // 外键约束
    ForeignKeyConstraint fk_constraint;
    fk_constraint.addColumn("user_id");
    fk_constraint.setReferencedTable("users");
    fk_constraint.setReferencedColumn("id");

    EXPECT_EQ(fk_constraint.getType(), TableConstraint::FOREIGN_KEY);
    EXPECT_EQ(fk_constraint.getReferencedTable(), "users");
    EXPECT_EQ(fk_constraint.getReferencedColumn(), "id");
}

// 测试语句类型识别
TEST_F(ASTNodesTest, StatementTypesTest) {
    CreateStatement create_stmt(CreateStatement::TABLE);
    EXPECT_EQ(create_stmt.getType(), Statement::CREATE);
    EXPECT_EQ(create_stmt.getTypeName(), "CREATE");

    SelectStatement select_stmt;
    EXPECT_EQ(select_stmt.getType(), Statement::SELECT);
    EXPECT_EQ(select_stmt.getTypeName(), "SELECT");

    InsertStatement insert_stmt;
    EXPECT_EQ(insert_stmt.getType(), Statement::INSERT);
    EXPECT_EQ(insert_stmt.getTypeName(), "INSERT");

    UpdateStatement update_stmt;
    EXPECT_EQ(update_stmt.getType(), Statement::UPDATE);
    EXPECT_EQ(update_stmt.getTypeName(), "UPDATE");

    DeleteStatement delete_stmt;
    EXPECT_EQ(delete_stmt.getType(), Statement::DELETE);
    EXPECT_EQ(delete_stmt.getTypeName(), "DELETE");

    DropStatement drop_stmt(DropStatement::TABLE);
    EXPECT_EQ(drop_stmt.getType(), Statement::DROP);
    EXPECT_EQ(drop_stmt.getTypeName(), "DROP");
}

} // namespace sql_parser
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
