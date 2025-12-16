#include <iostream>
#include <memory>
#include "sql_parser/parser_new.h"
#include "sql_parser/lexer.h"
#include "../../../include/sql_parser/ast_nodes.h"

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Debugging ALTER TABLE parsing...\n";
    
    try {
        // 先测试词法分析器
        std::string sql = "ALTER TABLE users ADD COLUMN age INT";
        std::cout << "Input SQL: " << sql << std::endl;
        
        Lexer lexer(sql);
        std::cout << "Lexer created successfully" << std::endl;
        
        // 逐个获取token
        Token token = lexer.nextToken();
        int token_count = 0;
        while (token.getType() != Token::END_OF_INPUT && token_count < 10) {
            std::cout << "Token " << token_count << ": " << token.getLexeme() 
                      << " (type: " << token.getType() << ")" << std::endl;
            token = lexer.nextToken();
            token_count++;
        }
        std::cout << "Finished tokenizing" << std::endl;
        
        // 现在测试解析器
        std::cout << "\nTesting parser..." << std::endl;
        ParserNew parser(sql);
        std::cout << "Parser created successfully" << std::endl;
        
        auto statements = parser.parse();
        std::cout << "Parsed " << statements.size() << " statements" << std::endl;
        
        if (!statements.empty()) {
            std::cout << "First statement type: " << statements[0]->getType() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Debug test completed." << std::endl;
    return 0;
}