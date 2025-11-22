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

#include "exception.h"

namespace sqlcc {

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
