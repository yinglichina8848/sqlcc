#include "include/sql_parser/parser_new.h"
#include "../../../include/sql_parser/ast_nodes.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing ALTER TABLE parsing..." << std::endl;
    
    // 测试 ALTER TABLE ADD COLUMN
    std::string sql1 = "ALTER TABLE users ADD COLUMN age INT;";
    std::cout << "Parsing: " << sql1 << std::endl;
    
    try {
        ParserNew parser1(sql1);
        auto statements1 = parser1.parse();
        
        if (!statements1.empty()) {
            std::cout << "Parsed successfully!" << std::endl;
            // 检查是否为AlterStatement
            auto alterStmt = dynamic_cast<AlterStatement*>(statements1[0].get());
            if (alterStmt) {
                std::cout << "Statement type: ALTER" << std::endl;
                std::cout << "Table name: " << alterStmt->getTableName() << std::endl;
                std::cout << "Action: ADD COLUMN" << std::endl;
            }
        } else {
            std::cout << "Failed to parse." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    // 测试 ALTER TABLE DROP COLUMN
    std::string sql2 = "ALTER TABLE users DROP COLUMN age;";
    std::cout << "\nParsing: " << sql2 << std::endl;
    
    try {
        ParserNew parser2(sql2);
        auto statements2 = parser2.parse();
        
        if (!statements2.empty()) {
            std::cout << "Parsed successfully!" << std::endl;
            // 检查是否为AlterStatement
            auto alterStmt = dynamic_cast<AlterStatement*>(statements2[0].get());
            if (alterStmt) {
                std::cout << "Statement type: ALTER" << std::endl;
                std::cout << "Table name: " << alterStmt->getTableName() << std::endl;
                std::cout << "Action: DROP COLUMN" << std::endl;
                std::cout << "Column name: " << alterStmt->getColumnName() << std::endl;
            }
        } else {
            std::cout << "Failed to parse." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}