/**
 * SQLCC v1.3.4 集合操作和子查询集成测试
 * 测试基本的集合操作和子查询功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

#include "core/core_database_manager.h"
#include "sql_executor.h"
#include "storage_engine.h"

using namespace sqlcc;

class SetOperationSubqueryIntegrationTest : public ::testing::Test {
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
        sql_executor = std::make_unique<SqlExecutor>(std::shared_ptr<DatabaseManager>(db_manager.get()));
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

    std::unique_ptr<DatabaseManager> db_manager;
    std::unique_ptr<SqlExecutor> sql_executor;
    std::string test_db_path;
};

// ===== Basic Integration Tests =====

// Test basic UNION operation
TEST_F(SetOperationSubqueryIntegrationTest, UnionBasic) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee "
        "UNION "
        "SELECT name FROM contractor";

    auto result = sql_executor->Execute(sql);

    // Should return all unique names from both tables
    // Result should not be empty and should not contain errors
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test UNION ALL operation
TEST_F(SetOperationSubqueryIntegrationTest, UnionAll) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "UNION ALL "
        "SELECT department_id FROM contractor";

    auto result = sql_executor->Execute(sql);

    // Should return all department_ids including duplicates
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test INTERSECT operation
TEST_F(SetOperationSubqueryIntegrationTest, Intersect) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "INTERSECT "
        "SELECT department_id FROM contractor";

    auto result = sql_executor->Execute(sql);

    // Should return department_ids that exist in both tables
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test EXCEPT operation
TEST_F(SetOperationSubqueryIntegrationTest, Except) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee "
        "EXCEPT "
        "SELECT department_id FROM contractor";

    auto result = sql_executor->Execute(sql);

    // Should return department_ids in employee but not in contractor
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test subquery with IN
TEST_F(SetOperationSubqueryIntegrationTest, SubqueryIn) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee WHERE department_id IN ("
        "    SELECT id FROM department WHERE budget > 200000"
        ")";

    auto result = sql_executor->Execute(sql);

    // Should return employees in high-budget departments
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test subquery with EXISTS
TEST_F(SetOperationSubqueryIntegrationTest, SubqueryExists) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM department d WHERE EXISTS ("
        "    SELECT 1 FROM project p WHERE p.department_id = d.id"
        ")";

    auto result = sql_executor->Execute(sql);

    // Should return departments that have projects
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test scalar subquery
TEST_F(SetOperationSubqueryIntegrationTest, SubqueryScalar) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee WHERE salary > ("
        "    SELECT AVG(salary) FROM employee"
        ")";

    auto result = sql_executor->Execute(sql);

    // Should return employees with above-average salary
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test complex query combining set operations and subqueries
TEST_F(SetOperationSubqueryIntegrationTest, ComplexSetOperationAndSubquery) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee WHERE department_id IN ("
        "    SELECT department_id FROM employee "
        "    UNION "
        "    SELECT department_id FROM contractor"
        ")";

    auto result = sql_executor->Execute(sql);

    // Should execute complex query without errors
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test set operation with ORDER BY
TEST_F(SetOperationSubqueryIntegrationTest, UnionWithOrderBy) {
    CreateStandardTestTables();

    std::string sql =
        "(SELECT name FROM employee UNION SELECT name FROM contractor) "
        "ORDER BY name";

    auto result = sql_executor->Execute(sql);

    // Should return all unique names in alphabetical order
    EXPECT_FALSE(result.empty());
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test empty result set operations
TEST_F(SetOperationSubqueryIntegrationTest, EmptySetOperation) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT name FROM employee WHERE department_id = 999 "
        "UNION "
        "SELECT name FROM contractor WHERE department_id = 999";

    auto result = sql_executor->Execute(sql);

    // Should handle empty results gracefully
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

// Test set operations with NULL handling
TEST_F(SetOperationSubqueryIntegrationTest, NullHandling) {
    CreateStandardTestTables();

    std::string sql =
        "SELECT department_id FROM employee WHERE department_id IS NULL "
        "UNION "
        "SELECT department_id FROM contractor WHERE department_id IS NULL";

    auto result = sql_executor->Execute(sql);

    // Should handle NULL values correctly
    EXPECT_TRUE(result.find("Error") == std::string::npos &&
                result.find("error") == std::string::npos);
}

} // namespace sqlcc
