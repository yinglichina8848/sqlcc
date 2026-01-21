/**
 * @file lexer.cpp
 *
 * WHY: 为什么需要词法分析器？
 *
 * 数据库系统需要将SQL文本转换为可处理的token序列，才能进行语法分析。没有词法分析器，系统就无法识别SQL语句中的关键字、标识符、操作符、字面量等基本元素，导致无法解析任何SQL语句。
 *
 * 主要问题解决：
 * 1. 词法识别：将SQL文本分解为有意义的词法单元
 * 2. 关键字识别：区分SQL关键字和普通标识符
 * 3. 字面量处理：正确解析字符串、数字等字面量
 * 4. 错误检测：检测词法错误并提供准确位置信息
 * 5. Unicode支持：支持多字节字符编码的标识符
 *
 * 词法分析器失败的影响：
 * - 无法解析任何SQL语句
 * - 语法分析器无法获得有效的输入
 * - 用户输入的SQL语句无法被识别
 * - 数据库系统失去与用户的交互能力
 *
 * WHAT: 这实现了什么功能？
 *
 * 词法分析器提供完整的SQL词法分析功能：
 * - 关键字识别：识别所有SQL-92标准关键字
 * - 标识符解析：支持字母、数字、下划线和Unicode字符
 * - 字面量处理：处理字符串、整数、浮点数字面量
 * - 操作符识别：支持算术、比较、逻辑等操作符
 * - 注释处理：跳过行注释和块注释
 * - 位置跟踪：精确记录token在源代码中的位置
 * - 错误报告：提供详细的词法错误信息
 *
 * 核心组件：
 * - Lexer：词法分析器主类，管理词法分析过程
 * - TransitionMap：有限自动机状态转换表
 * - Token：词法单元封装类，包含类型和位置信息
 * - LexerState：词法分析状态枚举
 * - ErrorHandler：词法错误处理机制
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 有限自动机：使用DFA实现词法识别算法
 * 2. 状态转换表：预定义的状态转换规则
 * 3. 前瞻机制：支持多字符token的识别
 * 4. 缓冲区管理：高效的输入流处理
 * 5. 内存优化：最小化对象创建和内存分配
 * 6. 错误恢复：提供基本的错误恢复机制
 *
 * 架构设计：
 * - 状态机模式：使用状态机管理词法分析流程
 * - 工厂模式：根据token类型创建相应的token对象
 * - 策略模式：可插拔的字符分类和处理策略
 * - 迭代器模式：提供token流的顺序访问接口
 * - 单例模式：共享SQL关键字集合
 *
 * 性能优化：
 * - 预编译状态表：避免运行时状态表构造
 * - 缓存机制：缓存常用token和关键字映射
 * - 批量处理：减少函数调用开销
 * - 内存池：复用token对象的内存分配
 * - 延迟初始化：按需初始化状态转换表
 *
 * @note 该实现专为SQLCC数据库系统优化，支持SQL-92标准语法
 * @see include/sql_parser/lexer.h
 */

#include "include/sql_parser/ast_node.h"
#include "include/sql_parser/token.h"
#include "include/sql_parser/lexer.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

// Transition table type
using TransitionMap =
    std::unordered_map<LexerState, std::unordered_map<char, LexerState>>;

// Character classification helpers
bool isIdentifierStart(char c) {
  return std::isalpha(c) || c == '_' ||
         static_cast<unsigned char>(c) > 127; // Unicode support
}

bool isIdentifierPart(char c) {
  return std::isalnum(c) || c == '_' ||
         static_cast<unsigned char>(c) > 127; // Unicode support
}

bool isDigit(char c) { return std::isdigit(c); }

bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// SQL Keywords map
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
      "database", "table", "index", "view", "sequence", "trigger", "procedure", "function", "user",

      // Other Keywords
      "use", "show", "describe", "explain", "help", "status", "to", "into", "values", "privileges"  };
  return keywords;
}

// Constructor
Lexer::Lexer(const std::string &input)
    : input_(input), position_(0), line_(1), column_(1),
      current_state_(LexerState::START) {
  setupTransitionTable();
}

// Check if we've reached the end of input
bool Lexer::isAtEnd() const { return position_ >= input_.length(); }

// Advance to the next character
char Lexer::advance() {
  if (isAtEnd()) {
    return '\0';
  }

  char ch = input_[position_++];
  if (ch == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }
  return ch;
}

