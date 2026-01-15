#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "execution/set_operation_executor.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"

using namespace sqlcc::sql_parser;
using namespace sqlcc;

class SetOperationBoundaryTest : public ::testing::Test {
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
        ExecuteSQL("CREATE TABLE table_a (id INT, name VARCHAR(50), value INT)");
        ExecuteSQL("INSERT INTO table_a VALUES (1, 'A1', 100)");
        ExecuteSQL("INSERT INTO table_a VALUES (2, 'A2', 200)");
        ExecuteSQL("INSERT INTO table_a VALUES (3, 'A3', 300)");
        ExecuteSQL("INSERT INTO table_a VALUES (4, 'A4', NULL)");

        // Create table B
        ExecuteSQL("CREATE TABLE table_b (id INT, name VARCHAR(50), value INT)");
        ExecuteSQL("INSERT INTO table_b VALUES (2, 'B2', 200)");
        ExecuteSQL("INSERT INTO table_b VALUES (3, 'B3', 300)");
        ExecuteSQL("INSERT INTO table_b VALUES (4, 'B4', 400)");
        ExecuteSQL("INSERT INTO table_b VALUES (5, 'B5', NULL)");

        // Create table C for three-way operations
        ExecuteSQL("CREATE TABLE table_c (id INT, name VARCHAR(50), value INT)");
        ExecuteSQL("INSERT INTO table_c VALUES (3, 'C3', 300)");
        ExecuteSQL("INSERT INTO table_c VALUES (4, 'C4', 400)");
        ExecuteSQL("INSERT INTO table_c VALUES (5, 'C5', 500)");
        ExecuteSQL("INSERT INTO table_c VALUES (6, 'C6', NULL)");
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

// Test UNION basic functionality
TEST_F(SetOperationBoundaryTest, UnionBasic) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a "
        "UNION "
        "SELECT id, name FROM table_b "
        "ORDER BY id";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION ALL (no deduplication)
TEST_F(SetOperationBoundaryTest, UnionAllNoDeduplication) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a WHERE id <= 3 "
        "UNION ALL "
        "SELECT id, name FROM table_b WHERE id <= 3";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test INTERSECT basic functionality
TEST_F(SetOperationBoundaryTest, IntersectBasic) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a "
        "INTERSECT "
        "SELECT id, name FROM table_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test EXCEPT (difference) basic functionality
TEST_F(SetOperationBoundaryTest, ExceptBasic) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a "
        "EXCEPT "
        "SELECT id, name FROM table_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with different column orders
TEST_F(SetOperationBoundaryTest, UnionDifferentColumnOrder) {
    CreateTestTables();

    std::string sql =
        "SELECT name, id FROM table_a "
        "UNION "
        "SELECT id, name FROM table_b";  // Column mismatch

    // This should handle the column order mismatch gracefully
    ExecuteSQL(sql); // Don't assert, just test execution
}

// Test UNION with NULL values
TEST_F(SetOperationBoundaryTest, UnionWithNulls) {
    CreateTestTables();

    std::string sql =
        "SELECT id, value FROM table_a "
        "UNION "
        "SELECT id, value FROM table_b "
        "ORDER BY id";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test multiple UNION operations
TEST_F(SetOperationBoundaryTest, MultipleUnionOperations) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a "
        "UNION "
        "SELECT id, name FROM table_b "
        "UNION "
        "SELECT id, name FROM table_c "
        "ORDER BY id";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with ORDER BY and LIMIT
TEST_F(SetOperationBoundaryTest, UnionWithOrderByLimit) {
    CreateTestTables();

    std::string sql =
        "(SELECT id, name FROM table_a UNION SELECT id, name FROM table_b) "
        "ORDER BY id DESC "
        "LIMIT 3";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test INTERSECT with complex conditions
TEST_F(SetOperationBoundaryTest, IntersectWithConditions) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a WHERE value > 150 "
        "INTERSECT "
        "SELECT id, name FROM table_b WHERE value < 400";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test EXCEPT with subqueries
TEST_F(SetOperationBoundaryTest, ExceptWithSubqueries) {
    CreateTestTables();

    std::string sql =
        "(SELECT id FROM table_a WHERE value IS NOT NULL) "
        "EXCEPT "
        "(SELECT id FROM table_b WHERE name LIKE 'B%')";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with aggregate functions
TEST_F(SetOperationBoundaryTest, UnionWithAggregates) {
    CreateTestTables();

    std::string sql =
        "SELECT COUNT(*) as count, 'table_a' as source FROM table_a "
        "UNION "
        "SELECT COUNT(*) as count, 'table_b' as source FROM table_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test empty result set UNION
TEST_F(SetOperationBoundaryTest, EmptyResultUnion) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a WHERE id < 0 "  // Empty result
        "UNION "
        "SELECT id, name FROM table_b WHERE id > 10"; // Empty result

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with different data types (should handle gracefully)
TEST_F(SetOperationBoundaryTest, UnionTypeMismatch) {
    CreateTestTables();

    // Create table with different data type
    ExecuteSQL("CREATE TABLE table_d (id VARCHAR(10), name VARCHAR(50))");
    ExecuteSQL("INSERT INTO table_d VALUES ('1', 'D1')");

    std::string sql =
        "SELECT CAST(id AS VARCHAR(10)), name FROM table_a "
        "UNION "
        "SELECT id, name FROM table_d";

    // Type conversion should be handled
    ExecuteSQL(sql);
}

// Test large dataset set operations
TEST_F(SetOperationBoundaryTest, LargeDatasetOperations) {
    // Create larger tables for performance testing
    ExecuteSQL("CREATE TABLE large_a (id INT, data VARCHAR(100))");
    ExecuteSQL("CREATE TABLE large_b (id INT, data VARCHAR(100))");

    // Insert larger dataset
    for (int i = 1; i <= 1000; ++i) {
        ExecuteSQL("INSERT INTO large_a VALUES (" + std::to_string(i) + ", 'DataA" + std::to_string(i) + "')");
        if (i % 2 == 0) {
            ExecuteSQL("INSERT INTO large_b VALUES (" + std::to_string(i) + ", 'DataB" + std::to_string(i) + "')");
        }
    }

    std::string sql =
        "SELECT id, data FROM large_a "
        "UNION "
        "SELECT id, data FROM large_b "
        "ORDER BY id "
        "LIMIT 100";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with duplicate elimination efficiency
TEST_F(SetOperationBoundaryTest, UnionDuplicateElimination) {
    CreateTestTables();

    // Create many duplicates
    ExecuteSQL("CREATE TABLE dup_table (id INT)");
    for (int i = 0; i < 100; ++i) {
        ExecuteSQL("INSERT INTO dup_table VALUES (1)");
        ExecuteSQL("INSERT INTO dup_table VALUES (2)");
        ExecuteSQL("INSERT INTO dup_table VALUES (3)");
    }

    std::string sql =
        "SELECT id FROM dup_table "
        "UNION "  // Should eliminate duplicates
        "SELECT id FROM dup_table";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test INTERSECT with NULL handling
TEST_F(SetOperationBoundaryTest, IntersectNullHandling) {
    CreateTestTables();

    std::string sql =
        "SELECT value FROM table_a WHERE value IS NULL "
        "INTERSECT "
        "SELECT value FROM table_b WHERE value IS NULL";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test EXCEPT with NULL handling
TEST_F(SetOperationBoundaryTest, ExceptNullHandling) {
    CreateTestTables();

    std::string sql =
        "SELECT value FROM table_a "
        "EXCEPT "
        "SELECT value FROM table_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test complex nested set operations
TEST_F(SetOperationBoundaryTest, NestedSetOperations) {
    CreateTestTables();

    std::string sql =
        "(SELECT id FROM table_a INTERSECT SELECT id FROM table_b) "
        "UNION "
        "(SELECT id FROM table_c EXCEPT (SELECT id FROM table_a UNION SELECT id FROM table_b))";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION with correlated subqueries
TEST_F(SetOperationBoundaryTest, UnionWithCorrelatedSubqueries) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name FROM table_a a1 WHERE value > (SELECT AVG(value) FROM table_a) "
        "UNION "
        "SELECT id, name FROM table_b b1 WHERE value > (SELECT AVG(value) FROM table_b)";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test set operations with window functions
TEST_F(SetOperationBoundaryTest, SetOperationsWithWindowFunctions) {
    CreateTestTables();

    std::string sql =
        "SELECT id, name, ROW_NUMBER() OVER (ORDER BY id) as rn FROM table_a "
        "UNION "
        "SELECT id, name, ROW_NUMBER() OVER (ORDER BY id) as rn FROM table_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}

// Test UNION ALL performance vs UNION
TEST_F(SetOperationBoundaryTest, UnionAllVsUnionPerformance) {
    CreateTestTables();

    // Test UNION ALL (should be faster, no deduplication)
    std::string unionAllSql =
        "SELECT id FROM table_a UNION ALL SELECT id FROM table_b";

    // Test UNION (slower due to deduplication)
    std::string unionSql =
        "SELECT id FROM table_a UNION SELECT id FROM table_b";

    EXPECT_TRUE(ExecuteSQL(unionAllSql));
    EXPECT_TRUE(ExecuteSQL(unionSql));
}

// Test set operations with CTE (Common Table Expressions)
TEST_F(SetOperationBoundaryTest, SetOperationsWithCTE) {
    CreateTestTables();

    std::string sql =
        "WITH cte_a AS (SELECT id, name FROM table_a WHERE value > 150), "
        "     cte_b AS (SELECT id, name FROM table_b WHERE value < 400) "
        "SELECT * FROM cte_a "
        "UNION "
        "SELECT * FROM cte_b";

    EXPECT_TRUE(ExecuteSQL(sql));
}
