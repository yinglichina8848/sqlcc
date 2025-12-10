#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>

int main() {
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "Parsing SQL: " << sql << std::endl;
    
    sqlcc::sql_parser::ParserNew parser(sql);
    
    // 直接调用parseInsertStatement而不是整个parse方法
    // 这需要修改ParserNew类，让我们先测试完整的parse方法
    auto statements = parser.parse();
    
    std::cout << "Parsed " << statements.size() << " statements" << std::endl;
    
    for (size_t i = 0; i < statements.size(); ++i) {
        std::cout << "Statement " << i << ": " << typeid(*statements[i]).name() << std::endl;
    }
    
    return 0;
}