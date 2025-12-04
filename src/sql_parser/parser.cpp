#include "sql_parser/parser.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

Parser::Parser(const std::string &input) : lexer_(input) { nextToken(); }

void Parser::nextToken() { currentToken_ = lexer_.nextToken(); }

bool Parser::match(Token::Type type) { return currentToken_.getType() == type; }

void Parser::consume(Token::Type type) {
  if (currentToken_.getType() == type) {
    nextToken();
  } else {
    std::string expected = Token::getTypeName(type);
    std::string actual = Token::getTypeName(currentToken_.getType());
    throw std::runtime_error("Expected token '" + expected + "', but found '" +
                             actual + "'");
  }
}

void Parser::consume() { nextToken(); }

void Parser::expect(Token::Type type, const std::string &errorMessage) {
  if (!match(type)) {
    throw std::runtime_error(errorMessage);
  }
  consume();
}

void Parser::reportError(const std::string &message) {
  throw std::runtime_error(
      "Parse error at line " + std::to_string(currentToken_.getLine()) +
      ", column " + std::to_string(currentToken_.getColumn()) + ": " + message);
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatements() {
  std::vector<std::unique_ptr<Statement>> statements;

  while (!match(Token::END_OF_INPUT)) {
    if (match(Token::SEMICOLON)) {
      consume(); // 跳过分号
      continue;
    }

    std::unique_ptr<Statement> stmt = parseStatement();
    if (stmt) {
      statements.push_back(std::move(stmt));
    }

    // 跳过分号（如果存在）
    if (match(Token::SEMICOLON)) {
      consume();
    }
  }

  return statements;
}

std::unique_ptr<Statement> Parser::parseStatement() {
  if (match(Token::KEYWORD_CREATE)) {
    return parseCreateStatement();
  } else if (match(Token::KEYWORD_SELECT)) {
    return parseSelectStatement();
  } else if (match(Token::KEYWORD_INSERT)) {
    return parseInsertStatement();
  } else if (match(Token::KEYWORD_UPDATE)) {
    return parseUpdateStatement();
  } else if (match(Token::KEYWORD_DELETE)) {
    return parseDeleteStatement();
  } else if (match(Token::KEYWORD_DROP)) {
    return parseDropStatement();
  } else if (match(Token::KEYWORD_ALTER)) {
    return parseAlterStatement();
  } else if (match(Token::KEYWORD_USE)) {
    return parseUseStatement();
  } else if (match(Token::KEYWORD_GRANT)) {
    return parseGrantStatement();
  } else if (match(Token::KEYWORD_REVOKE)) {
    return parseRevokeStatement();
  } else if (match(Token::KEYWORD_SHOW)) {
    return parseShowStatement();
  } else {
    reportError("Unexpected token: " + currentToken_.getLexeme());
    return nullptr;
  }
}

std::unique_ptr<Statement> Parser::parseCreateStatement() {
  consume(Token::KEYWORD_CREATE);

  // 检查UNIQUE关键字（CREATE UNIQUE INDEX）
  bool isUnique = false;
  if (match(Token::KEYWORD_UNIQUE)) {
    isUnique = true;
    consume();

    // UNIQUE后面必须是INDEX
    if (!match(Token::KEYWORD_INDEX)) {
      reportError("Expected INDEX after UNIQUE");
      return nullptr;
    }

    std::unique_ptr<CreateIndexStatement> indexStmt =
        parseCreateIndexStatement();
    if (indexStmt) {
      indexStmt->setUnique(true);
    }
    return std::unique_ptr<Statement>(indexStmt.release());
  }

  if (match(Token::KEYWORD_DATABASE)) {
    auto stmt = parseCreateDatabaseStatement();
    return std::unique_ptr<Statement>(stmt.release());
  } else if (match(Token::KEYWORD_TABLE)) {
    auto stmt = parseCreateTableStatement();
    return std::unique_ptr<Statement>(stmt.release());
  } else if (match(Token::KEYWORD_USER)) {
    // 处理CREATE USER语句
    consume(); // 消耗USER关键字
    return parseCreateUserStatement();
  } else if (match(Token::KEYWORD_INDEX)) {
    // 注意：这里需要进行类型转换
    std::unique_ptr<CreateIndexStatement> indexStmt =
        parseCreateIndexStatement();
    return std::unique_ptr<Statement>(indexStmt.release());
  } else {
    reportError("Expected DATABASE, TABLE, USER, or INDEX after CREATE");
    return nullptr;
  }
}

std::unique_ptr<CreateStatement> Parser::parseCreateDatabaseStatement() {
  consume(Token::KEYWORD_DATABASE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected database name");
    return nullptr;
  }

  std::string dbName = currentToken_.getLexeme();
  consume();

  auto stmt =
      std::make_unique<CreateStatement>(CreateStatement::DATABASE, dbName);

  return stmt;
}

std::unique_ptr<CreateStatement> Parser::parseCreateTableStatement() {
  consume(Token::KEYWORD_TABLE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  auto stmt =
      std::make_unique<CreateStatement>(CreateStatement::TABLE, tableName);

  consume(Token::LPAREN);
  parseColumnDefinitions(*stmt);
  consume(Token::RPAREN);

  return stmt;
}

std::unique_ptr<CreateIndexStatement> Parser::parseCreateIndexStatement() {
  consume(Token::KEYWORD_INDEX);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected index name");
    return nullptr;
  }

  std::string indexName = currentToken_.getLexeme();
  consume();

  expect(Token::KEYWORD_ON, "Expected ON keyword");

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  consume(Token::LPAREN);

  // 解析第一个列名
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected column name");
    return nullptr;
  }

  std::string columnName = currentToken_.getLexeme();
  consume();

  auto stmt =
      std::make_unique<CreateIndexStatement>(indexName, tableName, columnName);

  // 解析额外的列名（多列索引）
  while (match(Token::COMMA)) {
    consume(); // 消耗逗号

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name after comma");
      return nullptr;
    }

    stmt->addColumn(currentToken_.getLexeme());
    consume();
  }

  consume(Token::RPAREN);

  return stmt;
}

