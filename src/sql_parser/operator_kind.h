#pragma once

#ifndef SQLCC_SQL_PARSER_OPERATOR_KIND_H
#define SQLCC_SQL_PARSER_OPERATOR_KIND_H

namespace sqlcc {
namespace sql_parser {

/**
 * @brief OperatorKind - 语义操作符类型枚举
 *
 * 定义SQL表达式中的操作符语义类型，与Token::Type分离。
 * 确保AST的语义独立性，操作符优先级和行为由AST语义层决定。
 */
enum class OperatorKind {
  // Arithmetic operators (算术运算符)
  Add,      // +
  Subtract, // -
  Multiply, // *
  Divide,   // /

  // Comparison operators (比较运算符)
  Equal,        // =
  NotEqual,     // != 或 <>
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=

  // Logical operators (逻辑运算符)
  And, // AND
  Or,  // OR

  // Unary operators (一元运算符)
  Negate, // - (负号)
  Not     // NOT
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_OPERATOR_KIND_H
