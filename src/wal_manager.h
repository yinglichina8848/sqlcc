#ifndef SQLCC_WAL_MANAGER_H
#define SQLCC_WAL_MANAGER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <cstdint>  // 添加 uint64_t 定义
#include <thread>   // 添加 thread 包含

#include "src/exception/exception.h"

namespace sqlcc {

/**
 * WHY: 为什么数据库系统必须实现WAL（预写日志）机制？
 *
 * 数据库系统作为企业级数据存储的核心，面临着比传统文件系统更为严峻的可靠性挑战。
 * WAL（Write-Ahead Logging）机制的诞生，是为了解决数据库系统在ACID属性实现中的根本性难题：
 *
 * 传统存储系统的可靠性缺陷：
 * 1. **缓冲区数据丢失风险**：
 *    - 数据库通常将数据缓存在内存缓冲区中以提高性能
 *    - 系统崩溃时，缓冲区中未写入磁盘的数据会永久丢失
 *    - 用户已提交的事务数据在重启后不复存在，违反了持久性原则
 *
 * 2. **数据页部分写入问题**：
 *    - 磁盘I/O操作可能在写入过程中被中断
 *    - 数据页只写入了一部分内容，导致磁盘上的数据结构损坏
 *    - 数据库重启时发现无法解析的二进制数据，系统无法正常启动
 *
 * 3. **事务执行顺序不确定性**：
 *    - 多个并发事务的修改操作在磁盘上的写入顺序无法保证
 *    - 无法区分哪些事务已提交，哪些事务处于执行过程中
 *    - 恢复时无法确定应该重做哪些操作，撤销哪些操作
 *
 * 4. **恢复时间不可控**：
 *    - 传统恢复需要扫描整个数据库文件系统
 *    - 随着数据库规模增大，恢复时间呈线性增长
 *    - 大型数据库系统可能需要数小时甚至数天才能完成恢复
 *    - 业务连续性受到严重影响，系统可用性大大降低
 *
 * WAL机制的革命性创新：
 * - **预写日志原则**：任何数据修改前必须先写入日志，确保日志记录的持久性
 * - **顺序I/O优化**：将随机的数据页写入转换为顺序的日志记录写入
 * - **原子性保证**：通过日志记录确保事务的原子性提交或回滚
 * - **快速恢复机制**：基于日志的增量恢复，大幅缩短系统重启时间
 * - **并发控制支持**：为多版本并发控制(MVCC)提供基础支撑
 *
 * WAL在数据库系统中的核心作用：
 * - **持久性实现**：确保已提交事务的数据在系统崩溃后仍然可用
 * - **原子性保障**：支持事务的完整执行或完整回滚
 * - **一致性维护**：通过日志重演确保数据状态的一致性
 * - **隔离性支持**：为事务隔离级别提供必要的日志记录
 * - **性能优化**：通过异步刷盘和组提交优化I/O性能
 * - **故障诊断**：提供详细的操作日志用于问题排查和审计
 *
 * WAL机制对数据库系统的深远影响：
 * - **可靠性提升**：将传统文件系统的可靠性提升到企业级标准
 * - **性能优化**：通过日志机制减少随机I/O，提高整体吞吐量
 * - **可扩展性增强**：支持更大规模的并发事务处理
 * - **运维效率提高**：提供快速恢复能力和详细的监控指标
 * - **数据安全保障**：为数据备份和灾难恢复提供基础支持
 *
 * 🏗️ 设计模式：WAL管理器架构设计
 *
 * 1. **单例模式(Singleton Pattern)**：全局唯一的WAL实例
 *    - 确保整个数据库系统只有一个WAL管理器实例
 *    - 提供线程安全的全局访问接口
 *    - 避免多实例间的状态不一致和资源竞争
 *    - 支持配置的集中管理和状态的统一维护
 *
 * 2. **生产者-消费者模式(Producer-Consumer Pattern)**：异步日志刷盘
 *    - 事务处理线程作为生产者，持续生成日志记录
 *    - 后台刷盘线程作为消费者，批量处理日志写入
 *    - 通过缓冲区队列解耦日志生成和磁盘I/O操作
 *    - 支持流量削峰和性能优化
 *
 * 3. **状态机模式(State Machine Pattern)**：日志记录状态管理
 *    - 定义明确的日志记录状态转换流程
 *    - 状态驱动的崩溃恢复逻辑实现
 *    - 防止日志状态不一致和并发访问冲突
 *    - 支持复杂的事务状态追踪和管理
 *
 * 4. **策略模式(Strategy Pattern)**：可插拔的日志处理策略
 *    - 同步刷盘策略：立即写入磁盘，确保强一致性
 *    - 异步刷盘策略：后台批量写入，提高性能
 *    - 组提交策略：多个事务批量刷盘，减少I/O次数
 *    - 压缩策略：日志记录的实时压缩和归档
 *
 * 5. **观察者模式(Observer Pattern)**：日志事件通知机制
 *    - 监控关键日志事件和状态变化
 *    - 通知相关组件进行相应的处理和响应
 *    - 支持日志事件的订阅和分发机制
 *    - 实现组件间的松耦合通信
 *
 * SOLID原则在WAL管理器中的体现：
 *
 * 1. **单一职责原则(SRP)**：职责高度集中和专业化
 *    - WALManager只负责预写日志的记录、管理和恢复
 *    - 日志格式定义、序列号分配、刷盘策略由专门的方法处理
 *    - 崩溃恢复、检查点管理、性能监控各司其职
 *    - 每个方法和组件都有明确的单一职责，易于理解和维护
 *
 * 2. **开闭原则(OCP)**：对扩展开放，对修改关闭
 *    - 新增日志记录类型无需修改WALManager核心逻辑
 *    - 新的刷盘策略可以通过策略模式轻松集成
 *    - 日志压缩算法和存储后端可以独立扩展
 *    - 插件化架构保证核心功能的稳定性和扩展性
 *
 * 3. **里氏替换原则(LSP)**：保证接口的一致性和可替换性
 *    - 任何WAL实现都可以替代默认的WALManager使用
 *    - 子类必须完全实现父类的接口契约
 *    - 客户端代码无需关心具体WAL实现的差异
 *    - 多态性保证了系统的灵活性和兼容性
 *
 * 4. **接口隔离原则(ISP)**：按需提供最小化接口
 *    - 客户端只依赖WAL管理所需的必要接口
 *    - 复杂的内部实现和管理接口不暴露给外部使用
 *    - 不同类型的客户端（事务管理器、存储引擎等）使用不同的接口子集
 *    - 避免接口污染和过度耦合的问题
 *
 * 5. **依赖倒置原则(DIP)**：依赖抽象而非具体实现
 *    - WALManager依赖抽象的存储接口和配置接口
 *    - 不依赖具体的文件系统或磁盘硬件实现
 *    - 通过依赖注入提高系统的可测试性和灵活性
 *    - 抽象层隔离了实现的变更，提高了系统的稳定性
 *
 * WHAT: WALManager - SQLCC预写日志管理系统
 *
 * 企业级数据库系统的ACID属性核心实现组件，负责所有数据修改操作的日志记录、
 * 事务持久性保证、系统崩溃恢复和性能优化。
 *
 * 核心功能特性：
 * - **日志记录与管理**：完整的数据修改操作日志记录和生命周期管理
 * - **事务原子性保障**：通过日志记录确保事务的原子性提交或回滚
 * - **数据持久性保证**：确保已提交事务的数据在系统崩溃后仍然可用
 * - **崩溃恢复机制**：基于日志的快速准确的系统状态恢复
 * - **检查点管理**：定期创建系统状态快照，支持增量恢复优化
 * - **并发控制支持**：为多事务并发执行提供必要的日志序列化
 * - **性能监控统计**：详细的日志操作性能指标和统计信息
 * - **存储优化管理**：智能的日志文件管理、压缩和清理策略
 *
 * 支持的日志记录类型：
 * - **BEGIN**：事务开始记录，标记事务生命周期的起点
 * - **COMMIT**：事务提交记录，确保事务修改的持久性
 * - **ABORT**：事务中止记录，支持事务回滚操作
 * - **UPDATE**：数据更新记录，记录字段值的修改操作
 * - **INSERT**：数据插入记录，记录新数据的添加操作
 * - **DELETE**：数据删除记录，记录数据的移除操作
 * - **COMPENSATE**：补偿记录，用于复杂事务的补偿操作
 *
 * 接口设计原则：
 * - **日志写入接口**：Log() - 写入单个日志记录，支持同步和异步模式
 * - **批量写入接口**：LogBatch() - 批量写入多个日志记录，提高效率
 * - **强制刷盘接口**：ForceFlush() - 强制将所有待写入日志刷盘
 * - **异步刷盘接口**：AsyncFlush() - 启动后台异步刷盘线程
 * - **日志读取接口**：ReadLogRange() - 读取指定范围的日志记录
 * - **恢复执行接口**：RecoverFromLog() - 执行基于日志的崩溃恢复
 * - **检查点接口**：CreateCheckpoint() - 创建系统状态检查点
 * - **性能监控接口**：GetMetrics() - 获取WAL性能指标和统计信息
 *
 * HOW: WAL预写日志机制的技术实现原理
 *
 * 1. **日志记录生成流程**：
 *    - 事务开始：生成BEGIN记录，分配全局唯一的事务ID
 *    - 数据修改前：为每个数据修改操作生成相应的日志记录
 *    - 记录内容：包含事务ID、操作类型、数据位置、旧值、新值等
 *    - LSN分配：原子递增的日志序列号，确保全局唯一和有序
 *    - 缓冲区暂存：先写入内存缓冲区，提高写入性能
 *
 * 2. **日志刷盘策略**：
 *    - 同步刷盘：事务提交时立即将相关日志写入磁盘
 *    - 异步刷盘：后台线程定期批量将缓冲区日志写入磁盘
 *    - 组提交优化：多个事务的日志记录批量刷盘，减少I/O次数
 *    - 条件变量同步：协调生产者线程和消费者刷盘线程
 *    - 性能监控：实时监控刷盘延迟和成功率
 *
 * 3. **崩溃恢复执行流程**：
 *    - 分析阶段：从最后一个检查点开始，扫描WAL日志
 *    - 事务状态识别：区分已提交、未提交和进行中的事务
 *    - 重做阶段：重演已提交事务的修改操作（REDO）
 *    - 撤销阶段：撤销未提交事务的修改操作（UNDO）
 *    - 一致性验证：确保恢复后的数据库状态满足ACID属性
 *    - 性能优化：通过并行恢复和增量检查点优化恢复速度
 *
 * 4. **检查点管理机制**：
 *    - 定期创建：基于时间间隔或日志大小触发检查点创建
 *    - 状态快照：记录当前系统的完整状态信息
 *    - 日志清理：清理检查点之前的无用日志记录
 *    - 恢复优化：从最近检查点开始恢复，减少恢复时间
 *    - 并发控制：在创建检查点时不阻塞正常事务处理
 *
 * 5. **并发控制和线程安全**：
 *    - 原子操作：使用原子变量保证LSN分配的线程安全
 *    - 互斥锁：保护共享缓冲区和状态变量的访问
 *    - 读写锁：允许多个读操作，独占写操作
 *    - 条件变量：协调异步刷盘线程的执行
 *    - 死锁避免：通过锁的有序获取避免死锁情况
 *
 * 6. **性能优化策略**：
 *    - 内存缓冲：减少磁盘I/O频率，提高吞吐量
 *    - 顺序写入：将随机I/O转换为高性能的顺序I/O
 *    - 批量处理：多个日志记录的批量写入优化
 *    - 压缩存储：日志记录的压缩减少存储空间
 *    - 预分配空间：日志文件的预分配减少动态扩展开销
 *    - 缓存优化：热点日志记录的内存缓存
 *
 * 7. **存储管理和维护**：
 *    - 多文件轮转：支持多个日志文件的循环使用
 *    - 空间管理：自动清理过期和无用的日志文件
 *    - 完整性校验：CRC校验确保日志记录的完整性
 *    - 备份策略：日志文件的定期备份和归档
 *    - 故障转移：日志文件损坏时的自动切换机制
 *
 * 错误处理和恢复机制：
 * - **日志完整性验证**：CRC校验和确保日志记录的正确性
 * - **坏块检测和跳过**：自动检测并跳过损坏的日志记录
 * - **空间不足处理**：日志文件空间不足时的自动清理策略
 * - **并发冲突解决**：多线程访问时的冲突检测和解决
 * - **事务状态一致性**：确保事务状态在异常情况下的正确性
 * - **自动修复机制**：常见错误的自动检测和修复
 *
 * 性能监控和诊断支持：
 * - **吞吐量统计**：日志记录的写入速度和处理能力
 * - **延迟监控**：刷盘操作的平均延迟和最大延迟
 * - **缓存效率**：缓冲区命中率和利用率的统计
 * - **恢复时间**：崩溃恢复操作的时间消耗分析
 * - **存储效率**：日志文件大小和压缩比率的监控
 * - **错误统计**：各类错误的发生频率和处理情况
 * - **健康检查**：WAL系统的整体健康状态评估
 *
 * 扩展性和可维护性设计：
 * - **插件化架构**：支持自定义的日志处理和存储插件
 * - **配置驱动设计**：可配置的刷盘策略和性能参数
 * - **多日志文件支持**：支持分布式环境下的多日志文件管理
 * - **存储后端抽象**：支持不同类型的存储后端（本地磁盘、分布式存储等）
 * - **压缩算法扩展**：可插拔的日志压缩算法
 * - **监控接口标准化**：统一的性能监控和诊断接口
 *
 * WAL的生命周期管理：
 * 1. **初始化阶段**：创建日志文件，初始化缓冲区和状态
 * 2. **运行阶段**：持续记录日志，支持并发读写操作
 * 3. **检查点阶段**：定期创建检查点，清理过期日志
 * 4. **恢复阶段**：系统重启时执行崩溃恢复操作
 * 5. **维护阶段**：日志文件的压缩、备份和清理
 * 6. **关闭阶段**：确保所有待写入日志被刷盘，释放资源
 *
 * 监控指标和告警：
 * - 性能指标：日志写入速度、刷盘延迟、缓存命中率等
 * - 健康指标：日志文件完整性、缓冲区使用率等
 * - 错误指标：校验失败率、空间不足告警等
 * - 容量指标：日志文件大小、清理频率等
 * - 恢复指标：恢复时间、成功率等
 */

// 前向声明
class TransactionManager;

// 事务ID类型定义
using TransactionId = uint64_t;

// WAL 记录类型
enum class LogRecordType {
    BEGIN,           // 事务开始
    COMMIT,          // 事务提交
    ABORT,           // 事务中止
    UPDATE,          // 数据更新
    INSERT,          // 数据插入
    DELETE,          // 数据删除
    COMPENSATE       // 补偿记录
};

// WAL 记录值类型
struct Value {
    enum class Type { INT, DOUBLE, STRING } type;
    union {
        int64_t int_val;
        double double_val;
    };
    std::string str_val;

