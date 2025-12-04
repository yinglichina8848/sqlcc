#include "parser_new.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

ParserNew::ParserNew(const std::string& input)
    : lexer_(input), hasLookahead_(false), panicMode_(false) {
    initializeSyncTokens();
    advance(); // Get first token
}

std::vector<std::unique_ptr<Statement>> ParserNew::parse() {
    std::vector<std::unique_ptr<Statement>> statements;

    while (!isAtEnd()) {
        try {
            if (match(Token::SEMICOLON)) {
                continue; // Skip empty statements
            }

            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }

            // Skip semicolon if present
            if (match(Token::SEMICOLON)) {
                advance();
            }
        } catch (const std::runtime_error& e) {
            reportError(e.what());
            synchronize();
            if (panicMode_) {
                break; // Stop parsing if in panic mode
            }
        }
    }

    return statements;
}

// Core parsing methods
void ParserNew::advance() {
    if (hasLookahead_) {
        currentToken_ = lookaheadToken_;
        hasLookahead_ = false;
    } else {
        currentToken_ = lexer_.nextToken();
    }
}

bool ParserNew::match(Token::Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void ParserNew::consume(Token::Type type) {
    if (check(type)) {
        advance();
        return;
    }

    std::string expected = Token::getTypeName(type);
    std::string actual = currentToken_.getTypeName();
    throw std::runtime_error("Expected token '" + expected + "', but found '" + actual + "'");
}

bool ParserNew::check(Token::Type type) const {
    if (isAtEnd()) return false;
    return currentToken_.getType() == type;
}

bool ParserNew::isAtEnd() const {
    return currentToken_.getType() == Token::END_OF_INPUT;
}

Token ParserNew::peek() const {
    if (hasLookahead_) {
        return lookaheadToken_;
    }

    // This would require modifying LexerNew to support peeking
    // For now, return current token
    return currentToken_;
}

Token ParserNew::previous() const {
    // For now, return current token (simplified implementation)
    return currentToken_;
}

// Error handling
void ParserNew::reportError(const std::string& message) {
    std::stringstream ss;
    ss << "Parse error at line " << currentToken_.getLine()
       << ", column " << currentToken_.getColumn()
       << ": " << message;
    errors_.push_back(ss.str());
    panicMode_ = true;
}

void ParserNew::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (syncTokens_.count(currentToken_.getType()) > 0) {
            return;
        }
        advance();
    }
}

bool ParserNew::hadError() const {
    return !errors_.empty();
}

void ParserNew::initializeSyncTokens() {
    syncTokens_ = {
        Token::KEYWORD_SELECT, Token::KEYWORD_INSERT, Token::KEYWORD_UPDATE,
        Token::KEYWORD_DELETE, Token::KEYWORD_CREATE, Token::KEYWORD_DROP,
        Token::KEYWORD_ALTER, Token::KEYWORD_GRANT, Token::KEYWORD_REVOKE,
        Token::KEYWORD_SHOW, Token::KEYWORD_COMMIT, Token::KEYWORD_ROLLBACK
    };
}

// Statement parsing (strict BNF compliance)
std::unique_ptr<Statement> ParserNew::parseStatement() {
    if (match(Token::KEYWORD_SELECT) || match(Token::LPAREN)) {
        return parseDMLStatement();
    } else if (match(Token::KEYWORD_CREATE)) {
        return parseDDLStatement();
    } else if (match(Token::KEYWORD_INSERT)) {
        return parseDMLStatement();
    } else if (match(Token::KEYWORD_UPDATE)) {
        return parseDMLStatement();
    } else if (match(Token::KEYWORD_DELETE)) {
        return parseDMLStatement();
    } else if (match(Token::KEYWORD_DROP)) {
        return parseDDLStatement();
    } else if (match(Token::KEYWORD_ALTER)) {
        return parseDDLStatement();
    } else if (match(Token::KEYWORD_GRANT)) {
        return parseDCLStatement();
    } else if (match(Token::KEYWORD_REVOKE)) {
        return parseDCLStatement();
    } else if (match(Token::KEYWORD_COMMIT)) {
        return parseTCLStatement();
    } else if (match(Token::KEYWORD_ROLLBACK)) {
        return parseTCLStatement();
    } else if (match(Token::KEYWORD_SHOW)) {
        return parseShowStatement();
    } else {
        reportError("Unexpected token: " + currentToken_.getLexeme());
        return nullptr;
    }
}

