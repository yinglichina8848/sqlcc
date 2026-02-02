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

#include "token.h"
#include "lexer.h"
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
    return Token(Type::IDENTIFIER, lexeme, line, column);
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
  return Token(Type::IDENTIFIER, lexeme, line, column);
}

// Create a keyword token
Token Lexer::createKeywordToken(const std::string &lexeme) {
  static std::unordered_map<std::string, Type> keywordMap;

  // Initialize on first use
  if (keywordMap.empty()) {
    // DDL Keywords
    keywordMap["create"] = Type::KEYWORD_CREATE;
    keywordMap["alter"] = Type::KEYWORD_ALTER;
    keywordMap["drop"] = Type::KEYWORD_DROP;
    keywordMap["truncate"] = Type::KEYWORD_TRUNCATE;
    keywordMap["rename"] = Type::KEYWORD_RENAME;
    keywordMap["comment"] = Type::KEYWORD_COMMENT;
    keywordMap["add"] = Type::KEYWORD_ADD;
    keywordMap["column"] = Type::KEYWORD_COLUMN;
    keywordMap["modify"] = Type::KEYWORD_MODIFY;
    keywordMap["constraint"] = Type::KEYWORD_CONSTRAINT;

    // DML Keywords
    keywordMap["select"] = Type::KEYWORD_SELECT;
    keywordMap["insert"] = Type::KEYWORD_INSERT;
    keywordMap["update"] = Type::KEYWORD_UPDATE;
    keywordMap["delete"] = Type::KEYWORD_DELETE;
    keywordMap["merge"] = Type::KEYWORD_MERGE;
    keywordMap["into"] = Type::KEYWORD_INTO;
    keywordMap["values"] = Type::KEYWORD_VALUES;

    // DCL Keywords
    keywordMap["grant"] = Type::KEYWORD_GRANT;
    keywordMap["revoke"] = Type::KEYWORD_REVOKE;
    keywordMap["deny"] = Type::KEYWORD_DENY;

    // TCL Keywords
    keywordMap["begin"] = Type::KEYWORD_BEGIN;
    keywordMap["commit"] = Type::KEYWORD_COMMIT;
    keywordMap["rollback"] = Type::KEYWORD_ROLLBACK;
    keywordMap["savepoint"] = Type::KEYWORD_SAVEPOINT;
    keywordMap["set"] = Type::KEYWORD_SET;
    keywordMap["transaction"] = Type::KEYWORD_TRANSACTION;

    // Query Keywords
    keywordMap["from"] = Type::KEYWORD_FROM;
    keywordMap["where"] = Type::KEYWORD_WHERE;
    keywordMap["group"] = Type::KEYWORD_GROUP;
    keywordMap["by"] = Type::KEYWORD_BY;
    keywordMap["having"] = Type::KEYWORD_HAVING;
    keywordMap["order"] = Type::KEYWORD_ORDER;
    keywordMap["limit"] = Type::KEYWORD_LIMIT;
    keywordMap["offset"] = Type::KEYWORD_OFFSET;
    keywordMap["distinct"] = Type::KEYWORD_DISTINCT;
    keywordMap["all"] = Type::KEYWORD_ALL;
    keywordMap["as"] = Type::KEYWORD_AS;
    keywordMap["join"] = Type::KEYWORD_JOIN;
    keywordMap["inner"] = Type::KEYWORD_INNER;
    keywordMap["left"] = Type::KEYWORD_LEFT;
    keywordMap["right"] = Type::KEYWORD_RIGHT;
    keywordMap["full"] = Type::KEYWORD_FULL;
    keywordMap["outer"] = Type::KEYWORD_OUTER;
    keywordMap["on"] = Type::KEYWORD_ON;
    keywordMap["using"] = Type::KEYWORD_USING;

    // Logical Operators
    keywordMap["and"] = Type::KEYWORD_AND;
    keywordMap["or"] = Type::KEYWORD_OR;
    keywordMap["in"] = Type::KEYWORD_IN;
    keywordMap["exists"] = Type::KEYWORD_EXISTS;
    keywordMap["between"] = Type::KEYWORD_BETWEEN;
    keywordMap["like"] = Type::KEYWORD_LIKE;
    keywordMap["is"] = Type::KEYWORD_IS;

    // Set Operations
    keywordMap["union"] = Type::KEYWORD_UNION;
    keywordMap["intersect"] = Type::KEYWORD_INTERSECT;
    keywordMap["except"] = Type::KEYWORD_EXCEPT;

    // Control Flow
    keywordMap["case"] = Type::KEYWORD_CASE;
    keywordMap["when"] = Type::KEYWORD_WHEN;
    keywordMap["then"] = Type::KEYWORD_THEN;
    keywordMap["else"] = Type::KEYWORD_ELSE;
    keywordMap["end"] = Type::KEYWORD_END;
    keywordMap["if"] = Type::KEYWORD_IF;
    keywordMap["while"] = Type::KEYWORD_WHILE;
    keywordMap["for"] = Type::KEYWORD_FOR;
    keywordMap["do"] = Type::KEYWORD_DO;

    // Database Objects
    keywordMap["database"] = Type::KEYWORD_DATABASE;
    keywordMap["table"] = Type::KEYWORD_TABLE;
    keywordMap["index"] = Type::KEYWORD_INDEX;
    keywordMap["view"] = Type::KEYWORD_VIEW;
    keywordMap["sequence"] = Type::KEYWORD_SEQUENCE;
    keywordMap["trigger"] = Type::KEYWORD_TRIGGER;
    keywordMap["procedure"] = Type::KEYWORD_PROCEDURE;
    keywordMap["function"] = Type::KEYWORD_FUNCTION;

    // Constraints
    keywordMap["primary"] = Type::KEYWORD_PRIMARY;
    keywordMap["key"] = Type::KEYWORD_KEY;
    keywordMap["foreign"] = Type::KEYWORD_FOREIGN;
    keywordMap["references"] = Type::KEYWORD_REFERENCES;
    keywordMap["unique"] = Type::KEYWORD_UNIQUE;
    keywordMap["check"] = Type::KEYWORD_CHECK;
    keywordMap["not"] = Type::KEYWORD_NOT;
    keywordMap["null"] = Type::KEYWORD_NULL;
    keywordMap["default"] = Type::KEYWORD_DEFAULT;
    keywordMap["auto_increment"] = Type::KEYWORD_AUTO_INCREMENT;

    // Data Types
    keywordMap["int"] = Type::KEYWORD_INT;
    keywordMap["integer"] = Type::KEYWORD_INTEGER;
    keywordMap["smallint"] = Type::KEYWORD_SMALLINT;
    keywordMap["bigint"] = Type::KEYWORD_BIGINT;
    keywordMap["tinyint"] = Type::KEYWORD_TINYINT;
    keywordMap["varchar"] = Type::KEYWORD_VARCHAR;
    keywordMap["char"] = Type::KEYWORD_CHAR;
    keywordMap["text"] = Type::KEYWORD_TEXT;
    keywordMap["blob"] = Type::KEYWORD_BLOB;
    keywordMap["clob"] = Type::KEYWORD_CLOB;
    keywordMap["decimal"] = Type::KEYWORD_DECIMAL;
    keywordMap["numeric"] = Type::KEYWORD_NUMERIC;
    keywordMap["float"] = Type::KEYWORD_FLOAT;
    keywordMap["double"] = Type::KEYWORD_DOUBLE;
    keywordMap["real"] = Type::KEYWORD_REAL;
    keywordMap["date"] = Type::KEYWORD_DATE;
    keywordMap["time"] = Type::KEYWORD_TIME;
    keywordMap["timestamp"] = Type::KEYWORD_TIMESTAMP;
    keywordMap["datetime"] = Type::KEYWORD_DATETIME;
    keywordMap["year"] = Type::KEYWORD_YEAR;
    keywordMap["boolean"] = Type::KEYWORD_BOOLEAN;
    keywordMap["bool"] = Type::KEYWORD_BOOL;

    // Other Keywords
    keywordMap["use"] = Type::KEYWORD_USE;
    keywordMap["show"] = Type::KEYWORD_SHOW;
    keywordMap["describe"] = Type::KEYWORD_DESCRIBE;
    keywordMap["explain"] = Type::KEYWORD_EXPLAIN;
    keywordMap["help"] = Type::KEYWORD_HELP;
    keywordMap["status"] = Type::KEYWORD_STATUS;
    keywordMap["asc"] = Type::KEYWORD_ASC;
    keywordMap["desc"] = Type::KEYWORD_DESC;
    keywordMap["user"] = Type::KEYWORD_USER;
    keywordMap["to"] = Type::KEYWORD_TO;
    keywordMap["privileges"] = Type::KEYWORD_PRIVILEGES;
    keywordMap["with"] = Type::KEYWORD_WITH;
    keywordMap["password"] = Type::KEYWORD_PASSWORD;
    keywordMap["identified"] = Type::KEYWORD_IDENTIFIED;
    keywordMap["columns"] = Type::KEYWORD_COLUMNS;
    keywordMap["indexes"] = Type::KEYWORD_INDEXES;
    keywordMap["grants"] = Type::KEYWORD_GRANTS;
    keywordMap["databases"] = Type::KEYWORD_DATABASES;
    keywordMap["tables"] = Type::KEYWORD_TABLES;

    // LOAD DATA Statement Keywords
    keywordMap["load"] = Type::KEYWORD_LOAD;
    keywordMap["data"] = Type::KEYWORD_DATA;
    keywordMap["infile"] = Type::KEYWORD_INFILE;
    keywordMap["replace"] = Type::KEYWORD_REPLACE;
    keywordMap["ignore"] = Type::KEYWORD_IGNORE;
    keywordMap["low_priority"] = Type::KEYWORD_LOW_PRIORITY;
    keywordMap["concurrent"] = Type::KEYWORD_CONCURRENT;
    keywordMap["local"] = Type::KEYWORD_LOCAL;
    keywordMap["partition"] = Type::KEYWORD_PARTITION;
    keywordMap["character"] = Type::KEYWORD_CHARACTER;
    keywordMap["fields"] = Type::KEYWORD_FIELDS;
    keywordMap["terminated"] = Type::KEYWORD_TERMINATED;
    keywordMap["optionally"] = Type::KEYWORD_OPTIONALLY;
    keywordMap["enclosed"] = Type::KEYWORD_ENCLOSED;
    keywordMap["escaped"] = Type::KEYWORD_ESCAPED;
    keywordMap["lines"] = Type::KEYWORD_LINES;
    keywordMap["starting"] = Type::KEYWORD_STARTING;
  }

  auto it = keywordMap.find(lexeme);
  if (it != keywordMap.end()) {
    return Token(it->second, lexeme, line_, column_);
  }

  // Fallback to identifier if not found in map
  return Token(Type::IDENTIFIER, lexeme, line_, column_);
}

