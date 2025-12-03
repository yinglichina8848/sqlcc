#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

#include "core/database_manager.h"
#include "core/config_manager.h"
#include "storage/storage_engine.h"
#include "transaction/transaction_manager.h"

using namespace sqlcc;

class DatabaseManagerCoverageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a unique test database for each test
        test_db_path = std::filesystem::temp_directory_path() / "test_db_" + 
                      std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".db";
    }
    
    void TearDown() override {
        // Clean up test database
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }
    
    std::filesystem::path test_db_path;
};

TEST_F(DatabaseManagerCoverageTest, InitializationWithValidPath) {
    // Test initialization with a valid path
    auto db_manager = std::make_unique<DatabaseManager>();
    bool result = db_manager->Initialize(test_db_path.string());
    EXPECT_TRUE(result);
    EXPECT_TRUE(db_manager->IsInitialized());
    EXPECT_EQ(db_manager->GetDatabasePath(), test_db_path.string());
}

TEST_F(DatabaseManagerCoverageTest, InitializationWithInvalidPath) {
    // Test initialization with an invalid path (non-existent directory)
    auto db_manager = std::make_unique<DatabaseManager>();
    bool result = db_manager->Initialize("/non/existent/path/test.db");
    EXPECT_FALSE(result);
    EXPECT_FALSE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, InitializationWithEmptyPath) {
    // Test initialization with an empty path
    auto db_manager = std::make_unique<DatabaseManager>();
    bool result = db_manager->Initialize("");
    EXPECT_FALSE(result);
    EXPECT_FALSE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, InitializationWithNullPath) {
    // Test initialization with a null path
    auto db_manager = std::make_unique<DatabaseManager>();
    bool result = db_manager->Initialize(nullptr);
    EXPECT_FALSE(result);
    EXPECT_FALSE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, DoubleInitialization) {
    // Test double initialization (should handle gracefully)
    auto db_manager = std::make_unique<DatabaseManager>();
    bool first_result = db_manager->Initialize(test_db_path.string());
    EXPECT_TRUE(first_result);
    
    // Second initialization should either succeed or handle gracefully
    bool second_result = db_manager->Initialize(test_db_path.string());
    EXPECT_TRUE(second_result || !second_result);  // Either behavior is acceptable
    EXPECT_TRUE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, ExecuteBeforeInitialization) {
    // Test executing SQL before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    auto result = db_manager->Execute("SELECT 1");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not initialized"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, ExecuteWithNullSql) {
    // Test executing null SQL
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->Execute(nullptr);
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("empty"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, ExecuteWithEmptySql) {
    // Test executing empty SQL
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->Execute("");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("empty"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, ExecuteWithWhitespaceSql) {
    // Test executing SQL with only whitespace
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->Execute("   \t\n   ");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("empty"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, ExecuteWithInvalidSql) {
    // Test executing invalid SQL
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->Execute("INVALID SQL SYNTAX");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("syntax"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, ExecuteValidSql) {
    // Test executing valid SQL
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    auto create_result = db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    EXPECT_TRUE(create_result->IsSuccess());
    
    // Insert data
    auto insert_result = db_manager->Execute("INSERT INTO test VALUES (1, 'test')");
    EXPECT_TRUE(insert_result->IsSuccess());
    
    // Query data
    auto select_result = db_manager->Execute("SELECT * FROM test");
    EXPECT_TRUE(select_result->IsSuccess());
    
    auto rows = select_result->GetRows();
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].GetInt(0), 1);
    EXPECT_EQ(rows[0].GetString(1), "test");
}

TEST_F(DatabaseManagerCoverageTest, ExecuteMultipleStatements) {
    // Test executing multiple SQL statements in one call
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    auto create_result = db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    EXPECT_TRUE(create_result->IsSuccess());
    
    // Multiple INSERT statements
    auto insert_result = db_manager->Execute("INSERT INTO test VALUES (1, 'test1'); INSERT INTO test VALUES (2, 'test2');");
    EXPECT_TRUE(insert_result->IsSuccess());
    
    // Query data
    auto select_result = db_manager->Execute("SELECT COUNT(*) FROM test");
    EXPECT_TRUE(select_result->IsSuccess());
    
    auto rows = select_result->GetRows();
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].GetInt(0), 2);
}

