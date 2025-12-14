#include "sql_parser/parser.h"
#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/set_operation.h"
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
    std::cout << "[PARSER DEBUG] 检测到CREATE关键字，调用parseCreateStatement()"
              << std::endl;
    return parseCreateStatement();
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
      return;

    default:
      advance();
    }
  }
}

bool Parser::hadError() const { return !errors_.empty(); }

void Parser::initializeSyncTokens() {
  syncTokens_ = {
      Token::KEYWORD_CREATE, Token::KEYWORD_DROP,   Token::KEYWORD_ALTER,
      Token::KEYWORD_SELECT, Token::KEYWORD_INSERT, Token::KEYWORD_UPDATE,
      Token::KEYWORD_DELETE, Token::KEYWORD_USE,    Token::KEYWORD_SHOW};
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
  } else {
    // 如果不是已知的对象类型，抛出错误
    std::stringstream ss;
    ss << "Expected TABLE, DATABASE, or INDEX after CREATE, but got " 
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
}

std::unique_ptr<CreateStatement> Parser::parseCreateTableStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateTableStatement()方法" << std::endl;
  
  // 创建一个TABLE类型的CreateStatement
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE);
  
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
    // 当前SelectStatement不支持DISTINCT，暂不处理
  }
  
  // 解析选择列表
  if (match(Token::OPERATOR_MULTIPLY)) {
    // SELECT *
    std::cout << "[PARSER DEBUG] 解析SELECT *" << std::endl;
    stmt->setSelectAll(true);
  } else {
    // 解析具体的列名列表
    std::cout << "[PARSER DEBUG] 解析具体的列名列表" << std::endl;
    bool first = true;
    while (!check(Token::KEYWORD_FROM) && !isAtEnd()) {
      if (!first) {
        if (!match(Token::COMMA)) {
          break;
        }
      }
      first = false;
      
      // 解析列名（简化处理，实际应该支持表达式）
      std::string column = parseIdentifier();
      stmt->addSelectColumn(column);
      std::cout << "[PARSER DEBUG] 添加列: " << column << std::endl;
    }
  }
  
  // 解析FROM子句
  if (match(Token::KEYWORD_FROM)) {
    std::cout << "[PARSER DEBUG] 解析FROM子句" << std::endl;
    std::string table = parseIdentifier();
    stmt->setTableName(table);
    std::cout << "[PARSER DEBUG] 表名: " << table << std::endl;
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
  throw std::runtime_error("parseCreateUserStatement not yet implemented");
}

std::unique_ptr<DropUserStatement> Parser::parseDropUserStatement() {
  throw std::runtime_error("parseDropUserStatement not yet implemented");
}

std::unique_ptr<GrantStatement> Parser::parseGrantStatement() {
  throw std::runtime_error("parseGrantStatement not yet implemented");
}

std::unique_ptr<RevokeStatement> Parser::parseRevokeStatement() {
  throw std::runtime_error("parseRevokeStatement not yet implemented");
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

} // namespace sql_parser
} // namespace sqlcc