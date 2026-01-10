#ifndef SQLCC_WINDOW_FUNCTION_H_H
#define SQLCC_WINDOW_FUNCTION_H_H

#include <memory>
#include <string>
#include <vector>
#include "ast_node.h"

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 窗口函数类型枚举
 */
enum class FunctionType {
    ROW_NUMBER,
    RANK,
    DENSE_RANK,
    SUM,
    AVG,
    COUNT,
    MIN,
    MAX,
    FIRST_VALUE,
    LAST_VALUE,
    LAG,
    LEAD,
    NTH_VALUE
};

/**
 * @brief 窗口帧边界枚举
 */
enum class FrameBoundary {
    UNBOUNDED_PRECEDING,
    CURRENT_ROW,
    UNBOUNDED_FOLLOWING,
    PRECEDING_ROWS,
    FOLLOWING_ROWS
};

/**
 * @brief 窗口函数表达式节点
 */
class WindowFunction : public Expression {
private:
    FunctionType functionType_;
    std::string functionName_;
    std::unique_ptr<Expression> expression_;
    std::unique_ptr<class WindowSpecification> windowSpec_;

public:
    WindowFunction(FunctionType type);
    ~WindowFunction() override = default;

    // 实现基类方法
    std::string getTypeName() const override { return "WindowFunction"; }
    void accept(NodeVisitor& visitor) override;
    Type getType() const override { return FUNCTION; }

    // 窗口函数特定方法
    FunctionType getFunctionType() const;
    const std::string& getFunctionName() const;
    void setExpression(std::unique_ptr<Expression> expr);
    Expression* getExpression() const;
    void setWindowSpecification(std::unique_ptr<class WindowSpecification> spec);
    class WindowSpecification* getWindowSpecification() const;
};

/**
 * @brief 窗口规格类
 */
class WindowSpecification : public Node {
private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    FrameBoundary frameStart_;
    FrameBoundary frameEnd_;

public:
    WindowSpecification();
    ~WindowSpecification() override = default;

    void accept(NodeVisitor& visitor) override { /* 默认实现 */ }

    // 分区相关方法
    void setPartitionBy(std::vector<std::string> columns);
    const std::vector<std::string>& getPartitionBy() const { return partitionByColumns_; }
    bool hasPartitionBy() const;

    // 排序相关方法
    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<std::string>& getOrderBy() const { return orderByColumns_; }
    const std::vector<bool>& getOrderByAscending() const;
    bool hasOrderBy() const;

    // 窗口帧相关方法
    void setFrame(FrameBoundary start, FrameBoundary end);
    FrameBoundary getFrameStart() const;
    FrameBoundary getFrameEnd() const;
    bool hasFrame() const;
};

} // namespace sql_parser

// 向后兼容的别名
using WindowFunction = sql_parser::WindowFunction;
using WindowSpecification = sql_parser::WindowSpecification;

} // namespace sqlcc

#endif // SQLCC_WINDOW_FUNCTION_H_H