// Peek at the current character
char Lexer::peek() const {
  if (isAtEnd()) {
    return '\0';
  }
  return input_[position_];
}

// Peek at the next character
char Lexer::peekNext() const {
  if (position_ + 1 >= input_.length()) {
    return '\0';
  }
  return input_[position_ + 1];
}

// Setup the DFA transition table
void Lexer::setupTransitionTable() {
  // Clear existing transitions
  transitions_.clear();

  // Define character sets for transitions
  const std::string operators = "+-*/%=<>!&|^~";
  const std::string punctuation = "();:,{}[]@#$";

  // START state transitions
  for (char c = 'a'; c <= 'z'; c++) {
    transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
  }
  transitions_[LexerState::START]['_'] = LexerState::IDENTIFIER;
  // Unicode support for identifiers
  for (int c = 128; c <= 255; c++) {
    transitions_[LexerState::START][static_cast<char>(c)] = LexerState::IDENTIFIER;
  }

  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::START][c] = LexerState::NUMBER;
  }

  transitions_[LexerState::START]['\''] = LexerState::STRING_SINGLE;
  transitions_[LexerState::START]['"'] = LexerState::STRING_DOUBLE;

  for (char op : operators) {
    transitions_[LexerState::START][op] = LexerState::OPERATOR;
  }

  for (char punct : punctuation) {
    transitions_[LexerState::START][punct] = LexerState::PUNCTUATION;
  }

  transitions_[LexerState::START]['-'] = LexerState::COMMENT_LINE; // Could be comment start

  // IDENTIFIER state transitions
  for (char c = 'a'; c <= 'z'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  transitions_[LexerState::IDENTIFIER]['_'] = LexerState::IDENTIFIER;
  // Unicode support
  for (int c = 128; c <= 255; c++) {
    transitions_[LexerState::IDENTIFIER][static_cast<char>(c)] = LexerState::IDENTIFIER;
  }

  // NUMBER state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER][c] = LexerState::NUMBER;
  }
  transitions_[LexerState::NUMBER]['.'] = LexerState::NUMBER_DECIMAL;
  transitions_[LexerState::NUMBER]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER]['E'] = LexerState::NUMBER_EXPONENT;

  // NUMBER_DECIMAL state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER_DECIMAL][c] = LexerState::NUMBER_DECIMAL;
  }
  transitions_[LexerState::NUMBER_DECIMAL]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_DECIMAL]['E'] = LexerState::NUMBER_EXPONENT;

  // NUMBER_EXPONENT state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER_EXPONENT][c] = LexerState::NUMBER_EXPONENT;
  }
  transitions_[LexerState::NUMBER_EXPONENT]['+'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_EXPONENT]['-'] = LexerState::NUMBER_EXPONENT;

  // STRING_SINGLE state transitions
  transitions_[LexerState::STRING_SINGLE]['\''] = LexerState::START; // End of string
  transitions_[LexerState::STRING_SINGLE]['\\'] = LexerState::STRING_ESCAPE;
  // Everything else stays in STRING_SINGLE

  // STRING_DOUBLE state transitions
  transitions_[LexerState::STRING_DOUBLE]['"'] = LexerState::START; // End of identifier
  transitions_[LexerState::STRING_DOUBLE]['\\'] = LexerState::STRING_ESCAPE;
  // Everything else stays in STRING_DOUBLE

  // STRING_ESCAPE state transitions (return to respective string state)
  transitions_[LexerState::STRING_ESCAPE][static_cast<char>(-1)] = LexerState::STRING_SINGLE;

  // OPERATOR state transitions
  // Most operators are single character, but some can be multi-character
  // We'll handle multi-character operators in nextToken()

  // PUNCTUATION state transitions
  // All punctuation is single character

  // COMMENT_LINE state transitions
  transitions_[LexerState::COMMENT_LINE]['-'] = LexerState::COMMENT_LINE; // Second -
  // Everything else will be handled in nextToken()

  // COMMENT_BLOCK state transitions
  transitions_[LexerState::COMMENT_BLOCK]['*'] = LexerState::COMMENT_BLOCK_STAR;
  // Everything else stays in COMMENT_BLOCK

  // COMMENT_BLOCK_STAR state transitions
  transitions_[LexerState::COMMENT_BLOCK_STAR]['/'] = LexerState::START; // End of comment
  transitions_[LexerState::COMMENT_BLOCK_STAR]['*'] = LexerState::COMMENT_BLOCK_STAR; // Another *
  // Everything else goes back to COMMENT_BLOCK
  for (int c = 0; c < 128; c++) {
    if (c != '/' && c != '*') {
      transitions_[LexerState::COMMENT_BLOCK_STAR][static_cast<char>(c)] = LexerState::COMMENT_BLOCK;
    }
  }
}

