#include "sql_parser/ast_node.h"
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试基本的SELECT语句
        std::string sql1 = "SELECT * FROM users;";
        
        std::cout << "Parsing SQL: " << sql1 << std::endl;
        
        Parser parser1(sql1);
        auto statements1 = parser1.parse();
        
        if (!statements1.empty()) {
            std::cout << "Parsed successfully!" << std::endl;
            std::cout << "Number of statements: " << statements1.size() << std::endl;
            
            // 检查语句类型
            auto stmt = statements1[0].get();
            if (stmt->getType() == Statement::SELECT) {
                std::cout << "Statement type: SELECT" << std::endl;
            } else {
                std::cout << "Statement type: " << stmt->getType() << std::endl;
            }
        } else {
            std::cout << "No statements parsed." << std::endl;
        }
        
        // 测试带WHERE条件的SELECT语句
        std::string sql2 = "SELECT id, name FROM users WHERE id = 1;";
        
        std::cout << "\nParsing SQL: " << sql2 << std::endl;
        
        Parser parser2(sql2);
        auto statements2 = parser2.parse();
        
        if (!statements2.empty()) {
            std::cout << "Parsed successfully!" << std::endl;
            std::cout << "Number of statements: " << statements2.size() << std::endl;
        } else {
            std::cout << "No statements parsed." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}