#include "gtest/gtest.h"
#include "sql_executor.h"
#include "execution_engine.h"
#include "database_manager.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/parser.h"
#include <iostream>
#include <memory>

// DDL测试用例
class DDLTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建数据库管理器
        db_manager = std::make_shared<sqlcc::DatabaseManager>("./test.db", 1024, 4, 2);
        // 创建DDL执行器
        ddl_executor = std::make_unique<sqlcc::DDLExecutor>(db_manager);
    }

    void TearDown() override {
        ddl_executor.reset();
        db_manager.reset();
    }

    std::shared_ptr<sqlcc::DatabaseManager> db_manager;
    std::unique_ptr<sqlcc::DDLExecutor> ddl_executor;
};

// 测试CREATE DATABASE语句
TEST_F(DDLTest, CreateDatabase) {
    // 创建CREATE DATABASE语句
    auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
        sqlcc::sql_parser::CreateStatement::DATABASE);
    stmt->setDatabaseName("test_db");
    
    // 执行语句
    sqlcc::ExecutionResult result = ddl_executor->execute(std::move(stmt));
    
    // 检查结果
    EXPECT_EQ(result.getStatus(), sqlcc::ExecutionResult::SUCCESS);
    EXPECT_NE(result.getMessage().find("created successfully"), std::string::npos);
}

// 测试CREATE TABLE语句
TEST_F(DDLTest, CreateTable) {
    // 首先创建数据库
    {
        auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
            sqlcc::sql_parser::CreateStatement::DATABASE);
        stmt->setDatabaseName("test_db");
        ddl_executor->execute(std::move(stmt));
    }
    
    // 切换到该数据库
    db_manager->UseDatabase("test_db");
    
    // 创建表
    auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
        sqlcc::sql_parser::CreateStatement::TABLE);
    stmt->setTableName("test_table");
    
    // 添加列定义
    sqlcc::sql_parser::ColumnDefinition col1("id", "INT");
    col1.setPrimaryKey(true);
    stmt->addColumn(std::move(col1));
    
    sqlcc::sql_parser::ColumnDefinition col2("name", "VARCHAR(50)");
    col2.setNullable(false);
    stmt->addColumn(std::move(col2));
    
    // 执行语句
    sqlcc::ExecutionResult result = ddl_executor->execute(std::move(stmt));
    
    // 检查结果
    EXPECT_EQ(result.getStatus(), sqlcc::ExecutionResult::SUCCESS);
    EXPECT_NE(result.getMessage().find("created successfully"), std::string::npos);
}

// 测试DROP TABLE语句
TEST_F(DDLTest, DropTable) {
    // 首先创建数据库和表
    {
        auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
            sqlcc::sql_parser::CreateStatement::DATABASE);
        stmt->setDatabaseName("test_db");
        ddl_executor->execute(std::move(stmt));
    }
    
    db_manager->UseDatabase("test_db");
    
    {
        auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
            sqlcc::sql_parser::CreateStatement::TABLE);
        stmt->setTableName("test_table");
        
        sqlcc::sql_parser::ColumnDefinition col1("id", "INT");
        col1.setPrimaryKey(true);
        stmt->addColumn(std::move(col1));
        
        sqlcc::sql_parser::ColumnDefinition col2("name", "VARCHAR(50)");
        stmt->addColumn(std::move(col2));
        
        ddl_executor->execute(std::move(stmt));
    }
    
    // 删除表
    auto stmt = std::make_unique<sqlcc::sql_parser::DropStatement>(
        sqlcc::sql_parser::DropStatement::TABLE);
    stmt->setTableName("test_table");
    
    // 执行语句
    sqlcc::ExecutionResult result = ddl_executor->execute(std::move(stmt));
    
    // 检查结果
    EXPECT_EQ(result.getStatus(), sqlcc::ExecutionResult::SUCCESS);
    EXPECT_NE(result.getMessage().find("dropped successfully"), std::string::npos);
}

// 测试DROP DATABASE语句
TEST_F(DDLTest, DropDatabase) {
    // 首先创建数据库
    {
        auto stmt = std::make_unique<sqlcc::sql_parser::CreateStatement>(
            sqlcc::sql_parser::CreateStatement::DATABASE);
        stmt->setDatabaseName("test_db");
        ddl_executor->execute(std::move(stmt));
    }
    
    // 删除数据库
    auto stmt = std::make_unique<sqlcc::sql_parser::DropStatement>(
        sqlcc::sql_parser::DropStatement::DATABASE);
    stmt->setDatabaseName("test_db");
    
    // 执行语句
    sqlcc::ExecutionResult result = ddl_executor->execute(std::move(stmt));
    
    // 检查结果
    EXPECT_EQ(result.getStatus(), sqlcc::ExecutionResult::SUCCESS);
    EXPECT_NE(result.getMessage().find("dropped successfully"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

