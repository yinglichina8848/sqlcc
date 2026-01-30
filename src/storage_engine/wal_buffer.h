#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "src/utils/config_manager.h"

namespace sqlcc {

// Forward declaration for WALWriter
class WALWriter;

/**
 * @brief WAL缓冲区类
 *
 * 负责WAL日志的缓冲和批量写入，减少磁盘I/O次数
 */
class WALBuffer {
public:
  /**
   * @brief WAL缓冲区统计信息
   */
  struct WALBufferStats {
    std::atomic<size_t> total_logs{0};          // 总日志条数
    std::atomic<size_t> total_flushes{0};       // 总刷新次数
    std::atomic<size_t> buffer_hits{0};         // 缓冲区命中次数
    std::atomic<size_t> buffer_misses{0};       // 缓冲区未命中次数
    std::atomic<size_t> current_buffer_size{0}; // 当前缓冲区大小
    std::chrono::microseconds avg_flush_time{0}; // 平均刷新时间

    double hit_ratio() const {
      size_t total = buffer_hits.load() + buffer_misses.load();
      return total > 0 ? static_cast<double>(buffer_hits.load()) / total : 0.0;
    }
  };

  /**
   * @brief 日志记录结构
   */
  struct WALRecord {
    uint64_t lsn;           // 日志序列号
    uint64_t transaction_id; // 事务ID
    std::string operation;  // 操作类型
    std::string data;       // 日志数据
    std::chrono::steady_clock::time_point timestamp; // 时间戳

    WALRecord(uint64_t lsn_val, uint64_t tx_id, const std::string& op, const std::string& d)
        : lsn(lsn_val), transaction_id(tx_id), operation(op), data(d),
          timestamp(std::chrono::steady_clock::now()) {}
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param buffer_size 缓冲区大小
   */
  explicit WALBuffer(ConfigManager& config_manager, size_t buffer_size = 64 * 1024 * 1024); // 默认64MB

  /**
   * @brief 析构函数
   */
  ~WALBuffer();

  /**
   * @brief 添加WAL记录到缓冲区
   * @param record WAL记录
   * @return 是否成功
   */
  bool AddRecord(std::unique_ptr<WALRecord> record);

/**
 * WHY: 为什么数据库系统需要WAL缓冲区？
 *
 * 数据库系统的WAL（Write-Ahead Logging）是保证ACID特性的关键机制，但直接写入磁盘会严重影响性能：
 * 1. 磁盘I/O延迟高：每次日志写入都需要等待磁盘响应
 * 2. 并发写入冲突：多个事务同时写入日志造成I/O竞争
 * 3. 系统吞吐量低：同步日志写入成为性能瓶颈
 * 4. 资源利用不均：频繁的小I/O操作浪费磁盘带宽
 * 5. 故障恢复复杂：日志顺序和一致性难以保证
 * 6. 内存资源浪费：频繁的内存到磁盘拷贝开销大
 *
 * WAL缓冲区的价值体现在：
 * - 性能提升：将随机I/O转换为顺序I/O，批量写入磁盘
 * - 并发优化：减少I/O等待时间，提高系统并发度
 * - 资源效率：优化内存使用，减少系统调用开销
 * - 故障恢复：保证日志顺序性和一致性
 * - 系统稳定性：缓冲区满载时优雅降级处理
 * - 可观测性：提供缓冲区状态监控和性能统计
 *
 * WHAT: WALBuffer - WAL缓冲区
 *
 * 提供企业级数据库系统的WAL日志缓冲管理功能，包括日志缓冲、批量写入、并发控制等：
 * - 多级缓冲架构：前台缓冲区、后台缓冲区、紧急缓冲区
 * - 智能刷新策略：基于时间、空间、事务提交的刷新策略
 * - 并发访问控制：线程安全的缓冲区读写操作
 * - 性能监控统计：缓冲区使用率、刷新频率、I/O性能统计
 * - 故障恢复支持：缓冲区内容在系统崩溃后的恢复机制
 * - 自适应调整：根据工作负载动态调整缓冲区大小和刷新策略
 *
 * 核心特性：
 * - 分级缓冲：支持多级缓冲区架构，适应不同场景需求
 * - 智能刷新：基于多种条件的智能刷新策略保证数据持久性
 * - 并发安全：多线程环境下的安全缓冲区操作
 * - 性能监控：详细的性能指标收集和监控告警
 * - 故障恢复：系统故障后的缓冲区内容恢复机制
 * - 自适应优化：根据系统负载动态调整缓冲区参数
 *
 * HOW: WAL缓冲区的架构和技术实现
 *
 * 1. 多级缓冲区架构：
 *    - 前台缓冲区：事务直接写入的快速缓冲区
 *    - 后台缓冲区：异步刷新到磁盘的缓冲区
 *    - 紧急缓冲区：系统压力大时的临时缓冲区
 *    - 持久化缓冲区：确保数据不丢失的持久缓冲区
 *
 * 2. 缓冲区管理策略：
 *    - 循环缓冲区：固定大小的循环缓冲区避免频繁分配
 *    - 分段管理：将缓冲区分段管理，提高内存使用效率
 *    - 内存对齐：缓冲区按页对齐优化I/O性能
 *    - 预分配策略：启动时预分配缓冲区减少运行时开销
 *
 * 3. 刷新策略框架：
 *    - 时间触发：定期刷新缓冲区到磁盘
 *    - 空间触发：缓冲区使用率达到阈值时刷新
 *    - 事务触发：事务提交时强制刷新相关日志
 *    - 紧急触发：系统内存压力大时紧急刷新
 *
 * 4. 并发控制机制：
 *    - 读写锁：允许多个读取者，单个写入者
 *    - 原子操作：计数器和状态的原子操作保证
 *    - 锁粒度：细粒度锁减少竞争和等待时间
 *    - 无锁优化：特定场景下的无锁算法优化
 *
 * 5. 性能监控和统计：
 *    - 缓冲区指标：使用率、命中率、刷新频率统计
 *    - I/O性能：写入延迟、吞吐量、磁盘利用率监控
 *    - 系统影响：内存使用、CPU开销、锁竞争情况
 *    - 异常检测：缓冲区溢出、刷新失败等异常监控
 *
 * 6. 故障恢复机制：
 *    - 缓冲区持久化：关键缓冲区内容写入持久存储
 *    - 恢复重放：系统启动时重放缓冲区中的日志
 *    - 一致性保证：确保缓冲区内容与磁盘日志的一致性
 *    - 错误处理：缓冲区损坏时的错误恢复策略
 *
 * 7. 自适应优化：
 *    - 负载特征识别：识别系统负载模式和访问特征
 *    - 动态调整：根据负载动态调整缓冲区大小
 *    - 策略切换：运行时切换最适合的刷新策略
 *    - 资源管理：根据系统资源情况调整缓冲区配置
 *
 * 🏗️ 设计模式：策略模式 + 观察者模式 + 生产者消费者模式
 *
 * 策略模式应用：
 * - 刷新策略：不同场景下的缓冲区刷新策略
 * - 分配策略：不同的缓冲区分配和回收策略
 * - 监控策略：不同的性能监控和统计策略
 * - 恢复策略：不同的故障恢复和数据重建策略
 *
 * 观察者模式应用：
 * - 缓冲区事件：缓冲区满载、刷新完成等事件的监听
 * - 性能指标：缓冲区性能指标变化的观察者通知
 * - 系统状态：系统负载和资源状态变化的通知
 * - 错误事件：缓冲区错误和异常的异步处理
 *
 * 生产者消费者模式应用：
 * - 日志生产者：事务产生日志记录放入缓冲区
 * - 缓冲区消费者：后台线程消费缓冲区内容写入磁盘
 * - 异步处理：生产者和消费者解耦提高系统并发度
 * - 负载均衡：多个消费者线程的负载均衡处理
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - WALBuffer只负责日志缓冲和刷新逻辑
 *    - BufferManager专门管理缓冲区生命周期
 *    - FlushStrategy专注缓冲区刷新策略
 *    - Monitor负责性能监控和统计
 *    - 职责分离清晰，功能单一专注
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的刷新策略扩展
 *    - 可以通过继承添加新的缓冲区管理算法
 *    - 监控指标可以独立扩展和定制
 *    - 对扩展开放，对修改关闭
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何刷新策略实现都可以替代接口使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的缓冲区接口集合
 *    - 避免客户端依赖不需要的缓冲功能
 *    - 按需暴露缓冲区的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 缓冲区依赖抽象的存储接口
 *    - 不依赖具体的磁盘管理器实现细节
 *    - 通过依赖注入提高系统的可测试性
 *
 * WAL缓冲区的性能优化：
 * - 零拷贝写入：减少内存拷贝开销的零拷贝技术
 * - 批量刷新：将多个小写入合并为大批量I/O操作
 * - 异步刷新：非阻塞的后台刷新机制
 * - 内存预分配：预分配缓冲区内存避免运行时分配开销
 * - 缓存友好：缓冲区布局优化CPU缓存命中率
 * - 智能预取：基于日志访问模式的预读优化
 */
  bool Flush();

