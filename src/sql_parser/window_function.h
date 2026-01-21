#ifndef SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
#define SQLCC_SQL_PARSER_WINDOW_FUNCTION_H

#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace sql_parser {

class ASTNode;
class Expression;

// 窗口函数类型枚举
enum class FunctionType {
    ROW_NUMBER,
    RANK,
    DENSE_RANK,
    SUM,
    AVG,
    COUNT,
    MIN,
    MAX,
    UNKNOWN
};

// 窗口帧边界枚举
enum class FrameBoundary {
    UNBOUNDED_PRECEDING,
    PRECEDING,
    CURRENT_ROW,
    FOLLOWING,
    UNBOUNDED_FOLLOWING
};

// 窗口函数类
class WindowFunction {
public:
    WindowFunction(FunctionType type);
    ~WindowFunction();

    FunctionType getFunctionType() const;
    const std::string& getFunctionName() const;

    void setExpression(std::unique_ptr<Expression> expr);
    Expression* getExpression() const;

    void setWindowSpecification(std::unique_ptr<class WindowSpecification> spec);
    class WindowSpecification* getWindowSpecification() const;

private:
    FunctionType functionType_;
    std::string functionName_;
    std::unique_ptr<Expression> expression_;
    std::unique_ptr<class WindowSpecification> windowSpec_;
};

// 窗口规范类
class WindowSpecification {
public:
    WindowSpecification();
    ~WindowSpecification();

    void setPartitionBy(std::vector<std::string> columns);
    const std::vector<std::string>& getPartitionBy() const;

    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<std::string>& getOrderBy() const;
    const std::vector<bool>& getOrderByAscending() const;

    void setFrame(FrameBoundary start, FrameBoundary end);
    FrameBoundary getFrameStart() const;
    FrameBoundary getFrameEnd() const;

    bool hasPartitionBy() const;
    bool hasOrderBy() const;
    bool hasFrame() const;

private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    FrameBoundary frameStart_;
    FrameBoundary frameEnd_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
