#include "src/sql_parser/token_stream.h"
#include <sstream>

namespace sqlcc {
namespace sql_parser {

TokenStream::TokenStream(Lexer& lexer)
    : lexer_(lexer), hasLookahead_(false) {
    // 获取第一个token
    currentToken_ = lexer_.nextToken();
    previousToken_ = currentToken_; // 初始时前一个token与当前token相同
}

const Token& TokenStream::current() const {
    return currentToken_;
}

const Token& TokenStream::peek() {
    if (!hasLookahead_) {
        lookaheadToken_ = lexer_.nextToken();
        hasLookahead_ = true;
    }
    return lookaheadToken_;
}

const Token& TokenStream::previous() const {
    return previousToken_;
}

void TokenStream::advance() {
    previousToken_ = currentToken_;
    if (hasLookahead_) {
        currentToken_ = lookaheadToken_;
        hasLookahead_ = false;
    } else {
        currentToken_ = lexer_.nextToken();
    }
}

bool TokenStream::check(Type type) const {
    if (isAtEnd() && type != Type::END_OF_INPUT) {
        return false;
    }
    return currentToken_.getType() == type;
}

bool TokenStream::match(Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void TokenStream::expect(Type type, const std::string& message) {
    if (!check(type)) {
        std::stringstream ss;
        ss << "Expected token " << Token::getTypeName(type) << " but got "
           << Token::getTypeName(currentToken_.getType()) << " ("
           << currentToken_.getLexeme() << ")";
        if (!message.empty()) {
            ss << ": " << message;
        }
        throw std::runtime_error(ss.str());
    }
    advance();
}

bool TokenStream::isAtEnd() const {
    return currentToken_.getType() == Type::END_OF_INPUT;
}

} // namespace sql_parser
} // namespace sqlcc