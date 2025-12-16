#include <iostream>
#include <memory>
#include <string>

#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试DROP TABLE语句
        std::string sql1 = "DROP TABLE users";
        std::cout << "解析SQL: " << sql1 << std::endl;
        
        Parser parser1(sql1);
        auto statements1 = parser1.parse();
        
        if (!statements1.empty()) {
            auto drop_stmt = dynamic_cast<DropStatement*>(statements1[0].get());
            if (drop_stmt) {
                std::cout << "成功解析DROP TABLE语句" << std::endl;
                std::cout << "对象类型: " << (drop_stmt->getObjectType() == DropStatement::TABLE ? "TABLE" : "其他") << std::endl;
                std::cout << "对象名称: " << drop_stmt->getObjectName() << std::endl;
                std::cout << "IF EXISTS: " << (drop_stmt->isIfExists() ? "是" : "否") << std::endl;
            }
        }
        
        std::cout << "\n----------------------\n" << std::endl;
        
        // 测试DROP TABLE IF EXISTS语句
        std::string sql2 = "DROP TABLE IF EXISTS users";
        std::cout << "解析SQL: " << sql2 << std::endl;
        
        Parser parser2(sql2);
        auto statements2 = parser2.parse();
        
        if (!statements2.empty()) {
            auto drop_stmt = dynamic_cast<DropStatement*>(statements2[0].get());
            if (drop_stmt) {
                std::cout << "成功解析DROP TABLE IF EXISTS语句" << std::endl;
                std::cout << "对象类型: " << (drop_stmt->getObjectType() == DropStatement::TABLE ? "TABLE" : "其他") << std::endl;
                std::cout << "对象名称: " << drop_stmt->getObjectName() << std::endl;
                std::cout << "IF EXISTS: " << (drop_stmt->isIfExists() ? "是" : "否") << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}