/**
 * StatementParser - SQL语句解析器实现
 *
 * 此文件实现了StatementParser类，专门负责SQL语句的解析。
 * 采用分派模式实现，根据语句类型调用相应的专用解析方法。
 */

#include "src/sql_parser/statement_parser.h"
#include "src/sql_parser/token.h"
#include <iostream>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

StatementParser::StatementParser(TokenStream& tokens, ExpressionParser& expr_parser)
    : tokens_(tokens), expr_parser_(expr_parser) {
  std::cout << "[STATEMENT_PARSER] StatementParser initialized" << std::endl;
}

std::unique_ptr<Statement> StatementParser::parseStatement() {
  std::cout << "[STATEMENT_PARSER] parseStatement() called" << std::endl;

  // 根据当前token识别语句类型
  if (tokens_.check(Type::KEYWORD_CREATE)) {
    return parseCreateStatement();
  } else if (tokens_.check(Type::KEYWORD_DROP)) {
    return parseDropStatement();
  } else if (tokens_.check(Type::KEYWORD_ALTER)) {
    return parseAlterStatement();
  } else if (tokens_.check(Type::KEYWORD_INSERT)) {
    return parseInsertStatement();
  } else if (tokens_.check(Type::KEYWORD_UPDATE)) {
    return parseUpdateStatement();
  } else if (tokens_.check(Type::KEYWORD_DELETE)) {
    return parseDeleteStatement();
  } else if (tokens_.check(Type::KEYWORD_GRANT)) {
    return parseGrantStatement();
  } else if (tokens_.check(Type::KEYWORD_REVOKE)) {
    return parseRevokeStatement();
  } else if (tokens_.check(Type::KEYWORD_USE)) {
    return parseUseStatement();
  } else if (tokens_.check(Type::KEYWORD_SHOW)) {
    return parseShowStatement();
  } else if (tokens_.check(Type::KEYWORD_LOAD)) {
    return parseLoadDataStatement();
  } else {
    std::stringstream ss;
    ss << "Unknown statement type starting with: " << tokens_.peek().getLexeme();
    throw std::runtime_error(ss.str());
  }
}

// INSERT语句解析
std::unique_ptr<InsertStatement> StatementParser::parseInsertStatement() {
  std::cout << "[STATEMENT_PARSER] parseInsertStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_INSERT);
  tokens_.expect(Type::KEYWORD_INTO);

  std::string tableName = parseIdentifier();
  auto stmt = std::make_unique<InsertStatement>(tableName);

  // 可选的列列表
  if (tokens_.check(Type::LPAREN)) {
    tokens_.expect(Type::LPAREN);
    bool first = true;
    while (!tokens_.check(Type::RPAREN) && !tokens_.isAtEnd()) {
      if (!first) {
        if (!tokens_.check(Type::COMMA)) {
          break;
        }
        tokens_.expect(Type::COMMA);
      }
      first = false;

      std::string column = parseIdentifier();
      stmt->addColumn(column);
    }
    tokens_.expect(Type::RPAREN);
  }

  // VALUES子句
  tokens_.expect(Type::KEYWORD_VALUES);
  if (tokens_.check(Type::LPAREN)) {
    tokens_.expect(Type::LPAREN);
    bool first = true;
    while (!tokens_.check(Type::RPAREN) && !tokens_.isAtEnd()) {
      if (!first) {
        if (!tokens_.check(Type::COMMA)) {
          break;
        }
        tokens_.expect(Type::COMMA);
      }
      first = false;

  // 简化处理：支持字符串、数字和标识符
  std::string value;
  if (tokens_.check(Type::STRING_LITERAL) || tokens_.check(Type::INTEGER_LITERAL) ||
      tokens_.check(Type::FLOAT_LITERAL)) {
    tokens_.expect(tokens_.peek().getType());
    value = tokens_.current().getLexeme();
  } else {
    value = parseIdentifier();
  }
      stmt->addValue(value);
    }
    tokens_.expect(Type::RPAREN);
    stmt->finishRow();
  }

  std::cout << "[STATEMENT_PARSER] INSERT statement parsed successfully" << std::endl;
  return stmt;
}

// UPDATE语句解析
std::unique_ptr<UpdateStatement> StatementParser::parseUpdateStatement() {
  std::cout << "[STATEMENT_PARSER] parseUpdateStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_UPDATE);

  // 这里应该实现UPDATE语句的完整解析
  // 暂时抛出异常表示未实现
  throw std::runtime_error("UPDATE statement parsing not yet implemented");
}

