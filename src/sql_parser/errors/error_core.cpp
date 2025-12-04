#include "../../../include/sql_parser/errors/error_core.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {
namespace sql_parser {
namespace errors {

ParseError::ParseError(ErrorType type, Severity severity,
                       const std::string& message,
                       const ast::SourceLocation& location)
    : type_(type), severity_(severity), message_(message), location_(location) {
}

ErrorType ParseError::getType() const {
    return type_;
}

Severity ParseError::getSeverity() const {
    return severity_;
}

const std::string& ParseError::getMessage() const {
    return message_;
}

const ast::SourceLocation& ParseError::getLocation() const {
    return location_;
}

void ParseError::setSuggestion(const std::string& suggestion) {
    suggestion_ = suggestion;
}

const std::string& ParseError::getSuggestion() const {
    return suggestion_;
}

void ParseError::setContext(const std::string& context) {
    context_ = context;
}

const std::string& ParseError::getContext() const {
    return context_;
}

std::string ParseError::toString() const {
    std::stringstream ss;

    // Add severity indicator
    switch (severity_) {
        case Severity::INFO:
            ss << "[INFO] ";
            break;
        case Severity::WARNING:
            ss << "[WARNING] ";
            break;
        case Severity::ERROR:
            ss << "[ERROR] ";
            break;
        case Severity::FATAL:
            ss << "[FATAL] ";
            break;
    }

    // Add location
    if (location_.isValid()) {
        ss << location_.toString() << ": ";
    }

    // Add message
    ss << message_;

    // Add suggestion if available
    if (!suggestion_.empty()) {
        ss << " (Suggestion: " << suggestion_ << ")";
    }

    return ss.str();
}

std::string ParseError::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"type\":\"" << static_cast<int>(type_) << "\",";
    ss << "\"severity\":\"" << static_cast<int>(severity_) << "\",";
    ss << "\"message\":\"" << message_ << "\",";
    ss << "\"location\":" << location_.toJson();

    if (!suggestion_.empty()) {
        ss << ",\"suggestion\":\"" << suggestion_ << "\"";
    }

    if (!context_.empty()) {
        ss << ",\"context\":\"" << context_ << "\"";
    }

    ss << "}";
    return ss.str();
}

bool ParseError::isFatal() const {
    return severity_ == Severity::FATAL;
}

bool ParseError::isWarning() const {
    return severity_ == Severity::WARNING;
}

// ErrorCollector implementation

ErrorCollector::ErrorCollector() = default;

void ErrorCollector::addError(std::unique_ptr<ParseError> error) {
    if (error->isWarning()) {
        warnings_.push_back(std::move(error));
    } else {
        errors_.push_back(std::move(error));
    }
}

void ErrorCollector::addWarning(const std::string& message,
                               const ast::SourceLocation& location) {
    auto warning = std::make_unique<ParseError>(
        ErrorType::RUNTIME_INTERNAL_ERROR, // Default type for warnings
        Severity::WARNING,
        message,
        location
    );
    warnings_.push_back(std::move(warning));
}

void ErrorCollector::addInfo(const std::string& message,
                            const ast::SourceLocation& location) {
    auto info = std::make_unique<ParseError>(
        ErrorType::RUNTIME_INTERNAL_ERROR, // Default type for info
        Severity::INFO,
        message,
        location
    );
    warnings_.push_back(std::move(info)); // Info messages stored with warnings
}

bool ErrorCollector::hasErrors() const {
    return !errors_.empty();
}

bool ErrorCollector::hasFatalErrors() const {
    return std::any_of(errors_.begin(), errors_.end(),
                      [](const std::unique_ptr<ParseError>& error) {
                          return error->isFatal();
                      });
}

bool ErrorCollector::hasWarnings() const {
    return !warnings_.empty();
}

size_t ErrorCollector::getErrorCount() const {
    return errors_.size();
}

size_t ErrorCollector::getWarningCount() const {
    return warnings_.size();
}

const std::vector<std::unique_ptr<ParseError>>& ErrorCollector::getErrors() const {
    return errors_;
}

const std::vector<std::unique_ptr<ParseError>>& ErrorCollector::getWarnings() const {
    return warnings_;
}

std::vector<std::unique_ptr<ParseError>> ErrorCollector::getErrorsByType(ErrorType type) const {
    std::vector<std::unique_ptr<ParseError>> result;

    for (const auto& error : errors_) {
        if (error->getType() == type) {
            // Create a copy for the result (this is a simplified implementation)
            // In a real implementation, you might want to return references or shared_ptr
            result.push_back(std::make_unique<ParseError>(
                error->getType(),
                error->getSeverity(),
                error->getMessage(),
                error->getLocation()
            ));
        }
    }

    return result;
}

std::vector<std::unique_ptr<ParseError>> ErrorCollector::getErrorsBySeverity(Severity severity) const {
    std::vector<std::unique_ptr<ParseError>> result;

    auto& source = (severity == Severity::WARNING || severity == Severity::INFO) ? warnings_ : errors_;

    for (const auto& error : source) {
        if (error->getSeverity() == severity) {
            result.push_back(std::make_unique<ParseError>(
                error->getType(),
                error->getSeverity(),
                error->getMessage(),
                error->getLocation()
            ));
        }
    }

    return result;
}

void ErrorCollector::clear() {
    errors_.clear();
    warnings_.clear();
}

void ErrorCollector::clearErrors() {
    errors_.clear();
}

void ErrorCollector::clearWarnings() {
    warnings_.clear();
}

std::string ErrorCollector::toString() const {
    std::stringstream ss;

    if (!errors_.empty()) {
        ss << "Errors:\n";
        for (const auto& error : errors_) {
            ss << "  " << error->toString() << "\n";
        }
    }

    if (!warnings_.empty()) {
        ss << "Warnings:\n";
        for (const auto& warning : warnings_) {
            ss << "  " << warning->toString() << "\n";
        }
    }

    if (errors_.empty() && warnings_.empty()) {
        ss << "No errors or warnings.";
    }

    return ss.str();
}

std::string ErrorCollector::toJson() const {
    std::stringstream ss;
    ss << "{";

    // Add errors array
    ss << "\"errors\":[";
    for (size_t i = 0; i < errors_.size(); ++i) {
        if (i > 0) ss << ",";
        ss << errors_[i]->toJson();
    }
    ss << "],";

    // Add warnings array
    ss << "\"warnings\":[";
    for (size_t i = 0; i < warnings_.size(); ++i) {
        if (i > 0) ss << ",";
        ss << warnings_[i]->toJson();
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

} // namespace errors
} // namespace sql_parser
} // namespace sqlcc
