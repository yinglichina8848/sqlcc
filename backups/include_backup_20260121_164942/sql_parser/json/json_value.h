#pragma once

#include <string>
#include <memory>

namespace sqlcc {
namespace sql_parser {
namespace json {

/**
 * @brief JSON值基类 - 所有JSON类型的基类
 *
 * 使用多态设计，支持不同的JSON数据类型
 */
class JsonValue {
public:
    // JSON类型枚举
    enum class Type {
        NULL_VALUE,
        BOOLEAN,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };

    JsonValue(Type type);
    virtual ~JsonValue() = default;

    // 获取值类型
    Type getType() const { return type_; }

    // 转换为字符串表示
    virtual std::string toString() const = 0;
    virtual std::string toString(int indent) const = 0;

    // 克隆操作
    virtual std::unique_ptr<JsonValue> clone() const = 0;

    // 比较操作
    virtual bool equals(const JsonValue& other) const = 0;

protected:
    Type type_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc
