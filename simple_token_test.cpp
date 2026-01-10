#include <iostream>
#include <string>
#include <cassert>
#include <cstdint>

// Simple Token class definition for testing
class Token {
public:
  enum Type {
    SEMICOLON,
    LPAREN,
    RPAREN,
    COMMA,
    DOT,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    IDENTIFIER,
    OPERATOR_PLUS,
    OPERATOR_EQUAL,
    KEYWORD_SELECT,
    KEYWORD_FROM,
    KEYWORD_WHERE,
    END_OF_INPUT,
    ERROR,
    UNKNOWN
  };

  Token() : type_(UNKNOWN), line_(0), column_(0) {}
  Token(Type type, const std::string &lexeme, size_t line, size_t column)
      : type_(type), lexeme_(lexeme), line_(line), column_(column) {}

  Type getType() const { return type_; }
  const std::string &getLexeme() const { return lexeme_; }
  size_t getLine() const { return line_; }
  size_t getColumn() const { return column_; }

  static std::string getTypeName(Type type) {
    switch (type) {
      case SEMICOLON: return "SEMICOLON";
      case LPAREN: return "LPAREN";
      case RPAREN: return "RPAREN";
      case COMMA: return "COMMA";
      case DOT: return "DOT";
      case INTEGER_LITERAL: return "INTEGER_LITERAL";
      case FLOAT_LITERAL: return "FLOAT_LITERAL";
      case STRING_LITERAL: return "STRING_LITERAL";
      case BOOLEAN_LITERAL: return "BOOLEAN_LITERAL";
      case NULL_LITERAL: return "NULL_LITERAL";
      case IDENTIFIER: return "IDENTIFIER";
      case OPERATOR_PLUS: return "OPERATOR_PLUS";
      case OPERATOR_EQUAL: return "OPERATOR_EQUAL";
      case KEYWORD_SELECT: return "KEYWORD_SELECT";
      case KEYWORD_FROM: return "KEYWORD_FROM";
      case KEYWORD_WHERE: return "KEYWORD_WHERE";
      case END_OF_INPUT: return "END_OF_INPUT";
      case ERROR: return "ERROR";
      case UNKNOWN: return "UNKNOWN";
      default: return "UNKNOWN_TYPE";
    }
  }

private:
  Type type_;
  std::string lexeme_;
  size_t line_;
  size_t column_;
};

// Test basic Token functionality
void test_token_construction() {
    std::cout << "Testing Token construction..." << std::endl;

    // Test default constructor
    Token token1;
    assert(token1.getType() == Token::UNKNOWN);
    assert(token1.getLexeme().empty());
    assert(token1.getLine() == 0);
    assert(token1.getColumn() == 0);

    // Test parameterized constructor with various inputs
    Token token2(Token::IDENTIFIER, "my_table", 1, 5);
    assert(token2.getType() == Token::IDENTIFIER);
    assert(token2.getLexeme() == "my_table");
    assert(token2.getLine() == 1);
    assert(token2.getColumn() == 5);

    // Test copy construction
    Token token3(token2);
    assert(token3.getType() == Token::IDENTIFIER);
    assert(token3.getLexeme() == "my_table");
    assert(token3.getLine() == 1);
    assert(token3.getColumn() == 5);

    std::cout << "Token construction test passed!" << std::endl;
}

