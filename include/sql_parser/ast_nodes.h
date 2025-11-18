#ifndef SQLCC_SQL_PARSER_AST_NODES_H
#define SQLCC_SQL_PARSER_AST_NODES_H

#include "ast_node.h"
#include "token.h"
#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace sql_parser {

// ================ 辅助结构（提前定义避免前向声明问题） ================

/**
 * 表引用
 */
class TableReference {
public:
    TableReference(const std::string& name);

    const std::string& getName() const;
    const std::string& getAlias() const;

    void setAlias(const std::string& alias);
    bool hasAlias() const;

private:
    std::string name_;  // 表名
    std::string alias_; // 别名
};

/**
 * 列定义
 */
class ColumnDefinition {
public:
    ColumnDefinition(const std::string& name, const std::string& type);
    ColumnDefinition(ColumnDefinition&&) noexcept = default;
    ColumnDefinition& operator=(ColumnDefinition&&) noexcept = default;

    const std::string& getName() const;
    const std::string& getType() const;

    void setNullable(bool nullable);
    void setDefaultValue(std::unique_ptr<Expression> defaultValue);
    void setPrimaryKey(bool primaryKey);
    void setUnique(bool unique);

    bool isNullable() const;
    bool hasDefaultValue() const;
    const std::unique_ptr<Expression>& getDefaultValue() const;
    bool isPrimaryKey() const;
    bool isUnique() const;

private:
    std::string name_;                     // 列名
    std::string type_;                     // 数据类型
    bool nullable_;                        // 是否可空
    std::unique_ptr<Expression> defaultValue_; // 默认值
    bool primaryKey_;                      // 是否为主键
    bool unique_;                          // 是否唯一
};

/**
 * SELECT项
 */
class SelectItem {
public:
    SelectItem(std::unique_ptr<Expression> expr);
    SelectItem(SelectItem&&) noexcept = default;

    void setAlias(const std::string& alias);

    const std::unique_ptr<Expression>& getExpression() const;
    const std::string& getAlias() const;
    bool hasAlias() const;

private:
    std::unique_ptr<Expression> expr_; // 表达式
    std::string alias_;               // 别名
};

// ================ 语句节点实现 ================

/**
 * CREATE语句节点
 */
class CreateStatement : public Statement {
public:
    /**
     * CREATE目标类型
     */
    enum Target {
        DATABASE,
        TABLE
    };
    
    /**
     * 构造函数
     * @param target 创建目标（数据库或表）
     */
    CreateStatement(Target target);
    
    /**
     * 获取语句类型
     */
    Type getType() const override { return CREATE; }
    
    /**
     * 接受访问者
     */
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setDatabaseName(const std::string& name);
    void setTableName(const std::string& name);
    void addColumn(class ColumnDefinition&& column);
    
    // 获取方法
    Target getTarget() const;
    const std::string& getDatabaseName() const;
    const std::string& getTableName() const;
    const std::vector<class ColumnDefinition>& getColumns() const;
    
private:
    Target target_;                          // 创建目标
    std::string databaseName_;               // 数据库名
    std::string tableName_;                  // 表名
    std::vector<class ColumnDefinition> columns_; // 列定义
};

/**
 * SELECT语句节点
 */
class SelectStatement : public Statement {
public:
    SelectStatement();
    
    Type getType() const override { return SELECT; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setDistinct(bool distinct);
    void addSelectItem(class SelectItem&& item);
    void addFromTable(const class TableReference& table);
    void setWhereClause(std::unique_ptr<WhereClause> clause);
    void addJoinClause(std::unique_ptr<JoinClause> clause);
    void setGroupByClause(std::unique_ptr<GroupByClause> clause);
    void setOrderByClause(std::unique_ptr<OrderByClause> clause);
    void setLimit(int limit);
    void setOffset(int offset);
    
    // 获取方法
    bool isDistinct() const;
    const std::vector<class SelectItem>& getSelectItems() const;
    const std::vector<class TableReference>& getFromTables() const;
    const std::unique_ptr<WhereClause>& getWhereClause() const;
    const std::vector<std::unique_ptr<JoinClause>>& getJoinClauses() const;
    const std::unique_ptr<GroupByClause>& getGroupByClause() const;
    const std::unique_ptr<OrderByClause>& getOrderByClause() const;
    int getLimit() const;
    int getOffset() const;
    
private:
    bool distinct_;                                   // 是否使用DISTINCT
    std::vector<class SelectItem> selectItems_;       // 选择列表
    std::vector<class TableReference> fromTables_;    // FROM子句中的表
    std::unique_ptr<WhereClause> whereClause_;        // WHERE子句
    std::vector<std::unique_ptr<JoinClause>> joinClauses_; // JOIN子句
    std::unique_ptr<GroupByClause> groupByClause_;    // GROUP BY子句
    std::unique_ptr<OrderByClause> orderByClause_;    // ORDER BY子句
    int limit_;                                       // LIMIT值
    int offset_;                                      // OFFSET值
};

/**
 * INSERT语句节点
 */
class InsertStatement : public Statement {
public:
    InsertStatement();
    