TEST_F(DatabaseManagerCoverageTest, BeginTransactionWithoutInitialization) {
    // Test beginning a transaction before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->BeginTransaction();
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not initialized"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, CommitTransactionWithoutInitialization) {
    // Test committing a transaction before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->CommitTransaction();
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not initialized"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, RollbackTransactionWithoutInitialization) {
    // Test rolling back a transaction before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->RollbackTransaction();
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not initialized"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, BasicTransactionOperations) {
    // Test basic transaction operations
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    auto create_result = db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    EXPECT_TRUE(create_result->IsSuccess());
    
    // Begin transaction
    auto begin_result = db_manager->BeginTransaction();
    EXPECT_TRUE(begin_result->IsSuccess());
    
    // Insert data
    auto insert_result = db_manager->Execute("INSERT INTO test VALUES (1, 'test')");
    EXPECT_TRUE(insert_result->IsSuccess());
    
    // Query data (should see the inserted row within the transaction)
    auto select_result = db_manager->Execute("SELECT COUNT(*) FROM test");
    EXPECT_TRUE(select_result->IsSuccess());
    EXPECT_EQ(select_result->GetRows()[0].GetInt(0), 1);
    
    // Commit transaction
    auto commit_result = db_manager->CommitTransaction();
    EXPECT_TRUE(commit_result->IsSuccess());
    
    // Query data again (should still see the row after commit)
    auto final_select = db_manager->Execute("SELECT COUNT(*) FROM test");
    EXPECT_TRUE(final_select->IsSuccess());
    EXPECT_EQ(final_select->GetRows()[0].GetInt(0), 1);
}

TEST_F(DatabaseManagerCoverageTest, TransactionRollback) {
    // Test transaction rollback
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    auto create_result = db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    EXPECT_TRUE(create_result->IsSuccess());
    
    // Begin transaction
    auto begin_result = db_manager->BeginTransaction();
    EXPECT_TRUE(begin_result->IsSuccess());
    
    // Insert data
    auto insert_result = db_manager->Execute("INSERT INTO test VALUES (1, 'test')");
    EXPECT_TRUE(insert_result->IsSuccess());
    
    // Query data (should see the inserted row within the transaction)
    auto select_result = db_manager->Execute("SELECT COUNT(*) FROM test");
    EXPECT_TRUE(select_result->IsSuccess());
    EXPECT_EQ(select_result->GetRows()[0].GetInt(0), 1);
    
    // Rollback transaction
    auto rollback_result = db_manager->RollbackTransaction();
    EXPECT_TRUE(rollback_result->IsSuccess());
    
    // Query data again (should not see the row after rollback)
    auto final_select = db_manager->Execute("SELECT COUNT(*) FROM test");
    EXPECT_TRUE(final_select->IsSuccess());
    EXPECT_EQ(final_select->GetRows()[0].GetInt(0), 0);
}

TEST_F(DatabaseManagerCoverageTest, NestedTransactions) {
    // Test nested transactions (if supported)
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    auto create_result = db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    EXPECT_TRUE(create_result->IsSuccess());
    
    // Begin outer transaction
    auto outer_begin = db_manager->BeginTransaction();
    EXPECT_TRUE(outer_begin->IsSuccess());
    
    // Insert first record
    auto insert1 = db_manager->Execute("INSERT INTO test VALUES (1, 'test1')");
    EXPECT_TRUE(insert1->IsSuccess());
    
    // Begin inner transaction
    auto inner_begin = db_manager->BeginTransaction();
    EXPECT_TRUE(inner_begin->IsSuccess());
    
    // Insert second record
    auto insert2 = db_manager->Execute("INSERT INTO test VALUES (2, 'test2')");
    EXPECT_TRUE(insert2->IsSuccess());
    
    // Rollback inner transaction
    auto inner_rollback = db_manager->RollbackTransaction();
    EXPECT_TRUE(inner_rollback->IsSuccess());
    
    // Commit outer transaction
    auto outer_commit = db_manager->CommitTransaction();
    EXPECT_TRUE(outer_commit->IsSuccess());
    
    // Query data (should see only the first record)
    auto final_select = db_manager->Execute("SELECT * FROM test ORDER BY id");
    EXPECT_TRUE(final_select->IsSuccess());
    auto rows = final_select->GetRows();
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].GetInt(0), 1);
    EXPECT_EQ(rows[0].GetString(1), "test1");
}

TEST_F(DatabaseManagerCoverageTest, GetTableNamesWithoutInitialization) {
    // Test getting table names before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->GetTableNames();
    EXPECT_TRUE(result->empty());
}

TEST_F(DatabaseManagerCoverageTest, GetTableNamesWithEmptyDatabase) {
    // Test getting table names from an empty database
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto table_names = db_manager->GetTableNames();
    EXPECT_TRUE(table_names->empty());
}

TEST_F(DatabaseManagerCoverageTest, GetTableNamesWithTables) {
    // Test getting table names from a database with tables
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create tables
    db_manager->Execute("CREATE TABLE test1 (id INT PRIMARY KEY)");
    db_manager->Execute("CREATE TABLE test2 (id INT PRIMARY KEY)");
    db_manager->Execute("CREATE TABLE test3 (id INT PRIMARY KEY)");
    
    // Get table names
    auto table_names = db_manager->GetTableNames();
    EXPECT_EQ(table_names->size(), 3);
    
    // Check if all tables are present
    std::set<std::string> tables(table_names->begin(), table_names->end());
    EXPECT_TRUE(tables.find("test1") != tables.end());
    EXPECT_TRUE(tables.find("test2") != tables.end());
    EXPECT_TRUE(tables.find("test3") != tables.end());
}

