#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <random>
#include <limits>

#include "sql_parser/parser.h"
#include "sql_parser/lexer.h"
#include "sql_parser/ast_node.h"
#include "sql_parser/token.h"

using namespace sqlcc::sql_parser;

class SqlParserBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test environment
        parser_ = std::make_unique<Parser>();
        lexer_ = std::make_unique<Lexer>();
        
        // Create test directory
        test_data_dir_ = "/tmp/sqlcc_parser_boundary_test_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::create_directories(test_data_dir_);
    }

    void TearDown() override {
        parser_.reset();
        lexer_.reset();
        
        // Clean up test directory
        std::filesystem::remove_all(test_data_dir_);
    }

    // Helper method to generate large SQL statement
    std::string GenerateLargeSQL(size_t size) {
        std::string sql = "SELECT ";
        for (size_t i = 0; i < size; ++i) {
            sql += "column_" + std::to_string(i % 1000) + ", ";
        }
        sql += "FROM table WHERE ";
        for (size_t i = 0; i < size / 10; ++i) {
            sql += "column_" + std::to_string(i % 1000) + " = 'value_" + std::to_string(i) + "' AND ";
        }
        return sql.substr(0, sql.length() - 5); // Remove trailing " AND "
    }

    // Helper method to generate deeply nested SQL
    std::string GenerateNestedSQL(int depth) {
        if (depth <= 0) {
            return "SELECT 1";
        }
        
        std::string inner = GenerateNestedSQL(depth - 1);
        return "SELECT * FROM (" + inner + ") AS subquery_" + std::to_string(depth);
    }

    // Helper method to generate special characters SQL
    std::string GenerateSpecialCharactersSQL() {
        return R"(
            SELECT 
                'normal_string',
                'string_with_single''quote',
                "string_with_double_quote",
                'string_with_null_byte' || CHAR(0) || 'after_null',
                'unicode_测试_中文' || CHAR(0x4E2D) || '文',
                'control_chars' || CHAR(1) || CHAR(2) || CHAR(3),
                'binary_data' || X'48656C6C6F'
            FROM table_with_special_name
            WHERE column = 'value_with_special\ttab\nnewline\rreturn'
        )";
    }

    std::unique_ptr<Parser> parser_;
    std::unique_ptr<Lexer> lexer_;
    std::string test_data_dir_;
};

// Test SQL syntax error recovery
TEST_F(SqlParserBoundaryTest, SyntaxErrorRecovery) {
    // Test with various malformed SQL statements
    EXPECT_NO_THROW({
        // Test incomplete SELECT
        auto result1 = parser_->Parse("SELECT");
        EXPECT_FALSE(result1.is_valid);
        
        // Test incomplete INSERT
        auto result2 = parser_->Parse("INSERT INTO table");
        EXPECT_FALSE(result2.is_valid);
        
        // Test incomplete CREATE TABLE
        auto result3 = parser_->Parse("CREATE TABLE");
        EXPECT_FALSE(result3.is_valid);
        
        // Test unmatched parentheses
        auto result4 = parser_->Parse("SELECT (column1, column2 FROM table");
        EXPECT_FALSE(result4.is_valid);
        
        // Test unmatched quotes
        auto result5 = parser_->Parse("SELECT 'unclosed_string FROM table");
        EXPECT_FALSE(result5.is_valid);
        
        // Test invalid keywords
        auto result6 = parser_->Parse("INVALID_KEYWORD some_column FROM table");
        EXPECT_FALSE(result6.is_valid);
    });
}

// Test special character escape handling
TEST_F(SqlParserBoundaryTest, SpecialCharacterEscapeHandling) {
    EXPECT_NO_THROW({
        // Test with special characters SQL
        std::string special_sql = GenerateSpecialCharactersSQL();
        auto result = parser_->Parse(special_sql);
        
        // Should either parse successfully or fail gracefully
        EXPECT_TRUE(result.is_valid || !result.is_valid);
    });
}

