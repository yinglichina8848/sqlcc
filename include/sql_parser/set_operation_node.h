#ifndef SQLCC_SQL_PARSER_SET_OPERATION_NODE_H
#define SQLCC_SQL_PARSER_SET_OPERATION_NODE_H

#include "ast_node.h"
#include "node_visitor.h"
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
class SetOperationNode : public Statement {
public:
    /**
     * 构造函数
     * @param operationType 集合操作类型
     * @param leftOperand 左操作数（Select语句）
     * @param rightOperand 右操作数（Select语句）
     * @param allFlag 是否包含ALL关键字（默认为false）
     */
    SetOperationNode(SetOperationType operationType, 
                    std::unique_ptr<SelectStatement> leftOperand,
                    std::unique_ptr<SelectStatement> rightOperand,
                    bool allFlag = false);
    
    ~SetOperationNode();

    // Getters
    SetOperationType getOperationType() const;
    const std::string& getOperationName() const;
    SelectStatement* getLeftOperand() const;
    SelectStatement* getRightOperand() const;
    bool isAll() const;
    
    // 获取操作类型名称（用于调试和序列化）
    static std::string getOperationTypeName(SetOperationType type);
    
    // Visitor模式支持
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    SetOperationType operationType_;
    std::unique_ptr<SelectStatement> leftOperand_;
    std::unique_ptr<SelectStatement> rightOperand_;
    bool allFlag_;
};

/**
 * 复合Select语句类
 * 扩展SelectStatement以支持嵌套的集合操作
 */
class CompositeSelectStatement : public Statement {
public:
    CompositeSelectStatement();
    ~CompositeSelectStatement();
    
    /**
     * 添加基础Select语句
     * @param selectStmt Select语句
     */
    void addSelectStatement(std::unique_ptr<SelectStatement> selectStmt);
    
    /**
     * 添加集合操作
     * @param setOperation 集合操作节点
     */
    void addSetOperation(std::unique_ptr<SetOperationNode> setOperation);
    
    /**
     * 获取所有Select语句
     */
    const std::vector<std::unique_ptr<SelectStatement>>& getSelectStatements() const;
    
    /**
     * 获取所有集合操作
     */
    const std::vector<std::unique_ptr<SetOperationNode>>& getSetOperations() const;
    
    /**
     * 检查是否为简单Select语句（无集合操作）
     */
    bool isSimpleSelect() const;
    
    /**
     * 检查是否包含集合操作
     */
    bool hasSetOperations() const;
    
    /**
     * 获取语句数量
     */
    size_t getStatementCount() const;
    
    /**
     * 获取操作数量
     */
    size_t getOperationCount() const;
    
    // Visitor模式支持
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::vector<std::unique_ptr<SelectStatement>> selectStatements_;
    std::vector<std::unique_ptr<SetOperationNode>> setOperations_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_SET_OPERATION_NODE_H