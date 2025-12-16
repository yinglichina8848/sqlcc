#include <iostream>
#include <memory>
#include <string>

#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试ALTER TABLE MODIFY COLUMN语句
        std::string sql = "ALTER TABLE products MODIFY COLUMN description VARCHAR(255) NOT NULL DEFAULT 'N/A'";
        std::cout << "解析SQL: " << sql << std::endl;
        
        Parser parser(sql);
        auto statements = parser.parse();
        
        if (!statements.empty()) {
            auto alter_stmt = dynamic_cast<AlterStatement*>(statements[0].get());
            if (alter_stmt) {
                std::cout << "成功解析ALTER TABLE语句" << std::endl;
                std::cout << "目标: " << (alter_stmt->getTarget() == AlterStatement::TABLE ? "TABLE" : "DATABASE") << std::endl;
                std::cout << "操作: " << alter_stmt->getAction() << std::endl;
                std::cout << "表名: " << alter_stmt->getTableName() << std::endl;
                
                auto column_def = alter_stmt->getColumnDefinition();
                std::cout << "列名: " << column_def.getName() << std::endl;
                std::cout << "数据类型: " << column_def.getTypeString() << std::endl;
                std::cout << "是否可为空: " << (column_def.isNullable() ? "是" : "否") << std::endl;
                std::cout << "默认值: " << column_def.getDefaultValue() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "解析错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
