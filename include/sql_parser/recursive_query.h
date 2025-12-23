/**
 * @file recursive_query.h
 * @brief 递归查询语句定义
 *
 * 支持WITH RECURSIVE语法，实现递归CTE (Common Table Expression)
 */

#pragma once

#include "ast_node.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

class SelectStatement;

/**
 * @brief WITH RECURSIVE语句
 *
 * 表示递归公共表表达式 (Recursive Common Table Expression)
 * 语法: WITH RECURSIVE cte_name AS (base_query UNION recursive_query) SELECT ...
 */
class WithRecursiveClause : public Statement {
public:
    /**
     * @brief 构造函数
     * @param cte_name CTE名称
     * @param base_query 基础查询
     * @param recursive_query 递归查询
     */
    WithRecursiveClause(std::string cte_name,
                       std::unique_ptr<SelectStatement> base_query,
                       std::unique_ptr<SelectStatement> recursive_query);

    /**
     * @brief 析构函数
     */
    ~WithRecursiveClause();

    /**
     * @brief 获取CTE名称
     * @return CTE名称
     */
    const std::string& getCteName() const;

    /**
     * @brief 获取基础查询
     * @return 基础查询指针
     */
    SelectStatement* getBaseQuery() const;

    /**
     * @brief 获取递归查询
     * @return 递归查询指针
     */
    SelectStatement* getRecursiveQuery() const;

    /**
     * @brief 接受访问者
     * @param visitor 访问者对象
     */
    void accept(NodeVisitor& visitor) override;

private:
    std::string cte_name_;                           ///< CTE名称
    std::unique_ptr<SelectStatement> base_query_;    ///< 基础查询
    std::unique_ptr<SelectStatement> recursive_query_; ///< 递归查询
};

} // namespace sql_parser
} // namespace sqlcc
