/**
 * ExpressionParser - SQL表达式解析器实现
 *
 * 此文件实现了ExpressionParser类，专门负责SQL表达式的解析。
 * 采用递归下降解析算法，支持运算符优先级和各种SQL表达式类型。
 */

#include "expression_parser.h"
#include "ast/ast_node.h"
#include "ast/literal_expressions.h"
#include "ast/identifier_expression.h"
#include "ast/function_call_expression.h"
#include "token.h"
#include <vector>
#include <iostream>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

ExpressionParser::ExpressionParser(TokenStream& tokens)
    : tokens_(tokens) {
  std::cout << "[EXPRESSION_PARSER] ExpressionParser initialized" << std::endl;
}

std::unique_ptr<Expression> ExpressionParser::parseExpression() {
  std::cout << "[EXPRESSION_PARSER] parseExpression() called" << std::endl;
  return parseLogicalOr();
}

std::unique_ptr<Expression> ExpressionParser::parseLogicalOr() {
  std::cout << "[EXPRESSION_PARSER] parseLogicalOr() called" << std::endl;
  auto expr = parseLogicalAnd();

  while (tokens_.check(Type::KEYWORD_OR)) {
    std::cout << "[EXPRESSION_PARSER] Found OR operator" << std::endl;
    auto right = parseLogicalAnd();
    expr = std::make_unique<BinaryExpression>(
        std::move(expr), std::move(right), TokenType::KEYWORD_OR);
  }

  return expr;
}

std::unique_ptr<Expression> ExpressionParser::parseLogicalAnd() {
  std::cout << "[EXPRESSION_PARSER] parseLogicalAnd() called" << std::endl;
  auto expr = parseEquality();

  while (tokens_.check(Type::KEYWORD_AND)) {
    std::cout << "[EXPRESSION_PARSER] Found AND operator" << std::endl;
    auto right = parseEquality();
    expr = std::make_unique<BinaryExpression>(
        std::move(expr), std::move(right), TokenType::KEYWORD_AND);
  }

  return expr;
}

std::unique_ptr<Expression> ExpressionParser::parseEquality() {
  std::cout << "[EXPRESSION_PARSER] parseEquality() called" << std::endl;
  auto expr = parseComparison();

  std::vector<Type> operators = {
    Type::OPERATOR_EQUAL,
    Type::OPERATOR_NOT_EQUAL,
    Type::OPERATOR_LESS_EQUAL,
    Type::OPERATOR_GREATER_EQUAL,
    Type::OPERATOR_LESS_THAN,
    Type::OPERATOR_GREATER_THAN
  };

  return parseBinaryOp([this]() { return parseComparison(); }, operators);
}

std::unique_ptr<Expression> ExpressionParser::parseComparison() {
  std::cout << "[EXPRESSION_PARSER] parseComparison() called" << std::endl;
  auto expr = parseTerm();

  std::vector<Type> operators = {
    Type::OPERATOR_LESS_THAN,
    Type::OPERATOR_GREATER_THAN,
    Type::OPERATOR_LESS_EQUAL,
    Type::OPERATOR_GREATER_EQUAL
  };

  return parseBinaryOp([this]() { return parseTerm(); }, operators);
}

std::unique_ptr<Expression> ExpressionParser::parseTerm() {
  std::cout << "[EXPRESSION_PARSER] parseTerm() called" << std::endl;
  auto expr = parseFactor();

  std::vector<Type> operators = {
    Type::OPERATOR_PLUS,
    Type::OPERATOR_MINUS
  };

  return parseBinaryOp([this]() { return parseFactor(); }, operators);
}

std::unique_ptr<Expression> ExpressionParser::parseFactor() {
  std::cout << "[EXPRESSION_PARSER] parseFactor() called" << std::endl;
  auto expr = parseUnary();

  std::vector<Type> operators = {
    Type::OPERATOR_MULTIPLY,
    Type::OPERATOR_DIVIDE,
    Type::OPERATOR_MODULO
  };

  return parseBinaryOp([this]() { return parseUnary(); }, operators);
}

std::unique_ptr<Expression> ExpressionParser::parseUnary() {
  std::cout << "[EXPRESSION_PARSER] parseUnary() called" << std::endl;

  if (tokens_.check(Type::OPERATOR_MINUS)) {
    tokens_.expect(Type::OPERATOR_MINUS);
    std::cout << "[EXPRESSION_PARSER] Found unary minus" << std::endl;
    auto right = parseUnary();
    // 简化实现：使用负数字面量代替一元表达式
    if (auto numericExpr = dynamic_cast<NumericLiteralExpression*>(right.get())) {
      return std::make_unique<NumericLiteralExpression>(-numericExpr->getValue());
    } else {
      // 如果不是数字字面量，返回原始表达式
      return right;
    }
  }

  if (tokens_.check(Type::KEYWORD_NOT)) {
    tokens_.expect(Type::KEYWORD_NOT);
    std::cout << "[EXPRESSION_PARSER] Found NOT operator" << std::endl;
    auto right = parseUnary();
    // 简化实现：返回原始表达式（实际应实现NOT逻辑）
    return right;
  }

  return parsePrimary();
}

