#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>

// 模拟SchemaManager类
namespace sqlcc {

enum class DataType {
    INTEGER,
    TEXT,
    REAL,
    BLOB,
    BOOLEAN
};

struct Column {
    std::string name;
    DataType type;
    bool nullable;
    bool primary_key;
    std::string default_value;
    
    Column(const std::string& n, DataType t, bool null = true, bool pk = false, const std::string& def = "")
        : name(n), type(t), nullable(null), primary_key(pk), default_value(def) {}
};

struct Table {
    std::string name;
    std::vector<Column> columns;
    std::unordered_set<std::string> unique_columns;
    std::string primary_key_column;
    
    Table() = default;
    Table(const std::string& n) : name(n) {}
};

enum class ConstraintType {
    PRIMARY_KEY,
    FOREIGN_KEY,
    UNIQUE,
    NOT_NULL,
    CHECK
};

struct Constraint {
    ConstraintType type;
    std::string name;
    std::vector<std::string> columns;
    std::string reference_table;
    std::string reference_column;
    
    Constraint(ConstraintType t, const std::string& n) : type(t), name(n) {}
};

class SchemaManager {
public:
    std::unordered_map<std::string, Table> tables;
    std::vector<Constraint> constraints;
    bool is_initialized_ = true;
    
    SchemaManager() {
        // Initialize with system schema
        Table system_table("system_schema");
        system_table.columns.emplace_back("version", DataType::TEXT, false, true);
        system_table.columns.emplace_back("created_at", DataType::TEXT, false);
        tables["system_schema"] = system_table;
        
        // Add default constraint
        Constraint pk_constraint(ConstraintType::PRIMARY_KEY, "system_schema_pk");
        pk_constraint.columns.push_back("version");
        constraints.push_back(pk_constraint);
    }
    
    bool IsInitialized() const { return is_initialized_; }
    void SetInitialized(bool initialized) { is_initialized_ = initialized; }
    
    bool CreateTable(const std::string& table_name, const std::vector<Column>& columns) {
        if (table_name.empty() || columns.empty()) return false;
        if (tables.find(table_name) != tables.end()) return false;
        
        Table new_table(table_name);
        new_table.columns = columns;
        
        // Find primary key column
        for (const auto& col : columns) {
            if (col.primary_key) {
                new_table.primary_key_column = col.name;
                break;
            }
        }
        
        tables[table_name] = new_table;
        return true;
    }
    
    bool DropTable(const std::string& table_name) {
        auto it = tables.find(table_name);
        if (it == tables.end() || table_name == "system_schema") {
            return false;
        }
        
        // Remove related constraints
        constraints.erase(
            std::remove_if(constraints.begin(), constraints.end(),
                [&table_name](const Constraint& c) {
                    return std::find(c.columns.begin(), c.columns.end(), table_name) != c.columns.end();
                }),
            constraints.end()
        );
        
        tables.erase(it);
        return true;
    }
    
    bool AddColumn(const std::string& table_name, const Column& column) {
        auto it = tables.find(table_name);
        if (it == tables.end()) return false;
        
        // Check if column already exists
        for (const auto& col : it->second.columns) {
            if (col.name == column.name) return false;
        }
        
        it->second.columns.push_back(column);
        return true;
    }
    
    bool DropColumn(const std::string& table_name, const std::string& column_name) {
        auto it = tables.find(table_name);
        if (it == tables.end()) return false;
        
        auto& columns = it->second.columns;
        auto col_it = std::find_if(columns.begin(), columns.end(),
            [&column_name](const Column& c) { return c.name == column_name; });
        
        if (col_it == columns.end()) return false;
        if (col_it->primary_key) return false; // Cannot drop primary key
        
        columns.erase(col_it);
        return true;
    }
    
    Table* GetTable(const std::string& table_name) {
        auto it = tables.find(table_name);
        return it != tables.end() ? &it->second : nullptr;
    }
    
    const Table* GetTable(const std::string& table_name) const {
        auto it = tables.find(table_name);
        return it != tables.end() ? &it->second : nullptr;
    }
    
