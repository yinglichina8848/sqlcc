#include <iostream>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

// DFA State definitions
enum class LexerState {
  START,              // Initial state
  IDENTIFIER,         // Identifier state
  NUMBER,             // Number literal state
  NUMBER_DECIMAL,     // Decimal part of number
  NUMBER_EXPONENT,    // Exponent part of number
  STRING_SINGLE,      // Single-quoted string
  STRING_DOUBLE,      // Double-quoted identifier
  STRING_ESCAPE,      // Escape sequence in string
  COMMENT_LINE,       // Single-line comment (--)
  COMMENT_BLOCK,      // Multi-line comment (/* */)
  COMMENT_BLOCK_STAR, // In block comment, saw *
  OPERATOR,           // Operator state
  PUNCTUATION,        // Punctuation state
  ERROR               // Error state
};

// Transition table type
using TransitionMap =
    std::unordered_map<LexerState, std::unordered_map<char, LexerState>>;

void debugSetupTransitionTable(TransitionMap& transitions_) {
  std::cout << "Starting setupTransitionTable" << std::endl;
  
  // Clear existing transitions
  std::cout << "Clearing transitions" << std::endl;
  transitions_.clear();

  // Define character sets for transitions
  const std::string operators = "+-*/%=<>!&|^~";
  const std::string punctuation = "();:,{}[]@#$";

  std::cout << "Setting up START state transitions" << std::endl;
  // START state transitions
  for (char c = 'a'; c <= 'z'; c++) {
    transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
  }
  transitions_[LexerState::START]['_'] = LexerState::IDENTIFIER;
  // Unicode support for identifiers
  for (int c = 128; c <= 255; c++) {
    transitions_[LexerState::START][static_cast<char>(c)] = LexerState::IDENTIFIER;
  }

  std::cout << "Setting up NUMBER transitions" << std::endl;
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::START][c] = LexerState::NUMBER;
  }

  transitions_[LexerState::START]['\''] = LexerState::STRING_SINGLE;
  transitions_[LexerState::START]['"'] = LexerState::STRING_DOUBLE;

  for (char op : operators) {
    transitions_[LexerState::START][op] = LexerState::OPERATOR;
  }

  for (char punct : punctuation) {
    transitions_[LexerState::START][punct] = LexerState::PUNCTUATION;
  }

  transitions_[LexerState::START]['-'] = LexerState::COMMENT_LINE; // Could be comment start

  std::cout << "Setting up IDENTIFIER transitions" << std::endl;
  // IDENTIFIER state transitions
  for (char c = 'a'; c <= 'z'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
  }
  transitions_[LexerState::IDENTIFIER]['_'] = LexerState::IDENTIFIER;
  // Unicode support
  for (int c = 128; c <= 255; c++) {
    transitions_[LexerState::IDENTIFIER][static_cast<char>(c)] = LexerState::IDENTIFIER;
  }

  std::cout << "Setting up NUMBER state transitions" << std::endl;
  // NUMBER state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER][c] = LexerState::NUMBER;
  }
  transitions_[LexerState::NUMBER]['.'] = LexerState::NUMBER_DECIMAL;
  transitions_[LexerState::NUMBER]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER]['E'] = LexerState::NUMBER_EXPONENT;

  // NUMBER_DECIMAL state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER_DECIMAL][c] = LexerState::NUMBER_DECIMAL;
  }
  transitions_[LexerState::NUMBER_DECIMAL]['e'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_DECIMAL]['E'] = LexerState::NUMBER_EXPONENT;

  // NUMBER_EXPONENT state transitions
  for (char c = '0'; c <= '9'; c++) {
    transitions_[LexerState::NUMBER_EXPONENT][c] = LexerState::NUMBER_EXPONENT;
  }
  transitions_[LexerState::NUMBER_EXPONENT]['+'] = LexerState::NUMBER_EXPONENT;
  transitions_[LexerState::NUMBER_EXPONENT]['-'] = LexerState::NUMBER_EXPONENT;

  std::cout << "Setting up STRING transitions" << std::endl;
  // STRING_SINGLE state transitions
  transitions_[LexerState::STRING_SINGLE]['\''] = LexerState::START; // End of string
  transitions_[LexerState::STRING_SINGLE]['\\'] = LexerState::STRING_ESCAPE;
  // Everything else stays in STRING_SINGLE

  // STRING_DOUBLE state transitions
  transitions_[LexerState::STRING_DOUBLE]['"'] = LexerState::START; // End of identifier
  transitions_[LexerState::STRING_DOUBLE]['\\'] = LexerState::STRING_ESCAPE;
  // Everything else stays in STRING_DOUBLE

  // STRING_ESCAPE state transitions (return to respective string state)
  transitions_[LexerState::STRING_ESCAPE][static_cast<char>(-1)] = LexerState::STRING_SINGLE;

  // OPERATOR state transitions
  // Most operators are single character, but some can be multi-character
  // We'll handle multi-character operators in nextToken()

  // PUNCTUATION state transitions
  // All punctuation is single character

  // COMMENT_LINE state transitions
  transitions_[LexerState::COMMENT_LINE]['-'] = LexerState::COMMENT_LINE; // Second -
  // Everything else will be handled in nextToken()

  // COMMENT_BLOCK state transitions
  transitions_[LexerState::COMMENT_BLOCK]['*'] = LexerState::COMMENT_BLOCK_STAR;
  // Everything else stays in COMMENT_BLOCK

  // COMMENT_BLOCK_STAR state transitions
  transitions_[LexerState::COMMENT_BLOCK_STAR]['/'] = LexerState::START; // End of comment
  transitions_[LexerState::COMMENT_BLOCK_STAR]['*'] = LexerState::COMMENT_BLOCK_STAR; // Another *
  // Everything else goes back to COMMENT_BLOCK
  std::cout << "Setting up COMMENT_BLOCK_STAR transitions" << std::endl;
  for (char c = 0; c < 128; c++) {
    if (c != '/' && c != '*') {
      transitions_[LexerState::COMMENT_BLOCK_STAR][c] = LexerState::COMMENT_BLOCK;
    }
  }
  
  std::cout << "Finished setupTransitionTable" << std::endl;
}

} // namespace sql_parser
} // namespace sqlcc

int main() {
    std::cout << "Starting debug setup test" << std::endl;
    
    sqlcc::sql_parser::TransitionMap transitions_;
    sqlcc::sql_parser::debugSetupTransitionTable(transitions_);
    
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}