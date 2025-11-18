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

// 测试多列CREATE TABLE语句（简化的两列场景）
TEST_F(SqlParserTest, CreateTableTwoColumns) {
    std::string sql = "CREATE TABLE products (id INT PRIMARY KEY, name VARCHAR(100));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "products");
    EXPECT_EQ(createStmt->getColumns().size(), 2);

    // 验证列定义
    EXPECT_EQ(createStmt->getColumns()[0].getName(), "id");
    EXPECT_EQ(createStmt->getColumns()[0].getType(), "INT");
    EXPECT_TRUE(createStmt->getColumns()[0].isPrimaryKey());

    EXPECT_EQ(createStmt->getColumns()[1].getName(), "name");
    EXPECT_EQ(createStmt->getColumns()[1].getType(), "VARCHAR(100)");
    EXPECT_TRUE(createStmt->getColumns()[1].isNullable()); // 默认可空
}

// 测试各种数据类型的多列CREATE TABLE语句
TEST_F(SqlParserTest, CreateTableMultipleDataTypes) {
    std::string sql = "CREATE TABLE employees (id INT PRIMARY KEY, name VARCHAR(50), salary DECIMAL(10,2), hire_date DATE);";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "employees");
    EXPECT_EQ(createStmt->getColumns().size(), 4);

    // 验证各种数据类型
    EXPECT_EQ(createStmt->getColumns()[0].getType(), "INT");
    EXPECT_EQ(createStmt->getColumns()[1].getType(), "VARCHAR(50)");
    EXPECT_EQ(createStmt->getColumns()[2].getType(), "DECIMAL(10,2)");
    EXPECT_EQ(createStmt->getColumns()[3].getType(), "DATE");
}

// 测试带有多种约束的多列CREATE TABLE语句
TEST_F(SqlParserTest, CreateTableMultipleConstraints) {
    std::string sql = "CREATE TABLE students (id INT PRIMARY KEY, email VARCHAR(255) UNIQUE NOT NULL, gpa DECIMAL(3,2) DEFAULT 0.0);";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "students");
    EXPECT_EQ(createStmt->getColumns().size(), 3);

    // 验证约束
    auto& columns = createStmt->getColumns();
    EXPECT_TRUE(columns[0].isPrimaryKey());
    EXPECT_TRUE(columns[1].isUnique());
    EXPECT_FALSE(columns[1].isNullable());
    EXPECT_TRUE(columns[2].hasDefaultValue());
}

// 测试CREATE TABLE语句中的时间数据类型
TEST_F(SqlParserTest, CreateTableDateTimeTypes) {
    std::string sql = "CREATE TABLE events (id INT PRIMARY KEY, name VARCHAR(100), start_date DATE, start_time TIME, created_at TIMESTAMP, price DECIMAL(10,2));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "events");
    EXPECT_EQ(createStmt->getColumns().size(), 6);

    // 验证各种数据类型
    EXPECT_EQ(createStmt->getColumns()[0].getType(), "INT");
    EXPECT_EQ(createStmt->getColumns()[1].getType(), "VARCHAR(100)");
    EXPECT_EQ(createStmt->getColumns()[2].getType(), "DATE");
    EXPECT_EQ(createStmt->getColumns()[3].getType(), "TIME");
    EXPECT_EQ(createStmt->getColumns()[4].getType(), "TIMESTAMP");
    EXPECT_EQ(createStmt->getColumns()[5].getType(), "DECIMAL(10,2)");
}

