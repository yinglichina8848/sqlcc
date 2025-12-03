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

class InnerJoinTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>();
        db_manager->Initialize("test_join.db");
        
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

TEST_F(InnerJoinTest, BasicInnerJoin) {
    // Test basic INNER JOIN
    std::string sql = 
        "SELECT employee.name, department.name, employee.salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 5);  // All 5 employees have matching departments
    
    // Check a few specific rows
    bool john_found = false;
    bool jane_found = false;
    
    for (const auto& row : rows) {
        std::string emp_name = row.GetString(0);
        std::string dept_name = row.GetString(1);
        float salary = row.GetFloat(2);
        
        if (emp_name == "John Doe" && dept_name == "Engineering" && salary == 50000.0) {
            john_found = true;
        }
        
        if (emp_name == "Jane Smith" && dept_name == "Marketing" && salary == 60000.0) {
            jane_found = true;
        }
    }
    
    EXPECT_TRUE(john_found);
    EXPECT_TRUE(jane_found);
}

TEST_F(InnerJoinTest, InnerJoinWithWhere) {
    // Test INNER JOIN with WHERE clause
    std::string sql = 
        "SELECT employee.name, department.name "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "WHERE department.name = 'Engineering'";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 2);  // 2 employees in Engineering
    
    // Check that all returned employees are from Engineering
    for (const auto& row : rows) {
        std::string dept_name = row.GetString(1);
        EXPECT_EQ(dept_name, "Engineering");
    }
}

TEST_F(InnerJoinTest, InnerJoinWithOrderBy) {
    // Test INNER JOIN with ORDER BY
    std::string sql = 
        "SELECT employee.name, employee.salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "ORDER BY employee.salary DESC";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 5);
    
    // Check that results are ordered by salary in descending order
    float prev_salary = std::numeric_limits<float>::max();
    for (const auto& row : rows) {
        float salary = row.GetFloat(1);
        EXPECT_LE(salary, prev_salary);
        prev_salary = salary;
    }
}

TEST_F(InnerJoinTest, MultiTableInnerJoin) {
    // Create a third table for multi-table join
    std::string create_project_sql = 
        "CREATE TABLE project ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100),"
        "department_id INT"
        ")";
    
    auto project_result = db_manager->Execute(create_project_sql);
    ASSERT_TRUE(project_result->IsSuccess());
    
    // Insert project data
    std::vector<std::string> project_inserts = {
        "INSERT INTO project VALUES (1, 'Project X', 1)",
        "INSERT INTO project VALUES (2, 'Project Y', 2)",
        "INSERT INTO project VALUES (3, 'Project Z', 1)"
    };
    
    for (const auto& sql : project_inserts) {
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
    }
    
    // Test multi-table INNER JOIN
    std::string sql = 
        "SELECT department.name, project.name "
        "FROM department "
        "INNER JOIN project ON department.id = project.department_id "
        "ORDER BY department.name, project.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 3);
    
    // Check specific results
    for (const auto& row : rows) {
        std::string dept_name = row.GetString(0);
        std::string proj_name = row.GetString(1);
        
        // Verify that project belongs to department
        if (dept_name == "Engineering") {
            EXPECT_TRUE(proj_name == "Project X" || proj_name == "Project Z");
        } else if (dept_name == "Marketing") {
            EXPECT_EQ(proj_name, "Project Y");
        }
    }
}

TEST_F(InnerJoinTest, InnerJoinWithAggregate) {
    // Test INNER JOIN with aggregate function
    std::string sql = 
        "SELECT department.name, COUNT(employee.id) as employee_count, AVG(employee.salary) as avg_salary "
        "FROM employee "
        "INNER JOIN department ON employee.department_id = department.id "
        "GROUP BY department.id, department.name "
        "ORDER BY department.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 3);  // 3 departments
    
    // Check specific results
    for (const auto& row : rows) {
        std::string dept_name = row.GetString(0);
        int count = row.GetInt(1);
        float avg_salary = row.GetFloat(2);
        
        if (dept_name == "Engineering") {
            EXPECT_EQ(count, 2);
            EXPECT_FLOAT_EQ(avg_salary, 52500.0);  // (50000 + 55000) / 2
        } else if (dept_name == "Marketing") {
            EXPECT_EQ(count, 2);
            EXPECT_FLOAT_EQ(avg_salary, 61000.0);  // (60000 + 62000) / 2
        } else if (dept_name == "Sales") {
            EXPECT_EQ(count, 1);
            EXPECT_FLOAT_EQ(avg_salary, 70000.0);
        }
    }
}

TEST_F(InnerJoinTest, SelfJoin) {
    // Create employee table with manager_id for self-join test
    std::string create_manager_sql = 
        "CREATE TABLE employee_manager ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100),"
        "manager_id INT"
        ")";
    
    auto result = db_manager->Execute(create_manager_sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Insert data for self-join test
    std::vector<std::string> inserts = {
        "INSERT INTO employee_manager VALUES (1, 'John Doe', NULL)",
        "INSERT INTO employee_manager VALUES (2, 'Jane Smith', 1)",
        "INSERT INTO employee_manager VALUES (3, 'Bob Johnson', 1)",
        "INSERT INTO employee_manager VALUES (4, 'Alice Brown', 2)"
    };
    
    for (const auto& sql : inserts) {
        auto result = db_manager->Execute(sql);
        ASSERT_TRUE(result->IsSuccess());
    }
    
    // Test self-join
    std::string sql = 
        "SELECT e.name as employee_name, m.name as manager_name "
        "FROM employee_manager e "
        "LEFT JOIN employee_manager m ON e.manager_id = m.id "
        "ORDER BY e.name";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 4);
    
    // Check specific results
    for (const auto& row : rows) {
        std::string emp_name = row.GetString(0);
        
        if (emp_name == "John Doe") {
            EXPECT_TRUE(row.IsNull(1));  // John has no manager
        } else if (emp_name == "Jane Smith") {
            EXPECT_EQ(row.GetString(1), "John Doe");
        } else if (emp_name == "Bob Johnson") {
            EXPECT_EQ(row.GetString(1), "John Doe");
        } else if (emp_name == "Alice Brown") {
            EXPECT_EQ(row.GetString(1), "Jane Smith");
        }
    }
}