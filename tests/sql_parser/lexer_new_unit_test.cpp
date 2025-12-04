#include <gtest/gtest.h>
#include <sql_parser/lexer_new.h>
#include <sql_parser/token_new.h>
#include <sql_parser/parser_new.h>
#include <iostream>
#include <vector>

namespace sqlcc {
namespace sql_parser {
namespace test {

class LexerNewUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每测试前初始化
    }
    
    void TearDown() override {
        // 每测试后清理
    }
};

// 测试基本Token识别
TEST_F(LexerNewUnitTest, BasicTokenRecognition) {
    std::string input = "SELECT * FROM users WHERE id = 1;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查关键Token
    ASSERT_GE(tokens.size(), 6); // SELECT, *, FROM, users, WHERE, id, =, 1, ;
    EXPECT_EQ(tokens[0].type, Token::Type::SELECT);
    EXPECT_EQ(tokens[1].type, Token::Type::ASTERISK);
    EXPECT_EQ(tokens[2].type, Token::Type::FROM);
    EXPECT_EQ(tokens[3].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[4].type, Token::Type::WHERE);
    EXPECT_EQ(tokens[5].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[6].type, Token::Type::EQUALS);
    EXPECT_EQ(tokens[7].type, Token::Type::NUMBER);
    EXPECT_EQ(tokens[8].type, Token::Type::SEMICOLON);
}

// 测试数字字面量
TEST_F(LexerNewUnitTest, NumericLiterals) {
    std::string input = "123 456.78";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    ASSERT_EQ(tokens.size(), 3); // 123, 456.78, EOF
    EXPECT_EQ(tokens[0].type, Token::Type::NUMBER);
    EXPECT_EQ(tokens[1].type, Token::Type::NUMBER);
}

// 测试字符串字面量
TEST_F(LexerNewUnitTest, StringLiterals) {
    std::string input = "'hello' 'world'";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    ASSERT_EQ(tokens.size(), 3); // 'hello', 'world', EOF
    EXPECT_EQ(tokens[0].type, Token::Type::STRING);
    EXPECT_EQ(tokens[1].type, Token::Type::STRING);
}

// 测试关键字识别
TEST_F(LexerNewUnitTest, KeywordsRecognition) {
    std::string input = "CREATE TABLE users (id INT, name VARCHAR(50));";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查关键字
    EXPECT_EQ(tokens[0].type, Token::Type::CREATE);
    EXPECT_EQ(tokens[1].type, Token::Type::TABLE);
    EXPECT_EQ(tokens[5].type, Token::Type::INT);
    EXPECT_EQ(tokens[6].type, Token::Type::VARCHAR);
}

// 测试运算符识别
TEST_F(LexerNewUnitTest, OperatorsRecognition) {
    std::string input = "SELECT * FROM users WHERE id > 10 AND name LIKE 'John%';";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查运算符
    EXPECT_EQ(tokens[8].type, Token::Type::GREATER_THAN);
    EXPECT_EQ(tokens[10].type, Token::Type::AND);
    EXPECT_EQ(tokens[11].type, Token::Type::LIKE);
}

// 测试标点符号
TEST_F(LexerNewUnitTest, PunctuationRecognition) {
    std::string input = "SELECT id, name FROM users WHERE id = 1;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查标点符号
    EXPECT_EQ(tokens[2].type, Token::Type::COMMA);
    EXPECT_EQ(tokens[7].type, Token::Type::COMMA);
    EXPECT_EQ(tokens[9].type, Token::Type::EQUALS);
    EXPECT_EQ(tokens[11].type, Token::Type::SEMICOLON);
}

// 测试标识符识别
TEST_F(LexerNewUnitTest, IdentifierRecognition) {
    std::string input = "SELECT user_id, user_name, created_at FROM user_table;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查标识符
    EXPECT_EQ(tokens[1].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[2].type, Token::Type::COMMA);
    EXPECT_EQ(tokens[3].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[4].type, Token::Type::COMMA);
    EXPECT_EQ(tokens[5].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[6].type, Token::Type::FROM);
    EXPECT_EQ(tokens[7].type, Token::Type::IDENTIFIER);
}

// 测试特殊字符
TEST_F(LexerNewUnitTest, SpecialCharacters) {
    std::string input = "INSERT INTO table_name (col1, col2) VALUES (?, ?)";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 检查特殊字符
    EXPECT_EQ(tokens[0].type, Token::Type::INSERT);
    EXPECT_EQ(tokens[1].type, Token::Type::INTO);
    EXPECT_EQ(tokens[2].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[3].type, Token::Type::LEFT_PAREN);
    EXPECT_EQ(tokens[6].type, Token::Type::RIGHT_PAREN);
    EXPECT_EQ(tokens[7].type, Token::Type::VALUES);
    EXPECT_EQ(tokens[8].type, Token::Type::LEFT_PAREN);
    EXPECT_EQ(tokens[9].type, Token::Type::QUESTION_MARK);
    EXPECT_EQ(tokens[10].type, Token::Type::COMMA);
    EXPECT_EQ(tokens[11].type, Token::Type::QUESTION_MARK);
    EXPECT_EQ(tokens[12].type, Token::Type::RIGHT_PAREN);
}

// 测试注释处理
TEST_F(LexerNewUnitTest, CommentHandling) {
    std::string input = "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 应该正确处理注释而不影响其他Token
    ASSERT_GE(tokens.size(), 6);
    EXPECT_EQ(tokens[0].type, Token::Type::SELECT);
    EXPECT_EQ(tokens[1].type, Token::Type::ASTERISK);
    EXPECT_EQ(tokens[2].type, Token::Type::FROM);
    EXPECT_EQ(tokens[3].type, Token::Type::IDENTIFIER);
    EXPECT_EQ(tokens[4].type, Token::Type::WHERE);
}

// 测试错误处理
TEST_F(LexerNewUnitTest, ErrorHandling) {
    std::string input = "SELECT @invalid FROM users;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 应该跳过无效字符但继续解析
    ASSERT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens[0].type, Token::Type::SELECT);
    EXPECT_EQ(tokens[1].type, Token::Type::FROM);
    EXPECT_EQ(tokens[2].type, Token::Type::IDENTIFIER);
}

// 测试空白字符处理
TEST_F(LexerNewUnitTest, WhitespaceHandling) {
    std::string input = "SELECT    *    FROM     users    WHERE   id   =   1;";
    LexerNew lexer(input);
    
    std::vector<Token> tokens;
    Token token;
    while (!lexer.isAtEnd()) {
        token = lexer.nextToken();
        tokens.push_back(token);
        if (token.type == Token::Type::END_OF_FILE) break;
    }
    
    // 空白字符应该被忽略
    ASSERT_GE(tokens.size(), 7);
    EXPECT_EQ(tokens[0].type, Token::Type::SELECT);
    EXPECT_EQ(tokens[1].type, Token::Type::ASTERISK);
    EXPECT_EQ(tokens[2].type, Token::Type::FROM);
}

} // namespace test
} // namespace sql_parser
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