    Value() : type(Type::INT), int_val(0) {}
    explicit Value(int64_t v) : type(Type::INT), int_val(v) {}
    explicit Value(double v) : type(Type::DOUBLE), double_val(v) {}
    explicit Value(const std::string& s) : type(Type::STRING), str_val(s) {}
    
    // 重载==操作符，用于比较两个Value对象是否相等
    bool operator==(const Value& other) const {
        if (type != other.type) {
            return false;
        }
        switch (type) {
            case Type::INT:
                return int_val == other.int_val;
            case Type::DOUBLE:
                return double_val == other.double_val;
            case Type::STRING:
                return str_val == other.str_val;
            default:
                return false;
        }
    }

    // 重载!=操作符，用于比较两个Value对象是否不相等
    bool operator!=(const Value& other) const {
        return !(*this == other);
    }

    // 重载<操作符，用于比较两个Value对象
    bool operator<(const Value& other) const {
        if (type != other.type) {
            return static_cast<int>(type) < static_cast<int>(other.type);
        }
        switch (type) {
            case Type::INT:
                return int_val < other.int_val;
            case Type::DOUBLE:
                return double_val < other.double_val;
            case Type::STRING:
                return str_val < other.str_val;
            default:
                return false;
        }
    }

    // 重载>操作符，用于比较两个Value对象
    bool operator>(const Value& other) const {
        return other < *this;
    }

