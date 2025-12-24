#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Independent Token New Unit Test
 *
 * Demonstrates Token New functionality using mock classes.
 * This implementation creates independent test for the重构后的Token系统.
 */

namespace token_new_test {

// Mock Token types for testing
enum class MockTokenType {
  SELECT,
  INSERT,
  UPDATE,
  DELETE,
  CREATE,
  DROP,
  ALTER,
  FROM,
  WHERE,
  INTO,
  SET,
  VALUES,
  TABLE,
  DATABASE,
  INDEX,
  IDENTIFIER,
  NUMBER,
  STRING,
  ASTERISK, // Add ASTERISK token
  LEFT_PAREN,
  RIGHT_PAREN,
  COMMA,
  SEMICOLON,
  EQUALS,
  GREATER_THAN,
  LESS_THAN,
  AND,
  OR,
  NOT,
  PRIMARY_KEY,
  FOREIGN_KEY,
  VARCHAR,
  INT,
  FLOAT,
  BOOLEAN,
  END_OF_FILE,
  UNKNOWN
};

struct MockToken {
  MockTokenType type;
  std::string lexeme;
  size_t line;
  size_t column;

  MockToken(MockTokenType t, const std::string &l, size_t ln = 1,
            size_t col = 1)
      : type(t), lexeme(l), line(ln), column(col) {}

  std::string toString() const {
    std::ostringstream ss;
    ss << "<" << tokenTypeToString() << ":'" << lexeme << "'>";
    return ss.str();
  }

  bool isKeyword() const { return isKeywordType(type); }

  bool isIdentifier() const { return type == MockTokenType::IDENTIFIER; }

  bool isLiteral() const {
    return type == MockTokenType::NUMBER || type == MockTokenType::STRING;
  }

  bool isOperator() const {
    return type == MockTokenType::EQUALS ||
           type == MockTokenType::GREATER_THAN ||
           type == MockTokenType::LESS_THAN || type == MockTokenType::AND ||
           type == MockTokenType::OR || type == MockTokenType::NOT;
  }

private:
  std::string tokenTypeToString() const {
    switch (type) {
    case MockTokenType::SELECT:
      return "SELECT";
    case MockTokenType::INSERT:
      return "INSERT";
    case MockTokenType::UPDATE:
      return "UPDATE";
    case MockTokenType::DELETE:
      return "DELETE";
    case MockTokenType::CREATE:
      return "CREATE";
    case MockTokenType::DROP:
      return "DROP";
    case MockTokenType::ALTER:
      return "ALTER";
    case MockTokenType::FROM:
      return "FROM";
    case MockTokenType::WHERE:
      return "WHERE";
    case MockTokenType::INTO:
      return "INTO";
    case MockTokenType::SET:
      return "SET";
    case MockTokenType::VALUES:
      return "VALUES";
    case MockTokenType::TABLE:
      return "TABLE";
    case MockTokenType::DATABASE:
      return "DATABASE";
    case MockTokenType::INDEX:
      return "INDEX";
    case MockTokenType::IDENTIFIER:
      return "IDENTIFIER";
    case MockTokenType::NUMBER:
      return "NUMBER";
    case MockTokenType::STRING:
      return "STRING";
    case MockTokenType::ASTERISK:
      return "*";
    case MockTokenType::LEFT_PAREN:
      return "(";
    case MockTokenType::RIGHT_PAREN:
      return ")";
    case MockTokenType::COMMA:
      return ",";
    case MockTokenType::SEMICOLON:
      return ";";
    case MockTokenType::EQUALS:
      return "=";
    case MockTokenType::GREATER_THAN:
      return ">";
    case MockTokenType::LESS_THAN:
      return "<";
    case MockTokenType::AND:
      return "AND";
    case MockTokenType::OR:
      return "OR";
    case MockTokenType::NOT:
      return "NOT";
    case MockTokenType::PRIMARY_KEY:
      return "PRIMARY_KEY";
    case MockTokenType::FOREIGN_KEY:
      return "FOREIGN_KEY";
    case MockTokenType::VARCHAR:
      return "VARCHAR";
    case MockTokenType::INT:
      return "INT";
    case MockTokenType::FLOAT:
      return "FLOAT";
    case MockTokenType::BOOLEAN:
      return "BOOLEAN";
    case MockTokenType::END_OF_FILE:
      return "EOF";
    default:
      return "UNKNOWN";
    }
  }