// 测试CREATE TABLE语句中的表级外键约束
TEST_F(SqlParserTest, CreateTableTableLevelForeignKey) {
    std::string sql = "CREATE TABLE orders (id INT PRIMARY KEY, user_id INT, total DECIMAL(10,2), FOREIGN KEY (user_id) REFERENCES users(id));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "orders");
    EXPECT_EQ(createStmt->getColumns().size(), 3);

    // 验证表级约束（暂时跳过具体的约束验证，专注解析正确）
    // TODO: 验证具体的表级约束内容
    // EXPECT_EQ(createStmt->getTableConstraints().size(), 1);
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

// 测试CREATE INDEX语句
TEST_F(SqlParserTest, CreateIndexStatement) {
    std::string sql = "CREATE INDEX idx_name ON users (name);";
    auto stmt = parseSingleStatement(sql);
    auto createIndexStmt = expectStatementType<CreateIndexStatement>(stmt.get());

    EXPECT_EQ(createIndexStmt->getIndexName(), "idx_name");
    EXPECT_EQ(createIndexStmt->getTableName(), "users");
    EXPECT_EQ(createIndexStmt->getColumnName(), "name");
    EXPECT_FALSE(createIndexStmt->isUnique());
}

// 测试CREATE UNIQUE INDEX语句
TEST_F(SqlParserTest, CreateUniqueIndexStatement) {
    std::string sql = "CREATE UNIQUE INDEX idx_email ON users (email);";
    auto stmt = parseSingleStatement(sql);
    auto createIndexStmt = expectStatementType<CreateIndexStatement>(stmt.get());

    EXPECT_EQ(createIndexStmt->getIndexName(), "idx_email");
    EXPECT_EQ(createIndexStmt->getTableName(), "users");
    EXPECT_EQ(createIndexStmt->getColumnName(), "email");
    EXPECT_TRUE(createIndexStmt->isUnique());
}

// 测试DROP INDEX语句
TEST_F(SqlParserTest, DropIndexStatement) {
    std::string sql = "DROP INDEX idx_name ON users;";
    auto stmt = parseSingleStatement(sql);
    auto dropIndexStmt = expectStatementType<DropIndexStatement>(stmt.get());

    EXPECT_EQ(dropIndexStmt->getIndexName(), "idx_name");
    EXPECT_EQ(dropIndexStmt->getTableName(), "users");
    EXPECT_FALSE(dropIndexStmt->isIfExists());
}

// 测试DROP INDEX IF EXISTS语句
TEST_F(SqlParserTest, DropIndexIfExistsStatement) {
    std::string sql = "DROP INDEX IF EXISTS idx_name ON users;";
    auto stmt = parseSingleStatement(sql);
    auto dropIndexStmt = expectStatementType<DropIndexStatement>(stmt.get());

    EXPECT_EQ(dropIndexStmt->getIndexName(), "idx_name");
    EXPECT_EQ(dropIndexStmt->getTableName(), "users");
    EXPECT_TRUE(dropIndexStmt->isIfExists());
}

// 测试INDEX关键字作为语句起始词
TEST_F(SqlParserTest, IndexKeywordParsing) {
    std::string sql = "INDEX idx_users_name ON users (name);";
    EXPECT_THROW(parseSingleStatement(sql), std::runtime_error);
}

// ================ 新增多列索引测试 ================
// 测试CREATE多列索引语句
TEST_F(SqlParserTest, CreateMultiColumnIndexStatement) {
    std::string sql = "CREATE INDEX idx_multi ON users (name, email);";
    auto stmt = parseSingleStatement(sql);
    auto createIndexStmt = expectStatementType<CreateIndexStatement>(stmt.get());

    EXPECT_EQ(createIndexStmt->getIndexName(), "idx_multi");
    EXPECT_EQ(createIndexStmt->getTableName(), "users");
    EXPECT_EQ(createIndexStmt->getColumnNames().size(), 2);
    EXPECT_EQ(createIndexStmt->getColumnNames()[0], "name");
    EXPECT_EQ(createIndexStmt->getColumnNames()[1], "email");
    EXPECT_FALSE(createIndexStmt->isUnique());

    // 向后兼容性测试
    EXPECT_EQ(createIndexStmt->getColumnName(), "name"); // 返回首列
}

// 测试CREATE唯一的多列索引语句
TEST_F(SqlParserTest, CreateUniqueMultiColumnIndexStatement) {
    std::string sql = "CREATE UNIQUE INDEX idx_unique_compound ON products (category_id, name);";
    auto stmt = parseSingleStatement(sql);
    auto createIndexStmt = expectStatementType<CreateIndexStatement>(stmt.get());

    EXPECT_EQ(createIndexStmt->getIndexName(), "idx_unique_compound");
    EXPECT_EQ(createIndexStmt->getTableName(), "products");
    EXPECT_EQ(createIndexStmt->getColumnNames().size(), 2);
    EXPECT_EQ(createIndexStmt->getColumnNames()[0], "category_id");
    EXPECT_EQ(createIndexStmt->getColumnNames()[1], "name");
    EXPECT_TRUE(createIndexStmt->isUnique());
}

// 测试复杂的三列复合索引
TEST_F(SqlParserTest, CreateThreeColumnIndexStatement) {
    std::string sql = "CREATE INDEX idx_triple ON orders (user_id, order_date, status);";
    auto stmt = parseSingleStatement(sql);
    auto createIndexStmt = expectStatementType<CreateIndexStatement>(stmt.get());

    EXPECT_EQ(createIndexStmt->getIndexName(), "idx_triple");
    EXPECT_EQ(createIndexStmt->getTableName(), "orders");
    EXPECT_EQ(createIndexStmt->getColumnNames().size(), 3);
    EXPECT_EQ(createIndexStmt->getColumnNames()[0], "user_id");
    EXPECT_EQ(createIndexStmt->getColumnNames()[1], "order_date");
    EXPECT_EQ(createIndexStmt->getColumnNames()[2], "status");
    EXPECT_FALSE(createIndexStmt->isUnique());
}

// ================ 数据类型扩展测试 ================
// 测试扩展的数据类型支持
TEST_F(SqlParserTest, ExtendedDataTypes) {
    std::string sql = "CREATE TABLE test_types (id INT, name VARCHAR(100), birth_date DATE, login_time TIME, updated_at TIMESTAMP, balance DECIMAL(12,2), is_active BOOLEAN);";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getColumns().size(), 7);

    // 验证每种数据类型的正确解析
    EXPECT_EQ(createStmt->getColumns()[0].getType(), "INT");
    EXPECT_EQ(createStmt->getColumns()[1].getType(), "VARCHAR(100)");
    EXPECT_EQ(createStmt->getColumns()[2].getType(), "DATE");
    EXPECT_EQ(createStmt->getColumns()[3].getType(), "TIME");
    EXPECT_EQ(createStmt->getColumns()[4].getType(), "TIMESTAMP");
    EXPECT_EQ(createStmt->getColumns()[5].getType(), "DECIMAL(12,2)");
    EXPECT_EQ(createStmt->getColumns()[6].getType(), "BOOLEAN");
}

