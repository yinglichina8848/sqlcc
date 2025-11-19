// tests/unit/ast_nodes_comprehensive_test.cc
// Comprehensive unit tests for AST nodes to achieve >60% coverage
#include "../../include/sql_parser/ast_node.h"
#include "../../include/sql_parser/ast_nodes.h"
#include "../../include/sql_parser/token.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace sqlcc::sql_parser;

namespace sqlcc {
namespace sql_parser {

// Mock NodeVisitor for testing
class MockNodeVisitor : public NodeVisitor {
public:
  // Tracking visits for verification
  void visit(CreateStatement &stmt) override { createVisitCount++; }
  void visit(SelectStatement &stmt) override { selectVisitCount++; }
  void visit(InsertStatement &stmt) override { insertVisitCount++; }
  void visit(UpdateStatement &stmt) override { updateVisitCount++; }
  void visit(DeleteStatement &stmt) override { deleteVisitCount++; }
  void visit(DropStatement &stmt) override { dropVisitCount++; }
  void visit(AlterStatement &stmt) override { alterVisitCount++; }
  void visit(UseStatement &stmt) override { useVisitCount++; }
  void visit(CreateIndexStatement &stmt) override { createIndexVisitCount++; }
  void visit(DropIndexStatement &stmt) override { dropIndexVisitCount++; }

  void visit(IdentifierExpression &expr) override { identifierVisitCount++; }
  void visit(StringLiteralExpression &expr) override {
    stringLiteralVisitCount++;
  }
  void visit(NumericLiteralExpression &expr) override {
    numericLiteralVisitCount++;
  }
  void visit(BinaryExpression &expr) override { binaryVisitCount++; }
  void visit(UnaryExpression &expr) override { unaryVisitCount++; }
  void visit(FunctionExpression &expr) override { functionVisitCount++; }
  void visit(ExistsExpression &expr) override { existsVisitCount++; }
  void visit(class InExpression &expr) override { inVisitCount++; }

  void visit(WhereClause &clause) override { whereVisitCount++; }
  void visit(JoinClause &clause) override { joinVisitCount++; }
  void visit(GroupByClause &clause) override { groupByVisitCount++; }
  void visit(OrderByClause &clause) override { orderByVisitCount++; }

  // Counters
  int createVisitCount = 0;
  int selectVisitCount = 0;
  int insertVisitCount = 0;
  int updateVisitCount = 0;
  int deleteVisitCount = 0;
  int dropVisitCount = 0;
  int alterVisitCount = 0;
  int useVisitCount = 0;
  int createIndexVisitCount = 0;
  int dropIndexVisitCount = 0;

  int identifierVisitCount = 0;
  int stringLiteralVisitCount = 0;
  int numericLiteralVisitCount = 0;
  int binaryVisitCount = 0;
  int unaryVisitCount = 0;
  int functionVisitCount = 0;
  int existsVisitCount = 0;
  int inVisitCount = 0;

  int whereVisitCount = 0;
  int joinVisitCount = 0;
  int groupByVisitCount = 0;
  int orderByVisitCount = 0;
};

class AstNodesComprehensiveTest : public ::testing::Test {
protected:
  void SetUp() override { visitor = std::make_unique<MockNodeVisitor>(); }

