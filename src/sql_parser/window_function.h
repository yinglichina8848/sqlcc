#include "sql_parser/token.h"
#ifndef SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
#define SQLCC_SQL_PARSER_WINDOW_FUNCTION_H

#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace parser {

// 前向声明
class ASTNode;
class NodeVisitor;
class Token;

// 窗口函数节点类
class WindowFunctionNode : public ASTNode {
public:
    WindowFunctionNode();
    virtual ~WindowFunctionNode();

    void accept(NodeVisitor& visitor) override;
    std::string to_string() const override;

    // 窗口函数相关属性
    std::string function_name;
    std::vector<std::shared_ptr<ASTNode>> arguments;
    std::string window_name;

    // PARTITION BY 子句
    std::vector<std::shared_ptr<ASTNode>> partition_by;

    // ORDER BY 子句
    std::vector<std::shared_ptr<ASTNode>> order_by;

    // 窗口帧
    std::string frame_type;  // ROWS, RANGE, GROUPS
    std::shared_ptr<ASTNode> frame_start;
    std::shared_ptr<ASTNode> frame_end;
};

// 窗口函数解析器类
class WindowFunctionParser {
public:
    WindowFunctionParser();
    ~WindowFunctionParser();

    std::shared_ptr<WindowFunctionNode> parse_window_function(Token& token);
    std::shared_ptr<WindowFunctionNode> parse_window_specification(Token& token);

private:
    // 辅助解析方法
    std::vector<std::shared_ptr<ASTNode>> parse_partition_by(Token& token);
    std::vector<std::shared_ptr<ASTNode>> parse_order_by(Token& token);
    std::pair<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>> parse_frame_clause(Token& token);
};

} // namespace parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