    Type getType() const override { return INSERT; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setTableName(const std::string& name);
    void addColumn(const std::string& column);
    void addValueRow(const std::vector<std::unique_ptr<Expression>>& values);
    
    // 获取方法
    const std::string& getTableName() const;
    const std::vector<std::string>& getColumns() const;
    const std::vector<std::vector<std::unique_ptr<Expression>>>& getValueRows() const;
    
private:
    std::string tableName_;                          // 表名
    std::vector<std::string> columns_;               // 列名列表
    std::vector<std::vector<std::unique_ptr<Expression>>> valueRows_; // 值行列表
};

/**
 * UPDATE语句节点
 */
class UpdateStatement : public Statement {
public:
    UpdateStatement();
    
    Type getType() const override { return UPDATE; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setTableName(const std::string& name);
    void addSetItem(const std::string& column, std::unique_ptr<Expression> value);
    void setWhereClause(std::unique_ptr<WhereClause> clause);
    
    // 获取方法
    const std::string& getTableName() const;
    const std::vector<std::pair<std::string, std::unique_ptr<Expression>>>& getSetItems() const;
    const std::unique_ptr<WhereClause>& getWhereClause() const;
    
private:
    std::string tableName_;                             // 表名
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> setItems_; // SET项列表
    std::unique_ptr<WhereClause> whereClause_;          // WHERE子句
};

/**
 * DELETE语句节点
 */
class DeleteStatement : public Statement {
public:
    DeleteStatement();
    
    Type getType() const override { return DELETE; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setTableName(const std::string& name);
    void setWhereClause(std::unique_ptr<WhereClause> clause);
    
    // 获取方法
    const std::string& getTableName() const;
    const std::unique_ptr<WhereClause>& getWhereClause() const;
    
private:
    std::string tableName_;                  // 表名
    std::unique_ptr<WhereClause> whereClause_; // WHERE子句
};

/**
 * DROP语句节点
 */
class DropStatement : public Statement {
public:
    enum Target {
        DATABASE,
        TABLE
    };
    
    DropStatement(Target target);
    
    Type getType() const override { return DROP; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setDatabaseName(const std::string& name);
    void setTableName(const std::string& name);
    void setIfExists(bool ifExists);
    
    // 获取方法
    Target getTarget() const;
    const std::string& getDatabaseName() const;
    const std::string& getTableName() const;
    bool isIfExists() const;
    
private:
    Target target_;          // 删除目标
    std::string databaseName_; // 数据库名
    std::string tableName_;  // 表名
    bool ifExists_;          // 是否包含IF EXISTS
};

/**
 * ALTER语句节点
 */
class AlterStatement : public Statement {
public:
    enum Target {
        DATABASE,
        TABLE
    };
    
    enum Action {
        ADD_COLUMN,
        DROP_COLUMN,
        MODIFY_COLUMN,
        RENAME_TABLE,
        ADD_INDEX,
        DROP_INDEX
    };
    
    AlterStatement(Target target);
    
    Type getType() const override { return ALTER; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setDatabaseName(const std::string& name);
    void setTableName(const std::string& name);
    void setAction(Action action);
    void setColumnName(const std::string& name);
    void setColumnDefinition(class ColumnDefinition&& columnDef);
    void setNewTableName(const std::string& newName);
    void setIndexName(const std::string& indexName);
    
    // 获取方法
    Target getTarget() const;
    Action getAction() const;
    const std::string& getDatabaseName() const;
    const std::string& getTableName() const;
    const std::string& getColumnName() const;
    const class ColumnDefinition& getColumnDefinition() const;
    const std::string& getNewTableName() const;
    const std::string& getIndexName() const;
    
private:
    Target target_;              // 修改目标
    Action action_;              // 修改动作
    std::string databaseName_;   // 数据库名
    std::string tableName_;      // 表名
    std::string columnName_;     // 列名
    class ColumnDefinition columnDef_; // 列定义
    std::string newTableName_;   // 新表名
    std::string indexName_;      // 索引名
};

/**
 * USE语句节点
 */
class UseStatement : public Statement {
public:
    UseStatement();
    
    Type getType() const override { return USE; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setDatabaseName(const std::string& name);
    
    // 获取方法
    const std::string& getDatabaseName() const;
    
private:
    std::string databaseName_; // 数据库名
};

/**
 * CREATE INDEX语句节点
 */
class CreateIndexStatement : public Statement {
public:
    CreateIndexStatement();
    
    Type getType() const override { return CREATE_INDEX; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setIndexName(const std::string& name);
    void setTableName(const std::string& name);
    void setColumnName(const std::string& name);
    void setUnique(bool unique);
    
    // 获取方法
    const std::string& getIndexName() const;
    const std::string& getTableName() const;
    const std::string& getColumnName() const;
    bool isUnique() const;
    
private:
    std::string indexName_;  // 索引名
    std::string tableName_;  // 表名
    std::string columnName_; // 列名
    bool unique_;           // 是否为唯一索引
};

/**
 * DROP INDEX语句节点
 */
class DropIndexStatement : public Statement {
public:
    DropIndexStatement();
    
