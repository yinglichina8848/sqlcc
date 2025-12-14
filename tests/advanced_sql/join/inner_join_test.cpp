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