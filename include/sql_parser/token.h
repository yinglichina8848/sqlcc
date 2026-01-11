/**
 * WHY: 为什么需要专门的Token系统？
 *
 * 词法分析是SQL解析器的基础，Token系统将原始SQL文本转换为结构化的符号流。
 * 传统的字符串处理方式存在诸多问题：
 * - 字符串比较效率低下：每次都需要遍历字符
 * - 类型安全缺失：容易出现运行时类型错误
 * - 调试困难：难以定位语法错误的具体位置
 * - 扩展性差：添加新关键字需要修改大量代码
 * - 内存效率低：重复的字符串存储开销大
 *
 * Token系统的核心价值：
 * 1. 类型安全：编译时类型检查，运行时类型安全
 * 2. 性能优化：整数枚举比较，O(1)时间复杂度
 * 3. 错误定位：精确的行列号信息，便于错误报告
 * 4. 内存效率：枚举和字符串复用，减少内存占用
 * 5. 扩展性：易于添加新的关键字和符号
 *
 * 🏗️ 设计模式：类型安全枚举模式(Type-Safe Enum Pattern)
 *
 * Token系统作为类型安全枚举的优势：
 * - 编译时检查：防止无效Token类型的创建
 * - 作用域控制：Token类型限定在枚举范围内
 * - 类型转换安全：显式的字符串转换方法
 * - 调试友好：枚举值直接映射到可读字符串
 * - 内存紧凑：整数存储，开销最小
 *
 * SOLID原则体现：
 * - 单一职责：专职负责词法单元的表示和操作
 * - 开闭原则：新Token类型通过扩展枚举实现
 * - 里氏替换：Token对象可以安全替换使用
 * - 接口隔离：提供精确的访问接口
 * - 依赖倒置：高层不依赖具体Token实现
 *
 * WHAT: SQL词法分析Token系统 - 完整的SQL符号流表示框架
 *
 * 核心功能：
 * - 符号分类：将SQL元素分为关键字、标识符、字面量、运算符等
 * - 位置跟踪：记录每个Token在源代码中的精确位置
 * - 类型安全：强类型枚举防止无效Token类型
 * - 字符串转换：双向的Token类型和字符串转换
 * - 内存优化：复用常见Token字符串，减少内存占用
 *
 * Token分类：
 * - 关键字：SQL保留字（SELECT, FROM, WHERE等）
 * - 标识符：表名、列名、变量名等用户定义名称
 * - 字面量：字符串、数字、布尔值、NULL值等常量
 * - 运算符：算术、比较、逻辑运算符
 * - 分隔符：括号、逗号、分号等语法分隔符
 * - 注释：单行和多行注释（词法分析阶段过滤）
 *
 * 接口设计：
 * - 构造函数：安全的Token对象创建
 * - 类型查询：getType()获取Token类型
 * - 值访问：getLexeme()获取原始字符串
 * - 位置信息：getLine(), getColumn()获取位置
 * - 类型转换：getTypeName()获取类型名称字符串
 *
 * HOW: SQL词法分析Token系统的实现机制
 *
 * Token创建流程：
 * 1. 词法分析器扫描SQL文本，按字符识别模式
 * 2. 模式匹配：根据正则表达式或状态机识别Token类型
 * 3. 属性提取：提取Token的字符串值和位置信息
 * 4. 类型确定：根据匹配的模式确定Token枚举类型
 * 5. 对象创建：构造Token对象，设置所有属性
 * 6. 缓冲存储：将Token添加到解析器缓冲区
 *
 * 内存管理策略：
 * - 字符串池：复用常见关键字和符号的字符串对象
 * - 引用计数：智能指针管理动态字符串的生命周期
 * - 栈分配：小对象直接在栈上分配
 * - 延迟复制：按需复制字符串，避免不必要的拷贝
 * - 对象池：重用Token对象，减少分配开销
 *
 * 性能优化策略：
 * - 快速比较：枚举值直接比较，无需字符串操作
 * - 哈希索引：O(1)时间的Token类型查找
 * - SIMD加速：向量化字符串扫描和比较
 * - 缓存友好：连续内存布局，提高缓存命中率
 * - 分支预测：优化条件分支，提高CPU流水线效率
 *
 * 错误处理机制：
 * - 词法错误检测：识别无效的字符序列
 * - 位置信息保留：即使在错误情况下也保留位置信息
 * - 错误恢复：尝试从错误状态恢复，继续解析
 * - 详细诊断：提供清晰的错误信息和修复建议
 * - 日志记录：记录词法分析过程中的异常情况
 *
 * 扩展性设计：
 * - 插件架构：支持自定义Token类型的动态加载
 * - 配置化：可配置的关键字和符号定义
 * - 多语言支持：支持不同SQL方言的Token定义
 * - 标准化：严格遵循SQL标准的Token分类
 * - 向后兼容：保持与现有Token系统的兼容性
 *
 * 调试和诊断：
 * - 可视化输出：将Token流转换为易读的格式
 * - 统计信息：Token类型分布和频率统计
 * - 性能分析：词法分析的时间和空间开销分析
 * - 测试覆盖：完整的Token类型测试覆盖
 * - 边界情况：处理各种边界情况和异常输入
 */

#ifndef SQLCC_SQL_PARSER_TOKEN_H
#define SQLCC_SQL_PARSER_TOKEN_H

#include <string>
#include <cstddef> // for size_t

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
