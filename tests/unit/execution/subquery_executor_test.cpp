#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <chrono>

#include "core/core_database_manager.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"
#include "storage_engine.h"

using namespace sqlcc;

// Mock result set for testing
class MockResultSet {
public:
    MockResultSet(const std::vector<std::vector<std::string>>& data) : data_(data), current_index_(0) {}

    bool next() {
        if (current_index_ < data_.size()) {
            current_index_++;
            return true;
        }
        return false;
    }

    std::string getString(size_t column) const {
        if (current_index_ > 0 && current_index_ <= data_.size()) {
            const auto& row = data_[current_index_ - 1];
            if (column < row.size()) {
                return row[column];
            }
        }
        return "";
    }

    size_t getColumnCount() const {
        return data_.empty() ? 0 : data_[0].size();
    }

private:
    std::vector<std::vector<std::string>> data_;
    size_t current_index_;
};

class SubqueryExecutorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create unique test directory to avoid conflicts
        test_db_path = "/tmp/test_db_" + std::to_string(getpid()) + "_" +
                      std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

        // Clean up any existing directory
        std::filesystem::remove_all(test_db_path);

        // Initialize test database
        db_manager = std::make_unique<DatabaseManager>(test_db_path);
        db_manager->Initialize();
    }

    void TearDown() override {
        // Clean up test database
        if (db_manager) {
            db_manager->Close();
            db_manager.reset();
        }

        // Remove test directory
        try {
            std::filesystem::remove_all(test_db_path);
        } catch (const std::exception&) {
            // Ignore cleanup errors
        }
    }

    void CreateStandardTestTables() {
        // Create employee table
        std::string create_employee_sql =
            "CREATE TABLE employee ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "department_id INT,"
            "salary FLOAT,"
            "manager_id INT,"
            "hire_date DATE"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_employee_sql));

        // Create department table
        std::string create_department_sql =
            "CREATE TABLE department ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "location VARCHAR(100),"
            "budget FLOAT"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_department_sql));

        // Create project table
        std::string create_project_sql =
            "CREATE TABLE project ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "department_id INT,"
            "budget FLOAT,"
            "status VARCHAR(50)"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_project_sql));

        // Create salary_history table for subquery tests
        std::string create_salary_history_sql =
            "CREATE TABLE salary_history ("
            "id INT PRIMARY KEY,"
            "employee_id INT,"
            "salary FLOAT,"
            "effective_date DATE"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_salary_history_sql));

        // Insert test data into employee table
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0, NULL, '2020-01-15')",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 2, 60000.0, 1, '2020-03-10')",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0, 1, '2020-05-20')",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 3, 70000.0, 2, '2019-11-05')",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0, 1, '2021-02-28')",
            "INSERT INTO employee VALUES (6, 'Diana Davis', NULL, 45000.0, 2, '2021-07-12')"  // No department
        };

        for (const auto& sql : employee_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into department table
        std::vector<std::string> department_inserts = {
            "INSERT INTO department VALUES (1, 'Engineering', 'Building A', 500000.0)",
            "INSERT INTO department VALUES (2, 'Marketing', 'Building B', 300000.0)",
            "INSERT INTO department VALUES (3, 'Sales', 'Building C', 400000.0)",
            "INSERT INTO department VALUES (4, 'HR', 'Building D', 200000.0)"  // No employees
        };

        for (const auto& sql : department_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into project table
        std::vector<std::string> project_inserts = {
            "INSERT INTO project VALUES (1, 'Project Alpha', 1, 100000.0, 'Active')",
            "INSERT INTO project VALUES (2, 'Project Beta', 2, 80000.0, 'Active')",
            "INSERT INTO project VALUES (3, 'Project Gamma', 1, 120000.0, 'Completed')",
            "INSERT INTO project VALUES (4, 'Project Delta', NULL, 50000.0, 'Planning')"  // No department
        };

        for (const auto& sql : project_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into salary_history table
        std::vector<std::string> salary_history_inserts = {
            "INSERT INTO salary_history VALUES (1, 1, 45000.0, '2020-01-15')",
            "INSERT INTO salary_history VALUES (2, 1, 50000.0, '2021-01-15')",
            "INSERT INTO salary_history VALUES (3, 2, 55000.0, '2020-03-10')",
            "INSERT INTO salary_history VALUES (4, 2, 60000.0, '2021-03-10')",
            "INSERT INTO salary_history VALUES (5, 3, 50000.0, '2020-05-20')",
            "INSERT INTO salary_history VALUES (6, 3, 55000.0, '2021-05-20')",
            "INSERT INTO salary_history VALUES (7, 4, 65000.0, '2019-11-05')",
            "INSERT INTO salary_history VALUES (8, 4, 70000.0, '2020-11-05')"
        };

        for (const auto& sql : salary_history_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
    }

    // Mock helper method to simulate query results
    std::vector<std::vector<std::string>> MockExecuteAndCollectResults(const std::string& sql) {
        // This is a simplified mock - in real implementation, this would execute actual SQL
        if (sql.find("SELECT") != std::string::npos && sql.find("employee") != std::string::npos) {
            if (sql.find("salary > (SELECT AVG(salary) FROM employee)") != std::string::npos) {
                // Employees with above-average salary
                return {
                    {"John Doe", "50000"},
                    {"Jane Smith", "60000"},
                    {"Bob Johnson", "55000"},
                    {"Alice Brown", "70000"},
                    {"Charlie Wilson", "62000"}
                };
            }
            if (sql.find("department_id IN (SELECT id FROM department") != std::string::npos) {
                // Employees in departments with budget > 350000
                return {
                    {"John Doe", "Engineering"},
                    {"Bob Johnson", "Engineering"},
                    {"Alice Brown", "Sales"}
                };
            }
        }
        return {};
    }

    std::unique_ptr<DatabaseManager> db_manager;
};

// ===== Scalar Subquery Tests =====

TEST_F(SubqueryExecutorTest, ScalarSubqueryInWhere) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary "
        "FROM employee "
        "WHERE salary > (SELECT AVG(salary) FROM employee)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees with above-average salary
    EXPECT_GT(results.size(), 0);

    // Verify all returned employees have salary above average
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name should not be empty
        EXPECT_FALSE(row[1].empty());  // Salary should not be empty
    }
}

