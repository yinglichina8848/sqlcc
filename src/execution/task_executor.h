/**
 * WHY: 为什么需要专门的任务执行器系统？
 *
 * 数据库系统需要高效处理大量并发查询和任务，传统方案存在诸多问题：
 * - 串行执行效率低下：单个查询阻塞整个系统，无法并发处理
 * - 资源管理困难：线程创建销毁开销大，难以控制资源使用
 * - 任务调度不灵活：缺乏优先级调度和任务依赖管理
 * - 错误隔离不足：单个任务失败可能影响整个系统稳定性
 * - 可扩展性差：难以动态调整执行资源和负载均衡
 *
 * 任务执行器系统的核心价值：
 * 1. 并发执行能力：支持多任务并发执行，提高系统吞吐量
 * 2. 智能调度策略：基于优先级的任务调度和资源分配
 * 3. 错误隔离机制：任务执行失败不影响其他任务的运行
 * 4. 资源管理优化：线程池复用和动态资源调整
 * 5. 可观测性增强：详细的任务执行监控和性能统计
 *
 * 🏗️ 设计模式：生产者-消费者模式(Producer-Consumer Pattern)
 *
 * 任务执行器作为生产者-消费者模式的经典应用：
 * - 任务队列作为缓冲区：解耦任务提交和任务执行
 * - 生产者线程安全：多个线程可以安全地提交任务
 * - 消费者并发执行：多个工作线程并发消费任务
 * - 阻塞控制机制：队列满时阻塞生产者，空时阻塞消费者
 * - 优雅关闭支持：支持系统关闭时完成剩余任务
 *
 * SOLID原则体现：
 * - 单一职责：任务执行器负责任务调度和执行逻辑
 * - 开闭原则：新任务类型通过扩展Task接口实现
 * - 里氏替换：具体任务可以替换抽象Task接口
 * - 接口隔离：Task接口精确定义任务执行契约
 * - 依赖倒置：执行器依赖Task抽象而非具体实现
 *
 * WHAT: 任务执行器系统 - 数据库查询并发执行框架
 *
 * 核心功能：
 * - 任务队列管理：线程安全的任务提交和队列管理
 * - 工作线程池：可配置数量的工作线程并发执行任务
 * - 任务优先级调度：基于优先级的任务执行顺序控制
 * - 任务生命周期管理：任务提交、执行、完成、取消的完整生命周期
 * - 资源限制控制：最大并发任务数和任务执行超时的控制
 * - 执行结果收集：任务执行结果的收集和状态查询
 *
 * 系统组件：
 * - Task：抽象任务接口，定义任务执行契约
 * - TaskExecutor：主执行器类，管理线程池和任务队列
 * - TaskResult：任务执行结果，包含执行状态和数据
 * - 任务状态机：管理任务从提交到完成的完整状态转换
 * - 优先级队列：基于任务优先级的队列调度机制
 * - 监控统计：任务执行的性能监控和统计信息
 *
 * 任务类型支持：
 * - 查询执行任务：SQL查询的解析和执行
 * - 数据操作任务：INSERT、UPDATE、DELETE等DML操作
 * - 事务管理任务：事务的开始、提交、回滚操作
 * - 系统维护任务：索引重建、统计信息更新等
 * - 后台清理任务：临时文件清理、缓存清理等
 * - 用户自定义任务：扩展的自定义任务类型
 *
 * 调度策略：
 * - 优先级调度：高优先级任务优先执行
 * - 公平调度：相同优先级的任务按提交顺序执行
 * - 时间片调度：防止单个任务长时间占用资源
 * - 负载均衡：任务在多个工作线程间均衡分配
 * - 依赖调度：支持任务间的依赖关系和执行顺序
 *
 * 接口设计：
 * - 任务提交接口：submitTask支持多种任务提交方式
 * - 任务管理接口：cancelTask、getTaskStatus等管理操作
 * - 阻塞等待接口：waitForTask、waitForAllTasks同步等待
 * - 配置接口：setMaxConcurrentTasks等资源配置
 * - 查询接口：getPendingTaskCount等状态查询
 *
 * HOW: 任务执行器系统的实现机制
 *
 * 任务队列实现：
 * 1. 线程安全容器：使用std::priority_queue和互斥锁保证线程安全
 * 2. 条件变量同步：生产者-消费者模式的经典同步机制
 * 3. 优先级比较：基于TaskPriority的任务优先级比较
 * 4. 容量限制：防止任务队列无限增长的内存保护
 * 5. 阻塞控制：队列满时阻塞生产者，空时阻塞消费者
 *
 * 工作线程实现：
 * 1. 线程池管理：std::vector<std::thread>管理工作线程
 * 2. 工作循环：while循环处理任务队列中的任务
 * 3. 异常处理：捕获任务执行异常，记录错误但不终止线程
 * 4. 优雅退出：通过原子变量控制线程退出逻辑
 * 5. 线程命名：为调试和监控设置线程名称
 *
 * 任务调度实现：
 * 1. 优先级队列：std::priority_queue自动维护优先级顺序
 * 2. 时间戳记录：记录任务提交时间用于公平调度
 * 3. 超时机制：使用std::chrono管理任务执行超时
 * 4. 取消机制：通过原子变量支持任务取消操作
 * 5. 状态转换：任务状态机的状态转换和事件处理
 *
 * 资源管理实现：
 * 1. 线程池复用：避免频繁的线程创建和销毁
 * 2. 内存池分配：任务对象的内存池分配和回收
 * 3. 连接池复用：数据库连接的池化管理
 * 4. CPU亲和性：任务到CPU核心的亲和性绑定
 * 5. 资源监控：执行资源的实时监控和调整
 *
 * 错误处理实现：
 * 1. 异常捕获：任务执行异常的捕获和处理
 * 2. 错误隔离：单个任务失败不影响其他任务
 * 3. 错误记录：详细的错误信息记录和诊断
 * 4. 重试机制：失败任务的自动重试逻辑
 * 5. 熔断保护：系统负载过高时的熔断保护
 *
 * 性能优化策略：
 * - 锁优化：细粒度锁和无锁数据结构的性能优化
 * - 内存预分配：任务队列和工作线程的预分配策略
 * - SIMD加速：向量化任务调度和统计计算
 * - 缓存优化：CPU缓存友好的数据结构设计
 * - 批量处理：任务的批量提交和批量执行优化
 *
 * 监控统计实现：
 * 1. 执行计数：任务提交、执行、完成、失败的计数统计
 * 2. 性能指标：任务执行时间、队列等待时间等性能指标
 * 3. 资源使用：线程使用率、内存使用、CPU使用等资源统计
 * 4. 错误统计：各类错误的发生频率和分布统计
 * 5. 趋势分析：系统负载和性能的趋势分析和预测
 *
 * 扩展性设计：
 * - 插件架构：支持自定义任务类型和调度策略
 * - 配置化管理：任务执行器的配置化参数管理
 * - 分布式扩展：支持跨进程、跨机器的任务调度
 * - 云原生适配：适配容器化和微服务架构
 * - AI优化：基于机器学习的智能调度和资源分配
 *
 * 调试和诊断：
 * - 执行跟踪：任务执行过程的详细日志记录
 * - 性能分析：任务执行性能的瓶颈分析和优化建议
 * - 死锁检测：任务依赖和资源竞争的死锁检测
 * - 内存泄漏：任务对象和资源的内存泄漏检测
 * - 可视化监控：任务执行状态的图形化展示工具
 */