    bool TableExists(const std::string& table_name) const {
        return tables.find(table_name) != tables.end();
    }
    
    std::vector<std::string> GetAllTableNames() const {
        std::vector<std::string> result;
        for (const auto& pair : tables) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    std::vector<std::string> GetTableColumns(const std::string& table_name) const {
        std::vector<std::string> result;
        auto it = tables.find(table_name);
        if (it != tables.end()) {
            for (const auto& col : it->second.columns) {
                result.push_back(col.name);
            }
        }
        return result;
    }
    
    bool AddConstraint(const Constraint& constraint) {
        constraints.push_back(constraint);
        return true;
    }
    
    std::vector<Constraint> GetTableConstraints(const std::string& table_name) const {
        std::vector<Constraint> result;
        for (const auto& constraint : constraints) {
            if (std::find(constraint.columns.begin(), constraint.columns.end(), table_name) != constraint.columns.end()) {
                result.push_back(constraint);
            }
        }
        return result;
    }
    
    size_t GetTableCount() const { return tables.size(); }
    size_t GetConstraintCount() const { return constraints.size(); }
};

} // namespace sqlcc

namespace sqlcc {
namespace test {

class SchemaManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        schema_mgr = std::make_unique<SchemaManager>();
    }
    
    void TearDown() override {
        schema_mgr.reset();
    }
    
    std::unique_ptr<SchemaManager> schema_mgr;
};

// Test basic functionality
TEST_F(SchemaManagerTest, DefaultConstructor) {
    EXPECT_TRUE(schema_mgr->IsInitialized());
    EXPECT_EQ(schema_mgr->GetTableCount(), 1);  // system_schema table
    EXPECT_EQ(schema_mgr->GetConstraintCount(), 1);  // system_schema_pk constraint
    EXPECT_TRUE(schema_mgr->TableExists("system_schema"));
}

// Test initialization state
TEST_F(SchemaManagerTest, InitializationState) {
    EXPECT_TRUE(schema_mgr->IsInitialized());
    
    schema_mgr->SetInitialized(false);
    EXPECT_FALSE(schema_mgr->IsInitialized());
    
    schema_mgr->SetInitialized(true);
    EXPECT_TRUE(schema_mgr->IsInitialized());
}

// Test table creation
TEST_F(SchemaManagerTest, CreateTable) {
    std::vector<Column> columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false),
        Column("age", DataType::INTEGER, true)
    };
    
    EXPECT_TRUE(schema_mgr->CreateTable("users", columns));
    EXPECT_EQ(schema_mgr->GetTableCount(), 2);  // system_schema + users
    EXPECT_TRUE(schema_mgr->TableExists("users"));
    
    auto* table = schema_mgr->GetTable("users");
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->name, "users");
    EXPECT_EQ(table->columns.size(), 3);
    EXPECT_EQ(table->primary_key_column, "id");
}

TEST_F(SchemaManagerTest, CreateEmptyTable) {
    EXPECT_FALSE(schema_mgr->CreateTable("", {}));
    EXPECT_FALSE(schema_mgr->CreateTable("empty", {}));
}

TEST_F(SchemaManagerTest, CreateDuplicateTable) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    
    EXPECT_TRUE(schema_mgr->CreateTable("users", columns));
    EXPECT_FALSE(schema_mgr->CreateTable("users", columns));  // Duplicate
}

// Test table dropping
TEST_F(SchemaManagerTest, DropTable) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    schema_mgr->CreateTable("temp_table", columns);
    EXPECT_EQ(schema_mgr->GetTableCount(), 2);
    
    EXPECT_TRUE(schema_mgr->DropTable("temp_table"));
    EXPECT_EQ(schema_mgr->GetTableCount(), 1);  // Back to system_schema only
    EXPECT_FALSE(schema_mgr->TableExists("temp_table"));
}

TEST_F(SchemaManagerTest, DropSystemTable) {
    EXPECT_FALSE(schema_mgr->DropTable("system_schema"));  // Cannot drop system table
}

