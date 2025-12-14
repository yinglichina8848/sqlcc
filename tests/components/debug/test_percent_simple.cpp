#include <iostream>
#include "include/sql_parser/lexer.h"
#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/token_new.h"

using namespace sqlcc::sql_parser;

int main() {
    // 测试百分号字符
    std::string input = "%";
    std::cout << "测试输入: '" << input << "'" << std::endl;
    
    try {
        LexerNew lexer(input);
        Token token = lexer.nextToken();
        
        std::cout << "Token类型: " << token.getType() << std::endl;
        std::cout << "Token词素: '" << token.getLexeme() << "'" << std::endl;
        
        if (token.getType() == Token::END_OF_INPUT) {
            std::cout << "已到达输入末尾" << std::endl;
        } else {
            // 获取下一个token
            Token nextToken = lexer.nextToken();
            std::cout << "下一个Token类型: " << nextToken.getType() << std::endl;
            if (nextToken.getType() == Token::END_OF_INPUT) {
                std::cout << "已到达输入末尾" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}