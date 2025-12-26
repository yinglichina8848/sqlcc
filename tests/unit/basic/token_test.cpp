#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
#include "../../../include/sql_parser/token.h"

using namespace sqlcc::sql_parser;

// Test basic Token functionality
void test_token_construction() {
    std::cout << "Testing Token construction..." << std::endl;

    // Test default constructor
    Token token1;
    assert(token1.getType() == Token::UNKNOWN);
    assert(token1.getLexeme().empty());
    assert(token1.getLine() == 0);
    assert(token1.getColumn() == 0);

    // Test parameterized constructor with various inputs
    Token token2(Token::IDENTIFIER, "my_table", 1, 5);
    assert(token2.getType() == Token::IDENTIFIER);
    assert(token2.getLexeme() == "my_table");
    assert(token2.getLine() == 1);
    assert(token2.getColumn() == 5);

    // Test copy construction
    Token token3(token2);
    assert(token3.getType() == Token::IDENTIFIER);
    assert(token3.getLexeme() == "my_table");
    assert(token3.getLine() == 1);
    assert(token3.getColumn() == 5);

    // Test assignment
    Token token4;
    token4 = token2;
    assert(token4.getType() == Token::IDENTIFIER);
    assert(token4.getLexeme() == "my_table");

    std::cout << "Token construction test passed!" << std::endl;
}

// Test getTypeName static method
void test_token_type_names() {
    std::cout << "Testing Token type names..." << std::endl;

    // Test punctuation tokens
    assert(Token::getTypeName(Token::SEMICOLON) == "SEMICOLON");
    assert(Token::getTypeName(Token::COLON) == "COLON");
    assert(Token::getTypeName(Token::LPAREN) == "LPAREN");
    assert(Token::getTypeName(Token::RPAREN) == "RPAREN");
    assert(Token::getTypeName(Token::COMMA) == "COMMA");
    assert(Token::getTypeName(Token::DOT) == "DOT");

    // Test literal tokens
    assert(Token::getTypeName(Token::INTEGER_LITERAL) == "INTEGER_LITERAL");
    assert(Token::getTypeName(Token::FLOAT_LITERAL) == "FLOAT_LITERAL");
    assert(Token::getTypeName(Token::STRING_LITERAL) == "STRING_LITERAL");
    assert(Token::getTypeName(Token::BOOLEAN_LITERAL) == "BOOLEAN_LITERAL");
    assert(Token::getTypeName(Token::NULL_LITERAL) == "NULL_LITERAL");

    // Test identifier token
    assert(Token::getTypeName(Token::IDENTIFIER) == "IDENTIFIER");

    // Test operator tokens
    assert(Token::getTypeName(Token::OPERATOR_PLUS) == "OPERATOR_PLUS");
    assert(Token::getTypeName(Token::OPERATOR_MINUS) == "OPERATOR_MINUS");
    assert(Token::getTypeName(Token::OPERATOR_MULTIPLY) == "OPERATOR_MULTIPLY");
    assert(Token::getTypeName(Token::OPERATOR_DIVIDE) == "OPERATOR_DIVIDE");
    assert(Token::getTypeName(Token::OPERATOR_EQUAL) == "OPERATOR_EQUAL");
    assert(Token::getTypeName(Token::OPERATOR_NOT_EQUAL) == "OPERATOR_NOT_EQUAL");

    // Test DDL keywords
    assert(Token::getTypeName(Token::KEYWORD_CREATE) == "KEYWORD_CREATE");
    assert(Token::getTypeName(Token::KEYWORD_ALTER) == "KEYWORD_ALTER");
    assert(Token::getTypeName(Token::KEYWORD_DROP) == "KEYWORD_DROP");
    assert(Token::getTypeName(Token::KEYWORD_SELECT) == "KEYWORD_SELECT");
    assert(Token::getTypeName(Token::KEYWORD_FROM) == "KEYWORD_FROM");
    assert(Token::getTypeName(Token::KEYWORD_WHERE) == "KEYWORD_WHERE");
    assert(Token::getTypeName(Token::KEYWORD_TABLE) == "KEYWORD_TABLE");

    // Test special tokens
    assert(Token::getTypeName(Token::END_OF_INPUT) == "END_OF_INPUT");
    assert(Token::getTypeName(Token::ERROR) == "ERROR");
    assert(Token::getTypeName(Token::UNKNOWN) == "UNKNOWN");

    // Test that all token types have valid names (no UNKNOWN_TYPE)
    for (int i = 0; i <= static_cast<int>(Token::UNKNOWN); ++i) {
        std::string name = Token::getTypeName(static_cast<Token::Type>(i));
        assert(!name.empty());
        assert(name != "UNKNOWN_TYPE");
    }

    std::cout << "Token type names test passed!" << std::endl;
}

