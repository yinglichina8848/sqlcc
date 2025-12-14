#include "../../../include/sql_parser/ast_nodes.h"
#include "sql_parser/parser.h"
#include "sql_parser/parser_new.h"
#include <iostream>
#include <memory>
#include <string>

using namespace sqlcc::sql_parser;

int main() {
  // 测试 INSERT 语句解析
  std::string sql = "INSERT INTO users (name, age) VALUES ('John', 25);";

  std::cout << "测试 SQL: " << sql << std::endl;

  try {
    ParserNew parser(sql);

    // 使用公共的parse方法
    auto statements = parser.parse();

    if (statements.size() > 0) {
      std::cout << "解析成功！" << std::endl;
      std::cout << "解析了 " << statements.size() << " 条语句" << std::endl;

      auto &stmt = statements[0];
      std::cout << "语句类型: " << stmt->getTypeName() << std::endl;

      // 尝试将Statement转换为InsertStatement
      auto insertStmt = dynamic_cast<InsertStatement *>(stmt.get());
      if (insertStmt) {
        std::cout << "表名: " << insertStmt->getTableName() << std::endl;
        std::cout << "列数: " << insertStmt->getColumns().size() << std::endl;
        std::cout << "值行数: " << insertStmt->getValues().size() << std::endl;

        if (insertStmt->getValues().size() > 0) {
          std::cout << "第一行值: ";
          for (const auto &value : insertStmt->getValues()[0]) {
            std::cout << "'" << value << "' ";
          }
          std::cout << std::endl;
        }
      } else {
        std::cout << "不是 INSERT 语句" << std::endl;
      }
    } else {
      std::cout << "解析失败，没有语句！" << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cout << "异常: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}