void Parser::parseColumnDefinitions(CreateStatement &stmt) {
  do {
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name");
      return;
    }

    std::string columnName = currentToken_.getLexeme();
    consume();

    std::string dataType = parseDataType();

    ColumnDefinition column(columnName, dataType);

    // 解析列约束
    while (isColumnConstraint()) {
      parseColumnConstraint(column);
    }

    stmt.addColumn(std::move(column));

    // 如果下一个token不是逗号，则结束列定义
    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (!match(Token::RPAREN));
}

std::string Parser::parseDataType() {
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected data type");
    return "";
  }

  std::string type = currentToken_.getLexeme();
  consume();

  // 检查是否有大小参数，如VARCHAR(255)
  if (match(Token::LPAREN)) {
    consume();

    if (!match(Token::INTEGER_LITERAL)) {
      reportError("Expected number in data type size");
      return type;
    }

    std::string size = currentToken_.getLexeme();
    consume();

    consume(Token::RPAREN);

    type += "(" + size + ")";
  }

  return type;
}

bool Parser::isColumnConstraint() {
  return match(Token::KEYWORD_PRIMARY) || match(Token::KEYWORD_NOT) ||
         match(Token::KEYWORD_UNIQUE) || match(Token::KEYWORD_CHECK) ||
         match(Token::KEYWORD_DEFAULT) ||
         match(Token::KEYWORD_AUTO_INCREMENT) ||
         match(Token::KEYWORD_REFERENCES);
}

void Parser::parseColumnConstraint(ColumnDefinition &columnDef) {
  std::string lexeme = currentToken_.getLexeme();

  if (match(Token::KEYWORD_PRIMARY)) {
    consume();
    expect(Token::KEYWORD_KEY, "Expected KEY after PRIMARY");
    columnDef.setIsPrimaryKey(true);
    columnDef.setIsNullable(false);
  } else if (match(Token::KEYWORD_NOT)) {
    consume();
    expect(Token::KEYWORD_NULL, "Expected NULL after NOT");
    columnDef.setIsNullable(false);
  } else if (match(Token::KEYWORD_NULL)) {
    consume();
    columnDef.setIsNullable(true);
  } else if (match(Token::KEYWORD_UNIQUE)) {
    consume();
    columnDef.setIsUnique(true);
  } else if (match(Token::KEYWORD_DEFAULT)) {
    consume();
    // 简化处理，只处理字符串和数字默认值
    if (match(Token::STRING_LITERAL) || match(Token::INTEGER_LITERAL) ||
        match(Token::FLOAT_LITERAL)) {
      std::string defaultValue = currentToken_.getLexeme();
      consume();
      columnDef.setDefaultValue(defaultValue);
    } else {
      reportError("Expected default value");
    }
  } else if (match(Token::KEYWORD_AUTO_INCREMENT)) {
    consume();
    columnDef.setIsAutoIncrement(true);
  } else {
    reportError("Unknown column constraint: " + lexeme);
  }
}

std::unique_ptr<SelectStatement> Parser::parseSelectStatement() {
  consume(Token::KEYWORD_SELECT);

  auto stmt = std::make_unique<SelectStatement>();

  parseSelectList(*stmt);

  if (match(Token::KEYWORD_FROM)) {
    parseFromClause(*stmt);
  }

  if (match(Token::KEYWORD_WHERE)) {
    parseWhereClause(*stmt);
  }

  if (match(Token::KEYWORD_GROUP)) {
    parseGroupByClause(*stmt);
  }

  if (match(Token::KEYWORD_ORDER)) {
    parseOrderByClause(*stmt);
  }

  // 处理LIMIT和OFFSET子句
  if (match(Token::KEYWORD_LIMIT) || currentToken_.getLexeme() == "LIMIT") {
    parseLimitOffsetClause(*stmt);
  }

  return stmt;
}

