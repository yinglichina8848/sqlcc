#ifndef SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
#define SQLCC_SQL_PARSER_WINDOW_FUNCTION_H

#include "ast_node.h"
#include "node_visitor.h"
#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace sql_parser {

// 前向声明
class Expression;
class WindowSpecification;

// 窗口函数节点
class WindowFunction : public Node {
public:
    enum FunctionType {
        ROW_NUMBER,
        RANK,
        DENSE_RANK,
        SUM,
        AVG,
        COUNT,
        MIN,
        MAX
    };

    WindowFunction(FunctionType type);
    ~WindowFunction();

    FunctionType getFunctionType() const;
    const std::string& getFunctionName() const;
    const std::string& getAlias() const { return alias_; }
    const std::vector<std::string>& getPartitionByColumns() const { return partitionByColumns_; }
    const std::vector<std::string>& getOrderByColumns() const { return orderByColumns_; }
    const std::string& getOrderDirection() const { return orderDirection_; }

    void setExpression(std::unique_ptr<Expression> expr);
    Expression* getExpression() const;
    void setWindowSpecification(std::unique_ptr<WindowSpecification> spec);
    WindowSpecification* getWindowSpecification() const;

    void setAlias(const std::string& alias) { alias_ = alias; }
    void addPartitionByColumn(const std::string& column) { partitionByColumns_.push_back(column); }
    void addOrderByColumn(const std::string& column) { orderByColumns_.push_back(column); }
    void setOrderDirection(const std::string& direction) { orderDirection_ = direction; }

    void accept(NodeVisitor& visitor) override;

private:
    FunctionType functionType_;
    std::string functionName_;
    std::string alias_;
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::string orderDirection_;
    std::unique_ptr<Expression> expression_;
    std::unique_ptr<WindowSpecification> windowSpec_;
};

// 窗口边界类型
enum class FrameBoundary {
    UNBOUNDED_PRECEDING,
    CURRENT_ROW,
    UNBOUNDED_FOLLOWING
};

// 窗口规范节点
class WindowSpecification : public Node {
public:
    WindowSpecification();
    ~WindowSpecification();

    const std::vector<std::string>& getPartitionByColumns() const { return partitionByColumns_; }
    const std::vector<std::string>& getOrderByColumns() const { return orderByColumns_; }
    const std::string& getOrderDirection() const { return orderDirection_; }

    void setPartitionBy(std::vector<std::string> columns);
    const std::vector<std::string>& getPartitionBy() const { return partitionByColumns_; }
    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<bool>& getOrderByAscending() const;
    void setFrame(FrameBoundary start, FrameBoundary end);
    FrameBoundary getFrameStart() const;
    FrameBoundary getFrameEnd() const;
    bool hasPartitionBy() const;
    bool hasOrderBy() const;
    bool hasFrame() const;

    void addPartitionByColumn(const std::string& column) { partitionByColumns_.push_back(column); }
    void addOrderByColumn(const std::string& column) { orderByColumns_.push_back(column); }
    void setOrderDirection(const std::string& direction) { orderDirection_ = direction; }

    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }

private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::string orderDirection_;
    std::vector<bool> orderByAscending_;
    FrameBoundary frameStart_;
    FrameBoundary frameEnd_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_WINDOW_FUNCTION_H