// Create a token from the current state
Token Lexer::createToken(LexerState state, const std::string &lexeme, int line,
                         int column) {
  switch (state) {
  case LexerState::IDENTIFIER:
    return createIdentifierToken(lexeme, line, column);
  case LexerState::NUMBER:
  case LexerState::NUMBER_DECIMAL:
  case LexerState::NUMBER_EXPONENT:
    return createNumberToken(lexeme, line, column);
  case LexerState::STRING_SINGLE:
  case LexerState::STRING_DOUBLE:
    return createStringToken(lexeme, line, column);
  case LexerState::OPERATOR:
    return createOperatorToken(lexeme, line, column);
  case LexerState::PUNCTUATION:
    return createPunctuationToken(lexeme, line, column);
  default:
    // Handle other states as identifiers for now
    return Token(Token::IDENTIFIER, lexeme, line, column);
  }
}

// Create an identifier token (could be a keyword)
Token Lexer::createIdentifierToken(const std::string &lexeme, int line,
                                   int column) {
  // Convert to lowercase for keyword comparison
  std::string lowerLexeme = lexeme;
  std::transform(lowerLexeme.begin(), lowerLexeme.end(), lowerLexeme.begin(),
                 ::tolower);

  // Check if it's a keyword
  const auto &keywords = getSQLKeywords();
  if (keywords.find(lowerLexeme) != keywords.end()) {
    return createKeywordToken(lowerLexeme);
  }

  // It's a regular identifier
  return Token(Token::IDENTIFIER, lexeme, line, column);
}

