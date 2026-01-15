#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include "parser.h"
#include "json.h"

using namespace sqlcc::sql_parser;

// Test fixture for JSON parser testing
class JsonParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test parsing JSON_EXTRACT function
TEST_F(JsonParserTest, ParseJsonExtractFunction) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.name') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_SET function
TEST_F(JsonParserTest, ParseJsonSetFunction) {
    std::string sql = "UPDATE users SET data = JSON_SET(data, '$.age', 25);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_INSERT function
TEST_F(JsonParserTest, ParseJsonInsertFunction) {
    std::string sql = "UPDATE users SET data = JSON_INSERT(data, '$.city', 'New York');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_REPLACE function
TEST_F(JsonParserTest, ParseJsonReplaceFunction) {
    std::string sql = "UPDATE users SET data = JSON_REPLACE(data, '$.status', 'active');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_REMOVE function
TEST_F(JsonParserTest, ParseJsonRemoveFunction) {
    std::string sql = "UPDATE users SET data = JSON_REMOVE(data, '$.temp');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_CONTAINS function
TEST_F(JsonParserTest, ParseJsonContainsFunction) {
    std::string sql = "SELECT * FROM users WHERE JSON_CONTAINS(data, 'admin', '$.roles');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_SEARCH function
TEST_F(JsonParserTest, ParseJsonSearchFunction) {
    std::string sql = "SELECT JSON_SEARCH(data, 'one', 'John') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_KEYS function
TEST_F(JsonParserTest, ParseJsonKeysFunction) {
    std::string sql = "SELECT JSON_KEYS(data) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_DEPTH function
TEST_F(JsonParserTest, ParseJsonDepthFunction) {
    std::string sql = "SELECT JSON_DEPTH(data) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_LENGTH function
TEST_F(JsonParserTest, ParseJsonLengthFunction) {
    std::string sql = "SELECT JSON_LENGTH(data, '$.items') FROM orders;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_TYPE function
TEST_F(JsonParserTest, ParseJsonTypeFunction) {
    std::string sql = "SELECT JSON_TYPE(data, '$.age') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_VALID function
TEST_F(JsonParserTest, ParseJsonValidFunction) {
    std::string sql = "SELECT JSON_VALID(data) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_QUOTE function
TEST_F(JsonParserTest, ParseJsonQuoteFunction) {
    std::string sql = "SELECT JSON_QUOTE(name) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_UNQUOTE function
TEST_F(JsonParserTest, ParseJsonUnquoteFunction) {
    std::string sql = "SELECT JSON_UNQUOTE(data->>'$.name') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_ARRAY function
TEST_F(JsonParserTest, ParseJsonArrayFunction) {
    std::string sql = "SELECT JSON_ARRAY(1, 2, 3, 'test');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_OBJECT function
TEST_F(JsonParserTest, ParseJsonObjectFunction) {
    std::string sql = "SELECT JSON_OBJECT('name', 'John', 'age', 30);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_MERGE function
TEST_F(JsonParserTest, ParseJsonMergeFunction) {
    std::string sql = "SELECT JSON_MERGE(data1, data2) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_PRETTY function
TEST_F(JsonParserTest, ParseJsonPrettyFunction) {
    std::string sql = "SELECT JSON_PRETTY(data) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON_STORAGE_SIZE function
TEST_F(JsonParserTest, ParseJsonStorageSizeFunction) {
    std::string sql = "SELECT JSON_STORAGE_SIZE(data) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with path expressions
TEST_F(JsonParserTest, ParseJsonPathExpressions) {
    std::string sql = "SELECT data->>'$.name' as name, data->'$.age' as age FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON column references
TEST_F(JsonParserTest, ParseJsonColumnReferences) {
    std::string sql = "SELECT data->'$.address.city' FROM users WHERE data->>'$.status' = 'active';";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in WHERE clauses
TEST_F(JsonParserTest, ParseJsonInWhereClause) {
    std::string sql = "SELECT * FROM users WHERE JSON_EXTRACT(data, '$.age') > 21;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in ORDER BY clauses
TEST_F(JsonParserTest, ParseJsonInOrderBy) {
    std::string sql = "SELECT * FROM users ORDER BY JSON_EXTRACT(data, '$.score') DESC;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in GROUP BY clauses
TEST_F(JsonParserTest, ParseJsonInGroupBy) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.category'), COUNT(*) FROM products GROUP BY JSON_EXTRACT(data, '$.category');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in HAVING clauses
TEST_F(JsonParserTest, ParseJsonInHaving) {
    std::string sql = "SELECT category, COUNT(*) FROM products GROUP BY category HAVING JSON_LENGTH(data) > 5;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON array access
TEST_F(JsonParserTest, ParseJsonArrayAccess) {
    std::string sql = "SELECT data->>'$[0].name' FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON nested object access
TEST_F(JsonParserTest, ParseJsonNestedAccess) {
    std::string sql = "SELECT data->>'$.profile.settings.theme' FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with complex paths
TEST_F(JsonParserTest, ParseJsonComplexPaths) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.items[0].details[*].value') FROM orders;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with wildcard paths
TEST_F(JsonParserTest, ParseJsonWildcardPaths) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.*.name') FROM companies;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with descendant paths
TEST_F(JsonParserTest, ParseJsonDescendantPaths) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$..email') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON functions with NULL handling
TEST_F(JsonParserTest, ParseJsonWithNullHandling) {
    std::string sql = "SELECT COALESCE(JSON_EXTRACT(data, '$.optional'), 'default') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in subqueries
TEST_F(JsonParserTest, ParseJsonInSubqueries) {
    std::string sql = "SELECT * FROM users WHERE id IN (SELECT JSON_EXTRACT(settings, '$.user_id') FROM preferences);";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with aggregation functions
TEST_F(JsonParserTest, ParseJsonWithAggregation) {
    std::string sql = "SELECT AVG(CAST(JSON_EXTRACT(metrics, '$.response_time') AS DECIMAL)) FROM logs;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON functions with error handling
TEST_F(JsonParserTest, ParseJsonErrorHandling) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.invalid.path') FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with special characters in paths
TEST_F(JsonParserTest, ParseJsonSpecialCharacters) {
    std::string sql = "SELECT data->>'$.special-key' FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in CREATE TABLE statements
TEST_F(JsonParserTest, ParseJsonInCreateTable) {
    std::string sql = "CREATE TABLE user_profiles (id INT, data JSON, created_at TIMESTAMP DEFAULT JSON_EXTRACT(data, '$.timestamp'));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON in ALTER TABLE statements
TEST_F(JsonParserTest, ParseJsonInAlterTable) {
    std::string sql = "ALTER TABLE users ADD COLUMN age INT GENERATED ALWAYS AS (JSON_EXTRACT(data, '$.age'));";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing complex JSON expressions
TEST_F(JsonParserTest, ParseComplexJsonExpressions) {
    std::string sql = "SELECT JSON_SET(JSON_SET(data, '$.updated', NOW()), '$.version', JSON_EXTRACT(data, '$.version') + 1) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with conditional logic
TEST_F(JsonParserTest, ParseJsonConditionalLogic) {
    std::string sql = "UPDATE users SET data = CASE WHEN JSON_TYPE(data, '$.status') = 'string' THEN JSON_SET(data, '$.processed', true) ELSE data END;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with UNION operations
TEST_F(JsonParserTest, ParseJsonWithUnion) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.name') as name FROM users UNION SELECT JSON_EXTRACT(settings, '$.title') FROM configs;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with JOIN operations
TEST_F(JsonParserTest, ParseJsonWithJoin) {
    std::string sql = "SELECT u.name, p.title FROM users u JOIN preferences p ON JSON_EXTRACT(u.data, '$.id') = JSON_EXTRACT(p.data, '$.user_id');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with window functions
TEST_F(JsonParserTest, ParseJsonWithWindowFunctions) {
    std::string sql = "SELECT name, JSON_EXTRACT(data, '$.score'), ROW_NUMBER() OVER (ORDER BY JSON_EXTRACT(data, '$.score') DESC) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with recursive queries
TEST_F(JsonParserTest, ParseJsonWithRecursiveQueries) {
    std::string sql = "WITH RECURSIVE hierarchy AS (SELECT id, JSON_EXTRACT(data, '$.parent_id') as parent_id FROM users WHERE JSON_EXTRACT(data, '$.parent_id') IS NULL UNION ALL SELECT u.id, JSON_EXTRACT(u.data, '$.parent_id') FROM users u JOIN hierarchy h ON JSON_EXTRACT(u.data, '$.parent_id') = h.id) SELECT * FROM hierarchy;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with very nested paths
TEST_F(JsonParserTest, ParseJsonVeryNestedPaths) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.level1.level2.level3.level4.very.deep.value') FROM complex_data;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with array operations
TEST_F(JsonParserTest, ParseJsonArrayOperations) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.items[0:5]') FROM collections;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with filter expressions
TEST_F(JsonParserTest, ParseJsonFilterExpressions) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.items[?(@.status == \"active\")].name') FROM orders;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with mathematical operations
TEST_F(JsonParserTest, ParseJsonMathematicalOperations) {
    std::string sql = "SELECT JSON_SET(data, '$.total', JSON_EXTRACT(data, '$.price') * JSON_EXTRACT(data, '$.quantity')) FROM cart;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with string operations
TEST_F(JsonParserTest, ParseJsonStringOperations) {
    std::string sql = "SELECT JSON_SET(data, '$.display_name', UPPER(JSON_EXTRACT(data, '$.first_name') || ' ' || JSON_EXTRACT(data, '$.last_name'))) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with date operations
TEST_F(JsonParserTest, ParseJsonDateOperations) {
    std::string sql = "SELECT JSON_SET(data, '$.age', EXTRACT(YEAR FROM NOW()) - EXTRACT(YEAR FROM JSON_EXTRACT(data, '$.birth_date'))) FROM users;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with very long paths
TEST_F(JsonParserTest, ParseJsonVeryLongPaths) {
    std::string sql = "SELECT JSON_EXTRACT(settings, '$.application.modules.database.connection.pool.size.maximum.allowed.connections') FROM configs;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with Unicode characters
TEST_F(JsonParserTest, ParseJsonUnicodeCharacters) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.résumé.éducation.niveau') FROM profiles;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with escaped characters
TEST_F(JsonParserTest, ParseJsonEscapedCharacters) {
    std::string sql = "SELECT JSON_EXTRACT(data, '$.special\\\"key') FROM configs;";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}

// Test parsing JSON with multiple functions in complex query
TEST_F(JsonParserTest, ParseJsonMultipleFunctionsComplex) {
    std::string sql = "SELECT id, JSON_PRETTY(JSON_SET(data, '$.computed.total', JSON_EXTRACT(data, '$.price') + JSON_EXTRACT(data, '$.tax'))) as computed_data FROM invoices WHERE JSON_TYPE(data, '$.status') = 'string' AND JSON_CONTAINS(JSON_EXTRACT(data, '$.tags'), 'urgent');";

    Parser parser(sql);
    auto statements = parser.parse();
    ASSERT_FALSE(statements.empty());
    EXPECT_EQ(statements.size(), 1);
}
