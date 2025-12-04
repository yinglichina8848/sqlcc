#include "sql_parser/lexer_new.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

namespace sqlcc {
namespace sql_parser {

// Test helper function
void assertToken(const Token& token, Token::Type expectedType, const std::string& expectedLexeme) {
    if (token.getType() != expectedType || token.getLexeme() != expectedLexeme) {
        std::cerr << "FAIL: Expected (" << Token::getTypeName(expectedType) << ", '" << expectedLexeme
                  << "'), got (" << Token::getTypeName(token.getType()) << ", '" << token.getLexeme() << "')" << std::endl;
        assert(false);
    }
}

// Test basic tokenization
void testBasicTokens() {
    std::cout << "Testing basic tokens..." << std::endl;

    LexerNew lexer("SELECT * FROM users WHERE id = 123;");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    // Should have 9 tokens: SELECT, *, FROM, users, WHERE, id, =, 123, ;
    assert(tokens.size() == 9);

    assertToken(tokens[0], Token::KEYWORD_SELECT, "select");
    assertToken(tokens[1], Token::OPERATOR_MULTIPLY, "*");
    assertToken(tokens[2], Token::KEYWORD_FROM, "from");
    assertToken(tokens[3], Token::IDENTIFIER, "users");
    assertToken(tokens[4], Token::KEYWORD_WHERE, "where");
    assertToken(tokens[5], Token::IDENTIFIER, "id");
    assertToken(tokens[6], Token::OPERATOR_EQUAL, "=");
    assertToken(tokens[7], Token::INTEGER_LITERAL, "123");
    assertToken(tokens[8], Token::SEMICOLON, ";");

    std::cout << "✓ Basic tokens test passed" << std::endl;
}

// Test keywords recognition
void testKeywords() {
    std::cout << "Testing keyword recognition..." << std::endl;

    LexerNew lexer("CREATE TABLE DROP INDEX SELECT INSERT UPDATE DELETE");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 7);
    assertToken(tokens[0], Token::KEYWORD_CREATE, "create");
    assertToken(tokens[1], Token::KEYWORD_TABLE, "table");
    assertToken(tokens[2], Token::KEYWORD_DROP, "drop");
    assertToken(tokens[3], Token::KEYWORD_INDEX, "index");
    assertToken(tokens[4], Token::KEYWORD_SELECT, "select");
    assertToken(tokens[5], Token::KEYWORD_INSERT, "insert");
    assertToken(tokens[6], Token::KEYWORD_UPDATE, "update");

    std::cout << "✓ Keywords test passed" << std::endl;
}

// Test identifiers
void testIdentifiers() {
    std::cout << "Testing identifiers..." << std::endl;

    LexerNew lexer("table_name _private column123 user_id");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 4);
    assertToken(tokens[0], Token::IDENTIFIER, "table_name");
    assertToken(tokens[1], Token::IDENTIFIER, "_private");
    assertToken(tokens[2], Token::IDENTIFIER, "column123");
    assertToken(tokens[3], Token::IDENTIFIER, "user_id");

    std::cout << "✓ Identifiers test passed" << std::endl;
}

// Test numbers
void testNumbers() {
    std::cout << "Testing numbers..." << std::endl;

    LexerNew lexer("123 3.14 2.5e10 0.5 100");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 5);
    assertToken(tokens[0], Token::INTEGER_LITERAL, "123");
    assertToken(tokens[1], Token::FLOAT_LITERAL, "3.14");
    assertToken(tokens[2], Token::FLOAT_LITERAL, "2.5e10");
    assertToken(tokens[3], Token::FLOAT_LITERAL, "0.5");
    assertToken(tokens[4], Token::INTEGER_LITERAL, "100");

    std::cout << "✓ Numbers test passed" << std::endl;
}

// Test strings
void testStrings() {
    std::cout << "Testing strings..." << std::endl;

    LexerNew lexer("'hello world' \"quoted identifier\" 'don\\'t worry'");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 3);
    assertToken(tokens[0], Token::STRING_LITERAL, "hello world");
    assertToken(tokens[1], Token::IDENTIFIER, "quoted identifier");
    assertToken(tokens[2], Token::STRING_LITERAL, "don\\'t worry");

    std::cout << "✓ Strings test passed" << std::endl;
}

// Test operators
void testOperators() {
    std::cout << "Testing operators..." << std::endl;

    LexerNew lexer("= != < <= > >= + - * /");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 10);
    assertToken(tokens[0], Token::OPERATOR_EQUAL, "=");
    assertToken(tokens[1], Token::OPERATOR_NOT_EQUAL, "!=");
    assertToken(tokens[2], Token::OPERATOR_LESS_THAN, "<");
    assertToken(tokens[3], Token::OPERATOR_LESS_EQUAL, "<=");
    assertToken(tokens[4], Token::OPERATOR_GREATER_THAN, ">");
    assertToken(tokens[5], Token::OPERATOR_GREATER_EQUAL, ">=");
    assertToken(tokens[6], Token::OPERATOR_PLUS, "+");
    assertToken(tokens[7], Token::OPERATOR_MINUS, "-");
    assertToken(tokens[8], Token::OPERATOR_MULTIPLY, "*");
    assertToken(tokens[9], Token::OPERATOR_DIVIDE, "/");

    std::cout << "✓ Operators test passed" << std::endl;
}