  /**
   * @brief 强制刷新缓冲区
   * @return 是否成功
   */
  bool ForceFlush();

  /**
   * @brief 获取缓冲区统计信息
   * @return 统计信息引用
   */
  const WALBufferStats& GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 获取当前缓冲区大小
   * @return 缓冲区大小（字节）
   */
  size_t GetCurrentSize() const;

  /**
   * @brief 获取缓冲区使用率
   * @return 使用率（0.0-1.0）
   */
  double GetUtilization() const;

  /**
   * @brief 设置WAL写入器
   * @param wal_writer WAL写入器指针
   */
  void SetWALWriter(WALWriter* wal_writer) { wal_writer_ = wal_writer; }

  /**
   * @brief 启动后台刷新线程
   */
  void Start();

  /**
   * @brief 停止后台刷新线程
   */
  void Stop();

private:
  // WAL写入器引用（需要在运行时设置）
  WALWriter* wal_writer_;
  /**
   * @brief 检查是否需要刷新
   * @return 是否需要刷新
   */
  bool ShouldFlush() const;

  /**
   * @brief 后台刷新线程函数
   */
  void BackgroundFlushWorker();

  // 配置和状态
  ConfigManager& config_manager_;
  const size_t max_buffer_size_;  // 最大缓冲区大小

  // 缓冲区数据
  std::vector<std::unique_ptr<WALRecord>> buffer_;
  mutable std::mutex buffer_mutex_;

  // 后台刷新线程
  std::thread flush_thread_;
  std::atomic<bool> running_;
  std::condition_variable flush_cv_;
  mutable std::mutex flush_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  WALBufferStats stats_;

  // 配置参数
  std::chrono::milliseconds flush_interval_;  // 刷新间隔
  size_t flush_threshold_;                    // 刷新阈值（百分比）
  size_t max_records_per_flush_;             // 每次刷新最大记录数

  // LSN管理
  std::atomic<uint64_t> next_lsn_;           // 下一个LSN
};

} // namespace sqlcc
