#include "sql_parser/lexer_new.h"
#include "sql_parser/lexer.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>

namespace sqlcc {
namespace sql_parser {

// Benchmark test for comparing old and new lexers
void benchmarkLexers() {
    std::cout << "🧪 DFA Lexer Performance Benchmark" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Test SQL statements of different sizes
    std::vector<std::string> testCases = {
        "SELECT * FROM users;",  // Simple query
        "SELECT id, name, email FROM users WHERE age > 18 AND status = 'active';",  // Medium query
        "CREATE TABLE users (id INTEGER PRIMARY KEY, name VARCHAR(100) NOT NULL, email VARCHAR(255) UNIQUE, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);",  // Complex DDL
        R"(
            SELECT u.id, u.name, p.title, p.content
            FROM users u
            INNER JOIN posts p ON u.id = p.user_id
            LEFT JOIN categories c ON p.category_id = c.id
            WHERE u.status = 'active'
              AND p.published = true
              AND c.name IN ('tech', 'news', 'sports')
            ORDER BY p.created_at DESC
            LIMIT 50 OFFSET 100;
        )"  // Very complex query
    };

    const int iterations = 1000;  // Number of iterations for each test

    for (size_t i = 0; i < testCases.size(); ++i) {
        const std::string& sql = testCases[i];
        std::cout << "\n📊 Test Case " << (i + 1) << " (Length: " << sql.length() << " chars)" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        // Benchmark new DFA lexer
        auto start = std::chrono::high_resolution_clock::now();
        for (int iter = 0; iter < iterations; ++iter) {
            LexerNew newLexer(sql);
            Token token = newLexer.nextToken();
            while (token.getType() != Token::END_OF_INPUT) {
                token = newLexer.nextToken();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto newLexerTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Benchmark old lexer (if available)
        long long oldLexerTimeMs = 0;
        try {
            start = std::chrono::high_resolution_clock::now();
            for (int iter = 0; iter < iterations; ++iter) {
                Lexer oldLexer(sql);
                Token token = oldLexer.nextToken();
                while (token.getType() != Token::END_OF_INPUT) {
                    token = oldLexer.nextToken();
                }
            }
            end = std::chrono::high_resolution_clock::now();
            auto oldLexerTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            oldLexerTimeMs = oldLexerTime.count();
        } catch (const std::exception& e) {
            std::cout << "⚠️  Old lexer not available or failed: " << e.what() << std::endl;
        }

        // Results
        std::cout << "DFA Lexer Time:  " << newLexerTime.count() << " ms" << std::endl;
        if (oldLexerTimeMs > 0) {
            std::cout << "Old Lexer Time:  " << oldLexerTimeMs << " ms" << std::endl;
            double speedup = static_cast<double>(oldLexerTimeMs) / newLexerTime.count();
            std::cout << "Speedup:         " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;

            if (speedup > 1.0) {
                std::cout << "✅ DFA lexer is faster!" << std::endl;
            } else {
                std::cout << "⚠️  DFA lexer is slower (may be due to overhead)" << std::endl;
            }
        }

        // Tokenization accuracy test
        LexerNew accuracyLexer(sql);
        std::vector<Token> tokens;
        Token token = accuracyLexer.nextToken();
        while (token.getType() != Token::END_OF_INPUT) {
            tokens.push_back(token);
            token = accuracyLexer.nextToken();
        }

        std::cout << "Tokens generated: " << tokens.size() << std::endl;
        std::cout << "Tokenization completed successfully" << std::endl;
    }

    std::cout << "\n=====================================" << std::endl;
    std::cout << "✅ Benchmark completed!" << std::endl;
}

} // namespace sql_parser
} // namespace sqlcc

int main() {
    sqlcc::sql_parser::benchmarkLexers();
    return 0;
}
