#include <gtest/gtest.h>
#include "execution_engine.h"
#include "database_manager.h"
#include "sql_parser/parser.h"
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class IndexMaintenanceTest : public ::testing::Test {
protected:
    std::string test_dir = "./index_maintenance_test";
    std::shared_ptr<sqlcc::DatabaseManager> db_manager;
    
    void SetUp() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
        db_manager = std::make_shared<sqlcc::DatabaseManager>(test_dir);
        ASSERT_TRUE(db_manager->CreateDatabase("testdb"));
        ASSERT_TRUE(db_manager->UseDatabase("testdb"));
    }
    
    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
};

// 测试INSERT时自动维护索引
TEST_F(IndexMaintenanceTest, InsertWithIndexMaintenance) {
    // 创建带索引的表
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("users", columns));
    
    // TODO: 创建索引
    // db_manager->CreateIndex("idx_user_id", "users", "id");
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入记录
    std::string insert_sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    sqlcc::sql_parser::Parser parser(insert_sql);
    auto stmts = parser.parseStatements();
    auto result = executor.execute(std::move(stmts[0]));
    
    EXPECT_TRUE(result.success);
    
    // TODO: 验证索引中确实有该记录
    // auto index = db_manager->GetIndex("idx_user_id");
    // std::vector<IndexEntry> entries = index->Search("1");
    // EXPECT_EQ(entries.size(), 1);
}

// 测试UPDATE时更新索引
TEST_F(IndexMaintenanceTest, UpdateWithIndexMaintenance) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("products", columns));
    
    // TODO: 创建索引
    // db_manager->CreateIndex("idx_product_id", "products", "id");
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入初始记录
    std::string insert_sql = "INSERT INTO products (id, name) VALUES (1, 'ProductA');";
    sqlcc::sql_parser::Parser parser1(insert_sql);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    // 更新记录
    std::string update_sql = "UPDATE products SET id = 2 WHERE id = 1;";
    sqlcc::sql_parser::Parser parser2(update_sql);
    auto stmts2 = parser2.parseStatements();
    auto result = executor.execute(std::move(stmts2[0]));
    
    EXPECT_TRUE(result.success);
    
    // TODO: 验证旧索引键被删除，新索引键被添加
    // auto index = db_manager->GetIndex("idx_product_id");
    // std::vector<IndexEntry> old_entries = index->Search("1");
    // std::vector<IndexEntry> new_entries = index->Search("2");
    // EXPECT_EQ(old_entries.size(), 0);
    // EXPECT_EQ(new_entries.size(), 1);
}

// 测试DELETE时删除索引条目
TEST_F(IndexMaintenanceTest, DeleteWithIndexMaintenance) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("orders", columns));
    
    // TODO: 创建索引
    // db_manager->CreateIndex("idx_order_id", "orders", "id");
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入记录
    std::string insert_sql = "INSERT INTO orders (id, name) VALUES (1, 'Order1');";
    sqlcc::sql_parser::Parser parser1(insert_sql);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    // 删除记录
    std::string delete_sql = "DELETE FROM orders WHERE id = 1;";
    sqlcc::sql_parser::Parser parser2(delete_sql);
    auto stmts2 = parser2.parseStatements();
    auto result = executor.execute(std::move(stmts2[0]));
    
    EXPECT_TRUE(result.success);
    
    // TODO: 验证索引中的记录被删除
    // auto index = db_manager->GetIndex("idx_order_id");
    // std::vector<IndexEntry> entries = index->Search("1");
    // EXPECT_EQ(entries.size(), 0);
}

// 测试多个索引的维护
TEST_F(IndexMaintenanceTest, MultipleIndexesMaintenance) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"email", "VARCHAR"},
        {"name", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("accounts", columns));
    
    // TODO: 创建多个索引
    // db_manager->CreateIndex("idx_account_id", "accounts", "id");
    // db_manager->CreateIndex("idx_account_email", "accounts", "email");
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入记录
    std::string insert_sql = "INSERT INTO accounts (id, email, name) VALUES (1, 'alice@example.com', 'Alice');";
    sqlcc::sql_parser::Parser parser(insert_sql);
    auto stmts = parser.parseStatements();
    auto result = executor.execute(std::move(stmts[0]));
    
    EXPECT_TRUE(result.success);
    
    // TODO: 验证两个索引都被更新
    // auto idx_id = db_manager->GetIndex("idx_account_id");
    // auto idx_email = db_manager->GetIndex("idx_account_email");
    // std::vector<IndexEntry> id_entries = idx_id->Search("1");
    // std::vector<IndexEntry> email_entries = idx_email->Search("alice@example.com");
    // EXPECT_EQ(id_entries.size(), 1);
    // EXPECT_EQ(email_entries.size(), 1);
}

// 测试索引在WHERE条件中的加速
TEST_F(IndexMaintenanceTest, IndexBasedWhereClauseOptimization) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("items", columns));
    
    // TODO: 创建索引
    // db_manager->CreateIndex("idx_item_id", "items", "id");
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入多条记录
    for (int i = 1; i <= 100; i++) {
        std::string sql = "INSERT INTO items (id, name) VALUES (" + std::to_string(i) + ", 'Item" + std::to_string(i) + "');";
        sqlcc::sql_parser::Parser parser(sql);
        auto stmts = parser.parseStatements();
        executor.execute(std::move(stmts[0]));
    }
    
    // TODO: 使用索引查询WHERE id = 50应该快速返回
    // std::string select_sql = "SELECT * FROM items WHERE id = 50;";
    // 验证使用了索引而不是全表扫描
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
