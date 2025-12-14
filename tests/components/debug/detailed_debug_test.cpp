#include <iostream>
#include <memory>
#include "sql_parser/lexer.h"
#include "sql_parser/lexer_new.h"
#include "sql_parser/token_new.h"

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Detailed debugging of lexer...\n";
    
    try {
        // 测试词法分析器
        std::string sql = "ALTER TABLE users ADD COLUMN age INT";
        std::cout << "Input SQL: " << sql << std::endl;
        
        LexerNew lexer(sql);
        std::cout << "Lexer created successfully" << std::endl;
        
        // 逐个获取token
        Token token = lexer.nextToken();
        int token_count = 0;
        while (token.getType() != Token::END_OF_INPUT && token_count < 15) {
            std::cout << "Token " << token_count << ": '" << token.getLexeme() 
                      << "' (type: " << token.getType() 
                      << ", line: " << token.getLine() 
                      << ", col: " << token.getColumn() << ")" << std::endl;
            
            // 特别检查ALTER关键字
            if (token.getType() == Token::KEYWORD_ALTER) {
                std::cout << "  Found ALTER keyword with lexeme: '" << token.getLexeme() << "'" << std::endl;
            }
            
            token = lexer.nextToken();
            token_count++;
        }
        std::cout << "Finished tokenizing" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Detailed debug test completed." << std::endl;
    return 0;
}