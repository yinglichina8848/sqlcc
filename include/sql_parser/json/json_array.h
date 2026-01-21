#pragma once

#include "json/json_value.h"
#include <vector>
#include <memory>

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonArray : public JsonValue {
public:
    JsonArray();

    size_t size() const { return elements_.size(); }
    bool empty() const { return elements_.empty(); }

    void push_back(std::unique_ptr<JsonValue> value);
    void pop_back();
    const JsonValue* operator[](size_t index) const;
    JsonValue* operator[](size_t index);

    std::string toString() const override;
    std::string toString(int indent) const override;
    std::unique_ptr<JsonValue> clone() const override;
    bool equals(const JsonValue& other) const override;

private:
    std::vector<std::unique_ptr<JsonValue>> elements_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
