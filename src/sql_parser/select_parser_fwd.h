/**
 * SelectParser 前向声明文件
 * 
 * 为了解决循环依赖问题，提供SelectParser类的前向声明。
 * 其他模块只需要知道SelectParser类存在，而不需要知道其完整定义时，
 * 可以包含此文件而不是select_parser.h。
 */

#ifndef SQLCC_SQL_PARSER_SELECT_PARSER_FWD_H
#define SQLCC_SQL_PARSER_SELECT_PARSER_FWD_H

namespace sqlcc {
namespace sql_parser {

// 前向声明
class SelectParser;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_SELECT_PARSER_FWD_H