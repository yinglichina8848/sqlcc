/**
 * WHY: 为什么需要Token类来表示词法分析的结果？
 *
 * 词法分析是SQL解析器的第一阶段，将SQL字符串分解为有意义的词法单元。
 * Token类作为词法分析的结果载体，封装了词法单元的所有必要信息：
 * - 类型标识：区分关键字、标识符、操作符、字面量等不同类型
 * - 词素内容：保留原始的字符串表示形式
 * - 位置信息：记录在源代码中的行列位置，便于错误报告
 * - 类型转换：提供类型名称映射和调试输出支持
 *
 * Token类设计的核心价值：
 * 1. 信息完整性：捕获词法分析的所有输出信息
 * 2. 类型安全性：通过枚举类型确保Token类型正确性
 * 3. 调试友好：提供详细的位置信息和类型描述
 * 4. 内存效率：使用紧凑的数据结构存储Token信息
 * 5. 扩展性：支持动态添加新的Token类型
 *
 * Token类型分类体系：
 * - 标点符号：基本的语法分隔符，如括号、逗号、分号
 * - 字面量：直接表示值的Token，如数字、字符串、布尔值
 * - 标识符：用户定义的名称，如表名、列名、变量名
 * - 关键字：SQL语言保留字，如SELECT、FROM、WHERE
 * - 操作符：算术、比较、逻辑等操作符
 * - 特殊Token：注释、错误、文件结束等
 *
 * SOLID原则体现：
 * - 单一职责：Token类专注于表示词法分析结果
 * - 开闭原则：通过枚举扩展支持新的Token类型
 * - 里氏替换：所有Token实例行为一致
 * - 接口隔离：提供精确的Token操作接口
 * - 依赖倒置：不依赖具体的词法分析器实现
 */

#ifndef SQLCC_SQL_PARSER_TOKEN_H
#define SQLCC_SQL_PARSER_TOKEN_H

#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief Token类型枚举
 *
 * 定义SQL词法分析器识别的所有Token类型。
 * 按照功能分类组织，便于理解和维护。
 */
enum class Type {
  // 标点符号 (Punctuation)
  SEMICOLON,          // ;
  COLON,             // :
  LPAREN,            // (
  RPAREN,            // )
  COMMA,             // ,
  DOT,               // .
  LEFT_BRACE,        // {
  RIGHT_BRACE,       // }
  LEFT_BRACKET,      // [
  RIGHT_BRACKET,     // ]

  // 字面量 (Literals)
  INTEGER_LITERAL,   // 整数字面量，如 123
  FLOAT_LITERAL,     // 浮点数字面量，如 123.45
  STRING_LITERAL,    // 字符串字面量，如 'hello'
  BOOLEAN_LITERAL,   // 布尔字面量，如 true, false
  NULL_LITERAL,      // NULL字面量

  // 标识符 (Identifiers)
  IDENTIFIER,        // 用户定义的标识符，如表名、列名

  // 操作符 (Operators)
  OPERATOR,          // 通用操作符
  OPERATOR_PLUS,     // +
  OPERATOR_MINUS,    // -
  OPERATOR_MULTIPLY, // *
  OPERATOR_DIVIDE,   // /
  OPERATOR_EQUAL,    // =
  OPERATOR_NOT_EQUAL, // != 或 <>
  OPERATOR_LESS_THAN, // <
  OPERATOR_LESS_EQUAL, // <=
  OPERATOR_GREATER_THAN, // >
  OPERATOR_GREATER_EQUAL, // >=
  OPERATOR_MODULO,   // %
  OPERATOR_AND,      // &
  OPERATOR_OR,       // |
  OPERATOR_NOT,      // !
  OPERATOR_BITWISE_AND, // &
  OPERATOR_BITWISE_OR,  // |
  OPERATOR_CONCATENATE, // ||
  OPERATOR_BITWISE_XOR, // ^
  OPERATOR_BITWISE_NOT, // ~
  OPERATOR_TERNARY,  // ?
  OPERATOR_AT,       // @

  // DDL关键字 (DDL Keywords)
  KEYWORD_CREATE,    // CREATE
  KEYWORD_ALTER,     // ALTER
  KEYWORD_DROP,      // DROP
  KEYWORD_TRUNCATE,  // TRUNCATE
  KEYWORD_RENAME,    // RENAME
  KEYWORD_COMMENT,   // COMMENT
  KEYWORD_ADD,       // ADD
  KEYWORD_COLUMN,    // COLUMN
  KEYWORD_MODIFY,    // MODIFY
  KEYWORD_CONSTRAINT,// CONSTRAINT

