#ifndef SQLCC_SQL_PARSER_AST_CORE_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_CORE_AST_NODE_H

#include "source_location.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {
namespace ast {

// Forward declaration
class ASTNode;

/**
 * @brief Visitor pattern base class for AST nodes
 *
 * Provides the visitor pattern interface for traversing and operating on AST nodes.
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    /**
     * @brief Visit an AST node
     * @param node The AST node to visit
     */
    virtual void visit(ASTNode& node) = 0;
};

/**
 * @brief Base class for all AST nodes
 *
 * Provides common functionality for all AST nodes including:
 * - Source location tracking for error reporting
 * - Visitor pattern support
 * - Clone functionality for AST transformations
 * - String representation for debugging
 */
class ASTNode {
public:
    /**
     * @brief Constructor
     * @param location Source location of this node
     */
    explicit ASTNode(const SourceLocation& location = SourceLocation());

    /**
     * @brief Virtual destructor
     */
    virtual ~ASTNode() = default;

    /**
     * @brief Accept a visitor (Visitor pattern)
     * @param visitor The visitor to accept
     */
    virtual void accept(ASTVisitor& visitor) = 0;

    /**
     * @brief Get source location
     * @return Source location of this node
     */
    const SourceLocation& getLocation() const;

    /**
     * @brief Set source location
     * @param location New source location
     */
    void setLocation(const SourceLocation& location);

    /**
     * @brief Clone this node (deep copy)
     * @return Unique pointer to cloned node
     */
    virtual std::unique_ptr<ASTNode> clone() const = 0;

    /**
     * @brief Get string representation for debugging
     * @return String representation of this node
     */
    virtual std::string toString() const = 0;

    /**
     * @brief Get node type name
     * @return Type name string (e.g., "SelectStatement")
     */
    virtual std::string getNodeType() const = 0;

    /**
     * @brief Check if node is valid
     * @return true if node is in a valid state
     */
    virtual bool isValid() const;

protected:
    SourceLocation location_;  ///< Source location of this node
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_CORE_AST_NODE_H
