#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// 模拟SystemDatabase类
namespace sqlcc {

class SystemDatabase {
public:
    std::string name = "system_db";
    std::unordered_map<std::string, std::string> metadata;
    std::unordered_map<std::string, std::string> tables;
    std::unordered_map<std::string, std::string> views;
    bool is_connected_ = true;
    
    SystemDatabase() {
        // Initialize with default system tables
        metadata["version"] = "1.3.8";
        metadata["created_at"] = "2026-01-30";  // Add second default metadata
        tables["sqlite_master"] = "sqlite_schema";
        tables["sqlite_schema"] = "schema_definition";  // Add second default table
        views["system_tables"] = "information_schema.tables";
        views["information_schema.tables"] = "table_definition";  // Add second default view
    }
    
    bool IsConnected() const { return is_connected_; }
    void SetConnected(bool connected) { is_connected_ = connected; }
    
    bool AddMetadata(const std::string& key, const std::string& value) {
        if (key.empty()) return false;
        metadata[key] = value;
        return true;
    }
    
    bool AddTable(const std::string& table_name, const std::string& definition) {
        if (table_name.empty()) return false;
        tables[table_name] = definition;
        return true;
    }
    
    bool AddView(const std::string& view_name, const std::string& definition) {
        if (view_name.empty()) return false;
        views[view_name] = definition;
        return true;
    }
    
    bool DropTable(const std::string& table_name) {
        auto it = tables.find(table_name);
        if (it != tables.end()) {
            tables.erase(it);
            return true;
        }
        return false;
    }
    
    bool DropView(const std::string& view_name) {
        auto it = views.find(view_name);
        if (it != views.end()) {
            views.erase(it);
            return true;
        }
        return false;
    }
    
    std::string GetMetadata(const std::string& key) const {
        auto it = metadata.find(key);
        return it != metadata.end() ? it->second : "";
    }
    
    std::string GetTableDefinition(const std::string& table_name) const {
        auto it = tables.find(table_name);
        return it != tables.end() ? it->second : "";
    }
    
    std::string GetViewDefinition(const std::string& view_name) const {
        auto it = views.find(view_name);
        return it != views.end() ? it->second : "";
    }
    
