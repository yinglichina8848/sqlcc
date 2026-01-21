#pragma once

#include "json/json_value.h"
#include <unordered_map>
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonObject : public JsonValue {
public:
    JsonObject();

    size_t size() const { return properties_.size(); }
    bool empty() const { return properties_.empty(); }

    bool contains(const std::string& key) const;
    void insert(const std::string& key, std::unique_ptr<JsonValue> value);
    void erase(const std::string& key);
    const JsonValue* operator[](const std::string& key) const;
    JsonValue* operator[](const std::string& key);

    std::vector<std::string> keys() const;

    std::string toString() const override;
    std::string toString(int indent) const override;
    std::unique_ptr<JsonValue> clone() const override;
    bool equals(const JsonValue& other) const override;

private:
    std::unordered_map<std::string, std::unique_ptr<JsonValue>> properties_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
