#pragma once

#include "token.h"
#include "operator_kind.h"
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief Token到OperatorKind的映射函数
 *
 * 将词法Token转换为语义操作符类型，确保AST语义层独立性。
 *
 * @param type Token类型
 * @return 对应的OperatorKind
 * @throws std::runtime_error 当Token类型不支持映射时
 */
OperatorKind tokenToOperatorKind(Token::Type type);

/**
 * @brief 获取操作符优先级
 *
 * 定义操作符的优先级顺序，用于表达式解析时的递归下降算法。
 * 数字越大优先级越高。
 *
 * @param op 操作符类型
 * @return 优先级数值
 */
int getOperatorPrecedence(OperatorKind op);

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_OPERATOR_MAPPER_H