// DELETE语句解析
std::unique_ptr<DeleteStatement> StatementParser::parseDeleteStatement() {
  std::cout << "[STATEMENT_PARSER] parseDeleteStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_DELETE);
  tokens_.expect(Type::KEYWORD_FROM);

  std::string tableName = parseIdentifier();
  std::vector<std::string> tableNames = {tableName};
  auto stmt = std::make_unique<DeleteStatement>(tableNames);

  // 可选的WHERE子句
  if (tokens_.check(Type::KEYWORD_WHERE)) {
    tokens_.expect(Type::KEYWORD_WHERE);
    // 简化实现：解析简单的条件表达式
    std::cout << "[STATEMENT_PARSER] WHERE clause found in DELETE (simplified)" << std::endl;
  }

  std::cout << "[STATEMENT_PARSER] DELETE statement parsed successfully" << std::endl;
  return stmt;
}

// CREATE语句解析
std::unique_ptr<Statement> StatementParser::parseCreateStatement() {
  std::cout << "[STATEMENT_PARSER] parseCreateStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_CREATE);

  if (tokens_.check(Type::KEYWORD_TABLE)) {
    return parseCreateTableStatement();
  } else if (tokens_.check(Type::KEYWORD_DATABASE)) {
    return parseCreateDatabaseStatement();
  } else if (tokens_.check(Type::KEYWORD_INDEX)) {
    return parseCreateIndexStatement();
  } else if (tokens_.check(Type::KEYWORD_USER)) {
    return parseCreateUserStatement();
  } else if (tokens_.check(Type::KEYWORD_PROCEDURE)) {
    return parseCreateProcedureStatement();
  } else if (tokens_.check(Type::KEYWORD_TRIGGER)) {
    return parseCreateTriggerStatement();
  } else if (tokens_.check(Type::KEYWORD_VIEW)) {
    return parseCreateViewStatement();
  } else {
    std::stringstream ss;
    ss << "Unsupported CREATE statement type: " << tokens_.peek().getLexeme();
    throw std::runtime_error(ss.str());
  }
}

// DROP语句解析
std::unique_ptr<DropStatement> StatementParser::parseDropStatement() {
  std::cout << "[STATEMENT_PARSER] parseDropStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_DROP);

  DropStatement::ObjectType objectType;
  if (tokens_.check(Type::KEYWORD_TABLE)) {
    objectType = DropStatement::TABLE;
  } else if (tokens_.check(Type::KEYWORD_DATABASE)) {
    objectType = DropStatement::DATABASE;
  } else if (tokens_.check(Type::KEYWORD_INDEX)) {
    objectType = DropStatement::INDEX;
  } else if (tokens_.check(Type::KEYWORD_USER)) {
    objectType = DropStatement::USER;
  } else {
    std::stringstream ss;
    ss << "Unsupported DROP statement type: " << tokens_.peek().getLexeme();
    throw std::runtime_error(ss.str());
  }

  bool ifExists = false;
  if (tokens_.check(Type::KEYWORD_IF)) {
    tokens_.expect(Type::KEYWORD_EXISTS);
    ifExists = true;
  }

  std::string objectName = parseIdentifier();
  auto stmt = std::make_unique<DropStatement>(objectType);
  stmt->setIfExists(ifExists);
  stmt->setObjectName(objectName);

  std::cout << "[STATEMENT_PARSER] DROP statement parsed successfully" << std::endl;
  return stmt;
}

// ALTER语句解析
std::unique_ptr<AlterStatement> StatementParser::parseAlterStatement() {
  std::cout << "[STATEMENT_PARSER] parseAlterStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_ALTER);

  AlterStatement::ObjectType target;
  if (tokens_.check(Type::KEYWORD_TABLE)) {
    target = AlterStatement::TABLE;
  } else if (tokens_.check(Type::KEYWORD_DATABASE)) {
    target = AlterStatement::DATABASE;
  } else {
    std::stringstream ss;
    ss << "Unsupported ALTER target: " << tokens_.peek().getLexeme();
    throw std::runtime_error(ss.str());
  }

  auto stmt = std::make_unique<AlterStatement>(target);
  std::string objectName = parseIdentifier();
  stmt->setTableName(objectName);

  // 解析操作类型
  if (tokens_.check(Type::KEYWORD_ADD)) {
    if (tokens_.check(Type::KEYWORD_COLUMN)) {
      stmt->setAlterType("ADD_COLUMN");
      auto columnDef = parseColumnDefinition();
      if (columnDef) {
        stmt->setColumnDefinition(std::move(columnDef));
      }
    }
  } else if (tokens_.check(Type::KEYWORD_DROP)) {
    if (tokens_.check(Type::KEYWORD_COLUMN)) {
      stmt->setAlterType("DROP_COLUMN");
      std::string columnName = parseIdentifier();
      // 需要在AlterStatement类中添加setColumnName方法
      // stmt->setColumnName(columnName);
    }
  } else if (tokens_.check(Type::KEYWORD_MODIFY)) {
    if (tokens_.check(Type::KEYWORD_COLUMN)) {
      stmt->setAlterType("MODIFY_COLUMN");
      auto columnDef = parseColumnDefinition();
      if (columnDef) {
        stmt->setColumnDefinition(std::move(columnDef));
      }
    }
  } else if (tokens_.check(Type::KEYWORD_RENAME)) {
    tokens_.expect(Type::KEYWORD_TO);
    stmt->setAlterType("RENAME_TABLE");
    std::string newName = parseIdentifier();
    // 需要在AlterStatement类中添加setNewTableName方法
    // stmt->setNewTableName(newName);
  }

  std::cout << "[STATEMENT_PARSER] ALTER statement parsed successfully" << std::endl;
  return stmt;
}

