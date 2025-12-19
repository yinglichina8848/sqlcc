#include <gtest/gtest.h>
#include "include/sql_parser/parser.h"
#include "include/execution/load_data_executor.h"
#include "include/storage_engine.h"
#include "include/core/sql_executor.h"
#include <filesystem>
#include <fstream>
#include <memory>

// Test fixture for LOAD DATA tests
class LoadDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data directory
        std::filesystem::create_directories("test_data");

        // Initialize storage engine and SQL executor
        storage_engine_ = std::make_shared<StorageEngine>();
        sql_executor_ = std::make_shared<SqlExecutor>();

        // Create load data executor
        load_executor_ = std::make_unique<LoadDataExecutor>(storage_engine_, sql_executor_);

        // Create test table
        createTestTable();
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all("test_data");
        std::filesystem::remove("test_file.csv");
    }

    void createTestTable() {
        // Create a simple test table
        std::string create_sql = R"(
            CREATE TABLE test_load (
                id INTEGER PRIMARY KEY,
                name VARCHAR(50),
                age INTEGER,
                salary DECIMAL(10,2)
            )
        )";

        // Parse and execute CREATE TABLE
        sqlcc::sql_parser::Parser parser(create_sql);
        auto statements = parser.parse();

        for (auto& stmt : statements) {
            if (stmt) {
                ExecuteResult result = sql_executor_->execute(std::move(stmt));
                ASSERT_TRUE(result.success) << "Failed to create test table: " << result.error_message;
            }
        }
    }

    void createTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        ASSERT_TRUE(file.is_open()) << "Failed to create test file: " << filename;
        file << content;
        file.close();
    }

    std::unique_ptr<LoadDataExecutor> load_executor_;
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<SqlExecutor> sql_executor_;
};

TEST_F(LoadDataTest, BasicLoadDataTest) {
    // Create test CSV file
    std::string csv_content = R"(
1,John,25,50000.00
2,Jane,30,60000.00
3,Bob,35,70000.00
)";

    createTestFile("test_file.csv", csv_content);

    // Create LOAD DATA statement
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "test_file.csv";
    stmt.fields_terminated_by = ",";
    stmt.lines_terminated_by = "\n";
    stmt.ignore_lines = 1; // Skip header line

    // Execute LOAD DATA
    ExecuteResult result = load_executor_->execute(stmt);

    // Verify result
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 3);
    EXPECT_EQ(result.message.find("Loaded 3 rows"), std::string::npos);
}

TEST_F(LoadDataTest, LoadDataWithOptionsTest) {
    // Create test CSV file with quotes and custom separators
    std::string csv_content = R"(
1|"John Doe"|25|50000.50
2|"Jane Smith"|30|60000.75
3|"Bob Johnson"|35|70000.25
)";

    createTestFile("test_file.csv", csv_content);

    // Create LOAD DATA statement with options
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "test_file.csv";
    stmt.fields_terminated_by = "|";
    stmt.fields_enclosed_by = "\"";
    stmt.lines_terminated_by = "\n";
    stmt.ignore_lines = 1;

    // Execute LOAD DATA
    ExecuteResult result = load_executor_->execute(stmt);

    // Verify result
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 3);
}

TEST_F(LoadDataTest, LoadDataWithColumnMappingTest) {
    // Create test CSV file (different column order)
    std::string csv_content = R"(
John,25,50000.00,1
Jane,30,60000.00,2
Bob,35,70000.00,3
)";

    createTestFile("test_file.csv", csv_content);

    // Create LOAD DATA statement with column mapping
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "test_file.csv";
    stmt.fields_terminated_by = ",";
    stmt.column_list = {"name", "age", "salary", "id"}; // Map CSV columns to table columns
    stmt.ignore_lines = 1;

    // Execute LOAD DATA
    ExecuteResult result = load_executor_->execute(stmt);

    // Verify result
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 3);
}

TEST_F(LoadDataTest, LoadDataWithReplaceTest) {
    // First, load some initial data
    std::string initial_csv = "1,John,25,50000.00\n";
    createTestFile("test_file.csv", initial_csv);

    LoadDataStatement stmt1;
    stmt1.table_name = "test_load";
    stmt1.file_name = "test_file.csv";
    stmt1.fields_terminated_by = ",";
    stmt1.ignore_lines = 0;

    ExecuteResult result1 = load_executor_->execute(stmt1);
    EXPECT_TRUE(result1.success);
    EXPECT_EQ(result1.rows_affected, 1);

    // Now load with REPLACE option (same ID)
    std::string replace_csv = "1,Jane,26,55000.00\n";
    createTestFile("test_file.csv", replace_csv);

    LoadDataStatement stmt2;
    stmt2.table_name = "test_load";
    stmt2.file_name = "test_file.csv";
    stmt2.fields_terminated_by = ",";
    stmt2.replace_or_ignore = "REPLACE";
    stmt2.ignore_lines = 0;

    ExecuteResult result2 = load_executor_->execute(stmt2);
    EXPECT_TRUE(result2.success);
    EXPECT_EQ(result2.rows_affected, 1);
}

