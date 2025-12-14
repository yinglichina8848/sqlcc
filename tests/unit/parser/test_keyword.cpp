#include "sql_parser/lexer.h"
#include <iostream>

int main() {
    // 创建一个临时的LexerNew实例来访问createKeywordToken
    sqlcc::sql_parser::LexerNew lexer("dummy");
    
    // 直接调用createKeywordToken方法（通过反射或友元，不过这里先用其他方式测试）
    // 先测试关键字是否在supportedKeywords中
    std::string test_keyword = "insert";
    
    // 模拟createIdentifierToken中的逻辑
    std::string lower_lexeme = test_keyword;
    for (char &c : lower_lexeme) {
        c = std::tolower(c);
    }
    
    std::cout << "Testing keyword: " << test_keyword << std::endl;
    std::cout << "Lowercase: " << lower_lexeme << std::endl;
    
    // 检查supportedKeywords
    static std::unordered_set<std::string> supportedKeywords = {
        // DML Keywords
        "select", "insert", "update", "delete", "from", "into", "values", "set",
        // ... 其他关键字
    };
    
    if (supportedKeywords.find(lower_lexeme) != supportedKeywords.end()) {
        std::cout << "Found in supportedKeywords: YES" << std::endl;
    } else {
        std::cout << "Found in supportedKeywords: NO" << std::endl;
    }
    
    return 0;
}