    // 转换为字符串
    std::string toString() const {
        switch (type) {
            case Type::INT:
                return std::to_string(int_val);
            case Type::DOUBLE:
                return std::to_string(double_val);
            case Type::STRING:
                return str_val;
            default:
                return "";
        }
    }

    // 比较方法（用于排序）
    int compare(const Value& other) const {
        if (type != other.type) {
            return static_cast<int>(type) - static_cast<int>(other.type);
        }
        switch (type) {
            case Type::INT:
                return (int_val < other.int_val) ? -1 : (int_val > other.int_val) ? 1 : 0;
            case Type::DOUBLE:
                return (double_val < other.double_val) ? -1 : (double_val > other.double_val) ? 1 : 0;
            case Type::STRING:
                return str_val.compare(other.str_val);
            default:
                return 0;
        }
    }
};

// WAL 日志记录
struct LogRecord {
    TransactionId txn_id;              // 事务ID
    LogRecordType type;                // 操作类型
    std::string key;                   // 键
    Value old_value;                   // 旧值
    Value new_value;                   // 新值
    uint64_t lsn;                      // 日志序列号
    std::chrono::system_clock::time_point timestamp; // 时间戳

    LogRecord() = default;
    LogRecord(TransactionId txn, LogRecordType t, const std::string& k)
        : txn_id(txn), type(t), key(k), timestamp(std::chrono::system_clock::now()) {}

