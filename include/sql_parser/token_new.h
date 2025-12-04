#ifndef SQLCC_SQL_PARSER_TOKEN_NEW_H
#define SQLCC_SQL_PARSER_TOKEN_NEW_H

#include <string>

namespace sqlcc {
namespace sql_parser {

class Token {
public:
    // Token types
    enum Type {
        // Punctuation
        SEMICOLON,      // ;
        LPAREN,         // (
        RPAREN,         // )
        COMMA,          // ,
        DOT,            // .

        // Literals
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        STRING_LITERAL,
        BOOLEAN_LITERAL,
        NULL_LITERAL,

        // Identifiers
        IDENTIFIER,

        // Database Keywords
        KEYWORD_USE,
        KEYWORD_DATABASE,

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
        OPERATOR_AND,
        OPERATOR_OR,
        OPERATOR_NOT,
        OPERATOR_CONCAT,
        OPERATOR_LIKE,
        OPERATOR_IN,
        OPERATOR_IS,
        OPERATOR_BETWEEN,
        OPERATOR_EXISTS,
        
        // DDL Keywords
        KEYWORD_CREATE,
        KEYWORD_ALTER,
        KEYWORD_DROP,
        KEYWORD_TABLE,
        KEYWORD_INDEX,
        KEYWORD_VIEW,
        KEYWORD_TRIGGER,
        KEYWORD_PROCEDURE,
        KEYWORD_FUNCTION,
        
        // DML Keywords
        KEYWORD_SELECT,
        KEYWORD_INSERT,
        KEYWORD_UPDATE,
        KEYWORD_DELETE,
        KEYWORD_FROM,
        KEYWORD_INTO,
        KEYWORD_VALUES,
        KEYWORD_SET,
        
        // Aggregate Functions
        KEYWORD_COUNT,
        KEYWORD_SUM,
        KEYWORD_AVG,
        KEYWORD_MIN,
        KEYWORD_MAX,
        
        // Clause Keywords
        KEYWORD_WHERE,
        KEYWORD_GROUP,
        KEYWORD_BY,
        KEYWORD_HAVING,
        KEYWORD_ORDER,
        KEYWORD_LIMIT,
        KEYWORD_OFFSET,
        
        // Join Keywords
        KEYWORD_JOIN,
        KEYWORD_INNER,
        KEYWORD_LEFT,
        KEYWORD_RIGHT,
        KEYWORD_FULL,
        KEYWORD_OUTER,
        KEYWORD_ON,
        KEYWORD_USING,
        
        // Constraint Keywords
        KEYWORD_PRIMARY,
        KEYWORD_KEY,
        KEYWORD_FOREIGN,
        KEYWORD_REFERENCES,
        KEYWORD_CONSTRAINT,
        KEYWORD_NOT,
        KEYWORD_NULL,
        KEYWORD_UNIQUE,
        KEYWORD_CHECK,
        KEYWORD_DEFAULT,
        KEYWORD_AUTO_INCREMENT,
        
        // Other Keywords
        KEYWORD_AND,
        KEYWORD_OR,
        KEYWORD_IN,
        KEYWORD_EXISTS,
        KEYWORD_BETWEEN,
        KEYWORD_LIKE,
        KEYWORD_AS,
        KEYWORD_DISTINCT,
        KEYWORD_ALL,
        KEYWORD_ANY,
        KEYWORD_SOME,
        KEYWORD_UNION,
        KEYWORD_INTERSECT,
        KEYWORD_EXCEPT,
        KEYWORD_CASE,
        KEYWORD_WHEN,
        KEYWORD_THEN,
        KEYWORD_ELSE,
        KEYWORD_END,
        KEYWORD_IF,
        KEYWORD_WHILE,
        KEYWORD_FOR,
        KEYWORD_DO,
        KEYWORD_BEGIN,
        KEYWORD_COMMIT,
        KEYWORD_ROLLBACK,
        KEYWORD_TRANSACTION,

        // Permission Keywords
        KEYWORD_GRANT,
        KEYWORD_REVOKE,
        KEYWORD_TO,
        KEYWORD_WITH,
        KEYWORD_PASSWORD,
        KEYWORD_USER,
        KEYWORD_IDENTIFIED,
        KEYWORD_PRIVILEGES,
        KEYWORD_SHOW,
        KEYWORD_COLUMNS,
        KEYWORD_INDEXES,
        KEYWORD_GRANTS,
        KEYWORD_DATABASES,
        KEYWORD_TABLES,

        KEYWORD_TRUE,
        KEYWORD_FALSE,
        
        // Comment
        COMMENT,
        
        // Unknown/Error
        UNKNOWN,
        
        // End of input
        END_OF_INPUT
    };
    
    // Constructor
    Token();
    Token(Type type, const std::string& lexeme = "", size_t line = 0, size_t column = 0)
        : type_(type), lexeme_(lexeme), line_(line), column_(column) {}
    ~Token() = default;
    
    // Getters
    Type getType() const;
    std::string getLexeme() const;
    size_t getLine() const;
    size_t getColumn() const;
    
    // Utility
    static std::string getTypeName(Type type);
    
private:
    Type type_;
    std::string lexeme_;
    size_t line_;
    size_t column_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_NEW_H
