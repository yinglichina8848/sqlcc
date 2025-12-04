#include "sql_parser/lexer_new.h"
#include "sql_parser/parser.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>

namespace sqlcc {
namespace sql_parser {

// Integration test helper
void assertParseSuccess(const std::string& sql, const std::string& description) {
    try {
        Parser parser(sql);
        auto statements = parser.parseStatements();

        if (!statements.empty()) {
            std::cout << "✅ " << description << " - parsed successfully (" << statements.size() << " statements)" << std::endl;
        } else {
            std::cout << "❌ " << description << " - no statements parsed" << std::endl;
            assert(false);
        }
    } catch (const std::exception& e) {
        std::cout << "❌ " << description << " - parse error: " << e.what() << std::endl;
        assert(false);
    }
}

void assertParseFailure(const std::string& sql, const std::string& description) {
    try {
        Parser parser(sql);
        auto statements = parser.parseStatements();
        std::cout << "❌ " << description << " - expected parse failure but succeeded" << std::endl;
        assert(false);
    } catch (const std::exception& e) {
        std::cout << "✅ " << description << " - correctly failed to parse: " << e.what() << std::endl;
    }
}

// Test DDL statements with DFA lexer
void testDDLStatements() {
    std::cout << "🧪 Testing DDL Statements with DFA Lexer..." << std::endl;

    // CREATE DATABASE
    assertParseSuccess("CREATE DATABASE testdb;", "CREATE DATABASE");

    // CREATE TABLE with various data types and constraints
    assertParseSuccess(
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTO_INCREMENT,"
        "username VARCHAR(50) NOT NULL UNIQUE,"
        "email VARCHAR(100),"
        "age INTEGER,"
        "balance DECIMAL(10,2) DEFAULT 0.00,"
        "created_at TIMESTAMP"
        ");",
        "CREATE TABLE with constraints"
    );

    // CREATE INDEX
    assertParseSuccess("CREATE INDEX idx_username ON users (username);", "CREATE INDEX");

    // DROP TABLE
    assertParseSuccess("DROP TABLE users;", "DROP TABLE");

    // ALTER TABLE
    assertParseSuccess("ALTER DATABASE testdb;", "ALTER DATABASE");

    std::cout << "✅ DDL statements test completed" << std::endl;
}

// Test DML statements with DFA lexer
void testDMLStatements() {
    std::cout << "🧪 Testing DML Statements with DFA Lexer..." << std::endl;

    // Simple SELECT
    assertParseSuccess("SELECT * FROM users;", "Simple SELECT");

    // SELECT with WHERE
    assertParseSuccess(
        "SELECT id, username, email FROM users WHERE age > 18 AND status = 'active';",
        "SELECT with WHERE"
    );

    // SELECT with JOIN
    assertParseSuccess(
        "SELECT u.name, p.title FROM users u INNER JOIN posts p ON u.id = p.user_id;",
        "SELECT with JOIN"
    );

    // INSERT
    assertParseSuccess(
        "INSERT INTO users (username, email, age) VALUES ('john', 'john@example.com', 25);",
        "INSERT statement"
    );

    // UPDATE
    assertParseSuccess(
        "UPDATE users SET email = 'new@example.com', age = 26 WHERE id = 1;",
        "UPDATE statement"
    );

    // DELETE
    assertParseSuccess("DELETE FROM users WHERE id = 1;", "DELETE statement");

    std::cout << "✅ DML statements test completed" << std::endl;
}

// Test DCL statements with DFA lexer
void testDCLStatements() {
    std::cout << "🧪 Testing DCL Statements with DFA Lexer..." << std::endl;

    // GRANT
    assertParseSuccess(
        "GRANT SELECT, INSERT ON TABLE users TO john;",
        "GRANT privileges"
    );

    // REVOKE
    assertParseSuccess(
        "REVOKE INSERT ON TABLE users FROM john;",
        "REVOKE privileges"
    );

    // CREATE USER
    assertParseSuccess(
        "CREATE USER admin IDENTIFIED BY 'password123';",
        "CREATE USER"
    );

    // DROP USER
    assertParseSuccess("DROP USER admin;", "DROP USER");

    std::cout << "✅ DCL statements test completed" << std::endl;
}

// Test complex SQL with DFA lexer
void testComplexSQL() {
    std::cout << "🧪 Testing Complex SQL with DFA Lexer..." << std::endl;

    // Complex CREATE TABLE
    assertParseSuccess(
        "CREATE TABLE orders ("
        "id INTEGER PRIMARY KEY,"
        "user_id INTEGER REFERENCES users(id),"
        "product_name VARCHAR(255) NOT NULL,"
        "quantity INTEGER DEFAULT 1,"
        "price DECIMAL(8,2) NOT NULL,"
        "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "status VARCHAR(20) DEFAULT 'pending' CHECK (status IN ('pending', 'shipped', 'delivered'))"
        ");",
        "Complex CREATE TABLE"
    );

    // Complex SELECT with subqueries (simplified)
    assertParseSuccess(
        "SELECT u.username, COUNT(o.id) as order_count "
        "FROM users u "
        "LEFT JOIN orders o ON u.id = o.user_id "
        "WHERE u.created_at > '2023-01-01' "
        "GROUP BY u.id, u.username "
        "HAVING COUNT(o.id) > 0 "
        "ORDER BY order_count DESC "
        "LIMIT 10;",
        "Complex SELECT with aggregation"
    );

    // Multiple statements
    assertParseSuccess(
        "CREATE DATABASE shop; "
        "USE shop; "
        "CREATE TABLE products (id INT, name VARCHAR(100)); "
        "INSERT INTO products VALUES (1, 'Laptop');",
        "Multiple statements"
    );

    std::cout << "✅ Complex SQL test completed" << std::endl;
}

// Test comments handling
void testComments() {
    std::cout << "🧪 Testing Comments Handling..." << std::endl;

    // Single line comments
    assertParseSuccess(
        "-- This is a comment\nSELECT * FROM users; -- Another comment",
        "Single line comments"
    );

    // Multi-line comments
    assertParseSuccess(
        "SELECT /* this is a multi-line\n   comment */ * FROM users;",
        "Multi-line comments"
    );

    // Mixed comments
    assertParseSuccess(
        "/* Start */ SELECT * /* middle */ FROM users; -- End",
        "Mixed comments"
    );

    std::cout << "✅ Comments handling test completed" << std::endl;
}

// Test literals and identifiers
void testLiteralsAndIdentifiers() {
    std::cout << "🧪 Testing Literals and Identifiers..." << std::endl;

    // String literals
    assertParseSuccess("SELECT * FROM users WHERE name = 'John';", "String literals");

    // Numeric literals
    assertParseSuccess("SELECT * FROM products WHERE price > 99.99;", "Numeric literals");

    // Scientific notation
    assertParseSuccess("SELECT * FROM data WHERE value > 1.23e10;", "Scientific notation");

    // Identifiers with underscores
    assertParseSuccess("SELECT user_name, user_email FROM user_table;", "Underscore identifiers");

    std::cout << "✅ Literals and identifiers test completed" << std::endl;
}

// Test error handling
void testErrorHandling() {
    std::cout << "🧪 Testing Error Handling..." << std::endl;

    // Invalid syntax
    assertParseFailure("SELECT * FROM;", "Missing table name");

    // Unterminated string
    assertParseFailure("SELECT * FROM users WHERE name = 'unterminated;", "Unterminated string");

    // Invalid keyword
    assertParseFailure("SELET * FROM users;", "Typo in keyword");

    std::cout << "✅ Error handling test completed" << std::endl;
}

// Test performance - quick smoke test
void testPerformanceSmoke() {
    std::cout << "🧪 Performance Smoke Test..." << std::endl;

    const std::string complexSQL = R"(
        CREATE TABLE complex_table (
            id INTEGER PRIMARY KEY AUTO_INCREMENT,
            name VARCHAR(255) NOT NULL,
            description TEXT,
            price DECIMAL(10,2),
            quantity INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
            UNIQUE KEY unique_name (name),
            INDEX idx_price (price),
            CHECK (price > 0),
            CHECK (quantity >= 0)
        );
    )";