void Parser::parseSelectList(SelectStatement &stmt) {
  if (match(Token::OPERATOR_MULTIPLY)) {
    stmt.setSelectAll(true);
    consume();
    return;
  }

  // 支持逗号分隔的列名列表
  do {
    std::string columnName;

    // 处理 table.column 格式或函数调用
    if (match(Token::IDENTIFIER)) {
      columnName = currentToken_.getLexeme();
      consume();

      // 检查是否是函数调用（如COUNT(*)）
      if (match(Token::LPAREN)) {
        consume(); // 消费左括号

        // 简化处理，将整个函数调用作为一个列名
        columnName += "(";

        // 处理函数参数，可能是*或列名或表达式
        if (match(Token::OPERATOR_MULTIPLY)) {
          columnName += "*";
          consume();
        } else if (match(Token::IDENTIFIER)) {
          columnName += currentToken_.getLexeme();
          consume();

          // 可能是DISTINCT关键字
          if (match(Token::KEYWORD_DISTINCT)) {
            columnName += " " + currentToken_.getLexeme();
            consume();

            if (match(Token::IDENTIFIER)) {
              columnName += " " + currentToken_.getLexeme();
              consume();
            }
          }
        } else if (match(Token::KEYWORD_DISTINCT)) {
          // 处理COUNT(DISTINCT column)的情况
          columnName += " " + currentToken_.getLexeme();
          consume();

          if (match(Token::IDENTIFIER)) {
            columnName += " " + currentToken_.getLexeme();
            consume();
          }
        }

        // 检查是否有更多参数（简化处理，只支持简单情况）
        while (match(Token::COMMA)) {
          columnName += ", ";
          consume();

          if (match(Token::IDENTIFIER)) {
            columnName += currentToken_.getLexeme();
            consume();
          } else if (match(Token::INTEGER_LITERAL) ||
                     match(Token::FLOAT_LITERAL)) {
            columnName += currentToken_.getLexeme();
            consume();
          } else if (match(Token::STRING_LITERAL)) {
            columnName += "'" + currentToken_.getLexeme() + "'";
            consume();
          }
        }

        columnName += ")";
        expect(Token::RPAREN, "Expected ) after function arguments");
      } else {
        // 检查是否有点号（table.column）
        if (match(Token::DOT)) {
          consume(); // 消费点号

          if (!match(Token::IDENTIFIER)) {
            reportError("Expected column name after dot");
            return;
          }

          columnName += "." + currentToken_.getLexeme();
          consume();
        }
      }

      stmt.addSelectColumn(columnName);
    } else {
      reportError("Expected column name or *");
      return;
    }

    // 检查是否有更多列
    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (match(Token::IDENTIFIER));
}

void Parser::parseFromClause(SelectStatement &stmt) {
  consume(Token::KEYWORD_FROM);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();
  stmt.setTableName(tableName);

  // 处理JOIN子句
  while (true) {
    // 检查是否有JOIN关键字
    if (match(Token::IDENTIFIER) || match(Token::KEYWORD_JOIN)) {
      std::string tokenLexeme = currentToken_.getLexeme();
      std::string joinType;

      // 检查JOIN类型
      if (tokenLexeme == "JOIN" || tokenLexeme == "INNER") {
        if (tokenLexeme == "INNER") {
          joinType = "INNER";
          consume();
          if (match(Token::KEYWORD_JOIN) ||
              currentToken_.getLexeme() == "JOIN") {
            consume();
          }
        } else {
          joinType = "INNER";
          consume();
        }
      } else if (tokenLexeme == "LEFT" || tokenLexeme == "RIGHT" ||
                 tokenLexeme == "FULL" || tokenLexeme == "CROSS") {
        joinType = tokenLexeme;
        consume();
        if (match(Token::KEYWORD_OUTER) ||
            currentToken_.getLexeme() == "OUTER") {
          consume();
          joinType += " OUTER";
        }
        if (match(Token::KEYWORD_JOIN) || currentToken_.getLexeme() == "JOIN") {
          consume();
        }
      } else {
        // 不是JOIN关键字，退出循环
        break;
      }

      // 解析JOIN的表名
      if (!match(Token::IDENTIFIER)) {
        reportError("Expected table name after JOIN");
        return;
      }

      std::string joinTableName = currentToken_.getLexeme();
      consume();

      // 解析ON子句（可选，CROSS JOIN不需要ON子句）
      if (joinType != "CROSS") {
        if (match(Token::KEYWORD_ON)) {
          consume();
          // 简化处理，只解析基本的连接条件
          parseJoinCondition(stmt);
        }
      }
    } else {
      // 没有JOIN关键字，退出循环
      break;
    }
  }
}

void Parser::parseJoinCondition(SelectStatement &stmt) {
  // 简化处理，只解析 table1.column1 = table2.column2 形式的连接条件
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name in JOIN condition");
    return;
  }

  std::string table1 = currentToken_.getLexeme();
  consume();

  if (!match(Token::DOT)) {
    reportError("Expected . in JOIN condition");
    return;
  }
  consume();

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected column name in JOIN condition");
    return;
  }

  std::string column1 = currentToken_.getLexeme();
  consume();

  if (!match(Token::OPERATOR_EQUAL)) {
    reportError("Expected = in JOIN condition");
    return;
  }
  consume();

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name in JOIN condition");
    return;
  }

  std::string table2 = currentToken_.getLexeme();
  consume();

  if (!match(Token::DOT)) {
    reportError("Expected . in JOIN condition");
    return;
  }
  consume();

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected column name in JOIN condition");
    return;
  }

  std::string column2 = currentToken_.getLexeme();
  consume();

  // 保存连接条件（简化处理，只保存第一个连接条件）
  if (stmt.getJoinCondition().empty()) {
    stmt.setJoinCondition(table1 + "." + column1 + " = " + table2 + "." +
                          column2);
  }
}