  std::unique_ptr<MockNodeVisitor> visitor;
};

// ================ HELPER STRUCTURES TESTS ================

// Test TableReference
TEST_F(AstNodesComprehensiveTest, TableReferenceTest) {
  // Test constructor
  TableReference table("users");
  EXPECT_EQ(table.getName(), "users");
  EXPECT_EQ(table.getAlias(), "");
  EXPECT_FALSE(table.hasAlias());

  // Test alias setting
  table.setAlias("u");
  EXPECT_EQ(table.getAlias(), "u");
  EXPECT_TRUE(table.hasAlias());

  // Test with alias constructor equivalent
  TableReference table2("orders");
  table2.setAlias("o");
  EXPECT_EQ(table2.getName(), "orders");
  EXPECT_EQ(table2.getAlias(), "o");
  EXPECT_TRUE(table2.hasAlias());
}

// Test TableConstraint base class
TEST_F(AstNodesComprehensiveTest, TableConstraintTest) {
  // Test PRIMARY_KEY constraint
  TableConstraint pkConstraint(TableConstraint::PRIMARY_KEY);
  EXPECT_EQ(pkConstraint.getType(), TableConstraint::PRIMARY_KEY);
  EXPECT_EQ(pkConstraint.getName(), "");

  // Test constraint naming
  pkConstraint.setName("pk_users");
  EXPECT_EQ(pkConstraint.getName(), "pk_users");
}

// Test PrimaryKeyConstraint
TEST_F(AstNodesComprehensiveTest, PrimaryKeyConstraintTest) {
  PrimaryKeyConstraint pk;

  // Test single column
  pk.addColumn("id");
  EXPECT_EQ(pk.getColumns().size(), 1);
  EXPECT_EQ(pk.getColumns()[0], "id");

  // Test multiple columns
  pk.addColumn("name");
  EXPECT_EQ(pk.getColumns().size(), 2);
  EXPECT_EQ(pk.getColumns()[1], "name");

  // Test constraint type
  EXPECT_EQ(pk.getType(), TableConstraint::PRIMARY_KEY);
}

// Test UniqueConstraint
TEST_F(AstNodesComprehensiveTest, UniqueConstraintTest) {
  UniqueConstraint unique;

  // Test adding columns
  unique.addColumn("email");
  EXPECT_EQ(unique.getColumns().size(), 1);
  EXPECT_EQ(unique.getColumns()[0], "email");

  // Test multiple columns
  unique.addColumn("phone");
  EXPECT_EQ(unique.getColumns().size(), 2);

  EXPECT_EQ(unique.getType(), TableConstraint::UNIQUE);
}

// Test ForeignKeyConstraint
TEST_F(AstNodesComprehensiveTest, ForeignKeyConstraintTest) {
  ForeignKeyConstraint fk;

  // Test adding source columns
  fk.addColumn("user_id");
  EXPECT_EQ(fk.getColumns().size(), 1);
  EXPECT_EQ(fk.getColumns()[0], "user_id");

  // Test referenced table and column
  fk.setReferencedTable("users");
  fk.setReferencedColumn("id");
  EXPECT_EQ(fk.getReferencedTable(), "users");
  EXPECT_EQ(fk.getReferencedColumn(), "id");

  EXPECT_EQ(fk.getType(), TableConstraint::FOREIGN_KEY);
}

// Test CheckConstraint
TEST_F(AstNodesComprehensiveTest, CheckConstraintTest) {
  CheckConstraint check;

  // Test setting condition
  auto condition = std::make_unique<NumericLiteralExpression>(18, true);
  check.setCondition(std::move(condition));
  ASSERT_TRUE(check.getCondition() != nullptr);
  ASSERT_TRUE(dynamic_cast<NumericLiteralExpression *>(
                  check.getCondition().get()) != nullptr);

  EXPECT_EQ(check.getType(), TableConstraint::CHECK);
}

// ================ COLUMN DEFINITION TESTS ================

// Test ColumnDefinition
TEST_F(AstNodesComprehensiveTest, ColumnDefinitionTest) {
  // Test constructor
  ColumnDefinition col("id", "INT");
  EXPECT_EQ(col.getName(), "id");
  EXPECT_EQ(col.getType(), "INT");

  // Test nullable setting
  EXPECT_TRUE(col.isNullable()); // Default
  col.setNullable(false);
  EXPECT_FALSE(col.isNullable());

  // Test primary key setting
  EXPECT_FALSE(col.isPrimaryKey()); // Default
  col.setPrimaryKey(true);
  EXPECT_TRUE(col.isPrimaryKey());

  // Test unique setting
  EXPECT_FALSE(col.isUnique()); // Default
  col.setUnique(true);
  EXPECT_TRUE(col.isUnique());

  // Test default value setting
  auto defaultVal = std::make_unique<StringLiteralExpression>("default");
  col.setDefaultValue(std::move(defaultVal));
  EXPECT_TRUE(col.hasDefaultValue());

  // Test foreign key setting
  col.setForeignKey("users", "id");
  EXPECT_TRUE(col.isForeignKey());
  EXPECT_EQ(col.getReferencedTable(), "users");
  EXPECT_EQ(col.getReferencedColumn(), "id");

  // Test check constraint setting
  auto checkExpr = std::make_unique<NumericLiteralExpression>(100);
  col.setCheckConstraint(std::move(checkExpr));
  EXPECT_TRUE(col.hasCheckConstraint());
}

// ================ SELECT ITEM TESTS ================

// Test SelectItem
TEST_F(AstNodesComprehensiveTest, SelectItemTest) {
  // Test constructor
  auto expr = std::make_unique<IdentifierExpression>("name");
  SelectItem item(std::move(expr));

  // Test alias setting
  EXPECT_FALSE(item.hasAlias());
  item.setAlias("employee_name");
  EXPECT_TRUE(item.hasAlias());
  EXPECT_EQ(item.getAlias(), "employee_name");

  // Test expression getting
  ASSERT_TRUE(item.getExpression() != nullptr);
  ASSERT_TRUE(dynamic_cast<IdentifierExpression *>(
                  item.getExpression().get()) != nullptr);
}

// ================ STATEMENT TESTS ================

// Test CreateStatement
TEST_F(AstNodesComprehensiveTest, CreateStatementTest) {
  // Test DATABASE creation
  CreateStatement dbStmt(CreateStatement::Target::DATABASE);
  EXPECT_EQ(dbStmt.getType(), Statement::CREATE);
  EXPECT_EQ(dbStmt.getTarget(), CreateStatement::Target::DATABASE);

  dbStmt.setDatabaseName("mydb");
  EXPECT_EQ(dbStmt.getDatabaseName(), "mydb");

  // Test TABLE creation
  CreateStatement tableStmt(CreateStatement::Target::TABLE);
  tableStmt.setTableName("users");

  // Add columns
  ColumnDefinition col1("id", "INT");
  tableStmt.addColumn(std::move(col1));

  ColumnDefinition col2("name", "VARCHAR(100)");
  tableStmt.addColumn(std::move(col2));

  EXPECT_EQ(tableStmt.getTableName(), "users");
  EXPECT_EQ(tableStmt.getColumns().size(), 2);

  // Add table constraints
  auto pkConstraint = std::make_unique<PrimaryKeyConstraint>();
  pkConstraint->addColumn("id");
  tableStmt.addTableConstraint(std::move(pkConstraint));

  EXPECT_EQ(tableStmt.getTableConstraints().size(), 1);

  // Test visitor pattern
  tableStmt.accept(*visitor);
  EXPECT_EQ(visitor->createVisitCount, 1);
}

// Test SelectStatement
TEST_F(AstNodesComprehensiveTest, SelectStatementTest) {
  SelectStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::SELECT);

