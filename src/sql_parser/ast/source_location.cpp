#include "sql_parser/ast/core/source_location.h"
#include <algorithm>

namespace sqlcc {
namespace sql_parser {
namespace ast {

SourceLocation::SourceLocation(size_t line, size_t column,
                               size_t offset, const std::string& file)
    : line_(line), column_(column), offset_(offset), file_(file) {
}

bool SourceLocation::isValid() const {
    return line_ > 0 && column_ > 0;
}

SourceLocation SourceLocation::merge(const SourceLocation& other) const {
    // If either location is invalid, return the other valid one
    if (!isValid()) return other;
    if (!other.isValid()) return *this;

    // Create a merged location spanning both ranges
    SourceLocation merged;

    // Use the earlier file (if both have files)
    if (!file_.empty() && !other.file_.empty()) {
        merged.file_ = file_; // Use this file as primary
    } else if (!file_.empty()) {
        merged.file_ = file_;
    } else {
        merged.file_ = other.file_;
    }

    // Use the earlier line
    merged.line_ = std::min(line_, other.line_);

    // For the same line, use the earlier column
    if (line_ == other.line_) {
        merged.column_ = std::min(column_, other.column_);
    } else {
        // For different lines, use the column from the earlier line
        merged.column_ = (line_ < other.line_) ? column_ : other.column_;
    }

    // Use the earlier offset
    merged.offset_ = std::min(offset_, other.offset_);

    return merged;
}

std::string SourceLocation::toString() const {
    std::stringstream ss;
    if (!file_.empty()) {
        ss << file_ << ":";
    }
    ss << line_ << ":" << column_;
    return ss.str();
}

std::string SourceLocation::toJson() const {
    std::stringstream ss;
    ss << "{";
    if (!file_.empty()) {
        ss << "\"file\":\"" << file_ << "\",";
    }
    ss << "\"line\":" << line_
       << ",\"column\":" << column_
       << ",\"offset\":" << offset_
       << "}";
    return ss.str();
}

bool SourceLocation::operator==(const SourceLocation& other) const {
    return line_ == other.line_ &&
           column_ == other.column_ &&
           offset_ == other.offset_ &&
           file_ == other.file_;
}

bool SourceLocation::operator!=(const SourceLocation& other) const {
    return !(*this == other);
}

bool SourceLocation::operator<(const SourceLocation& other) const {
    // Compare by file first
    if (file_ != other.file_) {
        return file_ < other.file_;
    }
    // Then by line
    if (line_ != other.line_) {
        return line_ < other.line_;
    }
    // Then by column
    if (column_ != other.column_) {
        return column_ < other.column_;
    }
    // Finally by offset
    return offset_ < other.offset_;
}

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc
