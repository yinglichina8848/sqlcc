#include "sql_parser/ast_node.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "sql_parser/parser.h"

namespace sqlcc {
namespace sql_parser {

// Complex Subquery Testing for Layer 4 Coverage Improvement
// Focuses on complex subquery parsing scenarios and edge cases
class ComplexSubqueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
    }

    void TearDown() override {
        // Cleanup
    }

    // Helper methods for subquery testing
    std::unique_ptr<Statement> parseAndValidate(const std::string& sql) {
        Parser parser(sql);
        auto statements = parser.parse();
        if (statements.empty()) {
            return nullptr;
        }
        return std::move(statements[0]);
    }

    bool hasExpectedAstStructure(const Statement* stmt, Statement::Type type) {
        return stmt != nullptr && stmt->getType() == type;
    }
};

// =============================================================================
// NESTED SUBQUERY TESTS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseTripleNestedSubquery) {
    std::string sql = R"(
        SELECT employee_name, salary 
        FROM employees 
        WHERE salary > (
            SELECT AVG(salary) 
            FROM employees 
            WHERE department_id IN (
                SELECT department_id 
                FROM departments 
                WHERE location = 'New York'
            )
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseFourLevelNestedSubquery) {
    std::string sql = R"(
        SELECT name
        FROM products
        WHERE category_id IN (
            SELECT category_id
            FROM categories
            WHERE parent_id IN (
                SELECT parent_id
                FROM category_hierarchy
                WHERE level IN (
                    SELECT level
                    FROM levels
                    WHERE level_name = 'electronics'
                )
            )
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// CORRELATED SUBQUERY TESTS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseCorrelatedSubquery) {
    std::string sql = R"(
        SELECT e1.employee_id, e1.employee_name
        FROM employees e1
        WHERE EXISTS (
            SELECT 1 
            FROM salaries s
            WHERE s.employee_id = e1.employee_id 
            AND s.amount > 50000
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseCorrelatedSubqueryInSelect) {
    std::string sql = R"(
        SELECT 
            e.employee_name,
            (SELECT AVG(s.amount) 
             FROM salaries s 
             WHERE s.employee_id = e.employee_id) as avg_salary,
            (SELECT COUNT(*) 
             FROM projects p 
             WHERE p.employee_id = e.employee_id) as project_count
        FROM employees e
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseCorrelatedSubqueryWithAggregation) {
    std::string sql = R"(
        SELECT department_id, department_name
        FROM departments d
        WHERE d.department_id IN (
            SELECT e.department_id
            FROM employees e
            WHERE e.salary > (
                SELECT AVG(e2.salary)
                FROM employees e2
                WHERE e2.department_id = d.department_id
            )
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// SUBQUERY IN DIFFERENT CLAUSES
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSubqueryInWhereClause) {
    std::string sql = R"(
        SELECT employee_name, salary
        FROM employees
        WHERE salary > (SELECT AVG(salary) FROM employees WHERE department = 'Engineering')
        AND department IN (SELECT department FROM departments WHERE budget > 100000)
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryInHavingClause) {
    std::string sql = R"(
        SELECT department_id, AVG(salary) as avg_salary
        FROM employees
        GROUP BY department_id
        HAVING AVG(salary) > (
            SELECT AVG(salary) 
            FROM employees 
            WHERE hire_date > '2020-01-01'
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryInFromClause) {
    std::string sql = R"(
        SELECT avg_stats.department, avg_stats.avg_salary
        FROM (
            SELECT department, AVG(salary) as avg_salary
            FROM employees
            WHERE active = 1
            GROUP BY department
        ) avg_stats
        WHERE avg_stats.avg_salary > (SELECT AVG(salary) FROM employees)
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryInSelectClause) {
    std::string sql = R"(
        SELECT 
            employee_name,
            salary,
            (SELECT department_name FROM departments WHERE department_id = e.department_id) as dept_name,
            (SELECT COUNT(*) FROM projects WHERE employee_id = e.employee_id) as project_count
        FROM employees e
        ORDER BY salary DESC
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// SUBQUERY WITH DIFFERENT OPERATORS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSubqueryWithInOperator) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees
        WHERE department_id IN (
            SELECT department_id
            FROM departments
            WHERE location IN ('New York', 'Los Angeles', 'Chicago')
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithExistsOperator) {
    std::string sql = R"(
        SELECT customer_name
        FROM customers c
        WHERE EXISTS (
            SELECT 1
            FROM orders o
            WHERE o.customer_id = c.customer_id
            AND o.total > 1000
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithNotExistsOperator) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees e
        WHERE NOT EXISTS (
            SELECT 1
            FROM performance_reviews pr
            WHERE pr.employee_id = e.employee_id
            AND pr.rating < 3
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithAllOperator) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees
        WHERE salary > ALL (
            SELECT salary
            FROM employees
            WHERE department = 'Intern'
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithAnyOperator) {
    std::string sql = R"(
        SELECT product_name
        FROM products
        WHERE price > ANY (
            SELECT price
            FROM products
            WHERE category = 'Electronics'
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// SUBQUERY WITH JOINS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSubqueryWithInnerJoin) {
    std::string sql = R"(
        SELECT e.employee_name, d.department_name
        FROM employees e
        INNER JOIN (
            SELECT department_id, department_name 
            FROM departments 
            WHERE location = 'New York'
        ) d ON e.department_id = d.department_id
        WHERE e.employee_id IN (
            SELECT employee_id 
            FROM high_performers 
            WHERE rating > 4.5
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithLeftJoin) {
    std::string sql = R"(
        SELECT c.customer_name, o.order_count
        FROM customers c
        LEFT JOIN (
            SELECT customer_id, COUNT(*) as order_count
            FROM orders
            WHERE order_date >= '2023-01-01'
            GROUP BY customer_id
        ) o ON c.customer_id = o.customer_id
        WHERE c.customer_id IN (
            SELECT customer_id FROM premium_customers
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithMultipleJoins) {
    std::string sql = R"(
        SELECT e.name, p.project_name, d.department_name
        FROM employees e
        JOIN (
            SELECT employee_id, project_id
            FROM employee_projects
            WHERE assignment_date >= '2023-01-01'
        ) ep ON e.employee_id = ep.employee_id
        JOIN projects p ON ep.project_id = p.project_id
        JOIN (
            SELECT department_id, department_name
            FROM departments
            WHERE active = 1
        ) d ON e.department_id = d.department_id
        WHERE e.employee_id IN (
            SELECT employee_id FROM managers
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// SUBQUERY IN DML STATEMENTS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSubqueryInInsertStatement) {
    std::string sql = R"(
        INSERT INTO employee_backup (employee_id, employee_name, salary)
        SELECT employee_id, employee_name, salary
        FROM employees
        WHERE department_id IN (
            SELECT department_id 
            FROM departments 
            WHERE budget > 1000000
        )
        AND salary > (SELECT AVG(salary) FROM employees)
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::INSERT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryInUpdateStatement) {
    std::string sql = R"(
        UPDATE products
        SET price = price * 1.1
        WHERE category_id IN (
            SELECT category_id 
            FROM categories 
            WHERE active = 1
        )
        AND product_id IN (
            SELECT product_id 
            FROM top_selling_products
            WHERE sales_volume > 1000
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::UPDATE));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryInDeleteStatement) {
    std::string sql = R"(
        DELETE FROM temp_orders
        WHERE order_id IN (
            SELECT order_id
            FROM orders
            WHERE status = 'cancelled'
            AND order_date < '2023-01-01'
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::DELETE));
}

// =============================================================================
// COMMON TABLE EXPRESSIONS (CTE) WITH SUBQUERIES
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSimpleCTE) {
    std::string sql = R"(
        WITH high_salary_employees AS (
            SELECT employee_id, employee_name, salary
            FROM employees
            WHERE salary > 100000
        )
        SELECT employee_name, salary
        FROM high_salary_employees
        WHERE employee_id IN (
            SELECT employee_id FROM managers
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseRecursiveCTE) {
    std::string sql = R"(
        WITH RECURSIVE employee_hierarchy AS (
            SELECT employee_id, manager_id, employee_name, 0 as level
            FROM employees
            WHERE manager_id IS NULL
            
            UNION ALL
            
            SELECT e.employee_id, e.manager_id, e.employee_name, eh.level + 1
            FROM employees e
            INNER JOIN employee_hierarchy eh ON e.manager_id = eh.employee_id
            WHERE e.employee_id IN (
                SELECT employee_id FROM active_employees
            )
        )
        SELECT employee_name, level
        FROM employee_hierarchy
        WHERE level <= 3
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseMultipleCTEs) {
    std::string sql = R"(
        WITH 
        dept_stats AS (
            SELECT department_id, AVG(salary) as avg_salary
            FROM employees
            GROUP BY department_id
        ),
        high_salary_employees AS (
            SELECT employee_id, employee_name, salary
            FROM employees
            WHERE salary > (SELECT AVG(avg_salary) FROM dept_stats)
        )
        SELECT hse.employee_name, hse.salary, ds.avg_salary
        FROM high_salary_employees hse
        JOIN dept_stats ds ON hse.employee_id = ds.department_id
        WHERE hse.employee_id IN (
            SELECT employee_id FROM department_managers
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// SCALAR SUBQUERY TESTS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseScalarSubqueryInSelect) {
    std::string sql = R"(
        SELECT 
            employee_name, 
            salary,
            (SELECT department_name FROM departments WHERE department_id = e.department_id) as dept_name,
            (SELECT COUNT(*) FROM projects WHERE employee_id = e.employee_id) as project_count
        FROM employees e
        ORDER BY salary DESC
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseScalarSubqueryInWhere) {
    std::string sql = R"(
        SELECT employee_name, salary
        FROM employees
        WHERE salary = (
            SELECT MAX(salary)
            FROM employees
            WHERE department = (SELECT department FROM employees WHERE employee_id = 1)
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseScalarSubqueryInOrderBy) {
    std::string sql = R"(
        SELECT employee_name, salary
        FROM employees
        ORDER BY (
            SELECT AVG(salary)
            FROM employees
            WHERE department = employees.department
        ) DESC
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseEmptySubquery) {
    std::string sql = R"(
        SELECT * FROM (SELECT 1) AS subquery
        WHERE id IN (SELECT id FROM non_existent_table)
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(stmt != nullptr);
}

TEST_F(ComplexSubqueryTest, ParseSingleColumnSubquery) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees
        WHERE employee_id = (SELECT employee_id FROM departments LIMIT 1)
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithAggregateFunctions) {
    std::string sql = R"(
        SELECT department_id
        FROM employees
        WHERE salary > (
            SELECT AVG(MAX(salary))
            FROM (SELECT salary FROM employees GROUP BY department) dept_salaries
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithUnion) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees
        WHERE employee_id IN (
            SELECT employee_id FROM (
                SELECT employee_id FROM full_time_employees
                UNION
                SELECT employee_id FROM part_time_employees
            ) all_employees
        )
    )";
    
    auto stmt = parseAndValidate(sql);
    ASSERT_TRUE(hasExpectedAstStructure(stmt.get(), Statement::SELECT));
}

// =============================================================================
// ERROR HANDLING AND VALIDATION
// =============================================================================

TEST_F(ComplexSubqueryTest, ParseSubqueryWithMismatchedColumns) {
    std::string sql = R"(
        SELECT employee_name
        FROM employees
        WHERE (employee_id, name) = (SELECT employee_id, salary FROM departments)
    )";
    
    auto stmt = parseAndValidate(sql);
    // Should handle gracefully (might return success with warning)
    ASSERT_TRUE(stmt != nullptr);
}

TEST_F(ComplexSubqueryTest, ParseSubqueryWithUnmatchedParentheses) {
    std::string sql = "SELECT * FROM employees WHERE id = (SELECT id FROM
