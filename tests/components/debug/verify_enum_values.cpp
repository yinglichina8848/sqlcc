#include "sql_parser/token.h"
#include <iostream>
#include <unordered_map>

int main() {
    std::cout << "=== Verifying Enum Values ===" << std::endl;
    
    // 手动创建映射表进行测试
    std::unordered_map<std::string, sqlcc::sql_parser::Token::Type> testMap;
    testMap["insert"] = sqlcc::sql_parser::Token::KEYWORD_INSERT;
    testMap["select"] = sqlcc::sql_parser::Token::KEYWORD_SELECT;
    testMap["create"] = sqlcc::sql_parser::Token::KEYWORD_CREATE;
    
    std::cout << "Manual mapping:" << std::endl;
    std::cout << "  'insert' -> " << static_cast<int>(testMap["insert"]) << std::endl;
    std::cout << "  'select' -> " << static_cast<int>(testMap["select"]) << std::endl;
    std::cout << "  'create' -> " << static_cast<int>(testMap["create"]) << std::endl;
    
    // 直接输出枚举值
    std::cout << "Direct enum values:" << std::endl;
    std::cout << "  KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "  KEYWORD_SELECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    std::cout << "  KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    
    return 0;
}