  // Test DISTINCT setting
  EXPECT_FALSE(stmt.isDistinct());
  stmt.setDistinct(true);
  EXPECT_TRUE(stmt.isDistinct());

  // Test adding select items
  auto expr1 = std::make_unique<IdentifierExpression>("id");
  SelectItem item1(std::move(expr1));
  stmt.addSelectItem(std::move(item1));

  auto expr2 = std::make_unique<IdentifierExpression>("name");
  SelectItem item2(std::move(expr2));
  stmt.addSelectItem(std::move(item2));

  EXPECT_EQ(stmt.getSelectItems().size(), 2);

  // Test FROM table
  TableReference table("users");
  stmt.addFromTable(table);
  EXPECT_EQ(stmt.getFromTables().size(), 1);
  EXPECT_EQ(stmt.getFromTables()[0].getName(), "users");

  // Test WHERE clause
  auto whereCondition = std::make_unique<NumericLiteralExpression>(1, true);
  auto whereClause = std::make_unique<WhereClause>(std::move(whereCondition));
  stmt.setWhereClause(std::move(whereClause));
  ASSERT_TRUE(stmt.getWhereClause() != nullptr);

  // Test JOIN clauses
  auto joinCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL,
      std::make_unique<IdentifierExpression>("u.id"),
      std::make_unique<IdentifierExpression>("o.user_id"));
  TableReference joinTable("orders");
  auto joinClause = std::make_unique<JoinClause>(
      JoinClause::Type::INNER, joinTable, std::move(joinCondition));
  stmt.addJoinClause(std::move(joinClause));
  EXPECT_EQ(stmt.getJoinClauses().size(), 1);

  // Test GROUP BY
  auto groupByClause = std::make_unique<GroupByClause>();
  auto groupExpr = std::make_unique<IdentifierExpression>("department");
  groupByClause->addGroupByItem(std::move(groupExpr));
  stmt.setGroupByClause(std::move(groupByClause));
  ASSERT_TRUE(stmt.getGroupByClause() != nullptr);

  // Test ORDER BY
  auto orderByClause = std::make_unique<OrderByClause>();
  auto orderExpr = std::make_unique<IdentifierExpression>("salary");
  orderByClause->addOrderByItem(std::move(orderExpr),
                                OrderByClause::Direction::DESC);
  stmt.setOrderByClause(std::move(orderByClause));
  ASSERT_TRUE(stmt.getOrderByClause() != nullptr);

  // Test LIMIT/OFFSET
  stmt.setLimit(100);
  stmt.setOffset(50);
  EXPECT_EQ(stmt.getLimit(), 100);
  EXPECT_EQ(stmt.getOffset(), 50);

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->selectVisitCount, 1);
}

// Test InsertStatement
TEST_F(AstNodesComprehensiveTest, InsertStatementTest) {
  InsertStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::INSERT);

  stmt.setTableName("users");
  EXPECT_EQ(stmt.getTableName(), "users");

  // Add columns
  stmt.addColumn("id");
  stmt.addColumn("name");
  stmt.addColumn("age");
  EXPECT_EQ(stmt.getColumns().size(), 3);

  // Add value rows
  std::vector<std::unique_ptr<Expression>> row1;
  row1.push_back(std::make_unique<NumericLiteralExpression>(1, true));
  row1.push_back(std::make_unique<StringLiteralExpression>("John"));
  row1.push_back(std::make_unique<NumericLiteralExpression>(25, true));
  stmt.addValueRow(row1);

  EXPECT_EQ(stmt.getValueRows().size(), 1);

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->insertVisitCount, 1);
}