std::unique_ptr<Statement> ParserNew::parseDDLStatement() {
    // This is a simplified implementation
    // Full implementation would handle all DDL variants
    reportError("DDL statements not yet implemented in new parser");
    return nullptr;
}

std::unique_ptr<Statement> ParserNew::parseDMLStatement() {
    if (currentToken_.getType() == Token::KEYWORD_SELECT ||
        currentToken_.getType() == Token::LPAREN) {
        return parseSelectStatement();
    } else if (match(Token::KEYWORD_INSERT)) {
        return parseInsertStatement();
    } else if (match(Token::KEYWORD_UPDATE)) {
        return parseUpdateStatement();
    } else if (match(Token::KEYWORD_DELETE)) {
        return parseDeleteStatement();
    } else {
        reportError("Unknown DML statement type");
        return nullptr;
    }
}

std::unique_ptr<Statement> ParserNew::parseDCLStatement() {
    reportError("DCL statements not yet implemented in new parser");
    return nullptr;
}

std::unique_ptr<Statement> ParserNew::parseTCLStatement() {
    if (match(Token::KEYWORD_COMMIT)) {
        // Simplified COMMIT implementation
        return std::make_unique<Statement>(Statement::Type::COMMIT);
    } else if (match(Token::KEYWORD_ROLLBACK)) {
        // Simplified ROLLBACK implementation
        return std::make_unique<Statement>(Statement::Type::ROLLBACK);
    } else {
        reportError("Unknown TCL statement");
        return nullptr;
    }
}

std::unique_ptr<Statement> ParserNew::parseShowStatement() {
    reportError("SHOW statements not yet implemented in new parser");
    return nullptr;
}

// DDL statements
std::unique_ptr<CreateStatement> ParserNew::parseCreateDatabaseStatement() {
    consume(Token::KEYWORD_DATABASE);
    std::string dbName = parseIdentifier();
    return std::make_unique<CreateStatement>(CreateStatement::DATABASE, dbName);
}

std::unique_ptr<CreateStatement> ParserNew::parseCreateTableStatement() {
    consume(Token::KEYWORD_TABLE);

    // Handle IF NOT EXISTS
    bool ifNotExists = false;
    if (match(Token::KEYWORD_IF)) {
        consume(Token::KEYWORD_NOT);
        consume(Token::KEYWORD_EXISTS);
        ifNotExists = true;
    }

    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE, tableName);

    consume(Token::LPAREN);

    // Parse column definitions
    while (!check(Token::RPAREN) && !isAtEnd()) {
        auto column = parseColumnDefinition();
        stmt->addColumn(std::move(column));

        if (!match(Token::COMMA)) {
            break;
        }
    }

    consume(Token::RPAREN);

    return stmt;
}

std::unique_ptr<CreateIndexStatement> ParserNew::parseCreateIndexStatement() {
    // Handle UNIQUE
    bool isUnique = match(Token::KEYWORD_UNIQUE);

    consume(Token::KEYWORD_INDEX);
    std::string indexName = parseIdentifier();

    consume(Token::KEYWORD_ON);
    std::string tableName = parseIdentifier();

    consume(Token::LPAREN);
    std::string columnName = parseIdentifier();
    consume(Token::RPAREN);

    auto stmt = std::make_unique<CreateIndexStatement>(indexName, tableName, columnName);
    if (isUnique) {
        stmt->setUnique(true);
    }

    return stmt;
}

// DML statements
std::unique_ptr<SelectStatement> ParserNew::parseSelectStatement() {
    auto stmt = std::make_unique<SelectStatement>();

    // Parse SELECT list
    parseSelectList(*stmt);

    // Parse FROM clause
    if (match(Token::KEYWORD_FROM)) {
        parseFromClause(*stmt);
    }

    // Parse WHERE clause
    if (match(Token::KEYWORD_WHERE)) {
        auto whereExpr = parseExpression();
        // Convert expression to WhereClause (simplified)
        stmt->setWhereClause(WhereClause("", "=", ""));
    }

    // Parse GROUP BY, HAVING, ORDER BY, LIMIT/OFFSET clauses
    // (simplified for initial implementation)

    return stmt;
}

