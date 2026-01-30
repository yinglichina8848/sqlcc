#include "src/sql_parser/ddl_parser.h"
#include "ast/ast_nodes.h"
#include "ast/ddl/ast_ddl_nodes.h"
#include <iostream>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

DDLParser::DDLParser(Lexer& lexer, Token& currentToken, Token& lookaheadToken, bool& hasLookahead)
    : lexer_(lexer), currentToken_(currentToken), lookaheadToken_(lookaheadToken), hasLookahead_(hasLookahead) {
}

std::unique_ptr<CreateStatement> DDLParser::parseCreateStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateStatement()" << std::endl;

    if (match(Type::KEYWORD_TABLE)) {
        return parseCreateTableStatement();
    } else if (match(Type::KEYWORD_DATABASE)) {
        return parseCreateDatabaseStatement();
    } else if (match(Type::KEYWORD_INDEX)) {
        // INDEX returns CreateIndexStatement, not CreateStatement
        reportError("CREATE INDEX not supported in this context");
        return nullptr;
    } else if (match(Type::KEYWORD_PROCEDURE)) {
        return parseCreateProcedureStatement();
    } else if (match(Type::KEYWORD_TRIGGER)) {
        return parseCreateTriggerStatement();
    }

    reportError("Expected TABLE, DATABASE, INDEX, PROCEDURE, or TRIGGER after CREATE");
    return nullptr;
}

std::unique_ptr<CreateStatement> DDLParser::parseCreateTableStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateTableStatement()方法" << std::endl;

    auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE);
    if (!stmt) {
        std::cerr << "Failed to create CreateStatement object" << std::endl;
        return nullptr;
    }

    std::string tableName = parseIdentifier();
    stmt->setObjectName(tableName);
    std::cout << "[DDL_PARSER] 表名: " << tableName << std::endl;

    consume(Type::LPAREN);

    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!first) {
            if (!match(Type::COMMA)) {
                break;
            }
        }
        first = false;

        if (check(Type::KEYWORD_PRIMARY) || check(Type::KEYWORD_UNIQUE) ||
            check(Type::KEYWORD_FOREIGN) || check(Type::KEYWORD_CHECK) ||
            check(Type::KEYWORD_CONSTRAINT)) {
            parseTableConstraint(*stmt);
        } else {
            auto columnDef = parseColumnDefinition();
            if (columnDef) {
                stmt->addColumn(std::move(columnDef));
            }
        }
    }

    consume(Type::RPAREN);

    std::cout << "[DDL_PARSER] CREATE TABLE语句解析完成" << std::endl;
    return stmt;
}

std::unique_ptr<CreateStatement> DDLParser::parseCreateDatabaseStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateDatabaseStatement()" << std::endl;

    auto stmt = std::make_unique<CreateStatement>(CreateStatement::DATABASE);
    std::string dbName = parseIdentifier();
    stmt->setObjectName(dbName);

    std::cout << "[DDL_PARSER] CREATE DATABASE语句解析完成: " << dbName << std::endl;
    return stmt;
}

std::unique_ptr<CreateIndexStatement> DDLParser::parseCreateIndexStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateIndexStatement()" << std::endl;

    consume(Type::KEYWORD_INDEX);
    std::string indexName = parseIdentifier();
    consume(Type::KEYWORD_ON);
    std::string tableName = parseIdentifier();
    consume(Type::LPAREN);
    std::vector<std::string> columns = parseColumnNames();
    consume(Type::RPAREN);

    if (columns.empty()) {
        reportError("Index must have at least one column");
        return nullptr;
    }

    auto stmt = std::make_unique<CreateIndexStatement>(indexName, tableName, columns[0]);
    for (size_t i = 1; i < columns.size(); ++i) {
        stmt->addColumn(columns[i]);
    }

    std::cout << "[DDL_PARSER] CREATE INDEX语句解析完成" << std::endl;
    return stmt;
}

