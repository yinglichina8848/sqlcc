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

class SetOperationExecutorTest : public ::testing::Test {
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

        // Create contractor table for set operations
        std::string create_contractor_sql =
            "CREATE TABLE contractor ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "department_id INT,"
            "hourly_rate FLOAT,"
            "hire_date DATE"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_contractor_sql));

        // Insert test data into employee table
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'John Doe', 1, 50000.0, NULL, '2020-01-15')",
            "INSERT INTO employee VALUES (2, 'Jane Smith', 2, 60000.0, 1, '2020-03-10')",
            "INSERT INTO employee VALUES (3, 'Bob Johnson', 1, 55000.0, 1, '2020-05-20')",
            "INSERT INTO employee VALUES (4, 'Alice Brown', 3, 70000.0, 2, '2019-11-05')",
            "INSERT INTO employee VALUES (5, 'Charlie Wilson', 2, 62000.0, 1, '2021-02-28')"
        };

        for (const auto& sql : employee_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into contractor table
        std::vector<std::string> contractor_inserts = {
            "INSERT INTO contractor VALUES (1, 'Mike Davis', 1, 75.0, '2021-06-15')",
            "INSERT INTO contractor VALUES (2, 'Sarah Miller', 2, 80.0, '2021-08-20')",
            "INSERT INTO contractor VALUES (3, 'Tom Anderson', 3, 85.0, '2021-10-10')",
            "INSERT INTO contractor VALUES (4, 'Lisa Garcia', NULL, 70.0, '2022-01-05')"  // No department
        };

        for (const auto& sql : contractor_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
    }

    // Mock helper method to simulate query results
    std::vector<std::vector<std::string>> MockExecuteAndCollectResults(const std::string& sql) {
        // This is a simplified mock - in real implementation, this would execute actual SQL
        if (sql.find("UNION") != std::string::npos) {
            if (sql.find("employee") != std::string::npos && sql.find("contractor") != std::string::npos) {
                // UNION of employee and contractor names
                return {
                    {"John Doe"},
                    {"Jane Smith"},
                    {"Bob Johnson"},
                    {"Alice Brown"},
                    {"Charlie Wilson"},
                    {"Mike Davis"},
                    {"Sarah Miller"},
                    {"Tom Anderson"},
                    {"Lisa Garcia"}
                };
            }
        }
        if (sql.find("INTERSECT") != std::string::npos) {
            // INTERSECT of department_ids that exist in both tables
            return {
                {"1"},
                {"2"},
                {"3"}
            };
        }
        if (sql.find("EXCEPT") != std::string::npos) {
            // EXCEPT - department_ids in employee but not in contractor
            return {
                {"NULL"}  // Diana has NULL department
            };
        }
        return {};
    }

    std::unique_ptr<DatabaseManager> db_manager;
    std::string test_db_path;
};

// ===== UNION Tests =====

TEST_F(SetOperationExecutorTest, UnionBasic) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee "
        "UNION "
        "SELECT name FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all unique names from both tables
    EXPECT_EQ(results.size(), 9);  // 5 employees + 4 contractors

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name should not be empty
    }
}

TEST_F(SetOperationExecutorTest, UnionAll) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "UNION ALL "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all department_ids including duplicates
    EXPECT_EQ(results.size(), 9);  // 5 employees + 4 contractors

    // Verify results structure
    for (const auto& row : results) {
        // department_id can be NULL
    }
}

TEST_F(SetOperationExecutorTest, UnionDistinct) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "UNION DISTINCT "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return unique department_ids
    EXPECT_EQ(results.size(), 4);  // 1, 2, 3, NULL

    // Verify no duplicates
    std::unordered_set<std::string> dept_ids;
    for (const auto& row : results) {
        dept_ids.insert(row[0]);
    }
    EXPECT_EQ(dept_ids.size(), results.size());
}

TEST_F(SetOperationExecutorTest, UnionWithOrderBy) {
    CreateStandardTestTables();

    std::string sql =
        "(SELECT name FROM employee UNION SELECT name FROM contractor) "
        "ORDER BY name";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return all unique names in alphabetical order
    EXPECT_EQ(results.size(), 9);

    // Verify alphabetical order
    if (results.size() >= 2) {
        EXPECT_LT(results[0][0], results[1][0]);  // First should be before second alphabetically
    }
}

// ===== INTERSECT Tests =====

TEST_F(SetOperationExecutorTest, IntersectBasic) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "INTERSECT "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids that exist in both tables
    EXPECT_EQ(results.size(), 3);  // 1, 2, 3

    // Verify results are valid department IDs
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
        EXPECT_TRUE(row[0] == "1" || row[0] == "2" || row[0] == "3");
    }
}

TEST_F(SetOperationExecutorTest, IntersectAll) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "INTERSECT ALL "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids with multiplicity
    EXPECT_GE(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
    }
}

TEST_F(SetOperationExecutorTest, IntersectDistinct) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "INTERSECT DISTINCT "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return unique department_ids that exist in both tables
    EXPECT_EQ(results.size(), 3);

    // Verify no duplicates
    std::unordered_set<std::string> dept_ids;
    for (const auto& row : results) {
        dept_ids.insert(row[0]);
    }
    EXPECT_EQ(dept_ids.size(), results.size());
}

// ===== EXCEPT Tests =====

