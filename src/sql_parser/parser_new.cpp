#include "sql_parser/parser_new.h"
#include "sql_parser/set_operation_node.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

ParserNew::ParserNew(const std::string &input)
    : lexer_(input), hasLookahead_(false), panicMode_(false) {
  try {
    std::cout << "[PARSER DEBUG] ParserNew构造函数开始，输入长度: "
              << input.length() << ", 输入内容: '" << input << "'" << std::endl;
    initializeSyncTokens();
    std::cout << "[PARSER DEBUG] 准备调用advance()获取第一个token" << std::endl;
    advance(); // Get first token
    std::cout << "[PARSER DEBUG] ParserNew构造函数完成，当前token类型: "
              << static_cast<int>(currentToken_.getType()) << std::endl;
  } catch (const std::exception &e) {
    std::cout << "[PARSER DEBUG] ParserNew构造函数异常: " << e.what()
              << std::endl;
    std::cout << "[PARSER DEBUG] 重新抛出异常..." << std::endl;
    throw;
  }
}

std::vector<std::unique_ptr<Statement>> ParserNew::parse() {
  std::vector<std::unique_ptr<Statement>> statements;

  std::cout << "[PARSER DEBUG] 开始解析SQL语句" << std::endl;
  std::cout << "[PARSER DEBUG] 当前token在parse开始时: "
            << currentToken_.getLexeme()
            << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;
  std::cout << "[PARSER DEBUG] 准备检查isAtEnd()" << std::endl;
  std::cout << "[PARSER DEBUG] 解析循环开始，isAtEnd(): "
            << (isAtEnd() ? "true" : "false") << std::endl;

  while (!isAtEnd()) {
    std::cout << "[PARSER DEBUG] 进入解析循环，当前token: "
              << currentToken_.getLexeme()
              << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
              << std::endl;
    try {
      std::cout << "[PARSER DEBUG] 当前token: " << currentToken_.getLexeme()
                << " (类型: " << static_cast<int>(currentToken_.getType())
                << ")" << std::endl;

      if (match(Token::SEMICOLON)) {
        std::cout << "[PARSER DEBUG] 跳过空语句，继续循环" << std::endl;
        continue; // Skip empty statements
      }

      // 记录当前token
      Token current = currentToken_;

      std::cout << "[PARSER DEBUG] 准备调用parseStatement()方法" << std::endl;

      // 检查当前token是否为SEMICOLON
      if (current.getType() == Token::SEMICOLON) {
        std::cout << "[PARSER DEBUG] 跳过空语句" << std::endl;
        continue;
      }

      std::cout << "[PARSER DEBUG] 调用parseStatement()方法" << std::endl;
      auto stmt = parseStatement();
      std::cout << "[PARSER DEBUG] parseStatement()返回，stmt是否为空: "
                << (stmt ? "否" : "是") << std::endl;
      if (stmt) {
        std::cout << "[PARSER DEBUG] 成功解析语句" << std::endl;
        statements.push_back(std::move(stmt));
      } else {
        std::cout << "[PARSER DEBUG] 解析语句失败" << std::endl;
      }

      // 如果解析失败且没有消费任何token，强制前进一个token
      if (current.getType() == currentToken_.getType()) {
        std::cout << "[PARSER DEBUG] 强制前进token" << std::endl;
        advance();
      }

      // Skip semicolon if present
      match(Token::SEMICOLON);
    } catch (const std::runtime_error &e) {
      std::cout << "[PARSER DEBUG] 解析异常: " << e.what() << std::endl;
      reportError(e.what());
      synchronize();
      if (panicMode_) {
        break; // Stop parsing if in panic mode
      }
    }
  }

  std::cout << "[PARSER DEBUG] 解析完成，语句数量: " << statements.size()
            << std::endl;
  return statements;
}