// Test UpdateStatement
TEST_F(AstNodesComprehensiveTest, UpdateStatementTest) {
  UpdateStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::UPDATE);

  stmt.setTableName("users");
  EXPECT_EQ(stmt.getTableName(), "users");

  // Add SET items
  auto val1 = std::make_unique<StringLiteralExpression>("John Doe");
  stmt.addSetItem("name", std::move(val1));

  auto val2 = std::make_unique<NumericLiteralExpression>(30, true);
  stmt.addSetItem("age", std::move(val2));

  EXPECT_EQ(stmt.getSetItems().size(), 2);

  // Add WHERE clause
  auto whereCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL, std::make_unique<IdentifierExpression>("id"),
      std::make_unique<NumericLiteralExpression>(1, true));
  auto whereClause = std::make_unique<WhereClause>(std::move(whereCondition));
  stmt.setWhereClause(std::move(whereClause));
  ASSERT_TRUE(stmt.getWhereClause() != nullptr);

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->updateVisitCount, 1);
}

// Test DeleteStatement
TEST_F(AstNodesComprehensiveTest, DeleteStatementTest) {
  DeleteStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::DELETE);

  stmt.setTableName("users");
  EXPECT_EQ(stmt.getTableName(), "users");

  // Add WHERE clause
  auto whereCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_LESS, std::make_unique<IdentifierExpression>("age"),
      std::make_unique<NumericLiteralExpression>(18, true));
  auto whereClause = std::make_unique<WhereClause>(std::move(whereCondition));
  stmt.setWhereClause(std::move(whereClause));
  ASSERT_TRUE(stmt.getWhereClause() != nullptr);

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->deleteVisitCount, 1);
}

// Test DropStatement
TEST_F(AstNodesComprehensiveTest, DropStatementTest) {
  // Test DROP DATABASE
  DropStatement dbStmt(DropStatement::Target::DATABASE);
  EXPECT_EQ(dbStmt.getType(), Statement::DROP);
  EXPECT_EQ(dbStmt.getTarget(), DropStatement::Target::DATABASE);

  dbStmt.setDatabaseName("mydb");
  dbStmt.setIfExists(true);
  EXPECT_EQ(dbStmt.getDatabaseName(), "mydb");
  EXPECT_TRUE(dbStmt.isIfExists());

  // Test DROP TABLE
  DropStatement tableStmt(DropStatement::Target::TABLE);
  tableStmt.setTableName("users");
  tableStmt.setIfExists(false);
  EXPECT_EQ(tableStmt.getTarget(), DropStatement::Target::TABLE);
  EXPECT_EQ(tableStmt.getTableName(), "users");
  EXPECT_FALSE(tableStmt.isIfExists());

  // Test visitor
  tableStmt.accept(*visitor);
  EXPECT_EQ(visitor->dropVisitCount, 1);
}

// Test AlterStatement
TEST_F(AstNodesComprehensiveTest, AlterStatementTest) {
  AlterStatement stmt(AlterStatement::Target::TABLE);
  EXPECT_EQ(stmt.getType(), Statement::ALTER);
  EXPECT_EQ(stmt.getTarget(), AlterStatement::Target::TABLE);

  stmt.setTableName("users");
  stmt.setAction(AlterStatement::Action::ADD_COLUMN);
  stmt.setColumnName("email");

  EXPECT_EQ(stmt.getTableName(), "users");
  EXPECT_EQ(stmt.getAction(), AlterStatement::Action::ADD_COLUMN);
  EXPECT_EQ(stmt.getColumnName(), "email");

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->alterVisitCount, 1);
}

// Test UseStatement
TEST_F(AstNodesComprehensiveTest, UseStatementTest) {
  UseStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::USE);

  stmt.setDatabaseName("mydb");
  EXPECT_EQ(stmt.getDatabaseName(), "mydb");

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->useVisitCount, 1);
}

// Test CreateIndexStatement
TEST_F(AstNodesComprehensiveTest, CreateIndexStatementTest) {
  CreateIndexStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::CREATE_INDEX);

  stmt.setIndexName("idx_users_name");
  stmt.setTableName("users");
  stmt.addColumnName("name");
  stmt.addColumnName("email"); // Multi-column index
  stmt.setUnique(true);

  EXPECT_EQ(stmt.getIndexName(), "idx_users_name");
  EXPECT_EQ(stmt.getTableName(), "users");
  EXPECT_EQ(stmt.getColumnNames().size(), 2);
  EXPECT_EQ(stmt.getColumnNames()[0], "name");
  EXPECT_EQ(stmt.getColumnNames()[1], "email");
  EXPECT_EQ(stmt.getColumnName(), "name"); // Backward compatibility
  EXPECT_TRUE(stmt.isUnique());

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->createIndexVisitCount, 1);
}

// Test DropIndexStatement
TEST_F(AstNodesComprehensiveTest, DropIndexStatementTest) {
  DropIndexStatement stmt;
  EXPECT_EQ(stmt.getType(), Statement::DROP_INDEX);

  stmt.setIndexName("idx_users_name");
  stmt.setTableName("users");
  stmt.setIfExists(true);

  EXPECT_EQ(stmt.getIndexName(), "idx_users_name");
  EXPECT_EQ(stmt.getTableName(), "users");
  EXPECT_TRUE(stmt.isIfExists());

  // Test visitor
  stmt.accept(*visitor);
  EXPECT_EQ(visitor->dropIndexVisitCount, 1);
}

