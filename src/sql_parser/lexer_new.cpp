#include "sql_parser/lexer_new.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

// DFA State definitions
enum class LexerState {
  START,              // Initial state
  IDENTIFIER,         // Identifier state
  NUMBER,             // Number literal state
  NUMBER_DECIMAL,     // Decimal part of number
  NUMBER_EXPONENT,    // Exponent part of number
  STRING_SINGLE,      // Single-quoted string
  STRING_DOUBLE,      // Double-quoted identifier
  STRING_ESCAPE,      // Escape sequence in string
  COMMENT_LINE,       // Single-line comment (--)
  COMMENT_BLOCK,      // Multi-line comment (/* */)
  COMMENT_BLOCK_STAR, // In block comment, saw *
  OPERATOR,           // Operator state
  PUNCTUATION,        // Punctuation state
  ERROR               // Error state
};

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
      "create", "alter", "drop", "truncate", "rename", "comment",

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

      // Boolean Values
      "true", "false",

      // Database Objects
      "database", "schema", "table", "view", "index", "trigger", "procedure",
      "function",

      // Permission Keywords
      "with", "password", "user", "identified", "privileges", "show", "columns",
      "indexes", "grants", "databases", "tables",

      // Miscellaneous
      "use", "into", "values", "set"};
  return keywords;
}

LexerNew::LexerNew(const std::string &input)
    : input_(input), position_(0), line_(1), column_(1),
      current_state_(LexerState::START) {
  std::cout << "[LEXER DEBUG] LexerNew构造函数，输入长度: " << input_.length()
            << ", 输入内容: '" << input_ << "'" << std::endl;
  setupTransitionTable();
}