  // DML关键字 (DML Keywords)
  KEYWORD_SELECT,    // SELECT
  KEYWORD_INSERT,    // INSERT
  KEYWORD_UPDATE,    // UPDATE
  KEYWORD_DELETE,    // DELETE
  KEYWORD_MERGE,     // MERGE
  KEYWORD_FROM,     // FROM
  KEYWORD_INTO,      // INTO
  KEYWORD_VALUES,    // VALUES
  KEYWORD_SET,       // SET
  KEYWORD_WHERE,     // WHERE
  KEYWORD_GROUP,     // GROUP
  KEYWORD_BY,        // BY
  KEYWORD_HAVING,    // HAVING
  KEYWORD_ORDER,     // ORDER
  KEYWORD_LIMIT,     // LIMIT
  KEYWORD_OFFSET,    // OFFSET
  KEYWORD_DISTINCT,  // DISTINCT
  KEYWORD_ALL,       // ALL
  KEYWORD_AS,        // AS
  KEYWORD_JOIN,      // JOIN
  KEYWORD_INNER,     // INNER
  KEYWORD_LEFT,      // LEFT
  KEYWORD_RIGHT,     // RIGHT
  KEYWORD_FULL,      // FULL
  KEYWORD_OUTER,     // OUTER
  KEYWORD_ON,        // ON
  KEYWORD_USING,     // USING

  // DCL关键字 (DCL Keywords)
  KEYWORD_GRANT,     // GRANT
  KEYWORD_REVOKE,    // REVOKE
  KEYWORD_DENY,      // DENY

  // TCL关键字 (TCL Keywords)
  KEYWORD_BEGIN,     // BEGIN
  KEYWORD_COMMIT,    // COMMIT
  KEYWORD_ROLLBACK,  // ROLLBACK
  KEYWORD_SAVEPOINT, // SAVEPOINT
  KEYWORD_TRANSACTION, // TRANSACTION

  // 逻辑操作符 (Logical Operators)
  KEYWORD_AND,       // AND
  KEYWORD_OR,        // OR
  KEYWORD_IN,        // IN
  KEYWORD_EXISTS,    // EXISTS
  KEYWORD_BETWEEN,   // BETWEEN
  KEYWORD_LIKE,      // LIKE
  KEYWORD_IS,        // IS

  // 集合操作 (Set Operations)
  KEYWORD_UNION,     // UNION
  KEYWORD_INTERSECT, // INTERSECT
  KEYWORD_EXCEPT,    // EXCEPT

  // 控制流 (Control Flow)
  KEYWORD_CASE,      // CASE
  KEYWORD_WHEN,      // WHEN
  KEYWORD_THEN,      // THEN
  KEYWORD_ELSE,      // ELSE
  KEYWORD_END,       // END
  KEYWORD_IF,        // IF
  KEYWORD_WHILE,     // WHILE
  KEYWORD_FOR,       // FOR
  KEYWORD_DO,        // DO

  // 数据库对象 (Database Objects)
  KEYWORD_DATABASE,  // DATABASE
  KEYWORD_TABLE,     // TABLE
  KEYWORD_INDEX,     // INDEX
  KEYWORD_VIEW,      // VIEW
  KEYWORD_SEQUENCE,  // SEQUENCE
  KEYWORD_TRIGGER,   // TRIGGER
  KEYWORD_PROCEDURE, // PROCEDURE
  KEYWORD_FUNCTION,  // FUNCTION

  // 约束 (Constraints)
  KEYWORD_PRIMARY,   // PRIMARY
  KEYWORD_KEY,       // KEY
  KEYWORD_FOREIGN,   // FOREIGN
  KEYWORD_REFERENCES,// REFERENCES
  KEYWORD_UNIQUE,    // UNIQUE
  KEYWORD_CHECK,     // CHECK
  KEYWORD_NOT,       // NOT
  KEYWORD_NULL,      // NULL
  KEYWORD_DEFAULT,   // DEFAULT
  KEYWORD_AUTO_INCREMENT, // AUTO_INCREMENT

  // 数据类型 (Data Types)
  KEYWORD_INT,       // INT
  KEYWORD_INTEGER,   // INTEGER
  KEYWORD_SMALLINT,  // SMALLINT
  KEYWORD_BIGINT,    // BIGINT
  KEYWORD_TINYINT,   // TINYINT
  KEYWORD_VARCHAR,   // VARCHAR
  KEYWORD_CHAR,      // CHAR
  KEYWORD_TEXT,      // TEXT
  KEYWORD_BLOB,      // BLOB
  KEYWORD_CLOB,      // CLOB
  KEYWORD_DECIMAL,   // DECIMAL
  KEYWORD_NUMERIC,   // NUMERIC
  KEYWORD_FLOAT,     // FLOAT
  KEYWORD_DOUBLE,    // DOUBLE
  KEYWORD_REAL,      // REAL
  KEYWORD_DATE,      // DATE
  KEYWORD_TIME,      // TIME
  KEYWORD_TIMESTAMP, // TIMESTAMP
  KEYWORD_DATETIME,  // DATETIME
  KEYWORD_YEAR,      // YEAR
  KEYWORD_BOOLEAN,   // BOOLEAN
  KEYWORD_BOOL,      // BOOL

