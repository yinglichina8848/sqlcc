#include "execution_engine.h"
#include "database_manager.h"
#include "config_manager.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {

// 测试辅助类
class ExecutionEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化配置管理器
        config_manager_ = std::make_shared<ConfigManager>();

        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>("./test.db", 1024, 4, 2);

        // 初始化执行器
        ddl_executor_ = std::make_unique<DDLExecutor>(db_manager_);
        dml_executor_ = std::make_unique<DMLExecutor>(db_manager_);
        query_executor_ = std::make_unique<QueryExecutor>(db_manager_);
    }

    void TearDown() override {
        // 清理资源
        ddl_executor_.reset();
        dml_executor_.reset();
        query_executor_.reset();
        db_manager_.reset();
        config_manager_.reset();
    }

    std::shared_ptr<ConfigManager> config_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::unique_ptr<DDLExecutor> ddl_executor_;
    std::unique_ptr<DMLExecutor> dml_executor_;
    std::unique_ptr<QueryExecutor> query_executor_;
};

// 测试ExecutionResult
TEST_F(ExecutionEngineTest, ExecutionResultTest) {
    ExecutionResult success(ExecutionResult::SUCCESS, "Operation completed", 5);
    EXPECT_EQ(success.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_EQ(success.getMessage(), "Operation completed");
    EXPECT_EQ(success.getAffectedRows(), 5);

    ExecutionResult error(ExecutionResult::ERROR, "Something went wrong");
    EXPECT_EQ(error.getStatus(), ExecutionResult::ERROR);
    EXPECT_EQ(error.getMessage(), "Something went wrong");
    EXPECT_EQ(error.getAffectedRows(), 0);
}

// 测试QueryResult
TEST_F(ExecutionEngineTest, QueryResultTest) {
    QueryResult result;

    result.addColumn("id", "INTEGER");
    result.addColumn("name", "VARCHAR");
    result.addColumn("age", "INTEGER");

    result.addRow({"1", "Alice", "25"});
    result.addRow({"2", "Bob", "30"});

    EXPECT_EQ(result.getColumnNames().size(), 3);
    EXPECT_EQ(result.getColumnNames()[0], "id");
    EXPECT_EQ(result.getColumnNames()[1], "name");
    EXPECT_EQ(result.getColumnNames()[2], "age");

    EXPECT_EQ(result.getColumnTypes().size(), 3);
    EXPECT_EQ(result.getColumnTypes()[0], "INTEGER");
    EXPECT_EQ(result.getColumnTypes()[1], "VARCHAR");
    EXPECT_EQ(result.getColumnTypes()[2], "INTEGER");

    EXPECT_EQ(result.getRows().size(), 2);
    EXPECT_EQ(result.getRows()[0][0], "1");
    EXPECT_EQ(result.getRows()[0][1], "Alice");
    EXPECT_EQ(result.getRows()[1][0], "2");
    EXPECT_EQ(result.getRows()[1][1], "Bob");
}

// 测试DDLExecutor - CREATE TABLE
TEST_F(ExecutionEngineTest, DDLExecutorCreateTableTest) {
    // 创建一个简单的CREATE TABLE语句的模拟对象
    class MockCreateStatement : public sqlcc::sql_parser::Statement {
    public:
        MockCreateStatement() : target_(sqlcc::sql_parser::CreateStatement::TABLE) {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::CREATE;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        // 模拟CreateStatement的方法
        sqlcc::sql_parser::CreateStatement::Target getTarget() const { return target_; }
        const std::string& getTableName() const { return table_name_; }
        const std::vector<sqlcc::sql_parser::ColumnDefinition>& getColumns() const { return columns_; }

        void setTableName(const std::string& name) { table_name_ = name; }
        void addColumn(sqlcc::sql_parser::ColumnDefinition&& col) { columns_.push_back(std::move(col)); }

    private:
        sqlcc::sql_parser::CreateStatement::Target target_;
        std::string table_name_;
        std::vector<sqlcc::sql_parser::ColumnDefinition> columns_;
    };

    // 创建测试用的表结构
    MockCreateStatement create_stmt;
    create_stmt.setTableName("test_users");

    sqlcc::sql_parser::ColumnDefinition col1("id", "INTEGER");
    col1.setPrimaryKey(true);
    sqlcc::sql_parser::ColumnDefinition col2("name", "VARCHAR");

    create_stmt.addColumn(std::move(col1));
    create_stmt.addColumn(std::move(col2));

    // 执行CREATE TABLE
    auto result = ddl_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&create_stmt)));

    // 验证结果
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().find("test_users") != std::string::npos);
    EXPECT_TRUE(result.getMessage().find("created successfully") != std::string::npos);

    // 验证表是否真的被创建
    EXPECT_TRUE(db_manager_->TableExists("test_users"));
}

