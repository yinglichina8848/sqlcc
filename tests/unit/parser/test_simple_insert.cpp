#include "sql_parser/parser.h"
#include "sql_parser/parser_new.h"
#include <iostream>

int main() {
    try {
        std::string sql = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);";
        std::cout << "测试SQL: " << sql << std::endl;
        
        sqlcc::sql_parser::ParserNew parser(sql);
        auto stmts = parser.parse();
        
        std::cout << "解析结果: " << stmts.size() << " 个语句" << std::endl;
        
        if (!stmts.empty()) {
            std::cout << "解析成功!" << std::endl;
        } else {
            std::cout << "解析失败，语句数量为0" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}