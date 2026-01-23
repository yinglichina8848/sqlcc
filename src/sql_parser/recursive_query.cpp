#include "ast/ast_node.h"
#include "recursive_query.h"
#include "ast/ast_nodes.h"

namespace sqlcc {
namespace sql_parser {

WithRecursiveClause::WithRecursiveClause(std::string cte_name,
                                       std::unique_ptr<SelectStatement> base_query,
                                       std::unique_ptr<SelectStatement> recursive_query)
    : Statement(Statement::WITH_RECURSIVE),
      cte_name_(std::move(cte_name)),
      base_query_(std::move(base_query)),
      recursive_query_(std::move(recursive_query)) {
}

WithRecursiveClause::~WithRecursiveClause() = default;

const std::string& WithRecursiveClause::getCteName() const {
    return cte_name_;
}

SelectStatement* WithRecursiveClause::getBaseQuery() const {
    return base_query_.get();
}

SelectStatement* WithRecursiveClause::getRecursiveQuery() const {
    return recursive_query_.get();
}

void WithRecursiveClause::accept(NodeVisitor& visitor) override {
    visitor.visit(*this);
}

} // namespace sql_parser
} // namespace sqlcc
