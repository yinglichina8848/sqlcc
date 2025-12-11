#include "sql_parser/lexer_new.h"
#include "sql_parser/parser_new.h"
#include <iostream>

int main() {
    std::cout << "Testing INSERT statement parsing:" << std::endl;
    
    // 测试一个简单的INSERT语句
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "SQL: " << sql << std::endl;
    
    // 创建解析器
    sqlcc::sql_parser::ParserNew parser(sql);
    
    // 尝试解析
    auto statements = parser.parse();
    
    std::cout << "Parsed " << statements.size() << " statements" << std::endl;
    
    return 0;
}