void Parser::parseWhereClause(SelectStatement &stmt) {
  consume(Token::KEYWORD_WHERE);

  // 解析条件表达式
  std::string condition;

  // 简化处理，只收集条件字符串，不进行详细解析
  int paren_count = 0;
  while ((paren_count > 0 ||
          !match(Token::KEYWORD_GROUP) && !match(Token::KEYWORD_ORDER) &&
              !match(Token::KEYWORD_LIMIT) && !match(Token::KEYWORD_OFFSET) &&
              !match(Token::SEMICOLON) && !match(Token::END_OF_INPUT)) &&
         !match(Token::KEYWORD_JOIN)) {
    if (match(Token::LPAREN)) {
      paren_count++;
    } else if (match(Token::RPAREN)) {
      paren_count--;
    }

    // 处理子查询
    if (match(Token::KEYWORD_SELECT) || match(Token::LPAREN)) {
      condition += currentToken_.getLexeme();
      consume();
      // 处理子查询内容
      int subquery_paren = 0;
      if (match(Token::LPAREN)) {
        subquery_paren++;
      }

      while ((subquery_paren > 0 ||
              !match(Token::KEYWORD_FROM) && !match(Token::RPAREN)) &&
             !match(Token::END_OF_INPUT)) {
        if (match(Token::LPAREN)) {
          subquery_paren++;
        } else if (match(Token::RPAREN)) {
          subquery_paren--;
        }
        condition += " " + currentToken_.getLexeme();
        consume();
      }

      if (match(Token::KEYWORD_FROM)) {
        // 继续处理FROM子句
        condition += " " + currentToken_.getLexeme();
        consume();

        // 处理表名
        if (match(Token::IDENTIFIER)) {
          condition += " " + currentToken_.getLexeme();
          consume();
        }

        // 处理子查询的WHERE子句
        if (match(Token::KEYWORD_WHERE)) {
          condition += " " + currentToken_.getLexeme();
          consume();

          // 处理WHERE条件直到结束
          while (!match(Token::RPAREN) && !match(Token::END_OF_INPUT) &&
                 !match(Token::KEYWORD_GROUP) && !match(Token::KEYWORD_ORDER)) {
            condition += " " + currentToken_.getLexeme();
            consume();
          }
        }
      }
    } else {
      // 普通条件
      condition += " " + currentToken_.getLexeme();
      consume();
    }
  }

  // 简化处理，只保存条件字符串
  WhereClause whereClause("", "", condition);
  stmt.setWhereClause(whereClause);
}

void Parser::parseGroupByClause(SelectStatement &stmt) {
  consume(Token::KEYWORD_GROUP);
  expect(Token::KEYWORD_BY, "Expected BY after GROUP");

  // 支持多个列名的GROUP BY子句
  do {
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name in GROUP BY");
      return;
    }

    std::string columnName = currentToken_.getLexeme();
    consume();
    stmt.setGroupByColumn(columnName);

    // 检查是否有聚合函数
    if (match(Token::LPAREN)) {
      // 消费左括号和函数内容
      consume();
      // 简化处理，跳过函数内容直到右括号
      int paren_count = 1;
      while (paren_count > 0 && !match(Token::END_OF_INPUT)) {
        if (match(Token::LPAREN)) {
          paren_count++;
        } else if (match(Token::RPAREN)) {
          paren_count--;
        }
        consume();
      }
    }

    // 检查是否有更多列
    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (match(Token::IDENTIFIER) || match(Token::LPAREN));
}

