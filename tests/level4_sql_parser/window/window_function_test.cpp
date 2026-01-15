#include "sql_parser/ast_node.h"
#include "sql_parser/parser.h"
#include "sql_parser/window_function.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

class WindowFunctionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化测试数据
    }

    void TearDown() override {
        // 清理资源
    }

    std::unique_ptr<Statement> parseStatement(const std::string& sql) {
        Parser parser(sql);
        auto statements = parser.parse();
        if (statements.empty()) {
            return nullptr;
        }
        return std::move(statements[0]);
    }
};

// 测试基本窗口函数解析
TEST_F(WindowFunctionTest, BasicWindowFunctionTest) {
    std::string sql = "SELECT id, name, ROW_NUMBER() OVER () as row_num FROM users;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
    
    // 这里需要根据实际的AST结构来检查窗口函数
    // 测试将在实际实现后完善
}

// 测试带分区的窗口函数解析
TEST_F(WindowFunctionTest, WindowFunctionWithPartitionTest) {
    std::string sql = "SELECT department, name, salary, AVG(salary) OVER (PARTITION BY department) as avg_salary FROM employees;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试带排序的窗口函数解析
TEST_F(WindowFunctionTest, WindowFunctionWithOrderTest) {
    std::string sql = "SELECT id, name, salary, RANK() OVER (ORDER BY salary DESC) as rank FROM employees;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试带分区和排序的窗口函数解析
TEST_F(WindowFunctionTest, WindowFunctionWithPartitionAndOrderTest) {
    std::string sql = "SELECT department, name, salary, ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as dept_rank FROM employees;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试带窗口框架的窗口函数解析
TEST_F(WindowFunctionTest, WindowFunctionWithFrameTest) {
    std::string sql = "SELECT id, value, AVG(value) OVER (ORDER BY id ROWS BETWEEN 2 PRECEDING AND CURRENT ROW) as moving_avg FROM data;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试命名窗口解析
TEST_F(WindowFunctionTest, NamedWindowTest) {
    std::string sql = "SELECT department, name, salary, "
                     "       ROW_NUMBER() OVER w as dept_rank, "
                     "       AVG(salary) OVER w as avg_salary "
                     "FROM employees "
                     "WINDOW w AS (PARTITION BY department ORDER BY salary DESC);";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试多个不同类型的窗口函数解析
TEST_F(WindowFunctionTest, MultipleWindowFunctionsTest) {
    std::string sql = "SELECT department, name, salary, "
                     "       ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as row_num, "
                     "       RANK() OVER (PARTITION BY department ORDER BY salary DESC) as rank, "
                     "       DENSE_RANK() OVER (PARTITION BY department ORDER BY salary DESC) as dense_rank, "
                     "       SUM(salary) OVER (PARTITION BY department) as total_salary "
                     "FROM employees;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试复杂窗口框架解析
TEST_F(WindowFunctionTest, ComplexWindowFrameTest) {
    std::string sql = "SELECT id, value, "
                     "       AVG(value) OVER (ORDER BY id ROWS BETWEEN 3 PRECEDING AND 1 FOLLOWING) as moving_avg, "
                     "       SUM(value) OVER (ORDER BY id RANGE BETWEEN CURRENT ROW AND 5 FOLLOWING) as range_sum "
                     "FROM data;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试窗口函数与聚合函数结合使用
TEST_F(WindowFunctionTest, WindowFunctionWithAggregationTest) {
    std::string sql = "SELECT department, "
                     "       COUNT(*) as total_employees, "
                     "       AVG(salary) as avg_dept_salary, "
                     "       MAX(salary) - MIN(salary) as salary_range, "
                     "       AVG(salary) OVER () as overall_avg_salary "
                     "FROM employees "
                     "GROUP BY department;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

// 测试窗口函数在WHERE子句中的使用（通过子查询）
TEST_F(WindowFunctionTest, WindowFunctionInSubqueryTest) {
    std::string sql = "SELECT * FROM ("
                     "    SELECT id, name, salary, ROW_NUMBER() OVER (ORDER BY salary DESC) as rank FROM employees"
                     ") sub WHERE sub.rank <= 10;";
    
    auto stmt = parseStatement(sql);
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->getType(), Statement::SELECT);
}

} // namespace sql_parser
} // namespace sqlcc