  // 其他 (Others)
  KEYWORD_USE,       // USE
  KEYWORD_SHOW,      // SHOW
  KEYWORD_DESCRIBE,  // DESCRIBE
  KEYWORD_EXPLAIN,   // EXPLAIN
  KEYWORD_HELP,      // HELP
  KEYWORD_STATUS,    // STATUS
  KEYWORD_ASC,       // ASC
  KEYWORD_DESC,      // DESC
  KEYWORD_USER,      // USER
  KEYWORD_TO,        // TO
  KEYWORD_PRIVILEGES,// PRIVILEGES
  KEYWORD_WITH,      // WITH
  KEYWORD_PASSWORD,  // PASSWORD
  KEYWORD_IDENTIFIED,// IDENTIFIED
  KEYWORD_COLUMNS,   // COLUMNS
  KEYWORD_INDEXES,   // INDEXES
  KEYWORD_GRANTS,    // GRANTS
  KEYWORD_DATABASES, // DATABASES
  KEYWORD_TABLES,    // TABLES

  // LOAD DATA 语句
  KEYWORD_LOAD,      // LOAD
  KEYWORD_DATA,      // DATA
  KEYWORD_INFILE,    // INFILE
  KEYWORD_REPLACE,   // REPLACE
  KEYWORD_IGNORE,    // IGNORE
  KEYWORD_LOW_PRIORITY, // LOW_PRIORITY
  KEYWORD_CONCURRENT, // CONCURRENT
  KEYWORD_LOCAL,     // LOCAL
  KEYWORD_PARTITION, // PARTITION
  KEYWORD_CHARACTER, // CHARACTER
  KEYWORD_FIELDS,    // FIELDS
  KEYWORD_TERMINATED,// TERMINATED
  KEYWORD_OPTIONALLY,// OPTIONALLY
  KEYWORD_ENCLOSED,  // ENCLOSED
  KEYWORD_ESCAPED,   // ESCAPED
  KEYWORD_LINES,     // LINES
  KEYWORD_STARTING,  // STARTING

  // 触发器特定
  KEYWORD_BEFORE,    // BEFORE
  KEYWORD_AFTER,     // AFTER
  KEYWORD_INSTEAD,   // INSTEAD
  KEYWORD_OF,        // OF
  KEYWORD_EACH,      // EACH
  KEYWORD_ROW,       // ROW
  KEYWORD_STATEMENT, // STATEMENT

  // 存储过程特定
  KEYWORD_OUT,       // OUT
  KEYWORD_INOUT,     // INOUT
  KEYWORD_RETURNS,   // RETURNS
  KEYWORD_RETURN,    // RETURN

  // 日期时间函数
  KEYWORD_NOW,       // NOW
  KEYWORD_CURRENT_TIMESTAMP, // CURRENT_TIMESTAMP

  // 特殊Token
  END_OF_INPUT,      // 文件结束
  ERROR,             // 词法错误
  UNKNOWN            // 未知Token
};

/**
 * @brief Token类
 *
 * 表示词法分析结果的词法单元，包含类型、词素和位置信息。
 */
class Token {
public:
  /**
   * @brief 默认构造函数
   */
  Token();

  /**
   * @brief 带参数构造函数
   *
   * @param type Token类型
   * @param lexeme 词素字符串
   * @param line 行号（从1开始）
   * @param column 列号（从1开始）
   */
  Token(Type type, const std::string& lexeme, size_t line, size_t column);

  /**
   * @brief 获取Token类型
   */
  Type getType() const { return type_; }

  /**
   * @brief 获取词素字符串
   */
  const std::string& getLexeme() const { return lexeme_; }

  /**
   * @brief 获取行号
   */
  size_t getLine() const { return line_; }

  /**
   * @brief 获取列号
   */
  size_t getColumn() const { return column_; }

  /**
   * @brief 获取类型名称字符串
   */
  static std::string getTypeName(Type type);

  /**
   * @brief 转换为调试字符串
   */
  std::string toString() const;

private:
  Type type_;           // Token类型
  std::string lexeme_;  // 词素字符串
  size_t line_;         // 行号
  size_t column_;       // 列号
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_H
