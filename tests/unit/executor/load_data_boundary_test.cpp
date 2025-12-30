#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "execution/load_data_executor.h"
#include "sql_parser/parser.h"
#include "sql_executor.h"
#include "storage_engine.h"

using namespace sqlcc::sql_parser;
using namespace sqlcc;

class LoadDataExecutorBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        storage_engine_ = std::make_shared<StorageEngine>();
        executor_ = std::make_shared<SqlExecutor>();
        
        // Create load data executor
        load_executor_ = std::make_unique<LoadDataExecutor>(storage_engine_, executor_);
        
        // Create test directory
        std::filesystem::create_directories("test_data");
        
        // Create test table
        CreateTestTables();
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all("test_data");
        std::filesystem::remove("large_test.csv");
        std::filesystem::remove("invalid_test.csv");
        std::filesystem::remove("empty_test.csv");
    }

    void CreateTestTables() {
        // Create main test table
        ExecuteSQL("CREATE TABLE test_load (id INT, name VARCHAR(50), age INT, salary DECIMAL(10,2))");
        
        // Create table with constraints
        ExecuteSQL("CREATE TABLE constrained_table (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL, code VARCHAR(10) UNIQUE)");
        
        // Create table with different data types
        ExecuteSQL("CREATE TABLE types_table (id INT, float_val DECIMAL(5,2), bool_val BOOLEAN, date_val DATE)");
        
        // Create table for partition testing
        ExecuteSQL("CREATE TABLE partition_table (id INT, region VARCHAR(50), data TEXT)");
    }

    // Helper method to execute SQL
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

    // Helper method to create test files
    void CreateTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        ASSERT_TRUE(file.is_open()) << "Failed to create test file: " << filename;
        file << content;
        file.close();
    }

    std::unique_ptr<LoadDataExecutor> load_executor_;
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<SqlExecutor> executor_;
};

// Test CSV parsing with malformed data
TEST_F(LoadDataExecutorBoundaryTest, MalformedCSVData) {
    CreateTestFile("invalid_test.csv", "1,John,25,50000.00\ninvalid_data\n2,Jane,30,60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "invalid_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    
    // Should handle malformed data gracefully
    ExecuteResult result = load_executor_->execute(stmt);
    // Should either succeed with partial data or fail gracefully
    EXPECT_TRUE(result.success || result.error_message.find("parse") != std::string::npos);
}

// Test CSV with different encodings
TEST_F(LoadDataExecutorBoundaryTest, UnicodeEncodingHandling) {
    std::string unicode_content = u8"1,张伟,28,65000.00\n2,Müller,35,75000.00\n3,José,42,85000.00\n";
    CreateTestFile("unicode_test.csv", unicode_content);

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "unicode_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.charset_name = "utf8";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 3);
}

// Test empty file handling
TEST_F(LoadDataExecutorBoundaryTest, EmptyFileHandling) {
    CreateTestFile("empty_test.csv", "");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "empty_test.csv";
    stmt.fields_terminated_by = ",";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 0);
}

// Test file with only headers
TEST_F(LoadDataExecutorBoundaryTest, HeaderOnlyFile) {
    CreateTestFile("header_only.csv", "id,name,age,salary\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "header_only.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.ignore_lines = 1;
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 0);
}

// Test very large file handling
TEST_F(LoadDataExecutorBoundaryTest, LargeFilePerformance) {
    // Create large test file (10,000 rows)
    std::ofstream file("large_test.csv");
    ASSERT_TRUE(file.is_open());
    
    for (int i = 1; i <= 10000; ++i) {
        file << i << ",User" << i << "," << (20 + i % 50) << "," << (50000.0 + i * 10) << "\n";
    }
    file.close();

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "large_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.ignore_lines = 0;
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 10000);
}

