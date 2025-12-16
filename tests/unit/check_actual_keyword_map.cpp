#include "sql_parser/lexer.h"
#include "sql_parser/lexer_new.h"
#include <iostream>
#include <unordered_map>

// 声明友元函数来访问私有成员
class TestableLexerNew : public sqlcc::sql_parser::LexerNew {
public:
    TestableLexerNew(const std::string& input) : sqlcc::sql_parser::LexerNew(input) {}
    
    // 访问keywordMap的函数
    void printKeywordMap() {
        // 通过创建一个token来触发keywordMap的初始化
        this->nextToken();
        
        // 注意：这里我们不能直接访问keywordMap，因为它是一个局部静态变量
        // 我们只能通过调试输出来观察它的行为
        std::cout << "Keyword map should be initialized now." << std::endl;
    }
};

int main() {
    std::cout << "=== Checking Actual Keyword Map ===" << std::endl;
    
    TestableLexerNew lexer("insert select create");
    lexer.printKeywordMap();
    
    // 测试各个关键字
    sqlcc::sql_parser::Token token1 = lexer.nextToken(); // insert
    std::cout << "First token - Type: " << static_cast<int>(token1.getType()) 
              << ", Lexeme: '" << token1.getLexeme() << "'" << std::endl;
              
    sqlcc::sql_parser::Token token2 = lexer.nextToken(); // select
    std::cout << "Second token - Type: " << static_cast<int>(token2.getType()) 
              << ", Lexeme: '" << token2.getLexeme() << "'" << std::endl;
              
    sqlcc::sql_parser::Token token3 = lexer.nextToken(); // create
    std::cout << "Third token - Type: " << static_cast<int>(token3.getType()) 
              << ", Lexeme: '" << token3.getLexeme() << "'" << std::endl;
    
    return 0;
}