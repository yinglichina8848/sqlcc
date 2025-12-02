#include <gtest/gtest.h>
#include "sql_parser/lexer.h"
#include "sql_parser/token.h"

using namespace sqlcc::sql_parser;

class LexerTest : public ::testing::Test {
protected:
    void expectToken(const Token& token, Token::Type expectedType, const std::string& expectedValue) {
        EXPECT_EQ(token.getType(), expectedType);
        EXPECT_EQ(token.getLexeme(), expectedValue);
    }
};

// 测试基本标记解析
TEST_F(LexerTest, BasicTokenParsing) {
    Lexer lexer("SELECT * FROM table;");
    
    Token token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_SELECT, "SELECT");
    
    token = lexer.nextToken();
    expectToken(token, Token::MULTIPLY, "*");
    
    token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_FROM, "FROM");
    
    token = lexer.nextToken();
    expectToken(token, Token::IDENTIFIER, "table");
    
    token = lexer.nextToken();
    expectToken(token, Token::SEMICOLON, ";");
    
    token = lexer.nextToken();
    expectToken(token, Token::END_OF_INPUT, "");
}

// 测试标识符和关键字解析
TEST_F(LexerTest, IdentifierAndKeywordParsing) {
    Lexer lexer("CREATE DATABASE test_db;");
    
    Token token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_CREATE, "CREATE");
    
    token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_DATABASE, "DATABASE");
    
    token = lexer.nextToken();
    expectToken(token, Token::IDENTIFIER, "test_db");
    
    token = lexer.nextToken();
    expectToken(token, Token::SEMICOLON, ";");
}

// 测试数字解析
TEST_F(LexerTest, NumberParsing) {
    Lexer lexer("SELECT * FROM table WHERE id = 123;");
    
    // Skip SELECT * FROM table WHERE id = 
    for (int i = 0; i < 7; i++) {
        lexer.nextToken();
    }
    
    Token token = lexer.nextToken();
    expectToken(token, Token::NUMBER, "123");
}

// 测试字符串解析
TEST_F(LexerTest, StringParsing) {
    Lexer lexer("SELECT * FROM table WHERE name = 'test';");
    
    // Skip SELECT * FROM table WHERE name = 
    for (int i = 0; i < 7; i++) {
        lexer.nextToken();
    }
    
    Token token = lexer.nextToken();
    expectToken(token, Token::STRING, "test");
}

// 测试空白字符处理
TEST_F(LexerTest, WhitespaceHandling) {
    Lexer lexer("SELECT  \t\n  *  \t\n  FROM;");
    
    Token token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_SELECT, "SELECT");
    
    token = lexer.nextToken();
    expectToken(token, Token::MULTIPLY, "*");
    
    token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_FROM, "FROM");
    
    token = lexer.nextToken();
    expectToken(token, Token::SEMICOLON, ";");
}




int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
