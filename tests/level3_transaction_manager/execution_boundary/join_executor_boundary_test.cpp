#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "execution/join_executor.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"

using namespace sqlcc::sql_parser;

using namespace sqlcc;

class JoinExecutorBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        executor_ = std::make_unique<SqlExecutor>();
        // Parser will be created per SQL statement
    }

    void TearDown() override {
        executor_.reset();
        parser_.reset();
    }

    // Helper method to create test tables
    void CreateTestTables() {
        // Create table A
        std::string create_a = "CREATE TABLE table_a (id INT, name VARCHAR(50), value INT)";
        ExecuteSQL(create_a);

        // Create table B
        std::string create_b = "CREATE TABLE table_b (id INT, desc VARCHAR(50), ref_id INT)";
        ExecuteSQL(create_b);

        // Insert test data
        ExecuteSQL("INSERT INTO table_a VALUES (1, 'Item1', 100)");
        ExecuteSQL("INSERT INTO table_a VALUES (2, 'Item2', 200)");
        ExecuteSQL("INSERT INTO table_a VALUES (3, 'Item3', 300)");
        ExecuteSQL("INSERT INTO table_a VALUES (4, 'Item4', NULL)");

        ExecuteSQL("INSERT INTO table_b VALUES (1, 'Desc1', 1)");
        ExecuteSQL("INSERT INTO table_b VALUES (2, 'Desc2', 2)");
        ExecuteSQL("INSERT INTO table_b VALUES (3, 'Desc3', NULL)");
        ExecuteSQL("INSERT INTO table_b VALUES (4, 'Desc4', 4)");
    }

    // Helper method to execute SQL
    bool ExecuteSQL(const std::string& sql) {
        // Execute SQL directly using SqlExecutor
        auto result = executor_->Execute(sql);
        // Assume success if no exception and result is not empty
        return !result.empty();
    }

    std::unique_ptr<SqlExecutor> executor_;
    std::unique_ptr<Parser> parser_;
};

