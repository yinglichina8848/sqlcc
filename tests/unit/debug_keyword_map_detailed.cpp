#include "sql_parser/token.h"
#include "sql_parser/lexer.h"
#include <iostream>
#include <unordered_map>

// 重新实现createKeywordToken函数以调试
sqlcc::sql_parser::Token::Type debugCreateKeywordToken(const std::string &keyword) {
    static std::unordered_map<std::string, sqlcc::sql_parser::Token::Type> keywordMap;
    
    std::cout << "[DEBUG] debugCreateKeywordToken called with: '" << keyword << "'" << std::endl;
    
    // Initialize on first use
    if (keywordMap.empty()) {
        std::cout << "[DEBUG] Initializing debug keywordMap" << std::endl;
        
        // DDL Keywords
        keywordMap["create"] = sqlcc::sql_parser::Token::KEYWORD_CREATE;
        keywordMap["alter"] = sqlcc::sql_parser::Token::KEYWORD_ALTER;
        keywordMap["drop"] = sqlcc::sql_parser::Token::KEYWORD_DROP;
        keywordMap["table"] = sqlcc::sql_parser::Token::KEYWORD_TABLE;
        keywordMap["index"] = sqlcc::sql_parser::Token::KEYWORD_INDEX;
        keywordMap["database"] = sqlcc::sql_parser::Token::KEYWORD_DATABASE;
        
        // DML Keywords
        keywordMap["select"] = sqlcc::sql_parser::Token::KEYWORD_SELECT;
        keywordMap["insert"] = sqlcc::sql_parser::Token::KEYWORD_INSERT;
        keywordMap["update"] = sqlcc::sql_parser::Token::KEYWORD_UPDATE;
        keywordMap["delete"] = sqlcc::sql_parser::Token::KEYWORD_DELETE;
        keywordMap["from"] = sqlcc::sql_parser::Token::KEYWORD_FROM;
        keywordMap["into"] = sqlcc::sql_parser::Token::KEYWORD_INTO;
        keywordMap["values"] = sqlcc::sql_parser::Token::KEYWORD_VALUES;
        keywordMap["set"] = sqlcc::sql_parser::Token::KEYWORD_SET;
        
        // 打印所有关键字映射
        std::cout << "[DEBUG] Keyword mappings:" << std::endl;
        for (const auto& pair : keywordMap) {
            std::cout << "  '" << pair.first << "' -> " << static_cast<int>(pair.second) << std::endl;
        }
    }
    
    auto it = keywordMap.find(keyword);
    if (it != keywordMap.end()) {
        std::cout << "[DEBUG] Found keyword '" << keyword
                  << "', returning type: " << static_cast<int>(it->second)
                  << std::endl;
        return it->second;
    }
    
    std::cout << "[DEBUG] Keyword '" << keyword << "' not found in map" << std::endl;
    return sqlcc::sql_parser::Token::IDENTIFIER;
}

int main() {
    std::cout << "Testing keyword mapping for 'insert':" << std::endl;
    auto type = debugCreateKeywordToken("insert");
    std::cout << "Result type for 'insert': " << static_cast<int>(type) << std::endl;
    
    std::cout << "\nTesting keyword mapping for 'values':" << std::endl;
    type = debugCreateKeywordToken("values");
    std::cout << "Result type for 'values': " << static_cast<int>(type) << std::endl;
    
    return 0;
}