void Parser::parseOrderByClause(SelectStatement &stmt) {
  consume(Token::KEYWORD_ORDER);
  expect(Token::KEYWORD_BY, "Expected BY after ORDER");

  // 支持多列排序，但只使用第一列（保持向后兼容）
  do {
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name in ORDER BY");
      return;
    }

    std::string columnName = currentToken_.getLexeme();
    consume();

    // 只保存第一列的排序
    if (stmt.getOrderByColumn().empty()) {
      stmt.setOrderByColumn(columnName);

      // 可选的ASC/DESC
      if (match(Token::IDENTIFIER)) {
        std::string direction = currentToken_.getLexeme();
        // 转换为大写
        std::transform(direction.begin(), direction.end(), direction.begin(),
                       ::toupper);
        if (direction == "ASC" || direction == "DESC") {
          consume();
          stmt.setOrderDirection(direction);
        }
      }
    } else {
      // 跳过后续列的ASC/DESC
      if (match(Token::IDENTIFIER)) {
        std::string direction = currentToken_.getLexeme();
        std::transform(direction.begin(), direction.end(), direction.begin(),
                       ::toupper);
        if (direction == "ASC" || direction == "DESC") {
          consume();
        }
      }
    }

    // 检查是否有更多列
    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (match(Token::IDENTIFIER));
}

void Parser::parseLimitOffsetClause(SelectStatement &stmt) {
  // 处理LIMIT关键字
  if (match(Token::KEYWORD_LIMIT) || currentToken_.getLexeme() == "LIMIT") {
    consume();
  }

  // 解析LIMIT值
  if (match(Token::INTEGER_LITERAL)) {
    stmt.setLimit(std::stoi(currentToken_.getLexeme()));
    consume();
  } else {
    reportError("Expected number after LIMIT");
    return;
  }

  // 可选的OFFSET子句
  if (match(Token::KEYWORD_OFFSET) || currentToken_.getLexeme() == "OFFSET") {
    consume();

    // 解析OFFSET值
    if (match(Token::INTEGER_LITERAL)) {
      stmt.setOffset(std::stoi(currentToken_.getLexeme()));
      consume();
    } else {
      reportError("Expected number after OFFSET");
      return;
    }
  }
}

std::unique_ptr<InsertStatement> Parser::parseInsertStatement() {
  consume(Token::KEYWORD_INSERT);
  expect(Token::KEYWORD_INTO, "Expected INTO after INSERT");

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<InsertStatement>(tableName);

  // 可选的列列表
  if (match(Token::LPAREN)) {
    parseInsertColumns(*stmt);
  }

  expect(Token::KEYWORD_VALUES, "Expected VALUES");
  parseInsertValues(*stmt);

  return stmt;
}

void Parser::parseInsertColumns(InsertStatement &stmt) {
  consume(Token::LPAREN);

  do {
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name");
      return;
    }

    std::string columnName = currentToken_.getLexeme();
    stmt.addColumn(columnName);
    consume();

    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (!match(Token::RPAREN));

  consume(Token::RPAREN);
}

void Parser::parseInsertValues(InsertStatement &stmt) {
  consume(Token::LPAREN);

  do {
    if (match(Token::STRING_LITERAL) || match(Token::INTEGER_LITERAL) ||
        match(Token::FLOAT_LITERAL)) {
      std::string value = currentToken_.getLexeme();
      stmt.addValue(value);
      consume();
    } else if (match(Token::KEYWORD_NULL) ||
               currentToken_.getLexeme() == "NULL") {
      // 处理NULL值
      stmt.addValue("NULL");
      consume();
    } else {
      reportError("Expected string, number, or NULL literal");
      return;
    }

    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (!match(Token::RPAREN));

  consume(Token::RPAREN);
}

std::unique_ptr<UpdateStatement> Parser::parseUpdateStatement() {
  consume(Token::KEYWORD_UPDATE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<UpdateStatement>(tableName);

  parseUpdateSetClause(*stmt);

  if (match(Token::KEYWORD_WHERE)) {
    parseWhereClause(*stmt);
  }

  return stmt;
}

void Parser::parseUpdateSetClause(UpdateStatement &stmt) {
  expect(Token::KEYWORD_SET, "Expected SET");

  do {
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected column name");
      return;
    }

    std::string columnName = currentToken_.getLexeme();
    consume();

    expect(Token::OPERATOR_EQUAL, "Expected =");

    if (match(Token::STRING_LITERAL) || match(Token::INTEGER_LITERAL) ||
        match(Token::FLOAT_LITERAL)) {
      std::string value = currentToken_.getLexeme();
      stmt.addUpdateValue(columnName, value);
      consume();
    } else {
      reportError("Expected value in SET clause");
      return;
    }

    if (!match(Token::COMMA)) {
      break;
    }

    consume(); // 消费逗号

  } while (!match(Token::KEYWORD_WHERE) && !match(Token::SEMICOLON) &&
           !match(Token::END_OF_INPUT));
}

void Parser::parseWhereClause(UpdateStatement &stmt) {
  consume(Token::KEYWORD_WHERE);

  // 简化处理，只处理简单的条件
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected column name");
    return;
  }

  std::string columnName = currentToken_.getLexeme();
  consume();

  if (!match(Token::OPERATOR_EQUAL) && !match(Token::OPERATOR_NOT_EQUAL) &&
      !match(Token::OPERATOR_LESS_THAN) && !match(Token::OPERATOR_LESS_EQUAL) &&
      !match(Token::OPERATOR_GREATER_THAN) &&
      !match(Token::OPERATOR_GREATER_EQUAL)) {
    reportError("Expected comparison operator");
    return;
  }

  std::string op = currentToken_.getLexeme();
  consume();

  if (!match(Token::STRING_LITERAL) && !match(Token::INTEGER_LITERAL) &&
      !match(Token::FLOAT_LITERAL)) {
    reportError("Expected value");
    return;
  }

  std::string value = currentToken_.getLexeme();
  consume();

  WhereClause whereClause(columnName, op, value);
  stmt.setWhereClause(whereClause);
}