// Core parsing methods
void ParserNew::advance() {
  std::cout << "[PARSER DEBUG] advance()方法被调用" << std::endl;
  if (hasLookahead_) {
    currentToken_ = lookaheadToken_;
    hasLookahead_ = false;
    std::cout << "[PARSER DEBUG] 使用lookahead token: "
              << currentToken_.getLexeme() << std::endl;
  } else {
    std::cout << "[PARSER DEBUG] 调用lexer_.nextToken()" << std::endl;
    currentToken_ = lexer_.nextToken();
    std::cout << "[PARSER DEBUG] 获取到token: '" << currentToken_.getLexeme()
              << "' (类型: " << static_cast<int>(currentToken_.getType()) << ")"
              << std::endl;
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

  std::string expected = std::to_string(type);
  std::string actual = std::to_string(currentToken_.getType());
  throw std::runtime_error("Expected token '" + expected + "', but found '" +
                           actual + "'");
}

bool ParserNew::check(Token::Type type) const {
  if (isAtEnd())
    return false;
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
void ParserNew::reportError(const std::string &message) {
  std::stringstream ss;
  ss << "Parse error at line " << currentToken_.getLine() << ", column "
     << currentToken_.getColumn() << ": " << message;
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

bool ParserNew::hadError() const { return !errors_.empty(); }

void ParserNew::initializeSyncTokens() {
  syncTokens_ = {
      Token::KEYWORD_SELECT, Token::KEYWORD_INSERT, Token::KEYWORD_UPDATE,
      Token::KEYWORD_DELETE, Token::KEYWORD_CREATE, Token::KEYWORD_DROP,
      Token::KEYWORD_ALTER,  Token::KEYWORD_GRANT,  Token::KEYWORD_REVOKE,
      Token::KEYWORD_SHOW,   Token::KEYWORD_COMMIT, Token::KEYWORD_ROLLBACK};
}

// Statement parsing (strict BNF compliance)
std::unique_ptr<Statement> ParserNew::parseStatement() {
  std::cout << "[PARSER DEBUG] parseStatement() 开始，token: "
            << currentToken_.getLexeme()
            << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;

  if (check(Token::KEYWORD_SELECT) || check(Token::LPAREN)) {
    std::cout << "[PARSER DEBUG] 检测到SELECT，调用parseDMLStatement()"
              << std::endl;
    return parseDMLStatement();
  } else if (check(Token::KEYWORD_CREATE)) {
    std::cout << "[PARSER DEBUG] 检测到CREATE，调用parseDDLStatement()"
              << std::endl;
    return parseDDLStatement();
  } else if (match(Token::KEYWORD_INSERT)) {
    std::cout << "[PARSER DEBUG] 检测到INSERT，调用parseInsertStatement()"
              << std::endl;
    return parseInsertStatement();  
  } else if (check(Token::KEYWORD_UPDATE)) {
    std::cout << "[PARSER DEBUG] 检测到UPDATE，调用parseDMLStatement()"
              << std::endl;
    return parseDMLStatement();
  } else if (check(Token::KEYWORD_DELETE)) {
    std::cout << "[PARSER DEBUG] 检测到DELETE，调用parseDMLStatement()"
              << std::endl;
    return parseDMLStatement();
  } else if (check(Token::KEYWORD_DROP)) {
    std::cout << "[PARSER DEBUG] 检测到DROP，调用parseDDLStatement()"
              << std::endl;
    return parseDDLStatement();
  } else if (check(Token::KEYWORD_ALTER)) {
    std::cout << "[PARSER DEBUG] 检测到ALTER，调用parseDDLStatement()"
              << std::endl;
    return parseDDLStatement();
  } else if (check(Token::KEYWORD_USE)) {
    std::cout << "[PARSER DEBUG] 检测到USE，调用parseUseStatement()"
              << std::endl;
    return parseUseStatement();
  } else if (check(Token::KEYWORD_GRANT)) {
    std::cout << "[PARSER DEBUG] 检测到GRANT，调用parseDCLStatement()"
              << std::endl;
    return parseDCLStatement();
  } else if (check(Token::KEYWORD_REVOKE)) {
    std::cout << "[PARSER DEBUG] 检测到REVOKE，调用parseDCLStatement()"
              << std::endl;
    return parseDCLStatement();
  } else if (check(Token::KEYWORD_COMMIT)) {
    std::cout << "[PARSER DEBUG] 检测到COMMIT，调用parseTCLStatement()"
              << std::endl;
    return parseTCLStatement();
  } else if (check(Token::KEYWORD_ROLLBACK)) {
    std::cout << "[PARSER DEBUG] 检测到ROLLBACK，调用parseTCLStatement()"
              << std::endl;
    return parseTCLStatement();
  } else if (check(Token::KEYWORD_SHOW)) {
    std::cout << "[PARSER DEBUG] 检测到SHOW，调用parseShowStatement()"
              << std::endl;
    return parseShowStatement();
  } else {
    std::string error = "Unexpected token: " + currentToken_.getLexeme();
    std::cout << "[PARSER DEBUG] parseStatement() 失败: " << error << std::endl;
    reportError(error);
    return nullptr;
  }
}

std::unique_ptr<Statement> ParserNew::parseDDLStatement() {
  if (currentToken_.getType() == Token::KEYWORD_CREATE) {
    advance(); // Consume CREATE

    if (match(Token::KEYWORD_DATABASE)) {
      return parseCreateDatabaseStatement();
    } else if (match(Token::KEYWORD_TABLE)) {
      return parseCreateTableStatement();
    } else if (match(Token::KEYWORD_INDEX)) {
      return parseCreateIndexStatement();
    } else if (match(Token::KEYWORD_USER)) {
      // CREATE USER is handled by DCL parser
      std::cout << "[PARSER DEBUG] 检测到CREATE USER，转交给DCL解析器"
                << std::endl;
      return parseCreateUserStatement();
    } else {
      reportError("Unknown CREATE statement type");
      return nullptr;
    }
  } else if (match(Token::KEYWORD_DROP)) {
    // Implement DROP statement parsing
    return parseDropStatement();
  } else if (match(Token::KEYWORD_ALTER)) {
    // TODO: Implement ALTER statement parsing
    reportError("ALTER statements not yet implemented");
    return nullptr;
  } else {
    reportError("Unknown DDL statement type");
    return nullptr;
  }
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
  std::cout << "[PARSER DEBUG] parseDCLStatement() 开始，token: "
            << currentToken_.getLexeme()
            << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;

  if (match(Token::KEYWORD_CREATE)) {
    // CREATE USER
    if (match(Token::KEYWORD_USER)) {
      return parseCreateUserStatement();
    } else {
      reportError("Expected USER after CREATE in DCL statement");
      return nullptr;
    }
  } else if (match(Token::KEYWORD_DROP)) {
    // DROP USER
    if (match(Token::KEYWORD_USER)) {
      return parseDropUserStatement();
    } else {
      reportError("Expected USER after DROP in DCL statement");
      return nullptr;
    }
  } else if (match(Token::KEYWORD_GRANT)) {
    // GRANT statement
    return parseGrantStatement();
  } else if (match(Token::KEYWORD_REVOKE)) {
    // REVOKE statement
    return parseRevokeStatement();
  } else {
    reportError("Unknown DCL statement type");
    return nullptr;
  }
}

std::unique_ptr<CreateUserStatement> ParserNew::parseCreateUserStatement() {
  std::cout << "[PARSER DEBUG] parseCreateUserStatement() 开始" << std::endl;

  // 用户名（标识符）
  std::string username = parseIdentifier();
  std::cout << "[PARSER DEBUG] parseCreateUserStatement() - 用户名: "
            << username << std::endl;

  std::string password = "";
  bool withPassword = false;

  // 处理IDENTIFIED BY或WITH PASSWORD子句
  if (match(Token::KEYWORD_IDENTIFIED)) {
    std::cout
        << "[PARSER DEBUG] parseCreateUserStatement() - 检测到IDENTIFIED关键字"
        << std::endl;
    consume(Token::KEYWORD_BY);
    std::cout << "[PARSER DEBUG] parseCreateUserStatement() - 已消费BY关键字"
              << std::endl;

    // 检查是否为字符串字面量
    if (check(Token::STRING_LITERAL)) {
      password = parseStringLiteral();
    } else {
      // 处理不带引号的密码
      std::string rawPassword = parseIdentifier();
      password = rawPassword;
    }

    std::cout << "[PARSER DEBUG] parseCreateUserStatement() - 密码: "
              << password << std::endl;
    withPassword = false; // IDENTIFIED BY 不设置withPassword标志
  } else if (match(Token::KEYWORD_WITH)) {
    std::cout << "[PARSER DEBUG] parseCreateUserStatement() - 检测到WITH关键字"
              << std::endl;
    consume(Token::KEYWORD_PASSWORD);
    std::cout
        << "[PARSER DEBUG] parseCreateUserStatement() - 已消费PASSWORD关键字"
        << std::endl;

    // 检查是否为字符串字面量
    if (check(Token::STRING_LITERAL)) {
      password = parseStringLiteral();
    } else {
      // 处理不带引号的密码
      std::string rawPassword = parseIdentifier();
      password = rawPassword;
    }

    std::cout << "[PARSER DEBUG] parseCreateUserStatement() - 密码: "
              << password << std::endl;
    withPassword = true; // WITH PASSWORD 设置withPassword标志
  }

  auto stmt = std::make_unique<CreateUserStatement>(username, password);
  stmt->setWithPassword(withPassword);

  std::cout << "[PARSER DEBUG] parseCreateUserStatement() 完成，用户: "
            << username << ", 密码: " << password
            << ", withPassword: " << withPassword << std::endl;
  return stmt;
}

std::unique_ptr<DropUserStatement> ParserNew::parseDropUserStatement() {
  std::cout << "[PARSER DEBUG] parseDropUserStatement() 开始" << std::endl;

  // 处理可选的IF EXISTS
  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume(Token::KEYWORD_EXISTS);
    ifExists = true;
    std::cout << "[PARSER DEBUG] parseDropUserStatement() - 检测到IF EXISTS"
              << std::endl;
  }

  // 用户名
  std::string username = parseIdentifier();
  std::cout << "[PARSER DEBUG] parseDropUserStatement() - 用户名: " << username
            << std::endl;

  auto stmt = std::make_unique<DropUserStatement>(username);
  stmt->setIfExists(ifExists);

  std::cout << "[PARSER DEBUG] parseDropUserStatement() 完成，用户: "
            << username << std::endl;
  return stmt;
}

std::unique_ptr<GrantStatement> ParserNew::parseGrantStatement() {
  std::cout << "[PARSER DEBUG] parseGrantStatement() 开始" << std::endl;

  auto stmt = std::make_unique<GrantStatement>();

  // 解析权限列表
  std::string privilege;
  do {
    if (check(Token::KEYWORD_ALL)) {
      // GRANT ALL PRIVILEGES
      privilege = "ALL";
      advance();
    } else if (check(Token::IDENTIFIER)) {
      privilege = parseIdentifier();
    } else {
      reportError("Expected privilege name or ALL");
      return nullptr;
    }
    stmt->addPrivilege(privilege);
    std::cout << "[PARSER DEBUG] parseGrantStatement() - 添加权限: "
              << privilege << std::endl;
  } while (match(Token::COMMA));

  // 检查PRIVILEGES关键字（可选）
  if (match(Token::KEYWORD_PRIVILEGES)) {
    std::cout << "[PARSER DEBUG] parseGrantStatement() - 已消费PRIVILEGES关键字"
              << std::endl;
  }

  // ON关键字
  consume(Token::KEYWORD_ON);
  std::cout << "[PARSER DEBUG] parseGrantStatement() - 已消费ON关键字"
            << std::endl;

  // 解析对象类型和名称
  if (match(Token::KEYWORD_DATABASE)) {
    // ON DATABASE database_name
    std::string dbName = parseIdentifier();
    stmt->setObjectType("DATABASE");
    stmt->setObjectName(dbName);
    std::cout << "[PARSER DEBUG] parseGrantStatement() - 数据库权限: " << dbName
              << std::endl;
  } else if (match(Token::KEYWORD_TABLE)) {
    // ON TABLE table_name
    std::string tableName = parseIdentifier();
    stmt->setObjectType("TABLE");
    stmt->setObjectName(tableName);
    std::cout << "[PARSER DEBUG] parseGrantStatement() - 表权限: " << tableName
              << std::endl;
  } else if (match(Token::IDENTIFIER)) {
    // 直接是表名
    std::string tableName = previous().getLexeme();
    stmt->setObjectType("TABLE");
    stmt->setObjectName(tableName);
    std::cout << "[PARSER DEBUG] parseGrantStatement() - 表权限: " << tableName
              << std::endl;
  } else {
    reportError("Expected DATABASE, TABLE, or object name after ON");
    return nullptr;
  }

  // TO关键字
  consume(Token::KEYWORD_TO);
  std::cout << "[PARSER DEBUG] parseGrantStatement() - 已消费TO关键字"
            << std::endl;

  // 解析被授权用户
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);
  std::cout << "[PARSER DEBUG] parseGrantStatement() - 被授权用户: " << grantee
            << std::endl;

  std::cout << "[PARSER DEBUG] parseGrantStatement() 完成" << std::endl;
  return stmt;
}

std::unique_ptr<RevokeStatement> ParserNew::parseRevokeStatement() {
  std::cout << "[PARSER DEBUG] parseRevokeStatement() 开始" << std::endl;

  auto stmt = std::make_unique<RevokeStatement>();

  // 解析权限列表
  std::string privilege;
  do {
    if (check(Token::KEYWORD_ALL)) {
      // REVOKE ALL PRIVILEGES
      privilege = "ALL";
      advance();
    } else if (check(Token::IDENTIFIER)) {
      privilege = parseIdentifier();
    } else {
      reportError("Expected privilege name or ALL");
      return nullptr;
    }
    stmt->addPrivilege(privilege);
    std::cout << "[PARSER DEBUG] parseRevokeStatement() - 添加权限: "
              << privilege << std::endl;
  } while (match(Token::COMMA));

  // 检查PRIVILEGES关键字（可选）
  if (match(Token::KEYWORD_PRIVILEGES)) {
    std::cout
        << "[PARSER DEBUG] parseRevokeStatement() - 已消费PRIVILEGES关键字"
        << std::endl;
  }

  // ON关键字
  consume(Token::KEYWORD_ON);
  std::cout << "[PARSER DEBUG] parseRevokeStatement() - 已消费ON关键字"
            << std::endl;

  // 解析对象类型和名称
  if (match(Token::KEYWORD_DATABASE)) {
    // ON DATABASE database_name
    std::string dbName = parseIdentifier();
    stmt->setObjectType("DATABASE");
    stmt->setObjectName(dbName);
    std::cout << "[PARSER DEBUG] parseRevokeStatement() - 数据库权限: "
              << dbName << std::endl;
  } else if (match(Token::KEYWORD_TABLE)) {
    // ON TABLE table_name
    std::string tableName = parseIdentifier();
    stmt->setObjectType("TABLE");
    stmt->setObjectName(tableName);
    std::cout << "[PARSER DEBUG] parseRevokeStatement() - 表权限: " << tableName
              << std::endl;
  } else if (match(Token::IDENTIFIER)) {
    // 直接是表名
    std::string tableName = previous().getLexeme();
    stmt->setObjectType("TABLE");
    stmt->setObjectName(tableName);
    std::cout << "[PARSER DEBUG] parseRevokeStatement() - 表权限: " << tableName
              << std::endl;
  } else {
    reportError("Expected DATABASE, TABLE, or object name after ON");
    return nullptr;
  }

  // FROM关键字
  consume(Token::KEYWORD_FROM);
  std::cout << "[PARSER DEBUG] parseRevokeStatement() - 已消费FROM关键字"
            << std::endl;

  // 解析被剥夺权限的用户
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);
  std::cout << "[PARSER DEBUG] parseRevokeStatement() - 被剥夺权限用户: "
            << grantee << std::endl;

  std::cout << "[PARSER DEBUG] parseRevokeStatement() 完成" << std::endl;
  return stmt;
}

