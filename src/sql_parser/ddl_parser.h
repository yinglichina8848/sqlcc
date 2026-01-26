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
 * DDLParser - 数据定义语言解析器
 *
 * 负责解析SQL的DDL语句：
 * - CREATE TABLE/DATABASE/INDEX/USER/PROCEDURE/TRIGGER/VIEW
 * - ALTER TABLE
 * - DROP TABLE/DATABASE/INDEX/USER/PROCEDURE/TRIGGER/VIEW
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