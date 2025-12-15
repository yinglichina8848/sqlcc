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

class InnerJoinTest : public ::testing::Test {
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

TEST_F(InnerJoinTest, BasicInnerJoin) {
    // Create test tables
    CreateTestTables();
    
    // Test basic INNER JOIN
    std::string sql = 
        "SELECT employee.name, department.name, employee.salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, InnerJoinWithWhere) {
    // Create test tables
    CreateTestTables();
    
    // Test INNER JOIN with WHERE clause
    std::string sql = 
        "SELECT employee.name, department.name "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "WHERE department.name = 'Engineering'";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, InnerJoinWithOrderBy) {
    // Create test tables
    CreateTestTables();
    
    // Test INNER JOIN with ORDER BY
    std::string sql = 
        "SELECT employee.name, employee.salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "ORDER BY employee.salary DESC";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, MultiTableInnerJoin) {
    // Create test tables
    CreateTestTables();
    
    // Create a third table for multi-table join
    std::string create_project_sql = 
        "CREATE TABLE project ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100),"
        "department_id INT"
        ")";
    
    ASSERT_TRUE(db_manager->Execute(create_project_sql));
    
    // Insert project data
    std::vector<std::string> project_inserts = {
        "INSERT INTO project VALUES (1, 'Project X', 1)",
        "INSERT INTO project VALUES (2, 'Project Y', 2)",
        "INSERT INTO project VALUES (3, 'Project Z', 1)"
    };
    
    for (const auto& sql : project_inserts) {
        ASSERT_TRUE(db_manager->Execute(sql));
    }
    
    // Test multi-table INNER JOIN
    std::string sql = 
        "SELECT department.name, project.name "
        "FROM department "
        "INNER JOIN project ON department.id = project.department_id "
        "ORDER BY department.name, project.name";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, InnerJoinWithAggregate) {
    // Create test tables
    CreateTestTables();
    
    // Test INNER JOIN with aggregate function
    std::string sql = 
        "SELECT department.name, COUNT(employee.id) as employee_count, AVG(employee.salary) as avg_salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "GROUP BY department.id, department.name "
        "ORDER BY department.name";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, SelfJoin) {
    // Create test tables
    CreateTestTables();

    // Create employee table with manager_id for self-join test
    std::string create_manager_sql =
        "CREATE TABLE employee_manager ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100),"
        "manager_id INT"
        ")";

    ASSERT_TRUE(db_manager->Execute(create_manager_sql));

    // Insert data for self-join test
    std::vector<std::string> inserts = {
        "INSERT INTO employee_manager VALUES (1, 'John Doe', NULL)",
        "INSERT INTO employee_manager VALUES (2, 'Jane Smith', 1)",
        "INSERT INTO employee_manager VALUES (3, 'Bob Johnson', 1)",
        "INSERT INTO employee_manager VALUES (4, 'Alice Brown', 2)"
    };

    for (const auto& sql : inserts) {
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    // Test self-join
    std::string sql =
        "SELECT e.name as employee_name, m.name as manager_name "
        "FROM employee_manager e "
        "LEFT JOIN employee_manager m ON e.manager_id = m.id "
        "ORDER BY e.name";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, LeftJoin) {
    // Create test tables
    CreateTestTables();

    // Add an employee without department
    ASSERT_TRUE(db_manager->Execute("INSERT INTO employee VALUES (6, 'No Department', NULL, 30000.0)"));

    // Test LEFT JOIN
    std::string sql =
        "SELECT employee.name, department.name "
        "FROM employee "
        "LEFT JOIN department ON employee.department_id = department.id "
        "ORDER BY employee.name";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, RightJoin) {
    // Create test tables
    CreateTestTables();

    // Add a department without employees
    ASSERT_TRUE(db_manager->Execute("INSERT INTO department VALUES (4, 'HR', 'Building D')"));

    // Test RIGHT JOIN
    std::string sql =
        "SELECT employee.name, department.name "
        "FROM employee "
        "RIGHT JOIN department ON employee.department_id = department.id "
        "ORDER BY department.name";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, FullJoin) {
    // Create test tables
    CreateTestTables();

    // Add an employee without department
    ASSERT_TRUE(db_manager->Execute("INSERT INTO employee VALUES (6, 'No Department', NULL, 30000.0)"));

    // Add a department without employees
    ASSERT_TRUE(db_manager->Execute("INSERT INTO department VALUES (4, 'HR', 'Building D')"));

    // Test FULL JOIN (simplified - may not be supported in all databases)
    std::string sql =
        "SELECT employee.name as employee_name, department.name as department_name "
        "FROM employee "
        "FULL OUTER JOIN department ON employee.department_id = department.id "
        "ORDER BY employee.name, department.name";

    // Note: FULL OUTER JOIN might not be supported, so we expect it to fail gracefully
    // or be handled as LEFT JOIN + RIGHT JOIN
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, JoinWithMultipleConditions) {
    // Create test tables
    CreateTestTables();

    // Create a more complex scenario
    std::string create_location_sql =
        "CREATE TABLE location ("
        "dept_id INT PRIMARY KEY,"
        "building VARCHAR(50),"
        "floor INT"
        ")";

    ASSERT_TRUE(db_manager->Execute(create_location_sql));

    std::vector<std::string> location_inserts = {
        "INSERT INTO location VALUES (1, 'Building A', 1)",
        "INSERT INTO location VALUES (2, 'Building B', 2)",
        "INSERT INTO location VALUES (3, 'Building C', 3)"
    };

    for (const auto& sql : location_inserts) {
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    // Test JOIN with multiple conditions (simplified)
    std::string sql =
        "SELECT employee.name, department.name, location.building "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "INNER JOIN location ON department.id = location.dept_id "
        "WHERE location.floor > 1";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, JoinWithAlias) {
    // Create test tables
    CreateTestTables();

    // Test JOIN with table aliases
    std::string sql =
        "SELECT e.name as employee_name, d.name as department_name, e.salary "
        "FROM employee e "
        "INNER JOIN department d ON e.department_id = d.id "
        "ORDER BY e.salary DESC";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, JoinPerformanceTest) {
    // Create larger test tables for performance testing
    CreateTestTables();

    // Add more employees for performance test
    for (int i = 7; i <= 50; ++i) {
        std::string dept_id = std::to_string((i % 3) + 1); // Distribute across departments
        std::string salary = std::to_string(30000 + (i * 1000));
        std::string sql = "INSERT INTO employee VALUES (" +
                          std::to_string(i) + ", 'Employee" + std::to_string(i) +
                          "', " + dept_id + ", " + salary + ".0)";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    // Test JOIN performance with larger dataset
    std::string sql =
        "SELECT employee.name, department.name "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "ORDER BY employee.salary DESC "
        "LIMIT 10";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, JoinWithComplexWhere) {
    // Create test tables
    CreateTestTables();

    // Test JOIN with complex WHERE conditions
    std::string sql =
        "SELECT employee.name, department.name, employee.salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "WHERE employee.salary > 50000 AND department.location LIKE 'Building A%' "
        "ORDER BY employee.salary DESC";

    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(InnerJoinTest, JoinWithGroupBy) {
    // Create test tables
    CreateTestTables();

    // Test JOIN with GROUP BY and aggregate functions
    std::string sql =
        "SELECT department.name, COUNT(employee.id) as employee_count, AVG(employee.salary) as avg_salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "GROUP BY department.id, department.name "
        "HAVING COUNT(employee.id) > 1 "
        "ORDER BY avg_salary DESC";

    EXPECT_TRUE(db_manager->Execute(sql));
}
