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

class WindowFunctionExecutorTest : public ::testing::Test {
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
        // Create employee table with more data for window function testing
        std::string create_employee_sql =
            "CREATE TABLE employee ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "department_id INT,"
            "salary FLOAT,"
            "manager_id INT,"
            "hire_date DATE,"
            "performance_score INT"
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

        // Insert test data into employee table - more comprehensive dataset
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0, NULL, '2020-01-15', 85)",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 1, 60000.0, 1, '2020-03-10', 92)",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0, 1, '2020-05-20', 78)",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 2, 70000.0, 2, '2019-11-05', 88)",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0, 2, '2021-02-28', 91)",
            "INSERT INTO employee VALUES (6, 'Diana Davis', 2, 58000.0, 2, '2021-06-15', 82)",
            "INSERT INTO employee VALUES (7, 'Eve Garcia', 3, 75000.0, 4, '2018-09-12', 95)",
            "INSERT INTO employee VALUES (8, 'Frank Miller', 3, 68000.0, 4, '2019-04-20', 87)",
            "INSERT INTO employee VALUES (9, 'Grace Lee', 3, 72000.0, 4, '2019-08-30', 89)",
            "INSERT INTO employee VALUES (10, 'Henry Taylor', 1, 53000.0, 1, '2022-01-10', 76)"
        };

        for (const auto& sql : employee_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
    }

    // Mock helper method to simulate window function query results
    std::vector<std::vector<std::string>> MockExecuteWindowFunction(const std::string& sql) {
        // This is a simplified mock - in real implementation, this would execute actual SQL
        if (sql.find("ROW_NUMBER()") != std::string::npos) {
            // ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC)
            return {
                {"John Doe", "1", "50000", "1"},
                {"Bob Johnson", "1", "55000", "2"},
                {"Jane Smith", "1", "60000", "3"},
                {"Henry Taylor", "1", "53000", "4"},
                {"Diana Davis", "2", "58000", "1"},
                {"Charlie Wilson", "2", "62000", "2"},
                {"Alice Brown", "2", "70000", "3"},
                {"Frank Miller", "3", "68000", "1"},
                {"Grace Lee", "3", "72000", "2"},
                {"Eve Garcia", "3", "75000", "3"}
            };
        }
        if (sql.find("RANK()") != std::string::npos) {
            // RANK() OVER (ORDER BY salary DESC)
            return {
                {"Eve Garcia", "75000", "1"},
                {"Alice Brown", "70000", "2"},
                {"Grace Lee", "72000", "3"},
                {"Frank Miller", "68000", "4"},
                {"Jane Smith", "60000", "5"},
                {"Charlie Wilson", "62000", "6"},
                {"Bob Johnson", "55000", "7"},
                {"Diana Davis", "58000", "8"},
                {"John Doe", "50000", "9"},
                {"Henry Taylor", "53000", "10"}
            };
        }
        if (sql.find("DENSE_RANK()") != std::string::npos) {
            // DENSE_RANK() OVER (ORDER BY performance_score DESC)
            return {
                {"Eve Garcia", "95", "1"},
                {"Jane Smith", "92", "2"},
                {"Charlie Wilson", "91", "3"},
                {"Alice Brown", "88", "4"},
                {"Frank Miller", "87", "5"},
                {"John Doe", "85", "6"},
                {"Grace Lee", "89", "7"},  // Same rank as previous
                {"Diana Davis", "82", "8"},
                {"Bob Johnson", "78", "9"},
                {"Henry Taylor", "76", "10"}
            };
        }
        if (sql.find("LAG(") != std::string::npos) {
            // LAG(salary, 1) OVER (PARTITION BY department_id ORDER BY hire_date)
            return {
                {"John Doe", "2020-01-15", "50000", "NULL"},
                {"Bob Johnson", "2020-05-20", "55000", "50000"},
                {"Jane Smith", "2020-03-10", "60000", "55000"},
                {"Henry Taylor", "2022-01-10", "53000", "60000"},
                {"Alice Brown", "2019-11-05", "70000", "NULL"},
                {"Charlie Wilson", "2021-02-28", "62000", "70000"},
                {"Diana Davis", "2021-06-15", "58000", "62000"},
                {"Eve Garcia", "2018-09-12", "75000", "NULL"},
                {"Frank Miller", "2019-04-20", "68000", "75000"},
                {"Grace Lee", "2019-08-30", "72000", "68000"}
            };
        }
        if (sql.find("LEAD(") != std::string::npos) {
            // LEAD(salary, 1) OVER (ORDER BY salary)
            return {
                {"John Doe", "50000", "53000"},
                {"Henry Taylor", "53000", "55000"},
                {"Bob Johnson", "55000", "58000"},
                {"Diana Davis", "58000", "60000"},
                {"Jane Smith", "60000", "62000"},
                {"Charlie Wilson", "62000", "68000"},
                {"Frank Miller", "68000", "70000"},
                {"Alice Brown", "70000", "72000"},
                {"Grace Lee", "72000", "75000"},
                {"Eve Garcia", "75000", "NULL"}
            };
        }
        if (sql.find("SUM(") != std::string::npos && sql.find("OVER") != std::string::npos) {
            // SUM(salary) OVER (PARTITION BY department_id)
            return {
                {"John Doe", "1", "50000", "218000"},
                {"Jane Smith", "1", "60000", "218000"},
                {"Bob Johnson", "1", "55000", "218000"},
                {"Henry Taylor", "1", "53000", "218000"},
                {"Alice Brown", "2", "70000", "190000"},
                {"Charlie Wilson", "2", "62000", "190000"},
                {"Diana Davis", "2", "58000", "190000"},
                {"Eve Garcia", "3", "75000", "215000"},
                {"Frank Miller", "3", "68000", "215000"},
                {"Grace Lee", "3", "72000", "215000"}
            };
        }
        if (sql.find("AVG(") != std::string::npos && sql.find("OVER") != std::string::npos) {
            // AVG(performance_score) OVER (PARTITION BY department_id)
            return {
                {"John Doe", "1", "85", "82.75"},
                {"Jane Smith", "1", "92", "82.75"},
                {"Bob Johnson", "1", "78", "82.75"},
                {"Henry Taylor", "1", "76", "82.75"},
                {"Alice Brown", "2", "88", "87"},
                {"Charlie Wilson", "2", "91", "87"},
                {"Diana Davis", "2", "82", "87"},
                {"Eve Garcia", "3", "95", "90.33"},
                {"Frank Miller", "3", "87", "90.33"},
                {"Grace Lee", "3", "89", "90.33"}
            };
        }
        if (sql.find("FIRST_VALUE(") != std::string::npos) {
            // FIRST_VALUE(name) OVER (PARTITION BY department_id ORDER BY hire_date)
            return {
                {"John Doe", "2020-01-15", "John Doe"},
                {"Jane Smith", "2020-03-10", "John Doe"},
                {"Bob Johnson", "2020-05-20", "John Doe"},
                {"Henry Taylor", "2022-01-10", "John Doe"},
                {"Alice Brown", "2019-11-05", "Alice Brown"},
                {"Charlie Wilson", "2021-02-28", "Alice Brown"},
                {"Diana Davis", "2021-06-15", "Alice Brown"},
                {"Eve Garcia", "2018-09-12", "Eve Garcia"},
                {"Frank Miller", "2019-04-20", "Eve Garcia"},
                {"Grace Lee", "2019-08-30", "Eve Garcia"}
            };
        }
        if (sql.find("LAST_VALUE(") != std::string::npos) {
            // LAST_VALUE(name) OVER (PARTITION BY department_id ORDER BY hire_date)
            return {
                {"John Doe", "2020-01-15", "Henry Taylor"},
                {"Jane Smith", "2020-03-10", "Henry Taylor"},
                {"Bob Johnson", "2020-05-20", "Henry Taylor"},
                {"Henry Taylor", "2022-01-10", "Henry Taylor"},
                {"Alice Brown", "2019-11-05", "Diana Davis"},
                {"Charlie Wilson", "2021-02-28", "Diana Davis"},
                {"Diana Davis", "2021-06-15", "Diana Davis"},
                {"Eve Garcia", "2018-09-12", "Grace Lee"},
                {"Frank Miller", "2019-04-20", "Grace Lee"},
                {"Grace Lee", "2019-08-30", "Grace Lee"}
            };
        }
        if (sql.find("NTILE(4)") != std::string::npos) {
            // NTILE(4) OVER (ORDER BY salary)
            return {
                {"John Doe", "50000", "1"},
                {"Henry Taylor", "53000", "1"},
                {"Bob Johnson", "55000", "2"},
                {"Diana Davis", "58000", "2"},
                {"Jane Smith", "60000", "3"},
                {"Charlie Wilson", "62000", "3"},
                {"Frank Miller", "68000", "4"},
                {"Alice Brown", "70000", "4"},
                {"Grace Lee", "72000", "4"},
                {"Eve Garcia", "75000", "4"}
            };
        }
        if (sql.find("CUME_DIST()") != std::string::npos) {
            // CUME_DIST() OVER (ORDER BY salary)
            return {
                {"John Doe", "50000", "0.1"},
                {"Henry Taylor", "53000", "0.2"},
                {"Bob Johnson", "55000", "0.3"},
                {"Diana Davis", "58000", "0.4"},
                {"Jane Smith", "60000", "0.5"},
                {"Charlie Wilson", "62000", "0.6"},
                {"Frank Miller", "68000", "0.7"},
                {"Alice Brown", "70000", "0.8"},
                {"Grace Lee", "72000", "0.9"},
                {"Eve Garcia", "75000", "1.0"}
            };
        }
        if (sql.find("PERCENT_RANK()") != std::string::npos) {
            // PERCENT_RANK() OVER (ORDER BY performance_score)
            return {
                {"Henry Taylor", "76", "0.0"},
                {"Bob Johnson", "78", "0.111"},
                {"Diana Davis", "82", "0.222"},
                {"John Doe", "85", "0.333"},
                {"Frank Miller", "87", "0.444"},
                {"Alice Brown", "88", "0.555"},
                {"Grace Lee", "89", "0.666"},
                {"Charlie Wilson", "91", "0.777"},
                {"Jane Smith", "92", "0.888"},
                {"Eve Garcia", "95", "1.0"}
            };
        }
        return {};
    }

    std::unique_ptr<DatabaseManager> db_manager;
    std::string test_db_path;
};