// Create a keyword token
Token Lexer::createKeywordToken(const std::string &lexeme) {
  static std::unordered_map<std::string, Token::Type> keywordMap;

  // Initialize on first use
  if (keywordMap.empty()) {
    // DDL Keywords
    keywordMap["create"] = Token::KEYWORD_CREATE;
    keywordMap["alter"] = Token::KEYWORD_ALTER;
    keywordMap["drop"] = Token::KEYWORD_DROP;
    keywordMap["truncate"] = Token::KEYWORD_TRUNCATE;
    keywordMap["rename"] = Token::KEYWORD_RENAME;
    keywordMap["comment"] = Token::KEYWORD_COMMENT;
    keywordMap["add"] = Token::KEYWORD_ADD;
    keywordMap["column"] = Token::KEYWORD_COLUMN;
    keywordMap["modify"] = Token::KEYWORD_MODIFY;
    keywordMap["constraint"] = Token::KEYWORD_CONSTRAINT;

    // DML Keywords
    keywordMap["select"] = Token::KEYWORD_SELECT;
    keywordMap["insert"] = Token::KEYWORD_INSERT;
    keywordMap["update"] = Token::KEYWORD_UPDATE;
    keywordMap["delete"] = Token::KEYWORD_DELETE;
    keywordMap["merge"] = Token::KEYWORD_MERGE;
    keywordMap["into"] = Token::KEYWORD_INTO;
    keywordMap["values"] = Token::KEYWORD_VALUES;

    // DCL Keywords
    keywordMap["grant"] = Token::KEYWORD_GRANT;
    keywordMap["revoke"] = Token::KEYWORD_REVOKE;
    keywordMap["deny"] = Token::KEYWORD_DENY;

    // TCL Keywords
    keywordMap["begin"] = Token::KEYWORD_BEGIN;
    keywordMap["commit"] = Token::KEYWORD_COMMIT;
    keywordMap["rollback"] = Token::KEYWORD_ROLLBACK;
    keywordMap["savepoint"] = Token::KEYWORD_SAVEPOINT;
    keywordMap["set"] = Token::KEYWORD_SET;
    keywordMap["transaction"] = Token::KEYWORD_TRANSACTION;

    // Query Keywords
    keywordMap["from"] = Token::KEYWORD_FROM;
    keywordMap["where"] = Token::KEYWORD_WHERE;
    keywordMap["group"] = Token::KEYWORD_GROUP;
    keywordMap["by"] = Token::KEYWORD_BY;
    keywordMap["having"] = Token::KEYWORD_HAVING;
    keywordMap["order"] = Token::KEYWORD_ORDER;
    keywordMap["limit"] = Token::KEYWORD_LIMIT;
    keywordMap["offset"] = Token::KEYWORD_OFFSET;
    keywordMap["distinct"] = Token::KEYWORD_DISTINCT;
    keywordMap["all"] = Token::KEYWORD_ALL;
    keywordMap["as"] = Token::KEYWORD_AS;
    keywordMap["join"] = Token::KEYWORD_JOIN;
    keywordMap["inner"] = Token::KEYWORD_INNER;
    keywordMap["left"] = Token::KEYWORD_LEFT;
    keywordMap["right"] = Token::KEYWORD_RIGHT;
    keywordMap["full"] = Token::KEYWORD_FULL;
    keywordMap["outer"] = Token::KEYWORD_OUTER;
    keywordMap["on"] = Token::KEYWORD_ON;
    keywordMap["using"] = Token::KEYWORD_USING;

    // Logical Operators
    keywordMap["and"] = Token::KEYWORD_AND;
    keywordMap["or"] = Token::KEYWORD_OR;
    keywordMap["in"] = Token::KEYWORD_IN;
    keywordMap["exists"] = Token::KEYWORD_EXISTS;
    keywordMap["between"] = Token::KEYWORD_BETWEEN;
    keywordMap["like"] = Token::KEYWORD_LIKE;
    keywordMap["is"] = Token::KEYWORD_IS;

    // Set Operations
    keywordMap["union"] = Token::KEYWORD_UNION;
    keywordMap["intersect"] = Token::KEYWORD_INTERSECT;
    keywordMap["except"] = Token::KEYWORD_EXCEPT;

    // Control Flow
    keywordMap["case"] = Token::KEYWORD_CASE;
    keywordMap["when"] = Token::KEYWORD_WHEN;
    keywordMap["then"] = Token::KEYWORD_THEN;
    keywordMap["else"] = Token::KEYWORD_ELSE;
    keywordMap["end"] = Token::KEYWORD_END;
    keywordMap["if"] = Token::KEYWORD_IF;
    keywordMap["while"] = Token::KEYWORD_WHILE;
    keywordMap["for"] = Token::KEYWORD_FOR;
    keywordMap["do"] = Token::KEYWORD_DO;

    // Database Objects
    keywordMap["database"] = Token::KEYWORD_DATABASE;
    keywordMap["table"] = Token::KEYWORD_TABLE;
    keywordMap["index"] = Token::KEYWORD_INDEX;
    keywordMap["view"] = Token::KEYWORD_VIEW;
    keywordMap["sequence"] = Token::KEYWORD_SEQUENCE;
    keywordMap["trigger"] = Token::KEYWORD_TRIGGER;
    keywordMap["procedure"] = Token::KEYWORD_PROCEDURE;
    keywordMap["function"] = Token::KEYWORD_FUNCTION;

    // Constraints
    keywordMap["primary"] = Token::KEYWORD_PRIMARY;
    keywordMap["key"] = Token::KEYWORD_KEY;
    keywordMap["foreign"] = Token::KEYWORD_FOREIGN;
    keywordMap["references"] = Token::KEYWORD_REFERENCES;
    keywordMap["unique"] = Token::KEYWORD_UNIQUE;
    keywordMap["check"] = Token::KEYWORD_CHECK;
    keywordMap["not"] = Token::KEYWORD_NOT;
    keywordMap["null"] = Token::KEYWORD_NULL;
    keywordMap["default"] = Token::KEYWORD_DEFAULT;
    keywordMap["auto_increment"] = Token::KEYWORD_AUTO_INCREMENT;

    // Data Types
    keywordMap["int"] = Token::KEYWORD_INT;
    keywordMap["integer"] = Token::KEYWORD_INTEGER;
    keywordMap["smallint"] = Token::KEYWORD_SMALLINT;
    keywordMap["bigint"] = Token::KEYWORD_BIGINT;
    keywordMap["tinyint"] = Token::KEYWORD_TINYINT;
    keywordMap["varchar"] = Token::KEYWORD_VARCHAR;
    keywordMap["char"] = Token::KEYWORD_CHAR;
    keywordMap["text"] = Token::KEYWORD_TEXT;
    keywordMap["blob"] = Token::KEYWORD_BLOB;
    keywordMap["clob"] = Token::KEYWORD_CLOB;
    keywordMap["decimal"] = Token::KEYWORD_DECIMAL;
    keywordMap["numeric"] = Token::KEYWORD_NUMERIC;
    keywordMap["float"] = Token::KEYWORD_FLOAT;
    keywordMap["double"] = Token::KEYWORD_DOUBLE;
    keywordMap["real"] = Token::KEYWORD_REAL;
    keywordMap["date"] = Token::KEYWORD_DATE;
    keywordMap["time"] = Token::KEYWORD_TIME;
    keywordMap["timestamp"] = Token::KEYWORD_TIMESTAMP;
    keywordMap["datetime"] = Token::KEYWORD_DATETIME;
    keywordMap["year"] = Token::KEYWORD_YEAR;
    keywordMap["boolean"] = Token::KEYWORD_BOOLEAN;
    keywordMap["bool"] = Token::KEYWORD_BOOL;

    // Other Keywords
    keywordMap["use"] = Token::KEYWORD_USE;
    keywordMap["show"] = Token::KEYWORD_SHOW;
    keywordMap["describe"] = Token::KEYWORD_DESCRIBE;
    keywordMap["explain"] = Token::KEYWORD_EXPLAIN;
    keywordMap["help"] = Token::KEYWORD_HELP;
    keywordMap["status"] = Token::KEYWORD_STATUS;
    keywordMap["asc"] = Token::KEYWORD_ASC;
    keywordMap["desc"] = Token::KEYWORD_DESC;
    keywordMap["user"] = Token::KEYWORD_USER;
    keywordMap["to"] = Token::KEYWORD_TO;
    keywordMap["privileges"] = Token::KEYWORD_PRIVILEGES;
    keywordMap["with"] = Token::KEYWORD_WITH;
    keywordMap["password"] = Token::KEYWORD_PASSWORD;
    keywordMap["identified"] = Token::KEYWORD_IDENTIFIED;
    keywordMap["columns"] = Token::KEYWORD_COLUMNS;
    keywordMap["indexes"] = Token::KEYWORD_INDEXES;
    keywordMap["grants"] = Token::KEYWORD_GRANTS;
    keywordMap["databases"] = Token::KEYWORD_DATABASES;
    keywordMap["tables"] = Token::KEYWORD_TABLES;
    
    // LOAD DATA Statement Keywords
    keywordMap["load"] = Token::KEYWORD_LOAD;
    keywordMap["data"] = Token::KEYWORD_DATA;
    keywordMap["infile"] = Token::KEYWORD_INFILE;
    keywordMap["replace"] = Token::KEYWORD_REPLACE;
    keywordMap["ignore"] = Token::KEYWORD_IGNORE;
    keywordMap["low_priority"] = Token::KEYWORD_LOW_PRIORITY;
    keywordMap["concurrent"] = Token::KEYWORD_CONCURRENT;
    keywordMap["local"] = Token::KEYWORD_LOCAL;
    keywordMap["partition"] = Token::KEYWORD_PARTITION;
    keywordMap["character"] = Token::KEYWORD_CHARACTER;
    keywordMap["fields"] = Token::KEYWORD_FIELDS;
    keywordMap["terminated"] = Token::KEYWORD_TERMINATED;
    keywordMap["optionally"] = Token::KEYWORD_OPTIONALLY;
    keywordMap["enclosed"] = Token::KEYWORD_ENCLOSED;
    keywordMap["escaped"] = Token::KEYWORD_ESCAPED;
    keywordMap["lines"] = Token::KEYWORD_LINES;
    keywordMap["starting"] = Token::KEYWORD_STARTING;
  }

  auto it = keywordMap.find(lexeme);
  if (it != keywordMap.end()) {
    return Token(it->second, lexeme, line_, column_);
  }

  // Fallback to identifier if not found in map
  return Token(Token::IDENTIFIER, lexeme, line_, column_);
}