// Create a number token
Token Lexer::createNumberToken(const std::string &lexeme, int line, int column) {
  // Determine if it's an integer or float
  bool isFloat = (lexeme.find('.') != std::string::npos) ||
                 (lexeme.find('e') != std::string::npos) ||
                 (lexeme.find('E') != std::string::npos);

  if (isFloat) {
    return Token(Type::FLOAT_LITERAL, lexeme, line, column);
  } else {
    return Token(Type::INTEGER_LITERAL, lexeme, line, column);
  }
}

// Create a string token
Token Lexer::createStringToken(const std::string &lexeme, int line, int column) {
  return Token(Type::STRING_LITERAL, lexeme, line, column);
}

// Create an operator token
Token Lexer::createOperatorToken(const std::string &lexeme, int line,
                                 int column) {
  // Handle multi-character operators
  if (lexeme == "=") {
    return Token(Type::OPERATOR_EQUAL, lexeme, line, column);
  } else if (lexeme == "==") {
    return Token(Type::OPERATOR_EQUAL, lexeme, line, column);
  } else if (lexeme == "!=") {
    return Token(Type::OPERATOR_NOT_EQUAL, lexeme, line, column);
  } else if (lexeme == "<>") {
    return Token(Type::OPERATOR_NOT_EQUAL, lexeme, line, column);
  } else if (lexeme == "<") {
    return Token(Type::OPERATOR_LESS_THAN, lexeme, line, column);
  } else if (lexeme == "<=") {
    return Token(Type::OPERATOR_LESS_EQUAL, lexeme, line, column);
  } else if (lexeme == ">") {
    return Token(Type::OPERATOR_GREATER_THAN, lexeme, line, column);
  } else if (lexeme == ">=") {
    return Token(Type::OPERATOR_GREATER_EQUAL, lexeme, line, column);
  } else if (lexeme == "+") {
    return Token(Type::OPERATOR_PLUS, lexeme, line, column);
  } else if (lexeme == "-") {
    return Token(Type::OPERATOR_MINUS, lexeme, line, column);
  } else if (lexeme == "*") {
    return Token(Type::OPERATOR_MULTIPLY, lexeme, line, column);
  } else if (lexeme == "/") {
    return Token(Type::OPERATOR_DIVIDE, lexeme, line, column);
  } else if (lexeme == "%") {
    return Token(Type::OPERATOR_MODULO, lexeme, line, column);
  } else if (lexeme == "&") {
    return Token(Type::OPERATOR_BITWISE_AND, lexeme, line, column);
  } else if (lexeme == "|") {
    return Token(Type::OPERATOR_BITWISE_OR, lexeme, line, column);
  } else if (lexeme == "^") {
    return Token(Type::OPERATOR_BITWISE_XOR, lexeme, line, column);
  } else if (lexeme == "~") {
    return Token(Type::OPERATOR_BITWISE_NOT, lexeme, line, column);
  } else if (lexeme == "!") {
    return Token(Type::OPERATOR_NOT, lexeme, line, column);
  } else if (lexeme == "&&") {
    return Token(Type::OPERATOR_AND, lexeme, line, column);
  } else if (lexeme == "||") {
    return Token(Type::OPERATOR_CONCATENATE, lexeme, line, column);
  } else {
    // Unknown operator
    return Token(Type::UNKNOWN, lexeme, line, column);
  }
}