// Test punctuation
void testPunctuation() {
    std::cout << "Testing punctuation..." << std::endl;

    LexerNew lexer("( ) , ; .");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 5);
    assertToken(tokens[0], Token::LPAREN, "(");
    assertToken(tokens[1], Token::RPAREN, ")");
    assertToken(tokens[2], Token::COMMA, ",");
    assertToken(tokens[3], Token::SEMICOLON, ";");
    assertToken(tokens[4], Token::DOT, ".");

    std::cout << "✓ Punctuation test passed" << std::endl;
}

// Test comments
void testComments() {
    std::cout << "Testing comments..." << std::endl;

    LexerNew lexer("SELECT /* comment */ * FROM users -- another comment\nWHERE id = 1");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 8);
    assertToken(tokens[0], Token::KEYWORD_SELECT, "select");
    assertToken(tokens[1], Token::OPERATOR_MULTIPLY, "*");
    assertToken(tokens[2], Token::KEYWORD_FROM, "from");
    assertToken(tokens[3], Token::IDENTIFIER, "users");
    assertToken(tokens[4], Token::KEYWORD_WHERE, "where");
    assertToken(tokens[5], Token::IDENTIFIER, "id");
    assertToken(tokens[6], Token::OPERATOR_EQUAL, "=");
    assertToken(tokens[7], Token::INTEGER_LITERAL, "1");

    std::cout << "✓ Comments test passed" << std::endl;
}

// Test complex SQL statement
void testComplexSQL() {
    std::cout << "Testing complex SQL..." << std::endl;

    std::string sql = R"(
        CREATE TABLE users (
            id INTEGER PRIMARY KEY AUTO_INCREMENT,
            username VARCHAR(50) NOT NULL UNIQUE,
            email VARCHAR(100),
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

    LexerNew lexer(sql);
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    // Verify some key tokens
    assert(tokens.size() > 20); // Should have many tokens

    // Check first few tokens
    assertToken(tokens[0], Token::KEYWORD_CREATE, "create");
    assertToken(tokens[1], Token::KEYWORD_TABLE, "table");
    assertToken(tokens[2], Token::IDENTIFIER, "users");

    std::cout << "✓ Complex SQL test passed (" << tokens.size() << " tokens)" << std::endl;
}

// Test whitespace handling
void testWhitespace() {
    std::cout << "Testing whitespace handling..." << std::endl;

    LexerNew lexer("SELECT\n\t * \r\n FROM  \t  users   ;");
    std::vector<Token> tokens;

    Token token = lexer.nextToken();
    while (token.getType() != Token::END_OF_INPUT) {
        tokens.push_back(token);
        token = lexer.nextToken();
    }

    assert(tokens.size() == 6);
    assertToken(tokens[0], Token::KEYWORD_SELECT, "select");
    assertToken(tokens[1], Token::OPERATOR_MULTIPLY, "*");
    assertToken(tokens[2], Token::KEYWORD_FROM, "from");
    assertToken(tokens[3], Token::IDENTIFIER, "users");
    assertToken(tokens[4], Token::SEMICOLON, ";");

    std::cout << "✓ Whitespace handling test passed" << std::endl;
}

// Test position tracking
void testPositionTracking() {
    std::cout << "Testing position tracking..." << std::endl;

    LexerNew lexer("SELECT\n  *\nFROM users;");

    Token token1 = lexer.nextToken();
    assert(token1.getLine() == 1 && token1.getColumn() == 1);

    Token token2 = lexer.nextToken();
    assert(token2.getLine() == 2 && token2.getColumn() == 3);

    Token token3 = lexer.nextToken();
    assert(token3.getLine() == 3 && token3.getColumn() == 1);

    std::cout << "✓ Position tracking test passed" << std::endl;
}

} // namespace sql_parser
} // namespace sqlcc

int main() {
    std::cout << "🧪 Running LexerNew DFA Tests..." << std::endl;
    std::cout << "=================================" << std::endl;

    try {
        sqlcc::sql_parser::testBasicTokens();
        sqlcc::sql_parser::testKeywords();
        sqlcc::sql_parser::testIdentifiers();
        sqlcc::sql_parser::testNumbers();
        sqlcc::sql_parser::testStrings();
        sqlcc::sql_parser::testOperators();
        sqlcc::sql_parser::testPunctuation();
        sqlcc::sql_parser::testComments();
        sqlcc::sql_parser::testComplexSQL();
        sqlcc::sql_parser::testWhitespace();
        sqlcc::sql_parser::testPositionTracking();

        std::cout << "=================================" << std::endl;
        std::cout << "✅ All LexerNew DFA tests passed!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