std::unique_ptr<InsertStatement> ParserNew::parseInsertStatement() {
    consume(Token::KEYWORD_INSERT);
    consume(Token::KEYWORD_INTO);

    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<InsertStatement>(tableName);

    // Parse optional column list
    if (match(Token::LPAREN)) {
        parseInsertColumns(*stmt);
        consume(Token::RPAREN);
    }

    // Parse VALUES clause
    consume(Token::KEYWORD_VALUES);
    consume(Token::LPAREN);
    parseInsertValues(*stmt);
    consume(Token::RPAREN);

    return stmt;
}

std::unique_ptr<UpdateStatement> ParserNew::parseUpdateStatement() {
    consume(Token::KEYWORD_UPDATE);
    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<UpdateStatement>(tableName);

    parseUpdateSetClause(*stmt);

    if (match(Token::KEYWORD_WHERE)) {
        auto whereExpr = parseExpression();
        // Convert expression to WhereClause (simplified)
        stmt->setWhereClause(WhereClause("", "=", ""));
    }

    return stmt;
}

std::unique_ptr<DeleteStatement> ParserNew::parseDeleteStatement() {
    consume(Token::KEYWORD_DELETE);
    consume(Token::KEYWORD_FROM);
    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<DeleteStatement>(tableName);

    if (match(Token::KEYWORD_WHERE)) {
        auto whereExpr = parseExpression();
        // Convert expression to WhereClause (simplified)
        stmt->setWhereClause(WhereClause("", "=", ""));
    }

    return stmt;
}

// Helper parsing methods
std::string ParserNew::parseIdentifier() {
    if (!check(Token::IDENTIFIER)) {
        reportError("Expected identifier");
        return "";
    }
    std::string result = currentToken_.getLexeme();
    advance();
    return result;
}

std::string ParserNew::parseStringLiteral() {
    if (!check(Token::STRING_LITERAL)) {
        reportError("Expected string literal");
        return "";
    }
    std::string result = currentToken_.getLexeme();
    advance();
    return result;
}

long long ParserNew::parseIntegerLiteral() {
    if (!check(Token::INTEGER_LITERAL)) {
        reportError("Expected integer literal");
        return 0;
    }
    long long result = std::stoll(currentToken_.getLexeme());
    advance();
    return result;
}

double ParserNew::parseNumericLiteral() {
    if (check(Token::INTEGER_LITERAL)) {
        long long intPart = parseIntegerLiteral();
        return static_cast<double>(intPart);
    } else if (check(Token::FLOAT_LITERAL)) {
        double result = std::stod(currentToken_.getLexeme());
        advance();
        return result;
    } else {
        reportError("Expected numeric literal");
        return 0.0;
    }
}

// Data types and constraints
std::string ParserNew::parseDataType() {
    std::string typeName = parseIdentifier();

    // Handle type parameters like VARCHAR(255), DECIMAL(10,2)
    if (match(Token::LPAREN)) {
        typeName += "(";
        typeName += std::to_string(parseIntegerLiteral());

        if (match(Token::COMMA)) {
            typeName += ",";
            typeName += std::to_string(parseIntegerLiteral());
        }
        consume(Token::RPAREN);
        typeName += ")";
    }

    return typeName;
}

ColumnDefinition ParserNew::parseColumnDefinition() {
    std::string columnName = parseIdentifier();
    std::string dataType = parseDataType();

    ColumnDefinition column(columnName, dataType);

    // Parse column constraints
    parseColumnConstraints(column);

    return column;
}

void ParserNew::parseColumnConstraints(ColumnDefinition& column) {
    while (isColumnConstraint()) {
        parseColumnConstraint(column);
    }
}

