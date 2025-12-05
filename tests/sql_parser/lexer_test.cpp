#include "lexer_new.h"
#include "token_new.h"
#include <gtest/gtest.h>

using namespace sqlcc::sql_parser;

class LexerTest : public ::testing::Test {
protected:
  void expectToken(const Token &token, Token::Type expectedType,
                   const std::string &expectedValue) {
    EXPECT_EQ(token.getType(), expectedType);
    EXPECT_EQ(token.getLexeme(), expectedValue);
  }
};

// 测试基本标记解析
TEST_F(LexerTest, BasicTokenParsing) {
  LexerNew lexer("SELECT * FROM table;");

  Token token = lexer.nextToken();
  expectToken(token, Token::KEYWORD_SELECT, "SELECT");

  token = lexer.nextToken();
  expectToken(token, Token::OPERATOR_MULTIPLY, "*");

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
  LexerNew lexer("CREATE DATABASE test_db;");

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
  LexerNew lexer("SELECT * FROM table WHERE id = 123;");

  // Skip SELECT * FROM table WHERE id =
  for (int i = 0; i < 7; i++) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();
  expectToken(token, Token::INTEGER_LITERAL, "123");
}

// 测试字符串解析
TEST_F(LexerTest, StringParsing) {
  LexerNew lexer("SELECT * FROM table WHERE name = 'test';");

  // Skip SELECT * FROM table WHERE name =
  for (int i = 0; i < 7; i++) {
    lexer.nextToken();
  }

  Token token = lexer.nextToken();
  expectToken(token, Token::STRING_LITERAL, "test");
}

// 测试空白字符处理
TEST_F(LexerTest, WhitespaceHandling) {
    LexerNew lexer("SELECT  \t\n  *  \t\n  FROM;");
    
    Token token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_SELECT, "SELECT");
    
    token = lexer.nextToken();
    expectToken(token, Token::OPERATOR_MULTIPLY, "*");
    
    token = lexer.nextToken();
    expectToken(token, Token::KEYWORD_FROM, "FROM");
    
    token = lexer.nextToken();
    expectToken(token, Token::SEMICOLON, ";");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
