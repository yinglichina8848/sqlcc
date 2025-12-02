#ifndef SQLCC_SQL_PARSER_PARSER_H
#define SQLCC_SQL_PARSER_PARSER_H

#include <memory>
#include <string>
#include <vector>
#include "token.h"
#include "ast_nodes.h"
#include "lexer.h"

namespace sqlcc {
namespace sql_parser {

class Parser {
public:
    Parser(const std::string& input);
    ~Parser() = default;
    
    std::vector<std::unique_ptr<Statement>> parseStatements();
    
private:
    Lexer lexer_;
    Token currentToken_;
    
    // 通用解析方法
    void nextToken();
    bool match(Token::Type type);
    void consume(Token::Type type);
    void consume();
    void expect(Token::Type type, const std::string& errorMessage);
    void reportError(const std::string& message);
    
    // 语句解析方法
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseCreateStatement();
    std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();
    std::unique_ptr<CreateStatement> parseCreateTableStatement();
    std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();
    std::unique_ptr<SelectStatement> parseSelectStatement();
    std::unique_ptr<InsertStatement> parseInsertStatement();
    std::unique_ptr<UpdateStatement> parseUpdateStatement();
    std::unique_ptr<DeleteStatement> parseDeleteStatement();
    std::unique_ptr<Statement> parseDropStatement();
    std::unique_ptr<AlterStatement> parseAlterStatement();
    std::unique_ptr<UseStatement> parseUseStatement();
    
    // DCL语句解析方法
    std::unique_ptr<Statement> parseCreateUserStatement();
    std::unique_ptr<Statement> parseDropUserStatement();
    std::unique_ptr<Statement> parseGrantStatement();
    std::unique_ptr<Statement> parseRevokeStatement();
    
    // SHOW语句解析方法
    std::unique_ptr<Statement> parseShowStatement();
    
    // 辅助解析方法
    void parseColumnDefinitions(CreateStatement& stmt);
    std::string parseDataType();
    void parseColumnConstraint(ColumnDefinition& columnDef);
    bool isColumnConstraint();
    void parseSelectList(SelectStatement& stmt);
    void parseFromClause(SelectStatement& stmt);
    void parseWhereClause(SelectStatement& stmt);
    void parseJoinClause(SelectStatement& stmt);
    void parseGroupByClause(SelectStatement& stmt);
    void parseOrderByClause(SelectStatement& stmt);
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
    
    // 新增的解析方法声明
    std::unique_ptr<DropStatement> parseDropDatabaseStatement();
    std::unique_ptr<DropStatement> parseDropTableStatement();
    std::unique_ptr<DropIndexStatement> parseDropIndexStatement();
    std::unique_ptr<AlterStatement> parseAlterDatabaseStatement();
    void parseInsertColumns(InsertStatement& stmt);
    void parseInsertValues(InsertStatement& stmt);
    void parseUpdateSetClause(UpdateStatement& stmt);
    void parseWhereClause(UpdateStatement& stmt);
    void parseWhereClause(DeleteStatement& stmt);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSER_H