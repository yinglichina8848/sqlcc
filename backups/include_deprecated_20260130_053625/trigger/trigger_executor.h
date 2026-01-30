/**
 * @file trigger_executor.h
 * @brief 触发器执行器接口定义
 *
 * Why: 需要定义触发器执行的统一接口
 * What: TriggerExecutor接口定义触发器执行的标准方法
 * How: 提供纯虚函数供具体实现类重写
 */

#pragma once

#include "trigger_definition.h"
#include <vector>
#include <string>

namespace sqlcc {
namespace trigger {

/**
 * @brief 行数据表示
 */
struct RowData {
    std::vector<std::string> columns;
    std::vector<std::string> values;

    RowData() = default;
    RowData(const std::vector<std::string>& cols, const std::vector<std::string>& vals)
        : columns(cols), values(vals) {}
};

/**
 * @brief 触发器执行器接口
 *
 * 定义触发器执行的标准接口，包括触发器执行和条件评估
 */
class TriggerExecutor {
public:
    /**
     * @brief 析构函数
     */
    virtual ~TriggerExecutor() = default;

    /**
     * @brief 执行触发器
     * @param trigger 触发器定义
     * @param old_row 旧行数据 (UPDATE/DELETE时有效)
     * @param new_row 新行数据 (INSERT/UPDATE时有效)
     * @return 执行结果
     */
    virtual bool executeTrigger(const TriggerDefinition* trigger,
                               const RowData* old_row,
                               const RowData* new_row) = 0;

    /**
     * @brief 检查触发条件
     * @param condition 条件表达式
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 条件是否满足
     */
    virtual bool evaluateCondition(const std::string& condition,
                                  const RowData* old_row,
                                  const RowData* new_row) = 0;
};

} // namespace trigger
} // namespace sqlcc