std::unique_ptr<DeleteStatement> Parser::parseDeleteStatement() {
  consume(Token::KEYWORD_DELETE);
  expect(Token::KEYWORD_FROM, "Expected FROM after DELETE");

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name after DELETE FROM");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<DeleteStatement>(tableName);

  if (match(Token::KEYWORD_WHERE)) {
    parseWhereClause(*stmt);
  }

  return stmt;
}

void Parser::parseWhereClause(DeleteStatement &stmt) {
  consume(Token::KEYWORD_WHERE);

  // 简化处理，只处理简单的条件
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected column name");
    return;
  }

  std::string columnName = currentToken_.getLexeme();
  consume();

  if (!match(Token::OPERATOR_EQUAL) && !match(Token::OPERATOR_NOT_EQUAL) &&
      !match(Token::OPERATOR_LESS_THAN) && !match(Token::OPERATOR_LESS_EQUAL) &&
      !match(Token::OPERATOR_GREATER_THAN) &&
      !match(Token::OPERATOR_GREATER_EQUAL)) {
    reportError("Expected comparison operator");
    return;
  }

  std::string op = currentToken_.getLexeme();
  consume();

  if (!match(Token::STRING_LITERAL) && !match(Token::INTEGER_LITERAL) &&
      !match(Token::FLOAT_LITERAL)) {
    reportError("Expected value");
    return;
  }

  std::string value = currentToken_.getLexeme();
  consume();

  WhereClause whereClause(columnName, op, value);
  stmt.setWhereClause(whereClause);
}

std::unique_ptr<Statement> Parser::parseDropStatement() {
  consume(Token::KEYWORD_DROP);

  if (match(Token::KEYWORD_DATABASE)) {
    auto stmt = parseDropDatabaseStatement();
    return std::unique_ptr<Statement>(stmt.release());
  } else if (match(Token::KEYWORD_TABLE)) {
    auto stmt = parseDropTableStatement();
    return std::unique_ptr<Statement>(stmt.release());
  } else if (match(Token::KEYWORD_INDEX)) {
    auto stmt = parseDropIndexStatement();
    return std::unique_ptr<Statement>(stmt.release());
  } else if (match(Token::KEYWORD_USER)) {
    // 处理DROP USER语句
    consume(); // 消耗USER关键字
    return parseDropUserStatement();
  } else {
    reportError("Expected DATABASE, TABLE, INDEX, or USER after DROP");
    return nullptr;
  }
}

std::unique_ptr<DropStatement> Parser::parseDropDatabaseStatement() {
  consume(Token::KEYWORD_DATABASE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected database name");
    return nullptr;
  }

  std::string dbName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<DropStatement>(DropStatement::DATABASE, dbName);

  return stmt;
}

std::unique_ptr<DropStatement> Parser::parseDropTableStatement() {
  consume(Token::KEYWORD_TABLE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected table name");
    return nullptr;
  }

  std::string tableName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<DropStatement>(DropStatement::TABLE, tableName);

  return stmt;
}

std::unique_ptr<DropIndexStatement> Parser::parseDropIndexStatement() {
  consume(Token::KEYWORD_INDEX);

  // 检查IF EXISTS
  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume();
    expect(Token::KEYWORD_EXISTS, "Expected EXISTS after IF");
    ifExists = true;
  }

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected index name");
    return nullptr;
  }

  std::string indexName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<DropIndexStatement>(indexName);
  stmt->setIfExists(ifExists);

  // 检查ON table_name
  if (match(Token::KEYWORD_ON)) {
    consume();

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected table name after ON");
      return nullptr;
    }

    stmt->setTableName(currentToken_.getLexeme());
    consume();
  }

  return stmt;
}

std::unique_ptr<AlterStatement> Parser::parseAlterStatement() {
  consume(Token::KEYWORD_ALTER);

  if (match(Token::KEYWORD_DATABASE)) {
    return parseAlterDatabaseStatement();
  } else {
    reportError("Unsupported ALTER statement type: " +
                currentToken_.getLexeme());
    return nullptr;
  }
}

std::unique_ptr<AlterStatement> Parser::parseAlterDatabaseStatement() {
  consume(Token::KEYWORD_DATABASE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected database name");
    return nullptr;
  }

  std::string dbName = currentToken_.getLexeme();
  consume();

  auto stmt =
      std::make_unique<AlterStatement>(AlterStatement::DATABASE, dbName);

  return stmt;
}

