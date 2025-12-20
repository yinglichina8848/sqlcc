#include "sql_parser/parser.h"
#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/set_operation.h"
#include "sql_parser/load_data_ast.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace sqlcc {
namespace sql_parser {

Parser::Parser(const std::string& input) 
    : lexer_(input), 
      hasLookahead_(false),
      panicMode_(false) {
    advance(); // Initialize current token
    
    // Initialize synchronization tokens for error recovery
    syncTokens_ = {
        Token::SEMICOLON,
        Token::KEYWORD_SELECT,
        Token::KEYWORD_INSERT,
        Token::KEYWORD_UPDATE,
        Token::KEYWORD_DELETE,
        Token::KEYWORD_CREATE,
        Token::KEYWORD_DROP,
        Token::KEYWORD_ALTER,
        Token::KEYWORD_USE,
        Token::KEYWORD_SHOW,
        Token::KEYWORD_DESCRIBE,
        Token::KEYWORD_COMMIT,
        Token::KEYWORD_ROLLBACK,
        Token::KEYWORD_GRANT,
        Token::KEYWORD_REVOKE,
        Token::KEYWORD_BEGIN,
        Token::KEYWORD_END
    };
}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
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

      std::unique_ptr<Statement> stmt = parseStatement();
      std::cout << "[PARSER DEBUG] parseStatement()返回，stmt是否为空: "
                << (stmt ? "false" : "true") << std::endl;

      if (stmt) {
        statements.push_back(std::move(stmt));
        std::cout << "[PARSER DEBUG] 成功添加语句到statements向量" << std::endl;
      }

      // Consume semicolon if present
      if (check(Token::SEMICOLON)) {
        std::cout << "[PARSER DEBUG] 发现分号，准备消费" << std::endl;
        consume(Token::SEMICOLON);
        std::cout << "[PARSER DEBUG] 分号消费完成" << std::endl;
      }

      std::cout << "[PARSER DEBUG] 循环结束，准备下次迭代" << std::endl;
    } catch (const std::exception &e) {
      std::cout << "[PARSER DEBUG] 解析过程中发生异常: " << e.what()
                << std::endl;
      if (!panicMode_) {
        reportError(e.what());
      }
      synchronize();
    }
  }

  std::cout << "[PARSER DEBUG] 解析循环结束，总共解析了 " << statements.size()
            << " 条语句" << std::endl;
  return statements;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  std::cout << "[PARSER DEBUG] 进入parseStatement()方法" << std::endl;
  std::cout << "[PARSER DEBUG] 当前token: " << currentToken_.getLexeme()
            << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;

  // Check for various statement types
  if (check(Token::KEYWORD_CREATE)) {
    // 检查是否是CREATE VIEW语句
    if (isCreateViewStatement()) {
      std::cout << "[PARSER DEBUG] 检测到CREATE VIEW语句，调用parseCreateViewStatement()"
                << std::endl;
      return parseCreateViewStatement();
    } else {
      std::cout << "[PARSER DEBUG] 检测到其他CREATE语句，调用parseCreateStatement()"
                << std::endl;
      return parseCreateStatement();
    }
  }

  if (check(Token::KEYWORD_DROP)) {
    std::cout << "[PARSER DEBUG] 检测到DROP关键字，调用parseDropStatement()"
              << std::endl;
    return parseDropStatement();
  }

  if (check(Token::KEYWORD_ALTER)) {
    std::cout << "[PARSER DEBUG] 检测到ALTER关键字，调用parseAlterStatement()"
              << std::endl;
    return parseAlterStatement();
  }

  if (check(Token::KEYWORD_SELECT)) {
    std::cout << "[PARSER DEBUG] 检测到SELECT关键字，调用parseSelectStatement()"
              << std::endl;
    return parseSelectStatement();
  }

  if (check(Token::KEYWORD_INSERT)) {
    std::cout << "[PARSER DEBUG] 检测到INSERT关键字，调用parseInsertStatement()"
              << std::endl;
    return parseInsertStatement();
  }

  if (check(Token::KEYWORD_UPDATE)) {
    std::cout << "[PARSER DEBUG] 检测到UPDATE关键字，调用parseUpdateStatement()"
              << std::endl;
    return parseUpdateStatement();
  }

  if (check(Token::KEYWORD_DELETE)) {
    std::cout << "[PARSER DEBUG] 检测到DELETE关键字，调用parseDeleteStatement()"
              << std::endl;
    return parseDeleteStatement();
  }

  if (check(Token::KEYWORD_USE)) {
    std::cout << "[PARSER DEBUG] 检测到USE关键字，调用parseUseStatement()"
              << std::endl;
    return parseUseStatement();
  }

  if (check(Token::KEYWORD_SHOW)) {
    std::cout << "[PARSER DEBUG] 检测到SHOW关键字，调用parseShowStatement()"
              << std::endl;
    return parseShowStatement();
  }
  
  if (check(Token::KEYWORD_GRANT)) {
    std::cout << "[PARSER DEBUG] 检测到GRANT关键字，调用parseGrantStatement()"
              << std::endl;
    return parseGrantStatement();
  }
  
  if (check(Token::KEYWORD_REVOKE)) {
    std::cout << "[PARSER DEBUG] 检测到REVOKE关键字，调用parseRevokeStatement()"
              << std::endl;
    return parseRevokeStatement();
  }

  if (match(Token::KEYWORD_LOAD)) {
    consume(Token::KEYWORD_DATA);
    std::cout << "[PARSER DEBUG] 检测到LOAD DATA语句，调用parseLoadDataStatement()"
              << std::endl;
    return parseLoadDataStatement();
  }

  // If we reach here, we have an unknown statement
  std::cout << "[PARSER DEBUG] 未知语句类型，抛出异常" << std::endl;
  std::stringstream ss;
  ss << "Unknown statement type: " << currentToken_.getLexeme();
  throw std::runtime_error(ss.str());
}

// Implement other parsing methods...

void Parser::advance() {
  if (hasLookahead_) {
    currentToken_ = lookaheadToken_;
    hasLookahead_ = false;
  } else {
    currentToken_ = lexer_.nextToken();
  }
  std::cout << "[PARSER DEBUG] advance() called, new current token: "
            << currentToken_.getLexeme()
            << " (type: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;
}