// Test different token types comprehensively
void test_token_types() {
    std::cout << "Testing comprehensive Token types..." << std::endl;

    // Test punctuation tokens
    Token semicolon(Token::SEMICOLON, ";", 1, 10);
    assert(semicolon.getType() == Token::SEMICOLON);
    assert(semicolon.getLexeme() == ";");

    Token lparen(Token::LPAREN, "(", 1, 15);
    assert(lparen.getType() == Token::LPAREN);

    Token rparen(Token::RPAREN, ")", 1, 20);
    assert(rparen.getType() == Token::RPAREN);

    Token comma(Token::COMMA, ",", 1, 25);
    assert(comma.getType() == Token::COMMA);

    Token dot(Token::DOT, ".", 1, 30);
    assert(dot.getType() == Token::DOT);

    // Test literal tokens
    Token int_literal(Token::INTEGER_LITERAL, "42", 2, 5);
    assert(int_literal.getType() == Token::INTEGER_LITERAL);
    assert(int_literal.getLexeme() == "42");

    Token float_literal(Token::FLOAT_LITERAL, "3.14", 2, 10);
    assert(float_literal.getType() == Token::FLOAT_LITERAL);

    Token string_literal(Token::STRING_LITERAL, "'hello world'", 2, 15);
    assert(string_literal.getType() == Token::STRING_LITERAL);

    Token bool_literal(Token::BOOLEAN_LITERAL, "true", 2, 20);
    assert(bool_literal.getType() == Token::BOOLEAN_LITERAL);

    Token null_literal(Token::NULL_LITERAL, "NULL", 2, 25);
    assert(null_literal.getType() == Token::NULL_LITERAL);

    // Test identifier tokens
    Token identifier1(Token::IDENTIFIER, "user_name", 3, 8);
    assert(identifier1.getType() == Token::IDENTIFIER);

    Token identifier2(Token::IDENTIFIER, "_private_var", 3, 12);
    assert(identifier2.getType() == Token::IDENTIFIER);

    Token identifier3(Token::IDENTIFIER, "table123", 3, 16);
    assert(identifier3.getType() == Token::IDENTIFIER);

    // Test operator tokens
    Token plus_op(Token::OPERATOR_PLUS, "+", 4, 12);
    assert(plus_op.getType() == Token::OPERATOR_PLUS);

    Token minus_op(Token::OPERATOR_MINUS, "-", 4, 14);
    assert(minus_op.getType() == Token::OPERATOR_MINUS);

    Token multiply_op(Token::OPERATOR_MULTIPLY, "*", 4, 16);
    assert(multiply_op.getType() == Token::OPERATOR_MULTIPLY);

    Token divide_op(Token::OPERATOR_DIVIDE, "/", 4, 18);
    assert(divide_op.getType() == Token::OPERATOR_DIVIDE);

    Token equal_op(Token::OPERATOR_EQUAL, "=", 4, 20);
    assert(equal_op.getType() == Token::OPERATOR_EQUAL);

    Token not_equal_op(Token::OPERATOR_NOT_EQUAL, "!=", 4, 22);
    assert(not_equal_op.getType() == Token::OPERATOR_NOT_EQUAL);

    // Test comparison operators
    Token less_op(Token::OPERATOR_LESS_THAN, "<", 5, 10);
    assert(less_op.getType() == Token::OPERATOR_LESS_THAN);

    Token greater_op(Token::OPERATOR_GREATER_THAN, ">", 5, 12);
    assert(greater_op.getType() == Token::OPERATOR_GREATER_THAN);

    Token less_equal_op(Token::OPERATOR_LESS_EQUAL, "<=", 5, 14);
    assert(less_equal_op.getType() == Token::OPERATOR_LESS_EQUAL);

    Token greater_equal_op(Token::OPERATOR_GREATER_EQUAL, ">=", 5, 16);
    assert(greater_equal_op.getType() == Token::OPERATOR_GREATER_EQUAL);

    // Test logical operators
    Token and_op(Token::OPERATOR_AND, "AND", 6, 10);
    assert(and_op.getType() == Token::OPERATOR_AND);

    Token or_op(Token::OPERATOR_OR, "OR", 6, 12);
    assert(or_op.getType() == Token::OPERATOR_OR);

    Token not_op(Token::OPERATOR_NOT, "NOT", 6, 14);
    assert(not_op.getType() == Token::OPERATOR_NOT);

    std::cout << "Comprehensive Token types test passed!" << std::endl;
}

