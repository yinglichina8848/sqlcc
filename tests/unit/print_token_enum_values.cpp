#include <iostream>
#include "sql_parser/token.h"

int main() {
    std::cout << "Token enum values:" << std::endl;
    
    // 单字符符号 (0-15)
    std::cout << "LPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::LPAREN) << std::endl;
    std::cout << "RPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::RPAREN) << std::endl;
    std::cout << "LEFT_BRACE = " << static_cast<int>(sqlcc::sql_parser::Token::LEFT_BRACE) << std::endl;
    std::cout << "RIGHT_BRACE = " << static_cast<int>(sqlcc::sql_parser::Token::RIGHT_BRACE) << std::endl;
    std::cout << "COMMA = " << static_cast<int>(sqlcc::sql_parser::Token::COMMA) << std::endl;
    std::cout << "DOT = " << static_cast<int>(sqlcc::sql_parser::Token::DOT) << std::endl;
    std::cout << "MINUS = " << static_cast<int>(sqlcc::sql_parser::Token::MINUS) << std::endl;
    std::cout << "PLUS = " << static_cast<int>(sqlcc::sql_parser::Token::PLUS) << std::endl;
    std::cout << "SEMICOLON = " << static_cast<int>(sqlcc::sql_parser::Token::SEMICOLON) << std::endl;
    std::cout << "SLASH = " << static_cast<int>(sqlcc::sql_parser::Token::SLASH) << std::endl;
    std::cout << "EQUAL = " << static_cast<int>(sqlcc::sql_parser::Token::EQUAL) << std::endl;
    std::cout << "NOT_EQUAL = " << static_cast<int>(sqlcc::sql_parser::Token::NOT_EQUAL) << std::endl;
    std::cout << "LESS = " << static_cast<int>(sqlcc::sql_parser::Token::LESS) << std::endl;
    std::cout << "LESS_EQUAL = " << static_cast<int>(sqlcc::sql_parser::Token::LESS_EQUAL) << std::endl;
    std::cout << "GREATER = " << static_cast<int>(sqlcc::sql_parser::Token::GREATER) << std::endl;
    std::cout << "GREATER_EQUAL = " << static_cast<int>(sqlcc::sql_parser::Token::GREATER_EQUAL) << std::endl;
    
    // 字面量 (16-18)
    std::cout << "IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    std::cout << "STRING = " << static_cast<int>(sqlcc::sql_parser::Token::STRING) << std::endl;
    std::cout << "NUMBER = " << static_cast<int>(sqlcc::sql_parser::Token::NUMBER) << std::endl;
    
    // 关键字
    std::cout << "KEYWORD_AND = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AND) << std::endl;
    std::cout << "KEYWORD_BREAK = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_BREAK) << std::endl;
    std::cout << "KEYWORD_CLASS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CLASS) << std::endl;
    std::cout << "KEYWORD_CONTINUE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CONTINUE) << std::endl;
    std::cout << "KEYWORD_DEF = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DEF) << std::endl;
    std::cout << "KEYWORD_DEL = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DEL) << std::endl;
    std::cout << "KEYWORD_DO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DO) << std::endl;
    std::cout << "KEYWORD_ELSE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ELSE) << std::endl;
    std::cout << "KEYWORD_FALSE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FALSE) << std::endl;
    std::cout << "KEYWORD_FOR = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FOR) << std::endl;
    std::cout << "KEYWORD_FROM = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FROM) << std::endl;
    std::cout << "KEYWORD_IF = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IF) << std::endl;
    std::cout << "KEYWORD_IN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IN) << std::endl;
    std::cout << "KEYWORD_IS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IS) << std::endl;
    std::cout << "KEYWORD_NULL = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_NULL) << std::endl;
    std::cout << "KEYWORD_OR = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_OR) << std::endl;
    std::cout << "KEYWORD_PRINT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRINT) << std::endl;
    std::cout << "KEYWORD_RETURN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_RETURN) << std::endl;
    std::cout << "KEYWORD_SUPER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SUPER) << std::endl;
    std::cout << "KEYWORD_THIS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_THIS) << std::endl;
    std::cout << "KEYWORD_TRUE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TRUE) << std::endl;
    std::cout << "KEYWORD_VAR = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VAR) << std::endl;
    std::cout << "KEYWORD_WHILE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_WHILE) << std::endl;
    std::cout << "KEYWORD_SELECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    std::cout << "KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "KEYWORD_UPDATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UPDATE) << std::endl;
    std::cout << "KEYWORD_DELETE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DELETE) << std::endl;
    std::cout << "KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    std::cout << "KEYWORD_DROP = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DROP) << std::endl;
    std::cout << "KEYWORD_ALTER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ALTER) << std::endl;
    std::cout << "KEYWORD_DATABASE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DATABASE) << std::endl;
    std::cout << "KEYWORD_TABLE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TABLE) << std::endl;
    std::cout << "KEYWORD_INDEX = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INDEX) << std::endl;
    std::cout << "KEYWORD_PRIMARY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRIMARY) << std::endl;
    std::cout << "KEYWORD_KEY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_KEY) << std::endl;
    std::cout << "KEYWORD_NOT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_NOT) << std::endl;
    std::cout << "KEYWORD_UNIQUE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UNIQUE) << std::endl;
    std::cout << "KEYWORD_CHECK = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CHECK) << std::endl;
    std::cout << "KEYWORD_DEFAULT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DEFAULT) << std::endl;
    std::cout << "KEYWORD_AUTO_INCREMENT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_AUTO_INCREMENT) << std::endl;
    std::cout << "KEYWORD_REFERENCES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_REFERENCES) << std::endl;
    std::cout << "KEYWORD_FOREIGN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_FOREIGN) << std::endl;
    std::cout << "KEYWORD_USE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_USE) << std::endl;
    std::cout << "KEYWORD_VALUES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    std::cout << "KEYWORD_SET = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SET) << std::endl;
    std::cout << "KEYWORD_WHERE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_WHERE) << std::endl;
    std::cout << "KEYWORD_GROUP = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_GROUP) << std::endl;
    std::cout << "KEYWORD_BY = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_BY) << std::endl;
    std::cout << "KEYWORD_ORDER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ORDER) << std::endl;
    std::cout << "KEYWORD_ASC = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ASC) << std::endl;
    std::cout << "KEYWORD_DESC = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DESC) << std::endl;
    std::cout << "KEYWORD_INTO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTO) << std::endl;
    std::cout << "KEYWORD_USER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_USER) << std::endl;
    std::cout << "KEYWORD_GRANT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_GRANT) << std::endl;
    std::cout << "KEYWORD_REVOKE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_REVOKE) << std::endl;
    std::cout << "KEYWORD_TO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TO) << std::endl;
    std::cout << "KEYWORD_ON = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ON) << std::endl;
    std::cout << "KEYWORD_EXISTS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_EXISTS) << std::endl;
    std::cout << "KEYWORD_JOIN = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_JOIN) << std::endl;
    std::cout << "KEYWORD_HAVING = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_HAVING) << std::endl;
    std::cout << "KEYWORD_CONSTRAINT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CONSTRAINT) << std::endl;
    std::cout << "KEYWORD_PRIVILEGES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PRIVILEGES) << std::endl;
    std::cout << "KEYWORD_WITH = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_WITH) << std::endl;
    std::cout << "KEYWORD_PASSWORD = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_PASSWORD) << std::endl;
    std::cout << "KEYWORD_IDENTIFIED = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_IDENTIFIED) << std::endl;
    std::cout << "KEYWORD_SHOW = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SHOW) << std::endl;
    std::cout << "KEYWORD_COLUMNS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_COLUMNS) << std::endl;
    std::cout << "KEYWORD_INDEXES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INDEXES) << std::endl;
    std::cout << "KEYWORD_GRANTS = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_GRANTS) << std::endl;
    std::cout << "KEYWORD_DATABASES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DATABASES) << std::endl;
    std::cout << "KEYWORD_TABLES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TABLES) << std::endl;
    std::cout << "KEYWORD_ALL = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_ALL) << std::endl;
    std::cout << "KEYWORD_DISTINCT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DISTINCT) << std::endl;
    std::cout << "KEYWORD_UNION = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UNION) << std::endl;
    std::cout << "KEYWORD_INTERSECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTERSECT) << std::endl;
    std::cout << "KEYWORD_EXCEPT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_EXCEPT) << std::endl;
    std::cout << "KEYWORD_LIMIT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_LIMIT) << std::endl;
    std::cout << "KEYWORD_OFFSET = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_OFFSET) << std::endl;
    std::cout << "KEYWORD_OUTER = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_OUTER) << std::endl;
    
    std::cout << "MULTIPLY = " << static_cast<int>(sqlcc::sql_parser::Token::MULTIPLY) << std::endl;
    std::cout << "END_OF_INPUT = " << static_cast<int>(sqlcc::sql_parser::Token::END_OF_INPUT) << std::endl;
    std::cout << "ERROR = " << static_cast<int>(sqlcc::sql_parser::Token::ERROR) << std::endl;
    
    return 0;
}