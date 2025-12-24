#include <iostream>
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

int main() {
  using namespace sqlcc::sql_parser;
  std::string sql = "SELECT COUNT(*) FROM users;";
  Parser parser(sql);
  auto stmts = parser.parse();
  if (stmts.empty()) {
    std::cerr << "No statements parsed" << std::endl;
    return 2;
  }
  auto *sel = dynamic_cast<SelectStatement *>(stmts[0].get());
  if (!sel) {
    std::cerr << "First statement is not SELECT" << std::endl;
    return 3;
  }
  const auto &cols = sel->getSelectColumns();
  if (cols.empty()) {
    std::cerr << "No select columns parsed" << std::endl;
    return 4;
  }
  std::cout << "Parsed select column[0]: " << cols[0] << std::endl;
  // 简单校验包含 "count(" 子串（不区分大小写）
  std::string s = cols[0];
  for (auto &c : s) c = tolower(c);
  if (s.find("count(") == std::string::npos) {
    std::cerr << "COUNT not recognized in select column: " << cols[0] << std::endl;
    return 5;
  }
  std::cout << "Parser aggregate recognition test passed." << std::endl;
  return 0;
}