std::unique_ptr<Statement> ParserNew::parseTCLStatement() {
  if (match(Token::KEYWORD_COMMIT)) {
    // Simplified COMMIT implementation
    // Create a dummy statement for now
    return nullptr;
  } else if (match(Token::KEYWORD_ROLLBACK)) {
    // Simplified ROLLBACK implementation
    // Create a dummy statement for now
    return nullptr;
  } else {
    reportError("Unknown TCL statement");
    return nullptr;
  }
}

std::unique_ptr<Statement> ParserNew::parseShowStatement() {
  try {
    // 当前token应该是SHOW关键字，已在调用方消费
    // 检查SHOW语句的类型
    if (match(Token::KEYWORD_DATABASES)) {
      return std::make_unique<ShowStatement>(ShowStatement::DATABASES);
    } else if (match(Token::KEYWORD_TABLES)) {
      auto stmt = std::make_unique<ShowStatement>(ShowStatement::TABLES);
      // 可选：解析 FROM database_name
      if (match(Token::KEYWORD_FROM)) {
        std::string dbName = parseIdentifier();
        stmt->setFromDatabase(dbName);
      }
      return stmt;
    } else if (match(Token::KEYWORD_CREATE)) {
      consume(Token::KEYWORD_TABLE);
      std::string tableName = parseIdentifier();
      auto stmt = std::make_unique<ShowStatement>(ShowStatement::CREATE_TABLE);
      stmt->setTargetObject(tableName);
      return stmt;
    } else if (match(Token::KEYWORD_COLUMNS)) {
      consume(Token::KEYWORD_FROM);
      std::string tableName = parseIdentifier();
      auto stmt = std::make_unique<ShowStatement>(ShowStatement::COLUMNS);
      stmt->setTargetObject(tableName);
      return stmt;
    } else if (match(Token::KEYWORD_INDEXES)) {
      consume(Token::KEYWORD_FROM);
      std::string tableName = parseIdentifier();
      auto stmt = std::make_unique<ShowStatement>(ShowStatement::INDEXES);
      stmt->setTargetObject(tableName);
      return stmt;
    } else if (match(Token::KEYWORD_GRANTS)) {
      consume(Token::KEYWORD_FOR);
      std::string username = parseIdentifier();
      auto stmt = std::make_unique<ShowStatement>(ShowStatement::GRANTS);
      stmt->setTargetObject(username);
      return stmt;
    } else {
      reportError("Unknown SHOW statement type");
      return nullptr;
    }
  } catch (const std::exception &e) {
    reportError("Error parsing SHOW statement: " + std::string(e.what()));
    return nullptr;
  }
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
  auto stmt =
      std::make_unique<CreateStatement>(CreateStatement::TABLE, tableName);

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

  auto stmt =
      std::make_unique<CreateIndexStatement>(indexName, tableName, columnName);
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
  std::cout << "[PARSER DEBUG] parseInsertStatement() 开始" << std::endl;

  try {
    // 注意：INSERT关键字已经在parseStatement()中被消费，这里不再消费
    consume(Token::KEYWORD_INTO);
    std::cout << "[PARSER DEBUG] parseInsertStatement() - 已消费INTO关键字"
              << std::endl;

    std::string tableName = parseIdentifier();
    std::cout << "[PARSER DEBUG] parseInsertStatement() - 表名: " << tableName
              << std::endl;

    auto stmt = std::make_unique<InsertStatement>(tableName);
    std::cout
        << "[PARSER DEBUG] parseInsertStatement() - 创建InsertStatement对象"
        << std::endl;

    // Parse optional column list
    if (match(Token::LPAREN)) {
      std::cout
          << "[PARSER DEBUG] parseInsertStatement() - 检测到左括号，开始解析列"
          << std::endl;
      parseInsertColumns(*stmt);
      std::cout << "[PARSER DEBUG] parseInsertStatement() - 列解析完成"
                << std::endl;
      consume(Token::RPAREN);
      std::cout << "[PARSER DEBUG] parseInsertStatement() - 已消费右括号"
                << std::endl;
    }

    // Parse VALUES clause
    std::cout << "[PARSER DEBUG] parseInsertStatement() - 准备解析VALUES关键字"
              << std::endl;
    consume(Token::KEYWORD_VALUES);
    std::cout << "[PARSER DEBUG] parseInsertStatement() - 已消费VALUES关键字"
              << std::endl;

    consume(Token::LPAREN);
    std::cout
        << "[PARSER DEBUG] parseInsertStatement() - 已消费VALUES后的左括号"
        << std::endl;

    parseInsertValues(*stmt);
    std::cout << "[PARSER DEBUG] parseInsertStatement() - VALUES解析完成"
              << std::endl;

    consume(Token::RPAREN);
    std::cout << "[PARSER DEBUG] parseInsertStatement() - "
                 "已消费VALUES后的右括号，返回语句"
              << std::endl;

    std::cout << "[PARSER DEBUG] parseInsertStatement() 完成，解析了"
              << stmt->getValues().size() << "行值" << std::endl;

    // 消费可选的分号
    match(Token::SEMICOLON);
    
    return stmt;
  } catch (const std::exception &e) {
    std::cout << "[PARSER DEBUG] parseInsertStatement() 异常: " << e.what()
              << std::endl;
    reportError(e.what());
    return nullptr;
  }
}std::unique_ptr<UpdateStatement> ParserNew::parseUpdateStatement() {
  // 注意：UPDATE关键字已经在parseDMLStatement()中被消费，这里不再消费
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
  std::cout << "[PARSER DEBUG] parseDeleteStatement() 开始" << std::endl;
  std::cout << "[PARSER DEBUG] parseDeleteStatement() - 进入try块" << std::endl;

  try {
    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 当前token: "
              << currentToken_.getLexeme()
              << " (类型: " << currentToken_.getType() << ")" << std::endl;
    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 输出token信息成功"
              << std::endl;

    // 注意：DELETE关键字已经在parseDMLStatement()中被消费，这里不再消费
    std::cout << "[PARSER DEBUG] parseDeleteStatement() - "
                 "DELETE关键字已被消费，直接消费FROM关键字"
              << std::endl;

    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 开始消费FROM关键字"
              << std::endl;
    consume(Token::KEYWORD_FROM);
    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 已消费FROM关键字"
              << std::endl;

    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 开始解析表名"
              << std::endl;
    std::string tableName = parseIdentifier();
    std::cout << "[PARSER DEBUG] parseDeleteStatement() - 表名: " << tableName
              << std::endl;

    std::cout
        << "[PARSER DEBUG] parseDeleteStatement() - 开始创建DeleteStatement对象"
        << std::endl;
    auto stmt = std::make_unique<DeleteStatement>(tableName);
    std::cout
        << "[PARSER DEBUG] parseDeleteStatement() - 已创建DeleteStatement对象"
        << std::endl;

    if (match(Token::KEYWORD_WHERE)) {
      std::cout << "[PARSER DEBUG] parseDeleteStatement() - 解析WHERE子句"
                << std::endl;
      auto whereExpr = parseExpression();
      // Convert expression to WhereClause (simplified)
      stmt->setWhereClause(WhereClause("", "=", ""));
      std::cout << "[PARSER DEBUG] parseDeleteStatement() - WHERE子句解析完成"
                << std::endl;
    }

    std::cout << "[PARSER DEBUG] parseDeleteStatement() 完成，返回语句"
              << std::endl;
    return stmt;

  } catch (const std::exception &e) {
    std::cout << "[PARSER DEBUG] parseDeleteStatement() 异常: " << e.what()
              << std::endl;
    return nullptr;
  }
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
  std::cout << "[PARSER DEBUG] parseStringLiteral() 开始" << std::endl;

  if (!check(Token::STRING_LITERAL)) {
    reportError("Expected string literal");
    return "";
  }

  // Get the raw string from the token
  std::string result = currentToken_.getLexeme();
  std::cout << "[PARSER DEBUG] parseStringLiteral() - 原始token内容: '"
            << result << "'" << std::endl;
  std::cout << "[PARSER DEBUG] parseStringLiteral() - token长度: "
            << result.length() << std::endl;
  std::cout << "[PARSER DEBUG] parseStringLiteral() - 第一个字符: "
            << static_cast<int>(result.front()) << std::endl;
  std::cout << "[PARSER DEBUG] parseStringLiteral() - 最后一个字符: "
            << static_cast<int>(result.back()) << std::endl;

  // Remove quotes if they exist
  if (result.length() >= 2 &&
      ((result.front() == '\'' && result.back() == '\'') ||
       (result.front() == '"' && result.back() == '"'))) {
    std::string before = result;
    result = result.substr(1, result.length() - 2);
    std::cout << "[PARSER DEBUG] parseStringLiteral() - 去除引号前: '" << before
              << "'" << std::endl;
    std::cout << "[PARSER DEBUG] parseStringLiteral() - 去除引号后: '" << result
              << "'" << std::endl;
  }

  // The lexer has already processed escape sequences, so we can return the
  // result as is
  advance();
  std::cout << "[PARSER DEBUG] parseStringLiteral() - 返回: '" << result << "'"
            << std::endl;
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

void ParserNew::parseColumnConstraints(ColumnDefinition &column) {
  while (isColumnConstraint()) {
    parseColumnConstraint(column);
  }
}

bool ParserNew::parseColumnConstraint(ColumnDefinition &column) {
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
         check(Token::KEYWORD_AUTO_INCREMENT) ||
         check(Token::KEYWORD_DEFAULT) || check(Token::KEYWORD_REFERENCES) ||
         check(Token::KEYWORD_CHECK);
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
    (void)op; // Mark as used to avoid warning
    auto right = parseUnaryExpression();
    // Create binary expression (simplified)
    expr = std::move(right); // Placeholder
  }

  return expr;
}

