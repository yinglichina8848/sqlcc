#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== Testing INSERT statement ===" << std::endl;
    std::string insertSQL = "INSERT INTO test_table (id, name) VALUES (1, 'hello');";
    std::cout << "SQL: " << insertSQL << std::endl;
    
    try {
        auto parser = std::make_unique<ParserNew>(insertSQL);
        auto result = parser->parse();
        std::cout << "INSERT parse result: SUCCESS, statement count: " << result.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "INSERT parse error: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Testing UPDATE statement ===" << std::endl;
    std::string updateSQL = "UPDATE test_table SET name = 'world' WHERE id = 1;";
    std::cout << "SQL: " << updateSQL << std::endl;
    
    try {
        auto parser = std::make_unique<ParserNew>(updateSQL);
        auto result = parser->parse();
        std::cout << "UPDATE parse result: SUCCESS, statement count: " << result.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "UPDATE parse error: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Testing DELETE statement ===" << std::endl;
    std::string deleteSQL = "DELETE FROM test_table WHERE id = 1;";
    std::cout << "SQL: " << deleteSQL << std::endl;
    
    try {
        auto parser = std::make_unique<ParserNew>(deleteSQL);
        auto result = parser->parse();
        std::cout << "DELETE parse result: SUCCESS, statement count: " << result.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "DELETE parse error: " << e.what() << std::endl;
    }
    
    return 0;
}