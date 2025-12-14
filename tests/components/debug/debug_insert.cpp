#include "../../../include/sql_parser/ast_nodes.h"
#include "sql_parser/lexer.h"
#include "sql_parser/lexer_new.h"
#include "sql_parser/parser_new.h"
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== INSERT Statement Parsing Test ===" << std::endl;
    std::cout << std::endl;
    
    // Test INSERT statement
    std::string sql = "INSERT INTO test_table (id, name) VALUES (1, 'Alice');";
    std::cout << "Testing SQL: " << sql << std::endl;
    std::cout << std::endl;
    
    // Debug lexer output
    {
        std::cout << "=== LEXER DEBUG ===" << std::endl;
        sqlcc::sql_parser::LexerNew lexer(sql);
        sqlcc::sql_parser::Token token;
        while (true) {
            token = lexer.nextToken();
            std::cout << "Token: '" << token.getLexeme() 
                      << "' Type: " << static_cast<int>(token.getType()) 
                      << " (" << (token.getType() == sqlcc::sql_parser::Token::KEYWORD_INSERT ? "KEYWORD_INSERT" : "OTHER") << ")"
                      << std::endl;
            if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
                break;
            }
        }
        std::cout << std::endl;
    }

  try {
    sqlcc::sql_parser::ParserNew parser(sql);
    std::cout << "开始解析SQL语句..." << std::endl;
    auto statements = parser.parse();
    std::cout << "解析完成。" << std::endl;

    if (statements.empty()) {
      std::cout << "ERROR: No statements generated!" << std::endl;

      // Check if parser has errors
      // We need to add a method to check for errors in the parser
    } else {
      std::cout << "Parse completed. Statement count: " << statements.size()
                << std::endl;

      for (size_t i = 0; i < statements.size(); i++) {
        std::cout << "Statement " << i + 1
                  << ": Type = " << statements[i]->getTypeName() << std::endl;

        if (statements[i]->getTypeName() == "INSERT") {
          auto insertStmt = dynamic_cast<sqlcc::sql_parser::InsertStatement *>(
              statements[i].get());
          if (insertStmt) {
            std::cout << "  Table name: " << insertStmt->getTableName()
                      << std::endl;
            std::cout << "  Columns: ";
            for (const auto &col : insertStmt->getColumns()) {
              std::cout << col << " ";
            }
            std::cout << std::endl;
            std::cout << "  Values: ";
            for (const auto &row : insertStmt->getValues()) {
              std::cout << "[";
              for (const auto &val : row) {
                std::cout << val << " ";
              }
              std::cout << "]";
            }
            std::cout << std::endl;
          }
        }
      }
    }
  } catch (const std::exception &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  std::cout << std::endl << "=== Test completed ===" << std::endl;
  return 0;
}