bool ParserNew::parseColumnConstraint(ColumnDefinition& column) {
    if (match(Token::KEYWORD_NOT)) {
        consume(Token::KEYWORD_NULL);
        column.setIsNullable(false);
        return true;
    } else if (match(Token::KEYWORD_NULL)) {
        column.setIsNullable(true);
        return true;
    } else if (match(Token::KEYWORD_PRIMARY)) {
        consume(Token::KEYWORD_KEY);
        column.setIsPrimaryKey(true);
        column.setIsNullable(false);
        return true;
    } else if (match(Token::KEYWORD_UNIQUE)) {
        column.setIsUnique(true);
        return true;
    } else if (match(Token::KEYWORD_AUTO_INCREMENT)) {
        column.setIsAutoIncrement(true);
        return true;
    } else if (match(Token::KEYWORD_DEFAULT)) {
        // Parse default value
        if (check(Token::STRING_LITERAL)) {
            column.setDefaultValue(parseStringLiteral());
        } else if (check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
            column.setDefaultValue(currentToken_.getLexeme());
            advance();
        } else if (match(Token::KEYWORD_NULL)) {
            column.setDefaultValue("NULL");
        } else {
            reportError("Expected default value");
        }
        return true;
    }

    return false;
}

bool ParserNew::isColumnConstraint() {
    return check(Token::KEYWORD_NOT) || check(Token::KEYWORD_NULL) ||
           check(Token::KEYWORD_PRIMARY) || check(Token::KEYWORD_UNIQUE) ||
           check(Token::KEYWORD_AUTO_INCREMENT) || check(Token::KEYWORD_DEFAULT) ||
           check(Token::KEYWORD_REFERENCES) || check(Token::KEYWORD_CHECK);
}

// Expressions (strict precedence)
std::unique_ptr<Expression> ParserNew::parseExpression() {
    return parseOrExpression();
}

std::unique_ptr<Expression> ParserNew::parseOrExpression() {
    auto expr = parseAndExpression();

    while (match(Token::KEYWORD_OR)) {
        auto right = parseAndExpression();
        // Create binary expression (simplified)
        expr = std::move(right); // Placeholder
    }

    return expr;
}

std::unique_ptr<Expression> ParserNew::parseAndExpression() {
    auto expr = parseComparisonExpression();

    while (match(Token::KEYWORD_AND)) {
        auto right = parseComparisonExpression();
        // Create binary expression (simplified)
        expr = std::move(right); // Placeholder
    }

    return expr;
}

std::unique_ptr<Expression> ParserNew::parseComparisonExpression() {
    auto expr = parseAdditiveExpression();

    if (isComparisonOperator()) {
        advance(); // consume operator
        auto right = parseAdditiveExpression();
        // Create comparison expression (simplified)
        expr = std::move(right); // Placeholder
    }

    return expr;
}

std::unique_ptr<Expression> ParserNew::parseAdditiveExpression() {
    auto expr = parseMultiplicativeExpression();

    while (match(Token::OPERATOR_PLUS) || match(Token::OPERATOR_MINUS)) {
        Token::Type op = previous().getType();
        auto right = parseMultiplicativeExpression();
        // Create binary expression (simplified)
        expr = std::move(right); // Placeholder
    }

    return expr;
}

std::unique_ptr<Expression> ParserNew::parseMultiplicativeExpression() {
    auto expr = parseUnaryExpression();

    while (match(Token::OPERATOR_MULTIPLY) || match(Token::OPERATOR_DIVIDE) ||
           match(Token::OPERATOR_MODULO)) {
        Token::Type op = previous().getType();
        auto right = parseUnaryExpression();
        // Create binary expression (simplified)
        expr = std::move(right); // Placeholder
    }

    return expr;
}

std::unique_ptr<Expression> ParserNew::parseUnaryExpression() {
    if (match(Token::OPERATOR_PLUS) || match(Token::OPERATOR_MINUS) ||
        match(Token::OPERATOR_NOT)) {
        Token::Type op = previous().getType();
        auto operand = parsePrimaryExpression();
        // Create unary expression (simplified)
        return operand;
    }

    return parsePrimaryExpression();
}