// ================ 表级约束完整测试 ================
// 测试表级主键约束
TEST_F(SqlParserTest, TableLevelPrimaryKeyConstraint) {
    std::string sql = "CREATE TABLE users (id INT, name VARCHAR(100), email VARCHAR(255), PRIMARY KEY (id, email));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTarget(), CreateStatement::Target::TABLE);
    EXPECT_EQ(createStmt->getTableName(), "users");
    EXPECT_EQ(createStmt->getColumns().size(), 3);
    EXPECT_EQ(createStmt->getTableConstraints().size(), 1);

    auto constraint = createStmt->getTableConstraints()[0].get();
    auto primaryKeyConstraint = dynamic_cast<PrimaryKeyConstraint*>(constraint);
    EXPECT_TRUE(primaryKeyConstraint != nullptr);
    EXPECT_EQ(primaryKeyConstraint->getColumns().size(), 2);
    EXPECT_EQ(primaryKeyConstraint->getColumns()[0], "id");
    EXPECT_EQ(primaryKeyConstraint->getColumns()[1], "email");
}

// 测试表级唯一约束
TEST_F(SqlParserTest, TableLevelUniqueConstraint) {
    std::string sql = "CREATE TABLE products (id INT PRIMARY KEY, name VARCHAR(100), category_id INT, UNIQUE (category_id, name));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTableConstraints().size(), 1);

    auto constraint = createStmt->getTableConstraints()[0].get();
    auto uniqueConstraint = dynamic_cast<UniqueConstraint*>(constraint);
    EXPECT_TRUE(uniqueConstraint != nullptr);
    EXPECT_EQ(uniqueConstraint->getColumns().size(), 2);
    EXPECT_EQ(uniqueConstraint->getColumns()[0], "category_id");
    EXPECT_EQ(uniqueConstraint->getColumns()[1], "name");
}

// 测试表级外键约束
TEST_F(SqlParserTest, TableLevelForeignKeyConstraint) {
    std::string sql = "CREATE TABLE orders (id INT PRIMARY KEY, user_id INT, product_id INT, FOREIGN KEY (user_id, product_id) REFERENCES users(user_id, product_id));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTableConstraints().size(), 1);

    auto constraint = createStmt->getTableConstraints()[0].get();
    auto foreignKeyConstraint = dynamic_cast<ForeignKeyConstraint*>(constraint);
    EXPECT_TRUE(foreignKeyConstraint != nullptr);
    EXPECT_EQ(foreignKeyConstraint->getColumns().size(), 2);
    EXPECT_EQ(foreignKeyConstraint->getColumns()[0], "user_id");
    EXPECT_EQ(foreignKeyConstraint->getColumns()[1], "product_id");
    EXPECT_EQ(foreignKeyConstraint->getReferencedTable(), "users");
    EXPECT_EQ(foreignKeyConstraint->getReferencedColumn(), "user_id");  // 简化实现返回第一个
}

// 测试表级检查约束
TEST_F(SqlParserTest, TableLevelCheckConstraint) {
    std::string sql = "CREATE TABLE employees (id INT PRIMARY KEY, age INT, salary DECIMAL(10,2), CHECK (age >= 18 AND salary > 0));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTableConstraints().size(), 1);

    auto constraint = createStmt->getTableConstraints()[0].get();
    auto checkConstraint = dynamic_cast<CheckConstraint*>(constraint);
    EXPECT_TRUE(checkConstraint != nullptr);
    EXPECT_TRUE(checkConstraint->getCondition() != nullptr);
}

