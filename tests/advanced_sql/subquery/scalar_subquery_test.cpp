#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "core/database_manager.h"
#include "sql/parser/parser.h"
#include "sql/executor/executor.h"
#include "storage/storage_engine.h"

using namespace sqlcc;

class ScalarSubqueryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>();
        db_manager->Initialize("test_subquery.db");
        
        // Create test tables
        CreateTestTables();
    }
    
    void TearDown() override {
        // Clean up test database
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
        
        auto employee_result = db_manager->Execute(create_employee_sql);
        ASSERT_TRUE(employee_result->IsSuccess());
        
        // Create department table
        std::string create_department_sql = 
            "CREATE TABLE department ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "location VARCHAR(100)"
            ")";
        
        auto department_result = db_manager->Execute(create_department_sql);
        ASSERT_TRUE(department_result->IsSuccess());
        
        // Insert test data into employee table
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0)",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 2, 60000.0)",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0)",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 3, 70000.0)",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0)"
        };
        
        for (const auto& sql : employee_inserts) {
            auto result = db_manager->Execute(sql);
            ASSERT_TRUE(result->IsSuccess());
        }
        
        // Insert test data into department table
        std::vector<std::string> department_inserts = {
            "INSERT INTO department VALUES (1, 'Engineering', 'Building A')",
            "INSERT INTO department VALUES (2, 'Marketing', 'Building B')",
            "INSERT INTO department VALUES (3, 'Sales', 'Building C')"
        };
        
        for (const auto& sql : department_inserts) {
            auto result = db_manager->Execute(sql);
            ASSERT_TRUE(result->IsSuccess());
        }
    }
    
    std::unique_ptr<DatabaseManager> db_manager;
};

TEST_F(ScalarSubqueryTest, BasicScalarSubquery) {
    // Test basic scalar subquery in SELECT clause
    std::string sql = 
        "SELECT name, (SELECT name FROM department WHERE id = 1) as dept_name "
        "FROM employee "
        "WHERE department_id = 1";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 2);  // 2 employees in department 1
    
    // Check that all returned employees have Engineering as dept_name
    for (const auto& row : rows) {
        std::string emp_name = row.GetString(0);
        std::string dept_name = row.GetString(1);
        EXPECT_EQ(dept_name, "Engineering");
    }
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInWhere) {
    // Test scalar subquery in WHERE clause
    std::string sql = 
        "SELECT name, salary "
        "FROM employee "
        "WHERE department_id = (SELECT id FROM department WHERE name = 'Marketing')";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 2);  // 2 employees in Marketing
    
    // Check that all returned employees are from Marketing
    for (const auto& row : rows) {
        std::string emp_name = row.GetString(0);
        float salary = row.GetFloat(1);
        
        EXPECT_TRUE(emp_name == "Jane Smith" || emp_name == "Charlie Wilson");
        EXPECT_TRUE(salary == 60000.0 || salary == 62000.0);
    }
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInOrderBy) {
    // Test scalar subquery in ORDER BY clause
    std::string sql = 
        "SELECT name, salary "
        "FROM employee "
        "ORDER BY (SELECT AVG(salary) FROM employee) - salary";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 5);
    
    // Check that results are ordered by difference from average salary
    // First we need to calculate the average salary
    std::string avg_sql = "SELECT AVG(salary) FROM employee";
    auto avg_result = db_manager->Execute(avg_sql);
    ASSERT_TRUE(avg_result->IsSuccess());
    float avg_salary = avg_result->GetRows()[0].GetFloat(0);
    
    // Now check the ordering
    float prev_diff = std::numeric_limits<float>::min();
    for (const auto& row : rows) {
        float salary = row.GetFloat(1);
        float diff = avg_salary - salary;
        EXPECT_GE(diff, prev_diff);
        prev_diff = diff;
    }
}