// ================ EXPRESSION TESTS ================

// Test IdentifierExpression
TEST_F(AstNodesComprehensiveTest, IdentifierExpressionTest) {
  IdentifierExpression expr("column_name");
  EXPECT_EQ(expr.getType(), Expression::IDENTIFIER);

  EXPECT_EQ(expr.getName(), "column_name");

  // Test name setting
  expr.setName("new_name");
  EXPECT_EQ(expr.getName(), "new_name");

  // Test visitor
  expr.accept(*visitor);
  EXPECT_EQ(visitor->identifierVisitCount, 1);
}

// Test StringLiteralExpression
TEST_F(AstNodesComprehensiveTest, StringLiteralExpressionTest) {
  StringLiteralExpression expr("hello world");
  EXPECT_EQ(expr.getType(), Expression::STRING_LITERAL);

  EXPECT_EQ(expr.getValue(), "hello world");

  // Test empty string
  StringLiteralExpression emptyExpr("");
  EXPECT_EQ(emptyExpr.getValue(), "");

  // Test visitor
  expr.accept(*visitor);
  EXPECT_EQ(visitor->stringLiteralVisitCount, 1);
}

// Test NumericLiteralExpression
TEST_F(AstNodesComprehensiveTest, NumericLiteralExpressionTest) {
  // Integer type
  NumericLiteralExpression intExpr(42, true);
  EXPECT_EQ(intExpr.getType(), Expression::NUMERIC_LITERAL);
  EXPECT_EQ(intExpr.getValue(), 42.0);
  EXPECT_TRUE(intExpr.isInteger());

  // Float type
  NumericLiteralExpression floatExpr(3.14159, false);
  EXPECT_EQ(floatExpr.getValue(), 3.14159);
  EXPECT_FALSE(floatExpr.isInteger());

  // Negative numbers
  NumericLiteralExpression negExpr(-100.5, false);
  EXPECT_EQ(negExpr.getValue(), -100.5);

  // Test visitor
  intExpr.accept(*visitor);
  EXPECT_EQ(visitor->numericLiteralVisitCount, 1);
}

// Test BinaryExpression
TEST_F(AstNodesComprehensiveTest, BinaryExpressionTest) {
  auto left = std::make_unique<NumericLiteralExpression>(10, true);
  auto right = std::make_unique<NumericLiteralExpression>(5, true);

  BinaryExpression expr(Token::Type::OPERATOR_PLUS, std::move(left),
                        std::move(right));
  EXPECT_EQ(expr.getType(), Expression::BINARY);
  EXPECT_EQ(expr.getOperator(), Token::Type::OPERATOR_PLUS);

  ASSERT_TRUE(expr.getLeft() != nullptr);
  ASSERT_TRUE(expr.getRight() != nullptr);

  // Test all binary operators
  std::vector<Token::Type> operators = {
      Token::Type::OPERATOR_PLUS,     Token::Type::OPERATOR_MINUS,
      Token::Type::OPERATOR_MULTIPLY, Token::Type::OPERATOR_DIVIDE,
      Token::Type::OPERATOR_EQUAL,    Token::Type::OPERATOR_NOT_EQUAL,
      Token::Type::OPERATOR_LESS,     Token::Type::OPERATOR_GREATER};

  for (auto op : operators) {
    auto leftOp = std::make_unique<IdentifierExpression>("a");
    auto rightOp = std::make_unique<IdentifierExpression>("b");
    BinaryExpression testExpr(op, std::move(leftOp), std::move(rightOp));
    EXPECT_EQ(testExpr.getOperator(), op);
  }

  // Test visitor
  expr.accept(*visitor);
  EXPECT_EQ(visitor->binaryVisitCount, 1);
}

// Test UnaryExpression
TEST_F(AstNodesComprehensiveTest, UnaryExpressionTest) {
  auto operand = std::make_unique<NumericLiteralExpression>(100, true);

  UnaryExpression expr(Token::Type::OPERATOR_MINUS, std::move(operand));
  EXPECT_EQ(expr.getType(), Expression::UNARY);
  EXPECT_EQ(expr.getOperator(), Token::Type::OPERATOR_MINUS);

  ASSERT_TRUE(expr.getOperand() != nullptr);

  // Test different unary operators
  std::vector<Token::Type> operators = {Token::Type::OPERATOR_PLUS,
                                        Token::Type::OPERATOR_MINUS,
                                        Token::Type::KEYWORD_NOT};

  for (auto op : operators) {
    auto testOperand = std::make_unique<IdentifierExpression>("value");
    UnaryExpression testExpr(op, std::move(testOperand));
    EXPECT_EQ(testExpr.getOperator(), op);
  }

  // Test visitor
  expr.accept(*visitor);
  EXPECT_EQ(visitor->unaryVisitCount, 1);
}

