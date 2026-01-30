/**
 * SelectParser - SQL SELECT语句解析器实现
 *
 * 此文件实现了SelectParser类，专门负责SQL SELECT语句的解析。
 * 采用分步骤解析策略，每个子句都有专门的处理方法。
 */

#include "select_parser.h"
#include "token.h"
#include <iostream>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

SelectParser::SelectParser(TokenStream& tokens, ExpressionParser& expr_parser)
    : tokens_(tokens), expr_parser_(expr_parser) {
  std::cout << "[SELECT_PARSER] SelectParser initialized" << std::endl;
}

std::unique_ptr<SelectStatement> SelectParser::parse() {
  std::cout << "[SELECT_PARSER] parse() called - parsing complete SELECT statement" << std::endl;

  auto stmt = std::make_unique<SelectStatement>();

  // 解析SELECT子句（包括DISTINCT和选择列表）
  parseSelectClause(*stmt);

  // 解析FROM子句和JOIN
  parseFromClause(*stmt);

  // 解析WHERE子句
  parseWhereClause(*stmt);

  // 解析GROUP BY子句
  parseGroupByClause(*stmt);

  // 解析HAVING子句
  parseHavingClause(*stmt);

  // 解析ORDER BY子句
  parseOrderByClause(*stmt);

  std::cout << "[SELECT_PARSER] SELECT statement parsing completed" << std::endl;
  return stmt;
}

void SelectParser::parseSelectClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseSelectClause() called" << std::endl;

  // 消费SELECT关键字（在调用此方法前已经消费）
  // tokens_.consume(Type::KEYWORD_SELECT);

  // 检查DISTINCT
  if (tokens_.check(Type::KEYWORD_DISTINCT)) {
    std::cout << "[SELECT_PARSER] Found DISTINCT keyword" << std::endl;
    stmt.setDistinct(true);
  }

  // 解析选择列表
  if (tokens_.check(Type::OPERATOR_MULTIPLY)) {
    // SELECT *
    std::cout << "[SELECT_PARSER] Found SELECT *" << std::endl;
    stmt.setSelectAll(true);
  } else {
    // 解析选择项列表
    bool first = true;
    while (!tokens_.check(Type::KEYWORD_FROM) && !tokens_.isAtEnd()) {
      if (!first) {
        if (!tokens_.check(Type::COMMA)) {
          break;
        }
      }
      first = false;

      std::string selectItem = parseSelectItem();
      stmt.addSelectColumn(std::make_unique<IdentifierExpression>(selectItem));
      std::cout << "[SELECT_PARSER] Added select column: " << selectItem << std::endl;
    }
  }
}

void SelectParser::parseFromClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseFromClause() called" << std::endl;

  if (!tokens_.check(Type::KEYWORD_FROM)) {
    std::cout << "[SELECT_PARSER] No FROM clause found" << std::endl;
    return;
  }

  // 解析表名
  tokens_.expect(Type::IDENTIFIER);
  std::string tableName = tokens_.current().getLexeme();
  stmt.setTableName(tableName);
  stmt.addFromTable(tableName);
  std::cout << "[SELECT_PARSER] FROM table: " << tableName << std::endl;

  // 解析JOIN子句（支持多个）
  while (true) {
    if (tokens_.check(Type::KEYWORD_JOIN) ||
        tokens_.check(Type::KEYWORD_INNER) ||
        tokens_.check(Type::KEYWORD_LEFT) ||
        tokens_.check(Type::KEYWORD_RIGHT) ||
        tokens_.check(Type::KEYWORD_FULL)) {
      auto joinClause = parseJoinClause();
      if (joinClause) {
        stmt.addJoinClause(std::move(joinClause));
      }
    } else {
      break;
    }
  }
}

