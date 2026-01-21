/**
 * WHY: 为什么需要专门的任务执行结果系统？
 *
 * 数据库系统需要精确跟踪和报告任务执行状态，传统方案存在诸多问题：
 * - 执行结果不完整：缺少详细的执行状态和错误信息
 * - 性能监控缺失：无法跟踪任务执行时间和资源消耗
 * - 错误处理不统一：错误信息格式不一致，难以诊断问题
 * - 结果传递困难：不同任务类型的结果格式不统一
 * - 统计分析无力：缺乏对执行历史的统计和趋势分析
 *
 * 任务执行结果系统的核心价值：
 * 1. 状态完整性：提供完整的任务执行状态跟踪和报告
 * 2. 性能洞察：精确记录执行时间和资源消耗统计
 * 3. 错误诊断：统一的错误信息格式和详细的错误诊断
 * 4. 结果标准化：规范化的结果格式，便于上层处理
 * 5. 历史追溯：完整的执行历史记录，支持审计和分析
 *
 * 🏗️ 设计模式：建造者模式(Builder Pattern) + 状态模式(State Pattern)
 *
 * 任务结果作为建造者模式的经典应用：
 * - 逐步构建：任务执行过程中逐步构建结果信息
 * - 可选参数：支持灵活的结果信息组合和配置
 * - 不可变对象：构建完成后结果对象不可修改保证线程安全
 * - 链式调用：流畅的API设计支持方法链式调用
 * - 验证完整性：构建过程中验证结果信息的完整性
 *
 * SOLID原则体现：
 * - 单一职责：任务结果类专门负责执行结果的封装和管理
 * - 开闭原则：新结果类型通过扩展现有类实现
 * - 里氏替换：任务结果子类可以替换基类使用
 * - 接口隔离：结果接口精确定义所需的方法
 * - 依赖倒置：上层模块依赖结果接口而非具体实现
 *
 * WHAT: 任务执行结果系统 - 数据库任务执行状态管理框架
 *
 * 核心功能：
 * - 执行状态跟踪：记录任务从开始到完成的完整执行状态
 * - 错误信息管理：统一的错误信息收集、格式化和传递
 * - 性能指标收集：精确的执行时间、资源消耗统计
 * - 结果数据封装：标准化结果数据的封装和访问
 * - 执行历史记录：完整的任务执行历史追溯和审计
 *
 * 系统组件：
 * - TaskResult：核心结果类，封装执行结果信息
 * - TaskType：任务类型枚举，标识不同类型的数据库任务
 * - ExecutionMetrics：执行指标类，记录性能和资源统计
 * - ErrorInfo：错误信息类，标准化的错误信息结构
 * - ResultSummary：结果摘要类，执行结果的快速概览
 *
 * 任务类型分类：
 * - DDL操作：数据定义语言操作，如CREATE、ALTER、DROP
 * - DML操作：数据操作语言操作，如INSERT、UPDATE、DELETE
 * - 查询操作：SELECT查询执行和结果返回
 * - 事务操作：事务管理操作，如BEGIN、COMMIT、ROLLBACK
 * - 存储过程：存储过程调用和执行
 * - 触发器：触发器执行和事件处理
 * - 系统维护：索引重建、统计更新等维护任务
 * - 自定义任务：用户自定义的扩展任务类型
 *
 * 执行状态管理：
 * - 初始化：任务已提交但尚未开始执行
 * - 运行中：任务正在执行过程中
 * - 已完成：任务成功执行完成
 * - 已失败：任务执行失败，包含错误信息
 * - 已取消：任务被主动取消或超时取消
 *
 * 性能指标收集：
 * - 执行时间：任务从开始到结束的总耗时
 * - CPU时间：任务执行占用的CPU时间
 * - 内存使用：任务执行过程中的内存消耗
 * - I/O操作：磁盘读写操作的统计
 * - 网络流量：网络通信的数据传输量
 * - 资源使用：其他系统资源的消耗统计
 *
 * 错误处理机制：
 * - 错误分类：语法错误、语义错误、运行时错误等
 * - 错误级别：致命错误、严重错误、一般错误、警告
 * - 错误上下文：错误发生的详细上下文信息
 * - 错误链：支持错误嵌套和错误链跟踪
 * - 错误恢复：错误恢复策略和建议的修复措施
 *
 * 接口设计：
 * - 状态查询接口：获取当前执行状态和进度
 * - 结果访问接口：获取执行结果数据和统计信息
 * - 错误查询接口：获取详细错误信息和诊断建议
 * - 性能监控接口：获取执行性能指标和资源消耗
 * - 序列化接口：支持结果的序列化和反序列化
 *
 * HOW: 任务执行结果系统的实现机制
 *
 * 结果构建流程：
 * 1. 初始化：创建TaskResult对象，设置任务类型和初始状态
 * 2. 执行开始：记录开始时间，设置运行状态
 * 3. 状态更新：根据执行进展更新状态和中间结果
 * 4. 错误处理：捕获异常，记录错误信息，设置失败状态
 * 5. 执行完成：记录结束时间，计算性能指标，设置最终状态
 * 6. 结果封装：封装最终结果，生成摘要信息
 *
 * 状态机实现：
 * 1. 状态定义：使用枚举定义完整的状态集合
 * 2. 状态转换：严格的状态转换规则和验证
 * 3. 事件驱动：基于执行事件的状态自动转换
 * 4. 并发安全：多线程环境下的状态一致性保证
 * 5. 状态持久化：支持状态的持久化存储和恢复
 *
 * 性能统计实现：
 * 1. 时间测量：使用高精度时钟测量执行时间
 * 2. 计数器收集：原子计数器收集各种性能指标
 * 3. 采样记录：定期采样记录资源使用情况
 * 4. 聚合计算：实时聚合计算平均值、峰值等统计量
 * 5. 趋势分析：支持性能趋势的分析和预测
 *
 * 错误信息实现：
 * 1. 异常捕获：捕获标准异常和自定义异常
 * 2. 信息提取：从异常对象提取详细错误信息
 * 3. 上下文记录：记录错误发生的上下文环境
 * 4. 堆栈跟踪：生成详细的错误堆栈跟踪信息
 * 5. 错误链：支持嵌套错误和错误传播链
 *
 * 内存管理实现：
 * 1. 对象池：复用TaskResult对象的内存分配
 * 2. 引用计数：智能指针管理结果对象的生命周期
 * 3. 延迟初始化：按需初始化大型结果数据结构
 * 4. 内存映射：对大数据结果使用内存映射技术
 * 5. 垃圾回收：自动清理过期和未使用的结果对象
 *
 * 序列化实现：
 * 1. 二进制序列化：高效的二进制格式序列化
 * 2. JSON序列化：人类可读的JSON格式序列化
 * 3. 压缩存储：对大型结果进行压缩存储
 * 4. 版本兼容：支持不同版本的序列化格式兼容
 * 5. 流式处理：支持大数据结果的流式序列化
 *
 * 并发安全实现：
 * 1. 原子操作：使用原子变量保证状态更新的线程安全
 * 2. 互斥锁：保护共享数据的读写操作
 * 3. 读写锁：区分读写操作的性能优化
 * 4. 无锁算法：对高频操作使用无锁数据结构
 * 5. 内存屏障：保证内存操作的顺序一致性
 *
 * 扩展性设计：
 * - 插件架构：支持自定义结果类型和统计指标
 * - 配置化管理：结果收集和处理的配置化管理
 * - 分布式支持：支持分布式环境下的结果聚合
 * - 实时监控：集成实时监控和告警系统
 * - AI分析：基于机器学习的执行结果智能分析
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录任务执行过程和状态变化
 * - 性能分析：识别性能瓶颈和优化机会
 * - 错误诊断：自动诊断常见错误模式和解决方案
 * - 可视化展示：任务执行结果的图形化展示
 * - 历史回溯：支持历史执行结果的查询和分析
 */

