#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>

int main() {
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "Parsing SQL: " << sql << std::endl;
    
    sqlcc::sql_parser::ParserNew parser(sql);
    auto statements = parser.parse();
    
    std::cout << "Parsed " << statements.size() << " statements" << std::endl;
    
    // Note: We can't access hadError() directly as it's private
    // In a real application, we would need to implement a public method to check for errors
    
    return 0;
}