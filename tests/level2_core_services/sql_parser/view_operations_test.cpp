#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"
#include "core/core_database_manager.h"

// 测试ALTER VIEW和DROP VIEW语句的解析和执行

using namespace sqlcc;
using namespace sql_parser;

class ViewOperationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>("/tmp/test_db", 1024, 1, 1);
        db_manager_->CreateDatabase("test_db");
        db_manager_->UseDatabase("test_db");

        // 创建测试表
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT"},
            {"name", "VARCHAR(50)"},
            {"salary", "DECIMAL(10,2)"}
        };
        db_manager_->CreateTable("employees", columns);

        // 创建测试视图
        CreateViewStatement create_stmt("employee_view");
        create_stmt.addColumnName("emp_id");
        create_stmt.addColumnName("emp_name");

        // 创建SELECT语句
        auto select_stmt = std::make_unique<SelectStatement>();
        select_stmt->addSelectColumn("id");
        select_stmt->addSelectColumn("name");
        select_stmt->setTableName("employees");

        create_stmt.setSelectStatement(std::move(select_stmt));

        // 解析并执行CREATE VIEW
        parser_ = std::make_unique<Parser>("CREATE VIEW employee_view (emp_id, emp_name) AS SELECT id, name FROM employees;");
        auto statements = parser_->parse();
        if (!statements.empty() && statements[0]) {
            // 简化测试：只验证解析成功
            SUCCEED() << "CREATE VIEW statement parsed successfully";
        }
    }

    void TearDown() override {
        // 清理测试数据
        if (db_manager_) {
            db_manager_->Close();
        }
    }

    std::shared_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<Parser> parser_;
};

// 测试ALTER VIEW语句解析
TEST_F(ViewOperationsTest, AlterViewStatementParsing) {
    // 测试ALTER VIEW语句的解析
    Parser parser("ALTER VIEW employee_view AS SELECT id, name, salary FROM employees;");
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    auto alter_stmt = dynamic_cast<AlterViewStatement*>(statements[0].get());
    ASSERT_TRUE(alter_stmt != nullptr);

    EXPECT_EQ(alter_stmt->getViewName(), "employee_view");
    EXPECT_FALSE(alter_stmt->hasColumnNames()); // 这个语句没有指定列名

    // 验证SELECT语句
    const auto& select_stmt = alter_stmt->getSelectStatement();
    EXPECT_EQ(select_stmt.getTableName(), "employees");

    const auto& select_columns = select_stmt.getSelectColumns();
    ASSERT_EQ(select_columns.size(), 3);
    EXPECT_EQ(select_columns[0], "id");
    EXPECT_EQ(select_columns[1], "name");
    EXPECT_EQ(select_columns[2], "salary");
}

// 测试ALTER VIEW语句带列名的解析
TEST_F(ViewOperationsTest, AlterViewStatementWithColumnsParsing) {
    // 测试ALTER VIEW语句带列名列表的解析
    Parser parser("ALTER VIEW employee_view (id, name, salary) AS SELECT id, name, salary FROM employees;");
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    auto alter_stmt = dynamic_cast<AlterViewStatement*>(statements[0].get());
    ASSERT_TRUE(alter_stmt != nullptr);

    EXPECT_EQ(alter_stmt->getViewName(), "employee_view");
    EXPECT_TRUE(alter_stmt->hasColumnNames());

    const auto& column_names = alter_stmt->getColumnNames();
    ASSERT_EQ(column_names.size(), 3);
    EXPECT_EQ(column_names[0], "id");
    EXPECT_EQ(column_names[1], "name");
    EXPECT_EQ(column_names[2], "salary");
}

// 测试DROP VIEW语句解析
TEST_F(ViewOperationsTest, DropViewStatementParsing) {
    // 测试DROP VIEW语句的解析
    Parser parser("DROP VIEW employee_view;");
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    auto drop_stmt = dynamic_cast<DropViewStatement*>(statements[0].get());
    ASSERT_TRUE(drop_stmt != nullptr);

    EXPECT_EQ(drop_stmt->getViewName(), "employee_view");
    EXPECT_EQ(drop_stmt->getDropBehavior(), DropViewStatement::DropBehavior::RESTRICT);
    EXPECT_FALSE(drop_stmt->isIfExists());
}

