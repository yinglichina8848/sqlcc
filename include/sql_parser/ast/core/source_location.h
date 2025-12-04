#ifndef SQLCC_SQL_PARSER_AST_CORE_SOURCE_LOCATION_H
#define SQLCC_SQL_PARSER_AST_CORE_SOURCE_LOCATION_H

#include <string>
#include <sstream>

namespace sqlcc {
namespace sql_parser {
namespace ast {

/**
 * @brief Source location information for AST nodes
 *
 * Tracks the position of AST nodes in the source SQL text for error reporting
 * and debugging purposes.
 */
struct SourceLocation {
    size_t line_;      ///< Line number (1-based)
    size_t column_;    ///< Column number (1-based)
    size_t offset_;    ///< Character offset from start (0-based)
    std::string file_; ///< Source file name (optional)

    /**
     * @brief Default constructor
     */
    SourceLocation(size_t line = 0, size_t column = 0,
                   size_t offset = 0, const std::string& file = "");

    /**
     * @brief Check if location is valid
     * @return true if location has valid line/column information
     */
    bool isValid() const;

    /**
     * @brief Merge two source locations (for multi-token spans)
     * @param other The other location to merge with
     * @return New location spanning both ranges
     */
    SourceLocation merge(const SourceLocation& other) const;

    /**
     * @brief Convert to string representation
     * @return String in format "file:line:column" or "line:column"
     */
    std::string toString() const;

    /**
     * @brief Convert to JSON representation
     * @return JSON string with location information
     */
    std::string toJson() const;

    /**
     * @brief Equality comparison
     */
    bool operator==(const SourceLocation& other) const;
    bool operator!=(const SourceLocation& other) const;

    /**
     * @brief Ordering comparison (by file, then line, then column)
     */
    bool operator<(const SourceLocation& other) const;
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_CORE_SOURCE_LOCATION_H