std::unique_ptr<UseStatement> Parser::parseUseStatement() {
  consume(Token::KEYWORD_USE);

  if (!match(Token::IDENTIFIER)) {
    reportError("Expected database name");
    return nullptr;
  }

  std::string dbName = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<UseStatement>(dbName);

  return stmt;
}

// DCL语句解析方法
std::unique_ptr<Statement> Parser::parseCreateUserStatement() {
  // USER关键字已经被parseCreateStatement函数消耗了，所以这里不需要再消耗

  // 检查当前token是否是用户名
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected username");
    return nullptr;
  }

  std::string username = currentToken_.getLexeme();
  consume();

  std::string password = "";
  bool withPassword = false;

  // 支持两种语法：IDENTIFIED BY 和 WITH PASSWORD
  if (match(Token::KEYWORD_IDENTIFIED)) {
    consume();
    expect(Token::KEYWORD_BY, "Expected BY keyword after IDENTIFIED");

    if (!match(Token::STRING)) {
      reportError("Expected password string");
      return nullptr;
    }

    password = currentToken_.getLexeme();
    // 移除字符串引号
    if (password.length() >= 2 && password.front() == '\'' &&
        password.back() == '\'') {
      password = password.substr(1, password.length() - 2);
    }
    consume();
  } else if (match(Token::KEYWORD_WITH)) {
    consume();
    expect(Token::KEYWORD_PASSWORD, "Expected PASSWORD keyword");
    withPassword = true;

    if (!match(Token::STRING)) {
      reportError("Expected password string");
      return nullptr;
    }

    password = currentToken_.getLexeme();
    // 移除字符串引号
    if (password.length() >= 2 && password.front() == '\'' &&
        password.back() == '\'') {
      password = password.substr(1, password.length() - 2);
    }
    consume();
  }

  auto stmt = std::make_unique<CreateUserStatement>(username, password);
  stmt->setWithPassword(withPassword);

  return stmt;
}

std::unique_ptr<Statement> Parser::parseDropUserStatement() {
  // USER关键字已经被parseDropStatement函数消耗了，所以这里不需要再消耗

  bool ifExists = false;
  if (match(Token::KEYWORD_IF)) {
    consume();
    expect(Token::KEYWORD_EXISTS, "Expected EXISTS after IF");
    ifExists = true;
  }

  // 直接读取当前token作为用户名
  std::string username = currentToken_.getLexeme();
  consume();

  auto stmt = std::make_unique<DropUserStatement>(username);
  stmt->setIfExists(ifExists);

  return stmt;
}

std::unique_ptr<Statement> Parser::parseGrantStatement() {
  consume(Token::KEYWORD_GRANT);

  auto stmt = std::make_unique<GrantStatement>();

  // 解析权限列表
  // 特殊处理 ALL PRIVILEGES
  if (match(Token::IDENTIFIER) && currentToken_.getLexeme() == "ALL") {
    consume();
    if ((match(Token::IDENTIFIER) || match(Token::KEYWORD_PRIVILEGES)) &&
        currentToken_.getLexeme() == "PRIVILEGES") {
      consume();
    }
    stmt->addPrivilege("ALL");
  } else {
    do {
      if (!(match(Token::IDENTIFIER) ||
            currentToken_.getType() == Token::KEYWORD_SELECT ||
            currentToken_.getType() == Token::KEYWORD_INSERT ||
            currentToken_.getType() == Token::KEYWORD_UPDATE ||
            currentToken_.getType() == Token::KEYWORD_DELETE)) {
        reportError("Expected privilege");
        return nullptr;
      }

      std::string privilege = currentToken_.getLexeme();
      stmt->addPrivilege(privilege);
      consume();

      if (!match(Token::COMMA)) {
        break;
      }

      consume(); // 消费逗号
    } while (true);
  }

  // 检查是否有ON关键字，如果没有则直接解析TO
  if (match(Token::KEYWORD_ON)) {
    consume();

    // 解析对象类型
    if (!(match(Token::IDENTIFIER) || match(Token::KEYWORD_TABLE) ||
          match(Token::KEYWORD_DATABASE))) {
      reportError("Expected object type");
      return nullptr;
    }

    std::string objectType = currentToken_.getLexeme();
    // 转换为大写
    std::transform(objectType.begin(), objectType.end(), objectType.begin(),
                   ::toupper);
    stmt->setObjectType(objectType);
    consume();

    // 解析对象名称
    if (!match(Token::IDENTIFIER)) {
      reportError("Expected object name");
      return nullptr;
    }

    std::string objectName = currentToken_.getLexeme();
    stmt->setObjectName(objectName);
    consume();
  }

  expect(Token::KEYWORD_TO, "Expected TO keyword");

  // 解析被授权者
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected grantee");
    return nullptr;
  }

  std::string grantee = currentToken_.getLexeme();
  stmt->setGrantee(grantee);
  consume();

  return stmt;
}

