#pragma once

#include "sql_parser/json/json_value.h"

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonBoolean : public JsonValue {
public:
    explicit JsonBoolean(bool value);

    bool getValue() const { return value_; }

    std::string toString() const override;
    std::string toString(int indent) const override;
    std::unique_ptr<JsonValue> clone() const override;
    bool equals(const JsonValue& other) const override;

private:
    bool value_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