    auto start = std::chrono::high_resolution_clock::now();

    // Parse multiple times to test performance
    for (int i = 0; i < 100; ++i) {
        Parser parser(complexSQL);
        auto statements = parser.parseStatements();
        assert(!statements.empty());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "✅ Performance smoke test: 100 complex parses in " << duration.count() << "ms" << std::endl;
    std::cout << "   Average: " << (duration.count() / 100.0) << "ms per parse" << std::endl;
}

} // namespace sql_parser
} // namespace sqlcc

int main() {
    std::cout << "🚀 DFA Lexer Integration Test Suite" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Testing DFA lexer integration with SQL parser..." << std::endl;
    std::cout << std::endl;

    try {
        // Run all integration tests
        sqlcc::sql_parser::testDDLStatements();
        std::cout << std::endl;

        sqlcc::sql_parser::testDMLStatements();
        std::cout << std::endl;

        sqlcc::sql_parser::testDCLStatements();
        std::cout << std::endl;

        sqlcc::sql_parser::testComplexSQL();
        std::cout << std::endl;

        sqlcc::sql_parser::testComments();
        std::cout << std::endl;

        sqlcc::sql_parser::testLiteralsAndIdentifiers();
        std::cout << std::endl;

        sqlcc::sql_parser::testErrorHandling();
        std::cout << std::endl;

        sqlcc::sql_parser::testPerformanceSmoke();
        std::cout << std::endl;

        std::cout << "===================================" << std::endl;
        std::cout << "🎉 All DFA Lexer Integration Tests PASSED!" << std::endl;
        std::cout << std::endl;
        std::cout << "✅ DFA lexer successfully integrated with SQL parser" << std::endl;
        std::cout << "✅ All SQL statement types parse correctly" << std::endl;
        std::cout << "✅ Token system migration completed successfully" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "===================================" << std::endl;
        std::cout << "❌ Integration test FAILED: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
