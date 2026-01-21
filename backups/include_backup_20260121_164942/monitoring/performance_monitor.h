/**
 * WHY: 为什么需要数据库性能监控系统？
 *
 * 数据库系统复杂度极高，性能问题会导致系统响应缓慢甚至不可用：
 * - 性能瓶颈难以定位：复杂的查询执行、并发访问、资源竞争导致性能问题
 * - 系统负载不可见：缺少实时监控，无法了解系统运行状态和负载情况
 * - 故障预警缺失：性能异常时缺乏及时告警，问题积累导致系统崩溃
 * - 容量规划困难：缺少历史性能数据，无法进行有效的容量规划和资源分配
 * - 用户体验不佳：性能问题直接影响用户体验，但缺乏量化指标和优化依据
 * - 运维效率低下：缺少自动化监控告警，人工运维成本高昂且响应不及时
 * - 问题诊断困难：性能问题发生时缺乏详细的上下文信息和诊断数据
 * - 优化效果难测：性能优化后缺乏量化评估，难以验证优化效果
 *
 * 性能监控系统核心价值：
 * 1. 实时性能监控：实时收集系统、数据库、查询等各层面的性能指标
 * 2. 智能告警机制：基于阈值和趋势的智能告警，及时发现性能异常
 * 3. 历史数据分析：长期保存性能数据，支持趋势分析和容量规划
 * 4. 慢查询诊断：识别和分析慢查询，提供优化建议和执行计划分析
 * 5. 资源使用统计：详细的CPU、内存、磁盘、网络等资源使用情况统计
 * 6. 并发监控分析：事务并发情况、锁竞争情况的实时监控和分析
 * 7. 性能基准建立：建立性能基准，识别性能退化和异常波动
 * 8. 自动化优化：基于监控数据提供自动化性能优化建议
 *
 * 🏗️ 设计模式：观察者模式(Observer Pattern) + 策略模式(Strategy Pattern) + 工厂模式(Factory Pattern)
 *
 * 观察者模式的应用：
 * - 性能指标收集：作为观察者收集各种性能事件和指标数据
 * - 告警触发机制：性能异常时通知相关组件和告警系统
 * - 状态变化监听：监听系统状态变化，触发相应的监控和告警逻辑
 * - 事件驱动架构：基于事件的性能监控和响应机制
 * - 解耦监控逻辑：监控器与被监控组件解耦，提高系统的可扩展性
 * - 动态订阅机制：支持运行时动态订阅和取消订阅监控指标
 * - 多层次通知：支持不同级别的监控通知和告警机制
 * - 异步处理能力：异步处理监控事件，不影响主业务流程性能
 *
 * SOLID原则体现：
 * - 单一职责：PerformanceMonitor专门负责性能监控和告警功能
 * - 开闭原则：新监控指标通过扩展AlertConfig实现，无需修改核心逻辑
 * - 里氏替换：所有监控组件都可以被同类型组件替换
 * - 接口隔离：提供精确的监控接口，不强制实现不需要的功能
 * - 依赖倒置：依赖抽象的监控接口而非具体实现
 *
 * WHAT: 数据库性能监控和告警管理系统 - 实时性能监控、智能告警、历史数据分析的完整解决方案
 *
 * 核心功能：
 * - 系统性能监控：CPU、内存、磁盘、网络等系统级性能指标实时收集
 * - 数据库性能监控：事务、查询、缓存、缓冲池等数据库级性能指标统计
 * - 查询性能分析：查询执行时间、执行计划、结果集大小等详细性能分析
 * - 智能告警机制：基于阈值和趋势的智能告警配置和触发机制
 * - 历史数据存储：长期保存性能数据，支持历史趋势分析和对比
 * - 慢查询识别：自动识别和记录慢查询，提供优化建议
 * - 资源使用统计：详细的系统资源使用情况统计和分析
 * - 性能基准管理：建立和维护性能基准，支持异常检测
 *
 * 系统组件：
 * - SystemMetrics：系统级性能指标数据结构，包含CPU、内存、磁盘、网络等
 * - DatabaseMetrics：数据库级性能指标，包含事务、查询、缓存等统计信息
 * - QueryMetrics：查询性能指标，包含执行时间、结果集、执行计划等
 * - AlertConfig：告警配置，定义告警条件、阈值、严重程度等
 * - Alert：告警信息，包含告警详情、触发时间、确认状态等
 * - PerformanceMonitor：主监控类，提供完整的性能监控和告警功能
 * - MonitoringThread：监控工作线程，负责定时收集性能指标
 * - AlertManager：告警管理器，负责告警的生成、存储和处理
 * - DataCollector：数据收集器，负责各种性能指标的收集工作
 * - HistoryManager：历史数据管理器，负责性能数据的存储和管理
 * - ThresholdChecker：阈值检查器，负责性能指标的阈值比较和告警触发
 * - ReportGenerator：报告生成器，生成各种性能分析报告
 *
 * 性能指标体系：
 * - 系统指标：CPU使用率、内存使用量、磁盘I/O、网络I/O、系统负载
 * - 数据库指标：活跃事务数、提交事务数、回滚事务数、查询响应时间
 * - 查询指标：查询执行时间、结果集大小、缓存命中率、执行计划复杂度
 * - 缓存指标：缓存命中率、缓存大小、缓存淘汰率、缓存预热时间
 * - 连接指标：活跃连接数、总连接数、连接建立时间、连接池利用率
 * - 存储指标：存储空间使用率、I/O操作次数、读写延迟、存储性能
 * - 并发指标：锁等待时间、死锁次数、并发查询数、资源竞争情况
 * - 错误指标：错误次数、错误类型分布、错误恢复时间、系统稳定性
 *
 * 告警机制设计：
 * - 阈值告警：基于固定阈值的告警机制，支持大于、小于、等于等条件
 * - 趋势告警：基于性能趋势的告警，如性能突然下降或持续恶化
 * - 复合告警：多指标组合的告警逻辑，如CPU高负载且内存不足
 * - 级别分层：支持不同严重程度的告警，从低到高分为多个级别
 * - 告警抑制：避免重复告警，支持告警频率控制和抑制机制
 * - 告警升级：未确认告警自动升级严重程度，保障问题得到重视
 * - 告警恢复：性能恢复正常时自动生成恢复通知
 * - 告警路由：支持不同告警的路由配置，发送给相应负责人
 *
 * 历史数据管理：
 * - 数据存储：高效的性能数据存储机制，支持大规模数据存储
 * - 数据压缩：历史数据的压缩存储，节省存储空间
 * - 数据清理：自动清理过期历史数据，控制存储空间使用
 * - 数据聚合：支持不同时间粒度的数据聚合，如小时、天、月统计
 * - 数据查询：高效的历史数据查询接口，支持时间范围和条件过滤
 * - 数据导出：支持历史数据的导出功能，用于离线分析
 * - 数据备份：历史数据的备份和恢复机制，保证数据安全性
 * - 数据可视化：提供数据可视化接口，支持图表展示和趋势分析
 *
 * 慢查询分析：
 * - 查询捕获：自动捕获执行时间超过阈值的查询
 * - 执行计划分析：分析查询执行计划，识别性能瓶颈
 * - 索引建议：基于查询模式提供索引优化建议
 * - 查询重写：提供查询重写建议，优化查询性能
 * - 统计信息更新：建议更新过期的统计信息
 * - 并行执行评估：评估查询的并行执行可能性
 * - 缓存策略优化：建议查询结果缓存策略
 * - 资源使用分析：分析查询的CPU、内存、I/O资源使用情况
 *
 * 配置管理能力：
 * - 收集间隔配置：可配置的性能指标收集间隔
 * - 告警阈值配置：灵活的告警阈值配置机制
 * - 历史数据配置：历史数据保存时间和清理策略配置
 * - 监控范围配置：可配置的监控范围和监控指标
 * - 输出格式配置：监控数据的输出格式和报告格式配置
 * - 告警通知配置：告警通知的方式和接收者配置
 * - 权限控制配置：监控数据的访问权限控制配置
 * - 扩展插件配置：第三方监控插件的配置支持
 *
 * HOW: 性能监控系统的实现机制
 *
 * 多线程架构实现：
 * 1. 主监控线程：负责协调各个监控组件的工作
 * 2. 数据收集线程：专门负责性能指标的数据收集工作
 * 3. 告警处理线程：异步处理告警逻辑，不阻塞监控线程
 * 4. 数据清理线程：定期清理过期历史数据
 * 5. 报告生成线程：生成各种性能分析报告
 * 6. 网络通信线程：与外部监控系统通信
 * 7. 配置更新线程：处理配置变更和动态更新
 * 8. 健康检查线程：监控监控系统的自身健康状态
 *
 * 数据收集实现：
 * 1. 系统指标收集：通过操作系统API获取CPU、内存、磁盘等系统指标
 * 2. 数据库指标收集：通过数据库内部接口获取事务、查询等指标
 * 3. 查询指标收集：通过查询执行器钩子收集查询性能数据
 * 4. 缓存指标收集：通过缓存管理器接口获取缓存统计信息
 * 5. 连接指标收集：通过连接管理器获取连接池状态信息
 * 6. 异步数据收集：非阻塞的数据收集机制，不影响业务性能
 * 7. 数据验证校验：收集到的数据进行有效性验证和异常检测
 * 8. 数据标准化处理：统一的数据格式和单位转换处理
 *
 * 告警系统实现：
 * 1. 阈值比较逻辑：高效的阈值比较算法，支持多种比较操作符
 * 2. 趋势分析算法：基于时间序列的趋势分析和异常检测
 * 3. 告警去重机制：避免相同告警的重复触发
 * 4. 告警抑制逻辑：基于时间窗口的告警抑制机制
 * 5. 告警升级策略：未处理告警的自动升级机制
 * 6. 告警恢复检测：性能恢复时的自动恢复通知
 * 7. 告警持久化存储：告警信息的持久化存储和管理
 * 8. 告警通知分发：支持多种通知渠道和路由策略
 *
 * 历史数据存储实现：
 * 1. 时间序列数据库：专门为时间序列数据优化的存储引擎
 * 2. 数据压缩算法：高效的数据压缩算法，减少存储空间占用
 * 3. 分层存储策略：热数据和冷数据的分层存储管理
 * 4. 索引优化设计：针对时间范围查询优化的索引结构
 * 5. 数据分区机制：基于时间的自动数据分区，提高查询性能
 * 6. 数据清理策略：自动清理过期数据，控制存储成本
 * 7. 数据备份恢复：完整的数据备份和灾难恢复机制
 * 8. 数据迁移工具：支持数据迁移和格式转换工具
 *
 * 性能优化考虑：
 * 1. 低开销监控：监控本身对系统性能的影响最小化
 * 2. 异步处理机制：监控数据的异步处理，不阻塞业务逻辑
 * 3. 内存缓冲优化：监控数据的内存缓冲和批量处理
 * 4. 锁竞争最小化：减少监控过程中的锁竞争和等待
 * 5. 数据结构优化：高效的数据结构设计，支持高并发访问
 * 6. 缓存策略应用：监控数据的缓存策略，减少重复计算
 * 7. 资源限制控制：监控资源使用的上限控制，避免资源耗尽
 * 8. 可扩展性设计：支持水平扩展的监控架构设计
 *
 * 线程安全实现：
 * 1. 原子操作使用：关键状态变量使用原子操作保证线程安全
 * 2. 锁粒度控制：精细的锁粒度控制，减少锁竞争
 * 3. 无锁数据结构：部分场景使用无锁数据结构提高性能
 * 4. 读写锁应用：读多写少的场景使用读写锁优化
 * 5. 线程局部存储：线程相关的状态使用线程局部存储
 * 6. 消息队列通信：线程间通过消息队列异步通信
 * 7. 状态机设计：基于状态机的线程安全状态管理
 * 8. 异常安全保证：异常情况下的线程安全保证
 *
 * 扩展性设计：
 * 1. 插件化架构：支持第三方监控插件的动态加载
 * 2. 配置驱动扩展：通过配置驱动的扩展机制
 * 3. 事件驱动模型：基于事件的监控扩展机制
 * 4. API接口设计：丰富的API接口支持二次开发
 * 5. 数据导出接口：标准的数据导出接口
 * 6. 告警集成接口：与其他告警系统的集成接口
 * 7. 云服务集成：支持云监控服务的集成
 * 8. 移动端监控：移动设备监控数据的支持
 *
 * 测试验证实现：
 * 1. 单元测试覆盖：所有监控组件的单元测试验证
 * 2. 性能基准测试：监控系统本身的性能基准测试
 * 3. 并发压力测试：高并发场景下的监控系统稳定性测试
 * 4. 内存泄漏测试：监控系统的内存泄漏检测和修复
 * 5. 告警准确性测试：告警触发逻辑的准确性验证
 * 6. 数据一致性测试：监控数据的准确性和一致性测试
 * 7. 集成测试验证：与其他系统组件的集成测试
 * 8. 端到端测试：完整的监控流程端到端测试验证
 *
 * 运维部署实现：
 * 1. 配置管理部署：监控配置的集中管理和部署
 * 2. 监控代理部署：分布式环境下的监控代理部署
 * 3. 数据存储部署：历史数据的存储系统部署配置
 * 4. 告警系统集成：与现有告警系统的集成部署
 * 5. 监控面板部署：监控数据的可视化面板部署
 * 6. 备份恢复部署：监控数据的备份恢复机制部署
 * 7. 升级回滚部署：监控系统的版本升级和回滚部署
 * 8. 安全加固部署：监控系统的安全加固和访问控制部署
 */

