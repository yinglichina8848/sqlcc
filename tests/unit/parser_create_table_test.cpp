#include "sql_parser/ast_node.h"
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试CREATE TABLE语句
        std::string sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, age INT DEFAULT 18);";
        
        std::cout << "解析SQL: " << sql << std::endl;
        
        Parser parser(sql);
        auto statements = parser.parse();
        
        if (!statements.empty()) {
            std::cout << "解析成功！" << std::endl;
            std::cout << "解析了 " << statements.size() << " 条语句" << std::endl;
            
            // 检查第一条语句是否为CREATE语句
            auto& stmt = statements[0];
            if (stmt->getType() == Statement::CREATE) {
                std::cout << "第一条语句是CREATE语句" << std::endl;
                
                // 尝试转换为CreateStatement
                CreateStatement* createStmt = dynamic_cast<CreateStatement*>(stmt.get());
                if (createStmt) {
                    std::cout << "成功转换为CreateStatement" << std::endl;
                    std::cout << "对象类型: " << static_cast<int>(createStmt->getObjectType()) << std::endl;
                    std::cout << "对象名称: " << createStmt->getObjectName() << std::endl;
                    
                    // 输出列信息
                    const auto& columns = createStmt->getColumns();
                    std::cout << "列数量: " << columns.size() << std::endl;
                    for (size_t i = 0; i < columns.size(); ++i) {
                        const auto& col = columns[i];
                        std::cout << "  列 " << i << ": " << col.getName() << " " << col.getTypeString() << std::endl;
                        if (col.isPrimaryKey()) {
                            std::cout << "    主键约束" << std::endl;
                        }
                        if (!col.isNullable()) {
                            std::cout << "    NOT NULL约束" << std::endl;
                        }
                        if (!col.getDefaultValue().empty()) {
                            std::cout << "    默认值: " << col.getDefaultValue() << std::endl;
                        }
                    }
                }
            }
        } else {
            std::cout << "没有解析到任何语句" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "解析过程中出现错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}