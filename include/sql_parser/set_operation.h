#include "sql_parser/ast_nodes.h"
#ifndef SQLCC_SQL_PARSER_SET_OPERATION_H
#define SQLCC_SQL_PARSER_SET_OPERATION_H

#include "sql_parser/ast_node.h"
#include "sql_parser/node_visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// Forward declarations
class SelectStatement;

/**
 * 集合操作类型枚举
 */
enum class SetOperationType {
    UNION,      // UNION操作
    INTERSECT,  // INTERSECT操作
    EXCEPT      // EXCEPT操作
};

/**
 * 集合操作节点类
 * 表示包含集合操作的复合查询语句
 */
class SetOperation : public Statement {
public:
    /**
     * 构造函数
     * @param operationType 集合操作类型
     * @param leftOperand 左操作数（Select语句）
     * @param rightOperand 右操作数（Select语句）
     * @param allFlag 是否包含ALL关键字（默认为false）
     */
    SetOperation(SetOperationType operationType, 
                 std::unique_ptr<SelectStatement> leftOperand,
                 std::unique_ptr<SelectStatement> rightOperand,
                 bool allFlag = false);
    
    ~SetOperation();

    // Getters
    SetOperationType getOperationType() const;
    const std::string& getOperationName() const;
    SelectStatement* getLeftOperand() const;
    SelectStatement* getRightOperand() const;
    bool isAll() const;

    // ORDER BY support
    void setOrderBy(std::vector<std::string> columns, std::vector<bool> ascending);
    const std::vector<std::string>& getOrderByColumns() const;
    const std::vector<bool>& getOrderByAscending() const;
    bool hasOrderBy() const;

    // LIMIT support
    void setLimit(size_t limit);
    size_t getLimit() const;
    bool hasLimit() const;

    // Node interface
    void accept(NodeVisitor& visitor) override;

private:
    SetOperationType operationType_;
    std::unique_ptr<SelectStatement> leftOperand_;
    std::unique_ptr<SelectStatement> rightOperand_;
    bool allFlag_;
    std::string operationName_;
    
    // ORDER BY support
    std::vector<std::string> orderByColumns_;
    std::vector<bool> orderByAscending_;
    
    // LIMIT support
    size_t limit_;
    bool hasLimit_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_SET_OPERATION_H
