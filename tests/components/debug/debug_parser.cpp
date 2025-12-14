#include "include/sql_parser/parser_new.h"
#include "include/sql_parser/lexer.h"
#include <iostream>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing parser with INSERT statement..." << std::endl;
    
    try {
        // 测试包含单引号的字符串，模拟测试中的INSERT语句
        std::string test_input = "INSERT INTO employees (id, name, age, salary) VALUES (1, 'Alice', 30, 50000.00);";
        std::cout << "Input: " << test_input << std::endl;
        
        ParserNew parser(test_input);
        
        std::cout << "Parsing..." << std::endl;
        auto statements = parser.parse();
        
        std::cout << "Parsed " << statements.size() << " statements" << std::endl;
        
        if (!statements.empty()) {
            std::cout << "First statement parsed successfully!" << std::endl;
        }
        
        std::cout << "Parser test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}