TEST_F(SchemaManagerTest, DropNonExistentTable) {
    EXPECT_FALSE(schema_mgr->DropTable("nonexistent"));
}

// Test column management
TEST_F(SchemaManagerTest, AddColumn) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    schema_mgr->CreateTable("users", columns);
    
    Column name_column("name", DataType::TEXT, false);
    EXPECT_TRUE(schema_mgr->AddColumn("users", name_column));
    
    auto* table = schema_mgr->GetTable("users");
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->columns.size(), 2);
    EXPECT_EQ(table->columns[1].name, "name");
}

TEST_F(SchemaManagerTest, AddDuplicateColumn) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    schema_mgr->CreateTable("users", columns);
    
    Column id_column("id", DataType::INTEGER, false);  // Duplicate name
    EXPECT_FALSE(schema_mgr->AddColumn("users", id_column));
}

TEST_F(SchemaManagerTest, AddColumnToNonExistentTable) {
    Column name_column("name", DataType::TEXT, false);
    EXPECT_FALSE(schema_mgr->AddColumn("nonexistent", name_column));
}

TEST_F(SchemaManagerTest, DropColumn) {
    std::vector<Column> columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false),
        Column("age", DataType::INTEGER, true)
    };
    schema_mgr->CreateTable("users", columns);
    
    EXPECT_TRUE(schema_mgr->DropColumn("users", "age"));
    auto* table = schema_mgr->GetTable("users");
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->columns.size(), 2);
    EXPECT_EQ(table->columns[0].name, "id");
    EXPECT_EQ(table->columns[1].name, "name");
}

TEST_F(SchemaManagerTest, DropPrimaryKeyColumn) {
    std::vector<Column> columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false)
    };
    schema_mgr->CreateTable("users", columns);
    
    EXPECT_FALSE(schema_mgr->DropColumn("users", "id"));  // Cannot drop primary key
}

TEST_F(SchemaManagerTest, DropNonExistentColumn) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    schema_mgr->CreateTable("users", columns);
    
    EXPECT_FALSE(schema_mgr->DropColumn("users", "nonexistent"));
}

// Test table queries
TEST_F(SchemaManagerTest, GetAllTableNames) {
    std::vector<Column> columns = {Column("id", DataType::INTEGER, false, true)};
    schema_mgr->CreateTable("users", columns);
    schema_mgr->CreateTable("products", columns);
    
    auto table_names = schema_mgr->GetAllTableNames();
    EXPECT_EQ(table_names.size(), 3);  // system_schema, users, products
    
    bool found_users = std::find(table_names.begin(), table_names.end(), "users") != table_names.end();
    bool found_products = std::find(table_names.begin(), table_names.end(), "products") != table_names.end();
    EXPECT_TRUE(found_users);
    EXPECT_TRUE(found_products);
}

TEST_F(SchemaManagerTest, GetTableColumns) {
    std::vector<Column> columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false),
        Column("email", DataType::TEXT, true)
    };
    schema_mgr->CreateTable("users", columns);
    
    auto column_names = schema_mgr->GetTableColumns("users");
    EXPECT_EQ(column_names.size(), 3);
    EXPECT_EQ(column_names[0], "id");
    EXPECT_EQ(column_names[1], "name");
    EXPECT_EQ(column_names[2], "email");
}

TEST_F(SchemaManagerTest, GetTableColumnsNonExistentTable) {
    auto column_names = schema_mgr->GetTableColumns("nonexistent");
    EXPECT_TRUE(column_names.empty());
}

// Test constraints
TEST_F(SchemaManagerTest, AddConstraint) {
    Constraint unique_constraint(ConstraintType::UNIQUE, "users_email_unique");
    unique_constraint.columns.push_back("users");
    unique_constraint.columns.push_back("email");
    
    EXPECT_TRUE(schema_mgr->AddConstraint(unique_constraint));
    EXPECT_EQ(schema_mgr->GetConstraintCount(), 2);  // Default + new constraint
}

