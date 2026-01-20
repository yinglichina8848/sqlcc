#include "trigger/trigger_executor.h"

#include <iostream>
#include <algorithm>
#include <regex>

namespace sqlcc {
namespace trigger {

// 默认触发器执行器实现
class DefaultTriggerExecutor : public TriggerExecutor {
public:
    bool executeTrigger(const TriggerDefinition* trigger,
                       const RowData* old_row,
                       const RowData* new_row) override {
        if (!trigger) {
            return false;
        }

        std::cout << "Executing trigger: " << trigger->getName()
                  << " on table: " << trigger->getTableName() << std::endl;

        // 检查触发条件
        if (!trigger->getCondition().empty()) {
            if (!evaluateCondition(trigger->getCondition(), old_row, new_row)) {
                std::cout << "Trigger condition not met, skipping execution" << std::endl;
                return true; // 条件不满足但不算执行失败
            }
        }

        // 执行触发器主体（这里简化实现，实际应该解析并执行SQL语句）
        std::cout << "Trigger body: " << trigger->getBody() << std::endl;

        // 模拟执行结果
        return true;
    }

    bool evaluateCondition(const std::string& condition,
                          const RowData* old_row,
                          const RowData* new_row) override {
        if (condition.empty()) {
            return true;
        }

        // 简化的条件评估（实际应该有完整的表达式解析器）
        // 这里只处理简单的等于比较

        try {
            // 查找变量引用 (OLD.column_name 或 NEW.column_name)
            std::regex old_pattern(R"(\bOLD\.(\w+)\b)");
            std::regex new_pattern(R"(\bNEW\.(\w+)\b)");

            std::string processed_condition = condition;

            // 替换OLD变量
            if (old_row) {
                std::string result;
                std::sregex_iterator iter(processed_condition.begin(), processed_condition.end(), old_pattern);
                std::sregex_iterator end;

                size_t last_pos = 0;
                for (; iter != end; ++iter) {
                    result += processed_condition.substr(last_pos, iter->position() - last_pos);

                    std::string column = (*iter)[1].str();
                    auto col_it = std::find(old_row->columns.begin(), old_row->columns.end(), column);
                    if (col_it != old_row->columns.end()) {
                        size_t index = std::distance(old_row->columns.begin(), col_it);
                        if (index < old_row->values.size()) {
                            result += "'" + old_row->values[index] + "'";
                        } else {
                            result += "'NULL'";
                        }
                    } else {
                        result += "'NULL'";
                    }

                    last_pos = iter->position() + iter->length();
                }
                result += processed_condition.substr(last_pos);
                processed_condition = result;
            }

            // 替换NEW变量
            if (new_row) {
                std::string result;
                std::sregex_iterator iter(processed_condition.begin(), processed_condition.end(), new_pattern);
                std::sregex_iterator end;

                size_t last_pos = 0;
                for (; iter != end; ++iter) {
                    result += processed_condition.substr(last_pos, iter->position() - last_pos);

                    std::string column = (*iter)[1].str();
                    auto col_it = std::find(new_row->columns.begin(), new_row->columns.end(), column);
                    if (col_it != new_row->columns.end()) {
                        size_t index = std::distance(new_row->columns.begin(), col_it);
                        if (index < new_row->values.size()) {
                            result += "'" + new_row->values[index] + "'";
                        } else {
                            result += "'NULL'";
                        }
                    } else {
                        result += "'NULL'";
                    }

                    last_pos = iter->position() + iter->length();
                }
                result += processed_condition.substr(last_pos);
                processed_condition = result;
            }

            // 简化的布尔表达式评估
            // 这里只处理最简单的比较
            if (processed_condition.find("!=") != std::string::npos) {
                // 处理不等于
                size_t pos = processed_condition.find("!=");
                std::string left = processed_condition.substr(0, pos);
                std::string right = processed_condition.substr(pos + 2);

                // 去除空白字符
                left.erase(std::remove_if(left.begin(), left.end(), ::isspace), left.end());
                right.erase(std::remove_if(right.begin(), right.end(), ::isspace), right.end());

                // 去除引号
                if (!left.empty() && left.front() == '\'' && left.back() == '\'') {
                    left = left.substr(1, left.size() - 2);
                }
                if (!right.empty() && right.front() == '\'' && right.back() == '\'') {
                    right = right.substr(1, right.size() - 2);
                }

                return left != right;
            } else if (processed_condition.find("=") != std::string::npos) {
                // 处理等于
                size_t pos = processed_condition.find("=");
                std::string left = processed_condition.substr(0, pos);
                std::string right = processed_condition.substr(pos + 1);

                // 去除空白字符
                left.erase(std::remove_if(left.begin(), left.end(), ::isspace), left.end());
                right.erase(std::remove_if(right.begin(), right.end(), ::isspace), right.end());

                // 去除引号
                if (!left.empty() && left.front() == '\'' && left.back() == '\'') {
                    left = left.substr(1, left.size() - 2);
                }
                if (!right.empty() && right.front() == '\'' && right.back() == '\'') {
                    right = right.substr(1, right.size() - 2);
                }

                return left == right;
            }

            // 如果无法解析，默认返回true
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error evaluating condition: " << e.what() << std::endl;
            return false;
        }
    }
};

// 全局触发器执行器实例
static DefaultTriggerExecutor default_executor;

TriggerExecutor* getDefaultTriggerExecutor() {
    return &default_executor;
}

} // namespace trigger
} // namespace sqlcc