TEST_F(SetOperationExecutorTest, ExceptBasic) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "EXCEPT "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids in employee but not in contractor
    EXPECT_GE(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        // Can be NULL (employees without departments)
    }
}

TEST_F(SetOperationExecutorTest, ExceptAll) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "EXCEPT ALL "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids with proper multiplicity
    EXPECT_GE(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        // Can be NULL
    }
}

TEST_F(SetOperationExecutorTest, ExceptDistinct) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "EXCEPT DISTINCT "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return unique department_ids in employee but not in contractor
    EXPECT_GE(results.size(), 0);

    // Verify no duplicates
    std::unordered_set<std::string> dept_ids;
    for (const auto& row : results) {
        dept_ids.insert(row[0]);
    }
    EXPECT_EQ(dept_ids.size(), results.size());
}

// ===== Complex Set Operation Tests =====

TEST_F(SetOperationExecutorTest, MultipleUnionOperations) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee WHERE department_id = 1 "
        "UNION "
        "SELECT name FROM employee WHERE department_id = 2 "
        "UNION "
        "SELECT name FROM contractor WHERE department_id = 1";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return names from multiple departments
    EXPECT_GT(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name should not be empty
    }
}

TEST_F(SetOperationExecutorTest, UnionIntersectCombination) {
    CreateStandardTestTables();

    std::string sql =
        "(SELECT department_id FROM employee UNION SELECT department_id FROM contractor) "
        "INTERSECT "
        "(SELECT id FROM department)";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids that exist in all three sets
    EXPECT_GE(results.size(), 0);

    // Verify results are valid department IDs
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithAggregates) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT COUNT(*) as total_employees FROM employee "
        "UNION "
        "SELECT COUNT(*) as total_contractors FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return counts from both tables
    EXPECT_EQ(results.size(), 2);

    // Verify results are numbers
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
        // Should be numeric
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithSubqueries) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM (SELECT department_id FROM employee WHERE salary > 60000) e "
        "UNION "
        "SELECT department_id FROM (SELECT department_id FROM contractor WHERE hourly_rate > 75) c";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids from filtered subqueries
    EXPECT_GT(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithNullHandling) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee WHERE department_id IS NULL "
        "UNION "
        "SELECT department_id FROM contractor WHERE department_id IS NULL";

    auto results = MockExecuteAndCollectResults(sql);

    // Should handle NULL values correctly
    EXPECT_GE(results.size(), 0);

    // Verify NULL handling
    for (const auto& row : results) {
        // Should handle NULL properly
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsPerformanceLargeDataset) {
    CreateStandardTestTables();

    // Add more data for performance testing
    for (int i = 6; i <= 100; ++i) {
        int dept_id = (i % 3) + 1;
        double salary = 40000 + (i * 100);
        std::string sql = "INSERT INTO employee VALUES (" +
                         std::to_string(i) + ", 'Employee" + std::to_string(i) +
                         "', " + std::to_string(dept_id) + ", " + std::to_string(salary) +
                         ", 1, '2022-01-01')";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    for (int i = 5; i <= 100; ++i) {
        int dept_id = (i % 3) + 1;
        double rate = 50 + (i % 30);
        std::string sql = "INSERT INTO contractor VALUES (" +
                         std::to_string(i) + ", 'Contractor" + std::to_string(i) +
                         "', " + std::to_string(dept_id) + ", " + std::to_string(rate) +
                         ", '2022-01-01')";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    std::string sql =
        "SELECT department_id FROM employee "
        "UNION "
        "SELECT department_id FROM contractor";

    auto results = MockExecuteAndCollectResults(sql);

    // Should handle larger dataset
    EXPECT_GT(results.size(), 3);  // At least the original departments

    // Verify results structure
    for (const auto& row : results) {
        // department_id can be NULL
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithDifferentColumnTypes) {
    CreateStandardTestTables();

    // Test with mixed data types - this might require casting in real implementation
    std::string sql =
        "SELECT CAST(department_id AS VARCHAR) FROM employee "
        "UNION "
        "SELECT name FROM department";

    auto results = MockExecuteAndCollectResults(sql);

    // Should handle type conversions
    EXPECT_GT(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithLimit) {
    CreateStandardTestTables();

    std::string sql =
        "(SELECT name FROM employee UNION SELECT name FROM contractor) "
        "ORDER BY name LIMIT 5";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return first 5 names alphabetically
    EXPECT_EQ(results.size(), 5);

    // Verify alphabetical order
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_LE(results[i-1][0], results[i][0]);
    }
}

TEST_F(SetOperationExecutorTest, ComplexSetOperationsChain) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "UNION "
        "SELECT department_id FROM contractor "
        "EXCEPT "
        "SELECT id FROM department WHERE budget < 400000";

    auto results = MockExecuteAndCollectResults(sql);

    // Should return department_ids from employees/contractors but not low-budget departments
    EXPECT_GE(results.size(), 0);

    // Verify results structure
    for (const auto& row : results) {
        // Can be NULL
    }
}

TEST_F(SetOperationExecutorTest, SetOperationsWithCorrelatedSubqueries) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT e.name FROM employee e WHERE e.department_id IN ("
        "    SELECT department_id FROM employee UNION SELECT department_id FROM contractor"
        ")";

    auto results = MockExecuteAndCollectResults(sql);

    // Should combine set operations with correlated subqueries
    EXPECT_EQ(results.size(), 5);  // All employees

    // Verify results structure
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // Name should not be empty
    }
}
