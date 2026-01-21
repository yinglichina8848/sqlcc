#ifndef SQLCC_SQL_PARSER_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_NODE_H

namespace sqlcc::sql_parser {

// Independent token type enum to avoid circular dependency
enum class TokenType : int {
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
  KEYWORD_ROLE,
  KEYWORD_ROLES,

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

  // Keywords - LOAD DATA Statement
  KEYWORD_LOAD,
  KEYWORD_DATA,
  KEYWORD_INFILE,
  KEYWORD_REPLACE,
  KEYWORD_IGNORE,
  KEYWORD_LOW_PRIORITY,
  KEYWORD_CONCURRENT,
  KEYWORD_LOCAL,
  KEYWORD_PARTITION,
  KEYWORD_CHARACTER,
  KEYWORD_FIELDS,
  KEYWORD_TERMINATED,
  KEYWORD_OPTIONALLY,
  KEYWORD_ENCLOSED,
  KEYWORD_ESCAPED,
  KEYWORD_LINES,
  KEYWORD_STARTING,

  // Special tokens
  END_OF_INPUT,
  ERROR,
  UNKNOWN
};

} // namespace sqlcc::sql_parser

#include "node_visitor.h"
#include <memory>
#include <string>

namespace sqlcc::sql_parser {

/**
 * AST节点基类
 */
class Node {
public:
  virtual ~Node();
  virtual void accept(NodeVisitor &visitor) { /* default empty implementation */ }
};

/**
 * 表达式节点基类
 */
class Expression : public Node {
public:
  enum Type {
    IDENTIFIER,
    STRING_LITERAL,
    NUMERIC_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    BINARY,
    UNARY,
    FUNCTION,
    EXISTS,
    IN
  };

  Expression() = default;
  virtual ~Expression();

  virtual std::string getTypeName() const { return "Expression"; }
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const;
};

// Forward declaration for Token to avoid circular dependency
class Token;

/**
 * 二元表达式节点
 */
class BinaryExpression : public Expression {
public:
  BinaryExpression(std::unique_ptr<Expression> left,
                   std::unique_ptr<Expression> right, TokenType op);
  virtual ~BinaryExpression();

  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return BINARY; }

  const Expression *getLeft() const;
  const Expression *getRight() const;
  TokenType getOperator() const;

private:
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
  TokenType op_;
};

/**
 * 语句节点基类
 */
class Statement : public Node {
public:
  enum Type {
    CREATE,
    CREATE_VIEW,
    SELECT,
    COMPOSITE_SELECT,
    INSERT,
    UPDATE,
    DELETE,
    DROP,
    ALTER,
    USE,
    CREATE_INDEX,
    DROP_INDEX,
    CREATE_USER,
    DROP_USER,
    CREATE_ROLE,
    DROP_ROLE,
    GRANT_ROLE,
    REVOKE_ROLE,
    GRANT,
    REVOKE,
    SHOW,
    COMMIT,
    ROLLBACK,
    CREATE_PROCEDURE,
    DROP_PROCEDURE,
    CALL_PROCEDURE,
    CREATE_TRIGGER,
    DROP_TRIGGER,
    ALTER_TRIGGER,
    LOAD_DATA,
    WITH_RECURSIVE,
    SAVEPOINT,
    RELEASE_SAVEPOINT,
    SET_TRANSACTION,
    CREATE_DOMAIN,
    ALTER_DOMAIN,
    DROP_DOMAIN,
    CREATE_FUNCTION,
    DROP_FUNCTION,
    ALTER_TABLE_ENHANCED
  };

  Statement(Type type) : type_(type) {}
  virtual ~Statement();

  Type getType() const { return type_; }
  std::string getTypeName() const { return "Statement"; }
  virtual void accept(NodeVisitor &visitor);

private:
  Type type_;
};

} // namespace sqlcc::sql_parser

// BinaryExpression 实现
inline sqlcc::sql_parser::BinaryExpression::BinaryExpression(
    std::unique_ptr<sqlcc::sql_parser::Expression> left,
    std::unique_ptr<sqlcc::sql_parser::Expression> right,
    sqlcc::sql_parser::TokenType op)
    : left_(std::move(left)), right_(std::move(right)), op_(op) {}

inline sqlcc::sql_parser::BinaryExpression::~BinaryExpression() = default;

inline std::string sqlcc::sql_parser::BinaryExpression::getTypeName() const {
  return "BinaryExpression";
}

namespace sqlcc::sql_parser {

inline void sqlcc::sql_parser::BinaryExpression::accept(
    sqlcc::sql_parser::NodeVisitor &visitor) {
  visitor.visit(*this);
}

inline const sqlcc::sql_parser::Expression *
sqlcc::sql_parser::BinaryExpression::getLeft() const {
  return left_.get();
}

inline const sqlcc::sql_parser::Expression *
sqlcc::sql_parser::BinaryExpression::getRight() const {
  return right_.get();
}

inline sqlcc::sql_parser::TokenType
sqlcc::sql_parser::BinaryExpression::getOperator() const {
  return op_;
}

} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODE_H