    Type getType() const override { return DROP_INDEX; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    // 设置方法
    void setIndexName(const std::string& name);
    void setTableName(const std::string& name);
    void setIfExists(bool ifExists);
    
    // 获取方法
    const std::string& getIndexName() const;
    const std::string& getTableName() const;
    bool isIfExists() const;
    
private:
    std::string indexName_;  // 索引名
    std::string tableName_;  // 表名
    bool ifExists_;          // 是否包含IF EXISTS
};

// ================ 表达式节点实现 ================

/**
 * 标识符表达式
 */
class IdentifierExpression : public Expression {
public:
    IdentifierExpression(const std::string& name);
    
    Type getType() const override { return IDENTIFIER; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    const std::string& getName() const;
    void setName(const std::string& name);
    
private:
    std::string name_; // 标识符名称
};

/**
 * 字符串字面量表达式
 */
class StringLiteralExpression : public Expression {
public:
    StringLiteralExpression(const std::string& value);
    
    Type getType() const override { return STRING_LITERAL; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    const std::string& getValue() const;
    
private:
    std::string value_; // 字符串值
};

/**
 * 数值字面量表达式
 */
class NumericLiteralExpression : public Expression {
public:
    NumericLiteralExpression(double value, bool isInteger = false);
    
    Type getType() const override { return NUMERIC_LITERAL; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    double getValue() const;
    bool isInteger() const;
    
private:
    double value_;      // 数值
    bool isInteger_;    // 是否为整数
};

/**
 * 二元表达式
 */
class BinaryExpression : public Expression {
public:
    BinaryExpression(Token::Type op, 
                     std::unique_ptr<Expression> left, 
                     std::unique_ptr<Expression> right);
    
    Type getType() const override { return BINARY; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    Token::Type getOperator() const;
    const std::unique_ptr<Expression>& getLeft() const;
    const std::unique_ptr<Expression>& getRight() const;
    
private:
    Token::Type op_;                     // 操作符
    std::unique_ptr<Expression> left_;   // 左操作数
    std::unique_ptr<Expression> right_;  // 右操作数
};

/**
 * 一元表达式
 */
class UnaryExpression : public Expression {
public:
    UnaryExpression(Token::Type op, std::unique_ptr<Expression> operand);
    
    Type getType() const override { return UNARY; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    Token::Type getOperator() const;
    const std::unique_ptr<Expression>& getOperand() const;
    
private:
    Token::Type op_;                     // 操作符
    std::unique_ptr<Expression> operand_; // 操作数
};

/**
 * 函数表达式
 */
class FunctionExpression : public Expression {
public:
    FunctionExpression(const std::string& name);
    
    Type getType() const override { return FUNCTION; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    void addArgument(std::unique_ptr<Expression> arg);
    const std::string& getName() const;
    const std::vector<std::unique_ptr<Expression>>& getArguments() const;
    
private:
    std::string name_;                         // 函数名
    std::vector<std::unique_ptr<Expression>> arguments_; // 参数列表
};

// ================ 子句节点实现 ================

/**
 * WHERE子句
 */
class WhereClause : public Node {
public:
    WhereClause(std::unique_ptr<Expression> condition);
    
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    const std::unique_ptr<Expression>& getCondition() const;
    void setCondition(std::unique_ptr<Expression> condition);
    
private:
    std::unique_ptr<Expression> condition_; // 条件表达式
};

/**
 * JOIN子句
 */
class JoinClause : public Node {
public:
    enum Type {
        INNER,
        LEFT,
        RIGHT,
        FULL,
        CROSS
    };
    
    JoinClause(Type type, 
               const class TableReference& table, 
               std::unique_ptr<Expression> condition);
    
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    Type getType() const;
    const class TableReference& getTable() const;
    const std::unique_ptr<Expression>& getCondition() const;
    
private:
    Type type_;                              // JOIN类型
    class TableReference table_;             // 连接的表
    std::unique_ptr<Expression> condition_;  // 连接条件
};

/**
 * GROUP BY子句
 */
class GroupByClause : public Node {
public:
    GroupByClause();
    
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    void addGroupByItem(std::unique_ptr<Expression> item);
    void setHavingCondition(std::unique_ptr<Expression> condition);
    
    const std::vector<std::unique_ptr<Expression>>& getGroupByItems() const;
    const std::unique_ptr<Expression>& getHavingCondition() const;
    bool hasHaving() const;
    
private:
    std::vector<std::unique_ptr<Expression>> groupByItems_; // GROUP BY表达式列表
    std::unique_ptr<Expression> havingCondition_;          // HAVING条件
};

/**
 * ORDER BY子句
 */
class OrderByClause : public Node {
public:
    /**
     * 排序方向
     */
    enum Direction {
        ASC,
        DESC
    };
    
    OrderByClause();
    
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    void addOrderByItem(std::unique_ptr<Expression> expr, Direction direction);
    
    const std::vector<std::pair<std::unique_ptr<Expression>, Direction>>& getOrderByItems() const;
    
private:
    std::vector<std::pair<std::unique_ptr<Expression>, Direction>> orderByItems_; // ORDER BY项列表
};

// 辅助结构已在文件开头定义

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODES_H