std::unique_ptr<JoinClause> SelectParser::parseJoinClause() {
  std::cout << "[SELECT_PARSER] parseJoinClause() called" << std::endl;

  JoinClause::JoinType joinType = JoinClause::INNER_JOIN;

  // 确定JOIN类型
  if (tokens_.check(Type::KEYWORD_INNER)) {
    joinType = JoinClause::INNER_JOIN;
    std::cout << "[SELECT_PARSER] INNER JOIN detected" << std::endl;
  } else if (tokens_.check(Type::KEYWORD_LEFT)) {
    if (tokens_.check(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[SELECT_PARSER] LEFT OUTER JOIN detected" << std::endl;
    } else {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[SELECT_PARSER] LEFT JOIN detected" << std::endl;
    }
  } else if (tokens_.check(Type::KEYWORD_RIGHT)) {
    if (tokens_.check(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[SELECT_PARSER] RIGHT OUTER JOIN detected" << std::endl;
    } else {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[SELECT_PARSER] RIGHT JOIN detected" << std::endl;
    }
  } else if (tokens_.check(Type::KEYWORD_FULL)) {
    if (tokens_.check(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[SELECT_PARSER] FULL OUTER JOIN detected" << std::endl;
    } else {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[SELECT_PARSER] FULL JOIN detected" << std::endl;
    }
  }

  // 消费JOIN关键字
  tokens_.expect(Type::KEYWORD_JOIN), tokens_.advance();

  // 解析表名
  tokens_.expect(Type::IDENTIFIER);
  std::string tableName = tokens_.current().getLexeme();
  std::cout << "[SELECT_PARSER] JOIN table: " << tableName << std::endl;

  // 解析ON条件（简化实现）
  std::unique_ptr<Expression> condition = nullptr;
  if (tokens_.check(Type::KEYWORD_ON)) {
    std::cout << "[SELECT_PARSER] Parsing ON condition" << std::endl;
    condition = expr_parser_.parseExpression();
  }

  return std::make_unique<JoinClause>(joinType, tableName, std::move(condition));
}

void SelectParser::parseWhereClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseWhereClause() called" << std::endl;

  if (!tokens_.check(Type::KEYWORD_WHERE)) {
    std::cout << "[SELECT_PARSER] No WHERE clause found" << std::endl;
    return;
  }

  // 解析WHERE条件表达式
  std::cout << "[SELECT_PARSER] Parsing WHERE condition" << std::endl;
  auto condition = expr_parser_.parseExpression();

  // 暂时简化：创建WhereClause对象
  // 这里需要根据实际的WhereClause构造函数来调整
  // stmt.setWhereClause(WhereClause("column", "op", "value"));
  std::cout << "[SELECT_PARSER] WHERE condition parsed (simplified)" << std::endl;
}

void SelectParser::parseGroupByClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseGroupByClause() called" << std::endl;

  if (!tokens_.check(Type::KEYWORD_GROUP)) {
    std::cout << "[SELECT_PARSER] No GROUP BY clause found" << std::endl;
    return;
  }

  tokens_.expect(Type::KEYWORD_BY), tokens_.advance();

  // 解析GROUP BY列列表
  bool first = true;
  while (!tokens_.check(Type::KEYWORD_HAVING) &&
         !tokens_.check(Type::KEYWORD_ORDER) &&
         !tokens_.check(Type::KEYWORD_LIMIT) &&
         !tokens_.check(Type::SEMICOLON) &&
         !tokens_.isAtEnd()) {
    if (!first) {
      if (!tokens_.check(Type::COMMA)) {
        break;
      }
    }
    first = false;

    tokens_.expect(Type::IDENTIFIER);
    std::string column = tokens_.current().getLexeme();
    stmt.addGroupByColumn(column);
    std::cout << "[SELECT_PARSER] GROUP BY column: " << column << std::endl;
  }
}

void SelectParser::parseHavingClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseHavingClause() called" << std::endl;

  if (!tokens_.check(Type::KEYWORD_HAVING)) {
    std::cout << "[SELECT_PARSER] No HAVING clause found" << std::endl;
    return;
  }

  // 解析HAVING条件表达式
  std::cout << "[SELECT_PARSER] Parsing HAVING condition" << std::endl;
  auto condition = expr_parser_.parseExpression();

  std::cout << "[SELECT_PARSER] HAVING condition parsed (simplified)" << std::endl;
}

