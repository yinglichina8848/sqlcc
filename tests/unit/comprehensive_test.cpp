#include "sql_parser/token.h"
#include <iostream>
#include <vector>
#include "include/sql_parser/lexer.h"
#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/token_new.h"

using namespace sqlcc::sql_parser;

void testInput(const std::string& input, const std::string& description) {
    std::cout << "=== " << description << " ===" << std::endl;
    std::cout << "Input: \"" << input << "\"" << std::endl;
    
    try {
        LexerNew lexer(input);
        std::vector<Token> tokens;
        int tokenCount = 0;
        const int maxTokens = 50;
        
        while (tokenCount < maxTokens) {
            Token token = lexer.nextToken();
            tokens.push_back(token);
            
            std::cout << "  Token " << tokenCount << ": \"" << token.getLexeme() 
                      << "\" (" << Token::getTypeName(token.getType()) << ")" << std::endl;
            
            tokenCount++;
            
            if (token.getType() == Token::END_OF_INPUT) {
                break;
            }
        }
        
        if (tokenCount >= maxTokens) {
            std::cout << "  WARNING: Potential infinite loop detected!" << std::endl;
        }
        
        std::cout << "  Total tokens: " << tokens.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "  ERROR: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    std::cout << "Comprehensive Lexer Test" << std::endl;
    std::cout << "======================" << std::endl << std::endl;
    
    // 测试基本功能
    testInput("SELECT * FROM users;", "Basic SELECT statement");
    
    // 测试百分号操作符
    testInput("%", "Percent operator alone");
    testInput("123%", "Number with percent");
    testInput("SELECT id % 2 FROM users;", "Percent in SELECT statement");
    testInput("INSERT INTO test (val) VALUES (50%);", "Percent in INSERT statement");
    
    // 测试边界情况
    testInput("", "Empty string");
    testInput("   ", "Whitespace only");
    testInput(";;;", "Multiple semicolons");
    testInput("...", "Multiple dots");
    
    // 测试标识符和关键字
    testInput("abc DEF _test _123", "Identifiers");
    testInput("SELECT INSERT UPDATE DELETE", "Keywords");
    
    // 测试数字
    testInput("123 456.78 1.23e10", "Numbers");
    
    // 测试字符串
    testInput("'hello' \"world\"", "Strings");
    
    // 测试运算符
    testInput("+ - * / = != < > <= >=", "Operators");
    
    // 测试标点符号
    testInput("() , ; . :", "Punctuation");
    
    std::cout << "Test completed." << std::endl;
    
    return 0;
}