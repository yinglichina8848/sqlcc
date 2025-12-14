#ifndef SQLCC_SQL_PARSER_WINDOW_FUNCTION_H
#define SQLCC_SQL_PARSER_WINDOW_FUNCTION_H

#include "ast_node.h"
#include "node_visitor.h"
#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace sql_parser {

// 窗口函数节点
class WindowFunction : public Node {
public:
    enum FunctionType {
        ROW_NUMBER,
        RANK,
        DENSE_RANK
    };

    WindowFunction(FunctionType type);
    ~WindowFunction();

    FunctionType getFunctionType() const { return functionType_; }
    const std::string& getAlias() const { return alias_; }
    const std::vector<std::string>& getPartitionByColumns() const { return partitionByColumns_; }
    const std::vector<std::string>& getOrderByColumns() const { return orderByColumns_; }
    const std::string& getOrderDirection() const { return orderDirection_; }

    void setAlias(const std::string& alias) { alias_ = alias; }
    void addPartitionByColumn(const std::string& column) { partitionByColumns_.push_back(column); }
    void addOrderByColumn(const std::string& column) { orderByColumns_.push_back(column); }
    void setOrderDirection(const std::string& direction) { orderDirection_ = direction; }

    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }

private:
    FunctionType functionType_;
    std::string alias_;
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::string orderDirection_;
};

// 窗口规范节点
class WindowSpecification : public Node {
public:
    WindowSpecification();
    ~WindowSpecification();

    const std::vector<std::string>& getPartitionByColumns() const { return partitionByColumns_; }
    const std::vector<std::string>& getOrderByColumns() const { return orderByColumns_; }
    const std::string& getOrderDirection() const { return orderDirection_; }

    void addPartitionByColumn(const std::string& column) { partitionByColumns_.push_back(column); }
    void addOrderByColumn(const std::string& column) { orderByColumns_.push_back(column); }
    void setOrderDirection(const std::string& direction) { orderDirection_ = direction; }

    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }

private:
    std::vector<std::string> partitionByColumns_;
    std::vector<std::string> orderByColumns_;
    std::string orderDirection_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_WINDOW_FUNCTION_H