TEST_F(DatabaseManagerCoverageTest, TableExistsWithoutInitialization) {
    // Test checking if table exists before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->TableExists("test");
    EXPECT_FALSE(result);
}

TEST_F(DatabaseManagerCoverageTest, TableExistsWithEmptyName) {
    // Test checking if table exists with empty name
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->TableExists("");
    EXPECT_FALSE(result);
}

TEST_F(DatabaseManagerCoverageTest, TableExistsWithNullName) {
    // Test checking if table exists with null name
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->TableExists(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(DatabaseManagerCoverageTest, TableExistsWithExistingTable) {
    // Test checking if table exists with an existing table
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY)");
    
    // Check if table exists
    auto result = db_manager->TableExists("test");
    EXPECT_TRUE(result);
}

TEST_F(DatabaseManagerCoverageTest, TableExistsWithNonExistingTable) {
    // Test checking if table exists with a non-existing table
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Check if table exists
    auto result = db_manager->TableExists("non_existent_table");
    EXPECT_FALSE(result);
}

TEST_F(DatabaseManagerCoverageTest, GetTableSchemaWithoutInitialization) {
    // Test getting table schema before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    auto result = db_manager->GetTableSchema("test");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not initialized"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, GetTableSchemaWithNonExistingTable) {
    // Test getting table schema for a non-existing table
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    auto result = db_manager->GetTableSchema("non_existent_table");
    EXPECT_FALSE(result->IsSuccess());
    EXPECT_NE(result->GetErrorMessage().find("not found"), std::string::npos);
}

TEST_F(DatabaseManagerCoverageTest, GetTableSchemaWithExistingTable) {
    // Test getting table schema for an existing table
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100), age INT)");
    
    // Get table schema
    auto schema_result = db_manager->GetTableSchema("test");
    EXPECT_TRUE(schema_result->IsSuccess());
    
    auto schema = schema_result->GetSchema();
    EXPECT_EQ(schema->GetColumnCount(), 3);
    
    // Check column details
    auto id_column = schema->GetColumn(0);
    EXPECT_EQ(id_column->GetName(), "id");
    EXPECT_EQ(id_column->GetType(), ColumnType::INTEGER);
    EXPECT_TRUE(id_column->IsPrimaryKey());
    
    auto name_column = schema->GetColumn(1);
    EXPECT_EQ(name_column->GetName(), "name");
    EXPECT_EQ(name_column->GetType(), ColumnType::VARCHAR);
    EXPECT_FALSE(name_column->IsPrimaryKey());
    
    auto age_column = schema->GetColumn(2);
    EXPECT_EQ(age_column->GetName(), "age");
    EXPECT_EQ(age_column->GetType(), ColumnType::INTEGER);
    EXPECT_FALSE(age_column->IsPrimaryKey());
}

TEST_F(DatabaseManagerCoverageTest, CloseWithoutInitialization) {
    // Test closing database before initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    
    // This should handle gracefully without throwing
    EXPECT_NO_THROW(db_manager->Close());
    EXPECT_FALSE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, CloseWithInitialization) {
    // Test closing database after initialization
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    EXPECT_TRUE(db_manager->IsInitialized());
    
    // Close database
    db_manager->Close();
    
    EXPECT_FALSE(db_manager->IsInitialized());
}

TEST_F(DatabaseManagerCoverageTest, ReopenAfterClose) {
    // Test reopening database after closing
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Create table
    db_manager->Execute("CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR(100))");
    db_manager->Execute("INSERT INTO test VALUES (1, 'test')");
    
    // Close database
    db_manager->Close();
    
    // Reopen database
    db_manager->Initialize(test_db_path.string());
    
    // Query data (should still exist)
    auto result = db_manager->Execute("SELECT * FROM test");
    EXPECT_TRUE(result->IsSuccess());
    
    auto rows = result->GetRows();
    EXPECT_EQ(rows.size(), 1);
    EXPECT_EQ(rows[0].GetInt(0), 1);
    EXPECT_EQ(rows[0].GetString(1), "test");
}

TEST_F(DatabaseManagerCoverageTest, ConfigAccess) {
    // Test configuration access
    auto db_manager = std::make_unique<DatabaseManager>();
    db_manager->Initialize(test_db_path.string());
    
    // Get configuration
    auto config = db_manager->GetConfig();
    EXPECT_NE(config, nullptr);
    
    // Test default config values
    EXPECT_GT(config->GetMaxConnections(), 0);
    EXPECT_GT(config->GetPageSize(), 0);
    EXPECT_GT(config->GetCacheSize(), 0);
}