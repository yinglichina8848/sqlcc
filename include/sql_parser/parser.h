#ifndef SQLCC_SQL_PARSER_PARSER_H
#define SQLCC_SQL_PARSER_PARSER_H

#include "sql_parser/ast_nodes.h"
#include "sql_parser/constraint.h"
#include "sql_parser/set_operation.h"
#include "sql_parser/token.h"
#include "sql_parser/window_function.h"
#include "sql_parser/lexer.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// Error handling
void reportError(const std::string &message);

class Parser {
public:
  Parser(const std::string &input);
  ~Parser() = default;

  std::vector<std::unique_ptr<Statement>> parse();

private:
  // Token stream management
  Lexer lexer_;
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
  void reportError(const std::string &message);
  void synchronize();
  bool hadError() const;

  // Statement parsing (strict BNF compliance)
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<CreateStatement> parseCreateStatement();
  std::unique_ptr<CreateStatement> parseCreateTableStatement();
  std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();
  std::unique_ptr<DropStatement> parseDropStatement();
  std::unique_ptr<AlterStatement> parseAlterStatement();
  std::unique_ptr<SelectStatement> parseSelectStatement();
  std::unique_ptr<InsertStatement> parseInsertStatement();
  std::unique_ptr<UpdateStatement> parseUpdateStatement();
  std::unique_ptr<DeleteStatement> parseDeleteStatement();
  std::unique_ptr<UseStatement> parseUseStatement();
  std::unique_ptr<ShowStatement> parseShowStatement();
  std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();
  std::unique_ptr<DropIndexStatement> parseDropIndexStatement();
  std::unique_ptr<CreateUserStatement> parseCreateUserStatement();
  std::unique_ptr<DropUserStatement> parseDropUserStatement();
  std::unique_ptr<GrantStatement> parseGrantStatement();
  std::unique_ptr<RevokeStatement> parseRevokeStatement();

  // Clause parsing
  std::vector<std::string> parseColumnNames();
  std::vector<std::unique_ptr<Expression>> parseExpressions();
  std::unique_ptr<Expression> parseExpression();
  std::unique_ptr<Expression> parseLogicalOr();
  std::unique_ptr<Expression> parseLogicalAnd();
  std::unique_ptr<Expression> parseEquality();
  std::unique_ptr<Expression> parseComparison();
  std::unique_ptr<Expression> parseTerm();
  std::unique_ptr<Expression> parseFactor();
  std::unique_ptr<Expression> parseUnary();
  std::unique_ptr<Expression> parsePrimary();
  std::unique_ptr<Expression> parseIdentifierExpression();

  // JOIN clause parsing
  std::unique_ptr<JoinClause> parseJoinClause();

  std::vector<std::unique_ptr<ColumnDefinition>> parseColumnDefinitions();
  std::unique_ptr<ColumnDefinition> parseColumnDefinition();
  std::string parseDataType();
  std::string parseDefaultValue();
  void parseTableConstraint(CreateStatement& stmt);

  // Helper methods
  void initializeSyncTokens();
  std::string parseQualifiedName();
  std::string parseIdentifier();
  std::string parseStringLiteral();
  int parseIntLiteral();

  // Set operation parsing
  std::unique_ptr<Statement> parseCompositeSelectStatement();
  std::unique_ptr<SetOperation> parseSetOperation();
  std::unique_ptr<SetOperation> parseUnion();
  std::unique_ptr<SetOperation> parseIntersect();
  std::unique_ptr<SetOperation> parseExcept();
  // Helpers for set-operation parsing
  SetOperationType parseSetOperationType();
  bool isSetOperation() const;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSER_H