// 测试DDLExecutor - DROP TABLE
TEST_F(ExecutionEngineTest, DDLExecutorDropTableTest) {
    // 先创建一张表用于测试DROP
    db_manager_->CreateTable("temp_table", {{"id", "INTEGER"}});
    ASSERT_TRUE(db_manager_->TableExists("temp_table"));

    // 创建DROP TABLE语句的模拟对象
    class MockDropStatement : public sqlcc::sql_parser::Statement {
    public:
        MockDropStatement() : target_(sqlcc::sql_parser::DropStatement::TABLE) {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::DROP;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        // 模拟DropStatement的方法
        sqlcc::sql_parser::DropStatement::Target getTarget() const { return target_; }
        const std::string& getTableName() const { return table_name_; }
        bool isIfExists() const { return if_exists_; }

        void setTableName(const std::string& name) { table_name_ = name; }
        void setIfExists(bool if_exists) { if_exists_ = if_exists; }

    private:
        sqlcc::sql_parser::DropStatement::Target target_;
        std::string table_name_;
        bool if_exists_;
    };

    MockDropStatement drop_stmt;
    drop_stmt.setTableName("temp_table");
    drop_stmt.setIfExists(true);

    // 执行DROP TABLE
    auto result = ddl_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&drop_stmt)));

    // 验证结果
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().find("temp_table") != std::string::npos);
    EXPECT_TRUE(result.getMessage().find("dropped successfully") != std::string::npos);
}

// 测试DMLExecutor - INSERT
TEST_F(ExecutionEngineTest, DMLExecutorInsertTest) {
    // 确保表存在
    if (!db_manager_->TableExists("users")) {
        db_manager_->CreateTable("users", {{"id", "INTEGER"}, {"name", "VARCHAR"}});
    }

    // 创建INSERT语句的模拟对象
    class MockInsertStatement : public sqlcc::sql_parser::Statement {
    public:
        MockInsertStatement() {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::INSERT;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        // 模拟InsertStatement的方法
        const std::string& getTableName() const { return table_name_; }
        const std::vector<std::string>& getColumns() const { return columns_; }

        void setTableName(const std::string& name) { table_name_ = name; }
        void addColumn(const std::string& col) { columns_.push_back(col); }

    private:
        std::string table_name_;
        std::vector<std::string> columns_;
    };

    MockInsertStatement insert_stmt;
    insert_stmt.setTableName("users");
    insert_stmt.addColumn("id");
    insert_stmt.addColumn("name");

    // 执行INSERT
    auto result = dml_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&insert_stmt)));

    // 验证结果
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().find("users") != std::string::npos);
    EXPECT_TRUE(result.getMessage().find("executed successfully") != std::string::npos);
}

// 测试DMLExecutor - 不存在的表
TEST_F(ExecutionEngineTest, DMLExecutorInsertNonExistentTableTest) {
    // 创建INSERT语句的模拟对象
    class MockInsertStatement : public sqlcc::sql_parser::Statement {
    public:
        MockInsertStatement() {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::INSERT;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        const std::string& getTableName() const { return table_name_; }
        void setTableName(const std::string& name) { table_name_ = name; }

    private:
        std::string table_name_;
    };

    MockInsertStatement insert_stmt;
    insert_stmt.setTableName("non_existent_table");

    // 执行INSERT
    auto result = dml_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&insert_stmt)));

    // 验证结果 - 应该失败
    EXPECT_EQ(result.getStatus(), ExecutionResult::ERROR);
    EXPECT_TRUE(result.getMessage().find("does not exist") != std::string::npos);
}

