#include "sql_parser/token.h"
#include <iostream>
#include <unordered_map>

int main() {
    std::cout << "Checking Token enum values:" << std::endl;
    std::cout << "Token::KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "Token::KEYWORD_PRIMARY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRIMARY) << std::endl;
    std::cout << "Token::KEYWORD_SELECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    std::cout << "Token::KEYWORD_UPDATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UPDATE) << std::endl;
    std::cout << "Token::KEYWORD_DELETE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DELETE) << std::endl;
    std::cout << "Token::KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    std::cout << "Token::KEYWORD_INTO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTO) << std::endl;
    std::cout << "Token::KEYWORD_VALUES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    
    // 手动创建一个keywordMap来测试
    std::unordered_map<std::string, sqlcc::sql_parser::Token::Type> testMap;
    testMap["insert"] = sqlcc::sql_parser::Token::KEYWORD_INSERT;
    testMap["primary"] = sqlcc::sql_parser::Token::KEYWORD_PRIMARY;
    testMap["select"] = sqlcc::sql_parser::Token::KEYWORD_SELECT;
    
    std::cout << "\nTesting manual keywordMap:" << std::endl;
    std::cout << "testMap['insert'] = " << static_cast<int>(testMap["insert"]) << std::endl;
    std::cout << "testMap['primary'] = " << static_cast<int>(testMap["primary"]) << std::endl;
    std::cout << "testMap['select'] = " << static_cast<int>(testMap["select"]) << std::endl;
    
    return 0;
}