// Test Token edge cases and special scenarios
void test_token_edge_cases() {
    std::cout << "Testing Token edge cases..." << std::endl;

    // Test empty lexeme
    Token empty_token(Token::IDENTIFIER, "", 1, 1);
    assert(empty_token.getLexeme().empty());
    assert(empty_token.getType() == Token::IDENTIFIER);

    // Test very long lexeme
    std::string long_lexeme(1000, 'a');
    Token long_token(Token::IDENTIFIER, long_lexeme, 1, 1);
    assert(long_token.getLexeme() == long_lexeme);
    assert(long_token.getLexeme().length() == 1000);

    // Test special characters in lexeme
    Token special_token(Token::STRING_LITERAL, "'hello\nworld\t'", 1, 1);
    assert(special_token.getLexeme() == "'hello\nworld\t'");

    // Test maximum line and column values
    Token max_pos_token(Token::IDENTIFIER, "test", SIZE_MAX, SIZE_MAX);
    assert(max_pos_token.getLine() == SIZE_MAX);
    assert(max_pos_token.getColumn() == SIZE_MAX);

    // Test zero line and column (after default construction)
    Token zero_pos_token(Token::IDENTIFIER, "test", 0, 0);
    assert(zero_pos_token.getLine() == 0);
    assert(zero_pos_token.getColumn() == 0);

    std::cout << "Token edge cases test passed!" << std::endl;
}

// Test Token copy and move semantics
void test_token_copy_move() {
    std::cout << "Testing Token copy and move semantics..." << std::endl;

    // Test copy construction
    Token original(Token::IDENTIFIER, "original", 10, 20);
    Token copy_constructed(original);
    assert(copy_constructed.getType() == Token::IDENTIFIER);
    assert(copy_constructed.getLexeme() == "original");
    assert(copy_constructed.getLine() == 10);
    assert(copy_constructed.getColumn() == 20);

    // Test copy assignment
    Token copy_assigned;
    copy_assigned = original;
    assert(copy_assigned.getType() == Token::IDENTIFIER);
    assert(copy_assigned.getLexeme() == "original");
    assert(copy_assigned.getLine() == 10);
    assert(copy_assigned.getColumn() == 20);

    // Test that original and copy are independent
    Token modified_copy = original;
    modified_copy = Token(Token::STRING_LITERAL, "modified", 30, 40);
    assert(original.getType() == Token::IDENTIFIER);
    assert(original.getLexeme() == "original");
    assert(modified_copy.getType() == Token::STRING_LITERAL);
    assert(modified_copy.getLexeme() == "modified");

    std::cout << "Token copy and move semantics test passed!" << std::endl;
}