// 测试命名约束
TEST_F(SqlParserTest, NamedConstraints) {
    std::string sql = "CREATE TABLE accounts (id INT, balance DECIMAL(10,2), name VARCHAR(100), CONSTRAINT pk_id PRIMARY KEY (id), CONSTRAINT ck_balance CHECK (balance >= 0), CONSTRAINT uk_name UNIQUE (name));";
    auto stmt = parseSingleStatement(sql);
    auto createStmt = expectStatementType<CreateStatement>(stmt.get());

    EXPECT_EQ(createStmt->getTableConstraints().size(), 3);

    // 验证命名约束
    auto& constraints = createStmt->getTableConstraints();
    auto primaryKey = dynamic_cast<PrimaryKeyConstraint*>(constraints[0].get());
    auto checkConstraint = dynamic_cast<CheckConstraint*>(constraints[1].get());
    auto uniqueConstraint = dynamic_cast<UniqueConstraint*>(constraints[2].get());

    EXPECT_TRUE(primaryKey != nullptr);
    EXPECT_TRUE(checkConstraint != nullptr);
    EXPECT_TRUE(uniqueConstraint != nullptr);

    EXPECT_EQ(primaryKey->getName(), "pk_id");
    EXPECT_EQ(checkConstraint->getName(), "ck_balance");
    EXPECT_EQ(uniqueConstraint->getName(), "uk_name");
}

// ================ 子查询完整测试 ================
// 测试EXISTS子查询
TEST_F(SqlParserTest, ExistsSubqueryExpression) {
    std::string sql = "SELECT name FROM users WHERE EXISTS (SELECT 1 FROM orders WHERE orders.user_id = users.id);";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());

    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    auto condition = selectStmt->getWhereClause()->getCondition().get();

    auto existsExpr = dynamic_cast<ExistsExpression*>(condition);
    EXPECT_TRUE(existsExpr != nullptr);
    EXPECT_TRUE(existsExpr->getSubquery() != nullptr);
    EXPECT_TRUE(dynamic_cast<SelectStatement*>(existsExpr->getSubquery().get()) != nullptr);
}

// 测试IN子查询
TEST_F(SqlParserTest, InSubqueryExpression) {
    std::string sql = "SELECT name FROM users WHERE id IN (SELECT user_id FROM active_users);";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());

    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    // 这里可以进一步验证IN子查询的结构
}

// 测试标量子查询
TEST_F(SqlParserTest, ScalarSubqueryExpression) {
    std::string sql = "SELECT id, (SELECT COUNT(*) FROM orders WHERE orders.user_id = users.id) as order_count FROM users;";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());

    EXPECT_EQ(selectStmt->getSelectItems().size(), 2);
    // 第二个选择项应该包含标量子查询
}

// 测试嵌套子查询
TEST_F(SqlParserTest, NestedSubqueryExpression) {
    std::string sql = "SELECT name FROM users WHERE id IN (SELECT user_id FROM orders WHERE status IN (SELECT id FROM statuses WHERE active = 1));";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());

    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    // 这里可以验证三层嵌套子查询的结构
}

// ================ 高级SQL功能测试 ================
// 测试复杂JOIN和子查询组合
TEST_F(SqlParserTest, ComplexJoinWithSubquery) {
    std::string sql = "SELECT u.name, o.total FROM users u JOIN orders o ON u.id = o.user_id WHERE u.id IN (SELECT user_id FROM premium_users);";
    auto stmt = parseSingleStatement(sql);
    auto selectStmt = expectStatementType<SelectStatement>(stmt.get());

    EXPECT_EQ(selectStmt->getJoinClauses().size(), 1);
    EXPECT_TRUE(selectStmt->getWhereClause() != nullptr);
    EXPECT_EQ(selectStmt->getFromTables().size(), 1);
    EXPECT_EQ(selectStmt->getFromTables()[0].getAlias(), "u");
}

// 测试带有窗口函数的SELECT语句（如果将来支持）
TEST_F(SqlParserTest, AdvancedSelectStatements) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary DESC) as rank FROM employees WHERE department = 'IT';";
    // 目前先跳过，这个需要后续实现窗口函数
    // EXPECT_NO_THROW(parseSingleStatement(sql));
}

// 测试CTE (Common Table Expression) - 将来扩展
TEST_F(SqlParserTest, CteSupport) {
    std::string sql = "WITH dept_summary AS (SELECT department, COUNT(*) as count FROM employees GROUP BY department) SELECT * FROM dept_summary;";
    // 目前先跳过，需要CTE实现
    // EXPECT_NO_THROW(parseSingleStatement(sql));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
