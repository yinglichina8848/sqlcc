#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>

int main() {
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "Parsing SQL: " << sql << std::endl;
    
    sqlcc::sql_parser::ParserNew parser(sql);
    auto statements = parser.parse();
    
    std::cout << "Parsed " << statements.size() << " statements" << std::endl;
    
    // We can't directly access hadError() as it's private, but we can infer from statement count
    if (statements.size() == 0) {
        std::cout << "No statements parsed - likely parser errors" << std::endl;
    } else {
        std::cout << "Successfully parsed statements" << std::endl;
    }
    
    return 0;
}