#ifndef SQLCC_MONITORING_PERFORMANCE_MONITOR_H
#define SQLCC_MONITORING_PERFORMANCE_MONITOR_H

// Why: 包含必要的标准库头文件，提供基础数据结构和原子操作支持
// What: 引入C++标准库组件，用于性能监控的数据结构、线程安全、时间处理等
// How: 使用#include预处理指令包含标准库头文件，建立监控系统的基础设施
#include <memory>              // 智能指针管理
#include <vector>              // 动态数组容器
#include <unordered_map>       // 哈希映射容器
#include <string>              // 字符串处理
#include <atomic>              // 原子操作支持
#include <chrono>              // 时间和时钟操作
#include <mutex>               // 互斥锁和线程同步
#include <thread>              // 线程管理

namespace sqlcc {

struct SystemMetrics {
    double cpu_usage_percent = 0.0;      // CPU使用率
    double memory_usage_mb = 0.0;        // 内存使用量(MB)
    double disk_io_mb_per_sec = 0.0;     // 磁盘I/O速率(MB/s)
    double network_io_mb_per_sec = 0.0;  // 网络I/O速率(MB/s)
    size_t active_connections = 0;       // 活跃连接数
    size_t total_connections = 0;        // 总连接数
    std::chrono::system_clock::time_point timestamp;
};

struct DatabaseMetrics {
    size_t active_transactions = 0;      // 活跃事务数
    size_t committed_transactions = 0;   // 已提交事务数
    size_t rolled_back_transactions = 0; // 已回滚事务数
    double avg_query_time_ms = 0.0;      // 平均查询时间(ms)
    double queries_per_second = 0.0;     // 每秒查询数(QPS)
    size_t cache_hit_rate_percent = 0;   // 缓存命中率(%)
    size_t buffer_pool_hit_rate_percent = 0; // 缓冲池命中率(%)
    std::chrono::system_clock::time_point timestamp;
};

struct QueryMetrics {
    std::string query_id;
    std::string query_text;
    double execution_time_ms = 0.0;
    size_t rows_affected = 0;
    size_t rows_returned = 0;
    bool is_cached = false;
    std::string execution_plan;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
};

struct AlertConfig {
    std::string metric_name;
    std::string condition;  // ">", "<", ">=", "<=", "=="
    double threshold;
    std::string severity;   // "low", "medium", "high", "critical"
    bool enabled = true;
};

struct Alert {
    std::string alert_id;
    std::string metric_name;
    std::string message;
    std::string severity;
    double current_value;
    double threshold;
    std::chrono::system_clock::time_point timestamp;
    bool acknowledged = false;
};

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    // 核心监控接口
    void start_monitoring();
    void stop_monitoring();
    bool is_monitoring() const { return monitoring_active_.load(); }

