#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>

using namespace sqlcc::sql_parser;

// 重现getSQLKeywords函数
const std::unordered_set<std::string> &getSQLKeywords() {
  static std::unordered_set<std::string> keywords = {
      // DDL Keywords
      "create", "alter", "drop", "truncate", "rename", "comment", "add", "column", "modify", "constraint",

      // DML Keywords
      "select", "insert", "update", "delete", "merge",

      // DCL Keywords
      "grant", "revoke", "deny",

      // TCL Keywords
      "begin", "commit", "rollback", "savepoint", "set", "transaction",

      // Data Types
      "int", "integer", "smallint", "bigint", "tinyint", "varchar", "char",
      "text", "blob", "clob", "decimal", "numeric", "float", "double", "real",
      "date", "time", "timestamp", "datetime", "year", "boolean", "bool",

      // Constraints
      "primary", "key", "foreign", "references", "unique", "check", "not",
      "null", "default", "auto_increment",

      // Query Keywords
      "from", "where", "group", "by", "having", "order", "limit", "offset",
      "distinct", "all", "as", "join", "inner", "left", "right", "full",
      "outer", "on", "using",

      // Aggregate Functions
      "count", "sum", "avg", "min", "max", "group_concat",

      // Logical Operators
      "and", "or", "in", "exists", "between", "like", "is",

      // Set Operations
      "union", "intersect", "except",

      // Control Flow
      "case", "when", "then", "else", "end", "if", "while", "for", "do",

      // Database Objects
      "database", "table", "index", "view", "sequence", "trigger", "procedure", "function",

      // Other Keywords
      "use", "show", "describe", "explain", "help", "status", "to", "into", "values"  // 注意这里添加了into和values
  };
  return keywords;
}

int main() {
    std::cout << "Testing keyword recognition:" << std::endl;
    
    // 测试词法分析器中的关键字集合
    const auto &keywords = getSQLKeywords();
    
    std::cout << "'into' in keywords: " << (keywords.find("into") != keywords.end() ? "YES" : "NO") << std::endl;
    std::cout << "'values' in keywords: " << (keywords.find("values") != keywords.end() ? "YES" : "NO") << std::endl;
    std::cout << "'insert' in keywords: " << (keywords.find("insert") != keywords.end() ? "YES" : "NO") << std::endl;
    
    // 测试大小写转换
    std::string upperInto = "INTO";
    std::string upperValues = "VALUES";
    
    std::string lowerInto = upperInto;
    std::transform(lowerInto.begin(), lowerInto.end(), lowerInto.begin(), ::tolower);
    
    std::string lowerValues = upperValues;
    std::transform(lowerValues.begin(), lowerValues.end(), lowerValues.begin(), ::tolower);
    
    std::cout << "Original 'INTO' -> lowercase: " << lowerInto << std::endl;
    std::cout << "Original 'VALUES' -> lowercase: " << lowerValues << std::endl;
    
    std::cout << "Lowercase 'into' in keywords: " << (keywords.find(lowerInto) != keywords.end() ? "YES" : "NO") << std::endl;
    std::cout << "Lowercase 'values' in keywords: " << (keywords.find(lowerValues) != keywords.end() ? "YES" : "NO") << std::endl;
    
    return 0;
}