std::unique_ptr<Expression> ParserNew::parsePrimaryExpression() {
    if (match(Token::LPAREN)) {
        auto expr = parseExpression();
        consume(Token::RPAREN);
        return expr;
    } else if (check(Token::IDENTIFIER)) {
        // Could be column reference, function call, or keyword
        return parseColumnReferenceOrFunction();
    } else if (check(Token::STRING_LITERAL)) {
        return parseStringLiteral();
    } else if (check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
        return parseNumericLiteral();
    } else if (match(Token::KEYWORD_NULL)) {
        // Create null literal
        return nullptr;
    } else if (match(Token::KEYWORD_TRUE) || match(Token::KEYWORD_FALSE)) {
        // Create boolean literal
        return nullptr;
    } else {
        reportError("Expected primary expression");
        return nullptr;
    }
}

// Placeholder implementations for complex parsing
std::unique_ptr<Expression> ParserNew::parseColumnReferenceOrFunction() {
    std::string identifier = parseIdentifier();

    if (match(Token::LPAREN)) {
        // Function call
        return parseFunctionCall();
    } else if (match(Token::DOT)) {
        // Column reference with table prefix
        std::string columnName = parseIdentifier();
        // Create column reference
        return nullptr;
    } else {
        // Simple column reference
        return nullptr;
    }
}

std::unique_ptr<Expression> ParserNew::parseFunctionCall() {
    // Parse function arguments
    std::vector<std::unique_ptr<Expression>> arguments;

    if (!check(Token::RPAREN)) {
        do {
            if (match(Token::OPERATOR_MULTIPLY)) {
                // SELECT COUNT(*) case
                break;
            }
            arguments.push_back(parseExpression());
        } while (match(Token::COMMA));
    }

    consume(Token::RPAREN);
    return nullptr; // Placeholder
}

// Simplified implementations
void ParserNew::parseSelectList(SelectStatement& stmt) {
    if (match(Token::OPERATOR_MULTIPLY)) {
        stmt.setSelectAll(true);
    } else {
        do {
            std::string columnName = parseIdentifier();
            stmt.addSelectColumn(columnName);
        } while (match(Token::COMMA));
    }
}

void ParserNew::parseFromClause(SelectStatement& stmt) {
    std::string tableName = parseIdentifier();
    stmt.setTableName(tableName);

    // Handle JOINs (simplified)
    while (true) {
        if (match(Token::KEYWORD_JOIN) || match(Token::KEYWORD_INNER) ||
            match(Token::KEYWORD_LEFT) || match(Token::KEYWORD_RIGHT) ||
            match(Token::KEYWORD_FULL)) {

            // Skip join type keywords
            while (match(Token::KEYWORD_INNER) || match(Token::KEYWORD_LEFT) ||
                   match(Token::KEYWORD_RIGHT) || match(Token::KEYWORD_FULL) ||
                   match(Token::KEYWORD_OUTER) || match(Token::KEYWORD_JOIN)) {
                // Continue
            }

            // Parse joined table
            std::string joinTable = parseIdentifier();

            // Parse ON condition (simplified)
            if (match(Token::KEYWORD_ON)) {
                // Skip ON condition for now
                while (!check(Token::KEYWORD_WHERE) && !check(Token::KEYWORD_GROUP) &&
                       !check(Token::KEYWORD_ORDER) && !check(Token::KEYWORD_LIMIT) &&
                       !check(Token::SEMICOLON) && !isAtEnd()) {
                    advance();
                }
            }
        } else {
            break;
        }
    }
}

void ParserNew::parseWhereClause(SelectStatement& stmt) {
    auto expr = parseExpression();
    // Convert to WhereClause (simplified)
}

void ParserNew::parseGroupByClause(SelectStatement& stmt) {
    do {
        std::string columnName = parseIdentifier();
        stmt.setGroupByColumn(columnName);
    } while (match(Token::COMMA));
}

void ParserNew::parseHavingClause(SelectStatement& stmt) {
    auto expr = parseExpression();
    // Handle HAVING clause
}