// GRANT语句解析
std::unique_ptr<GrantStatement> StatementParser::parseGrantStatement() {
  std::cout << "[STATEMENT_PARSER] parseGrantStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_GRANT);

  auto stmt = std::make_unique<GrantStatement>();

  // 解析权限列表
  bool first = true;
  while (!tokens_.check(Type::KEYWORD_ON) && !tokens_.isAtEnd()) {
    if (!first) {
      if (!tokens_.check(Type::COMMA)) {
        break;
      }
    }
    first = false;

    std::string privilege = parseIdentifier();
    stmt->addPrivilege(privilege);
  }

  tokens_.expect(Type::KEYWORD_ON);
  tokens_.expect(Type::KEYWORD_TABLE);

  std::string tableName = parseIdentifier();
  stmt->setObjectName(tableName);
  stmt->setObjectType("TABLE");

  tokens_.expect(Type::KEYWORD_TO);
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);

  std::cout << "[STATEMENT_PARSER] GRANT statement parsed successfully" << std::endl;
  return stmt;
}

// REVOKE语句解析
std::unique_ptr<RevokeStatement> StatementParser::parseRevokeStatement() {
  std::cout << "[STATEMENT_PARSER] parseRevokeStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_REVOKE);

  auto stmt = std::make_unique<RevokeStatement>();

  // 解析权限列表
  bool first = true;
  while (!tokens_.check(Type::KEYWORD_ON) && !tokens_.isAtEnd()) {
    if (!first) {
      if (!tokens_.check(Type::COMMA)) {
        break;
      }
    }
    first = false;

    std::string privilege = parseIdentifier();
    stmt->addPrivilege(privilege);
  }

  tokens_.expect(Type::KEYWORD_ON);
  tokens_.expect(Type::KEYWORD_TABLE);

  std::string tableName = parseIdentifier();
  stmt->setObjectName(tableName);
  stmt->setObjectType("TABLE");

  tokens_.expect(Type::KEYWORD_FROM);
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);

  std::cout << "[STATEMENT_PARSER] REVOKE statement parsed successfully" << std::endl;
  return stmt;
}

// USE语句解析
std::unique_ptr<UseStatement> StatementParser::parseUseStatement() {
  std::cout << "[STATEMENT_PARSER] parseUseStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_USE);
  std::string databaseName = parseIdentifier();

  auto stmt = std::make_unique<UseStatement>(databaseName);

  std::cout << "[STATEMENT_PARSER] USE statement parsed successfully" << std::endl;
  return stmt;
}

// SHOW语句解析
std::unique_ptr<ShowStatement> StatementParser::parseShowStatement() {
  std::cout << "[STATEMENT_PARSER] parseShowStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_SHOW);

  // 简化实现
  auto stmt = std::make_unique<ShowStatement>(ShowStatement::DATABASES);

  std::cout << "[STATEMENT_PARSER] SHOW statement parsed successfully" << std::endl;
  return stmt;
}

// LOAD DATA语句解析
std::unique_ptr<Statement> StatementParser::parseLoadDataStatement() {
  std::cout << "[STATEMENT_PARSER] parseLoadDataStatement() called" << std::endl;

  tokens_.expect(Type::KEYWORD_LOAD);
  tokens_.expect(Type::KEYWORD_DATA);

  // LOAD DATA语句暂不支持
  std::cout << "[STATEMENT_PARSER] LOAD DATA statement not supported" << std::endl;
  throw std::runtime_error("LOAD DATA statement not yet supported");
}

