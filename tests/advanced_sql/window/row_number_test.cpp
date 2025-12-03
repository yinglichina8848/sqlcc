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

class RowNumberTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>();
        db_manager->Initialize("test_window.db");
        
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
            "salary FLOAT,"
            "hire_date DATE"
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

TEST_F(RowNumberTest, BasicRowNumber) {
    // Test basic ROW_NUMBER function
    std::string sql = 
        "SELECT id, name, salary, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_rank "
        "FROM employee";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that rows are numbered sequentially from 1
    for (size_t i = 0; i < rows.size(); ++i) {
        int row_number = rows[i].GetInt(3);
        EXPECT_EQ(row_number, static_cast<int>(i + 1));
    }
    
    // Check that the highest salary has rank 1
    EXPECT_EQ(rows[0].GetString(1), "Alice Brown");  // Highest salary: 70000.0
    EXPECT_EQ(rows[0].GetInt(3), 1);
}

TEST_F(RowNumberTest, RowNumberWithPartitionBy) {
    // Test ROW_NUMBER with PARTITION BY
    std::string sql = 
        "SELECT e.name, d.name as department, e.salary, "
        "ROW_NUMBER() OVER (PARTITION BY e.department_id ORDER BY e.salary DESC) as dept_salary_rank "
        "FROM employee e "
        "JOIN department d ON e.department_id = d.id "
        "ORDER BY d.name, dept_salary_rank";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that within each department, employees are ranked by salary
    std::map<std::string, int> dept_ranks;
    
    for (const auto& row : rows) {
        std::string dept_name = row.GetString(1);
        int rank = row.GetInt(3);
        
        // First occurrence of this department should have rank 1
        if (dept_ranks.find(dept_name) == dept_ranks.end()) {
            EXPECT_EQ(rank, 1);
            dept_ranks[dept_name] = 1;
        } else {
            // Subsequent occurrences should have increasing rank
            EXPECT_EQ(rank, ++dept_ranks[dept_name]);
        }
    }
}

TEST_F(RowNumberTest, RowNumberWithMultipleOrderBy) {
    // Test ROW_NUMBER with multiple ORDER BY columns
    std::string sql = 
        "SELECT e.name, e.hire_date, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.department_id, e.hire_date) as dept_hire_rank "
        "FROM employee e "
        "ORDER BY e.department_id, e.hire_date";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that rows are numbered sequentially within each department by hire date
    std::map<int, int> dept_ranks;
    
    for (const auto& row : rows) {
        int dept_id = row.GetInt(0);  // This should be department_id, but we need to adjust the query
        
        // Let's modify the query to include department_id explicitly
    }
    
    // Let's modify the query to get the department_id
    sql = 
        "SELECT e.department_id, e.name, e.hire_date, e.salary, "
        "ROW_NUMBER() OVER (ORDER BY e.department_id, e.hire_date) as dept_hire_rank "
        "FROM employee e";
    
    result = db_manager->Execute(sql);
    rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that rows are numbered sequentially within each department by hire date
    std::map<int, int> dept_ranks;
    
    for (const auto& row : rows) {
        int dept_id = row.GetInt(0);
        int rank = row.GetInt(4);
        
        // First occurrence of this department should have rank 1
        if (dept_ranks.find(dept_id) == dept_ranks.end()) {
            EXPECT_EQ(rank, 1);
            dept_ranks[dept_id] = 1;
        } else {
            // Subsequent occurrences should have increasing rank
            EXPECT_EQ(rank, ++dept_ranks[dept_id]);
        }
    }
}

TEST_F(RowNumberTest, RowNumberWithComplexExpression) {
    // Test ROW_NUMBER with complex expression in ORDER BY
    std::string sql = 
        "SELECT name, salary, hire_date, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC, hire_date ASC) as salary_hire_rank "
        "FROM employee";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that rows are ordered by salary descending, then by hire_date ascending
    // within employees with the same salary
    float prev_salary = std::numeric_limits<float>::max();
    std::string prev_hire_date = "";
    
    for (const auto& row : rows) {
        float salary = row.GetFloat(1);
        std::string hire_date = row.GetString(2);
        int rank = row.GetInt(3);
        
        // Check salary ordering (descending)
        EXPECT_LE(salary, prev_salary);
        
        // If salaries are equal, check hire date ordering (ascending)
        if (salary == prev_salary) {
            EXPECT_LE(hire_date, prev_hire_date);
        }
        
        prev_salary = salary;
        prev_hire_date = hire_date;
    }
}

