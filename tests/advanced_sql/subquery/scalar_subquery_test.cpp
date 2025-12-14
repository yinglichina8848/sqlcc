#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "database_manager.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"
#include "storage_engine.h"

using namespace sqlcc;

class ScalarSubqueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>();
        db_manager->Initialize();
    }
    
    void TearDown() override {
        // Clean up test database
        db_manager->Close();
        db_manager.reset();
    }
    
    void CreateTestTables() {
        // Create employee table
        std::string create_employee_sql = 
            "CREATE TABLE employee ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "department_id INT,"
            "salary FLOAT"
            ")";
        
        ASSERT_TRUE(db_manager->Execute(create_employee_sql));
        
        // Create department table
        std::string create_department_sql = 
            "CREATE TABLE department ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "location VARCHAR(100)"
            ")";
        
        ASSERT_TRUE(db_manager->Execute(create_department_sql));
        
        // Insert test data into employee table
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0)",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 2, 60000.0)",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0)",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 3, 70000.0)",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0)"
        };
        
        for (const auto& sql : employee_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
        
        // Insert test data into department table
        std::vector<std::string> department_inserts = {
            "INSERT INTO department VALUES (1, 'Engineering', 'Building A')",
            "INSERT INTO department VALUES (2, 'Marketing', 'Building B')",
            "INSERT INTO department VALUES (3, 'Sales', 'Building C')"
        };
        
        for (const auto& sql : department_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
    }
    
    std::unique_ptr<DatabaseManager> db_manager;
};

TEST_F(ScalarSubqueryTest, BasicScalarSubquery) {
    // Create test tables
    CreateTestTables();
    
    // Test basic scalar subquery in SELECT clause
    std::string sql = 
        "SELECT name, (SELECT name FROM department WHERE id = 1) as dept_name "
        "FROM employee "
        "WHERE department_id = 1";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInWhere) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery in WHERE clause
    std::string sql = 
        "SELECT name, salary "
        "FROM employee "
        "WHERE department_id = (SELECT id FROM department WHERE name = 'Marketing')";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInOrderBy) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery in ORDER BY clause
    std::string sql = 
        "SELECT name, salary "
        "FROM employee "
        "ORDER BY (SELECT AVG(salary) FROM employee) - salary";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, CorrelatedScalarSubquery) {
    // Create test tables
    CreateTestTables();
    
    // Test correlated scalar subquery
    std::string sql = 
        "SELECT e1.name, e1.salary, "
        "(SELECT AVG(e2.salary) FROM employee e2 WHERE e2.department_id = e1.department_id) as dept_avg "
        "FROM employee e1 "
        "ORDER BY e1.name";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryWithAggregate) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery with aggregate function
    std::string sql = 
        "SELECT d.name, "
        "(SELECT AVG(e.salary) FROM employee e WHERE e.department_id = d.id) as avg_salary, "
        "(SELECT COUNT(e.id) FROM employee e WHERE e.department_id = d.id) as employee_count "
        "FROM department d";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, NestedScalarSubquery) {
    // Create test tables
    CreateTestTables();
    
    // Test nested scalar subquery
    std::string sql = 
        "SELECT name, salary "
        "FROM employee "
        "WHERE salary > (SELECT AVG(salary) FROM employee WHERE department_id = "
        "(SELECT id FROM department WHERE name = 'Engineering'))";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInHaving) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery in HAVING clause
    std::string sql = 
        "SELECT d.name, COUNT(e.id) as employee_count "
        "FROM department d LEFT JOIN employee e ON d.id = e.department_id "
        "GROUP BY d.id, d.name "
        "HAVING COUNT(e.id) > (SELECT AVG(emp_count) FROM "
        "(SELECT COUNT(e2.id) as emp_count FROM department d2 LEFT JOIN employee e2 ON d2.id = e2.department_id GROUP BY d2.id) sub)";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryWithCase) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery with CASE expression
    std::string sql = 
        "SELECT name, salary, "
        "CASE "
        "  WHEN salary > (SELECT AVG(salary) FROM employee) THEN 'Above Average' "
        "  ELSE 'Below Average' "
        "END as salary_status "
        "FROM employee";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInUpdate) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery in UPDATE statement
    std::string sql = 
        "UPDATE employee SET salary = salary * 1.1 "
        "WHERE department_id = (SELECT id FROM department WHERE name = 'Sales')";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInDelete) {
    // Create test tables
    CreateTestTables();
    
    // Test scalar subquery in DELETE statement
    std::string sql = 
        "DELETE FROM employee "
        "WHERE salary < (SELECT AVG(salary) FROM employee)";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}