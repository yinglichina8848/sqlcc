#pragma once

#include "sql_parser/json/json_value.h"

namespace sqlcc {
namespace sql_parser {
namespace json {

/**
 * @brief JSON null值类
 */
class JsonNull : public JsonValue {
public:
    JsonNull();

    // 转换为字符串
    std::string toString() const override;
    std::string toString(int indent) const override;

    // 克隆操作
    std::unique_ptr<JsonValue> clone() const override;

    // 比较操作
    bool equals(const JsonValue& other) const override;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