// Test FunctionExpression
TEST_F(AstNodesComprehensiveTest, FunctionExpressionTest) {
  FunctionExpression expr("COUNT");
  EXPECT_EQ(expr.getType(), Expression::FUNCTION);
  EXPECT_EQ(expr.getName(), "COUNT");

  // Add arguments
  auto arg1 = std::make_unique<IdentifierExpression>("*");
  expr.addArgument(std::move(arg1));

  auto arg2 = std::make_unique<StringLiteralExpression>("condition");
  expr.addArgument(std::move(arg2));

  EXPECT_EQ(expr.getArguments().size(), 2);

  // Test with multiple functions
  std::vector<std::string> functions = {"SUM", "AVG", "MAX", "MIN", "COUNT"};
  for (const auto &funcName : functions) {
    FunctionExpression funcExpr(funcName);
    EXPECT_EQ(funcExpr.getName(), funcName);
    EXPECT_EQ(funcExpr.getArguments().size(), 0);
  }

  // Test visitor
  expr.accept(*visitor);
  EXPECT_EQ(visitor->functionVisitCount, 1);
}

// Test SubqueryExpression types
TEST_F(AstNodesComprehensiveTest, SubqueryExpressionTest) {
  // Create a simple subquery
  auto subquery = std::make_unique<SelectStatement>();
  auto selectItem = std::make_unique<SelectItem>(
      std::make_unique<IdentifierExpression>("id"));
  subquery->addSelectItem(std::move(*selectItem));

  TableReference fromTable("users");
  subquery->addFromTable(fromTable);

  // Test ExistsExpression
  ExistsExpression existsExpr(std::move(subquery));
  EXPECT_EQ(existsExpr.getType(), Expression::EXISTS);
  EXPECT_EQ(existsExpr.getSubqueryType(), SubqueryExpression::EXISTS);
  ASSERT_TRUE(existsExpr.getSubquery() != nullptr);

  // Test visitor
  existsExpr.accept(*visitor);
  EXPECT_EQ(visitor->existsVisitCount, 1);
}

// Test InExpression
TEST_F(AstNodesComprehensiveTest, InExpressionTest) {
  auto leftExpr = std::make_unique<IdentifierExpression>("user_id");
  auto subquery = std::make_unique<SelectStatement>();
  auto selectItem = std::make_unique<SelectItem>(
      std::make_unique<IdentifierExpression>("id"));
  subquery->addSelectItem(std::move(*selectItem));

  TableReference fromTable("admins");
  subquery->addFromTable(fromTable);

  // Test IN expression
  InExpression inExpr(std::move(leftExpr), std::move(subquery), false);
  EXPECT_EQ(inExpr.getType(), Expression::IN);
  EXPECT_EQ(inExpr.getSubqueryType(), SubqueryExpression::IN);
  ASSERT_TRUE(inExpr.getLeftExpression() != nullptr);
  ASSERT_TRUE(inExpr.getSubquery() != nullptr);

  // Test NOT IN expression
  auto leftExpr2 = std::make_unique<IdentifierExpression>("status");
  auto subquery2 = std::make_unique<SelectStatement>();
  auto selectItem2 = std::make_unique<SelectItem>(
      std::make_unique<StringLiteralExpression>("active"));
  subquery2->addSelectItem(std::move(*selectItem2));

  InExpression notInExpr(std::move(leftExpr2), std::move(subquery2), true);
  EXPECT_EQ(notInExpr.getSubqueryType(), SubqueryExpression::NOT_IN);
}

// ================ CLAUSE TESTS ================

// Test WhereClause
TEST_F(AstNodesComprehensiveTest, WhereClauseTest) {
  auto condition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_GREATER,
      std::make_unique<IdentifierExpression>("age"),
      std::make_unique<NumericLiteralExpression>(18, true));

  WhereClause clause(std::move(condition));
  ASSERT_TRUE(clause.getCondition() != nullptr);

  // Test condition setting
  auto newCondition = std::make_unique<StringLiteralExpression>("active");
  clause.setCondition(std::move(newCondition));
  ASSERT_TRUE(clause.getCondition() != nullptr);

  // Test visitor
  clause.accept(*visitor);
  EXPECT_EQ(visitor->whereVisitCount, 1);
}

// Test JoinClause
TEST_F(AstNodesComprehensiveTest, JoinClauseTest) {
  TableReference table("orders");

  auto condition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL,
      std::make_unique<IdentifierExpression>("users.id"),
      std::make_unique<IdentifierExpression>("orders.user_id"));

  JoinClause join(JoinClause::Type::INNER, table, std::move(condition));
  EXPECT_EQ(join.getType(), JoinClause::Type::INNER);
  EXPECT_EQ(join.getTable().getName(), "orders");
  ASSERT_TRUE(join.getCondition() != nullptr);

  // Test different join types
  std::vector<JoinClause::Type> joinTypes = {
      JoinClause::Type::LEFT, JoinClause::Type::RIGHT, JoinClause::Type::FULL,
      JoinClause::Type::CROSS};

  for (auto type : joinTypes) {
    TableReference testTable("test");
    auto testCondition = std::make_unique<NumericLiteralExpression>(1, true);
    JoinClause testJoin(type, testTable, std::move(testCondition));
    EXPECT_EQ(testJoin.getType(), type);
  }

  // Test visitor
  join.accept(*visitor);
  EXPECT_EQ(visitor->joinVisitCount, 1);
}