void LexerNew::setupTransitionTable() {
  std::cout << "[LEXER DEBUG] 开始初始化转换表" << std::endl;

  // Initialize transition table for DFA
  transitions_[LexerState::START] = {
      {' ', LexerState::START},          {'\t', LexerState::START},
      {'\r', LexerState::START},         {'\n', LexerState::START},
      {'-', LexerState::OPERATOR},       {'+', LexerState::OPERATOR},
      {'*', LexerState::OPERATOR},       {'/', LexerState::OPERATOR},
      {'=', LexerState::OPERATOR},       {'!', LexerState::OPERATOR},
      {'<', LexerState::OPERATOR},       {'>', LexerState::OPERATOR},
      {'%', LexerState::OPERATOR},       {'&', LexerState::OPERATOR},
      {'|', LexerState::OPERATOR},       {'~', LexerState::OPERATOR},
      {'^', LexerState::OPERATOR},       {'?', LexerState::OPERATOR},
      {'@', LexerState::OPERATOR},       {'$', LexerState::OPERATOR},
      {'(', LexerState::PUNCTUATION},    {')', LexerState::PUNCTUATION},
      {',', LexerState::PUNCTUATION},    {';', LexerState::PUNCTUATION},
      {'.', LexerState::PUNCTUATION},    {':', LexerState::PUNCTUATION},
      {'[', LexerState::PUNCTUATION},    {']', LexerState::PUNCTUATION},
      {'{', LexerState::PUNCTUATION},    {'}', LexerState::PUNCTUATION},
      {'\'', LexerState::STRING_SINGLE}, {'"', LexerState::STRING_DOUBLE}};

  std::cout << "[LEXER DEBUG] START状态转换表初始化完成" << std::endl;

  // Add transitions for identifiers and numbers
  for (char c = 'a'; c <= 'z'; ++c) {
    transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
    transitions_[LexerState::START][c - 32] =
        LexerState::IDENTIFIER; // uppercase
  }
  transitions_[LexerState::START]['_'] = LexerState::IDENTIFIER;

  std::cout << "[LEXER DEBUG] 字母转换表初始化完成" << std::endl;

  // Add Unicode support for identifier start
  for (int c = 128; c <= 255; ++c) {
    transitions_[LexerState::START][static_cast<char>(c)] =
        LexerState::IDENTIFIER;
  }

  std::cout << "[LEXER DEBUG] Unicode转换表初始化完成" << std::endl;

  // Number transitions
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::START][c] = LexerState::NUMBER;
  }

  std::cout << "[LEXER DEBUG] 数字转换表初始化完成" << std::endl;

  // Identifier continuation
  transitions_[LexerState::IDENTIFIER] = std::unordered_map<char, LexerState>();
  for (char c = 'a'; c <= 'z'; ++c) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
    transitions_[LexerState::IDENTIFIER][c - 32] = LexerState::IDENTIFIER;
  }
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  transitions_[LexerState::IDENTIFIER]['_'] = LexerState::IDENTIFIER;

  // Add Unicode support for identifier continuation
  for (int c = 128; c <= 255; ++c) {
    transitions_[LexerState::IDENTIFIER][static_cast<char>(c)] =
        LexerState::IDENTIFIER;
  }

  // 为IDENTIFIER状态添加空白字符的转换规则（结束token）
  transitions_[LexerState::IDENTIFIER][' '] = LexerState::START;
  transitions_[LexerState::IDENTIFIER]['\t'] = LexerState::START;
  transitions_[LexerState::IDENTIFIER]['\r'] = LexerState::START;
  transitions_[LexerState::IDENTIFIER]['\n'] = LexerState::START;

  // Number transitions
  transitions_[LexerState::NUMBER] = std::unordered_map<char, LexerState>();
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::NUMBER][c] = LexerState::NUMBER;
  }
  transitions_[LexerState::NUMBER]['.'] = LexerState::NUMBER_DECIMAL;
  transitions_[LexerState::NUMBER]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER]['E'] = LexerState::NUMBER_EXPONENT;

  transitions_[LexerState::NUMBER_DECIMAL] =
      std::unordered_map<char, LexerState>();
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::NUMBER_DECIMAL][c] = LexerState::NUMBER_DECIMAL;
  }
  transitions_[LexerState::NUMBER_DECIMAL]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_DECIMAL]['E'] = LexerState::NUMBER_EXPONENT;

  transitions_[LexerState::NUMBER_EXPONENT] =
      std::unordered_map<char, LexerState>();
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::NUMBER_EXPONENT][c] = LexerState::NUMBER_EXPONENT;
  }
  transitions_[LexerState::NUMBER_EXPONENT]['+'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_EXPONENT]['-'] = LexerState::NUMBER_EXPONENT;

  // String transitions
  transitions_[LexerState::STRING_SINGLE] =
      std::unordered_map<char, LexerState>();
  transitions_[LexerState::STRING_SINGLE]['\\'] = LexerState::STRING_ESCAPE;
  transitions_[LexerState::STRING_SINGLE]['\''] =
      LexerState::START; // End of single-quoted string

  // For STRING_SINGLE state, most characters should stay in the same state
  // to continue reading the string content
  for (char c = 32; c <= 126; ++c) { // ASCII printable characters
    if (c != '\\' && c != '\'') {
      transitions_[LexerState::STRING_SINGLE][c] = LexerState::STRING_SINGLE;
    }
  }
  // Handle newlines and other special characters
  transitions_[LexerState::STRING_SINGLE]['\n'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_SINGLE]['\r'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_SINGLE]['\t'] = LexerState::STRING_SINGLE;

  transitions_[LexerState::STRING_DOUBLE] =
      std::unordered_map<char, LexerState>();
  transitions_[LexerState::STRING_DOUBLE]['"'] =
      LexerState::START; // End of identifier

  transitions_[LexerState::STRING_ESCAPE] =
      std::unordered_map<char, LexerState>();
  transitions_[LexerState::STRING_ESCAPE]['\''] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_ESCAPE]['"'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_ESCAPE]['\\'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_ESCAPE]['n'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_ESCAPE]['t'] = LexerState::STRING_SINGLE;
  transitions_[LexerState::STRING_ESCAPE]['r'] = LexerState::STRING_SINGLE;

  // 为PUNCTUATION状态添加转换表
  // 标点符号通常是单字符的，所以任何后续字符都应该结束当前token
  transitions_[LexerState::PUNCTUATION] =
      std::unordered_map<char, LexerState>();
  // 为PUNCTUATION状态添加空白字符的转换规则（结束token）
  transitions_[LexerState::PUNCTUATION][' '] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['\t'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['\r'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['\n'] = LexerState::START;

  // 为PUNCTUATION状态添加字母字符的转换规则（结束token，开始新token）
  for (char c = 'a'; c <= 'z'; ++c) {
    transitions_[LexerState::PUNCTUATION][c] = LexerState::START;
    transitions_[LexerState::PUNCTUATION][c - 32] =
        LexerState::START; // uppercase
  }
  transitions_[LexerState::PUNCTUATION]['_'] = LexerState::START;

  // 为PUNCTUATION状态添加数字字符的转换规则（结束token，开始新token）
  for (char c = '0'; c <= '9'; ++c) {
    transitions_[LexerState::PUNCTUATION][c] = LexerState::START;
  }

  // 为PUNCTUATION状态添加其他常见字符的转换规则（结束token，开始新token）
  transitions_[LexerState::PUNCTUATION]['('] = LexerState::START;
  transitions_[LexerState::PUNCTUATION][')'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION][','] = LexerState::START;
  transitions_[LexerState::PUNCTUATION][';'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['.'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['-'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['+'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['*'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['/'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['='] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['!'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['<'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['>'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['%'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['&'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['|'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['~'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['^'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['?'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['@'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['$'] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['\''] = LexerState::START;
  transitions_[LexerState::PUNCTUATION]['"'] = LexerState::START;

  // 为OPERATOR状态添加转换表
  transitions_[LexerState::OPERATOR] = std::unordered_map<char, LexerState>();
  // 运算符通常是单字符的，所以任何后续字符都应该结束当前token
  // 这里我们不为任何字符设置转换，这样遇到任何字符都会结束运算符token

  // 为注释相关状态添加空的转换表
  transitions_[LexerState::COMMENT_LINE] =
      std::unordered_map<char, LexerState>();
  transitions_[LexerState::COMMENT_BLOCK] =
      std::unordered_map<char, LexerState>();
  transitions_[LexerState::COMMENT_BLOCK_STAR] =
      std::unordered_map<char, LexerState>();

  // 为ERROR状态添加空的转换表
  transitions_[LexerState::ERROR] = std::unordered_map<char, LexerState>();

  std::cout << "[LEXER DEBUG] 转换表初始化完成" << std::endl;
}

bool LexerNew::isAtEnd() const { return position_ >= input_.length(); }

char LexerNew::advance() {
  if (isAtEnd())
    return '\0';
  char ch = input_[position_++];
  if (ch == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }
  return ch;
}

char LexerNew::peek() const {
  if (isAtEnd())
    return '\0';
  return input_[position_];
}

char LexerNew::peekNext() const {
  if (position_ + 1 >= input_.length())
    return '\0';
  return input_[position_ + 1];
}

Token LexerNew::nextToken() {
  std::cout << "[LEXER DEBUG] nextToken方法被调用，当前位置: " << position_
            << std::endl;

  while (!isAtEnd()) {
    char ch = peek();

    // Handle comments first
    if (ch == '-' && peekNext() == '-') {
      skipLineComment();
      continue;
    }
    if (ch == '/' && peekNext() == '*') {
      skipBlockComment();
      continue;
    }

    // Skip whitespace
    if (isWhitespace(ch)) {
      advance();
      continue;
    }

    // Reset to start state
    current_state_ = LexerState::START;
    int start_pos = position_;
    int start_line = line_;
    int start_column = column_;
    LexerState final_state = LexerState::START;

    std::cout << "[LEXER DEBUG] 开始处理token，当前位置: " << position_
              << ", 字符: '" << ch << "'" << std::endl;

    // DFA processing
    while (!isAtEnd()) {
      ch = peek();

      // 空白字符应该结束当前token，而不是在DFA循环中跳过
      // 检查当前字符是否为空白字符，如果是，则结束token
      if (isWhitespace(ch)) {
        std::cout << "[LEXER DEBUG] 遇到空白字符，结束token" << std::endl;
        break;
      }

      auto state_it = transitions_.find(current_state_);
      if (state_it == transitions_.end()) {
        // No transitions from this state, token complete
        std::cout << "[LEXER DEBUG] 状态 " << static_cast<int>(current_state_)
                  << " 没有转换表，结束token" << std::endl;
        break;
      }

      auto char_it = state_it->second.find(ch);
      if (char_it == state_it->second.end()) {
        // No transition for this character, token complete
        std::cout << "[LEXER DEBUG] 状态 " << static_cast<int>(current_state_)
                  << " 字符 '" << ch << "' 没有转换，结束token" << std::endl;
        break;
      }

      // Transition to new state
      current_state_ = char_it->second;
      std::cout << "[LEXER DEBUG] 转换到状态 "
                << static_cast<int>(current_state_) << ", 字符: '" << ch << "'"
                << std::endl;
      advance();

      // 特殊处理：标点符号应该是单字符的token
      if (current_state_ == LexerState::PUNCTUATION) {
        std::cout << "[LEXER DEBUG] 标点符号状态，立即结束token" << std::endl;
        break;
      }

      // 字符串状态处理：单引号字符串在遇到结束单引号时转换到START状态
      if (current_state_ == LexerState::START &&
          (state_it->first == LexerState::STRING_SINGLE ||
           state_it->first == LexerState::STRING_DOUBLE)) {
        std::cout << "[LEXER DEBUG] 字符串结束，状态转换到START" << std::endl;
        final_state = state_it->first; // 保存字符串状态用于创建token
        break;
      }

      // 检查是否到达输入末尾
      if (isAtEnd()) {
        std::cout << "[LEXER DEBUG] 到达输入末尾，结束token" << std::endl;
        break;
      }
    }

    // Create token based on final state
    std::string lexeme = input_.substr(start_pos, position_ - start_pos);

    // 添加调试输出
    std::cout << "[LEXER DEBUG] 生成token: '" << lexeme
              << "' (状态: " << static_cast<int>(final_state) << ")"
              << std::endl;

    return createToken(final_state, lexeme, start_line, start_column);
  }

  std::cout << "[LEXER DEBUG] 到达输入末尾，返回END_OF_INPUT" << std::endl;
  return Token(Token::END_OF_INPUT, "", line_, column_);
}

Token LexerNew::createToken(LexerState state, const std::string &lexeme,
                            int line, int column) {
  switch (state) {
  case LexerState::START:
    // START状态通常不应该到达这里，但为了安全处理
    // 检查lexeme是否为空或只包含空白字符
    if (lexeme.empty() ||
        std::all_of(lexeme.begin(), lexeme.end(), isWhitespace)) {
      // 这不应该发生，因为空白字符应该在nextToken中被跳过
      return Token(Token::UNKNOWN, lexeme, line, column);
    }
    // 对于非空白字符，尝试作为标识符处理
    return createIdentifierToken(lexeme, line, column);

  case LexerState::IDENTIFIER:
    return createIdentifierToken(lexeme, line, column);

  case LexerState::NUMBER:
  case LexerState::NUMBER_DECIMAL:
  case LexerState::NUMBER_EXPONENT:
    return createNumberToken(lexeme, line, column);

  case LexerState::STRING_SINGLE:
    // For STRING_SINGLE state, we need to handle escape sequences properly
    // The lexeme includes the quotes, so we need to extract the content
    if (lexeme.length() >= 2 && lexeme.front() == '\'' &&
        lexeme.back() == '\'') {
      std::string content = lexeme.substr(1, lexeme.length() - 2);
      // Process escape sequences
      std::string processed_content;
      for (size_t i = 0; i < content.length(); ++i) {
        if (content[i] == '\\' && i + 1 < content.length()) {
          switch (content[i + 1]) {
          case 'n':
            processed_content += '\n';
            break;
          case 't':
            processed_content += '\t';
            break;
          case 'r':
            processed_content += '\r';
            break;
          case '\\':
            processed_content += '\\';
            break;
          case '\'':
            processed_content += '\'';
            break;
          case '"':
            processed_content += '"';
            break;
          default:
            processed_content += content[i + 1];
            break;
          }
          ++i;
        } else {
          processed_content += content[i];
        }
      }
      std::cout << "[LEXER DEBUG] STRING_SINGLE token: '" << processed_content
                << "' (type: " << static_cast<int>(Token::STRING_LITERAL) << ")"
                << std::endl;
      return Token(Token::STRING_LITERAL, processed_content, line, column);
    }
    // Fallback: remove quotes and return
    std::cout << "[LEXER DEBUG] STRING_SINGLE fallback token: '"
              << lexeme.substr(1, lexeme.length() - 2)
              << "' (type: " << static_cast<int>(Token::STRING_LITERAL) << ")"
              << std::endl;
    return Token(Token::STRING_LITERAL, lexeme.substr(1, lexeme.length() - 2),
                 line, column);

  case LexerState::STRING_DOUBLE:
    // Remove quotes and return identifier
    return Token(Token::IDENTIFIER, lexeme.substr(1, lexeme.length() - 2), line,
                 column);

  case LexerState::STRING_ESCAPE:
    // STRING_ESCAPE state should never reach here as it's handled within
    // STRING_SINGLE This is a fallback case
    std::cout << "[LEXER DEBUG] STRING_ESCAPE state reached (should not happen)"
              << std::endl;
    return Token(Token::UNKNOWN, lexeme, line, column);

  case LexerState::OPERATOR:
    return createOperatorToken(lexeme, line, column);

  case LexerState::PUNCTUATION:
    return createPunctuationToken(lexeme, line, column);

  default:
    return Token(Token::UNKNOWN, lexeme, line, column);
  }
}

Token LexerNew::createIdentifierToken(const std::string &lexeme, int line,
                                      int column) {
  // Convert to lowercase for keyword comparison
  std::string lower_lexeme = lexeme;
  for (char &c : lower_lexeme) {
    c = std::tolower(c);
  }

  // Check if it's a keyword that we support
  static std::unordered_set<std::string> supportedKeywords = {
      // DDL Keywords
      "create", "alter", "drop", "table", "index", "database",

      // DML Keywords
      "select", "insert", "update", "delete", "from", "into", "values", "set",

      // Query Keywords
      "where", "group", "by", "having", "order", "limit", "offset", "distinct",

      // Join Keywords
      "join", "on", "outer",

      // Constraint Keywords
      "primary", "key", "foreign", "references", "unique", "not", "null",
      "default", "auto_increment",

      // Permission Keywords
      "grant", "revoke", "to", "user", "with", "password", "identified", "show",

      // Logical Operators
      "and", "or", "in", "exists",

      // Aggregate Functions
      "count", "sum", "avg", "min", "max"};

  if (supportedKeywords.find(lower_lexeme) != supportedKeywords.end()) {
    return createKeywordToken(lower_lexeme, line, column);
  }

  return Token(Token::IDENTIFIER, lexeme, line, column);
}

Token LexerNew::createKeywordToken(const std::string &keyword, int line,
                                   int column) {
  static std::unordered_map<std::string, Token::Type> keywordMap;

  std::cout << "[DEBUG] createKeywordToken called with: '" << keyword << "'"
            << std::endl;

  // Initialize on first use
  if (keywordMap.empty()) {
    std::cout << "[DEBUG] Initializing keywordMap" << std::endl;

    // DDL Keywords
    keywordMap["create"] = Token::KEYWORD_CREATE;
    keywordMap["alter"] = Token::KEYWORD_ALTER;
    keywordMap["drop"] = Token::KEYWORD_DROP;
    keywordMap["table"] = Token::KEYWORD_TABLE;
    keywordMap["index"] = Token::KEYWORD_INDEX;
    keywordMap["database"] = Token::KEYWORD_DATABASE;

    // DML Keywords
    keywordMap["select"] = Token::KEYWORD_SELECT;
    keywordMap["insert"] = Token::KEYWORD_INSERT;
    keywordMap["update"] = Token::KEYWORD_UPDATE;
    keywordMap["delete"] = Token::KEYWORD_DELETE;
    keywordMap["from"] = Token::KEYWORD_FROM;
    keywordMap["into"] = Token::KEYWORD_INTO;
    keywordMap["values"] = Token::KEYWORD_VALUES;
    keywordMap["set"] = Token::KEYWORD_SET;

    // Query Keywords
    keywordMap["where"] = Token::KEYWORD_WHERE;
    keywordMap["group"] = Token::KEYWORD_GROUP;
    keywordMap["by"] = Token::KEYWORD_BY;
    keywordMap["having"] = Token::KEYWORD_HAVING;
    keywordMap["order"] = Token::KEYWORD_ORDER;
    keywordMap["limit"] = Token::KEYWORD_LIMIT;
    keywordMap["offset"] = Token::KEYWORD_OFFSET;
    keywordMap["distinct"] = Token::KEYWORD_DISTINCT;

    // Join Keywords
    keywordMap["join"] = Token::KEYWORD_JOIN;
    keywordMap["on"] = Token::KEYWORD_ON;
    keywordMap["outer"] = Token::KEYWORD_OUTER;

    // Constraint Keywords
    keywordMap["primary"] = Token::KEYWORD_PRIMARY;
    keywordMap["key"] = Token::KEYWORD_KEY;
    keywordMap["foreign"] = Token::KEYWORD_FOREIGN;
    keywordMap["references"] = Token::KEYWORD_REFERENCES;
    keywordMap["unique"] = Token::KEYWORD_UNIQUE;
    keywordMap["not"] = Token::KEYWORD_NOT;
    keywordMap["null"] = Token::KEYWORD_NULL;
    keywordMap["default"] = Token::KEYWORD_DEFAULT;
    keywordMap["auto_increment"] = Token::KEYWORD_AUTO_INCREMENT;

    // Permission Keywords
    keywordMap["grant"] = Token::KEYWORD_GRANT;
    keywordMap["revoke"] = Token::KEYWORD_REVOKE;
    keywordMap["to"] = Token::KEYWORD_TO;
    keywordMap["user"] = Token::KEYWORD_USER;
    keywordMap["with"] = Token::KEYWORD_WITH;
    keywordMap["password"] = Token::KEYWORD_PASSWORD;
    keywordMap["identified"] = Token::KEYWORD_IDENTIFIED;
    keywordMap["show"] = Token::KEYWORD_SHOW;

    // Logical Operators
    keywordMap["and"] = Token::KEYWORD_AND;
    keywordMap["or"] = Token::KEYWORD_OR;
    keywordMap["in"] = Token::KEYWORD_IN;
    keywordMap["exists"] = Token::KEYWORD_EXISTS;

    // Aggregate Functions
    keywordMap["count"] = Token::KEYWORD_COUNT;
    keywordMap["sum"] = Token::KEYWORD_SUM;
    keywordMap["avg"] = Token::KEYWORD_AVG;
    keywordMap["min"] = Token::KEYWORD_MIN;
    keywordMap["max"] = Token::KEYWORD_MAX;
  }

  auto it = keywordMap.find(keyword);
  if (it != keywordMap.end()) {
    std::cout << "[DEBUG] Found keyword '" << keyword
              << "', returning type: " << static_cast<int>(it->second)
              << std::endl;
    return Token(it->second, keyword, line, column);
  }

  // If keyword is in getSQLKeywords() but not in keywordMap, treat it as an
  // identifier This avoids issues with unrecognized keywords
  const auto &keywords = getSQLKeywords();
  if (keywords.find(keyword) != keywords.end()) {
    std::cout
        << "[DEBUG] Keyword '" << keyword
        << "' in getSQLKeywords but not in keywordMap, returning IDENTIFIER"
        << std::endl;
    return Token(Token::IDENTIFIER, keyword, line, column);
  }

  std::cout << "[DEBUG] Keyword '" << keyword
            << "' not found, returning IDENTIFIER" << std::endl;
  return Token(Token::IDENTIFIER, keyword, line, column);
}

Token LexerNew::createNumberToken(const std::string &lexeme, int line,
                                  int column) {
  // Check if it's a float (contains a decimal point or exponent)
  if (lexeme.find('.') != std::string::npos ||
      lexeme.find('e') != std::string::npos ||
      lexeme.find('E') != std::string::npos) {
    return Token(Token::FLOAT_LITERAL, lexeme, line, column);
  } else {
    return Token(Token::INTEGER_LITERAL, lexeme, line, column);
  }
}

Token LexerNew::createOperatorToken(const std::string &lexeme, int line,
                                    int column) {
  if (lexeme == "+") {
    return Token(Token::OPERATOR_PLUS, lexeme, line, column);
  } else if (lexeme == "-") {
    return Token(Token::OPERATOR_MINUS, lexeme, line, column);
  } else if (lexeme == "*") {
    return Token(Token::OPERATOR_MULTIPLY, lexeme, line, column);
  } else if (lexeme == "/") {
    return Token(Token::OPERATOR_DIVIDE, lexeme, line, column);
  } else if (lexeme == "=") {
    return Token(Token::OPERATOR_EQUAL, lexeme, line, column);
  } else if (lexeme == "!=") {
    return Token(Token::OPERATOR_NOT_EQUAL, lexeme, line, column);
  } else if (lexeme == "<") {
    return Token(Token::OPERATOR_LESS_THAN, lexeme, line, column);
  } else if (lexeme == "<=") {
    return Token(Token::OPERATOR_LESS_EQUAL, lexeme, line, column);
  } else if (lexeme == ">") {
    return Token(Token::OPERATOR_GREATER_THAN, lexeme, line, column);
  } else if (lexeme == ">=") {
    return Token(Token::OPERATOR_GREATER_EQUAL, lexeme, line, column);
  }

  return Token(Token::UNKNOWN, lexeme, line, column);
}

Token LexerNew::createPunctuationToken(const std::string &lexeme, int line,
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
  }

  return Token(Token::UNKNOWN, lexeme, line, column);
}

void LexerNew::skipLineComment() {
  // Skip '--' and rest of line
  advance(); // skip first '-'
  advance(); // skip second '-'

  while (!isAtEnd() && peek() != '\n') {
    advance();
  }
}

void LexerNew::skipBlockComment() {
  // Skip '/*' ... '*/'
  advance(); // skip '/'
  advance(); // skip '*'

  while (!isAtEnd()) {
    if (peek() == '*' && peekNext() == '/') {
      advance(); // skip '*'
      advance(); // skip '/'
      break;
    }
    advance();
  }
}

} // namespace sql_parser
} // namespace sqlcc