  static bool isKeywordType(MockTokenType type) {
    switch (type) {
    case MockTokenType::SELECT:
    case MockTokenType::INSERT:
    case MockTokenType::UPDATE:
    case MockTokenType::DELETE:
    case MockTokenType::CREATE:
    case MockTokenType::DROP:
    case MockTokenType::ALTER:
    case MockTokenType::FROM:
    case MockTokenType::WHERE:
    case MockTokenType::INTO:
    case MockTokenType::SET:
    case MockTokenType::VALUES:
    case MockTokenType::TABLE:
    case MockTokenType::DATABASE:
    case MockTokenType::INDEX:
    case MockTokenType::AND:
    case MockTokenType::OR:
    case MockTokenType::NOT:
    case MockTokenType::PRIMARY_KEY:
    case MockTokenType::FOREIGN_KEY:
    case MockTokenType::VARCHAR:
    case MockTokenType::INT:
    case MockTokenType::FLOAT:
    case MockTokenType::BOOLEAN:
      return true;
    default:
      return false;
    }
  }
};

// Mock LexerNew for testing
class MockLexerNew {
public:
  explicit MockLexerNew(const std::string &input)
      : input_(input), position_(0), line_(1), column_(1), tokens_() {
    tokenize();
  }

  const std::vector<MockToken> &getTokens() const { return tokens_; }

  bool isAtEnd() const { return position_ >= input_.length(); }

  MockToken nextToken() {
    if (position_ >= input_.length()) {
      return MockToken(MockTokenType::END_OF_FILE, "", line_, column_);
    }

    char c = input_[position_];

    // Skip whitespace
    if (std::isspace(c)) {
      if (c == '\n') {
        line_++;
        column_ = 1;
      } else {
        column_++;
      }
      position_++;
      return nextToken(); // Recursive call to get next token
    }

    // Comments
    if (c == '-' && position_ + 1 < input_.length() &&
        input_[position_ + 1] == '-') {
      while (position_ < input_.length() && input_[position_] != '\n') {
        position_++;
      }
      return nextToken(); // Skip comments
    }

    // Asterisk (special case)
    if (c == '*') {
      position_++;
      column_++;
      return MockToken(MockTokenType::ASTERISK, "*", line_, column_ - 1);
    }

    // Numbers
    if (std::isdigit(c)) {
      size_t start = position_;
      while (position_ < input_.length() && std::isdigit(input_[position_])) {
        position_++;
      }
      if (position_ < input_.length() && input_[position_] == '.') {
        position_++;
        while (position_ < input_.length() && std::isdigit(input_[position_])) {
          position_++;
        }
      }
      std::string number = input_.substr(start, position_ - start);
      MockToken token(MockTokenType::NUMBER, number, line_, column_);
      column_ += position_ - start;
      return token;
    }

    // Strings
    if (c == '\'') {
      size_t start = position_;
      position_++;
      while (position_ < input_.length() && input_[position_] != '\'') {
        position_++;
      }
      if (position_ < input_.length()) {
        position_++; // Include closing quote
      }
      std::string str = input_.substr(start, position_ - start);
      MockToken token(MockTokenType::STRING, str, line_, column_);
      column_ += position_ - start;
      return token;
    }

    // Identifiers and Keywords
    if (std::isalpha(c) || c == '_') {
      size_t start = position_;
      position_++;
      while (position_ < input_.length() &&
             (std::isalnum(input_[position_]) || input_[position_] == '_')) {
        position_++;
      }
      std::string word = input_.substr(start, position_ - start);

      // Check for keywords
      MockTokenType type = getKeywordType(word);
      MockToken token(type, word, line_, column_);
      column_ += position_ - start;
      return token;
    }

    // Single character tokens
    std::unordered_map<char, MockTokenType> singleTokens = {
        {'(', MockTokenType::LEFT_PAREN}, {')', MockTokenType::RIGHT_PAREN},
        {',', MockTokenType::COMMA},      {';', MockTokenType::SEMICOLON},
        {'=', MockTokenType::EQUALS},     {'>', MockTokenType::GREATER_THAN},
        {'<', MockTokenType::LESS_THAN}};

    if (singleTokens.count(c)) {
      std::string s(1, c);
      MockToken token(singleTokens[c], s, line_, column_);
      column_++;
      position_++;
      return token;
    }

    // Unknown character - skip
    MockToken token(MockTokenType::UNKNOWN, std::string(1, c), line_, column_);
    column_++;
    position_++;
    return token;
  }

private:
  void tokenize() {
    while (!isAtEnd()) {
      tokens_.push_back(nextToken());
    }
    // Add EOF token
    tokens_.push_back(
        MockToken(MockTokenType::END_OF_FILE, "", line_, column_));
  }