void ParserNew::parseOrderByClause(SelectStatement& stmt) {
    do {
        std::string columnName = parseIdentifier();
        stmt.setOrderByColumn(columnName);

        if (match(Token::KEYWORD_ASC) || match(Token::KEYWORD_DESC)) {
            std::string direction = (previous().getType() == Token::KEYWORD_ASC) ? "ASC" : "DESC";
            stmt.setOrderDirection(direction);
        }
    } while (match(Token::COMMA));
}

void ParserNew::parseLimitOffsetClause(SelectStatement& stmt) {
    int limit = static_cast<int>(parseIntegerLiteral());
    stmt.setLimit(limit);

    if (match(Token::KEYWORD_OFFSET) || match(Token::COMMA)) {
        int offset = static_cast<int>(parseIntegerLiteral());
        stmt.setOffset(offset);
    }
}

void ParserNew::parseInsertColumns(InsertStatement& stmt) {
    do {
        std::string columnName = parseIdentifier();
        stmt.addColumn(columnName);
    } while (match(Token::COMMA));
}

void ParserNew::parseInsertValues(InsertStatement& stmt) {
    do {
        if (match(Token::STRING_LITERAL) || match(Token::INTEGER_LITERAL) ||
            match(Token::FLOAT_LITERAL) || match(Token::KEYWORD_NULL)) {
            std::string value = previous().getLexeme();
            stmt.addValue(value);
        } else {
            reportError("Expected literal value");
        }
    } while (match(Token::COMMA));
}

void ParserNew::parseUpdateSetClause(UpdateStatement& stmt) {
    consume(Token::KEYWORD_SET);

    do {
        std::string columnName = parseIdentifier();
        consume(Token::OPERATOR_EQUAL);

        std::string value;
        if (match(Token::STRING_LITERAL) || match(Token::INTEGER_LITERAL) ||
            match(Token::FLOAT_LITERAL) || match(Token::KEYWORD_NULL)) {
            value = previous().getLexeme();
        } else {
            reportError("Expected value in SET clause");
        }

        stmt.addUpdateValue(columnName, value);
    } while (match(Token::COMMA));
}

// Utility methods
bool ParserNew::isDataTypeKeyword() const {
    return check(Token::IDENTIFIER) &&
           (currentToken_.getLexeme() == "INT" ||
            currentToken_.getLexeme() == "VARCHAR" ||
            currentToken_.getLexeme() == "TEXT" ||
            currentToken_.getLexeme() == "DATE" ||
            currentToken_.getLexeme() == "DATETIME" ||
            currentToken_.getLexeme() == "DECIMAL" ||
            currentToken_.getLexeme() == "FLOAT" ||
            currentToken_.getLexeme() == "DOUBLE" ||
            currentToken_.getLexeme() == "BOOLEAN" ||
            currentToken_.getLexeme() == "BLOB");
}

bool ParserNew::isFunctionName() const {
    return check(Token::IDENTIFIER) &&
           (currentToken_.getLexeme() == "COUNT" ||
            currentToken_.getLexeme() == "SUM" ||
            currentToken_.getLexeme() == "AVG" ||
            currentToken_.getLexeme() == "MIN" ||
            currentToken_.getLexeme() == "MAX" ||
            currentToken_.getLexeme() == "CONCAT" ||
            currentToken_.getLexeme() == "SUBSTRING" ||
            currentToken_.getLexeme() == "LENGTH" ||
            currentToken_.getLexeme() == "UPPER" ||
            currentToken_.getLexeme() == "LOWER" ||
            currentToken_.getLexeme() == "TRIM");
}

bool ParserNew::isSetOperation() const {
    return check(Token::KEYWORD_UNION) || check(Token::KEYWORD_INTERSECT) ||
           check(Token::KEYWORD_EXCEPT);
}

bool ParserNew::isComparisonOperator() const {
    return check(Token::OPERATOR_EQUAL) || check(Token::OPERATOR_NOT_EQUAL) ||
           check(Token::OPERATOR_LESS_THAN) || check(Token::OPERATOR_LESS_EQUAL) ||
           check(Token::OPERATOR_GREATER_THAN) || check(Token::OPERATOR_GREATER_EQUAL) ||
           check(Token::OPERATOR_LIKE) || check(Token::OPERATOR_IN);
}