    std::string ToString() const;
};

// 检查点状态
struct CheckpointState {
    uint64_t checkpoint_lsn;           // 检查点LSN
    std::chrono::system_clock::time_point timestamp; // 时间戳
    std::unordered_map<std::string, Value> page_states; // 页面状态快照
};

/**
 * WAL（预写日志）管理器 - v0.4.8版本初始实现
 *
 * 核心功能：
 * - 日志记录写入和读取
 * - 原子性确保 (Atomicity)
 * - 持久性确保 (Durability)
 * - 检查点机制
 * - 崩溃恢复支持
 *
 * 设计原则：
 * - 写前日志：数据修改前必须先写日志
 * - 顺序写：高性能顺序I/O
 * - 批量提交：减少I/O次数
 * - 异步刷盘：性能和一致性平衡
 */
class WALManager {
public:
    /**
     * 构造函数
     * @param log_file_path 日志文件路径
     * @param force_sync 是否强制同步写入
     */
    explicit WALManager(const std::string& log_file_path, bool force_sync = false);

    /**
     * 析构函数 - 确保所有日志都刷盘
     */
    ~WALManager();

    // ---------- 核心日志操作 ----------

    /**
     * 写入日志记录
     * @param record 要写入的日志记录
     * @return 分配的LSN
     */
    uint64_t Log(LogRecord record);