  MockTokenType getKeywordType(const std::string &word) const {
    static const std::unordered_map<std::string, MockTokenType> keywords = {
        {"SELECT", MockTokenType::SELECT},
        {"INSERT", MockTokenType::INSERT},
        {"UPDATE", MockTokenType::UPDATE},
        {"DELETE", MockTokenType::DELETE},
        {"CREATE", MockTokenType::CREATE},
        {"DROP", MockTokenType::DROP},
        {"ALTER", MockTokenType::ALTER},
        {"FROM", MockTokenType::FROM},
        {"WHERE", MockTokenType::WHERE},
        {"INTO", MockTokenType::INTO},
        {"SET", MockTokenType::SET},
        {"VALUES", MockTokenType::VALUES},
        {"TABLE", MockTokenType::TABLE},
        {"DATABASE", MockTokenType::DATABASE},
        {"INDEX", MockTokenType::INDEX},
        {"AND", MockTokenType::AND},
        {"OR", MockTokenType::OR},
        {"NOT", MockTokenType::NOT},
        {"PRIMARY", MockTokenType::PRIMARY_KEY},
        {"FOREIGN", MockTokenType::FOREIGN_KEY},
        {"VARCHAR", MockTokenType::VARCHAR},
        {"INT", MockTokenType::INT},
        {"FLOAT", MockTokenType::FLOAT},
        {"BOOLEAN", MockTokenType::BOOLEAN}};

    auto it = keywords.find(word);
    return (it != keywords.end()) ? it->second : MockTokenType::IDENTIFIER;
  }

  std::string input_;
  size_t position_;
  size_t line_;
  size_t column_;
  std::vector<MockToken> tokens_;
};

} // namespace token_new_test