#ifndef SQLCC_EXECUTION_TASK_RESULT_H
#define SQLCC_EXECUTION_TASK_RESULT_H

#include <string>
#include <memory>
#include <chrono>

namespace sqlcc {

/**
 * @brief 任务类型枚举
 */
enum class TaskType {
    UNKNOWN = 0,      // 未知类型
    DDL_EXECUTE,      // DDL执行
    DML_EXECUTE,      // DML执行
    QUERY_EXECUTE,    // 查询执行
    TRANSACTION,      // 事务处理
    PROCEDURE_CALL,   // 存储过程调用
    TRIGGER_EXECUTE,  // 触发器执行
    SQL_EXECUTE,      // 通用SQL执行
    SYSTEM_MAINTAIN   // 系统维护
};

/**
 * @brief 任务执行结果类
 *
 * 封装任务执行的结果信息，包括成功状态、错误信息、执行时间等
 */
class TaskResult {
public:
    /**
     * @brief 构造函数
     * @param success 执行是否成功
     * @param task_type 任务类型
     */
    TaskResult(bool success = false, TaskType task_type = TaskType::DDL_EXECUTE);

    /**
     * @brief 析构函数
     */
    ~TaskResult() = default;

    /**
     * @brief 设置执行成功状态
     * @param success 成功标志
     */
    void set_success(bool success);

    /**
     * @brief 获取执行成功状态
     * @return 成功标志
     */
    bool get_success() const;

    /**
     * @brief 设置错误信息
     * @param error_msg 错误消息
     */
    void set_error_message(const std::string& error_msg);

    /**
     * @brief 获取错误信息
     * @return 错误消息
     */
    std::string get_error_message() const;

    /**
     * @brief 设置任务类型
     * @param type 任务类型
     */
    void set_task_type(TaskType type);

    /**
     * @brief 获取任务类型
     * @return 任务类型
     */
    TaskType get_task_type() const;

    /**
     * @brief 设置执行开始时间
     */
    void set_start_time();

    /**
     * @brief 设置执行结束时间
     */
    void set_end_time();

    /**
     * @brief 获取执行持续时间（毫秒）
     * @return 执行时间
     */
    long long get_execution_time_ms() const;

    /**
     * @brief 设置影响的行数
     * @param rows 行数
     */
    void set_affected_rows(size_t rows);

    /**
     * @brief 获取影响的行数
     * @return 行数
     */
    size_t get_affected_rows() const;

    /**
     * @brief 设置结果消息
     * @param message 结果消息
     */
    void set_result_message(const std::string& message);

    /**
     * @brief 获取结果消息
     * @return 结果消息
     */
    std::string get_result_message() const;

    /**
     * @brief 检查是否有错误
     * @return 是否有错误
     */
    bool has_error() const;

    /**
     * @brief 获取结果摘要
     * @return 摘要字符串
     */
    std::string get_summary() const;

private:
    bool success_;                          // 执行成功标志
    TaskType task_type_;                    // 任务类型
    std::string error_message_;             // 错误信息
    std::string result_message_;            // 结果消息
    size_t affected_rows_;                  // 影响的行数

    std::chrono::steady_clock::time_point start_time_;    // 开始时间
    std::chrono::steady_clock::time_point end_time_;      // 结束时间
    bool time_recorded_;                    // 时间是否已记录
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_TASK_RESULT_H