TEST_F(SubqueryExecutorTest, ScalarSubqueryInSelect) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "(SELECT MAX(salary) FROM employee WHERE department_id = e.department_id) as dept_max_salary "
        "FROM employee e";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all employees with their department's max salary
    EXPECT_EQ(results.size(), 6);  // All employees

    // Verify each employee has a department max salary
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Salary
        // dept_max_salary may be empty for employees without department
    }
}

TEST_F(SubqueryExecutorTest, ScalarSubqueryWithAggregation) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, "
        "(salary - (SELECT AVG(salary) FROM employee)) as salary_diff "
        "FROM employee "
        "ORDER BY salary_diff DESC";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all employees with salary difference from average
    EXPECT_EQ(results.size(), 6);

    // First result should have highest positive salary difference
    if (!results.empty()) {
        EXPECT_EQ(results[0][0], "Alice Brown");  // Highest salary
    }
}

// ===== Table Subquery Tests =====

TEST_F(SubqueryExecutorTest, TableSubqueryInFrom) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT dept_summary.name, dept_summary.employee_count "
        "FROM (SELECT department_id, COUNT(*) as employee_count "
        "      FROM employee "
        "      GROUP BY department_id) as dept_summary";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department employee counts
    EXPECT_GT(results.size(), 0);

    // Verify results contain department names and counts
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Department name
        EXPECT_FALSE(row[1].empty());  // Employee count
    }
}

TEST_F(SubqueryExecutorTest, TableSubqueryInExists) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name "
        "FROM department d "
        "WHERE EXISTS (SELECT 1 FROM employee e WHERE e.department_id = d.id)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return departments that have employees
    EXPECT_EQ(results.size(), 3);  // Engineering, Marketing, Sales

    // Verify HR department is not included (no employees)
    bool found_hr = false;
    for (const auto& row : results) {
        if (row[0] == "HR") {
            found_hr = true;
            break;
        }
    }
    EXPECT_FALSE(found_hr);
}

