#include "sql_parser/parser.h"
#include "sql_parser/set_operation_node.h"
#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

class SetOperationParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的准备工作
    }
    
    void TearDown() override {
        // 测试后的清理工作
    }
    
    // 辅助方法：解析SQL并返回语句列表
    std::vector<std::unique_ptr<Statement>> parseSQL(const std::string& sql) {
        Parser parser(sql);
        return parser.parseStatements();
    }
    
    // 辅助方法：验证语句类型
    void expectStatementType(Statement* stmt, Statement::Type expectedType) {
        ASSERT_NE(stmt, nullptr);
        EXPECT_EQ(stmt->getType(), expectedType);
    }
};

// 测试1：基本UNION操作
TEST_F(SetOperationParserTest, ParseBasicUnion) {
    std::string sql = "SELECT id FROM table1 UNION SELECT id FROM table2";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    EXPECT_EQ(compositeStmt->getStatementCount(), 2);
    EXPECT_EQ(compositeStmt->getOperationCount(), 1);
    EXPECT_TRUE(compositeStmt->hasSetOperations());
    
    const auto& operations = compositeStmt->getSetOperations();
    ASSERT_EQ(operations.size(), 1);
    
    auto* setOp = operations[0].get();
    EXPECT_EQ(setOp->getOperationType(), SetOperationType::UNION);
    EXPECT_FALSE(setOp->isAll());
}

// 测试2：UNION ALL操作
TEST_F(SetOperationParserTest, ParseUnionAll) {
    std::string sql = "SELECT id FROM table1 UNION ALL SELECT id FROM table2";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    const auto& operations = compositeStmt->getSetOperations();
    ASSERT_EQ(operations.size(), 1);
    
    auto* setOp = operations[0].get();
    EXPECT_EQ(setOp->getOperationType(), SetOperationType::UNION);
    EXPECT_TRUE(setOp->isAll());
}

// 测试3：INTERSECT操作
TEST_F(SetOperationParserTest, ParseIntersect) {
    std::string sql = "SELECT id FROM table1 INTERSECT SELECT id FROM table2";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    const auto& operations = compositeStmt->getSetOperations();
    ASSERT_EQ(operations.size(), 1);
    
    auto* setOp = operations[0].get();
    EXPECT_EQ(setOp->getOperationType(), SetOperationType::INTERSECT);
    EXPECT_FALSE(setOp->isAll());
}

// 测试4：EXCEPT操作
TEST_F(SetOperationParserTest, ParseExcept) {
    std::string sql = "SELECT id FROM table1 EXCEPT SELECT id FROM table2";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    const auto& operations = compositeStmt->getSetOperations();
    ASSERT_EQ(operations.size(), 1);
    
    auto* setOp = operations[0].get();
    EXPECT_EQ(setOp->getOperationType(), SetOperationType::EXCEPT);
    EXPECT_FALSE(setOp->isAll());
}

// 测试5：多个集合操作
TEST_F(SetOperationParserTest, ParseMultipleSetOperations) {
    std::string sql = "SELECT id FROM table1 UNION SELECT id FROM table2 INTERSECT SELECT id FROM table3";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    EXPECT_EQ(compositeStmt->getStatementCount(), 3);
    EXPECT_EQ(compositeStmt->getOperationCount(), 2);
    
    const auto& operations = compositeStmt->getSetOperations();
    ASSERT_EQ(operations.size(), 2);
    
    // 检查第一个操作（UNION）
    EXPECT_EQ(operations[0]->getOperationType(), SetOperationType::UNION);
    
    // 检查第二个操作（INTERSECT）
    EXPECT_EQ(operations[1]->getOperationType(), SetOperationType::INTERSECT);
}

// 测试6：简单SELECT语句（无集合操作）
TEST_F(SetOperationParserTest, ParseSimpleSelect) {
    std::string sql = "SELECT id FROM table1";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::SELECT);
    
    auto* selectStmt = dynamic_cast<SelectStatement*>(stmt);
    ASSERT_NE(selectStmt, nullptr);
    
    EXPECT_EQ(selectStmt->getTableName(), "table1");
}

// 测试7：带WHERE子句的集合操作
TEST_F(SetOperationParserTest, ParseSetOperationWithWhereClause) {
    std::string sql = "SELECT id FROM table1 WHERE id > 10 UNION SELECT id FROM table2 WHERE id < 20";
    auto statements = parseSQL(sql);
    
    ASSERT_EQ(statements.size(), 1);
    
    auto* stmt = statements[0].get();
    expectStatementType(stmt, Statement::COMPOSITE_SELECT);
    
    auto* compositeStmt = dynamic_cast<CompositeSelectStatement*>(stmt);
    ASSERT_NE(compositeStmt, nullptr);
    
    const auto& selectStmts = compositeStmt->getSelectStatements();
    ASSERT_EQ(selectStmts.size(), 2);
    
    // 检查第一个SELECT语句的WHERE子句
    auto* firstSelect = selectStmts[0].get();
    ASSERT_NE(firstSelect, nullptr);
    EXPECT_TRUE(firstSelect->hasWhereClause());
    
    // 检查第二个SELECT语句的WHERE子句
    auto* secondSelect = selectStmts[1].get();
    ASSERT_NE(secondSelect, nullptr);
    EXPECT_TRUE(secondSelect->hasWhereClause());
}

// 测试8：语法错误处理
TEST_F(SetOperationParserTest, ParseSyntaxError) {
    std::string sql = "SELECT id FROM table1 UNION";
    
    EXPECT_THROW({
        auto statements = parseSQL(sql);
    }, std::runtime_error);
}

// 测试9：无效的集合操作关键字
TEST_F(SetOperationParserTest, ParseInvalidSetOperation) {
    std::string sql = "SELECT id FROM table1 MERGE SELECT id FROM table2";
    
    EXPECT_THROW({
        auto statements = parseSQL(sql);
    }, std::runtime_error);
}

} // namespace sql_parser
} // namespace sqlcc