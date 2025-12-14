#include "../../include/sql_parser/token_new.h"
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

// Default constructor
Token::Token() : type_(UNKNOWN), lexeme_(""), line_(0), column_(0) {}

// Parameterized constructor with int parameters (for compatibility)
Token::Token(Type type, const std::string &lexeme, int line, int column)
    : type_(type), lexeme_(lexeme), line_(line), column_(column) {}

// Copy constructor
Token::Token(const Token& other)
    : type_(other.type_), lexeme_(other.lexeme_), line_(other.line_), column_(other.column_) {}

// Assignment operator
Token& Token::operator=(const Token& other) {
    if (this != &other) {
        type_ = other.type_;
        lexeme_ = other.lexeme_;
        line_ = other.line_;
        column_ = other.column_;
    }
    return *this;
}

// Move constructor
Token::Token(Token&& other) noexcept
    : type_(other.type_), lexeme_(std::move(other.lexeme_)), line_(other.line_), column_(other.column_) {
    // Reset the moved-from object to a valid state
    other.type_ = UNKNOWN;
    other.line_ = 0;
    other.column_ = 0;
}

// Move assignment operator
Token& Token::operator=(Token&& other) noexcept {
    if (this != &other) {
        type_ = other.type_;
        lexeme_ = std::move(other.lexeme_);
        line_ = other.line_;
        column_ = other.column_;
        
        // Reset the moved-from object to a valid state
        other.type_ = UNKNOWN;
        other.line_ = 0;
        other.column_ = 0;
    }
    return *this;
}

// Getters
Token::Type Token::getType() const { return type_; }

std::string Token::getLexeme() const { return lexeme_; }

size_t Token::getLine() const { return line_; }

size_t Token::getColumn() const { return column_; }

