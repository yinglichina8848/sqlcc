#include <iostream>
#include <memory>

namespace sqlcc {
namespace sql_parser {

class Statement {
public:
    virtual ~Statement() = default;
};

class Parser {
public:
    Parser() = default;
    std::unique_ptr<Statement> parseStatement();
};

std::unique_ptr<Statement> Parser::parseStatement() {
    std::cout << "Parsing statement..." << std::endl;
    return nullptr;
}

} // namespace sql_parser
} // namespace sqlcc

int main() {
    sqlcc::sql_parser::Parser parser;
    parser.parseStatement();
    return 0;
}