#ifndef SQLCC_SQL_PARSER_ERRORS_ERROR_CORE_H
#define SQLCC_SQL_PARSER_ERRORS_ERROR_CORE_H

#include "sql_parser/ast/core/source_location.h"
#include <string>
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {
namespace errors {

/**
 * @brief Parse error types
 */
enum class ErrorType {
    // Lexical errors (词法错误)
    LEXICAL_INVALID_CHARACTER,      ///< Invalid character in input
    LEXICAL_UNTERMINATED_STRING,    ///< Unterminated string literal
    LEXICAL_INVALID_STRING_ESCAPE,  ///< Invalid escape sequence
    LEXICAL_INVALID_NUMBER_FORMAT,  ///< Invalid number format
    LEXICAL_IDENTIFIER_TOO_LONG,    ///< Identifier exceeds length limit

    // Syntax errors (语法错误)
    SYNTAX_UNEXPECTED_TOKEN,        ///< Unexpected token
    SYNTAX_MISSING_TOKEN,           ///< Missing required token
    SYNTAX_INVALID_SYNTAX,          ///< Invalid syntax structure
    SYNTAX_UNEXPECTED_EOF,          ///< Unexpected end of input

    // Semantic errors (语义错误)
    SEMANTIC_UNDEFINED_TABLE,       ///< Undefined table reference
    SEMANTIC_UNDEFINED_COLUMN,      ///< Undefined column reference
    SEMANTIC_UNDEFINED_FUNCTION,    ///< Undefined function call
    SEMANTIC_DUPLICATE_TABLE,       ///< Duplicate table name
    SEMANTIC_DUPLICATE_COLUMN,      ///< Duplicate column name
    SEMANTIC_TYPE_MISMATCH,         ///< Type mismatch
    SEMANTIC_INVALID_CONSTRAINT,    ///< Invalid constraint definition
    SEMANTIC_INVALID_JOIN_CONDITION,///< Invalid JOIN condition
    SEMANTIC_AMBIGUOUS_COLUMN,      ///< Ambiguous column reference
    SEMANTIC_DIVISION_BY_ZERO,      ///< Division by zero
    SEMANTIC_INVALID_AGGREGATE,     ///< Invalid aggregate function usage

    // Runtime errors (运行时错误)
    RUNTIME_MEMORY_ERROR,           ///< Memory allocation failure
    RUNTIME_STACK_OVERFLOW,         ///< Parser stack overflow
    RUNTIME_INTERNAL_ERROR          ///< Internal parser error
};

/**
 * @brief Error severity levels
 */
enum class Severity {
    INFO,       ///< Informational message
    WARNING,    ///< Warning (parsing continues)
    ERROR,      ///< Error (may continue with recovery)
    FATAL       ///< Fatal error (parsing stops)
};

/**
 * @brief Parse error class
 *
 * Represents a single error or warning encountered during parsing.
 */
class ParseError {
public:
    /**
     * @brief Constructor
     * @param type Error type
     * @param severity Error severity
     * @param message Error message
     * @param location Source location of the error
     */
    ParseError(ErrorType type, Severity severity,
               const std::string& message,
               const ast::SourceLocation& location = ast::SourceLocation());

    /**
     * @brief Destructor
     */
    ~ParseError() = default;

    /**
     * @brief Get error type
     */
    ErrorType getType() const;

    /**
     * @brief Get error severity
     */
    Severity getSeverity() const;

    /**
     * @brief Get error message
     */
    const std::string& getMessage() const;

    /**
     * @brief Get source location
     */
    const ast::SourceLocation& getLocation() const;

    /**
     * @brief Set error suggestion
     */
    void setSuggestion(const std::string& suggestion);

    /**
     * @brief Get error suggestion
     */
    const std::string& getSuggestion() const;

    /**
     * @brief Set error context
     */
    void setContext(const std::string& context);

    /**
     * @brief Get error context
     */
    const std::string& getContext() const;

    /**
     * @brief Convert to string representation
     */
    std::string toString() const;

    /**
     * @brief Convert to JSON representation
     */
    std::string toJson() const;

    /**
     * @brief Check if this is a fatal error
     */
    bool isFatal() const;

    /**
     * @brief Check if this is a warning
     */
    bool isWarning() const;

private:
    ErrorType type_;
    Severity severity_;
    std::string message_;
    ast::SourceLocation location_;
    std::string suggestion_;
    std::string context_;
};

/**
 * @brief Error collector class
 *
 * Collects and manages multiple parse errors and warnings.
 */
class ErrorCollector {
public:
    /**
     * @brief Constructor
     */
    ErrorCollector();

    /**
     * @brief Destructor
     */
    ~ErrorCollector() = default;

    /**
     * @brief Add an error
     */
    void addError(std::unique_ptr<ParseError> error);

    /**
     * @brief Add a warning
     */
    void addWarning(const std::string& message,
                   const ast::SourceLocation& location = ast::SourceLocation());

    /**
     * @brief Add an informational message
     */
    void addInfo(const std::string& message,
                const ast::SourceLocation& location = ast::SourceLocation());

    /**
     * @brief Check if there are any errors
     */
    bool hasErrors() const;

    /**
     * @brief Check if there are any fatal errors
     */
    bool hasFatalErrors() const;

    /**
     * @brief Check if there are any warnings
     */
    bool hasWarnings() const;

    /**
     * @brief Get total error count
     */
    size_t getErrorCount() const;

    /**
     * @brief Get total warning count
     */
    size_t getWarningCount() const;

    /**
     * @brief Get all errors
     */
    const std::vector<std::unique_ptr<ParseError>>& getErrors() const;

    /**
     * @brief Get all warnings
     */
    const std::vector<std::unique_ptr<ParseError>>& getWarnings() const;

    /**
     * @brief Get errors by type
     */
    std::vector<std::unique_ptr<ParseError>> getErrorsByType(ErrorType type) const;

    /**
     * @brief Get errors by severity
     */
    std::vector<std::unique_ptr<ParseError>> getErrorsBySeverity(Severity severity) const;

    /**
     * @brief Clear all errors and warnings
     */
    void clear();

    /**
     * @brief Clear only errors
     */
    void clearErrors();

    /**
     * @brief Clear only warnings
     */
    void clearWarnings();

    /**
     * @brief Convert to string representation
     */
    std::string toString() const;

    /**
     * @brief Convert to JSON representation
     */
    std::string toJson() const;

private:
    std::vector<std::unique_ptr<ParseError>> errors_;
    std::vector<std::unique_ptr<ParseError>> warnings_;
};

} // namespace errors
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_ERRORS_ERROR_CORE_H
