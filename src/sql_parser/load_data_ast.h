#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../include/sql_parser/ast_node.h"

namespace sqlcc {
namespace sql_parser {

class LoadDataStatement : public Statement {
public:
    LoadDataStatement() : Statement(Type::LOAD_DATA) {}

    // LOAD DATA [LOW_PRIORITY | CONCURRENT]
    //     [LOCAL] INFILE 'file_name'
    //     [REPLACE | IGNORE]
    //     INTO TABLE tbl_name
    //     [PARTITION (partition_name [, partition_name] ...)]
    //     [CHARACTER SET charset_name]
    //     [{FIELDS | COLUMNS}
    //         [TERMINATED BY 'string']
    //         [[OPTIONALLY] ENCLOSED BY 'char']
    //         [ESCAPED BY 'char']
    //     ]
    //     [LINES
    //         [STARTING BY 'string']
    //         [TERMINATED BY 'string']
    //     ]
    //     [IGNORE number {LINES | ROWS}]
    //     [(col_name_or_user_var
    //         [, col_name_or_user_var] ...)]
    //     [SET col_name={expr | DEFAULT}
    //         [, col_name={expr | DEFAULT}] ...]

    // 基本信息
    std::string table_name;
    std::string file_name;
    bool is_local = false;

    // 选项
    bool low_priority = false;
    bool concurrent = false;
    std::string replace_or_ignore; // "REPLACE", "IGNORE", or ""

    // 分区
    std::vector<std::string> partitions;

    // 字符集
    std::string charset_name;

    // 字段选项
    std::string fields_terminated_by;
    std::string fields_enclosed_by;
    bool fields_optionally_enclosed = false;
    std::string fields_escaped_by;

    // 行选项
    std::string lines_starting_by;
    std::string lines_terminated_by;

    // 忽略行数
    int ignore_lines = 0;

    // 列映射和SET子句
    std::vector<std::string> column_list;
    std::vector<std::pair<std::string, std::string>> set_expressions;

    std::string to_string() const {
        std::string result = "LOAD DATA";
        if (low_priority) result += " LOW_PRIORITY";
        if (concurrent) result += " CONCURRENT";
        if (is_local) result += " LOCAL";
        result += " INFILE '" + file_name + "'";
        if (!replace_or_ignore.empty()) {
            result += " " + replace_or_ignore;
        }
        result += " INTO TABLE " + table_name;
        if (!partitions.empty()) {
            result += " PARTITION (";
            for (size_t i = 0; i < partitions.size(); ++i) {
                if (i > 0) result += ", ";
                result += partitions[i];
            }
            result += ")";
        }
        if (!charset_name.empty()) {
            result += " CHARACTER SET " + charset_name;
        }
        if (!fields_terminated_by.empty() || !fields_enclosed_by.empty() ||
            !fields_escaped_by.empty()) {
            result += " FIELDS";
            if (!fields_terminated_by.empty()) {
                result += " TERMINATED BY '" + fields_terminated_by + "'";
            }
            if (!fields_enclosed_by.empty()) {
                if (fields_optionally_enclosed) {
                    result += " OPTIONALLY";
                }
                result += " ENCLOSED BY '" + fields_enclosed_by + "'";
            }
            if (!fields_escaped_by.empty()) {
                result += " ESCAPED BY '" + fields_escaped_by + "'";
            }
        }
        if (!lines_starting_by.empty() || !lines_terminated_by.empty()) {
            result += " LINES";
            if (!lines_starting_by.empty()) {
                result += " STARTING BY '" + lines_starting_by + "'";
            }
            if (!lines_terminated_by.empty()) {
                result += " TERMINATED BY '" + lines_terminated_by + "'";
            }
        }
        if (ignore_lines > 0) {
            result += " IGNORE " + std::to_string(ignore_lines) + " LINES";
        }
        if (!column_list.empty()) {
            result += " (";
            for (size_t i = 0; i < column_list.size(); ++i) {
                if (i > 0) result += ", ";
                result += column_list[i];
            }
            result += ")";
        }
        if (!set_expressions.empty()) {
            result += " SET ";
            for (size_t i = 0; i < set_expressions.size(); ++i) {
                if (i > 0) result += ", ";
                result += set_expressions[i].first + " = " + set_expressions[i].second;
            }
        }
        return result;
    }
};

} // namespace sql_parser
} // namespace sqlcc