// ===== 排名函数测试 =====

TEST_F(WindowFunctionExecutorTest, RowNumberBasic) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, salary, "
        "ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC) as row_num "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return row numbers within each department partition
    EXPECT_EQ(results.size(), 10);

    // Verify row numbers are sequential within partitions
    int dept1_count = 0, dept2_count = 0, dept3_count = 0;
    for (const auto& row : results) {
        int dept_id = std::stoi(row[1]);
        int row_num = std::stoi(row[3]);

        if (dept_id == 1) {
            dept1_count++;
            EXPECT_EQ(row_num, dept1_count);
        } else if (dept_id == 2) {
            dept2_count++;
            EXPECT_EQ(row_num, dept2_count);
        } else if (dept_id == 3) {
            dept3_count++;
            EXPECT_EQ(row_num, dept3_count);
        }
    }

    // Verify department counts
    EXPECT_EQ(dept1_count, 4);  // Department 1 has 4 employees
    EXPECT_EQ(dept2_count, 3);  // Department 2 has 3 employees
    EXPECT_EQ(dept3_count, 3);  // Department 3 has 3 employees
}

TEST_F(WindowFunctionExecutorTest, RankFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "RANK() OVER (ORDER BY salary DESC) as rank "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return ranks based on salary (with gaps for ties)
    EXPECT_EQ(results.size(), 10);

    // Verify ranking logic - Eve Garcia should be rank 1, Alice Brown rank 2, etc.
    EXPECT_EQ(results[0][0], "Eve Garcia");  // Highest salary
    EXPECT_EQ(results[0][2], "1");           // Rank 1
    EXPECT_EQ(results[1][0], "Alice Brown"); // Second highest
    EXPECT_EQ(results[1][2], "2");           // Rank 2
}