// CREATE TABLE语句解析
std::unique_ptr<CreateStatement> StatementParser::parseCreateTableStatement() {
  std::cout << "[STATEMENT_PARSER] parseCreateTableStatement() called" << std::endl;

  std::string tableName = parseIdentifier();
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE);
  stmt->setObjectName(tableName);

  tokens_.expect(Type::LPAREN);

  bool first = true;
  while (!tokens_.check(Type::RPAREN) && !tokens_.isAtEnd()) {
    if (!first) {
      if (!tokens_.check(Type::COMMA)) {
        break;
      }
    }
    first = false;

    // 检查是否是表级约束
    if (tokens_.check(Type::KEYWORD_PRIMARY) || tokens_.check(Type::KEYWORD_UNIQUE) ||
        tokens_.check(Type::KEYWORD_FOREIGN) || tokens_.check(Type::KEYWORD_CHECK)) {
      parseTableConstraint(*stmt);
    } else {
      // 解析列定义
      auto columnDef = parseColumnDefinition();
      if (columnDef) {
        stmt->addColumn(std::move(columnDef));
      }
    }
  }

  tokens_.expect(Type::RPAREN);

  std::cout << "[STATEMENT_PARSER] CREATE TABLE statement parsed successfully" << std::endl;
  return stmt;
}

// 其他CREATE语句的简化实现
std::unique_ptr<CreateStatement> StatementParser::parseCreateDatabaseStatement() {
  std::string dbName = parseIdentifier();
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::DATABASE);
  stmt->setObjectName(dbName);
  return stmt;
}

std::unique_ptr<CreateUserStatement> StatementParser::parseCreateUserStatement() {
  std::string username = parseIdentifier();
  std::string password;

  if (tokens_.check(Type::KEYWORD_IDENTIFIED)) {
    tokens_.expect(Type::KEYWORD_BY);
    password = parseIdentifier();
  }

  auto stmt = std::make_unique<CreateUserStatement>(username, password);
  return stmt;
}

std::unique_ptr<DropUserStatement> StatementParser::parseDropUserStatement() {
  bool ifExists = false;
  if (tokens_.check(Type::KEYWORD_IF)) {
    tokens_.expect(Type::KEYWORD_EXISTS);
    ifExists = true;
  }

  std::string username = parseIdentifier();
  auto stmt = std::make_unique<DropUserStatement>(username);
  stmt->setIfExists(ifExists);
  return stmt;
}

std::unique_ptr<CreateStatement> StatementParser::parseCreateProcedureStatement() {
  std::string procName = parseIdentifier();
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::PROCEDURE);
  stmt->setObjectName(procName);
  return stmt;
}

std::unique_ptr<CreateStatement> StatementParser::parseCreateTriggerStatement() {
  std::string triggerName = parseIdentifier();
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::TRIGGER);
  stmt->setObjectName(triggerName);
  return stmt;
}

std::unique_ptr<Statement> StatementParser::parseCreateViewStatement() {
  std::string viewName = parseIdentifier();
  auto stmt = std::make_unique<CreateViewStatement>(viewName);

  if (tokens_.check(Type::KEYWORD_AS)) {
    // 这里应该解析SELECT语句，但暂时简化
    std::cout << "[STATEMENT_PARSER] CREATE VIEW AS clause (simplified)" << std::endl;
  }

  return stmt;
}

std::unique_ptr<CreateIndexStatement> StatementParser::parseCreateIndexStatement() {
  std::string indexName = parseIdentifier();
  tokens_.expect(Type::KEYWORD_ON);
  std::string tableName = parseIdentifier();

  // 简化实现：假设只有一个列
  std::string columnName = "id"; // 默认列名
  tokens_.expect(Type::LPAREN);
  if (!tokens_.check(Type::RPAREN)) {
    columnName = parseIdentifier();
  }
  tokens_.expect(Type::RPAREN);

  auto stmt = std::make_unique<CreateIndexStatement>(indexName, tableName, columnName);
  stmt->addColumn(columnName);

  return stmt;
}

std::unique_ptr<DropIndexStatement> StatementParser::parseDropIndexStatement() {
  std::string indexName = parseIdentifier();
  auto stmt = std::make_unique<DropIndexStatement>(indexName);
  return stmt;
}

// 辅助方法实现
std::string StatementParser::parseIdentifier() {
  tokens_.expect(Type::IDENTIFIER);
  return tokens_.previous().getLexeme();
}

