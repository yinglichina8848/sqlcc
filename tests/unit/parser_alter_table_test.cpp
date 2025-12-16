#include <iostream>
#include <memory>
#include <string>

#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试ALTER TABLE ADD COLUMN语句
        std::string sql1 = "ALTER TABLE users ADD COLUMN age INT";
        std::cout << "解析SQL: " << sql1 << std::endl;
        
        Parser parser1(sql1);
        auto statements1 = parser1.parse();
        
        if (!statements1.empty()) {
            auto alter_stmt = dynamic_cast<AlterStatement*>(statements1[0].get());
            if (alter_stmt) {
                std::cout << "成功解析ALTER TABLE语句" << std::endl;
                std::cout << "目标: " << (alter_stmt->getTarget() == AlterStatement::TABLE ? "TABLE" : "DATABASE") << std::endl;
                std::cout << "操作: " << alter_stmt->getAction() << std::endl;
                std::cout << "表名: " << alter_stmt->getTableName() << std::endl;
                
                auto column_def = alter_stmt->getColumnDefinition();
                std::cout << "列名: " << column_def.getName() << std::endl;
                std::cout << "数据类型: " << column_def.getTypeString() << std::endl;
            }
        }
        
        std::cout << "\n----------------------\n" << std::endl;
        
        // 测试ALTER TABLE DROP COLUMN语句
        std::string sql2 = "ALTER TABLE users DROP COLUMN age";
        std::cout << "解析SQL: " << sql2 << std::endl;
        
        Parser parser2(sql2);
        auto statements2 = parser2.parse();
        
        if (!statements2.empty()) {
            auto alter_stmt = dynamic_cast<AlterStatement*>(statements2[0].get());
            if (alter_stmt) {
                std::cout << "成功解析ALTER TABLE语句" << std::endl;
                std::cout << "目标: " << (alter_stmt->getTarget() == AlterStatement::TABLE ? "TABLE" : "DATABASE") << std::endl;
                std::cout << "操作: " << alter_stmt->getAction() << std::endl;
                std::cout << "表名: " << alter_stmt->getTableName() << std::endl;
                std::cout << "列名: " << alter_stmt->getColumnName() << std::endl;
            }
        }
        
        std::cout << "\n----------------------\n" << std::endl;
        
        // 测试ALTER TABLE RENAME TO语句
        std::string sql3 = "ALTER TABLE users RENAME TO customers";
        std::cout << "解析SQL: " << sql3 << std::endl;
        
        Parser parser3(sql3);
        auto statements3 = parser3.parse();
        
        if (!statements3.empty()) {
            auto alter_stmt = dynamic_cast<AlterStatement*>(statements3[0].get());
            if (alter_stmt) {
                std::cout << "成功解析ALTER TABLE语句" << std::endl;
                std::cout << "目标: " << (alter_stmt->getTarget() == AlterStatement::TABLE ? "TABLE" : "DATABASE") << std::endl;
                std::cout << "操作: " << alter_stmt->getAction() << std::endl;
                std::cout << "原表名: " << alter_stmt->getTableName() << std::endl;
                std::cout << "新表名: " << alter_stmt->getNewTableName() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