std::unique_ptr<CreateStatement> DDLParser::parseCreateProcedureStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateProcedureStatement()" << std::endl;
    // Simplified implementation
    auto stmt = std::make_unique<CreateStatement>(CreateStatement::PROCEDURE);
    std::string procName = parseIdentifier();
    stmt->setObjectName(procName);
    // Skip procedure body for now
    while (!match(Type::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    return stmt;
}

std::unique_ptr<CreateStatement> DDLParser::parseCreateTriggerStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateTriggerStatement()" << std::endl;
    // Simplified implementation
    auto stmt = std::make_unique<CreateStatement>(CreateStatement::TRIGGER);
    std::string triggerName = parseIdentifier();
    stmt->setObjectName(triggerName);
    // Skip trigger body for now
    while (!match(Type::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    return stmt;
}

std::unique_ptr<Statement> DDLParser::parseCreateViewStatement() {
    std::cout << "[DDL_PARSER] 进入parseCreateViewStatement()" << std::endl;
    // Simplified implementation - would need full SELECT parsing
    std::string viewName = parseIdentifier();
    auto stmt = std::make_unique<CreateViewStatement>(viewName);
    consume(Type::KEYWORD_AS);
    // Skip SELECT statement for now
    while (!match(Type::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    return stmt;
}

std::unique_ptr<DropStatement> DDLParser::parseDropStatement() {
    std::cout << "[DDL_PARSER] 进入parseDropStatement()" << std::endl;

    if (match(Type::KEYWORD_TABLE)) {
        std::string tableName = parseIdentifier();
        auto stmt = std::make_unique<DropStatement>(DropStatement::TABLE, tableName);
        std::cout << "[DDL_PARSER] DROP TABLE语句解析完成" << std::endl;
        return stmt;
    } else if (match(Type::KEYWORD_DATABASE)) {
        std::string dbName = parseIdentifier();
        auto stmt = std::make_unique<DropStatement>(DropStatement::DATABASE, dbName);
        std::cout << "[DDL_PARSER] DROP DATABASE语句解析完成" << std::endl;
        return stmt;
    } else if (match(Type::KEYWORD_INDEX)) {
        std::string indexName = parseIdentifier();
        auto stmt = std::make_unique<DropStatement>(DropStatement::INDEX, indexName);
        std::cout << "[DDL_PARSER] DROP INDEX语句解析完成" << std::endl;
        return stmt;
    }

    reportError("Expected TABLE, DATABASE, or INDEX after DROP");
    return nullptr;
}

std::unique_ptr<AlterStatement> DDLParser::parseAlterStatement() {
    std::cout << "[DDL_PARSER] 进入parseAlterStatement()" << std::endl;
    // Simplified implementation
    consume(Type::KEYWORD_TABLE);
    auto stmt = std::make_unique<AlterStatement>(AlterStatement::TABLE);
    std::string tableName = parseIdentifier();
    stmt->setTableName(tableName);
    // Skip ALTER details for now
    while (!match(Type::SEMICOLON) && !isAtEnd()) {
        advance();
    }
    return stmt;
}

std::unique_ptr<DropIndexStatement> DDLParser::parseDropIndexStatement() {
    std::cout << "[DDL_PARSER] 进入parseDropIndexStatement()" << std::endl;

    consume(Type::KEYWORD_INDEX);
    std::string indexName = parseIdentifier();
    auto stmt = std::make_unique<DropIndexStatement>(indexName);

    return stmt;
}

// Helper methods implementation
std::vector<std::unique_ptr<ColumnDefinition>> DDLParser::parseColumnDefinitions() {
    std::vector<std::unique_ptr<ColumnDefinition>> columns;
    consume(Type::LPAREN);

    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!first && !match(Type::COMMA)) break;
        first = false;

        auto col = parseColumnDefinition();
        if (col) columns.push_back(std::move(col));
    }

    consume(Type::RPAREN);
    return columns;
}

std::unique_ptr<ColumnDefinition> DDLParser::parseColumnDefinition() {
    std::cout << "[DDL_PARSER] 进入parseColumnDefinition()方法" << std::endl;

    std::string columnName = parseIdentifier();
    std::string dataType = parseDataType();

    auto columnDef = std::make_unique<ColumnDefinition>(columnName, dataType);

    while (!check(Type::COMMA) && !check(Type::RPAREN) && !isAtEnd()) {
        if (match(Type::KEYWORD_NOT)) {
            consume(Type::KEYWORD_NULL);
            columnDef->setNullable(false);
        } else if (match(Type::KEYWORD_NULL)) {
            columnDef->setNullable(true);
        } else if (match(Type::KEYWORD_PRIMARY)) {
            consume(Type::KEYWORD_KEY);
            columnDef->setPrimaryKey(true);
        } else if (match(Type::KEYWORD_UNIQUE)) {
            columnDef->setUnique(true);
        } else if (match(Type::KEYWORD_DEFAULT)) {
            std::string defaultValue = parseDefaultValue();
            columnDef->setDefaultValue(defaultValue);
        } else if (match(Type::KEYWORD_AUTO_INCREMENT)) {
            columnDef->setAutoIncrement(true);
        } else {
            break;
        }
    }

    std::cout << "[DDL_PARSER] 列定义解析完成" << std::endl;
    return columnDef;
}

std::string DDLParser::parseDataType() {
    std::stringstream dataType;

    if (check(Type::KEYWORD_INT) || check(Type::KEYWORD_INTEGER) ||
        check(Type::KEYWORD_SMALLINT) || check(Type::KEYWORD_BIGINT) ||
        check(Type::KEYWORD_TINYINT) || check(Type::KEYWORD_VARCHAR) ||
        check(Type::KEYWORD_CHAR) || check(Type::KEYWORD_DECIMAL) ||
        check(Type::KEYWORD_NUMERIC) || check(Type::KEYWORD_DATE) ||
        check(Type::KEYWORD_TIME) || check(Type::KEYWORD_TIMESTAMP) ||
        check(Type::KEYWORD_DATETIME) || check(Type::KEYWORD_BOOLEAN) ||
        check(Type::KEYWORD_BOOL)) {
        dataType << currentToken_.getLexeme();
        advance();

        if ((dataType.str() == "VARCHAR" || dataType.str() == "CHAR") && match(Type::LPAREN)) {
            dataType << "(" << currentToken_.getLexeme() << ")";
            advance();
            consume(Type::RPAREN);
        } else if ((dataType.str() == "DECIMAL" || dataType.str() == "NUMERIC") && match(Type::LPAREN)) {
            dataType << "(" << currentToken_.getLexeme();
            advance();
            if (match(Type::COMMA)) {
                dataType << "," << currentToken_.getLexeme();
                advance();
            }
            dataType << ")";
            consume(Type::RPAREN);
        }
    }

    return dataType.str();
}

std::string DDLParser::parseDefaultValue() {
    std::stringstream defaultValue;

    if (check(Type::INTEGER_LITERAL) || check(Type::FLOAT_LITERAL)) {
        defaultValue << currentToken_.getLexeme();
        advance();
    } else if (check(Type::STRING_LITERAL)) {
        defaultValue << "'" << currentToken_.getLexeme() << "'";
        advance();
    } else if (match(Type::KEYWORD_NULL)) {
        defaultValue << "NULL";
    }

    return defaultValue.str();
}

void DDLParser::parseTableConstraint(CreateStatement& stmt) {
    if (match(Type::KEYWORD_PRIMARY)) {
        consume(Type::KEYWORD_KEY);
        consume(Type::LPAREN);
        std::vector<std::string> columns = parseColumnNames();
        consume(Type::RPAREN);
        auto pkConstraint = std::make_unique<TableConstraint>(TableConstraint::PRIMARY_KEY);
        for (const auto& col : columns) {
            pkConstraint->addColumn(col);
        }
        stmt.addConstraint(std::move(pkConstraint));
    } else if (match(Type::KEYWORD_UNIQUE)) {
        consume(Type::LPAREN);
        std::vector<std::string> columns = parseColumnNames();
        consume(Type::RPAREN);
        auto uniqueConstraint = std::make_unique<TableConstraint>(TableConstraint::UNIQUE);
        for (const auto& col : columns) {
            uniqueConstraint->addColumn(col);
        }
        stmt.addConstraint(std::move(uniqueConstraint));
    }
    // Add more constraint types as needed
}

std::string DDLParser::parseQualifiedName() {
    std::string name = parseIdentifier();
    if (match(Type::DOT)) {
        name += "." + parseIdentifier();
    }
    return name;
}

std::string DDLParser::parseIdentifier() {
    if (check(Type::IDENTIFIER)) {
        std::string id = currentToken_.getLexeme();
        advance();
        return id;
    }
    reportError("Expected identifier");
    return "";
}

// Token management methods
void DDLParser::advance() {
    if (hasLookahead_) {
        currentToken_ = lookaheadToken_;
        hasLookahead_ = false;
    } else {
        currentToken_ = lexer_.nextToken();
    }
}

bool DDLParser::match(Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void DDLParser::consume(Type type) {
    if (check(type)) {
        advance();
    } else {
        reportError("Expected " + std::to_string(static_cast<int>(type)));
    }
}

bool DDLParser::check(Type type) const {
    return currentToken_.getType() == type;
}

Token DDLParser::peek() const {
    if (hasLookahead_) {
        return lookaheadToken_;
    }
    // This is a simplified version - in real implementation would need to lookahead
    return currentToken_;
}

Token DDLParser::previous() const {
    // Simplified - would need to track previous token
    return currentToken_;
}

void DDLParser::reportError(const std::string& message) {
    std::cerr << "[DDL_PARSER ERROR] " << message << std::endl;
}

std::vector<std::string> DDLParser::parseColumnNames() {
    std::vector<std::string> columns;
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!first && !match(Type::COMMA)) break;
        first = false;
        columns.push_back(parseIdentifier());
    }
    return columns;
}

bool DDLParser::isAtEnd() const {
    return currentToken_.getType() == Type::END_OF_INPUT;
}

} // namespace sql_parser
} // namespace sqlcc