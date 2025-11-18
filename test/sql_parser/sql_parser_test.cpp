#include "sql_parser/parser.h"
#include "sql_parser/lexer.h"
#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <string>
#include <memory>

using namespace sqlcc::sql_parser;

class SqlParserTest : public ::testing::Test {
protected:
    // 辅助方法：解析单个SQL语句
    std::unique_ptr<Statement> parseSingleStatement(const std::string& sql) {
        Lexer lexer(sql);
        Parser parser(lexer);
        return parser.parseStatement();
    }
    
    // 辅助方法：验证解析结果是否为预期的语句类型
    template<typename T>
    T* expectStatementType(Statement* stmt) {
        EXPECT_TRUE(dynamic_cast<T*>(stmt) != nullptr) << "Statement is not of expected type";
        return dynamic_cast<T*>(stmt);
    }
    
    // 辅助方法：验证表达式是否为预期的类型
    template<typename T>
    T* expectExpressionType(Expression* expr) {
        EXPECT_TRUE(dynamic_cast<T*>(expr) != nullptr) << "Expression is not of expected type";
        return dynamic_cast<T*>(expr);
    }
};

// 测试SELECT语句解析
TEST_F(SqlParserTest, SelectStatementBasic) {
    // 测试简单的SELECT语句
    std::string sql = "SELECT id, name, age FROM users;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_FALSE(selectStmt->isDistinct());
    EXPECT_EQ(selectStmt->getSelectItems().size(), 3);
    EXPECT_EQ(selectStmt->getFromTables().size(), 1);
    EXPECT_EQ(selectStmt->getFromTables()[0].getName(), "users");
}

// 测试SELECT语句的WHERE子句
TEST_F(SqlParserTest, SelectStatementWhereClause) {
    std::string sql = "SELECT id, name FROM users WHERE age > 18;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    auto condition = selectStmt->getWhereClause()->getCondition().get();
    auto binaryExpr = expectExpressionType<BinaryExpression>(condition);
    
    EXPECT_EQ(binaryExpr->getOperator(), Token::OPERATOR_GREATER);
    
    auto leftExpr = expectExpressionType<IdentifierExpression>(binaryExpr->getLeft().get());
    EXPECT_EQ(leftExpr->getName(), "age");
    
    auto rightExpr = expectExpressionType<NumericLiteralExpression>(binaryExpr->getRight().get());
    EXPECT_EQ(rightExpr->getValue(), 18);
    EXPECT_TRUE(rightExpr->isInteger());
}

// 测试SELECT语句的GROUP BY子句
TEST_F(SqlParserTest, SelectStatementGroupByClause) {
    std::string sql = "SELECT department, COUNT(*) FROM employees GROUP BY department;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_TRUE(selectStmt->getGroupByClause() != nullptr);
    EXPECT_FALSE(selectStmt->getGroupByClause()->hasHaving());
    EXPECT_EQ(selectStmt->getGroupByClause()->getGroupByItems().size(), 1);
}

// 测试SELECT语句的ORDER BY子句
TEST_F(SqlParserTest, SelectStatementOrderByClause) {
    std::string sql = "SELECT id, name FROM users ORDER BY age DESC, name ASC;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_TRUE(selectStmt->getOrderByClause() != nullptr);
    EXPECT_EQ(selectStmt->getOrderByClause()->getOrderByItems().size(), 2);
    // 这里可以进一步验证排序方向
}

// 测试SELECT语句的LIMIT和OFFSET子句
TEST_F(SqlParserTest, SelectStatementLimitOffset) {
    std::string sql = "SELECT * FROM users LIMIT 10 OFFSET 20;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_EQ(selectStmt->getLimit(), 10);
    EXPECT_EQ(selectStmt->getOffset(), 20);
}

// 测试SELECT语句的JOIN子句
TEST_F(SqlParserTest, SelectStatementJoinClause) {
    std::string sql = "SELECT users.id, orders.order_id FROM users JOIN orders ON users.id = orders.user_id;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_EQ(selectStmt->getJoinClauses().size(), 1);
    EXPECT_EQ(selectStmt->getJoinClauses()[0]->getType(), JoinClause::Type::INNER);
    EXPECT_EQ(selectStmt->getJoinClauses()[0]->getTable().getName(), "orders");
}

// 测试CREATE TABLE语句
TEST_F(SqlParserTest, CreateTableStatement) {
    std::string sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, age INT DEFAULT 0);";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());
    
    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "users");
    EXPECT_EQ(createStmt->getColumns().size(), 3);
    
    // 验证列定义
    EXPECT_EQ(createStmt->getColumns()[0].getName(), "id");
    EXPECT_EQ(createStmt->getColumns()[0].getType(), "INT");
    EXPECT_TRUE(createStmt->getColumns()[0].isPrimaryKey());
    
    EXPECT_EQ(createStmt->getColumns()[1].getName(), "name");
    EXPECT_EQ(createStmt->getColumns()[1].getType(), "VARCHAR(255)");
    EXPECT_FALSE(createStmt->getColumns()[1].isNullable());
}

// 测试CREATE DATABASE语句
TEST_F(SqlParserTest, CreateDatabaseStatement) {
    std::string sql = "CREATE DATABASE mydb;";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());
    
    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::DATABASE);
    EXPECT_EQ(createStmt->getDatabaseName(), "mydb");
}

// 测试INSERT语句
TEST_F(SqlParserTest, InsertStatement) {
    std::string sql = "INSERT INTO users (id, name, age) VALUES (1, 'John', 30), (2, 'Alice', 25);";
    auto stmt = parseSingleStatement(sql);
    auto insertStmt = expectStatementType<InsertStatement>(stmt.get());
    
    EXPECT_EQ(insertStmt->getTableName(), "users");
    EXPECT_EQ(insertStmt->getColumns().size(), 3);
    EXPECT_EQ(insertStmt->getColumns()[0], "id");
    EXPECT_EQ(insertStmt->getColumns()[1], "name");
    EXPECT_EQ(insertStmt->getColumns()[2], "age");
}

