#include "src/sql_parser/token.h"
#include <iostream>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

Token::Token() : type_(Type::UNKNOWN), line_(0), column_(0) {}

Token::Token(Type type, const std::string &lexeme, size_t line, size_t column)
    : type_(type), lexeme_(lexeme), line_(line), column_(column) {}

std::string Token::getTypeName(Type type) {
  static std::unordered_map<Type, std::string> typeNames = {
      // Punctuation
      {Type::SEMICOLON, "SEMICOLON"},
      {Type::COLON, "COLON"},
      {Type::LPAREN, "LPAREN"},
      {Type::RPAREN, "RPAREN"},
      {Type::COMMA, "COMMA"},
      {Type::DOT, "DOT"},
      {Type::LEFT_BRACE, "LEFT_BRACE"},
      {Type::RIGHT_BRACE, "RIGHT_BRACE"},
      {Type::LEFT_BRACKET, "LEFT_BRACKET"},
      {Type::RIGHT_BRACKET, "RIGHT_BRACKET"},

      // Literals
      {Type::INTEGER_LITERAL, "INTEGER_LITERAL"},
      {Type::FLOAT_LITERAL, "FLOAT_LITERAL"},
      {Type::STRING_LITERAL, "STRING_LITERAL"},
      {Type::BOOLEAN_LITERAL, "BOOLEAN_LITERAL"},
      {Type::NULL_LITERAL, "NULL_LITERAL"},

      // Identifiers
      {Type::IDENTIFIER, "IDENTIFIER"},

      // Operators
      {Type::OPERATOR, "OPERATOR"},
      {Type::OPERATOR_PLUS, "OPERATOR_PLUS"},
      {Type::OPERATOR_MINUS, "OPERATOR_MINUS"},
      {Type::OPERATOR_MULTIPLY, "OPERATOR_MULTIPLY"},
      {Type::OPERATOR_DIVIDE, "OPERATOR_DIVIDE"},
      {Type::OPERATOR_EQUAL, "OPERATOR_EQUAL"},
      {Type::OPERATOR_NOT_EQUAL, "OPERATOR_NOT_EQUAL"},
      {Type::OPERATOR_LESS_THAN, "OPERATOR_LESS_THAN"},
      {Type::OPERATOR_LESS_EQUAL, "OPERATOR_LESS_EQUAL"},
      {Type::OPERATOR_GREATER_THAN, "OPERATOR_GREATER_THAN"},
      {Type::OPERATOR_GREATER_EQUAL, "OPERATOR_GREATER_EQUAL"},
      {Type::OPERATOR_MODULO, "OPERATOR_MODULO"},
      {Type::OPERATOR_AND, "OPERATOR_AND"},
      {Type::OPERATOR_OR, "OPERATOR_OR"},
      {Type::OPERATOR_NOT, "OPERATOR_NOT"},
      {Type::OPERATOR_BITWISE_AND, "OPERATOR_BITWISE_AND"},
      {Type::OPERATOR_BITWISE_OR, "OPERATOR_BITWISE_OR"},
      {Type::OPERATOR_CONCATENATE, "OPERATOR_CONCATENATE"},
      {Type::OPERATOR_BITWISE_NOT, "OPERATOR_BITWISE_NOT"},
      {Type::OPERATOR_BITWISE_XOR, "OPERATOR_BITWISE_XOR"},
      {Type::OPERATOR_TERNARY, "OPERATOR_TERNARY"},
      {Type::OPERATOR_AT, "OPERATOR_AT"},

      // Keywords - DDL
      {Type::KEYWORD_CREATE, "KEYWORD_CREATE"},
      {Type::KEYWORD_ALTER, "KEYWORD_ALTER"},
      {Type::KEYWORD_DROP, "KEYWORD_DROP"},
      {Type::KEYWORD_TRUNCATE, "KEYWORD_TRUNCATE"},
      {Type::KEYWORD_RENAME, "KEYWORD_RENAME"},
      {Type::KEYWORD_COMMENT, "KEYWORD_COMMENT"},
      {Type::KEYWORD_ADD, "KEYWORD_ADD"},
      {Type::KEYWORD_COLUMN, "KEYWORD_COLUMN"},
      {Type::KEYWORD_MODIFY, "KEYWORD_MODIFY"},
      {Type::KEYWORD_CONSTRAINT, "KEYWORD_CONSTRAINT"},

      // Keywords - DML
      {Type::KEYWORD_SELECT, "KEYWORD_SELECT"},
      {Type::KEYWORD_INSERT, "KEYWORD_INSERT"},
      {Type::KEYWORD_UPDATE, "KEYWORD_UPDATE"},
      {Type::KEYWORD_DELETE, "KEYWORD_DELETE"},
      {Type::KEYWORD_MERGE, "KEYWORD_MERGE"},
      {Type::KEYWORD_FROM, "KEYWORD_FROM"},
      {Type::KEYWORD_INTO, "KEYWORD_INTO"},
      {Type::KEYWORD_VALUES, "KEYWORD_VALUES"},
      {Type::KEYWORD_SET, "KEYWORD_SET"},
      {Type::KEYWORD_WHERE, "KEYWORD_WHERE"},
      {Type::KEYWORD_GROUP, "KEYWORD_GROUP"},
      {Type::KEYWORD_BY, "KEYWORD_BY"},
      {Type::KEYWORD_HAVING, "KEYWORD_HAVING"},
      {Type::KEYWORD_ORDER, "KEYWORD_ORDER"},
      {Type::KEYWORD_LIMIT, "KEYWORD_LIMIT"},
      {Type::KEYWORD_OFFSET, "KEYWORD_OFFSET"},
      {Type::KEYWORD_DISTINCT, "KEYWORD_DISTINCT"},
      {Type::KEYWORD_ALL, "KEYWORD_ALL"},
      {Type::KEYWORD_AS, "KEYWORD_AS"},
      {Type::KEYWORD_JOIN, "KEYWORD_JOIN"},
      {Type::KEYWORD_INNER, "KEYWORD_INNER"},
      {Type::KEYWORD_LEFT, "KEYWORD_LEFT"},
      {Type::KEYWORD_RIGHT, "KEYWORD_RIGHT"},
      {Type::KEYWORD_FULL, "KEYWORD_FULL"},
      {Type::KEYWORD_OUTER, "KEYWORD_OUTER"},
      {Type::KEYWORD_ON, "KEYWORD_ON"},
      {Type::KEYWORD_USING, "KEYWORD_USING"},

      // Keywords - DCL
      {Type::KEYWORD_GRANT, "KEYWORD_GRANT"},
      {Type::KEYWORD_REVOKE, "KEYWORD_REVOKE"},
      {Type::KEYWORD_DENY, "KEYWORD_DENY"},

      // Keywords - TCL
      {Type::KEYWORD_BEGIN, "KEYWORD_BEGIN"},
      {Type::KEYWORD_COMMIT, "KEYWORD_COMMIT"},
      {Type::KEYWORD_ROLLBACK, "KEYWORD_ROLLBACK"},
      {Type::KEYWORD_SAVEPOINT, "KEYWORD_SAVEPOINT"},
      {Type::KEYWORD_TRANSACTION, "KEYWORD_TRANSACTION"},

      // Keywords - Logical Operators
      {Type::KEYWORD_AND, "KEYWORD_AND"},
      {Type::KEYWORD_OR, "KEYWORD_OR"},
      {Type::KEYWORD_IN, "KEYWORD_IN"},
      {Type::KEYWORD_EXISTS, "KEYWORD_EXISTS"},
      {Type::KEYWORD_BETWEEN, "KEYWORD_BETWEEN"},
      {Type::KEYWORD_LIKE, "KEYWORD_LIKE"},
      {Type::KEYWORD_IS, "KEYWORD_IS"},

      // Keywords - Set Operations
      {Type::KEYWORD_UNION, "KEYWORD_UNION"},
      {Type::KEYWORD_INTERSECT, "KEYWORD_INTERSECT"},
      {Type::KEYWORD_EXCEPT, "KEYWORD_EXCEPT"},

      // Keywords - Control Flow
      {Type::KEYWORD_CASE, "KEYWORD_CASE"},
      {Type::KEYWORD_WHEN, "KEYWORD_WHEN"},
      {Type::KEYWORD_THEN, "KEYWORD_THEN"},
      {Type::KEYWORD_ELSE, "KEYWORD_ELSE"},
      {Type::KEYWORD_END, "KEYWORD_END"},
      {Type::KEYWORD_IF, "KEYWORD_IF"},
      {Type::KEYWORD_WHILE, "KEYWORD_WHILE"},
      {Type::KEYWORD_FOR, "KEYWORD_FOR"},
      {Type::KEYWORD_DO, "KEYWORD_DO"},

      // Keywords - Database Objects
      {Type::KEYWORD_DATABASE, "KEYWORD_DATABASE"},
      {Type::KEYWORD_TABLE, "KEYWORD_TABLE"},
      {Type::KEYWORD_INDEX, "KEYWORD_INDEX"},
      {Type::KEYWORD_VIEW, "KEYWORD_VIEW"},
      {Type::KEYWORD_SEQUENCE, "KEYWORD_SEQUENCE"},
      {Type::KEYWORD_TRIGGER, "KEYWORD_TRIGGER"},
      {Type::KEYWORD_PROCEDURE, "KEYWORD_PROCEDURE"},
      {Type::KEYWORD_FUNCTION, "KEYWORD_FUNCTION"},

      // Keywords - Constraints
      {Type::KEYWORD_PRIMARY, "KEYWORD_PRIMARY"},
      {Type::KEYWORD_KEY, "KEYWORD_KEY"},
      {Type::KEYWORD_FOREIGN, "KEYWORD_FOREIGN"},
      {Type::KEYWORD_REFERENCES, "KEYWORD_REFERENCES"},
      {Type::KEYWORD_UNIQUE, "KEYWORD_UNIQUE"},
      {Type::KEYWORD_CHECK, "KEYWORD_CHECK"},
      {Type::KEYWORD_NOT, "KEYWORD_NOT"},
      {Type::KEYWORD_NULL, "KEYWORD_NULL"},
      {Type::KEYWORD_DEFAULT, "KEYWORD_DEFAULT"},
      {Type::KEYWORD_AUTO_INCREMENT, "KEYWORD_AUTO_INCREMENT"},

      // Keywords - Data Types
      {Type::KEYWORD_INT, "KEYWORD_INT"},
      {Type::KEYWORD_INTEGER, "KEYWORD_INTEGER"},
      {Type::KEYWORD_SMALLINT, "KEYWORD_SMALLINT"},
      {Type::KEYWORD_BIGINT, "KEYWORD_BIGINT"},
      {Type::KEYWORD_TINYINT, "KEYWORD_TINYINT"},
      {Type::KEYWORD_VARCHAR, "KEYWORD_VARCHAR"},
      {Type::KEYWORD_CHAR, "KEYWORD_CHAR"},
      {Type::KEYWORD_TEXT, "KEYWORD_TEXT"},
      {Type::KEYWORD_BLOB, "KEYWORD_BLOB"},
      {Type::KEYWORD_CLOB, "KEYWORD_CLOB"},
      {Type::KEYWORD_DECIMAL, "KEYWORD_DECIMAL"},
      {Type::KEYWORD_NUMERIC, "KEYWORD_NUMERIC"},
      {Type::KEYWORD_FLOAT, "KEYWORD_FLOAT"},
      {Type::KEYWORD_DOUBLE, "KEYWORD_DOUBLE"},
      {Type::KEYWORD_REAL, "KEYWORD_REAL"},
      {Type::KEYWORD_DATE, "KEYWORD_DATE"},
      {Type::KEYWORD_TIME, "KEYWORD_TIME"},
      {Type::KEYWORD_TIMESTAMP, "KEYWORD_TIMESTAMP"},
      {Type::KEYWORD_DATETIME, "KEYWORD_DATETIME"},
      {Type::KEYWORD_YEAR, "KEYWORD_YEAR"},
      {Type::KEYWORD_BOOLEAN, "KEYWORD_BOOLEAN"},
      {Type::KEYWORD_BOOL, "KEYWORD_BOOL"},

      // Keywords - Others
      {Type::KEYWORD_USE, "KEYWORD_USE"},
      {Type::KEYWORD_SHOW, "KEYWORD_SHOW"},
      {Type::KEYWORD_DESCRIBE, "KEYWORD_DESCRIBE"},
      {Type::KEYWORD_EXPLAIN, "KEYWORD_EXPLAIN"},
      {Type::KEYWORD_HELP, "KEYWORD_HELP"},
      {Type::KEYWORD_STATUS, "KEYWORD_STATUS"},
      {Type::KEYWORD_ASC, "KEYWORD_ASC"},
      {Type::KEYWORD_DESC, "KEYWORD_DESC"},
      {Type::KEYWORD_USER, "KEYWORD_USER"},
      {Type::KEYWORD_TO, "KEYWORD_TO"},
      {Type::KEYWORD_PRIVILEGES, "KEYWORD_PRIVILEGES"},
      {Type::KEYWORD_WITH, "KEYWORD_WITH"},
      {Type::KEYWORD_PASSWORD, "KEYWORD_PASSWORD"},
      {Type::KEYWORD_IDENTIFIED, "KEYWORD_IDENTIFIED"},
      {Type::KEYWORD_COLUMNS, "KEYWORD_COLUMNS"},
      {Type::KEYWORD_INDEXES, "KEYWORD_INDEXES"},
      {Type::KEYWORD_GRANTS, "KEYWORD_GRANTS"},
      {Type::KEYWORD_DATABASES, "KEYWORD_DATABASES"},
      {Type::KEYWORD_TABLES, "KEYWORD_TABLES"},

      // Keywords - LOAD DATA Statement
      {Type::KEYWORD_LOAD, "KEYWORD_LOAD"},
      {Type::KEYWORD_DATA, "KEYWORD_DATA"},
      {Type::KEYWORD_INFILE, "KEYWORD_INFILE"},
      {Type::KEYWORD_REPLACE, "KEYWORD_REPLACE"},
      {Type::KEYWORD_IGNORE, "KEYWORD_IGNORE"},
      {Type::KEYWORD_LOW_PRIORITY, "KEYWORD_LOW_PRIORITY"},
      {Type::KEYWORD_CONCURRENT, "KEYWORD_CONCURRENT"},
      {Type::KEYWORD_LOCAL, "KEYWORD_LOCAL"},
      {Type::KEYWORD_PARTITION, "KEYWORD_PARTITION"},
      {Type::KEYWORD_CHARACTER, "KEYWORD_CHARACTER"},
      {Type::KEYWORD_FIELDS, "KEYWORD_FIELDS"},
      {Type::KEYWORD_TERMINATED, "KEYWORD_TERMINATED"},
      {Type::KEYWORD_OPTIONALLY, "KEYWORD_OPTIONALLY"},
      {Type::KEYWORD_ENCLOSED, "KEYWORD_ENCLOSED"},
      {Type::KEYWORD_ESCAPED, "KEYWORD_ESCAPED"},
      {Type::KEYWORD_LINES, "KEYWORD_LINES"},
      {Type::KEYWORD_STARTING, "KEYWORD_STARTING"},

      // Keywords - Trigger specific
      {Type::KEYWORD_BEFORE, "KEYWORD_BEFORE"},
      {Type::KEYWORD_AFTER, "KEYWORD_AFTER"},
      {Type::KEYWORD_INSTEAD, "KEYWORD_INSTEAD"},
      {Type::KEYWORD_OF, "KEYWORD_OF"},
      {Type::KEYWORD_EACH, "KEYWORD_EACH"},
      {Type::KEYWORD_ROW, "KEYWORD_ROW"},
      {Type::KEYWORD_STATEMENT, "KEYWORD_STATEMENT"},

      // Keywords - Procedure specific
      {Type::KEYWORD_OUT, "KEYWORD_OUT"},
      {Type::KEYWORD_INOUT, "KEYWORD_INOUT"},
      {Type::KEYWORD_RETURNS, "KEYWORD_RETURNS"},
      {Type::KEYWORD_RETURN, "KEYWORD_RETURN"},

      // Keywords - Date/Time Functions
      {Type::KEYWORD_NOW, "KEYWORD_NOW"},
      {Type::KEYWORD_CURRENT_TIMESTAMP, "KEYWORD_CURRENT_TIMESTAMP"},

      // Keywords - LOAD DATA Statement
      {Type::KEYWORD_LOAD, "KEYWORD_LOAD"},
      {Type::KEYWORD_DATA, "KEYWORD_DATA"},
      {Type::KEYWORD_INFILE, "KEYWORD_INFILE"},
      {Type::KEYWORD_REPLACE, "KEYWORD_REPLACE"},
      {Type::KEYWORD_IGNORE, "KEYWORD_IGNORE"},
      {Type::KEYWORD_LOW_PRIORITY, "KEYWORD_LOW_PRIORITY"},
      {Type::KEYWORD_CONCURRENT, "KEYWORD_CONCURRENT"},
      {Type::KEYWORD_LOCAL, "KEYWORD_LOCAL"},
      {Type::KEYWORD_PARTITION, "KEYWORD_PARTITION"},
      {Type::KEYWORD_CHARACTER, "KEYWORD_CHARACTER"},
      {Type::KEYWORD_FIELDS, "KEYWORD_FIELDS"},
      {Type::KEYWORD_TERMINATED, "KEYWORD_TERMINATED"},
      {Type::KEYWORD_OPTIONALLY, "KEYWORD_OPTIONALLY"},
      {Type::KEYWORD_ENCLOSED, "KEYWORD_ENCLOSED"},
      {Type::KEYWORD_ESCAPED, "KEYWORD_ESCAPED"},
      {Type::KEYWORD_LINES, "KEYWORD_LINES"},
      {Type::KEYWORD_STARTING, "KEYWORD_STARTING"},

      // Special tokens
      {Type::END_OF_INPUT, "END_OF_INPUT"},
      {Type::ERROR, "ERROR"},
      {Type::UNKNOWN, "UNKNOWN"}};

  auto it = typeNames.find(type);
  if (it != typeNames.end()) {
    return it->second;
  }
  return "UNKNOWN_TYPE";
}

std::string Token::toString() const {
  std::string result = getTypeName(type_) + " '" + lexeme_ + "' at line " +
                       std::to_string(line_) + ", column " +
                       std::to_string(column_);
  return result;
}

} // namespace sql_parser
} // namespace sqlcc
