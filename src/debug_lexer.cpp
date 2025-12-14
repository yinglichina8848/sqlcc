#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/parser_new.h"
#include "include/sql_parser/lexer.h"
#include <iostream>

int main() {
    std::cout << "Testing lexer with INSERT statement..." << std::endl;
    
    std::string sql = "INSERT INTO employees (id, name, age, salary) VALUES (1, 'Alice', 30, 50000.00);";
    
    try {
        std::cout << "Creating lexer..." << std::endl;
        sqlcc::sql_parser::LexerNew lexer(sql);
        
        std::cout << "Creating parser..." << std::endl;
        sqlcc::sql_parser::ParserNew parser(sql);
        
        std::cout << "Parsing..." << std::endl;
        auto statements = parser.parse();
        
        std::cout << "Parsed " << statements.size() << " statements" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}