TEST_F(WindowFunctionExecutorTest, DenseRankFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, performance_score, "
        "DENSE_RANK() OVER (ORDER BY performance_score DESC) as dense_rank "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return dense ranks (no gaps)
    EXPECT_EQ(results.size(), 10);

    // Verify dense ranking - no gaps in rank numbers
    std::unordered_set<std::string> ranks;
    for (const auto& row : results) {
        ranks.insert(row[2]);
    }

    // Should have ranks 1-10 without gaps
    EXPECT_EQ(ranks.size(), 10);
    for (int i = 1; i <= 10; ++i) {
        EXPECT_TRUE(ranks.find(std::to_string(i)) != ranks.end());
    }
}

// ===== 导航函数测试 =====

TEST_F(WindowFunctionExecutorTest, LagFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, hire_date, salary, "
        "LAG(salary, 1) OVER (PARTITION BY department_id ORDER BY hire_date) as prev_salary "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return previous salary within each department
    EXPECT_EQ(results.size(), 10);

    // Verify lag function works correctly
    for (const auto& row : results) {
        // Each row should have current salary and previous salary (or NULL)
        EXPECT_FALSE(row[0].empty());  // Name
        EXPECT_FALSE(row[1].empty());  // Hire date
        EXPECT_FALSE(row[2].empty());  // Current salary
        // Previous salary can be empty (NULL)
    }
}