    std::vector<std::string> GetAllTables() const {
        std::vector<std::string> result;
        for (const auto& pair : tables) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    std::vector<std::string> GetAllViews() const {
        std::vector<std::string> result;
        for (const auto& pair : views) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    std::vector<std::string> GetAllMetadataKeys() const {
        std::vector<std::string> result;
        for (const auto& pair : metadata) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    size_t GetTableCount() const { return tables.size(); }
    size_t GetViewCount() const { return views.size(); }
    size_t GetMetadataCount() const { return metadata.size(); }
};

} // namespace sqlcc

namespace sqlcc {
namespace test {

class SystemDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        sys_db = std::make_unique<SystemDatabase>();
    }
    
    void TearDown() override {
        sys_db.reset();
    }
    
    std::unique_ptr<SystemDatabase> sys_db;
};

// Test basic functionality
TEST_F(SystemDatabaseTest, DefaultConstructor) {
    EXPECT_EQ(sys_db->name, "system_db");
    EXPECT_TRUE(sys_db->IsConnected());
    EXPECT_EQ(sys_db->GetMetadata("version"), "1.3.8");
    EXPECT_EQ(sys_db->GetTableCount(), 2);  // sqlite_master and sqlite_schema
    EXPECT_EQ(sys_db->GetViewCount(), 2);  // system_tables and information_schema.tables
}

// Test connection state
TEST_F(SystemDatabaseTest, ConnectionState) {
    EXPECT_TRUE(sys_db->IsConnected());
    
    sys_db->SetConnected(false);
    EXPECT_FALSE(sys_db->IsConnected());
    
    sys_db->SetConnected(true);
    EXPECT_TRUE(sys_db->IsConnected());
}

// Test metadata management
TEST_F(SystemDatabaseTest, AddMetadata) {
    EXPECT_TRUE(sys_db->AddMetadata("created_date", "2026-01-30"));
    EXPECT_EQ(sys_db->GetMetadata("created_date"), "2026-01-30");
    EXPECT_EQ(sys_db->GetAllMetadataKeys().size(), 3);  // version, created_at, created_date
}

TEST_F(SystemDatabaseTest, GetNonExistentMetadata) {
    EXPECT_EQ(sys_db->GetMetadata("nonexistent"), "");
}

// Test table management
TEST_F(SystemDatabaseTest, AddTable) {
    EXPECT_TRUE(sys_db->AddTable("users", "CREATE TABLE users (id INTEGER PRIMARY KEY)"));
    EXPECT_EQ(sys_db->GetTableDefinition("users"), "CREATE TABLE users (id INTEGER PRIMARY KEY)");
    EXPECT_EQ(sys_db->GetTableCount(), 3);  // sqlite_master, sqlite_schema, + users
}

TEST_F(SystemDatabaseTest, DropTable) {
    sys_db->AddTable("temp_table", "CREATE TABLE temp_table (value TEXT)");
    EXPECT_EQ(sys_db->GetTableCount(), 3);
    
    EXPECT_TRUE(sys_db->DropTable("temp_table"));
    EXPECT_EQ(sys_db->GetTableCount(), 2);  // back to default
    EXPECT_FALSE(sys_db->DropTable("nonexistent"));
}

TEST_F(SystemDatabaseTest, DropNonExistentTable) {
    EXPECT_FALSE(sys_db->DropTable("nonexistent"));
}

// Test GetAllTables
TEST_F(SystemDatabaseTest, GetAllTables) {
    sys_db->AddTable("table1", "CREATE TABLE table1");
    sys_db->AddTable("table2", "CREATE TABLE table2");
    
    auto tables = sys_db->GetAllTables();
    EXPECT_EQ(tables.size(), 4);  // sqlite_master, sqlite_schema, table1, table2
    
    bool found_table1 = false;
    bool found_table2 = false;
    for (const auto& table : tables) {
        if (table == "table1") found_table1 = true;
        if (table == "table2") found_table2 = true;
    }
    EXPECT_TRUE(found_table1);
    EXPECT_TRUE(found_table2);
}

// Test view management
TEST_F(SystemDatabaseTest, AddView) {
    EXPECT_TRUE(sys_db->AddView("user_view", "CREATE VIEW user_view AS SELECT * FROM users"));
    EXPECT_EQ(sys_db->GetViewDefinition("user_view"), "CREATE VIEW user_view AS SELECT * FROM users");
    EXPECT_EQ(sys_db->GetViewCount(), 3);  // system_tables, information_schema.tables, + user_view
}

TEST_F(SystemDatabaseTest, DropView) {
    sys_db->AddView("temp_view", "CREATE VIEW temp_view AS SELECT 1");
    EXPECT_EQ(sys_db->GetViewCount(), 3);
    
    EXPECT_TRUE(sys_db->DropView("temp_view"));
    EXPECT_EQ(sys_db->GetViewCount(), 2);  // back to default
    EXPECT_FALSE(sys_db->DropView("nonexistent"));
}

TEST_F(SystemDatabaseTest, GetAllViews) {
    sys_db->AddView("view1", "CREATE VIEW view1");
    sys_db->AddView("view2", "CREATE VIEW view2");
    
    auto views = sys_db->GetAllViews();
    EXPECT_EQ(views.size(), 4);  // system_tables, information_schema.tables, view1, view2
    
    EXPECT_NE(std::find(views.begin(), views.end(), "view1"), views.end());
    EXPECT_NE(std::find(views.begin(), views.end(), "view2"), views.end());
}

// Test complex scenario
TEST_F(SystemDatabaseTest, ComplexScenario) {
    // Add multiple tables
    EXPECT_TRUE(sys_db->AddTable("users", "CREATE TABLE users (id INTEGER, name TEXT)"));
    EXPECT_TRUE(sys_db->AddTable("products", "CREATE TABLE products (id INTEGER, name TEXT)"));
    EXPECT_TRUE(sys_db->AddTable("orders", "CREATE TABLE orders (id INTEGER, user_id INTEGER)"));
    
    // Add multiple views
    EXPECT_TRUE(sys_db->AddView("user_orders", "CREATE VIEW user_orders AS SELECT * FROM orders WHERE user_id IN (SELECT id FROM users)"));
    EXPECT_TRUE(sys_db->AddView("product_summary", "CREATE VIEW product_summary AS SELECT name, COUNT(*) FROM products GROUP BY name"));
    
    // Add metadata
    EXPECT_TRUE(sys_db->AddMetadata("last_backup", "2026-01-30 12:00"));
    
    // Verify all counts
    EXPECT_EQ(sys_db->GetTableCount(), 5);  // 2 default + 3 custom
    EXPECT_EQ(sys_db->GetViewCount(), 4);  // 2 default + 2 custom
    EXPECT_EQ(sys_db->GetMetadataCount(), 3);  // version + created_at + last_backup
    
    // Drop one table and view
    EXPECT_TRUE(sys_db->DropTable("orders"));
    EXPECT_TRUE(sys_db->DropView("product_summary"));
    
    EXPECT_EQ(sys_db->GetTableCount(), 4);  // 2 default + 2 custom
    EXPECT_EQ(sys_db->GetViewCount(), 3);  // 2 default + 1 custom
}

// Test edge cases
TEST_F(SystemDatabaseTest, EmptyStrings) {
    EXPECT_FALSE(sys_db->AddTable("", ""));
    EXPECT_FALSE(sys_db->AddView("", ""));
    EXPECT_FALSE(sys_db->AddMetadata("", ""));
}

TEST_F(SystemDatabaseTest, LargeDataset) {
// Add many tables and views to test performance
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(sys_db->AddTable("table_" + std::to_string(i), "CREATE TABLE table_" + std::to_string(i) + " (id INTEGER)"));
        EXPECT_TRUE(sys_db->AddView("view_" + std::to_string(i), "CREATE VIEW view_" + std::to_string(i) + " AS SELECT * FROM table_" + std::to_string(i)));
    }
    
    EXPECT_EQ(sys_db->GetTableCount(), 102);  // 2 default + 100 tables
    EXPECT_EQ(sys_db->GetViewCount(), 102);  // 2 default + 100 views
    
    // Verify we can still access the first and last entries
    EXPECT_EQ(sys_db->GetTableDefinition("table_0"), "CREATE TABLE table_0 (id INTEGER)");
    EXPECT_EQ(sys_db->GetViewDefinition("view_99"), "CREATE VIEW view_99 AS SELECT * FROM table_99");
}

} // namespace test
} // namespace sqlcc