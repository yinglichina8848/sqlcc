#ifndef SQLCC_SQL_PARSER_DDL_PARSER_H
#define SQLCC_SQL_PARSER_DDL_PARSER_H

#include "ast/ast_node.h"
#include "ast/ast_nodes.h"
#include "token.h"
#include "lexer.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @class DDLParser
 * @brief 数据定义语言（DDL）解析器 - 实现数据库模式结构的定义与维护
 *
 * WHY层 - 设计意图：
 *   数据库系统不仅需要处理数据，还需要定义数据的“形状”。DDL 语句用于创建和修改数据库的
 *   物理与逻辑结构。DDLParser 的存在是为了将结构化的 SQL 语句转换为抽象语法树（AST），
 *   从而让执行引擎知道如何创建表、分配存储空间或建立索引。
 *
 * WHAT层 - 功能说明：
 *   支持解析 CREATE（创建）、ALTER（修改）、DROP（删除）三大类操作。
 *   解析对象涵盖：DATABASE, TABLE, INDEX, VIEW, USER, PROCEDURE, TRIGGER。
 *   处理复杂的列定义（Column Definitions），包括数据类型、NULL/NOT NULL 约束、DEFAULT 值。
 *   支持表级约束解析，如 PRIMARY KEY, FOREIGN KEY。
 *
 * HOW层 - 实现机制：
 *   1. 递归下降解析：采用标准自顶向下的递归算法，基于 Token 预测分支。
 *   2. 共享状态：通过引用共享 Lexer 的当前 Token 和 Lookahead Token，实现高效的零拷贝扫描。
 *   3. 错误恢复：在解析列列表等重复项时，具备基本的同步点（Synchronization Points）以捕获局部语法错误。
 *   4. AST 节点工厂：将解析出的元数据（如列名、类型大小）封装为对应的 ASTNode 智能指针返回。
 */
class DDLParser {
public:
    DDLParser(Lexer& lexer, Token& currentToken, Token& lookaheadToken, bool& hasLookahead);

    // DDL语句解析
    std::unique_ptr<CreateStatement> parseCreateStatement();
    std::unique_ptr<CreateStatement> parseCreateTableStatement();
    std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();
    std::unique_ptr<CreateStatement> parseCreateProcedureStatement();
    std::unique_ptr<CreateStatement> parseCreateTriggerStatement();
    std::unique_ptr<Statement> parseCreateViewStatement();
    std::unique_ptr<DropStatement> parseDropStatement();
    std::unique_ptr<AlterStatement> parseAlterStatement();
    std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();
    std::unique_ptr<DropIndexStatement> parseDropIndexStatement();

    // 辅助方法
    std::vector<std::unique_ptr<ColumnDefinition>> parseColumnDefinitions();
    std::unique_ptr<ColumnDefinition> parseColumnDefinition();
    std::string parseDataType();
    std::string parseDefaultValue();
    void parseTableConstraint(CreateStatement& stmt);
    std::string parseQualifiedName();
    std::string parseIdentifier();

private:
    Lexer& lexer_;
    Token& currentToken_;
    Token& lookaheadToken_;
    bool& hasLookahead_;

    // Token管理辅助方法
    void advance();
    bool match(Type type);
    void consume(Type type);
    bool check(Type type) const;
    Token peek() const;
    Token previous() const;

    // Helper methods
    void reportError(const std::string& message);
    bool isAtEnd() const;
    std::vector<std::string> parseColumnNames();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_DDL_PARSER_H