std::unique_ptr<Expression> ParserNew::parseUnaryExpression() {
  if (match(Token::OPERATOR_PLUS) || match(Token::OPERATOR_MINUS) ||
      match(Token::KEYWORD_NOT)) {
    Token::Type op = previous().getType();
    (void)op; // Mark as used to avoid warning
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
    // Skip string literal for now
    parseStringLiteral();
    return nullptr;
  } else if (check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
    // Skip numeric literal for now
    parseNumericLiteral();
    return nullptr;
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
void ParserNew::parseSelectList(SelectStatement &stmt) {
  if (match(Token::OPERATOR_MULTIPLY)) {
    stmt.setSelectAll(true);
  } else {
    do {
      std::string columnName = parseIdentifier();
      stmt.addSelectColumn(columnName);
    } while (match(Token::COMMA));
  }
}

void ParserNew::parseFromClause(SelectStatement &stmt) {
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

void ParserNew::parseWhereClause(SelectStatement &stmt) {
  (void)stmt; // Mark as used to avoid warning
  auto expr = parseExpression();
  // Convert to WhereClause (simplified)
}

void ParserNew::parseGroupByClause(SelectStatement &stmt) {
  do {
    std::string columnName = parseIdentifier();
    stmt.setGroupByColumn(columnName);
  } while (match(Token::COMMA));
}

void ParserNew::parseHavingClause(SelectStatement &stmt) {
  (void)stmt; // Mark as used to avoid warning
  auto expr = parseExpression();
  // Handle HAVING clause
}

void ParserNew::parseOrderByClause(SelectStatement &stmt) {
  do {
    std::string columnName = parseIdentifier();
    stmt.setOrderByColumn(columnName);

    if (match(Token::KEYWORD_ASC) || match(Token::KEYWORD_DESC)) {
      std::string direction =
          (previous().getType() == Token::KEYWORD_ASC) ? "ASC" : "DESC";
      stmt.setOrderDirection(direction);
    }
  } while (match(Token::COMMA));
}

void ParserNew::parseLimitOffsetClause(SelectStatement &stmt) {
  int limit = static_cast<int>(parseIntegerLiteral());
  stmt.setLimit(limit);

  if (match(Token::KEYWORD_OFFSET) || match(Token::COMMA)) {
    int offset = static_cast<int>(parseIntegerLiteral());
    stmt.setOffset(offset);
  }
}

void ParserNew::parseInsertColumns(InsertStatement &stmt) {
  do {
    std::string columnName = parseIdentifier();
    stmt.addColumn(columnName);
  } while (match(Token::COMMA));
}

void ParserNew::parseInsertValues(InsertStatement &stmt) {
  std::cout << "[PARSER DEBUG] parseInsertValues() 开始" << std::endl;

  try {
    while (!check(Token::RPAREN) && !isAtEnd()) {
      std::cout << "[PARSER DEBUG] parseInsertValues() - 当前token: "
                << currentToken_.getLexeme()
                << " (类型: " << Token::getTypeName(currentToken_.getType())
                << ")" << std::endl;

      std::string valueStr;

      if (check(Token::STRING_LITERAL)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理字符串字面量"
                  << std::endl;
        valueStr = parseStringLiteral();
        std::cout << "[PARSER DEBUG] parseInsertValues() - 字符串值: "
                  << valueStr << std::endl;
      } else if (check(Token::INTEGER_LITERAL)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理整数字面量"
                  << std::endl;
        long long intValue = parseIntegerLiteral();
        valueStr = std::to_string(intValue);
        std::cout << "[PARSER DEBUG] parseInsertValues() - 整数值: " << intValue
                  << ", 转换为字符串: " << valueStr << std::endl;
      } else if (check(Token::FLOAT_LITERAL)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理浮点字面量"
                  << std::endl;
        double floatValue = parseNumericLiteral();
        valueStr = std::to_string(floatValue);
        std::cout << "[PARSER DEBUG] parseInsertValues() - 浮点值: "
                  << floatValue << ", 转换为字符串: " << valueStr << std::endl;
      } else if (match(Token::KEYWORD_NULL)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理NULL值"
                  << std::endl;
        valueStr = "NULL";
      } else if (match(Token::KEYWORD_TRUE)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理TRUE值"
                  << std::endl;
        valueStr = "TRUE";
      } else if (match(Token::KEYWORD_FALSE)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理FALSE值"
                  << std::endl;
        valueStr = "FALSE";
      } else if (check(Token::IDENTIFIER)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 处理标识符"
                  << std::endl;
        valueStr = parseIdentifier();
        std::cout << "[PARSER DEBUG] parseInsertValues() - 标识符: " << valueStr
                  << std::endl;
      } else {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 遇到无效值类型"
                  << std::endl;
        reportError("Invalid value in INSERT statement: " +
                    currentToken_.getLexeme());
        advance();
        continue;
      }

      // 添加值到语句
      std::cout << "[PARSER DEBUG] parseInsertValues() - 添加值到语句: "
                << valueStr << std::endl;
      stmt.addValue(valueStr);

      // 处理逗号或右括号
      if (match(Token::COMMA)) {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 跳过逗号"
                  << std::endl;
        // 继续循环处理下一个值
        continue;
      } else if (check(Token::RPAREN)) {
        std::cout
            << "[PARSER DEBUG] parseInsertValues() - 遇到右括号，结束值解析"
            << std::endl;
        // 退出循环
        break;
      } else {
        std::cout << "[PARSER DEBUG] parseInsertValues() - 遇到意外token: "
                  << currentToken_.getLexeme() << std::endl;
        // 如果不是逗号也不是右括号，可能是语法错误，记录错误并跳过
        reportError("Expected comma or right parenthesis in INSERT VALUES");
        advance();
      }
    }

    std::cout << "[PARSER DEBUG] parseInsertValues() 完成，解析了"
              << stmt.getValues().size() << "行值" << std::endl;
  } catch (const std::exception &e) {
    std::cout << "[PARSER DEBUG] parseInsertValues() 异常: " << e.what()
              << std::endl;
    reportError(e.what());
  }
}

void ParserNew::parseUpdateSetClause(UpdateStatement &stmt) {
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
  return check(Token::IDENTIFIER) && (currentToken_.getLexeme() == "INT" ||
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

bool ParserNew::isSetOperation() {
  return check(Token::KEYWORD_UNION) || check(Token::KEYWORD_INTERSECT) ||
         check(Token::KEYWORD_EXCEPT);
}

bool ParserNew::isComparisonOperator() const {
  return check(Token::OPERATOR_EQUAL) || check(Token::OPERATOR_NOT_EQUAL) ||
         check(Token::OPERATOR_LESS_THAN) ||
         check(Token::OPERATOR_LESS_EQUAL) ||
         check(Token::OPERATOR_GREATER_THAN) ||
         check(Token::OPERATOR_GREATER_EQUAL) || check(Token::OPERATOR_LIKE) ||
         check(Token::OPERATOR_IN);
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

void ParserNew::parseJoinClause(SelectStatement &stmt) {
  (void)stmt; // Mark as used to avoid warning
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

std::unique_ptr<SetOperationNode> ParserNew::parseSetOperation() {
  // 解析集合操作类型
  SetOperationType operationType = parseSetOperationType();

  // 解析可选的ALL关键字
  bool allFlag = false;
  if (match(Token::KEYWORD_ALL)) {
    allFlag = true;
  }

  // 解析右侧的SELECT语句
  auto rightOperand = parseSelectStatement();
  if (!rightOperand) {
    reportError("Expected SELECT statement after set operation");
    return nullptr;
  }

  // 创建集合操作节点
  return std::make_unique<SetOperationNode>(operationType, nullptr,
                                            std::move(rightOperand), allFlag);
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

// Add the parseDropStatement method implementation
std::unique_ptr<DropStatement> ParserNew::parseDropStatement() {
  // Handle IF EXISTS
  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume(Token::KEYWORD_EXISTS);
    ifExists = true;
  }

  // Check what type of object to drop
  if (match(Token::KEYWORD_DATABASE)) {
    std::string dbName = parseIdentifier();
    auto stmt = std::make_unique<DropStatement>(DropStatement::DATABASE, dbName);
    stmt->setIfExists(ifExists);
    return stmt;
  } else if (match(Token::KEYWORD_TABLE)) {
    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<DropStatement>(DropStatement::TABLE, tableName);
    stmt->setIfExists(ifExists);
    return stmt;
  } else if (match(Token::KEYWORD_INDEX)) {
    // For INDEX, we need to handle it differently since DropIndexStatement is not a DropStatement
    consume(Token::KEYWORD_INDEX);
    std::string indexName = parseIdentifier();
    // We'll create a special DropStatement for INDEX for now
    // In a more complete implementation, we might want to handle this differently
    auto stmt = std::make_unique<DropStatement>(DropStatement::INDEX, indexName);
    stmt->setIfExists(ifExists);
    return stmt;
  } else {
    reportError("Unknown DROP statement type");
    return nullptr;
  }
}

// Add the parseUseStatement method implementation
std::unique_ptr<UseStatement> ParserNew::parseUseStatement() {
  // Consume USE keyword
  consume(Token::KEYWORD_USE);
  
  // Parse database name
  std::string dbName = parseIdentifier();
  
  // Create and return UseStatement
  return std::make_unique<UseStatement>(dbName);
}

} // namespace sql_parser
} // namespace sqlcc
