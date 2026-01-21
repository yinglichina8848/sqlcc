#pragma once

#include "json/json_value.h"

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonNumber : public JsonValue {
public:
    JsonNumber(double value);

    double getValue() const { return value_; }

    std::string toString() const override;
    std::string toString(int indent) const override;
    std::unique_ptr<JsonValue> clone() const override;
    bool equals(const JsonValue& other) const override;

private:
    double value_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