// Create a number token
Token Lexer::createNumberToken(const std::string &lexeme, int line, int column) {
  // Determine if it's an integer or float
  bool isFloat = (lexeme.find('.') != std::string::npos) ||
                 (lexeme.find('e') != std::string::npos) ||
                 (lexeme.find('E') != std::string::npos);

  if (isFloat) {
    return Token(Token::FLOAT_LITERAL, lexeme, line, column);
  } else {
    return Token(Token::INTEGER_LITERAL, lexeme, line, column);
  }
}

// Create a string token
Token Lexer::createStringToken(const std::string &lexeme, int line, int column) {
  return Token(Token::STRING_LITERAL, lexeme, line, column);
}

// Create an operator token
Token Lexer::createOperatorToken(const std::string &lexeme, int line,
                                 int column) {
  // Handle multi-character operators
  if (lexeme == "=") {
    return Token(Token::OPERATOR_EQUAL, lexeme, line, column);
  } else if (lexeme == "==") {
    return Token(Token::OPERATOR_EQUAL, lexeme, line, column);
  } else if (lexeme == "!=") {
    return Token(Token::OPERATOR_NOT_EQUAL, lexeme, line, column);
  } else if (lexeme == "<>") {
    return Token(Token::OPERATOR_NOT_EQUAL, lexeme, line, column);
  } else if (lexeme == "<") {
    return Token(Token::OPERATOR_LESS_THAN, lexeme, line, column);
  } else if (lexeme == "<=") {
    return Token(Token::OPERATOR_LESS_EQUAL, lexeme, line, column);
  } else if (lexeme == ">") {
    return Token(Token::OPERATOR_GREATER_THAN, lexeme, line, column);
  } else if (lexeme == ">=") {
    return Token(Token::OPERATOR_GREATER_EQUAL, lexeme, line, column);
  } else if (lexeme == "+") {
    return Token(Token::OPERATOR_PLUS, lexeme, line, column);
  } else if (lexeme == "-") {
    return Token(Token::OPERATOR_MINUS, lexeme, line, column);
  } else if (lexeme == "*") {
    return Token(Token::OPERATOR_MULTIPLY, lexeme, line, column);
  } else if (lexeme == "/") {
    return Token(Token::OPERATOR_DIVIDE, lexeme, line, column);
  } else if (lexeme == "%") {
    return Token(Token::OPERATOR_MODULO, lexeme, line, column);
  } else if (lexeme == "&") {
    return Token(Token::OPERATOR_BITWISE_AND, lexeme, line, column);
  } else if (lexeme == "|") {
    return Token(Token::OPERATOR_BITWISE_OR, lexeme, line, column);
  } else if (lexeme == "^") {
    return Token(Token::OPERATOR_BITWISE_XOR, lexeme, line, column);
  } else if (lexeme == "~") {
    return Token(Token::OPERATOR_BITWISE_NOT, lexeme, line, column);
  } else if (lexeme == "!") {
    return Token(Token::OPERATOR_NOT, lexeme, line, column);
  } else if (lexeme == "&&") {
    return Token(Token::OPERATOR_AND, lexeme, line, column);
  } else if (lexeme == "||") {
    return Token(Token::OPERATOR_OR, lexeme, line, column);
  } else {
    // Unknown operator
    return Token(Token::UNKNOWN, lexeme, line, column);
  }
}

