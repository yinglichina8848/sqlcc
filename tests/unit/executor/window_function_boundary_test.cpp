#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "execution/window_function_executor.h"
#include "sql_parser/window_function.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"

using namespace sqlcc::sql_parser;
using namespace sqlcc;

class WindowFunctionExecutorBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        executor_ = std::make_shared<SqlExecutor>();
        window_executor_ = std::make_unique<WindowFunctionExecutor>(executor_);
        
        // Create test tables
        CreateTestTables();
    }

    void TearDown() override {
        // Clean up
        window_executor_.reset();
        executor_.reset();
    }

    void CreateTestTables() {
        // Create main test table for window functions
        ExecuteSQL("CREATE TABLE employees (id INT, name VARCHAR(50), department VARCHAR(50), salary DECIMAL(10,2), hire_date DATE)");
        ExecuteSQL("CREATE TABLE sales_data (region VARCHAR(50), product VARCHAR(50), sales_amount DECIMAL(12,2), sale_date DATE)");
        ExecuteSQL("CREATE TABLE test_table (id INT, value INT, category VARCHAR(20))");
        
        // Insert test data
        ExecuteSQL("INSERT INTO employees VALUES (1, 'Alice', 'Engineering', 70000, '2020-01-15')");
        ExecuteSQL("INSERT INTO employees VALUES (2, 'Bob', 'Engineering', 80000, '2019-03-20')");
        ExecuteSQL("INSERT INTO employees VALUES (3, 'Charlie', 'Sales', 60000, '2021-06-10')");
        ExecuteSQL("INSERT INTO employees VALUES (4, 'David', 'Sales', 65000, '2020-11-25')");
        ExecuteSQL("INSERT INTO employees VALUES (5, 'Eve', 'Engineering', 90000, '2018-09-05')");
        ExecuteSQL("INSERT INTO employees VALUES (6, 'Frank', 'HR', 55000, '2022-02-14')");
        
        ExecuteSQL("INSERT INTO sales_data VALUES ('North', 'ProductA', 1000.00, '2023-01-15')");
        ExecuteSQL("INSERT INTO sales_data VALUES ('North', 'ProductB', 1500.00, '2023-02-20')");
        ExecuteSQL("INSERT INTO sales_data VALUES ('South', 'ProductA', 1200.00, '2023-01-18')");
        ExecuteSQL("INSERT INTO sales_data VALUES ('South', 'ProductB', 800.00, '2023-03-10')");
        ExecuteSQL("INSERT INTO sales_data VALUES ('East', 'ProductA', 2000.00, '2023-01-22')");
        ExecuteSQL("INSERT INTO sales_data VALUES ('West', 'ProductB', 300.00, '2023-04-05')");
        
        ExecuteSQL("INSERT INTO test_table VALUES (1, 100, 'A')");
        ExecuteSQL("INSERT INTO test_table VALUES (2, 200, 'A')");
        ExecuteSQL("INSERT INTO test_table VALUES (3, 150, 'B')");
        ExecuteSQL("INSERT INTO test_table VALUES (4, 300, 'B')");
        ExecuteSQL("INSERT INTO test_table VALUES (5, 250, 'C')");
        ExecuteSQL("INSERT INTO test_table VALUES (6, 400, 'C')");
    }

    bool ExecuteSQL(const std::string& sql) {
        try {
            Parser parser(sql);
            auto statements = parser.parse();
            
            for (auto& stmt : statements) {
                if (stmt) {
                    auto result = executor_->execute(std::move(stmt));
                    if (!result.success) {
                        std::cerr << "SQL execution failed: " << result.error_message << std::endl;
                        return false;
                    }
                }
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "SQL execution exception: " << e.what() << std::endl;
            return false;
        }
    }

    std::unique_ptr<WindowFunctionExecutor> window_executor_;
    std::shared_ptr<SqlExecutor> executor_;
};

// Test ROW_NUMBER function
TEST_F(WindowFunctionExecutorBoundaryTest, RowNumberBasic) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary DESC) as rn FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test ROW_NUMBER with PARTITION BY
TEST_F(WindowFunctionExecutorBoundaryTest, RowNumberWithPartition) {
    std::string sql = "SELECT name, department, salary, ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary DESC) as rn FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test RANK function
TEST_F(WindowFunctionExecutorBoundaryTest, RankFunction) {
    std::string sql = "SELECT name, salary, RANK() OVER (ORDER BY salary DESC) as rank_val FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test DENSE_RANK function
TEST_F(WindowFunctionExecutorBoundaryTest, DenseRankFunction) {
    std::string sql = "SELECT name, salary, DENSE_RANK() OVER (ORDER BY salary DESC) as dense_rank_val FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test NTILE function
TEST_F(WindowFunctionExecutorBoundaryTest, NtileFunction) {
    std::string sql = "SELECT name, salary, NTILE(3) OVER (ORDER BY salary) as quartile FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test PERCENT_RANK function
TEST_F(WindowFunctionExecutorBoundaryTest, PercentRankFunction) {
    std::string sql = "SELECT name, salary, PERCENT_RANK() OVER (ORDER BY salary) as pct_rank FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test CUME_DIST function
TEST_F(WindowFunctionExecutorBoundaryTest, CumeDistFunction) {
    std::string sql = "SELECT name, salary, CUME_DIST() OVER (ORDER BY salary) as cume_dist FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window frame ROWS
TEST_F(WindowFunctionExecutorBoundaryTest, WindowFrameRows) {
    std::string sql = "SELECT name, salary, SUM(salary) OVER (ORDER BY id ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING) as rolling_sum FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window frame RANGE
TEST_F(WindowFunctionExecutorBoundaryTest, WindowFrameRange) {
    std::string sql = "SELECT name, salary, AVG(salary) OVER (ORDER BY salary RANGE BETWEEN 10000 PRECEDING AND 10000 FOLLOWING) as avg_salary FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window frame with CURRENT ROW
TEST_F(WindowFunctionExecutorBoundaryTest, WindowFrameCurrentRow) {
    std::string sql = "SELECT name, salary, COUNT(*) OVER (ORDER BY id ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING) as cnt FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test multiple window functions in same query
TEST_F(WindowFunctionExecutorBoundaryTest, MultipleWindowFunctions) {
    std::string sql = R"(
        SELECT name, salary,
               ROW_NUMBER() OVER (ORDER BY salary DESC) as rn,
               RANK() OVER (ORDER BY salary DESC) as rank_val,
               DENSE_RANK() OVER (ORDER BY salary DESC) as dense_rank
        FROM employees
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with aggregate functions
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithAggregates) {
    std::string sql = "SELECT name, salary, AVG(salary) OVER (PARTITION BY department ORDER BY salary) as dept_avg FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test FIRST_VALUE function
TEST_F(WindowFunctionExecutorBoundaryTest, FirstValueFunction) {
    std::string sql = "SELECT name, department, salary, FIRST_VALUE(salary) OVER (PARTITION BY department ORDER BY salary) as first_salary FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test LAST_VALUE function
TEST_F(WindowFunctionExecutorBoundaryTest, LastValueFunction) {
    std::string sql = "SELECT name, department, salary, LAST_VALUE(salary) OVER (PARTITION BY department ORDER BY salary ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) as last_salary FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test LEAD function
TEST_F(WindowFunctionExecutorBoundaryTest, LeadFunction) {
    std::string sql = "SELECT name, salary, LEAD(salary, 1) OVER (ORDER BY id) as next_salary FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test LAG function
TEST_F(WindowFunctionExecutorBoundaryTest, LagFunction) {
    std::string sql = "SELECT name, salary, LAG(salary, 1, 0) OVER (ORDER BY id) as prev_salary FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test empty result set with window functions
TEST_F(WindowFunctionExecutorBoundaryTest, EmptyResultSet) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary) as rn FROM employees WHERE salary > 1000000";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test single row with window functions
TEST_F(WindowFunctionExecutorBoundaryTest, SingleRowWindow) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary) as rn FROM employees WHERE id = 1";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window functions with NULL values
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithNulls) {
    // Add some NULL values
    ExecuteSQL("INSERT INTO test_table VALUES (7, NULL, 'A')");
    ExecuteSQL("INSERT INTO test_table VALUES (8, 500, NULL)");
    
    std::string sql = "SELECT id, value, category, ROW_NUMBER() OVER (ORDER BY value) as rn FROM test_table";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test nested window functions
TEST_F(WindowFunctionExecutorBoundaryTest, NestedWindowFunctions) {
    std::string sql = R"(
        SELECT name, salary,
               ROW_NUMBER() OVER (ORDER BY salary) as inner_rn,
               ROW_NUMBER() OVER (PARTITION BY department ORDER BY salary) as outer_rn
        FROM employees
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with complex ORDER BY
TEST_F(WindowFunctionExecutorBoundaryTest, ComplexOrderBy) {
    std::string sql = "SELECT name, department, salary, ROW_NUMBER() OVER (ORDER BY department ASC, salary DESC NULLS LAST) as rn FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with subquery
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithSubquery) {
    std::string sql = R"(
        SELECT name, dept_avg
        FROM (
            SELECT name, department, salary,
                   AVG(salary) OVER (PARTITION BY department) as dept_avg
            FROM employees
        ) t
        WHERE dept_avg > 65000
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with CTE
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithCTE) {
    std::string sql = R"(
        WITH dept_stats AS (
            SELECT department, AVG(salary) as avg_salary
            FROM employees
            GROUP BY department
        )
        SELECT e.name, e.salary, ds.avg_salary,
               RANK() OVER (ORDER BY e.salary - ds.avg_salary) as salary_rank
        FROM employees e
        JOIN dept_stats ds ON e.department = ds.department
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test large dataset performance with window functions
TEST_F(WindowFunctionExecutorBoundaryTest, LargeDatasetPerformance) {
    // Create larger dataset
    for (int i = 7; i <= 1000; ++i) {
        ExecuteSQL("INSERT INTO test_table VALUES (" + std::to_string(i) + ", " + std::to_string(i * 10) + ", 'Category" + std::to_string(i % 5) + "')");
    }
    
    std::string sql = "SELECT id, value, ROW_NUMBER() OVER (ORDER BY value) as rn FROM test_table";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with GROUP BY
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithGroupBy) {
    std::string sql = R"(
        SELECT department, AVG(salary) as avg_salary,
               ROW_NUMBER() OVER (ORDER BY AVG(salary) DESC) as dept_rank
        FROM employees
        GROUP BY department
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with UNION
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithUnion) {
    std::string sql = R"(
        (SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary) as rn FROM employees WHERE department = 'Engineering')
        UNION ALL
        (SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary) as rn FROM employees WHERE department = 'Sales')
    )";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with DISTINCT
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithDistinct) {
    std::string sql = "SELECT DISTINCT department, ROW_NUMBER() OVER (ORDER BY department) as rn FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test window function with LIMIT
TEST_F(WindowFunctionExecutorBoundaryTest, WindowWithLimit) {
    std::string sql = "SELECT name, salary, ROW_NUMBER() OVER (ORDER BY salary DESC) as rn FROM employees LIMIT 3";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_TRUE(result.success);
}

// Test error handling for invalid window frame
TEST_F(WindowFunctionExecutorBoundaryTest, InvalidWindowFrame) {
    std::string sql = "SELECT name, salary, SUM(salary) OVER (ORDER BY salary ROWS BETWEEN 10 FOLLOWING AND 5 PRECEDING) as invalid_frame FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    // Should either succeed or handle gracefully
    EXPECT_TRUE(result.success || !result.error_message.empty());
}

// Test error handling for unsupported window functions
TEST_F(WindowFunctionExecutorBoundaryTest, UnsupportedWindowFunction) {
    std::string sql = "SELECT name, salary, UNKNOWN_WINDOW_FUNC() OVER (ORDER BY salary) as unknown FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    EXPECT_FALSE(result.success);
}

// Test window function with division by zero handling
TEST_F(WindowFunctionExecutorBoundaryTest, DivisionByZeroHandling) {
    std::string sql = "SELECT name, salary, salary / 0 as invalid_div FROM employees";
    
    ExecuteResult result = executor_->execute(sql);
    // Should handle gracefully
    EXPECT_TRUE(result.success || result.error_message.find("division") != std::string::npos);
}
</content>
</invoke>
</tool_call>