// 测试UPDATE语句
TEST_F(SqlParserTest, UpdateStatement) {
    std::string sql = "UPDATE users SET name = 'Robert', age = 35 WHERE id = 1;";
    auto stmt = parseSingleStatement(sql);
    auto updateStmt = expectStatementType<UpdateStatement>(stmt.get());
    
    EXPECT_EQ(updateStmt->getTableName(), "users");
    EXPECT_EQ(updateStmt->getSetItems().size(), 2);
    EXPECT_TRUE(updateStmt->getWhereClause() != nullptr);
}

// 测试DELETE语句
TEST_F(SqlParserTest, DeleteStatement) {
    std::string sql = "DELETE FROM users WHERE age < 18;";
    auto stmt = parseSingleStatement(sql);
    auto deleteStmt = expectStatementType<DeleteStatement>(stmt.get());
    
    EXPECT_EQ(deleteStmt->getTableName(), "users");
    EXPECT_TRUE(deleteStmt->getWhereClause() != nullptr);
}

// 测试DROP TABLE语句
TEST_F(SqlParserTest, DropTableStatement) {
    std::string sql = "DROP TABLE IF EXISTS temp_table;";
    auto stmt = parseSingleStatement(sql);
    auto dropStmt = expectStatementType<DropStatement>(stmt.get());
    
    EXPECT_EQ(dropStmt->getTarget(), DropStatement::Target::TABLE);
    EXPECT_EQ(dropStmt->getTableName(), "temp_table");
    EXPECT_TRUE(dropStmt->isIfExists());
}

// 测试DROP DATABASE语句
TEST_F(SqlParserTest, DropDatabaseStatement) {
    std::string sql = "DROP DATABASE mydb;";
    auto stmt = parseSingleStatement(sql);
    auto dropStmt = expectStatementType<DropStatement>(stmt.get());
    
    EXPECT_EQ(dropStmt->getTarget(), DropStatement::Target::DATABASE);
    EXPECT_EQ(dropStmt->getDatabaseName(), "mydb");
    EXPECT_FALSE(dropStmt->isIfExists());
}

// 测试ALTER TABLE语句
TEST_F(SqlParserTest, AlterTableStatement) {
    std::string sql = "ALTER TABLE users ADD COLUMN email VARCHAR(255);";
    auto stmt = parseSingleStatement(sql);
    auto alterStmt = expectStatementType<AlterStatement>(stmt.get());
    
    EXPECT_EQ(alterStmt->getTarget(), AlterStatement::Target::TABLE);
    EXPECT_EQ(alterStmt->getTableName(), "users");
    EXPECT_EQ(alterStmt->getAction(), AlterStatement::Action::ADD_COLUMN);
}

// 测试USE语句
TEST_F(SqlParserTest, UseStatement) {
    std::string sql = "USE mydb;";
    auto stmt = parseSingleStatement(sql);
    auto useStmt = expectStatementType<UseStatement>(stmt.get());
    
    EXPECT_EQ(useStmt->getDatabaseName(), "mydb");
}

// 测试复合表达式
TEST_F(SqlParserTest, ComplexExpression) {
    std::string sql = "SELECT * FROM users WHERE age > 18 AND (name LIKE '%John%' OR email LIKE '%john.com%');";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    // 这里可以进一步验证复杂表达式的结构
}

// 测试函数调用
TEST_F(SqlParserTest, FunctionCall) {
    std::string sql = "SELECT COUNT(*), AVG(age), MAX(salary) FROM employees;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_EQ(selectStmt->getSelectItems().size(), 3);
    // 这里可以进一步验证函数调用的结构
}

// 测试多语句解析
TEST_F(SqlParserTest, MultipleStatements) {
    std::string sql = "SELECT * FROM users; INSERT INTO logs VALUES (NOW());";
    Lexer lexer(sql);
    Parser parser(lexer);
    auto statements = parser.parseStatements();
    
    EXPECT_EQ(statements.size(), 2);
    EXPECT_TRUE(dynamic_cast<SelectStatement*>(statements[0].get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<InsertStatement*>(statements[1].get()) != nullptr);
}

// 测试错误处理
TEST_F(SqlParserTest, ErrorHandling) {
    std::string invalidSql = "SELECT * FROM;";
    EXPECT_THROW(parseSingleStatement(invalidSql), std::exception);
}

// 测试词法分析器
TEST_F(SqlParserTest, LexerBasic) {
    std::string sql = "SELECT id, name FROM users WHERE age > 18;";
    Lexer lexer(sql);
    
    Token token = lexer.nextToken();
    EXPECT_EQ(token.getType(), Token::KEYWORD_SELECT);
    
    token = lexer.nextToken();
    EXPECT_EQ(token.getType(), Token::IDENTIFIER);
    EXPECT_EQ(token.getLexeme(), "id");
    
    token = lexer.nextToken();
    EXPECT_EQ(token.getType(), Token::PUNCTUATION_COMMA);
}

// 测试注释处理
TEST_F(SqlParserTest, CommentHandling) {
    std::string sql = "SELECT * FROM users -- This is a comment\nWHERE age > 18;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
}

// 测试字符串字面量
TEST_F(SqlParserTest, StringLiteral) {
    std::string sql = "SELECT 'Hello, World!' AS greeting;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());
    
    EXPECT_EQ(selectStmt->getSelectItems().size(), 1);
    // 这里可以进一步验证字符串字面量的值
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}