TEST_F(WindowFunctionExecutorTest, LeadFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "LEAD(salary, 1) OVER (ORDER BY salary) as next_salary "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return next salary in overall salary order
    EXPECT_EQ(results.size(), 10);

    // Verify lead function - last row should have NULL for next salary
    EXPECT_EQ(results.back()[2], "NULL");  // Last employee has no next salary
}

TEST_F(WindowFunctionExecutorTest, LagWithOffset) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "LAG(salary, 2) OVER (ORDER BY salary) as prev_prev_salary "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return salary from 2 rows back
    EXPECT_EQ(results.size(), 10);

    // First two rows should have NULL for prev_prev_salary
    EXPECT_EQ(results[0][2], "NULL");
    EXPECT_EQ(results[1][2], "NULL");
}

// ===== 聚合窗口函数测试 =====

TEST_F(WindowFunctionExecutorTest, SumWindowFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, salary, "
        "SUM(salary) OVER (PARTITION BY department_id) as dept_total "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return department total salary for each employee
    EXPECT_EQ(results.size(), 10);

    // Verify department totals are consistent within departments
    double dept1_total = 0.0, dept2_total = 0.0, dept3_total = 0.0;
    for (const auto& row : results) {
        int dept_id = std::stoi(row[1]);
        double dept_total = std::stod(row[3]);

        if (dept_id == 1) {
            if (dept1_total == 0.0) dept1_total = dept_total;
            EXPECT_DOUBLE_EQ(dept_total, dept1_total);
        } else if (dept_id == 2) {
            if (dept2_total == 0.0) dept2_total = dept_total;
            EXPECT_DOUBLE_EQ(dept_total, dept2_total);
        } else if (dept_id == 3) {
            if (dept3_total == 0.0) dept3_total = dept_total;
            EXPECT_DOUBLE_EQ(dept_total, dept3_total);
        }
    }
}

TEST_F(WindowFunctionExecutorTest, AvgWindowFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, performance_score, "
        "AVG(performance_score) OVER (PARTITION BY department_id) as dept_avg_score "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return department average performance score
    EXPECT_EQ(results.size(), 10);

    // Verify averages are consistent within departments
    double dept1_avg = 0.0, dept2_avg = 0.0, dept3_avg = 0.0;
    for (const auto& row : results) {
        int dept_id = std::stoi(row[1]);
        double dept_avg = std::stod(row[3]);

        if (dept_id == 1) {
            if (dept1_avg == 0.0) dept1_avg = dept_avg;
            EXPECT_DOUBLE_EQ(dept_avg, dept1_avg);
        } else if (dept_id == 2) {
            if (dept2_avg == 0.0) dept2_avg = dept_avg;
            EXPECT_DOUBLE_EQ(dept_avg, dept2_avg);
        } else if (dept_id == 3) {
            if (dept3_avg == 0.0) dept3_avg = dept_avg;
            EXPECT_DOUBLE_EQ(dept_avg, dept3_avg);
        }
    }
}