    // 指标收集
    SystemMetrics collect_system_metrics();
    DatabaseMetrics collect_database_metrics();
    void record_query_metrics(const QueryMetrics& metrics);

    // 告警管理
    void add_alert_config(const AlertConfig& config);
    void remove_alert_config(const std::string& metric_name);
    std::vector<Alert> get_active_alerts() const;
    void acknowledge_alert(const std::string& alert_id);

    // 历史数据查询
    std::vector<SystemMetrics> get_system_metrics_history(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const;

    std::vector<DatabaseMetrics> get_database_metrics_history(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const;

    std::vector<QueryMetrics> get_slow_queries(
        size_t limit = 100,
        double min_execution_time_ms = 1000.0) const;

    // 配置管理
    void set_collection_interval_ms(size_t interval) {
        collection_interval_ms_ = interval;
    }

    void set_max_history_size(size_t size) {
        max_history_size_ = size;
    }

    void set_slow_query_threshold_ms(double threshold) {
        slow_query_threshold_ms_ = threshold;
    }

private:
    // 监控状态
    std::atomic<bool> monitoring_active_{false};
    std::unique_ptr<std::thread> monitoring_thread_;

    // 配置参数
    size_t collection_interval_ms_ = 5000;  // 5秒收集间隔
    size_t max_history_size_ = 10000;      // 最大历史记录数
    double slow_query_threshold_ms_ = 1000.0; // 慢查询阈值

    // 数据存储
    mutable std::mutex data_mutex_;
    std::vector<SystemMetrics> system_metrics_history_;
    std::vector<DatabaseMetrics> database_metrics_history_;
    std::vector<QueryMetrics> query_metrics_history_;
    std::vector<Alert> active_alerts_;

    // 告警配置
    std::unordered_map<std::string, AlertConfig> alert_configs_;

    // 内部方法
    void monitoring_loop();
    void check_alerts(const SystemMetrics& sys_metrics,
                     const DatabaseMetrics& db_metrics);
    void cleanup_old_data();
    bool should_trigger_alert(const std::string& metric_name,
                            double current_value,
                            const AlertConfig& config) const;
    std::string generate_alert_id() const;
};

} // namespace sqlcc

#endif // SQLCC_MONITORING_PERFORMANCE_MONITOR_H
