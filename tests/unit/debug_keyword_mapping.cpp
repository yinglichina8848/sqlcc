#include "sql_parser/token.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include "sql_parser/token_new.h"

int main() {
    std::cout << "Checking keyword mapping manually..." << std::endl;
    
    // 手动创建keywordMap来检查映射
    std::unordered_map<std::string, sqlcc::sql_parser::Token::Type> keywordMap;
    
    // DDL Keywords
    keywordMap["create"] = sqlcc::sql_parser::Token::KEYWORD_CREATE;
    keywordMap["alter"] = sqlcc::sql_parser::Token::KEYWORD_ALTER;
    keywordMap["drop"] = sqlcc::sql_parser::Token::KEYWORD_DROP;
    keywordMap["table"] = sqlcc::sql_parser::Token::KEYWORD_TABLE;
    keywordMap["index"] = sqlcc::sql_parser::Token::KEYWORD_INDEX;
    keywordMap["database"] = sqlcc::sql_parser::Token::KEYWORD_DATABASE;

    // DML Keywords
    keywordMap["select"] = sqlcc::sql_parser::Token::KEYWORD_SELECT;
    keywordMap["insert"] = sqlcc::sql_parser::Token::KEYWORD_INSERT;  // 应该是52
    keywordMap["update"] = sqlcc::sql_parser::Token::KEYWORD_UPDATE;
    keywordMap["delete"] = sqlcc::sql_parser::Token::KEYWORD_DELETE;
    keywordMap["from"] = sqlcc::sql_parser::Token::KEYWORD_FROM;
    keywordMap["into"] = sqlcc::sql_parser::Token::KEYWORD_INTO;
    keywordMap["values"] = sqlcc::sql_parser::Token::KEYWORD_VALUES;  // 应该是57

    // Query Keywords
    keywordMap["where"] = sqlcc::sql_parser::Token::KEYWORD_WHERE;
    keywordMap["group"] = sqlcc::sql_parser::Token::KEYWORD_GROUP;
    keywordMap["by"] = sqlcc::sql_parser::Token::KEYWORD_BY;
    keywordMap["having"] = sqlcc::sql_parser::Token::KEYWORD_HAVING;
    keywordMap["order"] = sqlcc::sql_parser::Token::KEYWORD_ORDER;
    keywordMap["limit"] = sqlcc::sql_parser::Token::KEYWORD_LIMIT;
    keywordMap["offset"] = sqlcc::sql_parser::Token::KEYWORD_OFFSET;
    keywordMap["distinct"] = sqlcc::sql_parser::Token::KEYWORD_DISTINCT;

    // Join Keywords
    keywordMap["join"] = sqlcc::sql_parser::Token::KEYWORD_JOIN;
    keywordMap["on"] = sqlcc::sql_parser::Token::KEYWORD_ON;
    keywordMap["outer"] = sqlcc::sql_parser::Token::KEYWORD_OUTER;

    // Constraint Keywords
    keywordMap["primary"] = sqlcc::sql_parser::Token::KEYWORD_PRIMARY;
    keywordMap["key"] = sqlcc::sql_parser::Token::KEYWORD_KEY;
    keywordMap["foreign"] = sqlcc::sql_parser::Token::KEYWORD_FOREIGN;
    keywordMap["references"] = sqlcc::sql_parser::Token::KEYWORD_REFERENCES;
    keywordMap["unique"] = sqlcc::sql_parser::Token::KEYWORD_UNIQUE;
    keywordMap["not"] = sqlcc::sql_parser::Token::KEYWORD_NOT;
    keywordMap["null"] = sqlcc::sql_parser::Token::KEYWORD_NULL;
    keywordMap["default"] = sqlcc::sql_parser::Token::KEYWORD_DEFAULT;
    keywordMap["auto_increment"] = sqlcc::sql_parser::Token::KEYWORD_AUTO_INCREMENT;

    // Permission Keywords
    keywordMap["grant"] = sqlcc::sql_parser::Token::KEYWORD_GRANT;
    keywordMap["revoke"] = sqlcc::sql_parser::Token::KEYWORD_REVOKE;
    keywordMap["to"] = sqlcc::sql_parser::Token::KEYWORD_TO;
    keywordMap["user"] = sqlcc::sql_parser::Token::KEYWORD_USER;
    keywordMap["with"] = sqlcc::sql_parser::Token::KEYWORD_WITH;
    keywordMap["password"] = sqlcc::sql_parser::Token::KEYWORD_PASSWORD;
    keywordMap["identified"] = sqlcc::sql_parser::Token::KEYWORD_IDENTIFIED;
    keywordMap["show"] = sqlcc::sql_parser::Token::KEYWORD_SHOW;

    // Logical Operators
    keywordMap["and"] = sqlcc::sql_parser::Token::KEYWORD_AND;
    keywordMap["or"] = sqlcc::sql_parser::Token::KEYWORD_OR;
    keywordMap["in"] = sqlcc::sql_parser::Token::KEYWORD_IN;
    keywordMap["exists"] = sqlcc::sql_parser::Token::KEYWORD_EXISTS;

    // Aggregate Functions
    keywordMap["count"] = sqlcc::sql_parser::Token::KEYWORD_COUNT;
    keywordMap["sum"] = sqlcc::sql_parser::Token::KEYWORD_SUM;
    keywordMap["avg"] = sqlcc::sql_parser::Token::KEYWORD_AVG;
    keywordMap["min"] = sqlcc::sql_parser::Token::KEYWORD_MIN;
    keywordMap["max"] = sqlcc::sql_parser::Token::KEYWORD_MAX;
    
    // 检查特定关键字的映射
    auto insert_it = keywordMap.find("insert");
    auto values_it = keywordMap.find("values");
    
    if (insert_it != keywordMap.end()) {
        std::cout << "Manual map - insert token type: " << static_cast<int>(insert_it->second) << std::endl;
    } else {
        std::cout << "Manual map - insert not found" << std::endl;
    }
    
    if (values_it != keywordMap.end()) {
        std::cout << "Manual map - values token type: " << static_cast<int>(values_it->second) << std::endl;
    } else {
        std::cout << "Manual map - values not found" << std::endl;
    }
    
    // 检查实际的枚举值
    std::cout << "Actual enum values:" << std::endl;
    std::cout << "KEYWORD_INSERT: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "KEYWORD_VALUES: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    
    return 0;
}