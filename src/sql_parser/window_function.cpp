#include "window_function.h"
#include "ast/ast_node.h"
#include "ast/node_visitor.h"
#include "token.h"
#include <algorithm>

namespace sqlcc {
namespace sql_parser {

WindowFunction::WindowFunction(FunctionType type)
    : functionType_(type) {
    switch (type) {
        case FunctionType::ROW_NUMBER:
            functionName_ = "ROW_NUMBER";
            break;
        case FunctionType::RANK:
            functionName_ = "RANK";
            break;
        case FunctionType::DENSE_RANK:
            functionName_ = "DENSE_RANK";
            break;
        case FunctionType::SUM:
            functionName_ = "SUM";
            break;
        case FunctionType::AVG:
            functionName_ = "AVG";
            break;
        case FunctionType::COUNT:
            functionName_ = "COUNT";
            break;
        case FunctionType::MIN:
            functionName_ = "MIN";
            break;
        case FunctionType::MAX:
            functionName_ = "MAX";
            break;
        default:
            functionName_ = "UNKNOWN";
            break;
    }
}

FunctionType WindowFunction::getFunctionType() const {
    return functionType_;
}

const std::string& WindowFunction::getFunctionName() const {
    return functionName_;
}

void WindowFunction::setExpression(std::unique_ptr<Expression> expr) {
    expression_ = std::move(expr);
}

Expression* WindowFunction::getExpression() const {
    return expression_.get();
}

void WindowFunction::setWindowSpecification(std::unique_ptr<WindowSpecification> spec) {
    windowSpec_ = std::move(spec);
}

WindowSpecification* WindowFunction::getWindowSpecification() const {
    return windowSpec_.get();
}

void WindowFunction::accept(NodeVisitor& visitor) {
    visitor.visitWindowFunction(*this);
}



WindowSpecification::WindowSpecification()
    : frameStart_(FrameBoundary::UNBOUNDED_PRECEDING),
      frameEnd_(FrameBoundary::CURRENT_ROW) {
}

void WindowSpecification::setPartitionBy(std::vector<std::string> columns) {
    partitionByColumns_ = std::move(columns);
}



void WindowSpecification::setOrderBy(std::vector<std::string> columns,
                                   std::vector<bool> ascending) {
    orderByColumns_ = std::move(columns);
    orderByAscending_ = std::move(ascending);
}



const std::vector<bool>& WindowSpecification::getOrderByAscending() const {
    return orderByAscending_;
}

void WindowSpecification::setFrame(FrameBoundary start, FrameBoundary end) {
    frameStart_ = start;
    frameEnd_ = end;
}

FrameBoundary WindowSpecification::getFrameStart() const {
    return frameStart_;
}

FrameBoundary WindowSpecification::getFrameEnd() const {
    return frameEnd_;
}

bool WindowSpecification::hasPartitionBy() const {
    return !partitionByColumns_.empty();
}

bool WindowSpecification::hasOrderBy() const {
    return !orderByColumns_.empty();
}

bool WindowSpecification::hasFrame() const {
    return frameStart_ != FrameBoundary::UNBOUNDED_PRECEDING ||
           frameEnd_ != FrameBoundary::CURRENT_ROW;
}

} // namespace sql_parser
} // namespace sqlcc
