#ifndef SQLCC_SQL_PARSER_TOKEN_H
#define SQLCC_SQL_PARSER_TOKEN_H

#include <string>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

/**
 * SQL Token类，表示SQL语句中的一个标记
 */
class Token {
public:
  /**
   * Token类型枚举
   */
  enum Type {
    // 关键字
    KEYWORD_CREATE,
    KEYWORD_SELECT,
    KEYWORD_INSERT,
    KEYWORD_UPDATE,
    KEYWORD_DELETE,
    KEYWORD_DROP,
    KEYWORD_ALTER,
    KEYWORD_USE,
    KEYWORD_DATABASE,
    KEYWORD_TABLE,
    KEYWORD_WHERE,
    KEYWORD_JOIN,
    KEYWORD_ON,
    KEYWORD_GROUP,
    KEYWORD_BY,
    KEYWORD_HAVING,
    KEYWORD_ORDER,
    KEYWORD_INTO,
    KEYWORD_VALUES,
    KEYWORD_SET,
    KEYWORD_FROM,
    KEYWORD_AS,
    KEYWORD_DISTINCT,
    KEYWORD_AND,
    KEYWORD_OR,
    KEYWORD_NOT,
    KEYWORD_NULL,
    KEYWORD_IS,
    KEYWORD_LIKE,
    KEYWORD_IN,
    KEYWORD_EXISTS,
    KEYWORD_BETWEEN,
    KEYWORD_LIMIT,
    KEYWORD_OFFSET,
    KEYWORD_ASC,
    KEYWORD_DESC,
    KEYWORD_INDEX,
    KEYWORD_PRIMARY,
    KEYWORD_KEY,
    KEYWORD_UNIQUE,
    KEYWORD_IF,
    KEYWORD_NOT_NULL,
    KEYWORD_NULLABLE,
    KEYWORD_DEFAULT,
    KEYWORD_CONSTRAINT,
    KEYWORD_FOREIGN,
    KEYWORD_REFERENCES,
    KEYWORD_CHECK,
    KEYWORD_DECIMAL,
    KEYWORD_DATE,
    KEYWORD_TIME,
    KEYWORD_TIMESTAMP,
    KEYWORD_CHAR,
    KEYWORD_VARCHAR,
    KEYWORD_SMALLINT,
    KEYWORD_DOUBLE,
    KEYWORD_BOOLEAN,
    // 事务相关关键字
    KEYWORD_BEGIN,
    KEYWORD_TRANSACTION,
    KEYWORD_START,
    KEYWORD_COMMIT,
    KEYWORD_ROLLBACK,
    KEYWORD_SAVEPOINT,
    KEYWORD_AUTOCOMMIT,
    KEYWORD_ISOLATION,
    KEYWORD_LEVEL,
    KEYWORD_READ,
    KEYWORD_WRITE,
    KEYWORD_UNCOMMITTED,
    KEYWORD_COMMITTED,
    KEYWORD_REPEATABLE,
    KEYWORD_SERIALIZABLE,

    // 标识符
    IDENTIFIER,

    // 字面量
    STRING_LITERAL,
    NUMERIC_LITERAL,

    // 运算符
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MODULO,

    // 标点符号
    PUNCTUATION_LEFT_PAREN,
    PUNCTUATION_RIGHT_PAREN,
    PUNCTUATION_COMMA,
    PUNCTUATION_SEMICOLON,
    PUNCTUATION_DOT,
    PUNCTUATION_COLON,
    PUNCTUATION_DOUBLE_COLON,

    // 其他
    END_OF_INPUT,
    INVALID_TOKEN
  };

  /**
   * 构造函数
   * @param type Token类型
   * @param lexeme Token的文本内容
   * @param line 行号
   * @param column 列号
   */
  Token(Type type, const std::string &lexeme, int line, int column);

  /**
   * 获取Token类型
   */
  Type getType() const;

  /**
   * 获取Token的文本内容
   */
  const std::string &getLexeme() const;

  /**
   * 获取行号
   */
  int getLine() const;

  /**
   * 获取列号
   */
  int getColumn() const;

  /**
   * 获取Token类型的名称（用于调试）
   */
  std::string getTypeName() const;

  /**
   * 将Token转换为字符串表示（用于调试）
   */
  std::string toString() const;

private:
  Type type_;          // Token类型
  std::string lexeme_; // Token的文本内容
  int line_;           // 行号
  int column_;         // 列号

  // 类型名称映射表
  static std::unordered_map<Type, std::string> typeNames_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_H
