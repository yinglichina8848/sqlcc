#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include "parser.h"

using namespace sqlcc::sql_parser;

// Test fixture for advanced query parser testing
class AdvancedQueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test parsing WITH clause (Common Table Expressions)
TEST_F(AdvancedQueryTest, ParseWithClause) {
    std::string sql = "WITH sales_summary AS (SELECT department, SUM(amount) as total FROM sales GROUP BY department) SELECT * FROM sales_summary WHERE total > 1000;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing recursive CTE
TEST_F(AdvancedQueryTest, ParseRecursiveCTE) {
    std::string sql = "WITH RECURSIVE employee_hierarchy AS (SELECT id, name, manager_id, 0 as level FROM employees WHERE manager_id IS NULL UNION ALL SELECT e.id, e.name, e.manager_id, eh.level + 1 FROM employees e JOIN employee_hierarchy eh ON e.manager_id = eh.id) SELECT * FROM employee_hierarchy ORDER BY level, name;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing multiple CTEs
TEST_F(AdvancedQueryTest, ParseMultipleCTEs) {
    std::string sql = "WITH dept_sales AS (SELECT department, SUM(amount) as total FROM sales GROUP BY department), top_depts AS (SELECT department FROM dept_sales WHERE total > 5000) SELECT * FROM top_depts;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing window functions with OVER clause
TEST_F(AdvancedQueryTest, ParseWindowFunctions) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary DESC) as rank, RANK() OVER (ORDER BY salary DESC) as dense_rank, DENSE_RANK() OVER (ORDER BY salary DESC) as dense_rank2 FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing window functions with PARTITION BY
TEST_F(AdvancedQueryTest, ParseWindowFunctionsPartitionBy) {
    std::string sql = "SELECT department, name, salary, AVG(salary) OVER (PARTITION BY department ORDER BY salary) as dept_avg FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing window functions with frame clauses
TEST_F(AdvancedQueryTest, ParseWindowFunctionsFrameClause) {
    std::string sql = "SELECT name, salary, SUM(salary) OVER (ORDER BY hire_date ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) as running_total FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex subqueries
TEST_F(AdvancedQueryTest, ParseComplexSubqueries) {
    std::string sql = "SELECT * FROM employees WHERE department_id IN (SELECT id FROM departments WHERE budget > (SELECT AVG(budget) FROM departments));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing correlated subqueries
TEST_F(AdvancedQueryTest, ParseCorrelatedSubqueries) {
    std::string sql = "SELECT e.name, e.salary FROM employees e WHERE e.salary > (SELECT AVG(salary) FROM employees WHERE department_id = e.department_id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing EXISTS subqueries
TEST_F(AdvancedQueryTest, ParseExistsSubqueries) {
    std::string sql = "SELECT department_name FROM departments d WHERE EXISTS (SELECT 1 FROM employees e WHERE e.department_id = d.id AND e.salary > 50000);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing NOT EXISTS subqueries
TEST_F(AdvancedQueryTest, ParseNotExistsSubqueries) {
    std::string sql = "SELECT department_name FROM departments d WHERE NOT EXISTS (SELECT 1 FROM employees e WHERE e.department_id = d.id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing scalar subqueries
TEST_F(AdvancedQueryTest, ParseScalarSubqueries) {
    std::string sql = "SELECT name, salary, (SELECT department_name FROM departments WHERE id = employees.department_id) as dept_name FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing derived tables (subqueries in FROM clause)
TEST_F(AdvancedQueryTest, ParseDerivedTables) {
    std::string sql = "SELECT dept_name, avg_salary FROM (SELECT department_id, AVG(salary) as avg_salary FROM employees GROUP BY department_id) e JOIN departments d ON e.department_id = d.id;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex JOIN operations
TEST_F(AdvancedQueryTest, ParseComplexJoins) {
    std::string sql = "SELECT e.name, d.department_name, m.name as manager_name FROM employees e INNER JOIN departments d ON e.department_id = d.id LEFT JOIN employees m ON e.manager_id = m.id;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing FULL OUTER JOIN
TEST_F(AdvancedQueryTest, ParseFullOuterJoin) {
    std::string sql = "SELECT e.name, d.department_name FROM employees e FULL OUTER JOIN departments d ON e.department_id = d.id;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing CROSS JOIN
TEST_F(AdvancedQueryTest, ParseCrossJoin) {
    std::string sql = "SELECT e.name, p.product_name FROM employees e CROSS JOIN products p;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing multiple JOIN conditions
TEST_F(AdvancedQueryTest, ParseMultipleJoinConditions) {
    std::string sql = "SELECT * FROM orders o JOIN customers c ON o.customer_id = c.id AND o.order_date >= c.registration_date;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing UNION operations
TEST_F(AdvancedQueryTest, ParseUnionOperations) {
    std::string sql = "SELECT name, 'Employee' as type FROM employees UNION SELECT department_name, 'Department' as type FROM departments;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing UNION ALL operations
TEST_F(AdvancedQueryTest, ParseUnionAllOperations) {
    std::string sql = "SELECT name FROM employees UNION ALL SELECT department_name FROM departments;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing INTERSECT operations
TEST_F(AdvancedQueryTest, ParseIntersectOperations) {
    std::string sql = "SELECT department_id FROM employees INTERSECT SELECT id FROM departments;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing EXCEPT operations
TEST_F(AdvancedQueryTest, ParseExceptOperations) {
    std::string sql = "SELECT department_id FROM employees EXCEPT SELECT id FROM departments WHERE budget < 10000;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing CASE expressions
TEST_F(AdvancedQueryTest, ParseCaseExpressions) {
    std::string sql = "SELECT name, salary, CASE WHEN salary > 50000 THEN 'High' WHEN salary > 30000 THEN 'Medium' ELSE 'Low' END as salary_level FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing searched CASE expressions
TEST_F(AdvancedQueryTest, ParseSearchedCaseExpressions) {
    std::string sql = "SELECT product_name, CASE WHEN price > 100 THEN 'Expensive' WHEN price > 50 THEN 'Moderate' ELSE 'Cheap' END as price_category FROM products;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex WHERE conditions with AND/OR
TEST_F(AdvancedQueryTest, ParseComplexWhereConditions) {
    std::string sql = "SELECT * FROM employees WHERE (department_id = 1 OR department_id = 2) AND (salary > 30000 OR manager_id IS NULL) AND (hire_date BETWEEN '2020-01-01' AND '2023-12-31');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing aggregate functions with HAVING
TEST_F(AdvancedQueryTest, ParseAggregateWithHaving) {
    std::string sql = "SELECT department_id, AVG(salary), COUNT(*) FROM employees GROUP BY department_id HAVING AVG(salary) > 40000 AND COUNT(*) > 5;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing ORDER BY with multiple columns and directions
TEST_F(AdvancedQueryTest, ParseOrderByMultipleColumns) {
    std::string sql = "SELECT * FROM employees ORDER BY department_id ASC, salary DESC, hire_date ASC;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing LIMIT and OFFSET
TEST_F(AdvancedQueryTest, ParseLimitOffset) {
    std::string sql = "SELECT * FROM employees ORDER BY salary DESC LIMIT 10 OFFSET 20;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex expressions with functions
TEST_F(AdvancedQueryTest, ParseComplexExpressionsWithFunctions) {
    std::string sql = "SELECT name, ROUND(salary * 1.1, 2) as increased_salary, UPPER(SUBSTRING(name, 1, 1)) || LOWER(SUBSTRING(name, 2)) as formatted_name FROM employees WHERE LENGTH(name) > 3;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing nested function calls
TEST_F(AdvancedQueryTest, ParseNestedFunctionCalls) {
    std::string sql = "SELECT COALESCE(UPPER(TRIM(name)), 'UNKNOWN') as clean_name FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing mathematical expressions
TEST_F(AdvancedQueryTest, ParseMathematicalExpressions) {
    std::string sql = "SELECT name, (salary + bonus) * (1 + tax_rate) - insurance as net_pay FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing string concatenation
TEST_F(AdvancedQueryTest, ParseStringConcatenation) {
    std::string sql = "SELECT first_name || ' ' || last_name as full_name FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing NULL handling
TEST_F(AdvancedQueryTest, ParseNullHandling) {
    std::string sql = "SELECT name, COALESCE(manager_id, -1) as mgr_id, NULLIF(department_id, 0) as dept_id FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing CAST expressions
TEST_F(AdvancedQueryTest, ParseCastExpressions) {
    std::string sql = "SELECT name, CAST(salary AS VARCHAR(20)) as salary_str, CAST(hire_date AS DATE) as hire_date_only FROM employees;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex queries with multiple CTEs and subqueries
TEST_F(AdvancedQueryTest, ParseComplexQueryMultipleCTEs) {
    std::string sql = "WITH dept_stats AS (SELECT department_id, COUNT(*) as emp_count, AVG(salary) as avg_salary FROM employees GROUP BY department_id), high_performers AS (SELECT * FROM employees WHERE salary > (SELECT AVG(salary) FROM employees)) SELECT ds.department_id, ds.emp_count, hp.name FROM dept_stats ds JOIN high_performers hp ON ds.department_id = hp.department_id;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing queries with window functions and CTEs
TEST_F(AdvancedQueryTest, ParseWindowFunctionsWithCTEs) {
    std::string sql = "WITH ranked_employees AS (SELECT name, department_id, salary, ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC) as dept_rank FROM employees) SELECT * FROM ranked_employees WHERE dept_rank <= 3;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing queries with recursive CTEs and joins
TEST_F(AdvancedQueryTest, ParseRecursiveCTEWithJoins) {
    std::string sql = "WITH RECURSIVE org_chart AS (SELECT id, name, manager_id, 1 as level FROM employees WHERE manager_id IS NULL UNION ALL SELECT e.id, e.name, e.manager_id, oc.level + 1 FROM employees e JOIN org_chart oc ON e.manager_id = oc.id) SELECT oc.name, oc.level, d.department_name FROM org_chart oc LEFT JOIN departments d ON oc.id = d.manager_id ORDER BY oc.level, oc.name;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex analytical queries
TEST_F(AdvancedQueryTest, ParseComplexAnalyticalQueries) {
    std::string sql = "SELECT department_id, name, salary, AVG(salary) OVER (PARTITION BY department_id) as dept_avg, salary - AVG(salary) OVER (PARTITION BY department_id) as difference_from_avg, RANK() OVER (ORDER BY salary DESC) as global_rank, RANK() OVER (PARTITION BY department_id ORDER BY salary DESC) as dept_rank FROM employees WHERE hire_date >= '2020-01-01' ORDER BY department_id, salary DESC;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing queries with multiple levels of nesting
TEST_F(AdvancedQueryTest, ParseMultipleLevelsOfNesting) {
    std::string sql = "SELECT * FROM (SELECT dept_id, AVG(salary) as avg_sal FROM (SELECT department_id as dept_id, salary FROM employees WHERE salary > (SELECT AVG(salary) FROM employees)) GROUP BY dept_id) WHERE avg_sal > 50000;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing queries with set operations and ordering
TEST_F(AdvancedQueryTest, ParseSetOperationsWithOrdering) {
    std::string sql = "(SELECT name, 'Manager' as role FROM employees WHERE id IN (SELECT DISTINCT manager_id FROM employees WHERE manager_id IS NOT NULL)) UNION (SELECT department_name, 'Department' as role FROM departments) ORDER BY role, name;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing very complex query with all features combined
TEST_F(AdvancedQueryTest, ParseVeryComplexQuery) {
    std::string sql = "WITH RECURSIVE employee_hierarchy AS (SELECT id, name, manager_id, salary, 0 as level FROM employees WHERE manager_id IS NULL UNION ALL SELECT e.id, e.name, e.manager_id, e.salary, eh.level + 1 FROM employees e JOIN employee_hierarchy eh ON e.manager_id = eh.id), dept_stats AS (SELECT department_id, COUNT(*) as emp_count, AVG(salary) as avg_salary, MAX(salary) as max_salary FROM employees GROUP BY department_id HAVING COUNT(*) > 2) SELECT eh.name, eh.level, eh.salary, ds.avg_salary, eh.salary - ds.avg_salary as diff_from_avg, ROW_NUMBER() OVER (PARTITION BY eh.level ORDER BY eh.salary DESC) as level_rank FROM employee_hierarchy eh JOIN dept_stats ds ON eh.id = ds.department_id WHERE eh.salary > (SELECT AVG(salary) FROM employees) ORDER BY eh.level, eh.salary DESC LIMIT 20;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}