void SelectParser::parseOrderByClause(SelectStatement& stmt) {
  std::cout << "[SELECT_PARSER] parseOrderByClause() called" << std::endl;

  if (!tokens_.check(Type::KEYWORD_ORDER)) {
    std::cout << "[SELECT_PARSER] No ORDER BY clause found" << std::endl;
    return;
  }

  tokens_.expect(Type::KEYWORD_BY), tokens_.advance();

  // 解析ORDER BY列
  tokens_.expect(Type::IDENTIFIER);
  std::string orderByColumn = tokens_.current().getLexeme();
  stmt.setOrderByColumn(orderByColumn);

  // 检查排序方向
  if (tokens_.check(Type::KEYWORD_ASC)) {
    stmt.setOrderDirection("ASC");
    std::cout << "[SELECT_PARSER] ORDER BY direction: ASC" << std::endl;
  } else if (tokens_.check(Type::KEYWORD_DESC)) {
    stmt.setOrderDirection("DESC");
    std::cout << "[SELECT_PARSER] ORDER BY direction: DESC" << std::endl;
  } else {
    // 默认升序
    stmt.setOrderDirection("ASC");
    std::cout << "[SELECT_PARSER] ORDER BY direction: default ASC" << std::endl;
  }

  std::cout << "[SELECT_PARSER] ORDER BY column: " << orderByColumn << std::endl;
}

std::string SelectParser::parseSelectItem() {
  std::cout << "[SELECT_PARSER] parseSelectItem() called" << std::endl;

  std::string item;

  // 检查是否是函数调用
  if (tokens_.check(Type::IDENTIFIER)) {
    // 前瞻检查是否是函数
    if (tokens_.peek().getType() == Type::IDENTIFIER) {
      // 这里需要改进：应该检查下一个token是否是LPAREN
      // 暂时简化实现
      tokens_.expect(Type::IDENTIFIER);
      item = tokens_.current().getLexeme();

      // 检查是否是函数调用
      if (tokens_.check(Type::LPAREN)) {
        tokens_.expect(Type::LPAREN), tokens_.advance();
        // 简化：跳过参数
        int parenCount = 1;
        while (parenCount > 0 && !tokens_.isAtEnd()) {
          if (tokens_.check(Type::LPAREN)) {
            parenCount++;
            item += "(";
          } else if (tokens_.check(Type::RPAREN)) {
            parenCount--;
            if (parenCount > 0) {
              item += ")";
            }
          } else {
            auto tokenType = tokens_.peek().getType();
            tokens_.expect(tokenType);
            item += tokens_.current().getLexeme();
            if (parenCount > 0) {
              item += " ";
            }
          }
        }
        std::cout << "[SELECT_PARSER] Function call: " << item << std::endl;
      }
    }
  }

  // 如果还没有解析到内容，当作简单标识符
  if (item.empty()) {
    tokens_.expect(Type::IDENTIFIER);
    item = tokens_.current().getLexeme();
    std::cout << "[SELECT_PARSER] Simple identifier: " << item << std::endl;
  }

  // 检查是否有AS别名
  if (tokens_.check(Type::KEYWORD_AS)) {
    tokens_.expect(Type::KEYWORD_AS);
    tokens_.expect(Type::IDENTIFIER);
    std::string alias = tokens_.current().getLexeme();
    item += " AS " + alias;
    std::cout << "[SELECT_PARSER] Added alias: " << alias << std::endl;
  }

  std::cout << "[SELECT_PARSER] parseSelectItem completed: " << item << std::endl;

  return item;
}

} // namespace sql_parser
} // namespace sqlcc