// Test Token keyword coverage
void test_token_keywords() {
    std::cout << "Testing Token keyword coverage..." << std::endl;

    // DDL keywords
    std::vector<std::pair<Token::Type, std::string>> ddl_keywords = {
        {Token::KEYWORD_CREATE, "CREATE"},
        {Token::KEYWORD_ALTER, "ALTER"},
        {Token::KEYWORD_DROP, "DROP"},
        {Token::KEYWORD_TRUNCATE, "TRUNCATE"},
        {Token::KEYWORD_RENAME, "RENAME"},
        {Token::KEYWORD_ADD, "ADD"},
        {Token::KEYWORD_MODIFY, "MODIFY"},
        {Token::KEYWORD_CONSTRAINT, "CONSTRAINT"}
    };

    for (const auto& [type, lexeme] : ddl_keywords) {
        Token token(type, lexeme, 1, 1);
        assert(token.getType() == type);
        assert(token.getLexeme() == lexeme);
        assert(Token::getTypeName(type) == "KEYWORD_" + lexeme);
    }

    // DML keywords
    std::vector<std::pair<Token::Type, std::string>> dml_keywords = {
        {Token::KEYWORD_SELECT, "SELECT"},
        {Token::KEYWORD_INSERT, "INSERT"},
        {Token::KEYWORD_UPDATE, "UPDATE"},
        {Token::KEYWORD_DELETE, "DELETE"},
        {Token::KEYWORD_FROM, "FROM"},
        {Token::KEYWORD_WHERE, "WHERE"},
        {Token::KEYWORD_GROUP, "GROUP"},
        {Token::KEYWORD_BY, "BY"},
        {Token::KEYWORD_HAVING, "HAVING"},
        {Token::KEYWORD_ORDER, "ORDER"},
        {Token::KEYWORD_LIMIT, "LIMIT"},
        {Token::KEYWORD_OFFSET, "OFFSET"}
    };

    for (const auto& [type, lexeme] : dml_keywords) {
        Token token(type, lexeme, 1, 1);
        assert(token.getType() == type);
        assert(token.getLexeme() == lexeme);
        assert(Token::getTypeName(type) == "KEYWORD_" + lexeme);
    }

    // Data type keywords
    std::vector<std::pair<Token::Type, std::string>> data_type_keywords = {
        {Token::KEYWORD_INT, "INT"},
        {Token::KEYWORD_INTEGER, "INTEGER"},
        {Token::KEYWORD_VARCHAR, "VARCHAR"},
        {Token::KEYWORD_CHAR, "CHAR"},
        {Token::KEYWORD_TEXT, "TEXT"},
        {Token::KEYWORD_BOOLEAN, "BOOLEAN"},
        {Token::KEYWORD_DATE, "DATE"},
        {Token::KEYWORD_TIMESTAMP, "TIMESTAMP"}
    };

    for (const auto& [type, lexeme] : data_type_keywords) {
        Token token(type, lexeme, 1, 1);
        assert(token.getType() == type);
        assert(token.getLexeme() == lexeme);
    }

    std::cout << "Token keyword coverage test passed!" << std::endl;
}

int main() {
    std::cout << "Running comprehensive Token tests..." << std::endl;

    try {
        test_token_construction();
        test_token_type_names();
        test_token_types();
        test_token_edge_cases();
        test_token_copy_move();
        test_token_keywords();

        std::cout << "All comprehensive Token tests passed successfully!" << std::endl;
        std::cout << "Token class coverage: High (constructors, getters, type names, edge cases, keywords)" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception!" << std::endl;
        return 1;
    }
}
