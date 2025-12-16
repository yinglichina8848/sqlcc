#include <iostream>
#include <memory>
#include "sql_parser/parser_new.h"
#include "sql_parser/lexer.h"
#include "../../../include/sql_parser/ast_nodes.h"

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing ALTER TABLE compilation...\n";
    
    // 测试ALTER TABLE语句解析
    std::string sql = "ALTER TABLE users ADD COLUMN age INT";
    std::cout << "Parsing SQL: " << sql << std::endl;
    
    try {
        ParserNew parser(sql);
        
        auto statements = parser.parse();
        if (!statements.empty()) {
            std::cout << "Parse successful!" << std::endl;
            
            auto& stmt = statements[0];
            // 检查是否为ALTER语句
            if (stmt->getType() == Statement::ALTER) {
                std::cout << "Statement type is ALTER" << std::endl;
                
                auto alter_stmt = dynamic_cast<AlterStatement*>(stmt.get());
                if (alter_stmt) {
                    std::cout << "Successfully cast to AlterStatement" << std::endl;
                    std::cout << "Target: " << (alter_stmt->getTarget() == AlterStatement::TABLE ? "TABLE" : "DATABASE") << std::endl;
                    std::cout << "Action: " << alter_stmt->getAction() << std::endl;
                    std::cout << "Table name: " << alter_stmt->getTableName() << std::endl;
                    
                    if (alter_stmt->getAction() == AlterStatement::ADD_COLUMN) {
                        auto column_def = alter_stmt->getColumnDefinition();
                        std::cout << "Column name: " << column_def.getName() << std::endl;
                        std::cout << "Column type: " << column_def.getType() << std::endl;
                    }
                } else {
                    std::cout << "Failed to cast to AlterStatement" << std::endl;
                }
            } else {
                std::cout << "Statement is not ALTER type" << std::endl;
            }
        } else {
            std::cout << "Parse failed!" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Test completed." << std::endl;
    return 0;
}