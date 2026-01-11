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

#include "exception.h"

namespace sqlcc {

/**
 * WHY: 为什么需要WAL（预写日志）管理器？
 *
 * 传统数据库系统在崩溃恢复时面临数据丢失和不一致问题：
 * - 缓冲区数据未及时写入磁盘：系统崩溃导致数据丢失
 * - 部分写入的数据页：磁盘上存在不一致状态
 * - 事务执行顺序不确定：无法确定哪些事务已提交
 * - 恢复时间过长：需要扫描整个数据库进行完整性检查
 *
 * WAL机制的核心价值：
 * 1. 持久性保证：确保已提交事务的数据不会丢失
 * 2. 原子性保证：事务要么完全执行，要么完全不执行
 * 3. 崩溃恢复：快速准确地恢复到一致性状态
 * 4. 性能优化：将随机I/O转换为顺序I/O
 *
 * WAL在数据库系统中的关键作用：
 * - 故障恢复：系统崩溃后自动恢复数据一致性
 * - 并发控制：支持多事务并发执行的隔离性
 * - 数据安全：防止数据丢失和损坏
 * - 性能提升：减少磁盘随机访问次数
 *
 * 🏗️ 设计模式：WAL管理器架构设计
 *
 * 设计模式应用：
 * 1. 单例模式(Singleton Pattern)：全局唯一的WAL实例
 *    - 确保整个系统只有一个WAL管理器
 *    - 提供全局访问接口
 *    - 避免多实例间的状态不一致
 *
 * 2. 生产者-消费者模式(Producer-Consumer Pattern)：异步刷盘
 *    - 事务线程生产日志记录
 *    - 后台线程消费并刷盘
 *    - 解耦日志写入和刷盘操作
 *
 * 3. 状态机模式(State Machine Pattern)：日志状态管理
 *    - 明确的日志状态转换
 *    - 状态驱动的恢复逻辑
 *    - 防止状态不一致
 *
 * 4. 策略模式(Strategy Pattern)：可插拔刷盘策略
 *    - 同步刷盘：立即写入磁盘
 *    - 异步刷盘：后台批量写入
 *    - 组提交：多个事务批量刷盘
 *
 * SOLID原则体现：
 * - 单一职责：专职负责WAL日志管理
 * - 开闭原则：新日志类型通过扩展实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: WAL（预写日志）管理器 - 数据库ACID属性的核心保障
 *
 * 核心功能：
 * - 日志记录：记录所有数据修改操作
 * - 原子性保证：事务的原子性执行
 * - 持久性保证：已提交数据的持久保存
 * - 崩溃恢复：系统崩溃后的自动恢复
 * - 检查点管理：定期创建系统快照
 *
 * WAL记录类型：
 * - BEGIN：事务开始记录
 * - COMMIT：事务提交记录
 * - ABORT：事务中止记录
 * - UPDATE：数据更新记录
 * - INSERT：数据插入记录
 * - DELETE：数据删除记录
 * - COMPENSATE：补偿记录
 *
 * 接口设计：
 * - Log(): 写入单个日志记录
 * - LogBatch(): 批量写入日志记录
 * - ForceFlush(): 强制刷盘所有日志
 * - RecoverFromLog(): 从日志执行崩溃恢复
 * - CreateCheckpoint(): 创建检查点
 * - GetMetrics(): 获取性能指标
 *
 * HOW: WAL预写日志机制的实现原理
 *
 * 日志写入流程：
 * 1. 事务开始：写入BEGIN记录到WAL缓冲区
 * 2. 数据修改：写入UPDATE/INSERT/DELETE记录
 * 3. 事务提交：写入COMMIT记录并刷盘
 * 4. 异步刷盘：后台线程定期将缓冲区数据写入磁盘
 *
 * 崩溃恢复流程：
 * 1. 分析阶段：扫描WAL日志，识别未完成事务
 * 2. 重做阶段：重做已提交事务的修改操作
 * 3. 撤销阶段：撤销未提交事务的修改操作
 * 4. 检查点：从最后一个检查点开始恢复
 *
 * 性能优化策略：
 * - 组提交：多个事务的日志批量刷盘
 * - 异步刷盘：后台线程处理磁盘I/O
 * - 缓冲区管理：内存缓冲区减少磁盘访问
 * - 顺序写入：将随机I/O转换为顺序I/O
 * - 日志压缩：定期清理无用日志记录
 *
 * 并发控制机制：
 * - LSN分配：原子递增的日志序列号
 * - 锁机制：保护共享缓冲区和状态
 * - 条件变量：协调生产者和消费者线程
 * - 原子操作：无锁的状态更新
 *
 * 存储结构设计：
 * - 日志文件：顺序存储的日志记录
 * - 索引文件：快速定位日志记录
 * - 检查点文件：系统状态快照
 * - 缓冲区：内存中的日志缓存
 *
 * 错误处理策略：
 * - 日志完整性检查：CRC校验和验证
 * - 坏块处理：自动跳过损坏的日志记录
 * - 空间管理：自动清理过期日志
 * - 故障转移：备用日志文件的自动切换
 *
 * 监控和诊断：
 * - 性能指标：日志写入速度、刷盘延迟等
 * - 健康检查：日志文件完整性验证
 * - 统计信息：事务处理统计和恢复统计
 * - 告警机制：异常情况的自动告警
 *
 * 扩展性设计：
 * - 多日志文件：支持多个日志文件的并发写入
 * - 分布式支持：支持分布式事务的日志管理
 * - 压缩算法：可配置的日志压缩策略
 * - 存储后端：支持不同的存储后端
 * - 插件架构：支持自定义的日志处理插件
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