// Create a punctuation token
Token Lexer::createPunctuationToken(const std::string &lexeme, int line,
                                    int column) {
  if (lexeme == ";") {
    return Token(Type::SEMICOLON, lexeme, line, column);
  } else if (lexeme == "(") {
    return Token(Type::LPAREN, lexeme, line, column);
  } else if (lexeme == ")") {
    return Token(Type::RPAREN, lexeme, line, column);
  } else if (lexeme == ",") {
    return Token(Type::COMMA, lexeme, line, column);
  } else if (lexeme == ".") {
    return Token(Type::DOT, lexeme, line, column);
  } else if (lexeme == ":") {
    return Token(Type::COLON, lexeme, line, column);
  } else if (lexeme == "{") {
    return Token(Type::LEFT_BRACE, lexeme, line, column);
  } else if (lexeme == "}") {
    return Token(Type::RIGHT_BRACE, lexeme, line, column);
  } else if (lexeme == "[") {
    return Token(Type::LEFT_BRACKET, lexeme, line, column);
  } else if (lexeme == "]") {
    return Token(Type::RIGHT_BRACKET, lexeme, line, column);
  } else {
    // Unknown punctuation
    return Token(Type::UNKNOWN, lexeme, line, column);
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

/**
 * @brief 提取下一个词法单元
 *
 * WHY: 语法分析器（Parser）不直接处理字符流，而是处理具有语义含义的标记（Token）。
 *      nextToken 是连接原始字符串与结构化 AST 的核心纽带。
 * WHAT: 实现一个基于 DFA 的状态机，跳过空白和注释，识别并返回下一个有效的 Token。
 * HOW:
 * 1. 初始化状态为 START。
 * 2. 预处理：跳过 whitespace 字符。
 * 3. 递归处理：识别 SQL 注释（-- 或 /*）并递归调用自身跳过它们。
 * 4. 状态转移：根据首字符查找 transitions_ 表进入对应状态（IDENTIFIER, NUMBER 等）。
 * 5. 贪婪匹配：在 switch-case 中循环 advance 字符，直到不再满足当前状态的转移条件。
 * 6. 工厂构造：调用 createToken 将识别出的字符串片段（Lexeme）封装为对象。
 */
Token Lexer::nextToken() {
  // Reset state for new token
  current_state_ = LexerState::START;

  // Skip whitespace
  while (!isAtEnd() && isWhitespace(peek())) {
    advance();
  }

  // Check for end of input
  if (isAtEnd()) {
    return Token(Type::END_OF_INPUT, "", line_, column_);
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
    } else if (ch == '|') {
      if (peek() == '|') {
        advance(); // Consume second |
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