// Test getTypeName static method
void test_token_type_names() {
    std::cout << "Testing Token type names..." << std::endl;

    assert(Token::getTypeName(Token::SEMICOLON) == "SEMICOLON");
    assert(Token::getTypeName(Token::LPAREN) == "LPAREN");
    assert(Token::getTypeName(Token::RPAREN) == "RPAREN");
    assert(Token::getTypeName(Token::COMMA) == "COMMA");
    assert(Token::getTypeName(Token::DOT) == "DOT");

    assert(Token::getTypeName(Token::INTEGER_LITERAL) == "INTEGER_LITERAL");
    assert(Token::getTypeName(Token::FLOAT_LITERAL) == "FLOAT_LITERAL");
    assert(Token::getTypeName(Token::STRING_LITERAL) == "STRING_LITERAL");
    assert(Token::getTypeName(Token::BOOLEAN_LITERAL) == "BOOLEAN_LITERAL");
    assert(Token::getTypeName(Token::NULL_LITERAL) == "NULL_LITERAL");

    assert(Token::getTypeName(Token::IDENTIFIER) == "IDENTIFIER");
    assert(Token::getTypeName(Token::OPERATOR_PLUS) == "OPERATOR_PLUS");
    assert(Token::getTypeName(Token::OPERATOR_EQUAL) == "OPERATOR_EQUAL");

    assert(Token::getTypeName(Token::KEYWORD_SELECT) == "KEYWORD_SELECT");
    assert(Token::getTypeName(Token::KEYWORD_FROM) == "KEYWORD_FROM");
    assert(Token::getTypeName(Token::KEYWORD_WHERE) == "KEYWORD_WHERE");

    assert(Token::getTypeName(Token::END_OF_INPUT) == "END_OF_INPUT");
    assert(Token::getTypeName(Token::ERROR) == "ERROR");
    assert(Token::getTypeName(Token::UNKNOWN) == "UNKNOWN");

    std::cout << "Token type names test passed!" << std::endl;
}

// Test different token types comprehensively
void test_token_types() {
    std::cout << "Testing comprehensive Token types..." << std::endl;

    Token semicolon(Token::SEMICOLON, ";", 1, 10);
    assert(semicolon.getType() == Token::SEMICOLON);
    assert(semicolon.getLexeme() == ";");

    Token lparen(Token::LPAREN, "(", 1, 15);
    assert(lparen.getType() == Token::LPAREN);

    Token int_literal(Token::INTEGER_LITERAL, "42", 2, 5);
    assert(int_literal.getType() == Token::INTEGER_LITERAL);
    assert(int_literal.getLexeme() == "42");

    Token identifier1(Token::IDENTIFIER, "user_name", 3, 8);
    assert(identifier1.getType() == Token::IDENTIFIER);

    Token plus_op(Token::OPERATOR_PLUS, "+", 4, 12);
    assert(plus_op.getType() == Token::OPERATOR_PLUS);

    Token select_kw(Token::KEYWORD_SELECT, "SELECT", 5, 1);
    assert(select_kw.getType() == Token::KEYWORD_SELECT);

    std::cout << "Comprehensive Token types test passed!" << std::endl;
}

// Test Token edge cases and special scenarios
void test_token_edge_cases() {
    std::cout << "Testing Token edge cases..." << std::endl;

    // Test empty lexeme
    Token empty_token(Token::IDENTIFIER, "", 1, 1);
    assert(empty_token.getLexeme().empty());
    assert(empty_token.getType() == Token::IDENTIFIER);

    // Test very long lexeme
    std::string long_lexeme(1000, 'a');
    Token long_token(Token::IDENTIFIER, long_lexeme, 1, 1);
    assert(long_token.getLexeme() == long_lexeme);
    assert(long_token.getLexeme().length() == 1000);

    // Test special characters in lexeme
    Token special_token(Token::STRING_LITERAL, "'hello\nworld\t'", 1, 1);
    assert(special_token.getLexeme() == "'hello\nworld\t'");

    // Test maximum line and column values
    Token max_pos_token(Token::IDENTIFIER, "test", SIZE_MAX, SIZE_MAX);
    assert(max_pos_token.getLine() == SIZE_MAX);
    assert(max_pos_token.getColumn() == SIZE_MAX);

    std::cout << "Token edge cases test passed!" << std::endl;
}

int main() {
    std::cout << "Running comprehensive Token tests..." << std::endl;

    try {
        test_token_construction();
        test_token_type_names();
        test_token_types();
        test_token_edge_cases();

        std::cout << "All comprehensive Token tests passed successfully!" << std::endl;
        std::cout << "Token class coverage: High (constructors, getters, type names, edge cases)" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception!" << std::endl;
        return 1;
    }
}