TEST_F(SchemaManagerTest, GetTableConstraints) {
    Constraint unique_constraint(ConstraintType::UNIQUE, "users_email_unique");
    unique_constraint.columns.push_back("users");
    unique_constraint.columns.push_back("email");
    schema_mgr->AddConstraint(unique_constraint);
    
    auto constraints = schema_mgr->GetTableConstraints("users");
    EXPECT_EQ(constraints.size(), 1);
    EXPECT_EQ(constraints[0].type, ConstraintType::UNIQUE);
    EXPECT_EQ(constraints[0].name, "users_email_unique");
}

// Test complex scenario
TEST_F(SchemaManagerTest, ComplexScenario) {
    // Create users table
    std::vector<Column> user_columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false),
        Column("email", DataType::TEXT, false)
    };
    schema_mgr->CreateTable("users", user_columns);
    
    // Create products table
    std::vector<Column> product_columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("name", DataType::TEXT, false),
        Column("price", DataType::REAL, false)
    };
    schema_mgr->CreateTable("products", product_columns);
    
    // Create orders table
    std::vector<Column> order_columns = {
        Column("id", DataType::INTEGER, false, true),
        Column("user_id", DataType::INTEGER, false),
        Column("product_id", DataType::INTEGER, false),
        Column("quantity", DataType::INTEGER, false)
    };
    schema_mgr->CreateTable("orders", order_columns);
    
    // Add constraints
    Constraint fk_users(ConstraintType::FOREIGN_KEY, "orders_user_fk");
    fk_users.columns.push_back("orders");
    fk_users.columns.push_back("user_id");
    fk_users.reference_table = "users";
    fk_users.reference_column = "id";
    schema_mgr->AddConstraint(fk_users);
    
    Constraint fk_products(ConstraintType::FOREIGN_KEY, "orders_product_fk");
    fk_products.columns.push_back("orders");
    fk_products.columns.push_back("product_id");
    fk_products.reference_table = "products";
    fk_products.reference_column = "id";
    schema_mgr->AddConstraint(fk_products);
    
    // Verify state
    EXPECT_EQ(schema_mgr->GetTableCount(), 4);  // system_schema + 3 custom tables
    EXPECT_EQ(schema_mgr->GetConstraintCount(), 3);  // Default + 2 foreign keys
    
    // Add column to orders
    Column status_column("status", DataType::TEXT, false, false, "pending");
    EXPECT_TRUE(schema_mgr->AddColumn("orders", status_column));
    
    auto* orders_table = schema_mgr->GetTable("orders");
    ASSERT_NE(orders_table, nullptr);
    EXPECT_EQ(orders_table->columns.size(), 5);  // id, user_id, product_id, quantity, status
    
    // Drop products table
    EXPECT_TRUE(schema_mgr->DropTable("products"));
    EXPECT_EQ(schema_mgr->GetTableCount(), 3);  // system_schema + users + orders
    EXPECT_FALSE(schema_mgr->TableExists("products"));
}

// Test edge cases
TEST_F(SchemaManagerTest, LargeSchema) {
    // Create many tables
    for (int i = 0; i < 50; ++i) {
        std::vector<Column> columns = {
            Column("id", DataType::INTEGER, false, true),
            Column("value", DataType::TEXT, false)
        };
        std::string table_name = "table_" + std::to_string(i);
        schema_mgr->CreateTable(table_name, columns);
    }
    
    EXPECT_EQ(schema_mgr->GetTableCount(), 51);  // system_schema + 50 tables
    
    // Verify we can access first and last tables
    EXPECT_TRUE(schema_mgr->TableExists("table_0"));
    EXPECT_TRUE(schema_mgr->TableExists("table_49"));
    
    auto* first_table = schema_mgr->GetTable("table_0");
    ASSERT_NE(first_table, nullptr);
    EXPECT_EQ(first_table->columns.size(), 2);
    
    auto* last_table = schema_mgr->GetTable("table_49");
    ASSERT_NE(last_table, nullptr);
    EXPECT_EQ(last_table->columns.size(), 2);
}

} // namespace test
} // namespace sqlcc