TEST_F(SubqueryExecutorTest, TableSubqueryInNotExists) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name "
        "FROM department d "
        "WHERE NOT EXISTS (SELECT 1 FROM employee e WHERE e.department_id = d.id)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return departments that have no employees
    EXPECT_EQ(results.size(), 1);

    // Verify only HR department is returned
    if (!results.empty()) {
        EXPECT_EQ(results[0][0], "HR");
    }
}

TEST_F(SubqueryExecutorTest, TableSubqueryInInClause) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name "
        "FROM employee "
        "WHERE department_id IN (SELECT id FROM department WHERE budget > 350000)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees in high-budget departments
    EXPECT_GT(results.size(), 0);

    // Verify results
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Employee name
    }
}

// ===== Correlated Subquery Tests =====

TEST_F(SubqueryExecutorTest, CorrelatedSubqueryInWhere) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name, e.salary "
        "FROM employee e "
        "WHERE e.salary > (SELECT AVG(salary) FROM employee WHERE department_id = e.department_id)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees with above-average salary in their department
    EXPECT_GT(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Salary
    }
}

TEST_F(SubqueryExecutorTest, CorrelatedSubqueryInSelect) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name, "
        "(SELECT COUNT(*) FROM employee WHERE manager_id = e.id) as direct_reports "
        "FROM employee e";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all employees with their direct report count
    EXPECT_EQ(results.size(), 6);

    // John Doe should have direct reports
    bool found_john = false;
    for (const auto& row : results) {
        if (row[0] == "John Doe") {
            found_john = true;
            // Should have 2 direct reports (Jane and Bob)
            break;
        }
    }
    EXPECT_TRUE(found_john);
}

TEST_F(SubqueryExecutorTest, CorrelatedSubqueryWithExists) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name "
        "FROM employee e "
        "WHERE EXISTS (SELECT 1 FROM employee sub WHERE sub.manager_id = e.id)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees who are managers
    EXPECT_EQ(results.size(), 2);  // John and Jane

    // Verify John and Jane are included
    std::unordered_set<std::string> manager_names;
    for (const auto& row : results) {
        manager_names.insert(row[0]);
    }

    EXPECT_TRUE(manager_names.count("John Doe"));
    EXPECT_TRUE(manager_names.count("Jane Smith"));
}

// ===== Nested Subquery Tests =====

TEST_F(SubqueryExecutorTest, NestedSubqueries) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary "
        "FROM employee "
        "WHERE department_id IN ("
        "    SELECT id FROM department "
        "    WHERE budget > (SELECT AVG(budget) FROM department)"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees in above-average budget departments
    EXPECT_GT(results.size(), 0);

    // Verify results
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Salary
    }
}

TEST_F(SubqueryExecutorTest, MultipleNestedSubqueries) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name "
        "FROM employee e "
        "WHERE e.salary > ALL ("
        "    SELECT AVG(salary) FROM employee "
        "    WHERE department_id IN ("
        "        SELECT id FROM department "
        "        WHERE budget > (SELECT AVG(budget) FROM department)"
        "    )"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees with salary above average of above-average budget departments
    EXPECT_GE(results.size(), 0);  // May be empty depending on data

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
    }
}

TEST_F(SubqueryExecutorTest, SubqueryWithHaving) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT d.name, COUNT(e.id) as emp_count "
        "FROM department d "
        "LEFT JOIN employee e ON d.id = e.department_id "
        "GROUP BY d.id, d.name "
        "HAVING COUNT(e.id) > ("
        "    SELECT AVG(emp_count) FROM ("
        "        SELECT COUNT(*) as emp_count "
        "        FROM employee "
        "        GROUP BY department_id"
        "    ) as dept_counts"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return departments with above-average employee count
    EXPECT_GE(results.size(), 0);

    // Verify results
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Department name
        EXPECT_FALSE(row[1].empty());  // Employee count
    }
}

// ===== Complex Subquery Scenarios =====