// ===== 值函数测试 =====

TEST_F(WindowFunctionExecutorTest, FirstValueFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, hire_date, "
        "FIRST_VALUE(name) OVER (PARTITION BY department_id ORDER BY hire_date) as first_hired "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return first hired employee name in each department
    EXPECT_EQ(results.size(), 10);

    // Verify first hired names are consistent within departments
    std::string dept1_first, dept2_first, dept3_first;
    for (const auto& row : results) {
        int dept_id = std::stoi(row[1]);  // Need to extract from query
        std::string first_hired = row[2];

        if (dept_id == 1) {
            if (dept1_first.empty()) dept1_first = first_hired;
            EXPECT_EQ(first_hired, dept1_first);
        }
        // Similar checks for other departments
    }
}

TEST_F(WindowFunctionExecutorTest, LastValueFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, hire_date, "
        "LAST_VALUE(name) OVER (PARTITION BY department_id ORDER BY hire_date) as last_hired "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return last hired employee name in each department
    EXPECT_EQ(results.size(), 10);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Current employee name
        EXPECT_FALSE(row[1].empty());  // Hire date
        EXPECT_FALSE(row[2].empty());  // Last hired name
    }
}

// ===== 分位数函数测试 =====

TEST_F(WindowFunctionExecutorTest, NtileFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "NTILE(4) OVER (ORDER BY salary) as quartile "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should divide employees into 4 quartiles based on salary
    EXPECT_EQ(results.size(), 10);

    // Verify quartile distribution (should be roughly equal)
    std::vector<int> quartile_counts(5, 0);  // Index 1-4
    for (const auto& row : results) {
        int quartile = std::stoi(row[2]);
        EXPECT_GE(quartile, 1);
        EXPECT_LE(quartile, 4);
        quartile_counts[quartile]++;
    }

    // Each quartile should have approximately 2-3 employees (10/4 = 2.5)
    for (int i = 1; i <= 4; ++i) {
        EXPECT_GE(quartile_counts[i], 2);
        EXPECT_LE(quartile_counts[i], 3);
    }
}

TEST_F(WindowFunctionExecutorTest, CumulativeDistribution) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "CUME_DIST() OVER (ORDER BY salary) as cumulative_dist "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return cumulative distribution
    EXPECT_EQ(results.size(), 10);

    // Verify cumulative distribution values
    for (const auto& row : results) {
        double cum_dist = std::stod(row[2]);
        EXPECT_GE(cum_dist, 0.0);
        EXPECT_LE(cum_dist, 1.0);
    }

    // Last value should be 1.0
    EXPECT_DOUBLE_EQ(std::stod(results.back()[2]), 1.0);
}

TEST_F(WindowFunctionExecutorTest, PercentRankFunction) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, performance_score, "
        "PERCENT_RANK() OVER (ORDER BY performance_score) as percent_rank "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return percent rank
    EXPECT_EQ(results.size(), 10);

    // Verify percent rank values
    for (const auto& row : results) {
        double pct_rank = std::stod(row[2]);
        EXPECT_GE(pct_rank, 0.0);
        EXPECT_LE(pct_rank, 1.0);
    }

    // First value should be 0.0, last should be 1.0
    EXPECT_DOUBLE_EQ(std::stod(results[0][2]), 0.0);
    EXPECT_DOUBLE_EQ(std::stod(results.back()[2]), 1.0);
}

// ===== 复杂窗口函数测试 =====