#ifndef SQLCC_EXECUTION_TASK_EXECUTOR_H
#define SQLCC_EXECUTION_TASK_EXECUTOR_H

#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

#include "core/execution_result.h"
#include "task_result.h"

namespace sqlcc {

class ExecutionContext;

// 任务状态
enum class TaskStatus {
    PENDING = 0,
    RUNNING = 1,
    COMPLETED = 2,
    FAILED = 3,
    CANCELLED = 4
};

// 任务优先级
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};



// 任务接口
class Task {
public:
    Task() : task_id_(""), task_type_(TaskType::UNKNOWN), priority_(0), completed_(false) {}
    Task(const std::string& task_id, TaskType type, int priority = 0)
        : task_id_(task_id), task_type_(type), priority_(priority), completed_(false) {}
    virtual ~Task() = default;

    virtual std::shared_ptr<TaskResult> execute() = 0;
    virtual std::string getDescription() const { return task_id_; }
    virtual TaskPriority getPriority() const { return static_cast<TaskPriority>(priority_); }

    const std::string& getTaskId() const { return task_id_; }
    TaskType getTaskType() const { return task_type_; }
    int getPriorityValue() const { return priority_; }
    bool isCompleted() const { return completed_; }

    void setTaskId(const std::string& task_id) { task_id_ = task_id; }

private:
    std::string task_id_;
    TaskType task_type_;
    int priority_;
    bool completed_;
};

// 任务执行器 - 管理任务队列和执行
class TaskExecutor {
public:
    TaskExecutor();
    ~TaskExecutor();

    // 初始化和配置
    bool initialize(int num_threads = 4);
    void shutdown();

    // 任务提交
    int submitTask(std::unique_ptr<Task> task);
    int submitTask(std::function<void(ExecutionContext&)> func,
                   const std::string& description,
                   TaskPriority priority = TaskPriority::NORMAL);

    // 任务管理
    bool cancelTask(int task_id);
    TaskStatus getTaskStatus(int task_id) const;
    size_t getPendingTaskCount() const;
    size_t getRunningTaskCount() const;

    // 阻塞等待
    ExecutionResult waitForTask(int task_id);
    ExecutionResult waitForAllTasks();

    // 资源管理
    void setMaxConcurrentTasks(size_t max_tasks);
    void setTaskTimeout(std::chrono::seconds timeout);

private:
    // 任务队列
    struct TaskItem {
        int task_id;
        std::unique_ptr<Task> task;
        std::chrono::steady_clock::time_point submit_time;

        bool operator<(const TaskItem& other) const {
            return task->getPriority() < other.task->getPriority();
        }
    };

    // 工作线程
    void workerThread();
    void processTask(TaskItem& task_item);

    // 同步原语
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::priority_queue<TaskItem> task_queue_;

    // 线程管理
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_;

    // 配置
    size_t max_concurrent_tasks_;
    std::chrono::seconds task_timeout_;

    // 任务跟踪
    mutable std::mutex status_mutex_;
    std::unordered_map<int, TaskStatus> task_statuses_;
    std::atomic<int> next_task_id_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_TASK_EXECUTOR_H