// 测试DROP VIEW IF EXISTS语句解析
TEST_F(ViewOperationsTest, DropViewIfExistsStatementParsing) {
    // 测试DROP VIEW IF EXISTS语句的解析
    Parser parser("DROP VIEW IF EXISTS employee_view;");
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    auto drop_stmt = dynamic_cast<DropViewStatement*>(statements[0].get());
    ASSERT_TRUE(drop_stmt != nullptr);

    EXPECT_EQ(drop_stmt->getViewName(), "employee_view");
    EXPECT_TRUE(drop_stmt->isIfExists());
}

// 测试DROP VIEW CASCADE语句解析
TEST_F(ViewOperationsTest, DropViewCascadeStatementParsing) {
    // 测试DROP VIEW CASCADE语句的解析
    Parser parser("DROP VIEW employee_view CASCADE;");
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    auto drop_stmt = dynamic_cast<DropViewStatement*>(statements[0].get());
    ASSERT_TRUE(drop_stmt != nullptr);

    EXPECT_EQ(drop_stmt->getViewName(), "employee_view");
    EXPECT_EQ(drop_stmt->getDropBehavior(), DropViewStatement::DropBehavior::CASCADE);
}

// 测试多个语句的解析
TEST_F(ViewOperationsTest, MultipleViewStatementsParsing) {
    // 测试多个视图操作语句的解析
    std::string sql = R"(
        CREATE VIEW v1 AS SELECT id FROM employees;
        ALTER VIEW v1 AS SELECT id, name FROM employees;
        DROP VIEW v1;
    )";

    Parser parser(sql);
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 3);

    // 第一个语句：CREATE VIEW
    auto create_stmt = dynamic_cast<CreateViewStatement*>(statements[0].get());
    ASSERT_TRUE(create_stmt != nullptr);
    EXPECT_EQ(create_stmt->getViewName(), "v1");

    // 第二个语句：ALTER VIEW
    auto alter_stmt = dynamic_cast<AlterViewStatement*>(statements[1].get());
    ASSERT_TRUE(alter_stmt != nullptr);
    EXPECT_EQ(alter_stmt->getViewName(), "v1");

    // 第三个语句：DROP VIEW
    auto drop_stmt = dynamic_cast<DropViewStatement*>(statements[2].get());
    ASSERT_TRUE(drop_stmt != nullptr);
    EXPECT_EQ(drop_stmt->getViewName(), "v1");
}

// 测试复杂SELECT语句在视图中的使用
TEST_F(ViewOperationsTest, ComplexSelectInViewParsing) {
    std::string sql = R"(
        ALTER VIEW employee_view AS
        SELECT e.id, e.name, e.salary, d.name as dept_name
        FROM employees e
        LEFT JOIN departments d ON e.dept_id = d.id
        WHERE e.salary > 50000
        ORDER BY e.salary DESC;
    )";

    Parser parser(sql);
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    auto alter_stmt = dynamic_cast<AlterViewStatement*>(statements[0].get());
    ASSERT_TRUE(alter_stmt != nullptr);

    const auto& select_stmt = alter_stmt->getSelectStatement();
    EXPECT_EQ(select_stmt.getTableName(), "employees");

    // 验证JOIN子句
    const auto& join_clauses = select_stmt.getJoinClauses();
    ASSERT_EQ(join_clauses.size(), 1);

    // 验证WHERE子句存在
    EXPECT_TRUE(select_stmt.hasWhereClause());

    // 验证ORDER BY子句
    EXPECT_TRUE(select_stmt.hasOrderBy());
}

// 测试错误处理
TEST_F(ViewOperationsTest, ErrorHandling) {
    // 测试语法错误
    Parser parser("ALTER VIEW;"); // 缺少视图名
    auto statements = parser.parse();

    // 解析器应该能够处理错误并继续
    EXPECT_TRUE(parser.hadError() || statements.empty());
}

// 测试边界情况
TEST_F(ViewOperationsTest, EdgeCases) {
    // 测试视图名带引号
    Parser parser1("ALTER VIEW \"my_view\" AS SELECT * FROM table1;");
    auto statements1 = parser1.parse();
    ASSERT_EQ(statements1.size(), 1);

    // 测试视图名带反引号
    Parser parser2("ALTER VIEW `my_view` AS SELECT * FROM table1;");
    auto statements2 = parser2.parse();
    ASSERT_EQ(statements2.size(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
