#include "sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "All Token enum values in token_new.h:" << std::endl;
    
    // Punctuation
    std::cout << "SEMICOLON = " << static_cast<int>(sqlcc::sql_parser::Token::SEMICOLON) << std::endl;
    std::cout << "LPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::LPAREN) << std::endl;
    std::cout << "RPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::RPAREN) << std::endl;
    std::cout << "COMMA = " << static_cast<int>(sqlcc::sql_parser::Token::COMMA) << std::endl;
    std::cout << "COLON = " << static_cast<int>(sqlcc::sql_parser::Token::COLON) << std::endl;
    std::cout << "DOT = " << static_cast<int>(sqlcc::sql_parser::Token::DOT) << std::endl;
    
    // Literals
    std::cout << "INTEGER_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::INTEGER_LITERAL) << std::endl;
    std::cout << "FLOAT_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::FLOAT_LITERAL) << std::endl;
    std::cout << "STRING_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::STRING_LITERAL) << std::endl;
    std::cout << "BOOLEAN_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::BOOLEAN_LITERAL) << std::endl;
    std::cout << "NULL_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::NULL_LITERAL) << std::endl;
    
    // Identifiers
    std::cout << "IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    
    // Database Keywords
    std::cout << "KEYWORD_USE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_USE) << std::endl;
    std::cout << "KEYWORD_DATABASE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DATABASE) << std::endl;
    
    // DDL Keywords
    std::cout << "KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    std::cout << "KEYWORD_ALTER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ALTER) << std::endl;
    std::cout << "KEYWORD_DROP = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DROP) << std::endl;
    std::cout << "KEYWORD_TABLE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TABLE) << std::endl;
    std::cout << "KEYWORD_INDEX = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INDEX) << std::endl;
    std::cout << "KEYWORD_VIEW = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VIEW) << std::endl;
    std::cout << "KEYWORD_TRIGGER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TRIGGER) << std::endl;
    std::cout << "KEYWORD_PROCEDURE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PROCEDURE) << std::endl;
    std::cout << "KEYWORD_FUNCTION = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FUNCTION) << std::endl;
    
    // DML Keywords
    std::cout << "KEYWORD_SELECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    std::cout << "KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "KEYWORD_UPDATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UPDATE) << std::endl;
    std::cout << "KEYWORD_DELETE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DELETE) << std::endl;
    std::cout << "KEYWORD_FROM = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FROM) << std::endl;
    std::cout << "KEYWORD_INTO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTO) << std::endl;
    std::cout << "KEYWORD_VALUES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    std::cout << "KEYWORD_SET = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SET) << std::endl;
    
    // Aggregate Functions
    std::cout << "KEYWORD_COUNT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_COUNT) << std::endl;
    std::cout << "KEYWORD_SUM = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SUM) << std::endl;
    std::cout << "KEYWORD_AVG = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AVG) << std::endl;
    std::cout << "KEYWORD_MIN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_MIN) << std::endl;
    std::cout << "KEYWORD_MAX = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_MAX) << std::endl;
    
    // Constraint Keywords
    std::cout << "KEYWORD_PRIMARY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRIMARY) << std::endl;
    std::cout << "KEYWORD_KEY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_KEY) << std::endl;
    std::cout << "KEYWORD_FOREIGN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FOREIGN) << std::endl;
    std::cout << "KEYWORD_REFERENCES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_REFERENCES) << std::endl;
    std::cout << "KEYWORD_CONSTRAINT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CONSTRAINT) << std::endl;
    std::cout << "KEYWORD_NOT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_NOT) << std::endl;
    std::cout << "KEYWORD_NULL = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_NULL) << std::endl;
    std::cout << "KEYWORD_UNIQUE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UNIQUE) << std::endl;
    std::cout << "KEYWORD_CHECK = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CHECK) << std::endl;
    std::cout << "KEYWORD_DEFAULT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DEFAULT) << std::endl;
    std::cout << "KEYWORD_AUTO_INCREMENT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AUTO_INCREMENT) << std::endl;
    
    // Other Keywords
    std::cout << "KEYWORD_AND = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AND) << std::endl;
    std::cout << "KEYWORD_OR = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_OR) << std::endl;
    std::cout << "KEYWORD_IN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IN) << std::endl;
    std::cout << "KEYWORD_EXISTS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_EXISTS) << std::endl;
    std::cout << "KEYWORD_BETWEEN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_BETWEEN) << std::endl;
    std::cout << "KEYWORD_LIKE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_LIKE) << std::endl;
    std::cout << "KEYWORD_AS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AS) << std::endl;
    std::cout << "KEYWORD_DISTINCT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DISTINCT) << std::endl;
    std::cout << "KEYWORD_ALL = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ALL) << std::endl;
    std::cout << "KEYWORD_UNION = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UNION) << std::endl;
    std::cout << "KEYWORD_INTERSECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTERSECT) << std::endl;
    std::cout << "KEYWORD_EXCEPT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_EXCEPT) << std::endl;
    std::cout << "KEYWORD_LIMIT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_LIMIT) << std::endl;
    std::cout << "KEYWORD_OFFSET = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_OFFSET) << std::endl;
    
    // Permission Keywords
    std::cout << "KEYWORD_GRANT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_GRANT) << std::endl;
    std::cout << "KEYWORD_REVOKE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_REVOKE) << std::endl;
    std::cout << "KEYWORD_TO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TO) << std::endl;
    std::cout << "KEYWORD_WITH = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_WITH) << std::endl;
    std::cout << "KEYWORD_PASSWORD = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PASSWORD) << std::endl;
    std::cout << "KEYWORD_USER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_USER) << std::endl;
    std::cout << "KEYWORD_IDENTIFIED = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IDENTIFIED) << std::endl;
    std::cout << "KEYWORD_PRIVILEGES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRIVILEGES) << std::endl;
    std::cout << "KEYWORD_SHOW = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SHOW) << std::endl;
    
    // Others
    std::cout << "COMMENT = " << static_cast<int>(sqlcc::sql_parser::Token::COMMENT) << std::endl;
    std::cout << "UNKNOWN = " << static_cast<int>(sqlcc::sql_parser::Token::UNKNOWN) << std::endl;
    std::cout << "END_OF_INPUT = " << static_cast<int>(sqlcc::sql_parser::Token::END_OF_INPUT) << std::endl;
    
    return 0;
}