// Utility function to convert token type to string
std::string Token::getTypeName(Type type) {
  static std::unordered_map<Type, std::string> typeNames;
  
  // Initialize on first use
  if (typeNames.empty()) {
      typeNames[SEMICOLON] = "SEMICOLON";
      typeNames[COLON] = "COLON";
      typeNames[LPAREN] = "LPAREN";
      typeNames[RPAREN] = "RPAREN";
      typeNames[COMMA] = "COMMA";
      typeNames[DOT] = "DOT";

      typeNames[INTEGER_LITERAL] = "INTEGER_LITERAL";
      typeNames[FLOAT_LITERAL] = "FLOAT_LITERAL";
      typeNames[STRING_LITERAL] = "STRING_LITERAL";
      typeNames[BOOLEAN_LITERAL] = "BOOLEAN_LITERAL";
      typeNames[NULL_LITERAL] = "NULL_LITERAL";

      typeNames[IDENTIFIER] = "IDENTIFIER";

      typeNames[KEYWORD_USE] = "KEYWORD_USE";
      typeNames[KEYWORD_DATABASE] = "KEYWORD_DATABASE";

      typeNames[OPERATOR] = "OPERATOR";
      typeNames[OPERATOR_PLUS] = "OPERATOR_PLUS";
      typeNames[OPERATOR_MINUS] = "OPERATOR_MINUS";
      typeNames[OPERATOR_MULTIPLY] = "OPERATOR_MULTIPLY";
      typeNames[OPERATOR_DIVIDE] = "OPERATOR_DIVIDE";
      typeNames[OPERATOR_EQUAL] = "OPERATOR_EQUAL";
      typeNames[OPERATOR_NOT_EQUAL] = "OPERATOR_NOT_EQUAL";
      typeNames[OPERATOR_LESS_THAN] = "OPERATOR_LESS_THAN";
      typeNames[OPERATOR_LESS_EQUAL] = "OPERATOR_LESS_EQUAL";
      typeNames[OPERATOR_GREATER_THAN] = "OPERATOR_GREATER_THAN";
      typeNames[OPERATOR_GREATER_EQUAL] = "OPERATOR_GREATER_EQUAL";
      typeNames[OPERATOR_AND] = "OPERATOR_AND";
      typeNames[OPERATOR_OR] = "OPERATOR_OR";
      typeNames[OPERATOR_NOT] = "OPERATOR_NOT";
      typeNames[OPERATOR_BITWISE_AND] = "OPERATOR_BITWISE_AND";
      typeNames[OPERATOR_BITWISE_OR] = "OPERATOR_BITWISE_OR";
      typeNames[OPERATOR_BITWISE_NOT] = "OPERATOR_BITWISE_NOT";
      typeNames[OPERATOR_BITWISE_XOR] = "OPERATOR_BITWISE_XOR";
      typeNames[OPERATOR_TERNARY] = "OPERATOR_TERNARY";
      typeNames[OPERATOR_AT] = "OPERATOR_AT";
      typeNames[OPERATOR_DOLLAR] = "OPERATOR_DOLLAR";
      typeNames[OPERATOR_CONCAT] = "OPERATOR_CONCAT";
      typeNames[OPERATOR_LIKE] = "OPERATOR_LIKE";
      typeNames[OPERATOR_IN] = "OPERATOR_IN";
      typeNames[OPERATOR_IS] = "OPERATOR_IS";
      typeNames[OPERATOR_BETWEEN] = "OPERATOR_BETWEEN";
      typeNames[OPERATOR_EXISTS] = "OPERATOR_EXISTS";
      typeNames[OPERATOR_MODULO] = "OPERATOR_MODULO";
      typeNames[KEYWORD_CREATE] = "KEYWORD_CREATE";
      typeNames[KEYWORD_ALTER] = "KEYWORD_ALTER";
      typeNames[KEYWORD_DROP] = "KEYWORD_DROP";
      typeNames[KEYWORD_TABLE] = "KEYWORD_TABLE";
      typeNames[KEYWORD_INDEX] = "KEYWORD_INDEX";
      typeNames[KEYWORD_VIEW] = "KEYWORD_VIEW";
      typeNames[KEYWORD_TRIGGER] = "KEYWORD_TRIGGER";
      typeNames[KEYWORD_PROCEDURE] = "KEYWORD_PROCEDURE";
      typeNames[KEYWORD_FUNCTION] = "KEYWORD_FUNCTION";
      typeNames[KEYWORD_ADD] = "KEYWORD_ADD";
      typeNames[KEYWORD_COLUMN] = "KEYWORD_COLUMN";
      typeNames[KEYWORD_MODIFY] = "KEYWORD_MODIFY";
      typeNames[KEYWORD_RENAME] = "KEYWORD_RENAME";
      typeNames[KEYWORD_CONSTRAINT] = "KEYWORD_CONSTRAINT";

      typeNames[KEYWORD_SELECT] = "KEYWORD_SELECT";
      typeNames[KEYWORD_INSERT] = "KEYWORD_INSERT";
      typeNames[KEYWORD_UPDATE] = "KEYWORD_UPDATE";
      typeNames[KEYWORD_DELETE] = "KEYWORD_DELETE";
      typeNames[KEYWORD_FROM] = "KEYWORD_FROM";
      typeNames[KEYWORD_INTO] = "KEYWORD_INTO";
      typeNames[KEYWORD_VALUES] = "KEYWORD_VALUES";
      typeNames[KEYWORD_SET] = "KEYWORD_SET";

      typeNames[KEYWORD_COUNT] = "KEYWORD_COUNT";
      typeNames[KEYWORD_SUM] = "KEYWORD_SUM";
      typeNames[KEYWORD_AVG] = "KEYWORD_AVG";
      typeNames[KEYWORD_MIN] = "KEYWORD_MIN";
      typeNames[KEYWORD_MAX] = "KEYWORD_MAX";

      typeNames[KEYWORD_WHERE] = "KEYWORD_WHERE";
      typeNames[KEYWORD_GROUP] = "KEYWORD_GROUP";
      typeNames[KEYWORD_BY] = "KEYWORD_BY";
      typeNames[KEYWORD_HAVING] = "KEYWORD_HAVING";
      typeNames[KEYWORD_ORDER] = "KEYWORD_ORDER";
      typeNames[KEYWORD_LIMIT] = "KEYWORD_LIMIT";
      typeNames[KEYWORD_OFFSET] = "KEYWORD_OFFSET";

      typeNames[KEYWORD_JOIN] = "KEYWORD_JOIN";
      typeNames[KEYWORD_INNER] = "KEYWORD_INNER";
      typeNames[KEYWORD_LEFT] = "KEYWORD_LEFT";
      typeNames[KEYWORD_RIGHT] = "KEYWORD_RIGHT";
      typeNames[KEYWORD_FULL] = "KEYWORD_FULL";
      typeNames[KEYWORD_OUTER] = "KEYWORD_OUTER";
      typeNames[KEYWORD_ON] = "KEYWORD_ON";
      typeNames[KEYWORD_USING] = "KEYWORD_USING";

      typeNames[KEYWORD_PRIMARY] = "KEYWORD_PRIMARY";
      typeNames[KEYWORD_KEY] = "KEYWORD_KEY";
      typeNames[KEYWORD_FOREIGN] = "KEYWORD_FOREIGN";
      typeNames[KEYWORD_REFERENCES] = "KEYWORD_REFERENCES";
      typeNames[KEYWORD_NOT] = "KEYWORD_NOT";
      typeNames[KEYWORD_NULL] = "KEYWORD_NULL";
      typeNames[KEYWORD_UNIQUE] = "KEYWORD_UNIQUE";
      typeNames[KEYWORD_CHECK] = "KEYWORD_CHECK";
      typeNames[KEYWORD_DEFAULT] = "KEYWORD_DEFAULT";

      typeNames[KEYWORD_AND] = "KEYWORD_AND";
      typeNames[KEYWORD_OR] = "KEYWORD_OR";
      typeNames[KEYWORD_IN] = "KEYWORD_IN";
      typeNames[KEYWORD_OUT] = "KEYWORD_OUT";
      typeNames[KEYWORD_INOUT] = "KEYWORD_INOUT";
      typeNames[KEYWORD_EXISTS] = "KEYWORD_EXISTS";
      typeNames[KEYWORD_BETWEEN] = "KEYWORD_BETWEEN";
      typeNames[KEYWORD_LIKE] = "KEYWORD_LIKE";
      typeNames[KEYWORD_AS] = "KEYWORD_AS";
      typeNames[KEYWORD_DISTINCT] = "KEYWORD_DISTINCT";
      typeNames[KEYWORD_ALL] = "KEYWORD_ALL";
      typeNames[KEYWORD_ANY] = "KEYWORD_ANY";
      typeNames[KEYWORD_SOME] = "KEYWORD_SOME";
      typeNames[KEYWORD_UNION] = "KEYWORD_UNION";
      typeNames[KEYWORD_INTERSECT] = "KEYWORD_INTERSECT";
      typeNames[KEYWORD_EXCEPT] = "KEYWORD_EXCEPT";
      typeNames[KEYWORD_CASE] = "KEYWORD_CASE";
      typeNames[KEYWORD_WHEN] = "KEYWORD_WHEN";
      typeNames[KEYWORD_THEN] = "KEYWORD_THEN";
      typeNames[KEYWORD_ELSE] = "KEYWORD_ELSE";
      typeNames[KEYWORD_END] = "KEYWORD_END";
      typeNames[KEYWORD_IF] = "KEYWORD_IF";
      typeNames[KEYWORD_WHILE] = "KEYWORD_WHILE";
      typeNames[KEYWORD_FOR] = "KEYWORD_FOR";
      typeNames[KEYWORD_DO] = "KEYWORD_DO";
      typeNames[KEYWORD_BEGIN] = "KEYWORD_BEGIN";
      typeNames[KEYWORD_DECLARE] = "KEYWORD_DECLARE";
      typeNames[KEYWORD_CALL] = "KEYWORD_CALL";
      typeNames[KEYWORD_BEFORE] = "KEYWORD_BEFORE";
      typeNames[KEYWORD_AFTER] = "KEYWORD_AFTER";
      typeNames[KEYWORD_INSTEAD_OF] = "KEYWORD_INSTEAD_OF";
      typeNames[KEYWORD_EACH] = "KEYWORD_EACH";
      typeNames[KEYWORD_ROW] = "KEYWORD_ROW";
      typeNames[KEYWORD_STATEMENT] = "KEYWORD_STATEMENT";
      typeNames[KEYWORD_NEW] = "KEYWORD_NEW";
      typeNames[KEYWORD_OLD] = "KEYWORD_OLD";
      typeNames[KEYWORD_COMMIT] = "KEYWORD_COMMIT";
      typeNames[KEYWORD_ROLLBACK] = "KEYWORD_ROLLBACK";
      typeNames[KEYWORD_TRANSACTION] = "KEYWORD_TRANSACTION";
      typeNames[KEYWORD_GRANT] = "KEYWORD_GRANT";
      typeNames[KEYWORD_REVOKE] = "KEYWORD_REVOKE";
      typeNames[KEYWORD_TO] = "KEYWORD_TO";
      typeNames[KEYWORD_WITH] = "KEYWORD_WITH";
      typeNames[KEYWORD_PASSWORD] = "KEYWORD_PASSWORD";
      typeNames[KEYWORD_USER] = "KEYWORD_USER";
      typeNames[KEYWORD_IDENTIFIED] = "KEYWORD_IDENTIFIED";
      typeNames[KEYWORD_PRIVILEGES] = "KEYWORD_PRIVILEGES";
      typeNames[KEYWORD_SHOW] = "KEYWORD_SHOW";
      typeNames[KEYWORD_COLUMNS] = "KEYWORD_COLUMNS";
      typeNames[KEYWORD_INDEXES] = "KEYWORD_INDEXES";
      typeNames[KEYWORD_GRANTS] = "KEYWORD_GRANTS";
      typeNames[KEYWORD_DATABASES] = "KEYWORD_DATABASES";
      typeNames[KEYWORD_TABLES] = "KEYWORD_TABLES";
      typeNames[KEYWORD_TRUE] = "KEYWORD_TRUE";
      typeNames[KEYWORD_FALSE] = "KEYWORD_FALSE";

      typeNames[COMMENT] = "COMMENT";
      typeNames[UNKNOWN] = "UNKNOWN";
      typeNames[END_OF_INPUT] = "END_OF_INPUT";
  }

  auto it = typeNames.find(type);
  if (it != typeNames.end()) {
    return it->second;
  }
  return "UNKNOWN";
}
} // namespace sql_parser
} // namespace sqlcc