    /**
     * 批量写入日志记录
     * @param records 日志记录列表
     * @return 最后分配的LSN
     */
    uint64_t LogBatch(const std::vector<LogRecord>& records);

    /**
     * 强制刷盘所有待写入日志
     */
    void ForceFlush();

    /**
     * 异步刷盘（后台线程）
     */
    void AsyncFlush();

    // ---------- 日志读取和分析 ----------

    /**
     * 读取指定范围的日志记录
     * @param from_lsn 起始LSN
     * @param to_lsn 结束LSN
     * @return 日志记录列表
     */
    std::vector<LogRecord> ReadLogRange(uint64_t from_lsn, uint64_t to_lsn);

    /**
     * 分析日志文件状态
     * @return 状态信息
     */
    std::unordered_map<std::string, std::string> AnalyzeLog() const;

    // ---------- 检查点机制 ----------

    /**
     * 创建检查点
     * @param sync 是否同步写入
     * @return 检查点LSN
     */
    uint64_t CreateCheckpoint(bool sync = true);

    /**
     * 获取最后一个检查点状态
     * @return 检查点状态
     */
    CheckpointState GetLastCheckpoint() const;

    /**
     * 获取检查点历史
     * @return 检查点列表（按时间排序）
     */
    std::vector<CheckpointState> GetCheckpointHistory() const;

    // ---------- 崩溃恢复相关的操作 ----------

    /**
     * 从WAL执行崩溃恢复
     * @return 是否恢复成功
     */
    bool RecoverFromLog();

