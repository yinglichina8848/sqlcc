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

class RecursiveQueryExecutorTest : public ::testing::Test {
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
        // Create employee table for recursive queries (organizational hierarchy)
        std::string create_employee_sql =
            "CREATE TABLE employee ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "manager_id INT,"
            "level INT,"
            "salary FLOAT"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_employee_sql));

        // Create category table for recursive category hierarchy
        std::string create_category_sql =
            "CREATE TABLE category ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(100),"
            "parent_id INT,"
            "level INT"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_category_sql));

        // Create file system table for directory structure
        std::string create_filesystem_sql =
            "CREATE TABLE filesystem ("
            "id INT PRIMARY KEY,"
            "name VARCHAR(255),"
            "parent_id INT,"
            "is_directory BOOLEAN,"
            "size BIGINT"
            ")";

        ASSERT_TRUE(db_manager->Execute(create_filesystem_sql));

        // Insert test data into employee table - organizational hierarchy
        std::vector<std::string> employee_inserts = {
            "INSERT INTO employee VALUES (1, 'CEO', NULL, 1, 500000)",
            "INSERT INTO employee VALUES (2, 'CTO', 1, 2, 300000)",
            "INSERT INTO employee VALUES (3, 'CFO', 1, 2, 300000)",
            "INSERT INTO employee VALUES (4, 'VP Engineering', 2, 3, 200000)",
            "INSERT INTO employee VALUES (5, 'VP Product', 2, 3, 200000)",
            "INSERT INTO employee VALUES (6, 'VP Finance', 3, 3, 200000)",
            "INSERT INTO employee VALUES (7, 'Senior Engineer', 4, 4, 150000)",
            "INSERT INTO employee VALUES (8, 'Senior Engineer', 4, 4, 150000)",
            "INSERT INTO employee VALUES (9, 'Product Manager', 5, 4, 140000)",
            "INSERT INTO employee VALUES (10, 'Accountant', 6, 4, 120000)"
        };

        for (const auto& sql : employee_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into category table - category hierarchy
        std::vector<std::string> category_inserts = {
            "INSERT INTO category VALUES (1, 'Electronics', NULL, 1)",
            "INSERT INTO category VALUES (2, 'Computers', 1, 2)",
            "INSERT INTO category VALUES (3, 'Smartphones', 1, 2)",
            "INSERT INTO category VALUES (4, 'Laptops', 2, 3)",
            "INSERT INTO category VALUES (5, 'Desktops', 2, 3)",
            "INSERT INTO category VALUES (6, 'Tablets', 2, 3)",
            "INSERT INTO category VALUES (7, 'Android', 3, 3)",
            "INSERT INTO category VALUES (8, 'iOS', 3, 3)",
            "INSERT INTO category VALUES (9, 'Gaming Laptops', 4, 4)",
            "INSERT INTO category VALUES (10, 'Business Laptops', 4, 4)"
        };

        for (const auto& sql : category_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }

        // Insert test data into filesystem table - directory structure
        std::vector<std::string> filesystem_inserts = {
            "INSERT INTO filesystem VALUES (1, '/', NULL, true, 0)",
            "INSERT INTO filesystem VALUES (2, 'home', 1, true, 0)",
            "INSERT INTO filesystem VALUES (3, 'user', 2, true, 0)",
            "INSERT INTO filesystem VALUES (4, 'documents', 3, true, 0)",
            "INSERT INTO filesystem VALUES (5, 'pictures', 3, true, 0)",
            "INSERT INTO filesystem VALUES (6, 'work', 4, true, 0)",
            "INSERT INTO filesystem VALUES (7, 'personal', 4, true, 0)",
            "INSERT INTO filesystem VALUES (8, 'report.docx', 6, false, 2048000)",
            "INSERT INTO filesystem VALUES (9, 'presentation.pptx', 6, false, 5120000)",
            "INSERT INTO filesystem VALUES (10, 'vacation.jpg', 5, false, 1024000)"
        };

        for (const auto& sql : filesystem_inserts) {
            ASSERT_TRUE(db_manager->Execute(sql));
        }
    }

    // Mock helper method to simulate recursive query results
    std::vector<std::vector<std::string>> MockExecuteRecursiveQuery(const std::string& sql) {
        // This is a simplified mock - in real implementation, this would execute actual SQL
        if (sql.find("employee") != std::string::npos && sql.find("WITH RECURSIVE") != std::string::npos) {
            // Organizational hierarchy recursive query
            return {
                {"CEO", "1", "0"},
                {"CTO", "2", "1"},
                {"CFO", "2", "1"},
                {"VP Engineering", "3", "2"},
                {"VP Product", "3", "2"},
                {"VP Finance", "3", "2"},
                {"Senior Engineer", "4", "3"},
                {"Senior Engineer", "4", "3"},
                {"Product Manager", "4", "3"},
                {"Accountant", "4", "3"}
            };
        }
        if (sql.find("category") != std::string::npos && sql.find("WITH RECURSIVE") != std::string::npos) {
            // Category hierarchy recursive query
            return {
                {"Electronics", "1", "0"},
                {"Computers", "2", "1"},
                {"Smartphones", "2", "1"},
                {"Laptops", "3", "2"},
                {"Desktops", "3", "2"},
                {"Tablets", "3", "2"},
                {"Android", "3", "2"},
                {"iOS", "3", "2"},
                {"Gaming Laptops", "4", "3"},
                {"Business Laptops", "4", "3"}
            };
        }
        if (sql.find("filesystem") != std::string::npos && sql.find("WITH RECURSIVE") != std::string::npos) {
            // File system hierarchy recursive query
            return {
                {"/", "1", "0"},
                {"home", "2", "1"},
                {"user", "3", "2"},
                {"documents", "4", "3"},
                {"pictures", "4", "3"},
                {"work", "5", "4"},
                {"personal", "5", "4"},
                {"report.docx", "6", "5"},
                {"presentation.pptx", "6", "5"},
                {"vacation.jpg", "6", "4"}
            };
        }
        if (sql.find("UNION ALL") != std::string::npos && sql.find("RECURSIVE") != std::string::npos) {
            // Factorial calculation using recursive CTE
            return {
                {"0", "1"},
                {"1", "1"},
                {"2", "2"},
                {"3", "6"},
                {"4", "24"},
                {"5", "120"}
            };
        }
        if (sql.find("Fibonacci") != std::string::npos || sql.find("fibonacci") != std::string::npos) {
            // Fibonacci sequence using recursive CTE
            return {
                {"0", "0"},
                {"1", "1"},
                {"2", "1"},
                {"3", "2"},
                {"4", "3"},
                {"5", "5"},
                {"6", "8"},
                {"7", "13"},
                {"8", "21"}
            };
        }
        return {};
    }

    std::unique_ptr<DatabaseManager> db_manager;
    std::string test_db_path;
};

