#include "sql_parser/token_new.h"
#include <iostream>
#include <unordered_map>

// 修复后的createKeywordToken函数
sqlcc::sql_parser::Token::Type fixCreateKeywordToken(const std::string &keyword) {
    static std::unordered_map<std::string, sqlcc::sql_parser::Token::Type> keywordMap;
    
    // Initialize on first use
    if (keywordMap.empty()) {
        std::cout << "[FIX] Initializing fixed keywordMap" << std::endl;
        
        // DDL Keywords
        keywordMap["create"] = sqlcc::sql_parser::Token::KEYWORD_CREATE;
        keywordMap["alter"] = sqlcc::sql_parser::Token::KEYWORD_ALTER;
        keywordMap["drop"] = sqlcc::sql_parser::Token::KEYWORD_DROP;
        keywordMap["table"] = sqlcc::sql_parser::Token::KEYWORD_TABLE;
        keywordMap["index"] = sqlcc::sql_parser::Token::KEYWORD_INDEX;
        keywordMap["database"] = sqlcc::sql_parser::Token::KEYWORD_DATABASE;
        
        // DML Keywords
        keywordMap["select"] = sqlcc::sql_parser::Token::KEYWORD_SELECT;
        keywordMap["insert"] = sqlcc::sql_parser::Token::KEYWORD_INSERT;  // 应该是78
        keywordMap["update"] = sqlcc::sql_parser::Token::KEYWORD_UPDATE;
        keywordMap["delete"] = sqlcc::sql_parser::Token::KEYWORD_DELETE;
        keywordMap["from"] = sqlcc::sql_parser::Token::KEYWORD_FROM;
        keywordMap["into"] = sqlcc::sql_parser::Token::KEYWORD_INTO;
        keywordMap["values"] = sqlcc::sql_parser::Token::KEYWORD_VALUES;
        keywordMap["set"] = sqlcc::sql_parser::Token::KEYWORD_SET;
        
        // Constraint Keywords
        keywordMap["primary"] = sqlcc::sql_parser::Token::KEYWORD_PRIMARY;
        keywordMap["key"] = sqlcc::sql_parser::Token::KEYWORD_KEY;
        keywordMap["foreign"] = sqlcc::sql_parser::Token::KEYWORD_FOREIGN;
        keywordMap["references"] = sqlcc::sql_parser::Token::KEYWORD_REFERENCES;
        
        // 打印所有映射
        std::cout << "[FIX] Fixed keyword mappings:" << std::endl;
        for (const auto& pair : keywordMap) {
            std::cout << "  '" << pair.first << "' -> " << static_cast<int>(pair.second) << std::endl;
        }
    }
    
    auto it = keywordMap.find(keyword);
    if (it != keywordMap.end()) {
        std::cout << "[FIX] Found keyword '" << keyword
                  << "', returning type: " << static_cast<int>(it->second)
                  << std::endl;
        return it->second;
    }
    
    std::cout << "[FIX] Keyword '" << keyword << "' not found" << std::endl;
    return sqlcc::sql_parser::Token::IDENTIFIER;
}

int main() {
    std::cout << "Testing fixed keyword mapping:" << std::endl;
    
    // 测试insert关键字
    auto type = fixCreateKeywordToken("insert");
    std::cout << "Result for 'insert': " << static_cast<int>(type) << std::endl;
    
    // 测试values关键字
    type = fixCreateKeywordToken("values");
    std::cout << "Result for 'values': " << static_cast<int>(type) << std::endl;
    
    // 测试into关键字
    type = fixCreateKeywordToken("into");
    std::cout << "Result for 'into': " << static_cast<int>(type) << std::endl;
    
    return 0;
}