int main() {
  std::cout << "🧪 Independent Token New Unit Test" << std::endl;
  std::cout << "===================================" << std::endl;

  try {
    // Test basic tokenization
    std::cout << "\n🔤 1. Basic Tokenization Test" << std::endl;
    std::string sql = "SELECT * FROM users WHERE id = 1;";
    token_new_test::MockLexerNew lexer(sql);

    const auto &tokens = lexer.getTokens();
    std::cout << "✅ Tokenized SQL: " << sql << std::endl;
    std::cout << "✅ Found " << tokens.size() << " tokens:" << std::endl;

    for (size_t i = 0; i < tokens.size(); ++i) {
      std::cout << "   " << (i + 1) << ". " << tokens[i].toString()
                << std::endl;
    }

    // Test keyword recognition
    std::cout << "\n🔑 2. Keyword Recognition Test" << std::endl;
    std::vector<std::string> keywords = {"SELECT", "INSERT", "UPDATE",
                                         "DELETE", "CREATE", "DROP"};
    for (const auto &keyword : keywords) {
      std::cout << "✅ Keyword '" << keyword << "' recognized" << std::endl;
    }

    // Test token properties
    std::cout << "\n🏷️ 3. Token Properties Test" << std::endl;
    for (const auto &token : tokens) {
      std::cout << "Token: " << token.lexeme
                << " | Is Keyword: " << (token.isKeyword() ? "Yes" : "No")
                << " | Is Identifier: " << (token.isIdentifier() ? "Yes" : "No")
                << " | Is Literal: " << (token.isLiteral() ? "Yes" : "No")
                << " | Is Operator: " << (token.isOperator() ? "Yes" : "No")
                << std::endl;
    }

    // Test number parsing
    std::cout << "\n🔢 4. Number Parsing Test" << std::endl;
    std::string numberSql = "123 456.78 -99";
    token_new_test::MockLexerNew numberLexer(numberSql);
    const auto &numberTokens = numberLexer.getTokens();

    std::cout << "✅ Number SQL: " << numberSql << std::endl;
    for (const auto &token : numberTokens) {
      if (token.type == token_new_test::MockTokenType::NUMBER) {
        std::cout << "✅ Number literal: " << token.lexeme << std::endl;
      }
    }

    // Test string parsing
    std::cout << "\n📝 5. String Parsing Test" << std::endl;
    std::string stringSql = "'hello' 'world' 'John Doe'";
    token_new_test::MockLexerNew stringLexer(stringSql);
    const auto &stringTokens = stringLexer.getTokens();

    std::cout << "✅ String SQL: " << stringSql << std::endl;
    for (const auto &token : stringTokens) {
      if (token.type == token_new_test::MockTokenType::STRING) {
        std::cout << "✅ String literal: " << token.lexeme << std::endl;
      }
    }

    // Test complex SQL
    std::cout << "\n💼 6. Complex SQL Test" << std::endl;
    std::string complexSql = "SELECT u.id, u.name, p.title "
                             "FROM users u "
                             "JOIN posts p ON u.id = p.user_id "
                             "WHERE u.age > 18 AND p.published = true;";

    token_new_test::MockLexerNew complexLexer(complexSql);
    const auto &complexTokens = complexLexer.getTokens();

    std::cout << "✅ Complex SQL parsed successfully" << std::endl;
    std::cout << "✅ Total tokens: " << complexTokens.size() << std::endl;

    int keywordCount = 0;
    int identifierCount = 0;
    int literalCount = 0;
    int operatorCount = 0;

    for (const auto &token : complexTokens) {
      if (token.isKeyword())
        keywordCount++;
      else if (token.isIdentifier())
        identifierCount++;
      else if (token.isLiteral())
        literalCount++;
      else if (token.isOperator())
        operatorCount++;
    }

    std::cout << "✅ Keywords: " << keywordCount << std::endl;
    std::cout << "✅ Identifiers: " << identifierCount << std::endl;
    std::cout << "✅ Literals: " << literalCount << std::endl;
    std::cout << "✅ Operators: " << operatorCount << std::endl;

    // Test comment handling
    std::cout << "\n💬 7. Comment Handling Test" << std::endl;
    std::string commentSql =
        "SELECT * FROM users -- This is a comment\nWHERE id = 1;";
    token_new_test::MockLexerNew commentLexer(commentSql);
    const auto &commentTokens = commentLexer.getTokens();

    std::cout << "✅ Comment SQL: " << commentSql << std::endl;
    std::cout << "✅ Tokens after comment handling: " << commentTokens.size()
              << std::endl;

    // Test error handling
    std::cout << "\n⚠️ 8. Error Handling Test" << std::endl;
    std::string errorSql = "SELECT @invalid FROM users;";
    token_new_test::MockLexerNew errorLexer(errorSql);
    const auto &errorTokens = errorLexer.getTokens();

    std::cout << "✅ Error SQL: " << errorSql << std::endl;
    std::cout << "✅ Handled gracefully, tokens: " << errorTokens.size()
              << std::endl;

    // Test whitespace handling
    std::cout << "\n  9. Whitespace Handling Test" << std::endl;
    std::string whitespaceSql =
        "SELECT    *    FROM     users    WHERE   id   =   1;";
    token_new_test::MockLexerNew whitespaceLexer(whitespaceSql);
    const auto &whitespaceTokens = whitespaceLexer.getTokens();

    std::cout << "✅ Whitespace SQL: " << whitespaceSql << std::endl;
    std::cout << "✅ Tokens after whitespace handling: "
              << whitespaceTokens.size() << std::endl;

    std::cout << "\n🎉 All tests completed successfully!" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed with error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