// Test GroupByClause
TEST_F(AstNodesComprehensiveTest, GroupByClauseTest) {
  GroupByClause clause;

  // Add group by items
  auto expr1 = std::make_unique<IdentifierExpression>("department");
  clause.addGroupByItem(std::move(expr1));

  auto expr2 = std::make_unique<IdentifierExpression>("location");
  clause.addGroupByItem(std::move(expr2));

  EXPECT_EQ(clause.getGroupByItems().size(), 2);
  EXPECT_FALSE(clause.hasHaving());

  // Add HAVING condition
  auto havingCondition = std::make_unique<FunctionExpression>("COUNT");
  auto havingArg = std::make_unique<IdentifierExpression>("*");
  dynamic_cast<FunctionExpression *>(havingCondition.get())
      ->addArgument(std::move(havingArg));

  clause.setHavingCondition(std::move(havingCondition));
  EXPECT_TRUE(clause.hasHaving());
  ASSERT_TRUE(clause.getHavingCondition() != nullptr);

  // Test visitor
  clause.accept(*visitor);
  EXPECT_EQ(visitor->groupByVisitCount, 1);
}

// Test OrderByClause
TEST_F(AstNodesComprehensiveTest, OrderByClauseTest) {
  OrderByClause clause;

  // Add order by items
  auto expr1 = std::make_unique<IdentifierExpression>("salary");
  clause.addOrderByItem(std::move(expr1), OrderByClause::Direction::DESC);

  auto expr2 = std::make_unique<IdentifierExpression>("name");
  clause.addOrderByItem(std::move(expr2), OrderByClause::Direction::ASC);

  EXPECT_EQ(clause.getOrderByItems().size(), 2);

  // Test directions
  const auto &items = clause.getOrderByItems();
  ASSERT_GT(items.size(), 0);
  // Note: Direction testing would require accessing the private member directly
  // or through mocking

  // Test visitor
  clause.accept(*visitor);
  EXPECT_EQ(visitor->orderByVisitCount, 1);
}

// ================ EDGE CASE TESTS ================

// Test empty collections and boundary conditions
TEST_F(AstNodesComprehensiveTest, BoundaryConditionsTest) {
  // Empty TableReference
  TableReference emptyTable("");
  EXPECT_EQ(emptyTable.getName(), "");
  EXPECT_FALSE(emptyTable.hasAlias());

  // Empty ColumnDefinition
  ColumnDefinition emptyCol("", "");
  EXPECT_EQ(emptyCol.getName(), "");
  EXPECT_EQ(emptyCol.getType(), "");

  // SelectStatement with no items
  SelectStatement emptySelect;
  EXPECT_EQ(emptySelect.getSelectItems().size(), 0);
  EXPECT_EQ(emptySelect.getFromTables().size(), 0);
  EXPECT_FALSE(emptySelect.getWhereClause());
  EXPECT_EQ(emptySelect.getLimit(), 0);
  EXPECT_EQ(emptySelect.getOffset(), 0);

  // FunctionExpression with no arguments
  FunctionExpression emptyFunc("NOW");
  EXPECT_EQ(emptyFunc.getName(), "NOW");
  EXPECT_EQ(emptyFunc.getArguments().size(), 0);

  // GroupByClause with no items
  GroupByClause emptyGroupBy;
  EXPECT_EQ(emptyGroupBy.getGroupByItems().size(), 0);
  EXPECT_FALSE(emptyGroupBy.hasHaving());

  // OrderByClause with no items
  OrderByClause emptyOrderBy;
  EXPECT_EQ(emptyOrderBy.getOrderByItems().size(), 0);
}

// Test memory management and move semantics
TEST_F(AstNodesComprehensiveTest, MoveSemanticsTest) {
  // Test ColumnDefinition move constructor (defined as noexcept)
  ColumnDefinition col1("name", "VARCHAR(100)");
  ColumnDefinition col2(std::move(col1)); // Move constructor

  EXPECT_EQ(col2.getName(), "name");
  EXPECT_EQ(col2.getType(), "VARCHAR(100)");
}