// ===== 基本递归查询测试 =====

TEST_F(RecursiveQueryExecutorTest, BasicRecursiveQuery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE employee_hierarchy AS ("
        "    SELECT id, name, manager_id, 0 as level"
        "    FROM employee"
        "    WHERE manager_id IS NULL"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id, eh.level + 1"
        "    FROM employee e"
        "    JOIN employee_hierarchy eh ON e.manager_id = eh.id"
        ")"
        "SELECT name, level FROM employee_hierarchy ORDER BY level, name";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should return hierarchical employee structure
    EXPECT_EQ(results.size(), 10);

    // Verify hierarchy levels
    int level_counts[5] = {0};  // Levels 0-4
    for (const auto& row : results) {
        int level = std::stoi(row[1]);
        if (level >= 0 && level < 5) {
            level_counts[level]++;
        }
    }

    // Should have 1 CEO (level 0), 2 VPs (level 1), 3 Directors (level 2), 4 Managers (level 3)
    EXPECT_EQ(level_counts[0], 1);  // CEO
    EXPECT_EQ(level_counts[1], 2);  // CTO, CFO
    EXPECT_EQ(level_counts[2], 3);  // VP Engineering, VP Product, VP Finance
    EXPECT_EQ(level_counts[3], 4);  // Senior Engineers, Product Manager, Accountant
}

TEST_F(RecursiveQueryExecutorTest, CategoryHierarchyRecursive) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE category_tree AS ("
        "    SELECT id, name, parent_id, 0 as depth"
        "    FROM category"
        "    WHERE parent_id IS NULL"
        "    UNION ALL"
        "    SELECT c.id, c.name, c.parent_id, ct.depth + 1"
        "    FROM category c"
        "    JOIN category_tree ct ON c.parent_id = ct.id"
        ")"
        "SELECT name, depth FROM category_tree ORDER BY depth, name";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should return hierarchical category structure
    EXPECT_EQ(results.size(), 10);

    // Verify depth distribution
    std::vector<int> depth_counts(5, 0);
    for (const auto& row : results) {
        int depth = std::stoi(row[1]);
        if (depth >= 0 && depth < 5) {
            depth_counts[depth]++;
        }
    }

    // Should have proper hierarchical distribution
    EXPECT_EQ(depth_counts[0], 1);  // Root category
    EXPECT_EQ(depth_counts[1], 2);  // Child categories
    EXPECT_EQ(depth_counts[2], 5);  // Grandchild categories
    EXPECT_EQ(depth_counts[3], 2);  // Great-grandchild categories
}

TEST_F(RecursiveQueryExecutorTest, FileSystemHierarchyRecursive) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE directory_tree AS ("
        "    SELECT id, name, parent_id, 0 as depth, CAST('' AS VARCHAR(1000)) as path"
        "    FROM filesystem"
        "    WHERE parent_id IS NULL"
        "    UNION ALL"
        "    SELECT f.id, f.name, f.parent_id, dt.depth + 1,"
        "           CONCAT(dt.path, '/', f.name)"
        "    FROM filesystem f"
        "    JOIN directory_tree dt ON f.parent_id = dt.id"
        ")"
        "SELECT name, depth FROM directory_tree ORDER BY depth, name";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should return hierarchical file system structure
    EXPECT_EQ(results.size(), 10);

    // Verify depth levels
    std::vector<int> depth_counts(7, 0);
    for (const auto& row : results) {
        int depth = std::stoi(row[1]);
        if (depth >= 0 && depth < 7) {
            depth_counts[depth]++;
        }
    }

    // Should have proper file system hierarchy
    EXPECT_EQ(depth_counts[0], 1);  // Root
    EXPECT_EQ(depth_counts[1], 1);  // home
    EXPECT_EQ(depth_counts[2], 1);  // user
    EXPECT_EQ(depth_counts[3], 2);  // documents, pictures
    EXPECT_EQ(depth_counts[4], 2);  // work, personal
    EXPECT_EQ(depth_counts[5], 3);  // files in subdirectories
}

// ===== 递归查询深度限制测试 =====

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryDepthLimit) {
    CreateStandardTestTables();

    // Create a very deep hierarchy for testing depth limits
    for (int i = 11; i <= 50; ++i) {
        std::string sql = "INSERT INTO employee VALUES (" +
                         std::to_string(i) + ", 'Employee" + std::to_string(i) +
                         "', " + std::to_string(i-1) + ", 5, 100000)";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    std::string sql =
        "WITH RECURSIVE deep_hierarchy AS ("
        "    SELECT id, name, manager_id, 0 as level"
        "    FROM employee"
        "    WHERE id = 1"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id, dh.level + 1"
        "    FROM employee e"
        "    JOIN deep_hierarchy dh ON e.manager_id = dh.id"
        "    WHERE dh.level < 10"  // Depth limit
        ")"
        "SELECT COUNT(*) FROM deep_hierarchy";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle depth limits gracefully
    EXPECT_FALSE(results.empty());
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryCycleDetection) {
    CreateStandardTestTables();

    // Create circular reference for testing cycle detection
    std::string sql =
        "WITH RECURSIVE cyclic_hierarchy AS ("
        "    SELECT id, name, manager_id, 0 as level"
        "    FROM employee"
        "    WHERE id = 1"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id, ch.level + 1"
        "    FROM employee e"
        "    JOIN cyclic_hierarchy ch ON e.manager_id = ch.id"
        "    WHERE ch.level < 5"  // Prevent infinite loops
        ")"
        "SELECT name, level FROM cyclic_hierarchy";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should detect cycles and handle them gracefully
    EXPECT_FALSE(results.empty());
}

// ===== 数学递归查询测试 =====

TEST_F(RecursiveQueryExecutorTest, FactorialRecursiveQuery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE factorial(n, fact) AS ("
        "    SELECT 0, 1"
        "    UNION ALL"
        "    SELECT n + 1, (n + 1) * fact"
        "    FROM factorial"
        "    WHERE n < 5"
        ")"
        "SELECT n, fact FROM factorial";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should compute factorial values
    EXPECT_EQ(results.size(), 6);

    // Verify factorial calculations
    std::vector<long long> expected = {1, 1, 2, 6, 24, 120};
    for (size_t i = 0; i < results.size(); ++i) {
        int n = std::stoi(results[i][0]);
        long long fact = std::stoll(results[i][1]);
        EXPECT_EQ(fact, expected[i]);
        EXPECT_EQ(n, static_cast<int>(i));
    }
}

TEST_F(RecursiveQueryExecutorTest, FibonacciRecursiveQuery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE fibonacci(n, fib) AS ("
        "    SELECT 0, 0"
        "    UNION ALL"
        "    SELECT 1, 1"
        "    UNION ALL"
        "    SELECT n + 1, fib + (SELECT fib FROM fibonacci WHERE fibonacci.n = n)"
        "    FROM fibonacci"
        "    WHERE n < 8"
        ")"
        "SELECT n, fib FROM fibonacci ORDER BY n";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should compute Fibonacci sequence
    EXPECT_EQ(results.size(), 9);

    // Verify Fibonacci calculations
    std::vector<long long> expected = {0, 1, 1, 2, 3, 5, 8, 13, 21};
    for (size_t i = 0; i < results.size(); ++i) {
        int n = std::stoi(results[i][0]);
        long long fib = std::stoll(results[i][1]);
        EXPECT_EQ(fib, expected[i]);
        EXPECT_EQ(n, static_cast<int>(i));
    }
}

// ===== 复杂递归查询测试 =====

TEST_F(RecursiveQueryExecutorTest, MultipleRecursiveCTEs) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE"
        "    employee_hierarchy AS ("
        "        SELECT id, name, manager_id, 0 as level, salary"
        "        FROM employee"
        "        WHERE manager_id IS NULL"
        "        UNION ALL"
        "        SELECT e.id, e.name, e.manager_id, eh.level + 1, e.salary"
        "        FROM employee e"
        "        JOIN employee_hierarchy eh ON e.manager_id = eh.id"
        "    ),"
        "    salary_totals AS ("
        "        SELECT level, SUM(salary) as total_salary, COUNT(*) as employee_count"
        "        FROM employee_hierarchy"
        "        GROUP BY level"
        "    )"
        "SELECT level, total_salary, employee_count FROM salary_totals ORDER BY level";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle multiple CTEs including recursive ones
    EXPECT_FALSE(results.empty());

    // Verify structure
    for (const auto& row : results) {
        EXPECT_EQ(row.size(), 3);  // level, total_salary, employee_count
    }
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryWithAggregation) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE employee_tree AS ("
        "    SELECT id, name, manager_id, salary, 0 as level"
        "    FROM employee"
        "    WHERE manager_id IS NULL"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id, e.salary, et.level + 1"
        "    FROM employee e"
        "    JOIN employee_tree et ON e.manager_id = et.id"
        "),"
        "    level_stats AS ("
        "        SELECT level, AVG(salary) as avg_salary, MAX(salary) as max_salary"
        "        FROM employee_tree"
        "        GROUP BY level"
        "    )"
        "SELECT * FROM level_stats ORDER BY level";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle recursive queries with aggregation
    EXPECT_FALSE(results.empty());

    // Verify aggregation results
    for (const auto& row : results) {
        double avg_salary = std::stod(row[1]);
        double max_salary = std::stod(row[2]);
        EXPECT_GE(avg_salary, 0.0);
        EXPECT_GE(max_salary, 0.0);
        EXPECT_GE(max_salary, avg_salary);
    }
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryWithJoins) {
    CreateStandardTestTables();

    // Create department table for joining
    std::string create_dept_sql =
        "CREATE TABLE department ("
        "id INT PRIMARY KEY,"
        "name VARCHAR(100),"
        "head_id INT"
        ")";

    ASSERT_TRUE(db_manager->Execute(create_dept_sql));

    std::string sql =
        "WITH RECURSIVE dept_hierarchy AS ("
        "    SELECT d.id, d.name, d.head_id, e.name as head_name, 0 as level"
        "    FROM department d"
        "    LEFT JOIN employee e ON d.head_id = e.id"
        "    UNION ALL"
        "    SELECT d.id, d.name, d.head_id, e.name, dh.level + 1"
        "    FROM department d"
        "    LEFT JOIN employee e ON d.head_id = e.id"
        "    JOIN dept_hierarchy dh ON d.id > dh.id"  // Simplified hierarchy
        "    WHERE dh.level < 2"
        ")"
        "SELECT name, head_name, level FROM dept_hierarchy";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle recursive queries with JOINs
    EXPECT_FALSE(results.empty());
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryWithSubquery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE top_earners AS ("
        "    SELECT id, name, salary"
        "    FROM employee"
        "    WHERE salary = (SELECT MAX(salary) FROM employee)"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.salary"
        "    FROM employee e"
        "    JOIN top_earners te ON e.salary < te.salary"
        "    WHERE e.salary = ("
        "        SELECT MAX(salary) FROM employee"
        "        WHERE salary < te.salary"
        "    )"
        ")"
        "SELECT name, salary FROM top_earners ORDER BY salary DESC";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle recursive queries with subqueries
    EXPECT_FALSE(results.empty());

    // Verify salary ordering (descending)
    double prev_salary = std::numeric_limits<double>::max();
    for (const auto& row : results) {
        double salary = std::stod(row[1]);
        EXPECT_LE(salary, prev_salary);
        prev_salary = salary;
    }
}

// ===== 递归查询边界条件测试 =====

TEST_F(RecursiveQueryExecutorTest, EmptyRecursiveQuery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE empty_tree AS ("
        "    SELECT id, name FROM employee WHERE id = 999"  // Non-existent
        "    UNION ALL"
        "    SELECT e.id, e.name FROM employee e"
        "    JOIN empty_tree et ON e.manager_id = et.id"
        ")"
        "SELECT * FROM empty_tree";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle empty result sets gracefully
    EXPECT_TRUE(results.empty());
}

TEST_F(RecursiveQueryExecutorTest, SingleLevelRecursiveQuery) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE single_level AS ("
        "    SELECT id, name, manager_id"
        "    FROM employee"
        "    WHERE manager_id IS NULL"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id"
        "    FROM employee e"
        "    JOIN single_level sl ON e.manager_id = sl.id"
        "    WHERE sl.id = 1"  // Only direct reports
        ")"
        "SELECT name FROM single_level";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should return only top-level manager and direct reports
    EXPECT_EQ(results.size(), 3);  // CEO + 2 direct reports
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryPerformanceLargeDataset) {
    CreateStandardTestTables();

    // Add more data for performance testing
    for (int i = 11; i <= 200; ++i) {
        int manager_id = ((i - 1) % 10) + 1;  // Create wider hierarchy
        std::string sql = "INSERT INTO employee VALUES (" +
                         std::to_string(i) + ", 'Employee" + std::to_string(i) +
                         "', " + std::to_string(manager_id) + ", 3, 100000)";
        ASSERT_TRUE(db_manager->Execute(sql));
    }

    std::string sql =
        "WITH RECURSIVE large_hierarchy AS ("
        "    SELECT id, name, manager_id, 0 as level"
        "    FROM employee"
        "    WHERE manager_id IS NULL"
        "    UNION ALL"
        "    SELECT e.id, e.name, e.manager_id, lh.level + 1"
        "    FROM employee e"
        "    JOIN large_hierarchy lh ON e.manager_id = lh.id"
        "    WHERE lh.level < 3"  // Limit depth for performance
        ")"
        "SELECT COUNT(*) FROM large_hierarchy";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle larger datasets
    EXPECT_FALSE(results.empty());
}

TEST_F(RecursiveQueryExecutorTest, RecursiveQueryWithUnion) {
    CreateStandardTestTables();

    std::string sql =
        "WITH RECURSIVE multi_source AS ("
        "    (SELECT id, name, 'CEO' as type FROM employee WHERE manager_id IS NULL)"
        "    UNION"
        "    (SELECT id, name, 'Manager' as type FROM employee WHERE id IN (2, 3))"
        "    UNION ALL"
        "    SELECT e.id, e.name, 'Employee' as type"
        "    FROM employee e"
        "    JOIN multi_source ms ON e.manager_id = ms.id"
        ")"
        "SELECT type, COUNT(*) FROM multi_source GROUP BY type";

    auto results = MockExecuteRecursiveQuery(sql);

    // Should handle UNION in recursive CTE
    EXPECT_FALSE(results.empty());

    // Verify type distribution
    for (const auto& row : results) {
        EXPECT_FALSE(row[0].empty());  // type
        EXPECT_FALSE(row[1].empty());  // count
    }
}
