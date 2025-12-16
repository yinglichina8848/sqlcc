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

class RowNumberTest : public ::testing::Test {
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
            "salary FLOAT,"
            "hire_date DATE"
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
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0, '2020-01-15')",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 2, 60000.0, '2019-03-20')",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0, '2021-05-10')",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 3, 70000.0, '2018-11-05')",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0, '2020-07-22')",
            "INSERT INTO employee VALUES (6, 'David Miller', 1, 48000.0, '2022-02-14')",
            "INSERT INTO employee VALUES (7, 'Eve Davis', 3, 68000.0, '2019-09-30')",
            "INSERT INTO employee VALUES (8, 'Frank Garcia', 2, 58000.0, '2021-12-01')"
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

TEST_F(RowNumberTest, BasicRowNumber) {
    // Create test tables
    CreateTestTables();
    
    // Test basic ROW_NUMBER function
    std::string sql = 
        "SELECT id, name, salary, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_rank "
        "FROM employee";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithPartitionBy) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with PARTITION BY
    std::string sql = 
        "SELECT e.name, d.name as department, e.salary, "
        "ROW_NUMBER() OVER (PARTITION BY e.department_id ORDER BY e.salary DESC) as dept_salary_rank "
        "FROM employee e "
        "JOIN department d ON e.department_id = d.id "
        "ORDER BY d.name, dept_salary_rank";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithMultipleOrderBy) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with multiple ORDER BY columns
    std::string sql = 
        "SELECT e.department_id, e.name, e.hire_date, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.department_id, e.hire_date) as dept_hire_rank "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithComplexExpression) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with complex expression in ORDER BY
    std::string sql = 
        "SELECT e.name, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.salary * 1.1 DESC) as adjusted_salary_rank "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithFrameClause) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with frame clause (should be ignored as ROW_NUMBER doesn't use frames)
    std::string sql = 
        "SELECT e.name, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.salary ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) as row_num "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithNullValues) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with null values
    std::string sql = 
        "SELECT e.name, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.salary ASC NULLS LAST) as row_num "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, NestedWindowFunctions) {
    // Create test tables
    CreateTestTables();
    
    // Test nested window functions (ROW_NUMBER with other window functions)
    std::string sql = 
        "SELECT e.name, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.salary) as row_num, "
        "AVG(e.salary) OVER () as avg_salary "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberInSubquery) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER in subquery
    std::string sql = 
        "SELECT name, salary FROM ("
        "SELECT e.name, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.salary DESC) as row_num "
        "FROM employee e"
        ") ranked WHERE row_num <= 3";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RowNumberWithJoinAndGroupBy) {
    // Create test tables
    CreateTestTables();
    
    // Test ROW_NUMBER with join and GROUP BY
    std::string sql = 
        "SELECT d.name, COUNT(e.id) as employee_count, "
        "ROW_NUMBER() OVER (ORDER BY COUNT(e.id) DESC) as dept_rank "
        "FROM department d "
        "LEFT JOIN employee e ON d.id = e.department_id "
        "GROUP BY d.id, d.name";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, MultipleRowNumberColumns) {
    // Create test tables
    CreateTestTables();
    
    // Test multiple ROW_NUMBER columns with different ORDER BY clauses
    std::string sql = 
        "SELECT e.name, e.salary, e.hire_date, "
        "ROW_NUMBER() OVER (ORDER BY e.salary DESC) as salary_rank, "
        "ROW_NUMBER() OVER (ORDER BY e.hire_date) as hire_rank "
        "FROM employee e";
    
    EXPECT_TRUE(db_manager->Execute(sql));
}

TEST_F(RowNumberTest, RankFunction) {
    // Create test tables
    CreateTestTables();
    
    // Test RANK function (similar to ROW_NUMBER but with ties)
    std::string sql = 
        "SELECT name, salary, "
        "RANK() OVER (ORDER BY salary DESC) as salary_rank, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_row_num "
        "FROM employee";
    
    bool result = db_manager->Execute(sql);
    ASSERT_TRUE(result);
    
    // Let's add a tie by modifying the data
    db_manager->Execute("UPDATE employee SET salary = 60000.0 WHERE id = 5");  // Make Charlie Wilson's salary equal to Jane Smith's
    
    result = db_manager->Execute(sql);
    ASSERT_TRUE(result);

}

TEST_F(RowNumberTest, DenseRankFunction) {
    // Create test tables
    CreateTestTables();
    
    // Test DENSE_RANK function
    std::string sql = 
        "SELECT name, salary, "
        "RANK() OVER (ORDER BY salary DESC) as salary_rank, "
        "DENSE_RANK() OVER (ORDER BY salary DESC) as salary_dense_rank, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_row_num "
        "FROM employee";
    
    bool result = db_manager->Execute(sql);
    ASSERT_TRUE(result);
    
    // Let's add a tie
    db_manager->Execute("UPDATE employee SET salary = 60000.0 WHERE id = 5");
    
    result = db_manager->Execute(sql);
    ASSERT_TRUE(result);

}

TEST_F(RowNumberTest, NTileFunction) {
    // Create test tables
    CreateTestTables();
    
    // Test NTILE function
    std::string sql = 
        "SELECT name, salary, "
        "NTILE(4) OVER (ORDER BY salary DESC) as salary_quartile "
        "FROM employee "
        "ORDER BY salary DESC";
    
    bool result = db_manager->Execute(sql);
    ASSERT_TRUE(result);

}