std::unique_ptr<Expression> ExpressionParser::parsePrimary() {
  std::cout << "[EXPRESSION_PARSER] parsePrimary() called" << std::endl;

  // 字面量
  if (tokens_.check(Type::INTEGER_LITERAL)) {
    tokens_.expect(Type::INTEGER_LITERAL);
    std::cout << "[EXPRESSION_PARSER] Found integer literal" << std::endl;
    return std::make_unique<NumericLiteralExpression>(std::stod(tokens_.current().getLexeme()));
  }

  if (tokens_.check(Type::FLOAT_LITERAL)) {
    tokens_.expect(Type::FLOAT_LITERAL);
    std::cout << "[EXPRESSION_PARSER] Found float literal" << std::endl;
    return std::make_unique<NumericLiteralExpression>(std::stod(tokens_.current().getLexeme()));
  }

  if (tokens_.check(Type::STRING_LITERAL)) {
    tokens_.expect(Type::STRING_LITERAL);
    std::cout << "[EXPRESSION_PARSER] Found string literal" << std::endl;
    std::string value = tokens_.current().getLexeme();
    // 移除引号
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
      value = value.substr(1, value.size() - 2);
    } else if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
      value = value.substr(1, value.size() - 2);
    }
    return std::make_unique<StringLiteralExpression>(value);
  }

  if (tokens_.check(Type::BOOLEAN_LITERAL)) {
    tokens_.expect(Type::BOOLEAN_LITERAL);
    std::cout << "[EXPRESSION_PARSER] Found boolean literal" << std::endl;
    std::string value = tokens_.current().getLexeme();
    if (value == "true" || value == "TRUE") {
      return std::make_unique<BooleanLiteralExpression>(true);
    } else {
      return std::make_unique<BooleanLiteralExpression>(false);
    }
  }

  if (tokens_.check(Type::KEYWORD_NULL)) {
    tokens_.expect(Type::KEYWORD_NULL);
    std::cout << "[EXPRESSION_PARSER] Found NULL literal" << std::endl;
    return std::make_unique<NullLiteralExpression>();
  }

  // 括号表达式
  if (tokens_.check(Type::LPAREN)) {
    tokens_.expect(Type::LPAREN);
    std::cout << "[EXPRESSION_PARSER] Found parenthesized expression" << std::endl;
    auto expr = parseExpression();
    tokens_.expect(Type::RPAREN);
    return expr;
  }

  // 标识符表达式（列名、函数调用等）
  if (tokens_.check(Type::IDENTIFIER)) {
    return parseIdentifierExpression();
  }

  // 如果没有匹配任何模式，抛出异常
  std::stringstream ss;
  ss << "Unexpected token in expression: " << tokens_.peek().getLexeme();
  throw std::runtime_error(ss.str());
}

std::unique_ptr<Expression> ExpressionParser::parseIdentifierExpression() {
  std::cout << "[EXPRESSION_PARSER] parseIdentifierExpression() called" << std::endl;

  tokens_.expect(Type::IDENTIFIER);
  std::string identifier = tokens_.current().getLexeme();
  std::cout << "[EXPRESSION_PARSER] Found identifier: " << identifier << std::endl;

  // 检查是否是函数调用
  if (tokens_.check(Type::LPAREN)) {
    std::cout << "[EXPRESSION_PARSER] Function call detected" << std::endl;
    tokens_.expect(Type::LPAREN);

    std::vector<std::unique_ptr<Expression>> arguments;

    if (!tokens_.check(Type::RPAREN)) {
      arguments.push_back(parseExpression());
      while (tokens_.check(Type::COMMA)) {
        tokens_.expect(Type::COMMA);
        arguments.push_back(parseExpression());
      }
    }

    tokens_.expect(Type::RPAREN);

    return std::make_unique<FunctionCallExpression>(identifier, std::move(arguments));
  } else {
    // 简单标识符（列名或别名）
    return std::make_unique<IdentifierExpression>(identifier);
  }
}

std::unique_ptr<Expression> ExpressionParser::parseBinaryOp(
    std::function<std::unique_ptr<Expression>()> parseNextLevel,
    const std::vector<Type>& operators) {

  auto expr = parseNextLevel();

  for (const auto& op : operators) {
    while (tokens_.check(op)) {
      std::cout << "[EXPRESSION_PARSER] Found binary operator: " << Token::getTypeName(op) << std::endl;
      auto right = parseNextLevel();
      expr = std::make_unique<BinaryExpression>(std::move(expr), std::move(right), static_cast<TokenType>(op));
    }
  }

  return expr;
}

} // namespace sql_parser
} // namespace sqlcc
