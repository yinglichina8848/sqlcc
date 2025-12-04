#ifndef SQLCC_SQL_PARSER_PARSER_NEW_H
#define SQLCC_SQL_PARSER_PARSER_NEW_H

#include "lexer_new.h"
#include "ast_nodes.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

// Forward declarations for set operations
class SetOperationNode;
enum class SetOperationType;

class ParserNew {
public:
    ParserNew(const std::string& input);
    ~ParserNew() = default;

    std::vector<std::unique_ptr<Statement>> parse();

private:
    // Token stream management
    LexerNew lexer_;
    Token currentToken_;
    Token lookaheadToken_;
    bool hasLookahead_;

    // Error recovery
    std::vector<std::string> errors_;
    bool panicMode_;
    std::unordered_set<Token::Type> syncTokens_;

    // Core parsing methods
    void advance();
    bool match(Token::Type type);
    void consume(Token::Type type);
    bool check(Token::Type type) const;
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;

    // Error handling
    void reportError(const std::string& message);
    void synchronize();
    bool hadError() const;

    // Statement parsing (strict BNF compliance)
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseDDLStatement();
    std::unique_ptr<Statement> parseDMLStatement();
    std::unique_ptr<Statement> parseDCLStatement();
    std::unique_ptr<Statement> parseTCLStatement();
    std::unique_ptr<Statement> parseShowStatement();

    // DDL statements
    std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();
    std::unique_ptr<CreateStatement> parseCreateTableStatement();
    std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();
    std::unique_ptr<DropStatement> parseDropDatabaseStatement();
    std::unique_ptr<DropStatement> parseDropTableStatement();
    std::unique_ptr<DropIndexStatement> parseDropIndexStatement();
    std::unique_ptr<AlterStatement> parseAlterTableStatement();

    // DML statements
    std::unique_ptr<SelectStatement> parseSelectStatement();
    std::unique_ptr<InsertStatement> parseInsertStatement();
    std::unique_ptr<UpdateStatement> parseUpdateStatement();
    std::unique_ptr<DeleteStatement> parseDeleteStatement();

    // DCL statements
    std::unique_ptr<Statement> parseGrantStatement();
    std::unique_ptr<Statement> parseRevokeStatement();

    // TCL statements
    std::unique_ptr<Statement> parseCommitStatement();
    std::unique_ptr<Statement> parseRollbackStatement();

    // Helper parsing methods
    std::string parseIdentifier();
    std::string parseStringLiteral();
    long long parseIntegerLiteral();
    double parseNumericLiteral();

    // Data types and constraints
    std::string parseDataType();
    ColumnDefinition parseColumnDefinition();
    void parseColumnConstraints(ColumnDefinition& column);
    bool parseColumnConstraint(ColumnDefinition& column);

    // Table references and JOINs
    std::string parseTableReference();
    void parseJoinClause(SelectStatement& stmt);
    std::unique_ptr<Expression> parseJoinCondition();

    // Expressions (strict precedence)
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseNotExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();

    // Subqueries and complex expressions
    std::unique_ptr<Expression> parseSubquery();
    std::unique_ptr<Expression> parseFunctionCall();
    std::unique_ptr<Expression> parseCaseExpression();
    std::unique_ptr<Expression> parseExistsExpression();

    // SELECT components
    void parseSelectList(SelectStatement& stmt);
    std::unique_ptr<Expression> parseSelectItem();
    void parseFromClause(SelectStatement& stmt);
    void parseWhereClause(SelectStatement& stmt);
    void parseGroupByClause(SelectStatement& stmt);
    void parseHavingClause(SelectStatement& stmt);
    void parseOrderByClause(SelectStatement& stmt);
    void parseLimitOffsetClause(SelectStatement& stmt);

    // Set operations
    std::unique_ptr<Statement> parseSetOperation();
    SetOperationType parseSetOperationType();

    // INSERT components
    void parseInsertColumns(InsertStatement& stmt);
    void parseInsertValues(InsertStatement& stmt);

    // UPDATE components
    void parseUpdateSetClause(UpdateStatement& stmt);

    // WHERE clause (universal)
    std::unique_ptr<Expression> parseWhereCondition();

    // Utility methods
    bool isDataTypeKeyword() const;
    bool isFunctionName() const;
    bool isSetOperation() const;
    bool isComparisonOperator() const;
    bool isArithmeticOperator() const;
    bool isLogicalOperator() const;

    // Recovery synchronization points
    void initializeSyncTokens();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSER_NEW_H
