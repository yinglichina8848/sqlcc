#ifndef SQLCC_SQL_PARSER_JSON_JSON_VALUE_H_
#define SQLCC_SQL_PARSER_JSON_JSON_VALUE_H_

namespace sqlcc {
namespace sql_parser {
namespace json {

class JsonValue {
public:
    enum Type {
        NULL_VALUE,
        BOOLEAN,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };

    explicit JsonValue(Type type);

private:
    Type type_;
};

} // namespace json
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_JSON_JSON_VALUE_H_