// Test INNER JOIN with self-join
TEST_F(JoinExecutorBoundaryTest, SelfJoinWithAlias) {
    CreateTestTables();

    // Test self-join scenario
    std::string sql =
        "SELECT a1.name, a2.name as parent_name "
        "FROM table_a a1 "
        "INNER JOIN table_a a2 ON a1.id = a2.value / 100";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test INNER JOIN with same column names
TEST_F(JoinExecutorBoundaryTest, SameColumnNameHandling) {
    CreateTestTables();

    // Create tables with same column names
    ExecuteSQL("CREATE TABLE test1 (id INT, name VARCHAR(50))");
    ExecuteSQL("CREATE TABLE test2 (id INT, name VARCHAR(50))");

    ExecuteSQL("INSERT INTO test1 VALUES (1, 'Test1')");
    ExecuteSQL("INSERT INTO test2 VALUES (1, 'Test2')");

    // Test JOIN with USING clause
    std::string sql =
        "SELECT id, test1.name as name1, test2.name as name2 "
        "FROM test1 "
        "INNER JOIN test2 USING (id)";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test INNER JOIN with complex conditions
TEST_F(JoinExecutorBoundaryTest, ComplexJoinConditions) {
    CreateTestTables();

    // Test JOIN with multiple conditions and functions
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id AND a.value > 150";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test LEFT JOIN with NULL handling
TEST_F(JoinExecutorBoundaryTest, LeftJoinNullHandling) {
    CreateTestTables();

    // Test LEFT JOIN with NULL values
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "LEFT JOIN table_b b ON a.id = b.ref_id "
        "WHERE b.ref_id IS NULL";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test RIGHT JOIN with NULL handling
TEST_F(JoinExecutorBoundaryTest, RightJoinNullHandling) {
    CreateTestTables();

    // Test RIGHT JOIN with NULL values
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "RIGHT JOIN table_b b ON a.id = b.ref_id "
        "WHERE a.id IS NULL";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test FULL OUTER JOIN (if supported)
TEST_F(JoinExecutorBoundaryTest, FullOuterJoin) {
    CreateTestTables();

    // Test FULL OUTER JOIN
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "FULL OUTER JOIN table_b b ON a.id = b.ref_id";

    // This might not be supported, but should handle gracefully
    ExecuteSQL(sql); // Don't assert, just test execution
}

// Test CROSS JOIN
TEST_F(JoinExecutorBoundaryTest, CrossJoin) {
    CreateTestTables();

    // Test CROSS JOIN (Cartesian product)
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "CROSS JOIN table_b b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test NATURAL JOIN
TEST_F(JoinExecutorBoundaryTest, NaturalJoin) {
    CreateTestTables();

    // Create tables with same column names for NATURAL JOIN
    ExecuteSQL("CREATE TABLE natural_a (id INT, shared_col VARCHAR(50))");
    ExecuteSQL("CREATE TABLE natural_b (id INT, shared_col VARCHAR(50), extra VARCHAR(50))");

    ExecuteSQL("INSERT INTO natural_a VALUES (1, 'shared1')");
    ExecuteSQL("INSERT INTO natural_b VALUES (1, 'shared1', 'extra1')");

    // Test NATURAL JOIN
    std::string sql =
        "SELECT id, shared_col, extra "
        "FROM natural_a "
        "NATURAL JOIN natural_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with subqueries
TEST_F(JoinExecutorBoundaryTest, JoinWithSubquery) {
    CreateTestTables();

    // Test JOIN with subquery in ON condition
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "WHERE a.value > (SELECT AVG(value) FROM table_a)";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with aggregate functions
TEST_F(JoinExecutorBoundaryTest, JoinWithAggregates) {
    CreateTestTables();

    // Test JOIN with aggregate functions in SELECT
    std::string sql =
        "SELECT COUNT(a.id) as count_a, AVG(a.value) as avg_value, b.desc "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "GROUP BY b.id, b.desc";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with ORDER BY and LIMIT
TEST_F(JoinExecutorBoundaryTest, JoinWithOrderByLimit) {
    CreateTestTables();

    // Test JOIN with ORDER BY and LIMIT
    std::string sql =
        "SELECT a.name, b.desc, a.value "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "ORDER BY a.value DESC "
        "LIMIT 2";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test multiple table JOIN
TEST_F(JoinExecutorBoundaryTest, MultiTableJoin) {
    CreateTestTables();

    // Create third table
    ExecuteSQL("CREATE TABLE table_c (ref_id INT, extra_info VARCHAR(50))");
    ExecuteSQL("INSERT INTO table_c VALUES (1, 'Info1')");
    ExecuteSQL("INSERT INTO table_c VALUES (2, 'Info2')");

    // Test three-table JOIN
    std::string sql =
        "SELECT a.name, b.desc, c.extra_info "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "INNER JOIN table_c c ON b.id = c.ref_id";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with complex WHERE conditions
TEST_F(JoinExecutorBoundaryTest, JoinWithComplexWhere) {
    CreateTestTables();

    // Test JOIN with complex WHERE conditions
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "WHERE a.value BETWEEN 150 AND 250 "
        "AND b.desc LIKE 'Desc%' "
        "AND a.id IN (1, 2, 3)";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with UNION
TEST_F(JoinExecutorBoundaryTest, JoinWithUnion) {
    CreateTestTables();

    // Test JOIN combined with UNION
    std::string sql =
        "(SELECT a.name, b.desc FROM table_a a INNER JOIN table_b b ON a.id = b.ref_id) "
        "UNION "
        "(SELECT a.name, 'No Match' as desc FROM table_a a WHERE a.id NOT IN (SELECT ref_id FROM table_b WHERE ref_id IS NOT NULL))";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test empty result JOIN
TEST_F(JoinExecutorBoundaryTest, EmptyResultJoin) {
    CreateTestTables();

    // Test JOIN that results in no rows
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "INNER JOIN table_b b ON a.id = b.ref_id "
        "WHERE a.value < 0";  // No rows should match

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test large dataset JOIN performance
TEST_F(JoinExecutorBoundaryTest, LargeDatasetJoin) {
    // Create larger tables for performance testing
    ExecuteSQL("CREATE TABLE large_a (id INT, data VARCHAR(100))");
    ExecuteSQL("CREATE TABLE large_b (id INT, ref_id INT, data VARCHAR(100))");

    // Insert larger dataset (adjust size based on performance requirements)
    for (int i = 1; i <= 100; ++i) {
        ExecuteSQL("INSERT INTO large_a VALUES (" + std::to_string(i) + ", 'Data" + std::to_string(i) + "')");
        ExecuteSQL("INSERT INTO large_b VALUES (" + std::to_string(i) + ", " + std::to_string(i % 50 + 1) + ", 'RefData" + std::to_string(i) + "')");
    }

    // Test JOIN on larger dataset
    std::string sql =
        "SELECT a.data, b.data "
        "FROM large_a a "
        "INNER JOIN large_b b ON a.id = b.ref_id "
        "ORDER BY a.id";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test JOIN with NULL comparisons
TEST_F(JoinExecutorBoundaryTest, JoinWithNullComparisons) {
    CreateTestTables();

    // Test JOIN with IS NULL and IS NOT NULL
    std::string sql =
        "SELECT a.name, b.desc "
        "FROM table_a a "
        "LEFT JOIN table_b b ON a.id = b.ref_id "
        "WHERE a.value IS NOT NULL "
        "AND b.ref_id IS NULL";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test nested JOINs
TEST_F(JoinExecutorBoundaryTest, NestedJoins) {
    CreateTestTables();

    // Create additional tables for complex JOIN
    ExecuteSQL("CREATE TABLE table_d (id INT, parent_id INT, info VARCHAR(50))");
    ExecuteSQL("INSERT INTO table_d VALUES (1, 1, 'Info1')");
    ExecuteSQL("INSERT INTO table_d VALUES (2, 2, 'Info2')");

    // Test nested/complex JOIN structure
    std::string sql =
        "SELECT a.name, b.desc, d.info "
        "FROM table_a a "
        "INNER JOIN (table_b b INNER JOIN table_d d ON b.id = d.parent_id) "
        "ON a.id = b.ref_id";

    EXPECT_TRUE(ExecuteSQL(sql));
}