TEST_F(RowNumberTest, RankFunction) {
    // Test RANK function (similar to ROW_NUMBER but with ties)
    std::string sql = 
        "SELECT name, salary, "
        "RANK() OVER (ORDER BY salary DESC) as salary_rank, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_row_num "
        "FROM employee";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that RANK and ROW_NUMBER are the same when there are no ties
    for (const auto& row : rows) {
        int rank = row.GetInt(2);
        int row_num = row.GetInt(3);
        EXPECT_EQ(rank, row_num);
    }
    
    // Let's add a tie by modifying the data
    db_manager->Execute("UPDATE employee SET salary = 60000.0 WHERE id = 5");  // Make Charlie Wilson's salary equal to Jane Smith's
    
    result = db_manager->Execute(sql);
    rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Now we should see differences between RANK and ROW_NUMBER
    bool found_tie = false;
    
    for (size_t i = 0; i < rows.size(); ++i) {
        int rank = rows[i].GetInt(2);
        int row_num = rows[i].GetInt(3);
        float salary = rows[i].GetFloat(1);
        
        // For tied salaries, RANK should be the same but ROW_NUMBER should differ
        if (i > 0 && salary == rows[i-1].GetFloat(1)) {
            found_tie = true;
            EXPECT_EQ(rank, rows[i-1].GetInt(2));  // RANK should be the same
            EXPECT_GT(row_num, rows[i-1].GetInt(3));  // ROW_NUMBER should be greater
        }
    }
    
    EXPECT_TRUE(found_tie);
}

TEST_F(RowNumberTest, DenseRankFunction) {
    // Test DENSE_RANK function
    std::string sql = 
        "SELECT name, salary, "
        "RANK() OVER (ORDER BY salary DESC) as salary_rank, "
        "DENSE_RANK() OVER (ORDER BY salary DESC) as salary_dense_rank, "
        "ROW_NUMBER() OVER (ORDER BY salary DESC) as salary_row_num "
        "FROM employee";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that DENSE_RANK and ROW_NUMBER are the same when there are no ties
    for (const auto& row : rows) {
        int rank = row.GetInt(2);
        int dense_rank = row.GetInt(3);
        int row_num = row.GetInt(4);
        EXPECT_EQ(rank, dense_rank);
        EXPECT_EQ(rank, row_num);
    }
    
    // Let's add a tie
    db_manager->Execute("UPDATE employee SET salary = 60000.0 WHERE id = 5");
    
    result = db_manager->Execute(sql);
    rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Now we should see differences between RANK, DENSE_RANK, and ROW_NUMBER
    bool found_tie = false;
    
    for (size_t i = 0; i < rows.size(); ++i) {
        int rank = rows[i].GetInt(2);
        int dense_rank = rows[i].GetInt(3);
        int row_num = rows[i].GetInt(4);
        float salary = rows[i].GetFloat(1);
        
        // For tied salaries:
        // RANK should be the same
        // DENSE_RANK should be the same
        // ROW_NUMBER should be greater
        if (i > 0 && salary == rows[i-1].GetFloat(1)) {
            found_tie = true;
            EXPECT_EQ(rank, rows[i-1].GetInt(2));  // RANK should be the same
            EXPECT_EQ(dense_rank, rows[i-1].GetInt(3));  // DENSE_RANK should be the same
            EXPECT_GT(row_num, rows[i-1].GetInt(4));  // ROW_NUMBER should be greater
        }
        
        // Check that DENSE_RANK increments by 1 for each unique salary value
        if (i > 0 && salary != rows[i-1].GetFloat(1)) {
            EXPECT_EQ(dense_rank, rows[i-1].GetInt(3) + 1);
        }
    }
    
    EXPECT_TRUE(found_tie);
}

TEST_F(RowNumberTest, NTileFunction) {
    // Test NTILE function
    std::string sql = 
        "SELECT name, salary, "
        "NTILE(4) OVER (ORDER BY salary DESC) as salary_quartile "
        "FROM employee "
        "ORDER BY salary DESC";
    
    auto result = db_manager->Execute(sql);
    ASSERT_TRUE(result->IsSuccess());
    
    // Verify result
    auto rows = result->GetRows();
    ASSERT_EQ(rows.size(), 8);
    
    // Check that NTILE(4) divides the rows into 4 groups
    std::map<int, int> quartile_counts;
    
    for (const auto& row : rows) {
        int quartile = row.GetInt(2);
        quartile_counts[quartile]++;
    }
    
    // With 8 rows and 4 quartiles, we should have 2 rows in each quartile
    EXPECT_EQ(quartile_counts[1], 2);
    EXPECT_EQ(quartile_counts[2], 2);
    EXPECT_EQ(quartile_counts[3], 2);
    EXPECT_EQ(quartile_counts[4], 2);
    
    // Check that salaries are ordered correctly within each quartile
    float prev_salary = std::numeric_limits<float>::max();
    int prev_quartile = 0;
    
    for (const auto& row : rows) {
        float salary = row.GetFloat(1);
        int quartile = row.GetInt(2);
        
        // Check that salaries are in descending order
        EXPECT_LE(salary, prev_salary);
        
        // Check that quartiles are non-decreasing
        EXPECT_GE(quartile, prev_quartile);
        
        prev_salary = salary;
        prev_quartile = quartile;
    }
}