// 测试QueryExecutor - SELECT
TEST_F(ExecutionEngineTest, QueryExecutorSelectTest) {
    // 确保表存在并有数据（模拟）
    if (!db_manager_->TableExists("users")) {
        db_manager_->CreateTable("users", {{"id", "INTEGER"}, {"name", "VARCHAR"}});
    }

    // 创建SELECT语句的模拟对象
    class MockSelectStatement : public sqlcc::sql_parser::Statement {
    public:
        MockSelectStatement() {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::SELECT;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        // 模拟SelectStatement的方法
        const std::vector<sqlcc::sql_parser::TableReference>& getFromTables() const { return from_tables_; }
        const std::unique_ptr<sqlcc::sql_parser::WhereClause>& getWhereClause() const { return where_clause_; }

        void addFromTable(const sqlcc::sql_parser::TableReference& table) {
            from_tables_.push_back(table);
        }

    private:
        std::vector<sqlcc::sql_parser::TableReference> from_tables_;
        std::unique_ptr<sqlcc::sql_parser::WhereClause> where_clause_;
    };

    MockSelectStatement select_stmt;
    sqlcc::sql_parser::TableReference table("users");
    select_stmt.addFromTable(table);

    // 执行SELECT
    auto result = query_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&select_stmt)));

    // 验证结果
    EXPECT_EQ(result.getStatus(), ExecutionResult::SUCCESS);
    EXPECT_TRUE(result.getMessage().find("executed successfully") != std::string::npos);

    // 转换为QueryResult进行更详细的验证
    QueryResult* query_result = dynamic_cast<QueryResult*>(&result);
    ASSERT_TRUE(query_result != nullptr);
}

// 测试QueryExecutor - SELECT不存在的表
TEST_F(ExecutionEngineTest, QueryExecutorSelectNonExistentTableTest) {
    // 创建SELECT语句的模拟对象
    class MockSelectStatement : public sqlcc::sql_parser::Statement {
    public:
        MockSelectStatement() {}
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::SELECT;
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}

        const std::vector<sqlcc::sql_parser::TableReference>& getFromTables() const { return from_tables_; }

        void addFromTable(const sqlcc::sql_parser::TableReference& table) {
            from_tables_.push_back(table);
        }

    private:
        std::vector<sqlcc::sql_parser::TableReference> from_tables_;
    };

    MockSelectStatement select_stmt;
    sqlcc::sql_parser::TableReference table("non_existent_table");
    select_stmt.addFromTable(table);

    // 执行SELECT
    auto result = query_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&select_stmt)));

    // 验证结果 - 应该失败
    EXPECT_EQ(result.getStatus(), ExecutionResult::ERROR);
    EXPECT_TRUE(result.getMessage().find("does not exist") != std::string::npos);
}

// 测试不支持的语句类型
TEST_F(ExecutionEngineTest, UnsupportedStatementTypeTest) {
    // 创建一个未知类型的语句
    class MockUnknownStatement : public sqlcc::sql_parser::Statement {
    public:
        sqlcc::sql_parser::Statement::Type getType() const override {
            return sqlcc::sql_parser::Statement::OTHER;  // 不支持的类型
        }
        void accept(sqlcc::sql_parser::NodeVisitor &visitor) override {}
    };

    MockUnknownStatement unknown_stmt;

    // 测试DDL执行器
    auto ddl_result = ddl_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&unknown_stmt)));
    EXPECT_EQ(ddl_result.getStatus(), ExecutionResult::ERROR);

    // 测试DML执行器
    auto dml_result = dml_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&unknown_stmt)));
    EXPECT_EQ(dml_result.getStatus(), ExecutionResult::ERROR);

    // 测试Query执行器
    auto query_result = query_executor_->execute(std::unique_ptr<sqlcc::sql_parser::Statement>(
        dynamic_cast<sqlcc::sql_parser::Statement*>(&unknown_stmt)));
    EXPECT_EQ(query_result.getStatus(), ExecutionResult::ERROR);
}

} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