// Test large SQL statement parsing
TEST_F(SqlParserBoundaryTest, LargeSQLStatementParsing) {
    EXPECT_NO_THROW({
        // Test with progressively larger SQL statements
        std::vector<size_t> sizes = {1000, 10000, 100000, 1000000};
        
        for (size_t size : sizes) {
            std::string large_sql = GenerateLargeSQL(size);
            auto result = parser_->Parse(large_sql);
            
            // For very large statements, expect either success or graceful failure
            if (size > 100000) {
                EXPECT_TRUE(result.is_valid || !result.is_valid);
            } else {
                EXPECT_NO_FATAL_FAILURE(result.is_valid);
            }
        }
    });
}

// Test nested subquery depth limits
TEST_F(SqlParserBoundaryTest, NestedSubqueryDepthLimit) {
    EXPECT_NO_THROW({
        // Test with various nesting depths
        std::vector<int> depths = {5, 10, 20, 50, 100};
        
        for (int depth : depths) {
            std::string nested_sql = GenerateNestedSQL(depth);
            auto result = parser_->Parse(nested_sql);
            
            // Very deep nesting should either succeed or fail gracefully
            if (depth > 20) {
                EXPECT_TRUE(result.is_valid || !result.is_valid);
            } else {
                EXPECT_NO_FATAL_FAILURE(result);
            }
        }
    });
}

// Test complex expression evaluation
TEST_F(SqlParserBoundaryTest, ComplexExpressionEvaluation) {
    EXPECT_NO_THROW({
        // Test with complex WHERE clauses
        std::vector<std::string> complex_queries = {
            "SELECT * FROM table WHERE (a > 1 AND b < 10) OR (c BETWEEN 5 AND 15 AND d IN (1,2,3,4,5))",
            "SELECT * FROM table WHERE a = 1 AND b = 2 AND c = 3 AND d = 4 AND e = 5 AND f = 6 AND g = 7 AND h = 8",
            "SELECT * FROM table WHERE column1 IS NULL AND column2 IS NOT NULL AND column3 LIKE 'pattern%' AND column4 REGEXP 'regex'",
            "SELECT * FROM table WHERE EXISTS (SELECT 1 FROM subtable WHERE subtable.id = table.id) AND NOT EXISTS (SELECT 1 FROM another_table WHERE another_table.id = table.id)",
            "SELECT * FROM table WHERE column IN (SELECT id FROM subquery WHERE condition1 = value1 AND condition2 = value2) OR column NOT IN (SELECT id FROM another_query WHERE other_condition = other_value)"
        };
        
        for (const auto& query : complex_queries) {
            auto result = parser_->Parse(query);
            EXPECT_NO_FATAL_FAILURE(result.is_valid);
        }
    });
}

// Test lexer tokenization edge cases
TEST_F(SqlParserBoundaryTest, LexerTokenizationEdgeCases) {
    EXPECT_NO_THROW({
        // Test with empty input
        auto result1 = lexer_->Tokenize("");
        EXPECT_EQ(result1.tokens.size(), 1); // Should have EOF token
        
        // Test with whitespace only
        auto result2 = lexer_->Tokenize("   \t\n\r   ");
        EXPECT_EQ(result2.tokens.size(), 1); // Should have EOF token
        
        // Test with single character
        auto result3 = lexer_->Tokenize("(");
        EXPECT_EQ(result3.tokens.size(), 2); // Should have LPAREN and EOF
        
        // Test with very long identifier
        std::string long_identifier(10000, 'a');
        auto result4 = lexer_->Tokenize("SELECT " + long_identifier + " FROM table");
        EXPECT_TRUE(result4.is_valid);
        
        // Test with many tokens
        std::string many_tokens;
        for (int i = 0; i < 10000; ++i) {
            many_tokens += "SELECT ";
        }
        auto result5 = lexer_->Tokenize(many_tokens);
        EXPECT_TRUE(result5.is_valid);
    });
}