bool ParserNew::isArithmeticOperator() const {
    return check(Token::OPERATOR_PLUS) || check(Token::OPERATOR_MINUS) ||
           check(Token::OPERATOR_MULTIPLY) || check(Token::OPERATOR_DIVIDE) ||
           check(Token::OPERATOR_MODULO);
}

bool ParserNew::isLogicalOperator() const {
    return check(Token::KEYWORD_AND) || check(Token::KEYWORD_OR) ||
           check(Token::KEYWORD_NOT);
}

// Stub implementations for not-yet-implemented methods
std::unique_ptr<DropStatement> ParserNew::parseDropDatabaseStatement() {
    consume(Token::KEYWORD_DATABASE);
    std::string dbName = parseIdentifier();
    return std::make_unique<DropStatement>(DropStatement::DATABASE, dbName);
}

std::unique_ptr<DropStatement> ParserNew::parseDropTableStatement() {
    consume(Token::KEYWORD_TABLE);
    std::string tableName = parseIdentifier();
    return std::make_unique<DropStatement>(DropStatement::TABLE, tableName);
}

std::unique_ptr<DropIndexStatement> ParserNew::parseDropIndexStatement() {
    consume(Token::KEYWORD_INDEX);
    std::string indexName = parseIdentifier();
    return std::make_unique<DropIndexStatement>(indexName);
}

std::unique_ptr<AlterStatement> ParserNew::parseAlterTableStatement() {
    consume(Token::KEYWORD_TABLE);
    std::string tableName = parseIdentifier();
    return std::make_unique<AlterStatement>(AlterStatement::TABLE, tableName);
}

std::unique_ptr<Statement> ParserNew::parseGrantStatement() {
    consume(Token::KEYWORD_GRANT);
    // Simplified implementation
    return nullptr;
}

std::unique_ptr<Statement> ParserNew::parseRevokeStatement() {
    consume(Token::KEYWORD_REVOKE);
    // Simplified implementation
    return nullptr;
}

std::unique_ptr<Statement> ParserNew::parseCommitStatement() {
    consume(Token::KEYWORD_COMMIT);
    return std::make_unique<Statement>(Statement::Type::COMMIT);
}

std::unique_ptr<Statement> ParserNew::parseRollbackStatement() {
    consume(Token::KEYWORD_ROLLBACK);
    return std::make_unique<Statement>(Statement::Type::ROLLBACK);
}

std::string ParserNew::parseTableReference() {
    return parseIdentifier();
}

void ParserNew::parseJoinClause(SelectStatement& stmt) {
    // Simplified implementation
}

std::unique_ptr<Expression> ParserNew::parseJoinCondition() {
    return parseExpression();
}

std::unique_ptr<Expression> ParserNew::parseSubquery() {
    consume(Token::LPAREN);
    auto selectStmt = parseSelectStatement();
    consume(Token::RPAREN);
    return nullptr; // Placeholder
}

std::unique_ptr<Expression> ParserNew::parseExistsExpression() {
    consume(Token::KEYWORD_EXISTS);
    return parseSubquery();
}

std::unique_ptr<Expression> ParserNew::parseSelectItem() {
    return parseExpression();
}

std::unique_ptr<Statement> ParserNew::parseSetOperation() {
    return parseSelectStatement();
}

SetOperationType ParserNew::parseSetOperationType() {
    if (match(Token::KEYWORD_UNION)) {
        return SetOperationType::UNION;
    } else if (match(Token::KEYWORD_INTERSECT)) {
        return SetOperationType::INTERSECT;
    } else if (match(Token::KEYWORD_EXCEPT)) {
        return SetOperationType::EXCEPT;
    }
    return SetOperationType::UNION; // Default
}

std::unique_ptr<Expression> ParserNew::parseCaseExpression() {
    consume(Token::KEYWORD_CASE);
    // Simplified implementation
    while (!match(Token::KEYWORD_END) && !isAtEnd()) {
        advance();
    }
    return nullptr;
}

std::unique_ptr<Expression> ParserNew::parseWhereCondition() {
    return parseExpression();
}

} // namespace sql_parser
} // namespace sqlcc