TEST_F(LoadDataTest, LoadDataWithIgnoreTest) {
    // First, load some initial data
    std::string initial_csv = "1,John,25,50000.00\n";
    createTestFile("test_file.csv", initial_csv);

    LoadDataStatement stmt1;
    stmt1.table_name = "test_load";
    stmt1.file_name = "test_file.csv";
    stmt1.fields_terminated_by = ",";
    stmt1.ignore_lines = 0;

    ExecuteResult result1 = load_executor_->execute(stmt1);
    EXPECT_TRUE(result1.success);
    EXPECT_EQ(result1.rows_affected, 1);

    // Now try to load with IGNORE option (same ID - should be ignored)
    std::string ignore_csv = "1,Jane,26,55000.00\n";
    createTestFile("test_file.csv", ignore_csv);

    LoadDataStatement stmt2;
    stmt2.table_name = "test_load";
    stmt2.file_name = "test_file.csv";
    stmt2.fields_terminated_by = ",";
    stmt2.replace_or_ignore = "IGNORE";
    stmt2.ignore_lines = 0;

    ExecuteResult result2 = load_executor_->execute(stmt2);
    EXPECT_TRUE(result2.success);
    EXPECT_EQ(result2.rows_affected, 0); // Should be ignored
}

TEST_F(LoadDataTest, LoadDataErrorHandlingTest) {
    // Test with non-existent file
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "non_existent_file.csv";

    ExecuteResult result = load_executor_->execute(stmt);
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error_message.find("Cannot access file"), std::string::npos);
}

TEST_F(LoadDataTest, LoadDataWithSetExpressionsTest) {
    // Create test CSV file
    std::string csv_content = R"(
1,John,25,50000
2,Jane,30,60000
)";

    createTestFile("test_file.csv", csv_content);

    // Create LOAD DATA statement with SET expressions
    LoadDataStatement stmt;
    stmt.table_name = "test_load";
    stmt.file_name = "test_file.csv";
    stmt.fields_terminated_by = ",";
    stmt.ignore_lines = 1;

    // Add SET expression (multiply salary by 1.1)
    std::pair<std::string, std::string> set_expr("salary", "salary * 1.1");
    stmt.set_expressions.push_back(set_expr);

    // Execute LOAD DATA (SET expressions are not fully implemented yet)
    ExecuteResult result = load_executor_->execute(stmt);

    // Should succeed but with warnings about SET expressions
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.rows_affected, 2);
}

TEST_F(LoadDataTest, ParserIntegrationTest) {
    // Test parsing LOAD DATA statement
    std::string sql = R"(
        LOAD DATA INFILE 'test_file.csv'
        INTO TABLE test_load
        FIELDS TERMINATED BY ','
        LINES TERMINATED BY '\n'
        IGNORE 1 LINES
    )";

    sqlcc::sql_parser::Parser parser(sql);
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    ASSERT_TRUE(statements[0] != nullptr);

    // Verify it's a LOAD DATA statement
    EXPECT_EQ(statements[0]->getType(), sql_parser::Statement::LOAD_DATA);

    // Cast to LoadDataStatement and verify properties
    const LoadDataStatement* load_stmt = dynamic_cast<const LoadDataStatement*>(statements[0].get());
    ASSERT_TRUE(load_stmt != nullptr);

    EXPECT_EQ(load_stmt->table_name, "test_load");
    EXPECT_EQ(load_stmt->file_name, "test_file.csv");
    EXPECT_EQ(load_stmt->fields_terminated_by, ",");
    EXPECT_EQ(load_stmt->lines_terminated_by, "\n");
    EXPECT_EQ(load_stmt->ignore_lines, 1);
}

TEST_F(LoadDataTest, ParserWithOptionsTest) {
    // Test parsing LOAD DATA with various options
    std::string sql = R"(
        LOAD DATA LOW_PRIORITY LOCAL INFILE 'data.csv'
        REPLACE INTO TABLE users
        PARTITION (p1, p2)
        CHARACTER SET utf8
        FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"' ESCAPED BY '\\'
        LINES STARTING BY 'data:' TERMINATED BY '\n'
        IGNORE 2 LINES
        (name, email, age)
        SET created_at = NOW()
    )";

    sqlcc::sql_parser::Parser parser(sql);
    auto statements = parser.parse();

    ASSERT_EQ(statements.size(), 1);
    const LoadDataStatement* load_stmt = dynamic_cast<const LoadDataStatement*>(statements[0].get());
    ASSERT_TRUE(load_stmt != nullptr);

    EXPECT_TRUE(load_stmt->low_priority);
    EXPECT_TRUE(load_stmt->is_local);
    EXPECT_EQ(load_stmt->file_name, "data.csv");
    EXPECT_EQ(load_stmt->replace_or_ignore, "REPLACE");
    EXPECT_EQ(load_stmt->table_name, "users");
    EXPECT_EQ(load_stmt->partitions.size(), 2);
    EXPECT_EQ(load_stmt->charset_name, "utf8");
    EXPECT_EQ(load_stmt->fields_terminated_by, ",");
    EXPECT_TRUE(load_stmt->fields_optionally_enclosed);
    EXPECT_EQ(load_stmt->fields_enclosed_by, "\"");
    EXPECT_EQ(load_stmt->fields_escaped_by, "\\");
    EXPECT_EQ(load_stmt->lines_starting_by, "data:");
    EXPECT_EQ(load_stmt->lines_terminated_by, "\n");
    EXPECT_EQ(load_stmt->ignore_lines, 2);
    EXPECT_EQ(load_stmt->column_list.size(), 3);
    EXPECT_EQ(load_stmt->set_expressions.size(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