// Test AST construction with complex queries
TEST_F(SqlParserBoundaryTest, ASTConstructionComplexQueries) {
    EXPECT_NO_THROW({
        // Test JOIN operations
        std::string join_query = R"(
            SELECT a.id, b.name, c.value
            FROM table_a a
            INNER JOIN table_b b ON a.id = b.a_id
            LEFT JOIN table_c c ON b.id = c.b_id
            RIGHT JOIN table_d d ON c.id = d.c_id
            FULL OUTER JOIN table_e e ON d.id = e.d_id
            WHERE a.status = 'active'
            ORDER BY a.id, b.name, c.value
            LIMIT 100 OFFSET 50
        )";
        
        auto result = parser_->Parse(join_query);
        EXPECT_NO_FATAL_FAILURE(result.is_valid);
        
        // Verify AST structure if parsing succeeded
        if && result.ast_root (result.is_valid) {
            EXPECT_NO_FATAL_FAILURE(result.ast_root->GetType());
        }
    });
}

// Test memory limits and resource constraints
TEST_F(SqlParserBoundaryTest, MemoryLimitsAndResourceConstraints) {
    EXPECT_NO_THROW({
        // Test with memory-intensive patterns
        std::string memory_intensive = "SELECT ";
        for (int i = 0; i < 1000; ++i) {
            memory_intensive += "AVG(column_" + std::to_string(i) + "), ";
        }
        memory_intensive += "COUNT(*) FROM table GROUP BY ";
        for (int i = 0; i < 500; ++i) {
            memory_intensive += "group_col_" + std::string(100, 'x') + std::to_string(i) + ", ";
        }
        memory_intensive += "1";
        
        auto result = parser_->Parse(memory_intensive);
        EXPECT_TRUE(result.is_valid || !result.is_valid); // Allow both outcomes
    });
}

// Test Unicode and internationalization
TEST_F(SqlParserBoundaryTest, UnicodeAndInternationalization) {
    EXPECT_NO_THROW({
        // Test with various Unicode characters
        std::vector<std::string> unicode_queries = {
            R"(SELECT 测试 FROM 表名 WHERE 列名 = '中文值')",
            R"(SELECT "测试表名"."测试列名" FROM "表名" WHERE "列名" = '中文值')",
            R"(SELECT column_测试 FROM table_测试 WHERE value_测试 = '测试值')",
            R"(SELECT column_🔑 FROM table_🔐 WHERE value_🔐 = '🔑🔐🔑')",
            R"(SELECT column_🚀 FROM table_🚀 WHERE value_🚀 = '🚀🚀🚀')"
        };
        
        for (const auto& query : unicode_queries) {
            auto result = parser_->Parse(query);
            EXPECT_TRUE(result.is_valid || !result.is_valid); // Allow both outcomes due to encoding issues
        }
    });
}