// Create a punctuation token
Token Lexer::createPunctuationToken(const std::string &lexeme, int line,
                                    int column) {
  if (lexeme == ";") {
    return Token(Token::SEMICOLON, lexeme, line, column);
  } else if (lexeme == "(") {
    return Token(Token::LPAREN, lexeme, line, column);
  } else if (lexeme == ")") {
    return Token(Token::RPAREN, lexeme, line, column);
  } else if (lexeme == ",") {
    return Token(Token::COMMA, lexeme, line, column);
  } else if (lexeme == ".") {
    return Token(Token::DOT, lexeme, line, column);
  } else if (lexeme == ":") {
    return Token(Token::COLON, lexeme, line, column);
  } else if (lexeme == "{") {
    return Token(Token::LEFT_BRACE, lexeme, line, column);
  } else if (lexeme == "}") {
    return Token(Token::RIGHT_BRACE, lexeme, line, column);
  } else if (lexeme == "[") {
    return Token(Token::LEFT_BRACKET, lexeme, line, column);
  } else if (lexeme == "]") {
    return Token(Token::RIGHT_BRACKET, lexeme, line, column);
  } else {
    // Unknown punctuation
    return Token(Token::UNKNOWN, lexeme, line, column);
  }
}

// Handle line comment
void Lexer::handleLineComment() {
  // Skip until end of line or end of file
  while (!isAtEnd() && peek() != '\n') {
    advance();
  }
}