std::unique_ptr<Statement> Parser::parseRevokeStatement() {
  consume(Token::KEYWORD_REVOKE);

  auto stmt = std::make_unique<RevokeStatement>();

  // 解析权限列表
  // 特殊处理 ALL PRIVILEGES
  if (match(Token::IDENTIFIER) && currentToken_.getLexeme() == "ALL") {
    consume();
    if ((match(Token::IDENTIFIER) || match(Token::KEYWORD_PRIVILEGES)) &&
        currentToken_.getLexeme() == "PRIVILEGES") {
      consume();
    }
    stmt->addPrivilege("ALL");
  } else {
    do {
      if (!(match(Token::IDENTIFIER) ||
            currentToken_.getType() == Token::KEYWORD_SELECT ||
            currentToken_.getType() == Token::KEYWORD_INSERT ||
            currentToken_.getType() == Token::KEYWORD_UPDATE ||
            currentToken_.getType() == Token::KEYWORD_DELETE)) {
        reportError("Expected privilege");
        return nullptr;
      }

      std::string privilege = currentToken_.getLexeme();
      stmt->addPrivilege(privilege);
      consume();

      if (!match(Token::COMMA)) {
        break;
      }

      consume(); // 消费逗号
    } while (true);
  }

  expect(Token::KEYWORD_ON, "Expected ON keyword");

  // 解析对象类型
  if (!(match(Token::IDENTIFIER) || match(Token::KEYWORD_TABLE) ||
        match(Token::KEYWORD_DATABASE))) {
    reportError("Expected object type");
    return nullptr;
  }

  std::string objectType = currentToken_.getLexeme();
  // 转换为大写
  std::transform(objectType.begin(), objectType.end(), objectType.begin(),
                 ::toupper);
  stmt->setObjectType(objectType);
  consume();

  // 解析对象名称
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected object name");
    return nullptr;
  }

  std::string objectName = currentToken_.getLexeme();
  stmt->setObjectName(objectName);
  consume();

  expect(Token::KEYWORD_FROM, "Expected FROM keyword");

  // 解析被撤销权限者
  if (!match(Token::IDENTIFIER)) {
    reportError("Expected revokee");
    return nullptr;
  }

  std::string grantee = currentToken_.getLexeme();
  stmt->setGrantee(grantee);
  consume();

  return stmt;
}

std::unique_ptr<Statement> Parser::parseShowStatement() {
  consume(Token::KEYWORD_SHOW);

  // 判断SHOW的类型
  if (match(Token::KEYWORD_DATABASES)) {
    consume();
    auto stmt = std::make_unique<ShowStatement>(ShowStatement::DATABASES);
    return stmt;
  } else if (match(Token::KEYWORD_TABLES)) {
    consume();
    auto stmt = std::make_unique<ShowStatement>(ShowStatement::TABLES);

    // 检查是否有FROM子句：SHOW TABLES [FROM db]
    if (match(Token::KEYWORD_FROM)) {
      consume();
      if (!match(Token::IDENTIFIER)) {
        reportError("Expected database name after FROM");
        return nullptr;
      }
      stmt->setFromDatabase(currentToken_.getLexeme());
      consume();
    }

    return stmt;
  } else if (match(Token::KEYWORD_CREATE)) {
    consume();
    expect(Token::KEYWORD_TABLE, "Expected TABLE after CREATE");

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected table name");
      return nullptr;
    }

    auto stmt = std::make_unique<ShowStatement>(ShowStatement::CREATE_TABLE);
    stmt->setTargetObject(currentToken_.getLexeme());
    consume();

    return stmt;
  } else if (match(Token::KEYWORD_COLUMNS)) {
    consume();
    expect(Token::KEYWORD_FROM, "Expected FROM after COLUMNS");

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected table name");
      return nullptr;
    }

    auto stmt = std::make_unique<ShowStatement>(ShowStatement::COLUMNS);
    stmt->setTargetObject(currentToken_.getLexeme());
    consume();

    return stmt;
  } else if (match(Token::KEYWORD_INDEXES)) {
    consume();
    expect(Token::KEYWORD_FROM, "Expected FROM after INDEXES");

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected table name");
      return nullptr;
    }

    auto stmt = std::make_unique<ShowStatement>(ShowStatement::INDEXES);
    stmt->setTargetObject(currentToken_.getLexeme());
    consume();

    return stmt;
  } else if (match(Token::KEYWORD_GRANTS)) {
    consume();
    expect(Token::KEYWORD_FOR, "Expected FOR after GRANTS");

    if (!match(Token::IDENTIFIER)) {
      reportError("Expected user name");
      return nullptr;
    }

    auto stmt = std::make_unique<ShowStatement>(ShowStatement::GRANTS);
    stmt->setTargetObject(currentToken_.getLexeme());
    consume();

    return stmt;
  } else {
    reportError("Unexpected token after SHOW: " + currentToken_.getLexeme());
    return nullptr;
  }
}

} // namespace sql_parser
} // namespace sqlcc
