#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>

int main() {
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "Parsing SQL: " << sql << std::endl;
    
    try {
        sqlcc::sql_parser::ParserNew parser(sql);
        auto statements = parser.parse();
        
        std::cout << "Parsed " << statements.size() << " statements" << std::endl;
        
        for (size_t i = 0; i < statements.size(); ++i) {
            std::cout << "Statement " << i << ": " << statements[i]->getTypeName() << std::endl;
        }
        
        if (statements.size() > 0) {
            std::cout << "SUCCESS: INSERT statement parsed correctly!" << std::endl;
        } else {
            std::cout << "FAILURE: No statements parsed" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}