// Test deep nesting scenarios
TEST_F(AstNodesComprehensiveTest, DeepNestingTest) {
  // Create deeply nested expressions
  auto innerExpr = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_PLUS, std::make_unique<NumericLiteralExpression>(1),
      std::make_unique<NumericLiteralExpression>(2));

  auto middleExpr = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_MULTIPLY, std::move(innerExpr),
      std::make_unique<NumericLiteralExpression>(3));

  auto outerExpr = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL,
      std::make_unique<IdentifierExpression>("result"), std::move(middleExpr));

  // Test that nested structure is maintained
  auto binaryExpr = dynamic_cast<BinaryExpression *>(outerExpr.get());
  ASSERT_TRUE(binaryExpr != nullptr);
  EXPECT_EQ(binaryExpr->getOperator(), Token::Type::OPERATOR_EQUAL);

  // Test that nested expressions are accessible
  auto rightExpr =
      dynamic_cast<BinaryExpression *>(binaryExpr->getRight().get());
  ASSERT_TRUE(rightExpr != nullptr);
  EXPECT_EQ(rightExpr->getOperator(), Token::Type::OPERATOR_MULTIPLY);
}

// ================ COMPREHENSIVE COVERAGE SCENARIO TESTS ================

// Test complete SQL statement parsing coverage
TEST_F(AstNodesComprehensiveTest, CompleteSQLScenarioTest) {
  // Create a complete complex SQL statement's AST representation
  SelectStatement select;

  // Set DISTINCT
  select.setDistinct(true);

  // Add complex select items
  auto countExpr = std::make_unique<FunctionExpression>("COUNT");
  countExpr->addArgument(std::make_unique<IdentifierExpression>("*"));
  SelectItem countItem(std::move(countExpr));
  countItem.setAlias("total_records");
  select.addSelectItem(std::move(countItem));

  auto avgExpr = std::make_unique<FunctionExpression>("AVG");
  avgExpr->addArgument(std::make_unique<IdentifierExpression>("salary"));
  SelectItem avgItem(std::move(avgExpr));
  avgItem.setAlias("avg_salary");
  select.addSelectItem(std::move(avgItem));

  // Add FROM tables
  TableReference mainTable("employees");
  mainTable.setAlias("e");
  select.addFromTable(mainTable);

  // Add complex WHERE clause
  auto salaryCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_GREATER_EQUAL,
      std::make_unique<IdentifierExpression>("e.salary"),
      std::make_unique<NumericLiteralExpression>(50000));

  auto deptCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL,
      std::make_unique<IdentifierExpression>("e.department"),
      std::make_unique<StringLiteralExpression>("IT"));

  auto complexCondition = std::make_unique<BinaryExpression>(
      Token::Type::KEYWORD_AND, std::move(salaryCondition),
      std::move(deptCondition));

  auto whereClause = std::make_unique<WhereClause>(std::move(complexCondition));
  select.setWhereClause(std::move(whereClause));

  // Add JOIN
  TableReference joinTable("departments");
  joinTable.setAlias("d");

  auto joinCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_EQUAL,
      std::make_unique<IdentifierExpression>("e.dept_id"),
      std::make_unique<IdentifierExpression>("d.id"));

  auto joinClause = std::make_unique<JoinClause>(
      JoinClause::Type::INNER, joinTable, std::move(joinCondition));
  select.addJoinClause(std::move(joinClause));

  // Add GROUP BY
  auto groupBy = std::make_unique<GroupByClause>();
  auto groupExpr = std::make_unique<IdentifierExpression>("e.department");
  groupBy->addGroupByItem(std::move(groupExpr));

  auto havingCondition = std::make_unique<BinaryExpression>(
      Token::Type::OPERATOR_GREATER,
      std::make_unique<FunctionExpression>("COUNT"),
      std::make_unique<NumericLiteralExpression>(5));

  groupBy->setHavingCondition(std::move(havingCondition));
  select.setGroupByClause(std::move(groupBy));

  // Add ORDER BY
  auto orderBy = std::make_unique<OrderByClause>();

  auto orderExpr1 = std::make_unique<IdentifierExpression>("avg_salary");
  orderBy->addOrderByItem(std::move(orderExpr1),
                          OrderByClause::Direction::DESC);

  auto orderExpr2 = std::make_unique<IdentifierExpression>("total_records");
  orderBy->addOrderByItem(std::move(orderExpr2), OrderByClause::Direction::ASC);

  select.setOrderByClause(std::move(orderBy));

  // Set LIMIT and OFFSET
  select.setLimit(100);
  select.setOffset(50);

  // Verify the complete structure
  EXPECT_TRUE(select.isDistinct());
  EXPECT_EQ(select.getSelectItems().size(), 2);
  EXPECT_EQ(select.getFromTables().size(), 1);
  EXPECT_TRUE(select.getWhereClause() != nullptr);
  EXPECT_EQ(select.getJoinClauses().size(), 1);
  EXPECT_TRUE(select.getGroupByClause() != nullptr);
  EXPECT_TRUE(select.getOrderByClause() != nullptr);
  EXPECT_EQ(select.getLimit(), 100);
  EXPECT_EQ(select.getOffset(), 50);

  // Test visitor accepts all components
  select.accept(*visitor);
  EXPECT_EQ(visitor->selectVisitCount, 1);
}

} // namespace sql_parser
} // namespace sqlcc