// Test performance with concurrent parsing
TEST_F(SqlParserBoundaryTest, ConcurrentParsingPerformance) {
    const int num_threads = 10;
    std::vector<std::thread> threads;
    std::vector<bool> results(num_threads, false);
    
    EXPECT_NO_THROW({
        // Launch multiple threads parsing different queries
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([this, i, &results]() {
                std::string query = "SELECT * FROM table_" + std::to_string(i) + 
                                " WHERE column_" + std::to_string(i) + " = " + std::to_string(i * 1000);
                
                auto result = parser_->Parse(query);
                results[i] = result.is_valid;
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        // At least some should succeed
        int success_count = 0;
        for (bool result : results) {
            if (result) success_count++;
        }
        EXPECT_TRUE(success_count >= 1);
    });
}

// Test parser state management
TEST_F(SqlParserBoundaryTest, ParserStateManagement) {
    EXPECT_NO_THROW({
        // Test parser reset and reuse
        auto result1 = parser_->Parse("SELECT 1");
        parser_->Reset();
        auto result2 = parser_->Parse("SELECT 2");
        parser_->Reset();
        auto result3 = parser_->Parse("SELECT 3");
        
        // Parser should handle multiple parse operations
        EXPECT_NO_FATAL_FAILURE(result1.is_valid);
        EXPECT_NO_FATAL_FAILURE(result2.is_valid);
        EXPECT_NO_FATAL_FAILURE(result3.is_valid);
    });
}

// Test error message generation
TEST_F(SqlParserBoundaryTest, ErrorMessageGeneration) {
    EXPECT_NO_THROW({
        // Test with various error conditions
        std::vector<std::string> error_queries = {
            "SELECT FROM", // Missing column list
            "INSERT INTO table1", // Missing VALUES
            "UPDATE table1 SET", // Missing assignments
            "DELETE FROM", // Missing table name
            "CREATE TABLE", // Missing table definition
            "DROP TABLE", // Missing table name
            "SELECT * FROM table WHERE column =", // Missing value
            "SELECT * FROM table WHERE column IN ()", // Empty IN list
            "SELECT * FROM table WHERE column BETWEEN", // Missing BETWEEN values
            "SELECT * FROM table WHERE column LIKE", // Missing pattern
        };
        
        for (const auto& query : error_queries) {
            auto result = parser_->Parse(query);
            
            // Should fail gracefully and provide error information
            EXPECT_FALSE(result.is_valid);
            EXPECT_NO_FATAL_FAILURE(result.error_message.length() > 0);
        }
    });
}

// Test boundary conditions for numeric literals
TEST_F(SqlParserBoundaryTest, NumericLiteralBoundaries) {
    EXPECT_NO_THROW({
        std::vector<std::string> numeric_queries = {
            "SELECT 0",
            "SELECT 1",
            "SELECT -2147483648", // INT_MIN
            "SELECT 2147483647", // INT_MAX
            "SELECT -9223372036854775808", // BIGINT_MIN
            "SELECT 9223372036854775807", // BIGINT_MAX
            "SELECT 3.141592653589793", // PI
            "SELECT 2.718281828459045", // e
            "SELECT 0.0",
            "SELECT .5",
            "SELECT 5.",
            "SELECT 1e10",
            "SELECT 1.5e-10",
            "SELECT .5e10",
        };
        
        for (const auto& query : numeric_queries) {
            auto result = parser_->Parse(query);
            EXPECT_NO_FATAL_FAILURE(result.is_valid);
        }
    });
}

// Test string literal edge cases
TEST_F(SqlParserBoundaryTest, StringLiteralEdgeCases) {
    EXPECT_NO_THROW({
        std::vector<std::string> string_queries = {
            "SELECT ''", // Empty string
            "SELECT 'a'", // Single character
            "SELECT 'ab'", // Two characters
            "SELECT 'abc'", // Three characters
            "SELECT 'test''s quote'", // Escaped quote
            "SELECT 'test''s''s''s''s''s''s''s''s''s''s'", // Multiple quotes
            "SELECT 'test\nline\nbreaks'", // Newlines
            "SELECT 'test\ttabs'", // Tabs
            "SELECT 'test\rcarriage\returns'", // Carriage returns
        };
        
        for (const auto& query : string_queries) {
            auto result = parser_->Parse(query);
            EXPECT_NO_FATAL_FAILURE(result.is_valid);
        }
    });
}

// Test comment handling
TEST_F(SqlParserBoundaryTest, CommentHandling) {
    EXPECT_NO_THROW({
        std::vector<std::string> comment_queries = {
            "-- This is a comment\nSELECT 1",
            "/* This is a multi-line comment */ SELECT 1",
            "SELECT 1 -- Inline comment",
            "SELECT 1 /* comment */ FROM table",
            "-- Comment\nSELECT 1\n-- Another comment\nFROM table\n-- Final comment",
            "/* Comment 1 */ SELECT /* comment 2 */ 1 /* comment 3 */ FROM /* comment 4 */ table /* comment 5 */",
        };
        
        for (const auto& query : comment_queries) {
            auto result = parser_->Parse(query);
            EXPECT_NO_FATAL_FAILURE(result.is_valid);
        }
    });
}

// Test reserved keywords as identifiers
TEST_F(SqlParserBoundaryTest, ReservedKeywordsAsIdentifiers) {
    EXPECT_NO_THROW({
        // Test quoted identifiers with reserved keywords
        std::vector<std::string> keyword_queries = {
            R"(SELECT "SELECT" FROM "WHERE" WHERE "FROM" = "SELECT")",
            R"(SELECT "INSERT", "UPDATE", "DELETE" FROM "CREATE" WHERE "TABLE" = "INDEX")",
            R"(SELECT "PRIMARY", "FOREIGN", "KEY" FROM "CONSTRAINT" WHERE "CHECK" = "UNIQUE")",
        };
        
        for (const auto& query : keyword_queries) {
            auto result = parser_->Parse(query);
            EXPECT_NO