bool Parser::match(Token::Type type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

void Parser::consume(Token::Type type) {
  if (check(type)) {
    advance();
  } else {
    std::stringstream ss;
    ss << "Expected token " << Token::getTypeName(type) << " but got "
       << Token::getTypeName(currentToken_.getType()) << " ("
       << currentToken_.getLexeme() << ")";
    reportError(ss.str());
  }
}

bool Parser::check(Token::Type type) const {
  if (isAtEnd())
    return false;
  return currentToken_.getType() == type;
}

bool Parser::isAtEnd() const {
  return currentToken_.getType() == Token::END_OF_INPUT;
}

Token Parser::peek() const { return currentToken_; }

Token Parser::previous() const {
  // This is a placeholder implementation
  return currentToken_;
}

void Parser::reportError(const std::string &message) {
  std::string errorMsg = "Parse error at line " +
                         std::to_string(currentToken_.getLine()) + ", column " +
                         std::to_string(currentToken_.getColumn()) + ": " +
                         message;
  std::cout << "[PARSER ERROR] " << errorMsg << std::endl;
  errors_.push_back(errorMsg);
  panicMode_ = true;
}

void Parser::synchronize() {
  panicMode_ = false;

  // Skip tokens until we reach a synchronization point
  while (!isAtEnd()) {
    if (currentToken_.getType() == Token::SEMICOLON) {
      advance();
      return;
    }

    switch (currentToken_.getType()) {
    case Token::KEYWORD_CREATE:
    case Token::KEYWORD_DROP:
    case Token::KEYWORD_ALTER:
    case Token::KEYWORD_SELECT:
    case Token::KEYWORD_INSERT:
    case Token::KEYWORD_UPDATE:
    case Token::KEYWORD_DELETE:
    case Token::KEYWORD_USE:
    case Token::KEYWORD_SHOW:
    case Token::KEYWORD_GRANT:
    case Token::KEYWORD_REVOKE:
      return;

    default:
      advance();
    }
  }
}

bool Parser::hadError() const { return !errors_.empty(); }

// Helper method to check if current statement is CREATE VIEW
bool Parser::isCreateViewStatement() const {
  // Simple check: if current token is CREATE, assume it's CREATE VIEW for now
  // In a full implementation, we would need to lookahead to check the next token
  // Since this is a const method, we can't access currentToken_ directly
  // We'll assume CREATE is always followed by VIEW for now
  return true; // Placeholder implementation
}

void Parser::initializeSyncTokens() {
  syncTokens_ = {
      Token::KEYWORD_CREATE, Token::KEYWORD_DROP,   Token::KEYWORD_ALTER,
      Token::KEYWORD_SELECT, Token::KEYWORD_INSERT, Token::KEYWORD_UPDATE,
      Token::KEYWORD_DELETE, Token::KEYWORD_USE,    Token::KEYWORD_SHOW,
      Token::KEYWORD_GRANT,  Token::KEYWORD_REVOKE};
}

// Add implementations for all other methods declared in parser.h
// For brevity, these are left as placeholders

std::unique_ptr<CreateStatement> Parser::parseCreateStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateStatement()方法" << std::endl;
  
  // 消费CREATE关键字
  consume(Token::KEYWORD_CREATE);
  
  // 检查要创建的对象类型
  if (match(Token::KEYWORD_TABLE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE TABLE语句" << std::endl;
    return parseCreateTableStatement();
  } else if (match(Token::KEYWORD_DATABASE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE DATABASE语句" << std::endl;
    return parseCreateDatabaseStatement();
  } else if (match(Token::KEYWORD_INDEX)) {
    std::cout << "[PARSER DEBUG] 解析CREATE INDEX语句" << std::endl;
    auto indexStmt = parseCreateIndexStatement();
    // 为简化处理，我们将CreateIndexStatement转换为CreateStatement
    // 实际应用中应该有专门的处理逻辑
    return nullptr;
  } else if (match(Token::KEYWORD_PROCEDURE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE PROCEDURE语句" << std::endl;
    return parseCreateProcedureStatement();
  } else if (match(Token::KEYWORD_VIEW)) {
    std::cout << "[PARSER DEBUG] 解析CREATE VIEW语句" << std::endl;
    // VIEW语句返回Statement类型，需要特殊处理
    // 这里我们直接返回nullptr，稍后在parseStatement中处理
    reportError("CREATE VIEW not supported in this context");
    return nullptr;
  } else if (match(Token::KEYWORD_TRIGGER)) {
    std::cout << "[PARSER DEBUG] 解析CREATE TRIGGER语句" << std::endl;
    return parseCreateTriggerStatement();
  } else {
    // 如果不是已知的对象类型，抛出错误
    std::stringstream ss;
    ss << "Expected TABLE, DATABASE, INDEX, PROCEDURE, or TRIGGER after CREATE, but got "
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
}

std::unique_ptr<CreateStatement> Parser::parseCreateTableStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateTableStatement()方法" << std::endl;

  // 创建一个TABLE类型的CreateStatement
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE);
  if (!stmt) {
    std::cerr << "Failed to create CreateStatement object" << std::endl;
    return nullptr;
  }

  // 解析表名
  std::string tableName = parseIdentifier();
  stmt->setObjectName(tableName);
  std::cout << "[PARSER DEBUG] 表名: " << tableName << std::endl;
  
  // 消费左括号
  consume(Token::LPAREN);
  
  // 解析列定义和表级约束
  bool first = true;
  while (!check(Token::RPAREN) && !isAtEnd()) {
    if (!first) {
      // 如果不是第一个元素，需要消费逗号
      if (!match(Token::COMMA)) {
        break;
      }
    }
    first = false;
    
    // 检查是否是表级约束
    if (check(Token::KEYWORD_PRIMARY) || check(Token::KEYWORD_UNIQUE) || 
        check(Token::KEYWORD_FOREIGN) || check(Token::KEYWORD_CHECK) ||
        check(Token::KEYWORD_CONSTRAINT)) {
      // 解析表级约束
      parseTableConstraint(*stmt);
    } else {
      // 解析列定义
      auto columnDef = parseColumnDefinition();
      if (columnDef) {
        stmt->addColumn(std::move(*columnDef));
      }
    }
  }
  
  // 消费右括号
  consume(Token::RPAREN);
  
  std::cout << "[PARSER DEBUG] CREATE TABLE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<ColumnDefinition> Parser::parseColumnDefinition() {
  std::cout << "[PARSER DEBUG] 进入parseColumnDefinition()方法" << std::endl;
  
  // 解析列名
  std::string columnName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 列名: " << columnName << std::endl;
  
  // 解析数据类型
  std::string dataType = parseDataType();
  std::cout << "[PARSER DEBUG] 数据类型: " << dataType << std::endl;
  
  // 创建列定义
  auto columnDef = std::make_unique<ColumnDefinition>(columnName, dataType);
  
  // 解析列约束
  while (!check(Token::COMMA) && !check(Token::RPAREN) && !isAtEnd()) {
    std::cout << "[PARSER DEBUG] 解析列约束，当前token: " << currentToken_.getLexeme() << std::endl;
    
    if (match(Token::KEYWORD_NOT)) {
      consume(Token::KEYWORD_NULL);
      columnDef->setNullable(false);
      std::cout << "[PARSER DEBUG] 设置NOT NULL约束" << std::endl;
    } else if (match(Token::KEYWORD_NULL)) {
      columnDef->setNullable(true);
      std::cout << "[PARSER DEBUG] 明确设置NULL约束" << std::endl;
    } else if (match(Token::KEYWORD_PRIMARY)) {
      consume(Token::KEYWORD_KEY);
      columnDef->setPrimaryKey(true);
      std::cout << "[PARSER DEBUG] 设置主键约束" << std::endl;
    } else if (match(Token::KEYWORD_UNIQUE)) {
      columnDef->setUnique(true);
      std::cout << "[PARSER DEBUG] 设置唯一约束" << std::endl;
    } else if (match(Token::KEYWORD_DEFAULT)) {
      std::string defaultValue = parseDefaultValue();
      columnDef->setDefaultValue(defaultValue);
      std::cout << "[PARSER DEBUG] 设置默认值: " << defaultValue << std::endl;
    } else if (match(Token::KEYWORD_AUTO_INCREMENT)) {
      columnDef->setAutoIncrement(true);
      std::cout << "[PARSER DEBUG] 设置自增约束" << std::endl;
    } else if (match(Token::KEYWORD_REFERENCES)) {
      // 外键约束在列级别暂时跳过，会在表级约束中处理
      columnDef->setForeignKey(true);
      std::string refTable = parseIdentifier();
      std::cout << "[PARSER DEBUG] 设置外键约束，引用表: " << refTable << std::endl;
      // 跳过引用的列名（简化处理）
      if (match(Token::LPAREN)) {
        parseIdentifier(); // 跳过列名
        consume(Token::RPAREN);
      }
    } else {
      // 未知的约束类型，跳出循环
      break;
    }
  }
  
  std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
  return columnDef;
}

std::string Parser::parseDataType() {
  std::cout << "[PARSER DEBUG] 进入parseDataType()方法" << std::endl;
  
  std::stringstream dataType;
  
  // 解析基本数据类型
  if (check(Token::KEYWORD_INT) || check(Token::KEYWORD_INTEGER) || 
      check(Token::KEYWORD_SMALLINT) || check(Token::KEYWORD_BIGINT) || 
      check(Token::KEYWORD_TINYINT)) {
    dataType << currentToken_.getLexeme();
    advance();
  } else if (check(Token::KEYWORD_VARCHAR) || check(Token::KEYWORD_CHAR)) {
    dataType << currentToken_.getLexeme();
    advance();
    
    // 如果有长度参数
    if (match(Token::LPAREN)) {
      dataType << "(";
      
      // 解析长度值
      if (check(Token::INTEGER_LITERAL)) {
        dataType << currentToken_.getLexeme();
        advance();
      }
      
      dataType << ")";
      consume(Token::RPAREN);
    }
  } else if (check(Token::KEYWORD_DECIMAL) || check(Token::KEYWORD_NUMERIC)) {
    dataType << currentToken_.getLexeme();
    advance();
    
    // 如果有精度参数
    if (match(Token::LPAREN)) {
      dataType << "(";
      
      // 解析精度和小数位数
      if (check(Token::INTEGER_LITERAL)) {
        dataType << currentToken_.getLexeme();
        advance();
        
        if (match(Token::COMMA)) {
          dataType << ",";
          if (check(Token::INTEGER_LITERAL)) {
            dataType << currentToken_.getLexeme();
            advance();
          }
        }
      }
      
      dataType << ")";
      consume(Token::RPAREN);
    }
  } else if (check(Token::KEYWORD_DATE) || check(Token::KEYWORD_TIME) || 
             check(Token::KEYWORD_TIMESTAMP) || check(Token::KEYWORD_DATETIME) ||
             check(Token::KEYWORD_BOOLEAN) || check(Token::KEYWORD_BOOL)) {
    dataType << currentToken_.getLexeme();
    advance();
  } else {
    // 默认情况下，将标识符作为数据类型
    dataType << currentToken_.getLexeme();
    advance();
  }
  
  std::cout << "[PARSER DEBUG] 数据类型解析完成: " << dataType.str() << std::endl;
  return dataType.str();
}

std::string Parser::parseDefaultValue() {
  std::cout << "[PARSER DEBUG] 进入parseDefaultValue()方法" << std::endl;
  
  std::stringstream defaultValue;
  
  if (check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else if (check(Token::STRING_LITERAL)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else if (check(Token::KEYWORD_NULL)) {
    defaultValue << "NULL";
    advance();
  } else if (check(Token::KEYWORD_CURRENT_TIMESTAMP) || check(Token::KEYWORD_NOW)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else {
    // 尝试解析其他字面量
    defaultValue << currentToken_.getLexeme();
    advance();
  }
  
  std::cout << "[PARSER DEBUG] 默认值解析完成: " << defaultValue.str() << std::endl;
  return defaultValue.str();
}

void Parser::parseTableConstraint(CreateStatement& stmt) {
  std::cout << "[PARSER DEBUG] 进入parseTableConstraint()方法" << std::endl;
  
  // 检查是否有约束名称
  std::string constraintName;
  if (match(Token::KEYWORD_CONSTRAINT)) {
    constraintName = parseIdentifier();
  }
  
  // 解析约束类型
  if (match(Token::KEYWORD_PRIMARY)) {
    consume(Token::KEYWORD_KEY);
    auto constraint = TableConstraint(TableConstraint::PRIMARY_KEY, constraintName);
    
    // 解析列列表
    consume(Token::LPAREN);
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Token::RPAREN);
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析主键约束完成" << std::endl;
  } else if (match(Token::KEYWORD_UNIQUE)) {
    auto constraint = TableConstraint(TableConstraint::UNIQUE, constraintName);
    
    // 解析列列表
    consume(Token::LPAREN);
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Token::RPAREN);
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析唯一约束完成" << std::endl;
  } else if (match(Token::KEYWORD_FOREIGN)) {
    consume(Token::KEYWORD_KEY);
    auto constraint = TableConstraint(TableConstraint::FOREIGN_KEY, constraintName);
    
    // 解析列列表
    consume(Token::LPAREN);
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Token::RPAREN);
    
    // 解析REFERENCES子句
    consume(Token::KEYWORD_REFERENCES);
    std::string refTable = parseIdentifier();
    constraint.setReferencedTable(refTable);
    
    // 解析引用的列列表
    if (match(Token::LPAREN)) {
      bool firstRef = true;
      while (!check(Token::RPAREN) && !isAtEnd()) {
        if (!firstRef) {
          if (!match(Token::COMMA)) {
            break;
          }
        }
        firstRef = false;
        
        std::string refColumn = parseIdentifier();
        constraint.addReferencedColumn(refColumn);
      }
      consume(Token::RPAREN);
    }
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析外键约束完成" << std::endl;
  } else if (match(Token::KEYWORD_CHECK)) {
    auto constraint = TableConstraint(TableConstraint::CHECK, constraintName);
    
    // 解析检查表达式（简化处理）
    consume(Token::LPAREN);
    std::stringstream checkExpr;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      checkExpr << currentToken_.getLexeme() << " ";
      advance();
    }
    consume(Token::RPAREN);
    
    constraint.setCheckExpression(checkExpr.str());
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析检查约束完成" << std::endl;
  }
}

std::unique_ptr<CreateStatement> Parser::parseCreateDatabaseStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateDatabaseStatement()方法" << std::endl;
  
  // 创建一个DATABASE类型的CreateStatement
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::DATABASE);
  
  // 解析数据库名
  std::string dbName = parseIdentifier();
  stmt->setObjectName(dbName);
  std::cout << "[PARSER DEBUG] 数据库名: " << dbName << std::endl;
  
  std::cout << "[PARSER DEBUG] CREATE DATABASE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<AlterStatement> Parser::parseAlterStatement() {
  std::cout << "[PARSER DEBUG] 进入parseAlterStatement()方法" << std::endl;
  
  // 消费ALTER关键字
  consume(Token::KEYWORD_ALTER);
  
  // 检查目标对象类型
  AlterStatement::Target target;
  if (match(Token::KEYWORD_TABLE)) {
    target = AlterStatement::TABLE;
    std::cout << "[PARSER DEBUG] 解析ALTER TABLE语句" << std::endl;
  } else if (match(Token::KEYWORD_DATABASE)) {
    target = AlterStatement::DATABASE;
    std::cout << "[PARSER DEBUG] 解析ALTER DATABASE语句" << std::endl;
  } else {
    std::stringstream ss;
    ss << "Expected TABLE or DATABASE after ALTER, but got " 
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
  
  // 创建AlterStatement对象
  auto stmt = std::make_unique<AlterStatement>(target);
  
  // 解析对象名称
  if (target == AlterStatement::TABLE) {
    std::string tableName = parseIdentifier();
    stmt->setTableName(tableName);
    std::cout << "[PARSER DEBUG] 表名: " << tableName << std::endl;
  } else {
    std::string dbName = parseIdentifier();
    stmt->setDatabaseName(dbName);
    std::cout << "[PARSER DEBUG] 数据库名: " << dbName << std::endl;
  }
  
  // 解析具体的操作
  if (match(Token::KEYWORD_ADD)) {
    stmt->setAction(AlterStatement::ADD_COLUMN);
    std::cout << "[PARSER DEBUG] 解析ADD COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Token::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列定义
    auto columnDef = parseColumnDefinition();
    if (columnDef) {
      stmt->setColumnDefinition(std::move(*columnDef));
      std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
    }
  } else if (match(Token::KEYWORD_DROP)) {
    stmt->setAction(AlterStatement::DROP_COLUMN);
    std::cout << "[PARSER DEBUG] 解析DROP COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Token::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列名
    std::string columnName = parseIdentifier();
    stmt->setColumnName(columnName);
    std::cout << "[PARSER DEBUG] 列名: " << columnName << std::endl;
  } else if (match(Token::KEYWORD_MODIFY)) {
    stmt->setAction(AlterStatement::MODIFY_COLUMN);
    std::cout << "[PARSER DEBUG] 解析MODIFY COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Token::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列定义
    auto columnDef = parseColumnDefinition();
    if (columnDef) {
      stmt->setColumnDefinition(std::move(*columnDef));
      std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
    }
  } else if (match(Token::KEYWORD_RENAME)) {
    stmt->setAction(AlterStatement::RENAME_TABLE);
    std::cout << "[PARSER DEBUG] 解析RENAME TO操作" << std::endl;
    
    // 消费TO关键字
    consume(Token::KEYWORD_TO);
    
    // 解析新表名
    std::string newTableName = parseIdentifier();
    stmt->setNewTableName(newTableName);
    std::cout << "[PARSER DEBUG] 新表名: " << newTableName << std::endl;
  } else {
    std::stringstream ss;
    ss << "Unsupported ALTER operation: " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
  
  std::cout << "[PARSER DEBUG] ALTER语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<SelectStatement> Parser::parseSelectStatement() {
  std::cout << "[PARSER DEBUG] 进入parseSelectStatement()方法" << std::endl;
  
  // 消费SELECT关键字
  consume(Token::KEYWORD_SELECT);
  
  // 创建SelectStatement对象
  auto stmt = std::make_unique<SelectStatement>();
  
  // 检查是否有DISTINCT关键字
  if (match(Token::KEYWORD_DISTINCT)) {
    std::cout << "[PARSER DEBUG] 检测到DISTINCT关键字" << std::endl;
    stmt->setDistinct(true);
  }
  
  // 解析选择列表
  if (match(Token::OPERATOR_MULTIPLY)) {
    // SELECT *
    std::cout << "[PARSER DEBUG] 解析SELECT *" << std::endl;
    stmt->setSelectAll(true);
  } else {
    // 解析具体的列名列表或函数调用（支持简单的聚合函数识别，例如 COUNT(*) / SUM(col)）
    std::cout << "[PARSER DEBUG] 解析具体的列名列表或函数调用" << std::endl;
    bool first = true;
    while (!check(Token::KEYWORD_FROM) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      // 如果是函数调用形式: identifier '(' ... ')'
      if (check(Token::IDENTIFIER) && peek().getType() == Token::IDENTIFIER) {
        // lookahead to detect '(' after identifier
      }

      if (check(Token::IDENTIFIER) && (/* lookahead for '(' */ false)) {
        // placeholder - unreachable due to limitations of simple lookahead
      }

      // 解析列名或函数调用
      std::string columnExpr;
      if (check(Token::IDENTIFIER) && lexer_.peek() == '(') {
        // 函数调用
        std::string funcName = currentToken_.getLexeme();
        advance(); // consume function name
        consume(Token::LPAREN);

        // 解析函数参数（简化：只支持单个标识符或 '*'）
        std::string inner;
        if (check(Token::OPERATOR_MULTIPLY)) {
          inner = "*";
          advance();
        } else if (check(Token::IDENTIFIER)) {
          inner = parseIdentifier();
        } else if (check(Token::STRING_LITERAL) || check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
          inner = currentToken_.getLexeme();
          advance();
        } else {
          inner = "";
        }

        consume(Token::RPAREN);
        columnExpr = funcName + "(" + inner + ")";
        std::cout << "[PARSER DEBUG] 解析函数调用: " << columnExpr << std::endl;
      } else {
        // 解析普通列名
        columnExpr = parseIdentifier();
        std::cout << "[PARSER DEBUG] 解析列名: " << columnExpr << std::endl;
      }

      // 检查是否有AS别名
      if (match(Token::KEYWORD_AS)) {
        std::string alias = parseIdentifier();
        columnExpr += " AS " + alias;
        std::cout << "[PARSER DEBUG] 添加别名: " << alias << std::endl;
      }

      stmt->addSelectColumn(columnExpr);
      std::cout << "[PARSER DEBUG] 添加选择列: " << columnExpr << std::endl;
    }
  }
  
  // 解析FROM子句
  if (match(Token::KEYWORD_FROM)) {
    std::cout << "[PARSER DEBUG] 解析FROM子句" << std::endl;
    std::string table = parseIdentifier();
    stmt->setTableName(table);
    stmt->addFromTable(table);
    std::cout << "[PARSER DEBUG] 表名: " << table << std::endl;

    // 解析JOIN子句（支持多个JOIN）
    while (true) {
      if (check(Token::KEYWORD_JOIN) || check(Token::KEYWORD_INNER)) {
        std::cout << "[PARSER DEBUG] 检测到JOIN子句" << std::endl;
        auto joinClause = parseJoinClause();
        if (joinClause) {
          stmt->addJoinClause(std::move(joinClause));
        }
      } else {
        break;
      }
    }
  } else {
    std::stringstream ss;
    ss << "Expected FROM clause in SELECT statement";
    reportError(ss.str());
    return nullptr;
  }
  
  // 解析WHERE子句（可选）
  if (match(Token::KEYWORD_WHERE)) {
    std::cout << "[PARSER DEBUG] 解析WHERE子句" << std::endl;
    // 简化处理，只解析简单的条件 "column = value"
    std::string column = parseIdentifier();
    std::string op = currentToken_.getLexeme();
    advance(); // 消费操作符
    std::string value = currentToken_.getLexeme();
    advance(); // 消费值
    
    WhereClause whereClause(column, op, value);
    stmt->setWhereClause(whereClause);
    std::cout << "[PARSER DEBUG] WHERE条件: " << column << " " << op << " " << value << std::endl;
  }
  
  // 解析GROUP BY子句（可选）
  if (match(Token::KEYWORD_GROUP)) {
    consume(Token::KEYWORD_BY);
    std::cout << "[PARSER DEBUG] 解析GROUP BY子句" << std::endl;

    // 解析GROUP BY列列表
    bool first = true;
    while (!check(Token::KEYWORD_HAVING) && !check(Token::KEYWORD_ORDER) &&
           !check(Token::KEYWORD_LIMIT) && !check(Token::SEMICOLON) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      std::string column = parseIdentifier();
      stmt->addGroupByColumn(column);
      std::cout << "[PARSER DEBUG] GROUP BY列: " << column << std::endl;
    }
  }

  // 解析ORDER BY子句（可选）
  if (match(Token::KEYWORD_ORDER)) {
    consume(Token::KEYWORD_BY);
    std::cout << "[PARSER DEBUG] 解析ORDER BY子句" << std::endl;

    // 解析ORDER BY列
    std::string orderByColumn = parseIdentifier();
    stmt->setOrderByColumn(orderByColumn);

    // 检查是否有排序方向（ASC/DESC）
    if (match(Token::KEYWORD_ASC)) {
      stmt->setOrderDirection("ASC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: ASC" << std::endl;
    } else if (match(Token::KEYWORD_DESC)) {
      stmt->setOrderDirection("DESC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: DESC" << std::endl;
    } else {
      // 默认升序
      stmt->setOrderDirection("ASC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: 默认ASC" << std::endl;
    }

    std::cout << "[PARSER DEBUG] ORDER BY列: " << orderByColumn << std::endl;
  }
  
  // 解析HAVING子句（可选）
  if (match(Token::KEYWORD_HAVING)) {
    std::cout << "[PARSER DEBUG] 解析HAVING子句" << std::endl;
    // 简化实现：解析简单的HAVING条件表达式
    // 这里可以解析聚合函数相关的条件，如 COUNT(*) > 5 等
    // 暂时实现为简单的字符串存储，实际应该解析为表达式树

    std::stringstream having_expr;
    int paren_depth = 0;

    while (!isAtEnd()) {
      if (check(Token::LPAREN)) {
        paren_depth++;
        having_expr << currentToken_.getLexeme();
        advance();
      } else if (check(Token::RPAREN)) {
        paren_depth--;
        having_expr << currentToken_.getLexeme();
        advance();
        if (paren_depth == 0) {
          break; // 括号匹配完成
        }
      } else if (paren_depth == 0 &&
                 (check(Token::KEYWORD_ORDER) || check(Token::KEYWORD_LIMIT) ||
                  check(Token::KEYWORD_UNION) || check(Token::SEMICOLON))) {
        break; // 遇到下一个子句或语句结束
      } else {
        having_expr << currentToken_.getLexeme() << " ";
        advance();
      }
    }

    std::string having_condition = having_expr.str();
    // 移除末尾空格
    while (!having_condition.empty() && having_condition.back() == ' ') {
      having_condition.pop_back();
    }

    std::cout << "[PARSER DEBUG] HAVING条件: " << having_condition << std::endl;

    // 暂时不设置HAVING表达式，因为需要表达式解析器支持
    // stmt->setHavingClause(...);
  }
  
  std::cout << "[PARSER DEBUG] SELECT语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<InsertStatement> Parser::parseInsertStatement() {
  std::cout << "[PARSER DEBUG] 进入parseInsertStatement()方法" << std::endl;
  
  // 消费INSERT关键字
  consume(Token::KEYWORD_INSERT);
  
  // 消费INTO关键字
  consume(Token::KEYWORD_INTO);
  
  // 解析表名
  std::string tableName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 表名: " << tableName << std::endl;
  
  // 创建InsertStatement对象
  auto stmt = std::make_unique<InsertStatement>(tableName);
  
  // 解析列名列表（可选）
  if (match(Token::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析列名列表" << std::endl;
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      stmt->addColumn(column);
      std::cout << "[PARSER DEBUG] 添加列: " << column << std::endl;
    }
    consume(Token::RPAREN);
  }
  
  // 消费VALUES关键字
  consume(Token::KEYWORD_VALUES);
  
  // 解析值列表
  if (match(Token::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析值列表" << std::endl;
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      // 解析值（简化处理，只支持字符串和数字字面量）
      std::string value;
      if (check(Token::STRING_LITERAL) || check(Token::INTEGER_LITERAL) || check(Token::FLOAT_LITERAL)) {
        value = currentToken_.getLexeme();
        advance();
      } else {
        value = parseIdentifier();
      }
      stmt->addValue(value);
      std::cout << "[PARSER DEBUG] 添加值: " << value << std::endl;
    }
    consume(Token::RPAREN);
    stmt->finishRow();
  }
  
  std::cout << "[PARSER DEBUG] INSERT语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<UpdateStatement> Parser::parseUpdateStatement() {
  throw std::runtime_error("parseUpdateStatement not yet implemented");
}

std::unique_ptr<DeleteStatement> Parser::parseDeleteStatement() {
  throw std::runtime_error("parseDeleteStatement not yet implemented");
}

std::unique_ptr<UseStatement> Parser::parseUseStatement() {
  throw std::runtime_error("parseUseStatement not yet implemented");
}

std::unique_ptr<ShowStatement> Parser::parseShowStatement() {
  throw std::runtime_error("parseShowStatement not yet implemented");
}

std::unique_ptr<CreateIndexStatement> Parser::parseCreateIndexStatement() {
  throw std::runtime_error("parseCreateIndexStatement not yet implemented");
}

std::unique_ptr<DropIndexStatement> Parser::parseDropIndexStatement() {
  throw std::runtime_error("parseDropIndexStatement not yet implemented");
}

std::unique_ptr<DropStatement> Parser::parseDropStatement() {
  std::cout << "[PARSER DEBUG] 进入parseDropStatement()方法" << std::endl;
  
  // 消费DROP关键字
  consume(Token::KEYWORD_DROP);
  
  // 检查是否有IF EXISTS子句
  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume(Token::KEYWORD_EXISTS);
    ifExists = true;
    std::cout << "[PARSER DEBUG] 检测到IF EXISTS子句" << std::endl;
  }
  
  // 检查要删除的对象类型
  DropStatement::ObjectType objectType;
  if (match(Token::KEYWORD_TABLE)) {
    objectType = DropStatement::TABLE;
    std::cout << "[PARSER DEBUG] 解析DROP TABLE语句" << std::endl;
  } else if (match(Token::KEYWORD_DATABASE)) {
    objectType = DropStatement::DATABASE;
    std::cout << "[PARSER DEBUG] 解析DROP DATABASE语句" << std::endl;
  } else if (match(Token::KEYWORD_INDEX)) {
    objectType = DropStatement::INDEX;
    std::cout << "[PARSER DEBUG] 解析DROP INDEX语句" << std::endl;
  } else {
    std::stringstream ss;
    ss << "Expected TABLE, DATABASE, or INDEX after DROP, but got " 
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
  
  // 创建DropStatement对象
  auto stmt = std::make_unique<DropStatement>(objectType);
  stmt->setIfExists(ifExists);
  
  // 解析对象名称
  std::string objectName = parseIdentifier();
  stmt->setObjectName(objectName);
  std::cout << "[PARSER DEBUG] 对象名称: " << objectName << std::endl;
  
  std::cout << "[PARSER DEBUG] DROP语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<CreateUserStatement> Parser::parseCreateUserStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateUserStatement()方法" << std::endl;
  
  // 消费CREATE关键字
  consume(Token::KEYWORD_CREATE);
  
  // 消费USER关键字
  consume(Token::KEYWORD_USER);
  
  // 解析用户名
  std::string username = parseIdentifier();
  
  // 解析密码部分
  std::string password;
  bool withPassword = false;
  if (match(Token::KEYWORD_IDENTIFIED)) {
    consume(Token::KEYWORD_BY);
    password = parseIdentifier();
    withPassword = false;  // IDENTIFIED BY格式
  } else if (match(Token::KEYWORD_WITH)) {
    consume(Token::KEYWORD_PASSWORD);
    password = parseIdentifier();
    withPassword = true;   // WITH PASSWORD格式
  }
  
  // 创建CreateUserStatement对象
  auto stmt = std::make_unique<CreateUserStatement>(username, password);
  stmt->setWithPassword(withPassword);
  
  std::cout << "[PARSER DEBUG] CREATE USER语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<DropUserStatement> Parser::parseDropUserStatement() {
  std::cout << "[PARSER DEBUG] 进入parseDropUserStatement()方法" << std::endl;
  
  // 消费DROP关键字
  consume(Token::KEYWORD_DROP);
  
  // 消费USER关键字
  consume(Token::KEYWORD_USER);
  
  // 检查是否有IF EXISTS子句
  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume(Token::KEYWORD_EXISTS);
    ifExists = true;
  }
  
  // 解析用户名
  std::string username = parseIdentifier();
  
  // 创建DropUserStatement对象
  auto stmt = std::make_unique<DropUserStatement>(username);
  stmt->setIfExists(ifExists);
  
  std::cout << "[PARSER DEBUG] DROP USER语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<GrantStatement> Parser::parseGrantStatement() {
  std::cout << "[PARSER DEBUG] 进入parseGrantStatement()方法" << std::endl;
  
  // 消费GRANT关键字
  consume(Token::KEYWORD_GRANT);
  
  // 创建GrantStatement对象
  auto stmt = std::make_unique<GrantStatement>();
  
  // 解析权限列表
  if (match(Token::KEYWORD_ALL)) {
    // 处理ALL [PRIVILEGES]情况
    if (match(Token::KEYWORD_PRIVILEGES)) {
      stmt->addPrivilege("ALL PRIVILEGES");
    } else {
      stmt->addPrivilege("ALL");
    }
  } else {
    // 解析具体权限列表
    std::string privilege = parseIdentifier();
    stmt->addPrivilege(privilege);
    
    while (match(Token::COMMA)) {
      privilege = parseIdentifier();
      stmt->addPrivilege(privilege);
    }
  }
  
  // 可选的PRIVILEGES关键字
  if (check(Token::KEYWORD_PRIVILEGES)) {
    advance();
  }
  
  // 消费ON关键字
  consume(Token::KEYWORD_ON);
  
  // 解析对象类型和名称
  if (match(Token::KEYWORD_TABLE)) {
    stmt->setObjectType("TABLE");
    std::string tableName = parseIdentifier();
    stmt->setObjectName(tableName);
  } else {
    // 默认为TABLE类型
    stmt->setObjectType("TABLE");
    std::string objectName = parseIdentifier();
    stmt->setObjectName(objectName);
  }
  
  // 消费TO关键字
  consume(Token::KEYWORD_TO);
  
  // 解析被授权用户
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);
  
  std::cout << "[PARSER DEBUG] GRANT语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<RevokeStatement> Parser::parseRevokeStatement() {
  std::cout << "[PARSER DEBUG] 进入parseRevokeStatement()方法" << std::endl;
  
  // 消费REVOKE关键字
  consume(Token::KEYWORD_REVOKE);
  
  // 创建RevokeStatement对象
  auto stmt = std::make_unique<RevokeStatement>();
  
  // 解析权限列表
  if (match(Token::KEYWORD_ALL)) {
    // 处理ALL [PRIVILEGES]情况
    if (match(Token::KEYWORD_PRIVILEGES)) {
      stmt->addPrivilege("ALL PRIVILEGES");
    } else {
      stmt->addPrivilege("ALL");
    }
  } else {
    // 解析具体权限列表
    std::string privilege = parseIdentifier();
    stmt->addPrivilege(privilege);
    
    while (match(Token::COMMA)) {
      privilege = parseIdentifier();
      stmt->addPrivilege(privilege);
    }
  }
  
  // 可选的PRIVILEGES关键字
  if (check(Token::KEYWORD_PRIVILEGES)) {
    advance();
  }
  
  // 消费ON关键字
  consume(Token::KEYWORD_ON);
  
  // 解析对象类型和名称
  if (match(Token::KEYWORD_TABLE)) {
    stmt->setObjectType("TABLE");
    std::string tableName = parseIdentifier();
    stmt->setObjectName(tableName);
  } else {
    // 默认为TABLE类型
    stmt->setObjectType("TABLE");
    std::string objectName = parseIdentifier();
    stmt->setObjectName(objectName);
  }
  
  // 消费FROM关键字
  consume(Token::KEYWORD_FROM);
  
  // 解析被撤销权限的用户
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);
  
  std::cout << "[PARSER DEBUG] REVOKE语句解析完成" << std::endl;
  return stmt;
}

std::vector<std::string> Parser::parseColumnNames() {
  throw std::runtime_error("parseColumnNames not yet implemented");
}

std::vector<std::unique_ptr<Expression>> Parser::parseExpressions() {
  throw std::runtime_error("parseExpressions not yet implemented");
}

std::unique_ptr<Expression> Parser::parseExpression() {
  throw std::runtime_error("parseExpression not yet implemented");
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
  throw std::runtime_error("parseLogicalOr not yet implemented");
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
  throw std::runtime_error("parseLogicalAnd not yet implemented");
}

std::unique_ptr<Expression> Parser::parseEquality() {
  throw std::runtime_error("parseEquality not yet implemented");
}

std::unique_ptr<Expression> Parser::parseComparison() {
  throw std::runtime_error("parseComparison not yet implemented");
}

std::unique_ptr<Expression> Parser::parseTerm() {
  throw std::runtime_error("parseTerm not yet implemented");
}

std::unique_ptr<Expression> Parser::parseFactor() {
  throw std::runtime_error("parseFactor not yet implemented");
}

std::unique_ptr<Expression> Parser::parseUnary() {
  throw std::runtime_error("parseUnary not yet implemented");
}

std::unique_ptr<Expression> Parser::parsePrimary() {
  throw std::runtime_error("parsePrimary not yet implemented");
}

std::unique_ptr<Expression> Parser::parseIdentifierExpression() {
  throw std::runtime_error("parseIdentifierExpression not yet implemented");
}

std::vector<std::unique_ptr<ColumnDefinition>> Parser::parseColumnDefinitions() {
  std::cout << "[PARSER DEBUG] 进入parseColumnDefinitions()方法" << std::endl;
  
  std::vector<std::unique_ptr<ColumnDefinition>> columns;
  
  // 解析第一个列定义
  auto firstColumn = parseColumnDefinition();
  if (firstColumn) {
    columns.push_back(std::move(firstColumn));
  }
  
  // 解析后续的列定义（如果有逗号分隔）
  while (match(Token::COMMA)) {
    auto column = parseColumnDefinition();
    if (column) {
      columns.push_back(std::move(column));
    }
  }
  
  std::cout << "[PARSER DEBUG] 列定义解析完成，共" << columns.size() << "个列" << std::endl;
  return columns;
}

std::string Parser::parseQualifiedName() {
  throw std::runtime_error("parseQualifiedName not yet implemented");
}

std::string Parser::parseIdentifier() {
  std::cout << "[PARSER DEBUG] 进入parseIdentifier()方法" << std::endl;
  
  if (check(Token::IDENTIFIER)) {
    std::string identifier = currentToken_.getLexeme();
    advance();
    std::cout << "[PARSER DEBUG] 标识符: " << identifier << std::endl;
    return identifier;
  } else {
    std::stringstream ss;
    ss << "Expected identifier, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return "";
  }
}

std::string Parser::parseStringLiteral() {
  throw std::runtime_error("parseStringLiteral not yet implemented");
}

int Parser::parseIntLiteral() {
  throw std::runtime_error("parseIntLiteral not yet implemented");
}



std::unique_ptr<SetOperation> Parser::parseUnion() {
  throw std::runtime_error("parseUnion not yet implemented");
}

std::unique_ptr<SetOperation> Parser::parseIntersect() {
  throw std::runtime_error("parseIntersect not yet implemented");
}

std::unique_ptr<SetOperation> Parser::parseExcept() {
  throw std::runtime_error("parseExcept not yet implemented");
}

// JOIN clause parsing
std::unique_ptr<JoinClause> Parser::parseJoinClause() {
  std::cout << "[PARSER DEBUG] 进入parseJoinClause()方法" << std::endl;

  // 确定JOIN类型
  JoinClause::JoinType joinType = JoinClause::INNER_JOIN;

  // 检查JOIN类型
  if (match(Token::KEYWORD_INNER)) {
    joinType = JoinClause::INNER_JOIN;
    std::cout << "[PARSER DEBUG] 检测到INNER JOIN" << std::endl;
  } else if (match(Token::KEYWORD_LEFT)) {
    if (match(Token::KEYWORD_OUTER)) {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到LEFT OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到LEFT JOIN" << std::endl;
    }
  } else if (match(Token::KEYWORD_RIGHT)) {
    if (match(Token::KEYWORD_OUTER)) {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到RIGHT OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到RIGHT JOIN" << std::endl;
    }
  } else if (match(Token::KEYWORD_FULL)) {
    if (match(Token::KEYWORD_OUTER)) {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[PARSER DEBUG] 检测到FULL OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[PARSER DEBUG] 检测到FULL JOIN" << std::endl;
    }
  } else if (match(Token::KEYWORD_JOIN)) {
    // 默认为INNER JOIN
    joinType = JoinClause::INNER_JOIN;
    std::cout << "[PARSER DEBUG] 检测到默认JOIN (INNER)" << std::endl;
  }

  // 如果还没有消费JOIN关键字，现在消费
  if (!match(Token::KEYWORD_JOIN)) {
    std::stringstream ss;
    ss << "Expected JOIN keyword, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }

  // 解析表名
  std::string tableName = parseIdentifier();
  std::cout << "[PARSER DEBUG] JOIN表名: " << tableName << std::endl;

  // 解析ON条件
  std::unique_ptr<Expression> condition = nullptr;
  if (match(Token::KEYWORD_ON)) {
    std::cout << "[PARSER DEBUG] 解析JOIN ON条件" << std::endl;

    // 简化实现：解析形如 "table1.column = table2.column" 的条件
    // 实际实现中应该使用完整的表达式解析器

    // 解析左边列名（可能带表前缀）
    std::string leftColumn = parseIdentifier();
    if (leftColumn.empty()) {
      reportError("Expected column name in JOIN condition");
      return nullptr;
    }

    if (match(Token::DOT)) {
      std::string columnPart = parseIdentifier();
      if (!columnPart.empty()) {
        leftColumn += "." + columnPart;
      }
    }

    // 解析操作符（应该等于号）
    if (!match(Token::OPERATOR_EQUAL)) {
      std::stringstream ss;
      ss << "Expected '=' in JOIN condition, but got " << currentToken_.getLexeme();
      reportError(ss.str());
      return nullptr;
    }

    // 解析右边列名（可能带表前缀）
    std::string rightColumn = parseIdentifier();
    if (rightColumn.empty()) {
      reportError("Expected column name in JOIN condition");
      return nullptr;
    }

    if (match(Token::DOT)) {
      std::string columnPart = parseIdentifier();
      if (!columnPart.empty()) {
        rightColumn += "." + columnPart;
      }
    }

    // 创建二元表达式作为JOIN条件
    try {
      auto leftExpr = std::make_unique<IdentifierExpression>(leftColumn);
      auto rightExpr = std::make_unique<IdentifierExpression>(rightColumn);
      condition = std::make_unique<BinaryExpression>(
          std::move(leftExpr), std::move(rightExpr), Token::OPERATOR_EQUAL);
    } catch (const std::exception& e) {
      reportError(std::string("Failed to create JOIN condition: ") + e.what());
      return nullptr;
    }

    std::cout << "[PARSER DEBUG] JOIN条件: " << leftColumn << " = " << rightColumn << std::endl;
  } else if (match(Token::KEYWORD_USING)) {
    // USING子句的简化处理
    std::cout << "[PARSER DEBUG] 检测到USING子句（简化处理）" << std::endl;
    if (!match(Token::LPAREN)) {
      reportError("Expected '(' after USING");
      return nullptr;
    }

    std::string usingColumn = parseIdentifier();
    if (usingColumn.empty()) {
      reportError("Expected column name in USING clause");
      return nullptr;
    }

    if (!match(Token::RPAREN)) {
      reportError("Expected ')' after USING column");
      return nullptr;
    }

    std::cout << "[PARSER DEBUG] USING列: " << usingColumn << std::endl;
    // TODO: 将USING转换为ON条件
  } else {
    std::stringstream ss;
    ss << "Expected ON or USING clause in JOIN, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }

  // 创建并返回JoinClause
  auto joinClause = std::make_unique<JoinClause>(joinType, tableName, std::move(condition));
  std::cout << "[PARSER DEBUG] JOIN子句解析完成" << std::endl;
  return joinClause;
}

// ==================== Procedure and Trigger Parsing ====================

std::unique_ptr<CreateStatement> Parser::parseCreateProcedureStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateProcedureStatement()方法" << std::endl;

  // 解析过程名
  std::string procedureName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 过程名: " << procedureName << std::endl;

  // 解析参数列表（可选）
  std::vector<ProcedureParameter> parameters;
  if (match(Token::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析过程参数列表" << std::endl;
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      // 解析参数模式 (IN, OUT, INOUT)
      ProcedureParameter::Mode mode = ProcedureParameter::IN;
      if (match(Token::KEYWORD_IN)) {
        mode = ProcedureParameter::IN;
      } else if (match(Token::KEYWORD_OUT)) {
        mode = ProcedureParameter::OUT;
      } else if (match(Token::KEYWORD_INOUT)) {
        mode = ProcedureParameter::INOUT;
      }

      // 解析参数名
      std::string paramName = parseIdentifier();
      if (paramName.empty()) {
        reportError("Expected parameter name");
        return nullptr;
      }

      // 解析参数类型
      std::string paramType = parseIdentifier();
      if (paramType.empty()) {
        reportError("Expected parameter type");
        return nullptr;
      }

      parameters.emplace_back(paramName, paramType, mode);
      std::cout << "[PARSER DEBUG] 添加参数: " << ProcedureParameter(paramName, paramType, mode).getModeString()
                << " " << paramName << " " << paramType << std::endl;
    }
    consume(Token::RPAREN);
  }

  // 消费AS关键字
  consume(Token::KEYWORD_AS);

  // 解析过程体
  std::stringstream bodyStream;
  consume(Token::KEYWORD_BEGIN);

  int braceLevel = 1;
  while (!isAtEnd() && braceLevel > 0) {
    if (match(Token::KEYWORD_BEGIN)) {
      braceLevel++;
      bodyStream << "BEGIN ";
    } else if (match(Token::KEYWORD_END)) {
      braceLevel--;
      if (braceLevel > 0) {
        bodyStream << "END ";
      }
    } else {
      bodyStream << currentToken_.getLexeme() << " ";
      advance();
    }
  }

  std::string body = bodyStream.str();
  // 移除末尾空格
  while (!body.empty() && body.back() == ' ') {
    body.pop_back();
  }

  std::cout << "[PARSER DEBUG] 过程体: " << body << std::endl;

  // 创建CreateProcedureStatement对象
  auto stmt = std::make_unique<CreateProcedureStatement>(procedureName);
  for (const auto& param : parameters) {
    stmt->addParameter(param);
  }
  stmt->setBody(body);

  std::cout << "[PARSER DEBUG] CREATE PROCEDURE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<Statement> Parser::parseCreateViewStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateViewStatement()方法" << std::endl;

  // 解析视图名
  std::string viewName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 视图名: " << viewName << std::endl;

  // 解析可选的列名列表
  std::vector<std::string> columnNames;
  if (match(Token::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析视图列名列表" << std::endl;
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      std::string columnName = parseIdentifier();
      if (!columnName.empty()) {
        columnNames.push_back(columnName);
        std::cout << "[PARSER DEBUG] 添加视图列: " << columnName << std::endl;
      }
    }
    consume(Token::RPAREN);
  }

  // 消费AS关键字
  consume(Token::KEYWORD_AS);

  // 解析SELECT语句
  std::cout << "[PARSER DEBUG] 解析视图的SELECT语句" << std::endl;
  auto selectStmt = parseSelectStatement();
  if (!selectStmt) {
    reportError("Expected SELECT statement in CREATE VIEW");
    return nullptr;
  }

  // 创建CreateViewStatement对象
  auto stmt = std::make_unique<CreateViewStatement>(viewName);
  stmt->setSelectStatement(std::move(selectStmt));

  // 设置列名（如果有）
  for (const auto& columnName : columnNames) {
    stmt->addColumnName(columnName);
  }

  std::cout << "[PARSER DEBUG] CREATE VIEW语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<CreateStatement> Parser::parseCreateTriggerStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateTriggerStatement()方法" << std::endl;

  // 解析触发器名
  std::string triggerName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 触发器名: " << triggerName << std::endl;

  // 解析触发时机 (BEFORE/AFTER)
  TriggerDefinition::Timing timing;
  if (match(Token::KEYWORD_BEFORE)) {
    timing = TriggerDefinition::BEFORE;
    std::cout << "[PARSER DEBUG] 触发时机: BEFORE" << std::endl;
  } else if (match(Token::KEYWORD_AFTER)) {
    timing = TriggerDefinition::AFTER;
    std::cout << "[PARSER DEBUG] 触发时机: AFTER" << std::endl;
  } else {
    reportError("Expected BEFORE or AFTER for trigger timing");
    return nullptr;
  }

  // 解析触发事件 (INSERT/UPDATE/DELETE)
  TriggerDefinition::Event event;
  if (match(Token::KEYWORD_INSERT)) {
    event = TriggerDefinition::INSERT;
    std::cout << "[PARSER DEBUG] 触发事件: INSERT" << std::endl;
  } else if (match(Token::KEYWORD_UPDATE)) {
    event = TriggerDefinition::UPDATE;
    std::cout << "[PARSER DEBUG] 触发事件: UPDATE" << std::endl;
  } else if (match(Token::KEYWORD_DELETE)) {
    event = TriggerDefinition::DELETE;
    std::cout << "[PARSER DEBUG] 触发事件: DELETE" << std::endl;
  } else {
    reportError("Expected INSERT, UPDATE, or DELETE for trigger event");
    return nullptr;
  }

  // 消费ON关键字
  consume(Token::KEYWORD_ON);

  // 解析表名
  std::string tableName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 目标表名: " << tableName << std::endl;

  // 解析触发级别 (ROW/STATEMENT) - 可选，默认为ROW
  TriggerDefinition::Level level = TriggerDefinition::ROW;
  if (match(Token::KEYWORD_FOR)) {
    if (match(Token::KEYWORD_EACH)) {
      consume(Token::KEYWORD_ROW);
      level = TriggerDefinition::ROW;
      std::cout << "[PARSER DEBUG] 触发级别: ROW" << std::endl;
    } else {
      // 默认为STATEMENT级别（简化处理）
      level = TriggerDefinition::STATEMENT;
      std::cout << "[PARSER DEBUG] 触发级别: STATEMENT" << std::endl;
    }
  }

  // 解析触发条件 (WHEN子句) - 可选
  std::string condition;
  if (match(Token::KEYWORD_WHEN)) {
    consume(Token::LPAREN);
    std::stringstream conditionStream;
    int parenLevel = 1;
    while (!isAtEnd() && parenLevel > 0) {
      if (match(Token::LPAREN)) {
        parenLevel++;
        conditionStream << "(";
      } else if (match(Token::RPAREN)) {
        parenLevel--;
        if (parenLevel > 0) {
          conditionStream << ")";
        }
      } else {
        conditionStream << currentToken_.getLexeme() << " ";
        advance();
      }
    }
    condition = conditionStream.str();
    // 移除末尾空格
    while (!condition.empty() && condition.back() == ' ') {
      condition.pop_back();
    }
    std::cout << "[PARSER DEBUG] 触发条件: " << condition << std::endl;
  }

  // 消费AS关键字（可选）
  if (match(Token::KEYWORD_AS)) {
    std::cout << "[PARSER DEBUG] 消费AS关键字" << std::endl;
  }

  // 解析触发器体
  std::stringstream bodyStream;
  consume(Token::KEYWORD_BEGIN);

  int braceLevel = 1;
  while (!isAtEnd() && braceLevel > 0) {
    if (match(Token::KEYWORD_BEGIN)) {
      braceLevel++;
      bodyStream << "BEGIN ";
    } else if (match(Token::KEYWORD_END)) {
      braceLevel--;
      if (braceLevel > 0) {
        bodyStream << "END ";
      }
    } else {
      bodyStream << currentToken_.getLexeme() << " ";
      advance();
    }
  }

  std::string body = bodyStream.str();
  // 移除末尾空格
  while (!body.empty() && body.back() == ' ') {
    body.pop_back();
  }

  std::cout << "[PARSER DEBUG] 触发器体: " << body << std::endl;

  // 创建TriggerDefinition对象
  TriggerDefinition triggerDef(triggerName, timing, event, level, tableName);
  triggerDef.setCondition(condition);
  triggerDef.setBody(body);

  // 创建CreateTriggerStatement对象
  auto stmt = std::make_unique<CreateTriggerStatement>(triggerDef);

  std::cout << "[PARSER DEBUG] CREATE TRIGGER语句解析完成" << std::endl;
  return stmt;
}

// ==================== LOAD DATA Statement Parsing ====================

std::unique_ptr<Statement> Parser::parseLoadDataStatement() {
  std::cout << "[PARSER DEBUG] 进入parseLoadDataStatement()方法" << std::endl;

  // 创建LoadDataStatement对象
  auto stmt = std::make_unique<LoadDataStatement>();
  if (!stmt) {
    std::cerr << "Failed to create LoadDataStatement object" << std::endl;
    return nullptr;
  }

  // 解析可选的LOW_PRIORITY或CONCURRENT
  if (match(Token::KEYWORD_LOW_PRIORITY)) {
    stmt->low_priority = true;
    std::cout << "[PARSER DEBUG] 检测到LOW_PRIORITY" << std::endl;
  } else if (match(Token::KEYWORD_CONCURRENT)) {
    stmt->concurrent = true;
    std::cout << "[PARSER DEBUG] 检测到CONCURRENT" << std::endl;
  }

  // 解析可选的LOCAL
  if (match(Token::KEYWORD_LOCAL)) {
    stmt->is_local = true;
    std::cout << "[PARSER DEBUG] 检测到LOCAL" << std::endl;
  }

  // 消费INFILE关键字
  consume(Token::KEYWORD_INFILE);

  // 解析文件路径
  if (check(Token::STRING_LITERAL)) {
    stmt->file_name = currentToken_.getLexeme();
    // 移除引号
    if (stmt->file_name.size() >= 2 && stmt->file_name.front() == '"' && stmt->file_name.back() == '"') {
      stmt->file_name = stmt->file_name.substr(1, stmt->file_name.size() - 2);
    } else if (stmt->file_name.size() >= 2 && stmt->file_name.front() == '\'' && stmt->file_name.back() == '\'') {
      stmt->file_name = stmt->file_name.substr(1, stmt->file_name.size() - 2);
    }
    advance();
    std::cout << "[PARSER DEBUG] 文件名: " << stmt->file_name << std::endl;
  } else {
    reportError("Expected string literal for file name");
    return nullptr;
  }

  // 解析可选的REPLACE或IGNORE
  if (match(Token::KEYWORD_REPLACE)) {
    stmt->replace_or_ignore = "REPLACE";
    std::cout << "[PARSER DEBUG] 检测到REPLACE" << std::endl;
  } else if (match(Token::KEYWORD_IGNORE)) {
    stmt->replace_or_ignore = "IGNORE";
    std::cout << "[PARSER DEBUG] 检测到IGNORE" << std::endl;
  }

  // 消费INTO TABLE关键字
  consume(Token::KEYWORD_INTO);
  consume(Token::KEYWORD_TABLE);

  // 解析表名
  stmt->table_name = parseIdentifier();
  std::cout << "[PARSER DEBUG] 表名: " << stmt->table_name << std::endl;

  // 解析可选的分区子句
  if (match(Token::KEYWORD_PARTITION)) {
    consume(Token::LPAREN);
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      std::string partition = parseIdentifier();
      stmt->partitions.push_back(partition);
      std::cout << "[PARSER DEBUG] 分区: " << partition << std::endl;
    }
    consume(Token::RPAREN);
  }

  // 解析可选的CHARACTER SET子句
  if (match(Token::KEYWORD_CHARACTER)) {
    consume(Token::KEYWORD_SET);
    stmt->charset_name = parseIdentifier();
    std::cout << "[PARSER DEBUG] 字符集: " << stmt->charset_name << std::endl;
  }

  // 解析可选的FIELDS/COLUMNS选项
  if (match(Token::KEYWORD_FIELDS) || match(Token::KEYWORD_COLUMNS)) {
    // 解析字段终止符
    if (match(Token::KEYWORD_TERMINATED)) {
      consume(Token::KEYWORD_BY);
      if (check(Token::STRING_LITERAL)) {
        stmt->fields_terminated_by = currentToken_.getLexeme();
        // 移除引号
        if (stmt->fields_terminated_by.size() >= 2 &&
            stmt->fields_terminated_by.front() == '\'' &&
            stmt->fields_terminated_by.back() == '\'') {
          stmt->fields_terminated_by = stmt->fields_terminated_by.substr(1, stmt->fields_terminated_by.size() - 2);
        }
        advance();
        std::cout << "[PARSER DEBUG] 字段终止符: " << stmt->fields_terminated_by << std::endl;
      }
    }

    // 解析字段包围符
    if (match(Token::KEYWORD_OPTIONALLY)) {
      stmt->fields_optionally_enclosed = true;
    }
    if (match(Token::KEYWORD_ENCLOSED)) {
      consume(Token::KEYWORD_BY);
      if (check(Token::STRING_LITERAL)) {
        stmt->fields_enclosed_by = currentToken_.getLexeme();
        // 移除引号
        if (stmt->fields_enclosed_by.size() >= 2 &&
            stmt->fields_enclosed_by.front() == '\'' &&
            stmt->fields_enclosed_by.back() == '\'') {
          stmt->fields_enclosed_by = stmt->fields_enclosed_by.substr(1, stmt->fields_enclosed_by.size() - 2);
        }
        advance();
        std::cout << "[PARSER DEBUG] 字段包围符: " << stmt->fields_enclosed_by << std::endl;
      }
    }

    // 解析字段转义符
    if (match(Token::KEYWORD_ESCAPED)) {
      consume(Token::KEYWORD_BY);
      if (check(Token::STRING_LITERAL)) {
        stmt->fields_escaped_by = currentToken_.getLexeme();
        // 移除引号
        if (stmt->fields_escaped_by.size() >= 2 &&
            stmt->fields_escaped_by.front() == '\'' &&
            stmt->fields_escaped_by.back() == '\'') {
          stmt->fields_escaped_by = stmt->fields_escaped_by.substr(1, stmt->fields_escaped_by.size() - 2);
        }
        advance();
        std::cout << "[PARSER DEBUG] 字段转义符: " << stmt->fields_escaped_by << std::endl;
      }
    }
  }

  // 解析可选的LINES选项
  if (match(Token::KEYWORD_LINES)) {
    // 解析行起始符
    if (match(Token::KEYWORD_STARTING)) {
      consume(Token::KEYWORD_BY);
      if (check(Token::STRING_LITERAL)) {
        stmt->lines_starting_by = currentToken_.getLexeme();
        // 移除引号
        if (stmt->lines_starting_by.size() >= 2 &&
            stmt->lines_starting_by.front() == '\'' &&
            stmt->lines_starting_by.back() == '\'') {
          stmt->lines_starting_by = stmt->lines_starting_by.substr(1, stmt->lines_starting_by.size() - 2);
        }
        advance();
        std::cout << "[PARSER DEBUG] 行起始符: " << stmt->lines_starting_by << std::endl;
      }
    }

    // 解析行终止符
    if (match(Token::KEYWORD_TERMINATED)) {
      consume(Token::KEYWORD_BY);
      if (check(Token::STRING_LITERAL)) {
        stmt->lines_terminated_by = currentToken_.getLexeme();
        // 移除引号
        if (stmt->lines_terminated_by.size() >= 2 &&
            stmt->lines_terminated_by.front() == '\'' &&
            stmt->lines_terminated_by.back() == '\'') {
          stmt->lines_terminated_by = stmt->lines_terminated_by.substr(1, stmt->lines_terminated_by.size() - 2);
        }
        advance();
        std::cout << "[PARSER DEBUG] 行终止符: " << stmt->lines_terminated_by << std::endl;
      }
    }
  }

  // 解析可选的IGNORE子句
  if (match(Token::KEYWORD_IGNORE)) {
    if (check(Token::INTEGER_LITERAL)) {
      stmt->ignore_lines = std::stoi(currentToken_.getLexeme());
      advance();
    }
    consume(Token::KEYWORD_LINES);
    std::cout << "[PARSER DEBUG] 忽略行数: " << stmt->ignore_lines << std::endl;
  }

  // 解析可选的列列表
  if (match(Token::LPAREN)) {
    bool first = true;
    while (!check(Token::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      std::string column = parseIdentifier();
      stmt->column_list.push_back(column);
      std::cout << "[PARSER DEBUG] 列映射: " << column << std::endl;
    }
    consume(Token::RPAREN);
  }

  // 解析可选的SET子句
  if (match(Token::KEYWORD_SET)) {
    bool first = true;
    while (!isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;

      std::string column = parseIdentifier();
      consume(Token::OPERATOR_EQUAL);

      // 简化处理：解析表达式为字符串
      std::stringstream exprStream;
      int parenDepth = 0;
      while (!isAtEnd()) {
        if (check(Token::LPAREN)) {
          parenDepth++;
          exprStream << currentToken_.getLexeme();
          advance();
        } else if (check(Token::RPAREN)) {
          parenDepth--;
          exprStream << currentToken_.getLexeme();
          advance();
          if (parenDepth == 0) {
            break;
          }
        } else if (parenDepth == 0 &&
                   (check(Token::COMMA) || check(Token::SEMICOLON))) {
          break;
        } else {
          exprStream << currentToken_.getLexeme() << " ";
          advance();
        }
      }

      std::string expression = exprStream.str();
      // 移除末尾空格
      while (!expression.empty() && expression.back() == ' ') {
        expression.pop_back();
      }

      stmt->set_expressions.emplace_back(column, expression);
      std::cout << "[PARSER DEBUG] SET表达式: " << column << " = " << expression << std::endl;
    }
  }

  std::cout << "[PARSER DEBUG] LOAD DATA语句解析完成" << std::endl;
  return stmt;
}

} // namespace sql_parser
} // namespace sqlcc
