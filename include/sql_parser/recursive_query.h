#pragma once

#include "ast_node.h"
#include "node_visitor.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

// Forward declarations
class SelectStatement;

/**
 * WITH RECURSIVE子句类
 * 表示递归查询的定义
 */
class WithRecursiveClause : public Statement {
public:
    /**
     * 构造函数
     * @param cte_name 公共表表达式名称
     * @param base_query 基础查询（非递归部分）
     * @param recursive_query 递归查询（递归部分）
     */
    WithRecursiveClause(std::string cte_name,
                       std::unique_ptr<SelectStatement> base_query,
                       std::unique_ptr<SelectStatement> recursive_query);

    ~WithRecursiveClause();

    // Getters
    const std::string& getCteName() const;
    SelectStatement* getBaseQuery() const;
    SelectStatement* getRecursiveQuery() const;

    // Node interface
    void accept(NodeVisitor& visitor) override;

private:
    std::string cte_name_;
    std::unique_ptr<SelectStatement> base_query_;
    std::unique_ptr<SelectStatement> recursive_query_;
};

} // namespace sql_parser
} // namespace sqlcc