std::unique_ptr<ColumnDefinition> StatementParser::parseColumnDefinition() {
  std::string columnName = parseIdentifier();
  std::string dataType = parseDataType();

  auto columnDef = std::make_unique<ColumnDefinition>(columnName, dataType);

  // 解析约束
  while (!tokens_.check(Type::COMMA) && !tokens_.check(Type::RPAREN) && !tokens_.isAtEnd()) {
    if (tokens_.check(Type::KEYWORD_NOT)) {
      tokens_.expect(Type::KEYWORD_NULL);
      columnDef->setNullable(false);
    } else if (tokens_.check(Type::KEYWORD_NULL)) {
      columnDef->setNullable(true);
    } else if (tokens_.check(Type::KEYWORD_PRIMARY)) {
      tokens_.expect(Type::KEYWORD_KEY);
      columnDef->setPrimaryKey(true);
    } else if (tokens_.check(Type::KEYWORD_UNIQUE)) {
      columnDef->setUnique(true);
    } else if (tokens_.check(Type::KEYWORD_DEFAULT)) {
      std::string defaultValue = parseDefaultValue();
      columnDef->setDefaultValue(defaultValue);
    } else if (tokens_.check(Type::KEYWORD_AUTO_INCREMENT)) {
      columnDef->setAutoIncrement(true);
    } else {
      break;
    }
  }

  return columnDef;
}

std::string StatementParser::parseDataType() {
  std::cout << "[STATEMENT_PARSER] parseDataType() called" << std::endl;

  std::stringstream dataType;

  if (tokens_.check(Type::KEYWORD_INT) || tokens_.check(Type::KEYWORD_INTEGER)) {
    tokens_.expect(tokens_.peek().getType());
    dataType << tokens_.previous().getLexeme();
  } else if (tokens_.check(Type::KEYWORD_VARCHAR)) {
    tokens_.expect(Type::KEYWORD_VARCHAR);
    dataType << tokens_.previous().getLexeme();
    if (tokens_.check(Type::LPAREN)) {
      dataType << "(";
      if (tokens_.check(Type::INTEGER_LITERAL)) {
        tokens_.expect(Type::INTEGER_LITERAL);
        dataType << tokens_.previous().getLexeme();
      }
      tokens_.expect(Type::RPAREN);
      dataType << ")";
    }
  } else if (tokens_.check(Type::KEYWORD_DECIMAL) || tokens_.check(Type::KEYWORD_NUMERIC)) {
    tokens_.expect(tokens_.peek().getType());
    dataType << tokens_.previous().getLexeme();
    if (tokens_.check(Type::LPAREN)) {
      dataType << "(";
      if (tokens_.check(Type::INTEGER_LITERAL)) {
        tokens_.expect(Type::INTEGER_LITERAL);
        dataType << tokens_.previous().getLexeme();
        if (tokens_.check(Type::COMMA)) {
          dataType << ",";
          if (tokens_.check(Type::INTEGER_LITERAL)) {
            tokens_.expect(Type::INTEGER_LITERAL);
            dataType << tokens_.previous().getLexeme();
          }
        }
      }
      tokens_.expect(Type::RPAREN);
      dataType << ")";
    }
  } else {
    // 默认当作标识符处理
    tokens_.expect(Type::IDENTIFIER);
    dataType << tokens_.previous().getLexeme();
  }

  std::cout << "[STATEMENT_PARSER] Data type parsed: " << dataType.str() << std::endl;
  return dataType.str();
}

std::string StatementParser::parseDefaultValue() {
  std::cout << "[STATEMENT_PARSER] parseDefaultValue() called" << std::endl;

  std::string value;
  if (tokens_.check(Type::STRING_LITERAL) || tokens_.check(Type::INTEGER_LITERAL) ||
      tokens_.check(Type::FLOAT_LITERAL)) {
    tokens_.expect(tokens_.peek().getType());
    value = tokens_.previous().getLexeme();
  } else if (tokens_.check(Type::KEYWORD_NULL)) {
    tokens_.expect(Type::KEYWORD_NULL);
    value = "NULL";
  } else {
    tokens_.expect(Type::IDENTIFIER);
    value = tokens_.previous().getLexeme();
  }

  std::cout << "[STATEMENT_PARSER] Default value parsed: " << value << std::endl;
  return value;
}

void StatementParser::parseTableConstraint(CreateStatement& stmt) {
  std::cout << "[STATEMENT_PARSER] parseTableConstraint() called" << std::endl;

  // 简化实现：跳过表级约束的解析
  while (!tokens_.check(Type::COMMA) && !tokens_.check(Type::RPAREN) && !tokens_.isAtEnd()) {
    tokens_.advance();
  }

  std::cout << "[STATEMENT_PARSER] Table constraint parsed (simplified)" << std::endl;
}

} // namespace sql_parser
} // namespace sqlcc