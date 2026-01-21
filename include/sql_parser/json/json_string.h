#pragma once

#include "json/json_value.h"
#include <string>

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonString : public JsonValue {
public:
    JsonString(const std::string& value);
    JsonString(std::string&& value);

    const std::string& getValue() const { return value_; }

    std::string toString() const override;
    std::string toString(int indent) const override;
    std::unique_ptr<JsonValue> clone() const override;
    bool equals(const JsonValue& other) const override;

private:
    std::string value_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