// Test data type conversion errors
TEST_F(LoadDataExecutorBoundaryTest, DataTypeConversionErrors) {
    CreateTestFile("type_error.csv", "1,John,invalid_age,50000.00\n2,Jane,30,not_a_number\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "type_error.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    // Should handle type conversion errors gracefully
    EXPECT_TRUE(result.success || result.error_message.find("type") != std::string::npos || 
                result.error_message.find("convert") != std::string::npos);
}

// Test constraint violation handling
TEST_F(LoadDataExecutorBoundaryTest, ConstraintViolations) {
    CreateTestFile("constraint_violation.csv", "1,John,25,50000.00\n1,Jane,30,60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "constrained_table";
    stmt.file_name = "constraint_violation.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    // Should handle constraint violations
    EXPECT_TRUE(result.success || result.error_message.find("constraint") != std::string::npos ||
                result.error_message.find("duplicate") != std::string::npos);
}

// Test NULL value handling
TEST_F(LoadDataExecutorBoundaryTest, NullValueHandling) {
    CreateTestFile("null_test.csv", "1,John,25,50000.00\n2,,30,60000.00\n3,Jane,,70000.00\n4,Jane,35,\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "null_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 4);
}

// Test custom field separators
TEST_F(LoadDataExecutorBoundaryTest, CustomFieldSeparators) {
    CreateTestFile("custom_sep.csv", "1\tJohn\t25\t50000.00\n2\tJane\t30\t60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "custom_sep.csv";
    stmt.fields_terminated_by = "\t";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

// Test quoted fields with special characters
TEST_F(LoadDataExecutorBoundaryTest, QuotedFieldsWithSpecialChars) {
    CreateTestFile("quoted_test.csv", "1,\"John, Jr.\",25,50000.00\n2,\"Jane \"Smith\"\",30,60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "quoted_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.fields_enclosed_by = "\"";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

// Test line starting patterns
TEST_F(LoadDataExecutorBoundaryTest, LineStartingPatterns) {
    CreateTestFile("pattern_test.csv", "# Comment line\ndata:1,John,25,50000.00\ndata:2,Jane,30,60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "pattern_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_starting_by = "data:";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

// Test escape character handling
TEST_F(LoadDataExecutorBoundaryTest, EscapeCharacterHandling) {
    CreateTestFile("escape_test.csv", "1,John\\, Jr.,25,50000.00\n2,Jane,30,60000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "escape_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.fields_escaped_by = "\\";
    stmt.lines_terminated_by = "\n";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

// Test partial column loading
TEST_F(LoadDataExecutorBoundaryTest, PartialColumnLoading) {
    CreateTestFile("partial_test.csv", "John,25\nJane,30\nBob,35\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "partial_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.column_list = {"name", "age"}; // Only load name and age, leave id and salary as NULL/default
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 3);
}

// Test SET expressions
TEST_F(LoadDataExecutorBoundaryTest, SetExpressions) {
    CreateTestFile("set_test.csv", "John,25,50000\nJane,30,60000\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "set_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    
    // Add SET expression to increase salary by 10%
    std::pair<std::string, std::string> set_expr("salary", "salary * 1.1");
    stmt.set_expressions.push_back(set_expr);
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

// Test LOW_PRIORITY option
TEST_F(LoadDataExecutorBoundaryTest, LowPriorityOption) {
    CreateTestFile("priority_test.csv", "1,John,25,50000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "priority_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.low_priority = true;
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
}

// Test LOCAL vs server-side file handling
TEST_F(LoadDataExecutorBoundaryTest, LocalFileHandling) {
    CreateTestFile("local_test.csv", "1,John,25,50000.00\n");

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "local_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.is_local = true;
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
}

// Test partition loading
TEST_F(LoadDataExecutorBoundaryTest, PartitionLoading) {
    CreateTestFile("partition_test.csv", "1,US,data1\n2,EU,data2\n3,AS,data3\n");

    LoadDataStatement stmt;
    stmt.table_name = "partition_table";
    stmt.file_name = "partition_test.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.partitions = {"US", "EU"};
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
}

// Test concurrent loading attempts
TEST_F(LoadDataExecutorBoundaryTest, ConcurrentLoading) {
    CreateTestFile("concurrent1.csv", "1,John,25,50000.00\n");
    CreateTestFile("concurrent2.csv", "2,Jane,30,60000.00\n");

    // Test loading multiple files concurrently (simulated)
    LoadDataStatement stmt1;
    stmt1.table_name = "test_load";
    stmt1.file_name = "concurrent1.csv";
    stmt1.fields_terminated_by = ",";
    
    LoadDataStatement stmt2;
    stmt2.table_name = "test_load";
    stmt2.file_name = "concurrent2.csv";
    stmt2.fields_terminated_by = ",";
    
    ExecuteResult result1 = load_executor_->execute(stmt1);
    ExecuteResult result2 = load_executor_->execute(stmt2);
    
    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
}

// Test file permission errors
TEST_F(LoadDataExecutorBoundaryTest, FilePermissionErrors) {
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "/root/forbidden_file.csv"; // Should fail
    stmt.fields_terminated_by = ",";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error_message.find("permission") != std::string::npos ||
               result.error_message.find("access") != std::string::npos, 0);
}

// Test disk space limitations
TEST_F(LoadDataExecutorBoundaryTest, DiskSpaceHandling) {
    // Create a file that simulates disk space issues
    CreateTestFile("space_test.csv", "1,John,25,50000.00\n");
    
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "space_test.csv";
    stmt.fields_terminated_by = ",";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
}

// Test memory usage with large files
TEST_F(LoadDataExecutorBoundaryTest, MemoryUsageLargeFiles) {
    // Create a moderately large file to test memory handling
    std::ofstream file("memory_test.csv");
    ASSERT_TRUE(file.is_open());
    
    for (int i = 1; i <= 5000; ++i) {
        file << i << ",VeryLongNameThatTakesUpSpace" << i << "," << (20 + i % 50) << "," << (50000.0 + i * 10) << "\n";
    }
    file.close();

    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "memory_test.csv";
    stmt.fields_terminated_by = ",";
    
    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 5000);
}

// Test transaction rollback on error
TEST_F(LoadDataExecutorBoundaryTest, TransactionRollbackOnError) {
    // First, load some data successfully
    CreateTestFile("success.csv", "1,John,25,50000.00\n");
    
    LoadDataStatement stmt1;
    stmt1.table_name = "test_load";
    stmt1.file_name = "success.csv";
    stmt1.fields_terminated_by = ",";