    /**
     * 获取正在进行事务的ID列表
     * @return 事务ID列表
     */
    std::vector<TransactionId> GetInProgressTransactions() const;

    /**
     * 重演事务日志
     * @param from_lsn 起始LSN
     * @param to_lsn 结束LSN
     * @return 成功重演到最后一个LSN
     */
    uint64_t ReplayLog(uint64_t from_lsn, uint64_t to_lsn);

    // ---------- 性能监控 ----------

    /**
     * WAL性能指标
     */
    struct WALMetrics {
        size_t total_records;           // 总日志记录数
        size_t flushed_records;         // 已刷盘记录数
        size_t pending_records;         // 待刷盘记录数
        std::chrono::microseconds avg_flush_time{0};    // 平均刷盘时间
        std::chrono::microseconds total_flush_time{0};  // 总刷盘时间
        size_t total_checkpoints;       // 总检查点次数
        size_t log_file_size_bytes;     // 日志文件大小
    };

    /**
     * 获取WAL性能指标
     * @return 性能指标
     */
    WALMetrics GetMetrics() const;

    /**
     * 重置性能指标
     */
    void ResetMetrics();

    // ---------- 维护操作 ----------

    /**
     * 整理日志文件（移除不必要的旧日志）
     * @param keep_lsn 需要保持的最小LSN
     * @return 被清理的日志大小（字节）
     */
    size_t CompactLog(uint64_t keep_lsn);

    /**
     * 验证日志完整性
     * @return 是否完整
     */
    bool VerifyLogIntegrity() const;

private:
    // ---------- 内部实现 ----------

    /**
     * 初始化日志文件
     */
    void InitializeLogFile();

    /**
     * 生成新的LSN
     * @return 新的LSN
     */
    uint64_t GenerateLSN();

    /**
     * 实际写入日志记录到磁盘
     * @param records 要写入的记录
     * @return 成功写入的记录数量
     */
    size_t WriteRecordsToDisk(const std::vector<LogRecord>& records);

    /**
     * 读取日志记录从磁盘
     * @param lsn 指定的LSN
     * @return 日志记录
     */
    LogRecord ReadRecordFromDisk(uint64_t lsn);

    /**
     * 写入检查点到磁盘
     * @param checkpoint 检查点状态
     */
    void WriteCheckpointToDisk(const CheckpointState& checkpoint);

    /**
     * 从磁盘读取检查点
     * @return 检查点状态
     */
    CheckpointState ReadCheckpointFromDisk() const;

    /**
     * 异步刷盘线程函数
     */
    void AsyncFlushThread();

    // ---------- 成员变量 ----------

    std::string log_file_path_;                    // 日志文件路径
    std::string checkpoint_file_path_;             // 检查点文件路径

    std::atomic<uint64_t> next_lsn_;               // 下一个LSN
    std::atomic<uint64_t> last_flushed_lsn_;       // 最后刷盘的LSN
    std::atomic<uint64_t> last_checkpoint_lsn_;    // 最后检查点LSN

    // 日志缓冲区（内存中）
    std::vector<LogRecord> log_buffer_;            // 日志缓冲区
    std::mutex buffer_mutex_;                      // 缓冲区锁
    std::condition_variable buffer_cv_;            // 缓冲区条件变量

    // 异步刷盘线程
    std::unique_ptr<std::thread> flush_thread_;    // 刷盘线程
    std::atomic<bool> stop_flush_thread_;          // 停止刷盘线程标志

    bool force_sync_;                              // 是否强制同步写入
    uint32_t flush_interval_ms_;                   // 异步刷盘间隔（毫秒）

    // 性能指标
    mutable std::mutex metrics_mutex_;             // 指标锁
    WALMetrics metrics_;                           // 性能指标

    // 检查点历史
    std::vector<CheckpointState> checkpoint_history_; // 检查点历史列表
    mutable std::mutex checkpoint_mutex_;          // 检查点锁

    // 防止拷贝
    WALManager(const WALManager&) = delete;
    WALManager& operator=(const WALManager&) = delete;
};

} // namespace sqlcc

#endif // SQLCC_WAL_MANAGER_H