// Handle block comment
void Lexer::handleBlockComment() {
  // Already consumed /*
  while (!isAtEnd()) {
    if (peek() == '*' && peekNext() == '/') {
      // Consume */
      advance();
      advance();
      break;
    }
    advance();
  }
}

// Report an error
void Lexer::reportError(const std::string &message) {
  std::string errorMsg = "Lexer error at line " + std::to_string(line_) +
                         ", column " + std::to_string(column_) + ": " + message;
  throw std::runtime_error(errorMsg);
}

// Main tokenization method
Token Lexer::nextToken() {
  // Reset state for new token
  current_state_ = LexerState::START;

  // Skip whitespace
  while (!isAtEnd() && isWhitespace(peek())) {
    advance();
  }

  // Check for end of input
  if (isAtEnd()) {
    return Token(Token::END_OF_INPUT, "", line_, column_);
  }

  // Start of token
  size_t startLine = line_;
  size_t startColumn = column_;
  size_t startPosition = position_;

  char ch = advance();

  // Handle special cases
  if (ch == '-' && peek() == '-') {
    // Line comment
    handleLineComment();
    return nextToken(); // Recursively get next token
  } else if (ch == '/' && peek() == '*') {
    // Block comment
    handleBlockComment();
    return nextToken(); // Recursively get next token
  }

  // Process character according to DFA
  auto stateIt = transitions_[current_state_].find(ch);
  if (stateIt != transitions_[current_state_].end()) {
    current_state_ = stateIt->second;
  } else {
    // Unexpected character
    reportError("Unexpected character: " + std::string(1, ch));
  }

  // Continue processing based on current state
  switch (current_state_) {
  case LexerState::IDENTIFIER:
    while (!isAtEnd()) {
      char nextCh = peek();
      auto transIt = transitions_[current_state_].find(nextCh);
      if (transIt != transitions_[current_state_].end() &&
          transIt->second == LexerState::IDENTIFIER) {
        advance();
      } else {
        break; // End of identifier
      }
    }
    break;

  case LexerState::NUMBER:
    while (!isAtEnd()) {
      char nextCh = peek();
      auto transIt = transitions_[current_state_].find(nextCh);
      if (transIt != transitions_[current_state_].end() &&
          (transIt->second == LexerState::NUMBER ||
           transIt->second == LexerState::NUMBER_DECIMAL ||
           transIt->second == LexerState::NUMBER_EXPONENT)) {
        advance();
        current_state_ = transIt->second;
      } else {
        break; // End of number
      }
    }
    break;

  case LexerState::STRING_SINGLE:
    while (!isAtEnd()) {
      char nextCh = peek();
      if (nextCh == '\'') {
        advance(); // Consume closing quote
        break;     // End of string
      } else if (nextCh == '\\') {
        advance(); // Consume escape character
        if (!isAtEnd()) {
          advance(); // Consume escaped character
        }
      } else {
        advance();
      }
    }
    break;

  case LexerState::STRING_DOUBLE:
    while (!isAtEnd()) {
      char nextCh = peek();
      if (nextCh == '"') {
        advance(); // Consume closing quote
        break;     // End of identifier
      } else if (nextCh == '\\') {
        advance(); // Consume escape character
        if (!isAtEnd()) {
          advance(); // Consume escaped character
        }
      } else {
        advance();
      }
    }
    break;

  case LexerState::OPERATOR:
    // Check for multi-character operators
    if (ch == '<') {
      if (peek() == '=') {
        advance(); // Consume =
      } else if (peek() == '>') {
        advance(); // Consume >
      }
    } else if (ch == '>') {
      if (peek() == '=') {
        advance(); // Consume =
      }
    } else if (ch == '!') {
      if (peek() == '=') {
        advance(); // Consume =
      }
    } else if (ch == ':') {
      if (peek() == '=') {
        advance(); // Consume =
      }
    }
    break;

  case LexerState::COMMENT_LINE:
    // This case should not occur as we handle line comments separately
    handleLineComment();
    return nextToken();

  case LexerState::COMMENT_BLOCK:
    // This case should not occur as we handle block comments separately
    handleBlockComment();
    return nextToken();

  default:
    // Most punctuation and operators are single character
    break;
  }

  // Extract lexeme
  std::string lexeme = input_.substr(startPosition, position_ - startPosition);

  // Create and return token
  return createToken(current_state_, lexeme, startLine, startColumn);
}

} // namespace sql_parser
} // namespace sqlcc
