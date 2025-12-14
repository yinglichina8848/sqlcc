#include "sql_parser/lexer.h"
#include <iostream>

int main() {
    std::string sql = "INSERT INTO test_table (id, name) VALUES (1, 'Alice');";
    std::cout << "Testing SQL: " << sql << std::endl;
    std::cout << std::endl;
    
    sqlcc::sql_parser::LexerNew lexer(sql);
    sqlcc::sql_parser::Token token;
    int i = 0;
    while (true) {
        token = lexer.nextToken();
        std::cout << "Token " << i++ << ": '" << token.getLexeme() 
                  << "' Type: " << static_cast<int>(token.getType()) 
                  << std::endl;
        if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
            break;
        }
    }
    
    return 0;
}