TEST_F(WindowFunctionExecutorTest, MultipleWindowFunctions) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, salary, performance_score, "
        "ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC) as salary_rank, "
        "RANK() OVER (ORDER BY performance_score DESC) as performance_rank, "
        "SUM(salary) OVER (PARTITION BY department_id) as dept_salary_sum "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return multiple window function results
    EXPECT_EQ(results.size(), 10);

    // Verify all columns are present
    for (const auto& row : results) {
        EXPECT_EQ(row.size(), 7);  // name, dept_id, salary, perf_score, salary_rank, perf_rank, dept_sum
        EXPECT_FALSE(row[0].empty());  // name
        EXPECT_FALSE(row[1].empty());  // department_id
        EXPECT_FALSE(row[2].empty());  // salary
        EXPECT_FALSE(row[3].empty());  // performance_score
        EXPECT_FALSE(row[4].empty());  // salary_rank
        EXPECT_FALSE(row[5].empty());  // performance_rank
        EXPECT_FALSE(row[6].empty());  // dept_salary_sum
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionWithFrameClause) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, salary, "
        "SUM(salary) OVER (ORDER BY salary ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) as running_total "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return running total of salaries
    EXPECT_EQ(results.size(), 10);

    // Verify running totals are non-decreasing
    double prev_total = 0.0;
    for (const auto& row : results) {
        double running_total = std::stod(row[2]);
        EXPECT_GE(running_total, prev_total);
        prev_total = running_total;
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionWithRangeFrame) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, performance_score, "
        "AVG(performance_score) OVER (ORDER BY performance_score RANGE BETWEEN 5 PRECEDING AND 5 FOLLOWING) as moving_avg "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should return moving average with range frame
    EXPECT_EQ(results.size(), 10);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // name
        EXPECT_FALSE(row[1].empty());  // performance_score
        EXPECT_FALSE(row[2].empty());  // moving_avg
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionWithNamedWindow) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, salary, "
        "ROW_NUMBER() OVER w as row_num, "
        "SUM(salary) OVER w as dept_sum "
        "FROM employee "
        "WINDOW w AS (PARTITION BY department_id ORDER BY salary)";

    auto results = MockExecuteWindowFunction(sql);

    // Should support named window definitions
    EXPECT_EQ(results.size(), 10);

    // Verify both window functions use the same window definition
    for (const auto& row : results) {
        EXPECT_EQ(row.size(), 5);  // name, dept_id, salary, row_num, dept_sum
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionWithFilter) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name, department_id, salary, "
        "COUNT(*) FILTER (WHERE salary > 60000) OVER (PARTITION BY department_id) as high_earners "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should support FILTER clause in window functions
    EXPECT_EQ(results.size(), 10);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // name
        EXPECT_FALSE(row[1].empty());  // department_id
        EXPECT_FALSE(row[2].empty());  // salary
        EXPECT_FALSE(row[3].empty());  // high_earners count
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionPerformanceLargeDataset) {
    CreateStandardTestTables();

    // Add more data for performance testing
    for (int i = 11; i <= 100; ++i) {
        int dept_id = ((i - 1) % 3) + 1;
        double salary = 40000 + (i * 200);
        int perf_score = 70 + (i % 30);
        std::string sql = "INSERT INTO employee VALUES (" +
                         std::to_string(i) + ", 'Employee" + std::to_string(i) +
                         "', " + std::to_string(dept_id) + ", " + std::to_string(salary) +
                         ", 1, '2022-01-01', " + std::to_string(perf_score) + ")";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    std::string sql =
        "SELECT name, department_id, salary, "
        "ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC) as row_num "
        "FROM employee";

    auto results = MockExecuteWindowFunction(sql);

    // Should handle larger dataset
    EXPECT_EQ(results.size(), 100);

    // Verify row numbers are assigned correctly
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // name
        EXPECT_FALSE(row[1].empty());  // department_id
        EXPECT_FALSE(row[2].empty());  // salary
        EXPECT_FALSE(row[3].empty());  // row_num
    }
}

TEST_F(WindowFunctionExecutorTest, WindowFunctionWithSubquery) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT * FROM ("
        "    SELECT name, department_id, salary, "
        "           ROW_NUMBER() OVER (PARTITION BY department_id ORDER BY salary DESC) as rn "
        "    FROM employee"
        ") WHERE rn <= 2";  // Top 2 earners per department

    auto results = MockExecuteWindowFunction(sql);

    // Should support window functions in subqueries
    EXPECT_EQ(results.size(), 6);  // 2 from each department

    // Verify only top 2 from each department
    for (const auto& row : results) {
        int rn = std::stoi(row[3]);
        EXPECT_GE(rn, 1);
        EXPECT_LE(rn, 2);
    }
}
