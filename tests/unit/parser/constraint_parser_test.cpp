#include <gtest/gtest.h>
#include <string>
#include <memory>
#include "parser.h"

using namespace sqlcc::sql_parser;

// Test fixture for constraint parser testing
class ConstraintParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test parsing foreign key constraints
TEST_F(ConstraintParserTest, ParseForeignKeyConstraint) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT fk_customer "
                     "FOREIGN KEY (customer_id) REFERENCES customers(id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());

    // Verify the parsed statement is an ALTER statement
    // This would need actual AST verification based on your parser implementation
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing check constraints with expressions
TEST_F(ConstraintParserTest, ParseCheckConstraintWithExpression) {
    std::string sql = "ALTER TABLE employees ADD CONSTRAINT check_salary "
                     "CHECK (salary > 0 AND salary < 1000000);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing unique constraints with multiple columns
TEST_F(ConstraintParserTest, ParseUniqueConstraintWithMultipleColumns) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT unique_email_phone "
                     "UNIQUE (email, phone_number);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing primary key constraints
TEST_F(ConstraintParserTest, ParsePrimaryKeyConstraint) {
    std::string sql = "ALTER TABLE departments ADD CONSTRAINT pk_dept "
                     "PRIMARY KEY (dept_id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing NOT NULL constraints
TEST_F(ConstraintParserTest, ParseNotNullConstraint) {
    std::string sql = "ALTER TABLE users MODIFY COLUMN email VARCHAR(255) NOT NULL;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    // Verify NOT NULL constraint is parsed
}

// Test parsing default value constraints
TEST_F(ConstraintParserTest, ParseDefaultConstraint) {
    std::string sql = "ALTER TABLE orders ADD COLUMN status VARCHAR(20) DEFAULT 'pending';";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
}

// Test parsing composite foreign key constraints
TEST_F(ConstraintParserTest, ParseCompositeForeignKeyConstraint) {
    std::string sql = "ALTER TABLE order_items ADD CONSTRAINT fk_order_product "
                     "FOREIGN KEY (order_id, product_id) "
                     "REFERENCES orders_products(order_id, product_id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with CASCADE options
TEST_F(ConstraintParserTest, ParseForeignKeyWithCascade) {
    std::string sql = "ALTER TABLE child_table ADD CONSTRAINT fk_parent "
                     "FOREIGN KEY (parent_id) REFERENCES parent_table(id) "
                     "ON DELETE CASCADE ON UPDATE CASCADE;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with SET NULL options
TEST_F(ConstraintParserTest, ParseForeignKeyWithSetNull) {
    std::string sql = "ALTER TABLE child_table ADD CONSTRAINT fk_parent "
                     "FOREIGN KEY (parent_id) REFERENCES parent_table(id) "
                     "ON DELETE SET NULL;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing check constraints with functions
TEST_F(ConstraintParserTest, ParseCheckConstraintWithFunctions) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT check_age "
                     "CHECK (AGE(birth_date) >= 18 AND AGE(birth_date) <= 120);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing unique constraints on single column
TEST_F(ConstraintParserTest, ParseUniqueConstraintSingleColumn) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT unique_username UNIQUE (username);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with custom names
TEST_F(ConstraintParserTest, ParseConstraintWithCustomName) {
    std::string sql = "ALTER TABLE products ADD CONSTRAINT positive_price "
                     "CHECK (price >= 0);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing multiple constraints in one statement
TEST_F(ConstraintParserTest, ParseMultipleConstraints) {
    std::string sql = "ALTER TABLE users "
                     "ADD CONSTRAINT pk_users PRIMARY KEY (id), "
                     "ADD CONSTRAINT unique_email UNIQUE (email), "
                     "ADD CONSTRAINT check_age CHECK (age >= 0);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with table creation
TEST_F(ConstraintParserTest, ParseConstraintsInCreateTable) {
    std::string sql = "CREATE TABLE employees ("
                     "id INT PRIMARY KEY,"
                     "email VARCHAR(255) UNIQUE NOT NULL,"
                     "salary DECIMAL(10,2) CHECK (salary > 0),"
                     "dept_id INT REFERENCES departments(id)"
                     ");";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing self-referencing foreign keys
TEST_F(ConstraintParserTest, ParseSelfReferencingForeignKey) {
    std::string sql = "ALTER TABLE employees ADD CONSTRAINT fk_manager "
                     "FOREIGN KEY (manager_id) REFERENCES employees(id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with expressions involving NULL
TEST_F(ConstraintParserTest, ParseCheckConstraintWithNullCheck) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT check_optional_field "
                     "CHECK (optional_field IS NULL OR LENGTH(optional_field) > 5);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with BETWEEN operator
TEST_F(ConstraintParserTest, ParseCheckConstraintWithBetween) {
    std::string sql = "ALTER TABLE products ADD CONSTRAINT check_rating "
                     "CHECK (rating BETWEEN 1 AND 5);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with IN operator
TEST_F(ConstraintParserTest, ParseCheckConstraintWithIn) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT check_status "
                     "CHECK (status IN ('pending', 'processing', 'completed'));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with subqueries in check
TEST_F(ConstraintParserTest, ParseCheckConstraintWithSubquery) {
    std::string sql = "ALTER TABLE salaries ADD CONSTRAINT check_max_salary "
                     "CHECK (amount <= (SELECT MAX(amount) * 1.5 FROM salaries));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with complex expressions
TEST_F(ConstraintParserTest, ParseCheckConstraintComplexExpression) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT check_order_total "
                     "CHECK ((quantity * price) > 10 AND (quantity * price) < 10000);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints error handling
TEST_F(ConstraintParserTest, ParseInvalidConstraintSyntax) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT CHECK;"; // Invalid syntax

    Parser parser(sql);
    auto statements = parser.parse();
    // Should handle parsing errors gracefully
    // Depending on your parser implementation, may return empty or partial results
}

// Test parsing constraints with schema-qualified names
TEST_F(ConstraintParserTest, ParseConstraintWithSchemaQualification) {
    std::string sql = "ALTER TABLE sales.orders ADD CONSTRAINT fk_customer "
                     "FOREIGN KEY (customer_id) REFERENCES crm.customers(id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing deferrable constraints
TEST_F(ConstraintParserTest, ParseDeferrableConstraint) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT fk_customer "
                     "FOREIGN KEY (customer_id) REFERENCES customers(id) DEFERRABLE;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing initially deferred constraints
TEST_F(ConstraintParserTest, ParseInitiallyDeferredConstraint) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT fk_customer "
                     "FOREIGN KEY (customer_id) REFERENCES customers(id) "
                     "DEFERRABLE INITIALLY DEFERRED;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex CHECK constraints with mathematical expressions
TEST_F(ConstraintParserTest, ParseComplexCheckWithMath) {
    std::string sql = "ALTER TABLE products ADD CONSTRAINT check_price_calc "
                     "CHECK (price * discount > cost + (shipping * 1.1));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with string functions in CHECK
TEST_F(ConstraintParserTest, ParseCheckWithStringFunctions) {
    std::string sql = "ALTER TABLE users ADD CONSTRAINT check_email_format "
                     "CHECK (POSITION('@' IN email) > 0 AND LENGTH(email) > 5);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing multiple column UNIQUE constraints
TEST_F(ConstraintParserTest, ParseMultiColumnUniqueConstraint) {
    std::string sql = "ALTER TABLE user_sessions ADD CONSTRAINT unique_user_time "
                     "UNIQUE (user_id, login_time);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing PRIMARY KEY constraints with multiple columns
TEST_F(ConstraintParserTest, ParseMultiColumnPrimaryKey) {
    std::string sql = "ALTER TABLE composite_keys ADD CONSTRAINT pk_composite "
                     "PRIMARY KEY (tenant_id, entity_id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with quoted identifiers
TEST_F(ConstraintParserTest, ParseConstraintsWithQuotedIdentifiers) {
    std::string sql = "ALTER TABLE \"Order Details\" ADD CONSTRAINT \"fk_order\" "
                     "FOREIGN KEY (\"Order ID\") REFERENCES \"Orders\"(\"Order ID\");";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with comments
TEST_F(ConstraintParserTest, ParseConstraintsWithComments) {
    std::string sql = "-- This is a foreign key constraint\n"
                     "ALTER TABLE orders ADD CONSTRAINT fk_customer -- references customer table\n"
                     "FOREIGN KEY (customer_id) REFERENCES customers(id); -- end constraint";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing cascading delete with restrict
TEST_F(ConstraintParserTest, ParseForeignKeyWithRestrict) {
    std::string sql = "ALTER TABLE child_table ADD CONSTRAINT fk_parent "
                     "FOREIGN KEY (parent_id) REFERENCES parent_table(id) "
                     "ON DELETE RESTRICT ON UPDATE RESTRICT;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing NOT NULL constraints with ALTER TABLE
TEST_F(ConstraintParserTest, ParseNotNullAlterTable) {
    std::string sql = "ALTER TABLE users ALTER COLUMN email SET NOT NULL;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing DEFAULT constraints with functions
TEST_F(ConstraintParserTest, ParseDefaultWithFunction) {
    std::string sql = "ALTER TABLE audit_log ADD COLUMN created_at TIMESTAMP "
                     "DEFAULT CURRENT_TIMESTAMP;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing CHECK constraints with subqueries
TEST_F(ConstraintParserTest, ParseCheckWithSubquery) {
    std::string sql = "ALTER TABLE orders ADD CONSTRAINT check_customer_exists "
                     "CHECK (customer_id IN (SELECT id FROM customers));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with very long names
TEST_F(ConstraintParserTest, ParseConstraintsWithLongNames) {
    std::string sql = "ALTER TABLE very_long_table_name ADD CONSTRAINT "
                     "very_long_constraint_name_that_exceeds_normal_length_limits_and_tests_parser_limits "
                     "CHECK (some_column > 0);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing multiple constraints in single ALTER TABLE
TEST_F(ConstraintParserTest, ParseMultipleConstraintsInAlter) {
    std::string sql = "ALTER TABLE comprehensive_table "
                     "ADD CONSTRAINT pk_comp PRIMARY KEY (id), "
                     "ADD CONSTRAINT fk_ref FOREIGN KEY (ref_id) REFERENCES ref_table(id), "
                     "ADD CONSTRAINT chk_positive CHECK (value > 0), "
                     "ADD CONSTRAINT unq_name UNIQUE (name);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing constraints with Unicode characters
TEST_F(ConstraintParserTest, ParseConstraintsWithUnicode) {
    std::string sql = "ALTER TABLE (7h ADD CONSTRAINT (7. "
                     "FOREIGN KEY ((7ID) REFERENCES (7h(id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing circular foreign key references (self-referencing with different columns)
TEST_F(ConstraintParserTest, ParseCircularForeignKey) {
    std::string sql = "ALTER TABLE employees ADD CONSTRAINT fk_manager "
                     "FOREIGN KEY (manager_id) REFERENCES employees(employee_id);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}