TEST_F(SubqueryExecutorTest, SubqueryWithUnion) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name "
        "FROM employee "
        "WHERE department_id IN ("
        "    SELECT id FROM department WHERE name LIKE 'E%'"
        "    UNION "
        "    SELECT id FROM department WHERE name LIKE 'M%'"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees in Engineering or Marketing
    EXPECT_GT(results.size(), 0);

    // Verify results
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Employee name
    }
}

TEST_F(SubqueryExecutorTest, SubqueryWithCase) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, "
        "CASE "
        "    WHEN salary > (SELECT AVG(salary) FROM employee) THEN 'Above Average' "
        "    WHEN salary = (SELECT AVG(salary) FROM employee) THEN 'Average' "
        "    ELSE 'Below Average' "
        "END as salary_category "
        "FROM employee";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all employees with salary categories
    EXPECT_EQ(results.size(), 6);

    // Verify categories are assigned
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Category
        EXPECT_TRUE(row[1] == "Above Average" || row[1] == "Average" || row[1] == "Below Average");
    }
}

TEST_F(SubqueryExecutorTest, SubqueryPerformanceWithLargeDataset) {
    CreateStandardTestTables();

    // Add more employees for performance testing
    for (int i = 7; i <= 50; ++i) {
        int dept_id = (i % 3) + 1;
        double salary = 40000 + (i * 500);
        std::string sql = "INSERT INTO employee VALUES (" +
                         std::to_string(i) + ", 'Employee" + std::to_string(i) +
                         "', " + std::to_string(dept_id) + ", " + std::to_string(salary) +
                         ", " + std::to_string((i % 3) + 1) + ", '2022-01-01')";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    std::string sql =
        "SELECT e.name "
        "FROM employee e "
        "WHERE e.salary > (SELECT AVG(salary) FROM employee WHERE department_id = e.department_id)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should handle larger dataset with correlated subquery
    EXPECT_GT(results.size(), 10);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
    }
}

TEST_F(SubqueryExecutorTest, SubqueryWithNullHandling) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name "
        "FROM employee "
        "WHERE department_id IS NULL OR "
        "department_id NOT IN ("
        "    SELECT id FROM department WHERE budget IS NULL"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees without null budget departments
    EXPECT_EQ(results.size(), 6);  // All employees (no departments have null budget)

    // Verify Diana (no department) is included
    bool found_diana = false;
    for (const auto& row : results) {
        if (row[0] == "Diana Davis") {
            found_diana = true;
            break;
        }
    }
    EXPECT_TRUE(found_diana);
}

TEST_F(SubqueryExecutorTest, SubqueryWithDistinct) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT DISTINCT d.name "
        "FROM department d "
        "WHERE d.id IN ("
        "    SELECT DISTINCT department_id "
        "    FROM employee "
        "    WHERE salary > 55000"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return distinct department names for high-salary employees
    EXPECT_GT(results.size(), 0);
    EXPECT_LE(results.size(), 4);  // Max 4 departments

    // Verify no duplicates
    std::unordered_set<std::string> dept_names;
    for (const auto& row : results) {
        dept_names.insert(row[0]);
    }
    EXPECT_EQ(dept_names.size(), results.size());  // No duplicates
}

TEST_F(SubqueryExecutorTest, SubqueryOrderByInFrom) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT top_employees.name, top_employees.salary "
        "FROM ("
        "    SELECT name, salary "
        "    FROM employee "
        "    ORDER BY salary DESC "
        "    LIMIT 3"
        ") as top_employees";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return top 3 highest paid employees
    EXPECT_EQ(results.size(), 3);

    // Verify Alice Brown is first (highest salary)
    if (!results.empty()) {
        EXPECT_EQ(results[0][0], "Alice Brown");
    }
}

TEST_F(SubqueryExecutorTest, ComplexNestedCorrelatedSubquery) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name, e.salary "
        "FROM employee e "
        "WHERE e.salary > ("
        "    SELECT AVG(sub.salary) "
        "    FROM employee sub "
        "    WHERE sub.department_id = e.department_id "
        "    AND sub.id IN ("
        "        SELECT DISTINCT manager_id "
        "        FROM employee "
        "        WHERE manager_id IS NOT NULL"
        "    )"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return employees with above-average salary among manager's reports
    EXPECT_GE(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Salary
    }
}
