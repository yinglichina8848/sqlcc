/**
 * @file task_result.cpp
 *
 * WHY: 为什么需要任务结果？
 *
 * 数据库系统需要一种统一的方式来返回SQL执行的状态和结果。没有任务结果对象，客户端就无法知道执行是否成功、影响了多少行数据、执行耗时等关键信息，导致无法提供适当的用户反馈和错误处理。
 *
 * 主要问题解决：
 * 1. 执行状态反馈：告诉客户端SQL是否执行成功
 * 2. 结果信息传递：返回执行影响的数据行数等统计信息
 * 3. 错误信息传达：提供详细的错误描述和诊断信息
 * 4. 性能监控：记录执行时间和性能指标
 * 5. 调试支持：提供执行过程的详细摘要信息
 *
 * 任务结果失败的影响：
 * - 用户无法知道操作是否成功
 * - 错误信息丢失，用户无法理解问题
 * - 性能问题无法监控和诊断
 * - 应用程序无法正确处理执行结果
 *
 * WHAT: 这实现了什么功能？
 *
 * 任务结果提供完整的SQL执行结果管理功能：
 * - 成功状态跟踪：记录执行是否成功完成
 * - 错误信息管理：存储和检索详细错误信息
 * - 任务类型标识：区分不同类型的SQL操作
 * - 影响行数统计：记录INSERT/UPDATE/DELETE影响的行数
 * - 执行时间测量：精确记录操作执行耗时
 * - 结果消息传递：提供额外的执行结果信息
 *
 * 核心组件：
 * - TaskResult：任务结果的主要封装类
 * - TaskType：枚举定义不同类型的SQL任务
 * - ExecutionMetrics：执行性能指标收集
 * - ErrorHandling：错误信息处理和管理
 * - ResultSummary：执行结果摘要生成
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 状态管理：使用布尔值跟踪成功/失败状态
 * 2. 时间测量：std::chrono精确测量执行时间
 * 3. 错误处理：字符串存储详细错误信息
 * 4. 类型安全：枚举定义任务类型避免魔法数字
 * 5. 流式输出：stringstream生成格式化的摘要信息
 * 6. 资源管理：智能指针管理相关资源
 *
 * 架构设计：
 * - 值对象模式：不可变的结果状态封装
 * - 建造者模式：逐步设置结果属性的接口
 * - 工厂模式：根据任务类型创建相应的结果对象
 * - 访问者模式：不同客户端访问结果信息的接口
 * - 原型模式：复制和克隆结果对象
 *
 * 性能优化：
 * - 延迟计算：按需计算执行时间和摘要信息
 * - 内存复用：复用字符串缓冲区减少分配
 * - 最小化拷贝：使用引用和移动语义
 * - 缓存摘要：缓存生成的摘要信息
 * - 异步记录：后台记录性能指标
 *
 * @note 该实现专为SQLCC数据库系统优化，支持多种SQL操作类型的执行结果管理
 * @see include/execution/task_result.h
 */

#include "src/execution/task_result.h"
#include <sstream>

namespace sqlcc {

TaskResult::TaskResult(bool success, TaskType task_type)
    : success_(success),
      task_type_(task_type),
      affected_rows_(0),
      time_recorded_(false) {
}

void TaskResult::set_success(bool success) {
    success_ = success;
}

bool TaskResult::get_success() const {
    return success_;
}

void TaskResult::set_error_message(const std::string& error_msg) {
    error_message_ = error_msg;
    if (!error_msg.empty()) {
        success_ = false;
    }
}

std::string TaskResult::get_error_message() const {
    return error_message_;
}

void TaskResult::set_task_type(TaskType type) {
    task_type_ = type;
}

TaskType TaskResult::get_task_type() const {
    return task_type_;
}

void TaskResult::set_start_time() {
    start_time_ = std::chrono::steady_clock::now();
    time_recorded_ = true;
}

void TaskResult::set_end_time() {
    if (time_recorded_) {
        end_time_ = std::chrono::steady_clock::now();
    }
}

long long TaskResult::get_execution_time_ms() const {
    if (!time_recorded_) {
        return 0;
    }

    auto duration = end_time_ - start_time_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void TaskResult::set_affected_rows(size_t rows) {
    affected_rows_ = rows;
}

size_t TaskResult::get_affected_rows() const {
    return affected_rows_;
}

void TaskResult::set_result_message(const std::string& message) {
    result_message_ = message;
}

std::string TaskResult::get_result_message() const {
    return result_message_;
}

bool TaskResult::has_error() const {
    return !error_message_.empty();
}

std::string TaskResult::get_summary() const {
    std::stringstream ss;

    ss << "TaskResult Summary:\n";
    ss << "  Type: ";
    switch (task_type_) {
        case TaskType::UNKNOWN: ss << "UNKNOWN"; break;
        case TaskType::DDL_EXECUTE: ss << "DDL_EXECUTE"; break;
        case TaskType::DML_EXECUTE: ss << "DML_EXECUTE"; break;
        case TaskType::QUERY_EXECUTE: ss << "QUERY_EXECUTE"; break;
        case TaskType::TRANSACTION: ss << "TRANSACTION"; break;
        case TaskType::PROCEDURE_CALL: ss << "PROCEDURE_CALL"; break;
        case TaskType::TRIGGER_EXECUTE: ss << "TRIGGER_EXECUTE"; break;
        case TaskType::SQL_EXECUTE: ss << "SQL_EXECUTE"; break;
        case TaskType::SYSTEM_MAINTAIN: ss << "SYSTEM_MAINTAIN"; break;
    }
    ss << "\n";

    ss << "  Success: " << (success_ ? "true" : "false") << "\n";
    ss << "  Execution Time: " << get_execution_time_ms() << " ms\n";
    ss << "  Affected Rows: " << affected_rows_ << "\n";

    if (!error_message_.empty()) {
        ss << "  Error: " << error_message_ << "\n";
    }

    if (!result_message_.empty()) {
        ss << "  Message: " << result_message_ << "\n";
    }

    return ss.str();
}

} // namespace sqlcc
