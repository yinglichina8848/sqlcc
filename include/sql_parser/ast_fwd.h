#ifndef SQLCC_SQL_PARSER_AST_FWD_H
#define SQLCC_SQL_PARSER_AST_FWD_H

// Forward declarations for AST nodes to break circular dependencies
// This file should be included by execution layer instead of ast_nodes.h

namespace sqlcc {
namespace sql_parser {

// Base classes
class Statement;
class Expression;
class NodeVisitor;

// Statement types
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class CreateStatement;
class DropStatement;
class AlterStatement;
class UseStatement;
class CreateIndexStatement;
class DropIndexStatement;
class CreateUserStatement;
class DropUserStatement;
class GrantStatement;
class RevokeStatement;
class ShowStatement;
class CreateViewStatement;
class AlterViewStatement;
class DropViewStatement;
class CreateProcedureStatement;
class CallProcedureStatement;
class DropProcedureStatement;
class CreateTriggerStatement;
class DropTriggerStatement;
class AlterTriggerStatement;

// Expression types
class IdentifierExpression;
class StringLiteralExpression;
class NumericLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;

// Other types
class ColumnDefinition;
class TableConstraint;
class WhereClause;
class JoinClause;
class CompositeSelectStatement;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_FWD_H