TEST_F(ScalarSubqueryTest, CorrelatedScalarSubquery) {
    // Test correlated scalar subquery
    std::string sql = 
        "SELECT e1.name, e1.salary, "
        "(SELECT AVG(e2.salary) FROM employee e2 WHERE e2.department_id = e1.department_id) as dept_avg "
        "FROM employee e1 "
        "ORDER BY e1.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 5);
    
    // Check specific results
    for (const auto& row : rows) {
        std::string name = row.GetString(0);
        float salary = row.GetFloat(1);
        float dept_avg = row.GetFloat(2);
        
        if (name == "John Doe" || name == "Bob Johnson") {
            // Engineering department: (50000 + 55000) / 2 = 52500
            EXPECT_FLOAT_EQ(dept_avg, 52500.0);
        } else if (name == "Jane Smith" || name == "Charlie Wilson") {
            // Marketing department: (60000 + 62000) / 2 = 61000
            EXPECT_FLOAT_EQ(dept_avg, 61000.0);
        } else if (name == "Alice Brown") {
            // Sales department: 70000
            EXPECT_FLOAT_EQ(dept_avg, 70000.0);
        }
    }
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryWithAggregate) {
    // Test scalar subquery with aggregate function
    std::string sql = 
        "SELECT d.name, "
        "(SELECT AVG(e.salary) FROM employee e WHERE e.department_id = d.id) as avg_salary, "
        "(SELECT COUNT(e.id) FROM employee e WHERE e.department_id = d.id) as employee_count "
        "FROM department d "
        "ORDER BY d.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 3);
    
    // Check specific results
    for (const auto& row : rows) {
        std::string dept_name = row.GetString(0);
        float avg_salary = row.GetFloat(1);
        int count = row.GetInt(2);
        
        if (dept_name == "Engineering") {
            EXPECT_FLOAT_EQ(avg_salary, 52500.0);  // (50000 + 55000) / 2
            EXPECT_EQ(count, 2);
        } else if (dept_name == "Marketing") {
            EXPECT_FLOAT_EQ(avg_salary, 61000.0);  // (60000 + 62000) / 2
            EXPECT_EQ(count, 2);
        } else if (dept_name == "Sales") {
            EXPECT_FLOAT_EQ(avg_salary, 70000.0);  // Just 70000
            EXPECT_EQ(count, 1);
        }
    }
}

TEST_F(ScalarSubqueryTest, ScalarSubqueryInHaving) {
    // Test scalar subquery in HAVING clause
    std::string sql = 
        "SELECT department_id, AVG(salary) as dept_avg "
        "FROM employee "
        "GROUP BY department_id "
        "HAVING AVG(salary) > (SELECT AVG(salary) FROM employee)";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    
    // First, calculate the overall average salary
    std::string avg_sql = "SELECT AVG(salary) FROM employee";
    auto avg_result = db_manager->Execute(avg_sql);
    ASSERT_TRUE(avg_result->IsSuccess());
    float overall_avg = avg_result->GetRows()[0].GetFloat(0);
    
    // Check that all returned departments have average salary greater than overall average
    for (const auto& row : rows) {
        float dept_avg = row.GetFloat(1);
        EXPECT_GT(dept_avg, overall_avg);
    }
}

TEST_F(ScalarSubqueryTest, MultipleScalarSubqueries) {
    // Test multiple scalar subqueries in the same query
    std::string sql = 
        "SELECT e.name, e.salary, "
        "(SELECT d.name FROM department d WHERE d.id = e.department_id) as dept_name, "
        "(SELECT AVG(e2.salary) FROM employee e2 WHERE e2.department_id = e.department_id) as dept_avg, "
        "(SELECT COUNT(*) FROM employee e2 WHERE e2.department_id = e.department_id) as dept_count "
        "FROM employee e "
        "ORDER BY e.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 5);
    
    // Check specific results
    for (const auto& row : rows) {
        std::string name = row.GetString(0);
        float salary = row.GetFloat(1);
        std::string dept_name = row.GetString(2);
        float dept_avg = row.GetFloat(3);
        int dept_count = row.GetInt(4);
        
        if (name == "Alice Brown") {
            EXPECT_EQ(dept_name, "Sales");
            EXPECT_FLOAT_EQ(dept_avg, 70000.0);
            EXPECT_EQ(dept_count, 1);
        } else if (name == "Jane Smith") {
            EXPECT_EQ(dept_name, "Marketing");
            EXPECT_FLOAT_EQ(dept_avg, 61000.0);
            EXPECT_EQ(dept_count, 2);
        }
    }
}