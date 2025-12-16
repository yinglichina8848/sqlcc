#ifndef SQLCC_SQL_PARSER_TOKEN_H
#define SQLCC_SQL_PARSER_TOKEN_H

#include <string>

namespace sqlcc {
namespace sql_parser {

class Token {
public:
  enum Type {
    // Punctuation
    SEMICOLON,
    COLON,
    LPAREN,
    RPAREN,
    COMMA,
    DOT,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,

    // Literals
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,

    // Identifiers
    IDENTIFIER,

    // Operators
    OPERATOR,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS_THAN,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER_THAN,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_MODULO,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_NOT,
    OPERATOR_BITWISE_AND,
    OPERATOR_BITWISE_OR,
    OPERATOR_BITWISE_NOT,
    OPERATOR_BITWISE_XOR,
    OPERATOR_TERNARY,
    OPERATOR_AT,

    // Keywords - DDL
    KEYWORD_CREATE,
    KEYWORD_ALTER,
    KEYWORD_DROP,
    KEYWORD_TRUNCATE,
    KEYWORD_RENAME,
    KEYWORD_COMMENT,
    KEYWORD_ADD,
    KEYWORD_COLUMN,
    KEYWORD_MODIFY,
    KEYWORD_CONSTRAINT,

    // Keywords - DML
    KEYWORD_SELECT,
    KEYWORD_INSERT,
    KEYWORD_UPDATE,
    KEYWORD_DELETE,
    KEYWORD_MERGE,
    KEYWORD_FROM,
    KEYWORD_INTO,
    KEYWORD_VALUES,
    KEYWORD_SET,
    KEYWORD_WHERE,
    KEYWORD_GROUP,
    KEYWORD_BY,
    KEYWORD_HAVING,
    KEYWORD_ORDER,
    KEYWORD_LIMIT,
    KEYWORD_OFFSET,
    KEYWORD_DISTINCT,
    KEYWORD_ALL,
    KEYWORD_AS,
    KEYWORD_JOIN,
    KEYWORD_INNER,
    KEYWORD_LEFT,
    KEYWORD_RIGHT,
    KEYWORD_FULL,
    KEYWORD_OUTER,
    KEYWORD_ON,
    KEYWORD_USING,

    // Keywords - DCL
    KEYWORD_GRANT,
    KEYWORD_REVOKE,
    KEYWORD_DENY,

    // Keywords - TCL
    KEYWORD_BEGIN,
    KEYWORD_COMMIT,
    KEYWORD_ROLLBACK,
    KEYWORD_SAVEPOINT,
    KEYWORD_TRANSACTION,

    // Keywords - Logical Operators
    KEYWORD_AND,
    KEYWORD_OR,
    KEYWORD_IN,
    KEYWORD_EXISTS,
    KEYWORD_BETWEEN,
    KEYWORD_LIKE,
    KEYWORD_IS,

    // Keywords - Set Operations
    KEYWORD_UNION,
    KEYWORD_INTERSECT,
    KEYWORD_EXCEPT,

    // Keywords - Control Flow
    KEYWORD_CASE,
    KEYWORD_WHEN,
    KEYWORD_THEN,
    KEYWORD_ELSE,
    KEYWORD_END,
    KEYWORD_IF,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_DO,

    // Keywords - Database Objects
    KEYWORD_DATABASE,
    KEYWORD_TABLE,
    KEYWORD_INDEX,
    KEYWORD_VIEW,
    KEYWORD_SEQUENCE,
    KEYWORD_TRIGGER,
    KEYWORD_PROCEDURE,
    KEYWORD_FUNCTION,

    // Keywords - Trigger specific
    KEYWORD_BEFORE,
    KEYWORD_AFTER,
    KEYWORD_INSTEAD,
    KEYWORD_OF,
    KEYWORD_EACH,
    KEYWORD_ROW,
    KEYWORD_STATEMENT,

    // Keywords - Procedure specific
    KEYWORD_OUT,
    KEYWORD_INOUT,
    KEYWORD_RETURNS,
    KEYWORD_RETURN,

    // Keywords - Constraints
    KEYWORD_PRIMARY,
    KEYWORD_KEY,
    KEYWORD_FOREIGN,
    KEYWORD_REFERENCES,
    KEYWORD_UNIQUE,
    KEYWORD_CHECK,
    KEYWORD_NOT,
    KEYWORD_NULL,
    KEYWORD_DEFAULT,
    KEYWORD_AUTO_INCREMENT,

    // Keywords - Data Types
    KEYWORD_INT,
    KEYWORD_INTEGER,
    KEYWORD_SMALLINT,
    KEYWORD_BIGINT,
    KEYWORD_TINYINT,
    KEYWORD_VARCHAR,
    KEYWORD_CHAR,
    KEYWORD_TEXT,
    KEYWORD_BLOB,
    KEYWORD_CLOB,
    KEYWORD_DECIMAL,
    KEYWORD_NUMERIC,
    KEYWORD_FLOAT,
    KEYWORD_DOUBLE,
    KEYWORD_REAL,
    KEYWORD_DATE,
    KEYWORD_TIME,
    KEYWORD_TIMESTAMP,
    KEYWORD_DATETIME,
    KEYWORD_YEAR,
    KEYWORD_BOOLEAN,
    KEYWORD_BOOL,

    // Keywords - Others
    KEYWORD_USE,
    KEYWORD_SHOW,
    KEYWORD_DESCRIBE,
    KEYWORD_EXPLAIN,
    KEYWORD_HELP,
    KEYWORD_STATUS,
    KEYWORD_ASC,
    KEYWORD_DESC,
    KEYWORD_USER,
    KEYWORD_TO,
    KEYWORD_PRIVILEGES,
    KEYWORD_WITH,
    KEYWORD_PASSWORD,
    KEYWORD_IDENTIFIED,
    KEYWORD_COLUMNS,
    KEYWORD_INDEXES,
    KEYWORD_GRANTS,
    KEYWORD_DATABASES,
    KEYWORD_TABLES,
    
    // Keywords - Date/Time Functions
    KEYWORD_NOW,
    KEYWORD_CURRENT_TIMESTAMP,

    // Special tokens
    END_OF_INPUT,
    ERROR,
    UNKNOWN
  };

  // Constructors
  Token();
  Token(Type type, const std::string &lexeme, size_t line, size_t column);

  // Getters
  Type getType() const { return type_; }
  const std::string &getLexeme() const { return lexeme_; }
  size_t getLine() const { return line_; }
  size_t getColumn() const { return column_; }

  // Static utility method
  static std::string getTypeName(Type type);

private:
  Type type_;
  std::string lexeme_;
  size_t line_;
  size_t column_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_H
