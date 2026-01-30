#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/utils/config_manager.h"
#include "src/storage_engine/storage_engine.h"
#include "src/storage_engine/wal_writer.h"

namespace sqlcc {

/**
 * @class CheckpointManager
 * @brief SQLCC检查点管理器 - WAL系统的清理和同步核心组件
 *
 * WHY层 - 设计意图：
 *   检查点是数据库系统的"定期清理工"，解决了WAL日志无限增长的问题。
 *   通过定期将内存中的脏数据刷新到磁盘，检查点确保了系统在任意时刻都能快速恢复。
 *   检查点机制平衡了性能(减少恢复时间)和存储效率(清理过期日志)。
 *
 * WHAT层 - 功能说明：
 *   定期执行检查点操作，将所有脏页面刷新到磁盘，记录检查点位置。
 *   清理检查点之前的WAL日志，释放存储空间，支持崩溃恢复优化。
 *   提供手动和自动检查点触发，支持配置化的检查点策略。
 *
 * HOW层 - 实现细节：
 *   使用独立工作线程定期执行检查点，避免阻塞正常数据库操作。
 *   采用模糊检查点策略，允许事务在检查点期间继续执行。
 *   通过LSN(Log Sequence Number)跟踪检查点进度，支持精确的日志清理。
 *   实现渐进式检查点，将大批量I/O操作分解为多个小步骤。
 *
 * 检查点策略选择：
 *   - **定时检查点**: 按时间间隔执行，确保恢复时间可控
 *   - **条件检查点**: 基于脏页比例或日志大小触发
 *   - **模糊检查点**: 不阻塞事务，提高并发性能
 *   - **增量检查点**: 分阶段执行，减少单次I/O压力
 *
 * 并发安全保证：
 *   - 检查点操作不阻塞正常读写事务
 *   - 通过页面级锁确保数据一致性
 *   - 原子性更新检查点元数据
 *   - 线程安全的统计信息收集
 *
 * 性能优化策略：
 *   - 异步执行，不影响事务响应时间
 *   - 批量页面刷新，减少磁盘寻道
 *   - 智能日志清理，只删除已确认持久化的日志
 *   - 自适应间隔，根据负载动态调整检查点频率
 *
 * 故障恢复支持：
 *   - 检查点位置记录系统一致状态
 *   - 减少崩溃恢复需要重放的日志量
 *   - 支持从多个检查点进行增量恢复
 *   - 提供检查点完整性验证机制
 *
 * @note 检查点是数据库性能和可靠性的关键平衡点
 * @note 过于频繁的检查点会影响性能，过于稀疏会增加恢复时间
 * @note 现代数据库通常每几分钟执行一次检查点
 *
 * @see docs/design/storage_engine/checkpoint_system_design.md
 *      检查点系统的完整设计文档，包含算法分析和性能优化指南
 */
class CheckpointManager {
public:
  /**
   * @brief 检查点统计信息
   */
  struct CheckpointStats {
    std::atomic<size_t> total_checkpoints{0};     // 总检查点次数
    std::atomic<size_t> total_pages_flushed{0};   // 总刷新页面数
    std::atomic<size_t> total_bytes_flushed{0};   // 总刷新字节数
    std::atomic<size_t> wal_logs_cleaned{0};      // 清理的WAL日志数
    std::chrono::microseconds avg_checkpoint_time{0}; // 平均检查点时间
    std::chrono::microseconds max_checkpoint_time{0}; // 最大检查点时间
    std::chrono::steady_clock::time_point last_checkpoint; // 最后检查点时间

    double avg_pages_per_checkpoint() const {
      size_t checkpoints = total_checkpoints.load();
      return checkpoints > 0
                 ? static_cast<double>(total_pages_flushed.load()) / checkpoints
                 : 0.0;
    }
  };

  /**
   * @brief 检查点配置
   */
  struct CheckpointConfig {
    std::chrono::seconds interval{300};          // 检查点间隔（默认5分钟）
    size_t max_wal_size{1024 * 1024 * 1024};     // 最大WAL大小（默认1GB）
    size_t max_checkpoint_pages{10000};          // 单次检查点最大页面数
    bool enable_auto_checkpoint{true};           // 是否启用自动检查点
    double dirty_page_threshold{0.8};            // 脏页阈值（80%）
  };

  /**
   * @brief 构造函数 - 初始化检查点管理器核心组件
   *
   * WHY层 - 设计意图：
   *   检查点管理器是数据库系统的稳定运行保障，其初始化必须确保配置正确
   *   和依赖关系清晰。通过构造函数建立与存储引擎和WAL系统的紧密协作关系。
   *
   * WHAT层 - 功能说明：
   *   初始化检查点管理器，配置参数从配置文件读取，验证系统依赖关系。
   *   建立与存储引擎和WAL写入器的引用关系，准备统计信息收集。
   *
   * HOW层 - 实现细节：
   *   1. 存储对配置管理器、存储引擎、WAL写入器的引用
   *   2. 初始化默认检查点配置参数
   *   3. 从配置文件加载自定义参数覆盖默认值
   *   4. 初始化统计信息结构，准备性能监控
   *   5. 验证依赖组件的可用性
   *
   * 关键依赖关系：
   *   - ConfigManager: 提供检查点间隔、阈值等配置参数
   *   - StorageEngine: 执行脏页刷新操作
   *   - WALWriter: 提供LSN信息和日志同步
   *
   * @param config_manager 配置管理器引用，提供运行时参数
   * @param storage_engine 存储引擎引用，执行页面刷新操作
   * @param wal_writer WAL写入器引用，提供日志序列号管理
   *
   * @note 构造函数不启动后台线程，只进行初始化
   * @note 依赖组件必须在检查点管理器生命周期内保持有效
   * @note 配置参数可在运行时动态调整
   */
  CheckpointManager(ConfigManager& config_manager,
                    StorageEngine& storage_engine,
                    WALWriter& wal_writer);

  /**
   * @brief 析构函数 - 安全关闭检查点管理器
   *
   * WHY层 - 设计意图：
   *   检查点管理器的关闭必须保证正在进行的检查点操作完成，
   *   避免资源泄漏和数据不一致。优雅关闭确保系统可以安全重启。
   *
   * WHAT层 - 功能说明：
   *   停止后台检查点线程，等待当前检查点操作完成。
   *   清理系统资源，确保所有操作都被正确终止。
   *
   * HOW层 - 实现细节：
   *   1. 调用Stop()停止后台线程
   *   2. 等待线程完全停止
   *   3. 清理内部状态和资源
   *   4. 记录关闭统计信息
   *
   * @note 析构函数会阻塞直到检查点操作安全完成
   * @note 这是数据库正常关闭的关键步骤
   */
  ~CheckpointManager();

  /**
   * @brief 启动检查点管理器 - 激活定期检查点机制
   *
   * WHY层 - 设计意图：
   *   检查点管理器的启动标志着数据库进入自动维护模式。
   *   后台线程的启动确保检查点操作不会阻塞正常事务处理。
   *
   * WHAT层 - 功能说明：
   *   启动后台检查点线程，开始定期执行检查点操作。
   *   初始化运行状态标志和统计信息收集。
   *
   * HOW层 - 实现细节：
   *   1. 设置running_标志为true
   *   2. 创建并启动checkpoint_thread_
   *   3. 线程执行CheckpointWorker()函数
   *   4. 初始化条件变量和互斥锁
   *   5. 记录启动时间戳
   *
   * 线程安全保证：
   *   - 启动操作是原子的，不会被并发调用
   *   - 后台线程独立运行，不影响主线程
   *   - 状态标志使用原子操作保证可见性
   *
   * 性能影响：
   *   - 增加少量系统资源消耗（一个后台线程）
   *   - 定期I/O操作，但通过配置控制频率
   *   - 不影响正常事务处理性能
   *
   * @note 必须在数据库启动序列中调用
   * @note 重复调用Start()是安全的（无操作）
   * @note 启动后自动检查点立即生效
   */
  void Start();

  /**
   * @brief 停止检查点管理器 - 优雅关闭定期检查点机制
   *
   * WHY层 - 设计意图：
   *   检查点管理器的停止必须保证正在进行的检查点操作完成，
   *   避免强制终止可能导致的数据不一致。确保系统可以安全重启。
   *
   * WHAT层 - 功能说明：
   *   停止后台检查点线程，等待当前检查点操作完成。
   *   确保所有缓冲的数据都被正确处理。
   *
   * HOW层 - 实现细节：
   *   1. 设置running_标志为false，通知后台线程退出
   *   2. 通过条件变量唤醒可能等待的线程
   *   3. 等待checkpoint_thread_完全停止
   *   4. 清理线程相关资源
   *   5. 记录停止统计信息
   *
   * 数据完整性保证：
   *   - 后台线程会完成当前检查点操作
   *   - 不会中断正在进行的页面刷新
   *   - 所有状态变更都被正确保存
   *
   * 阻塞行为：
   *   - Stop()会阻塞直到检查点操作完成
   *   - 在检查点进行中时可能需要几秒钟
   *   - 这是数据库关闭时的必要等待
   *
   * @note 必须在数据库关闭序列中调用
   * @note 重复调用Stop()是安全的（无操作）
   * @note 调用后检查点管理器进入不可用状态
   */
  void Stop();

  /**
   * @brief 执行检查点操作 - 完整的检查点执行流程
   *
   * WHY层 - 设计意图：
   *   检查点操作是数据库系统的核心维护任务，保证数据一致性和可恢复性。
   *   通过刷新脏页和清理日志，在性能和可靠性之间取得最佳平衡。
   *
   * WHAT层 - 功能说明：
   *   执行完整的检查点流程：刷新脏页、记录检查点、清理旧日志。
   *   更新系统状态，确保崩溃恢复的效率和正确性。
   *
   * HOW层 - 实现细节：
   *   1. 记录检查点开始时间和统计信息
   *   2. 调用DoCheckpoint()执行实际检查点逻辑
   *   3. 更新统计信息（执行次数、耗时等）
   *   4. 处理检查点失败的情况
   *   5. 返回操作结果状态
   *
   * 执行流程：
   *   - 准备阶段：记录开始状态，验证系统状态
   *   - 执行阶段：调用DoCheckpoint()完成核心操作
   *   - 清理阶段：更新统计信息，重置临时状态
   *   - 通知阶段：可选的通知监控系统
   *
   * 错误处理：
   *   - 检查点失败时记录错误日志
   *   - 更新失败统计信息
   *   - 不影响系统正常运行
   *   - 支持下次重试
   *
   * 并发安全：
   *   - 检查点操作本身是串行的
   *   - 通过锁机制保证数据一致性
   *   - 不阻塞正常读写操作
   *
   * @return 检查点操作是否成功完成
   *
   * @note 这是一个同步操作，会等待检查点完成
   * @note 检查点失败不会影响数据库正常运行
   * @note 失败时会记录详细错误信息用于诊断
   */
  bool PerformCheckpoint();

  /**
   * @brief 强制执行检查点 - 立即触发检查点操作
   *
   * WHY层 - 设计意图：
   *   提供手动触发检查点的能力，在需要立即同步数据时使用。
   *   绕过定期检查点的等待时间，直接执行完整的检查点流程。
   *
   * WHAT层 - 功能说明：
   *   立即执行检查点操作，无论当前是否满足自动检查点条件。
   *   适用于系统维护、备份前的数据同步等场景。
   *
   * HOW层 - 实现细节：
   *   1. 直接调用PerformCheckpoint()执行检查点
   *   2. 记录强制检查点操作的统计信息
   *   3. 返回操作结果状态
   *
   * 使用场景：
   *   - 系统维护前的数据同步
   *   - 备份操作前确保数据一致性
   *   - 性能监控和诊断
   *   - 手动触发日志清理
   *
   * 性能影响：
   *   - 会立即触发I/O密集型操作
   *   - 可能影响系统响应时间
   *   - 完成后恢复正常性能
   *
   * @return 强制检查点操作是否成功完成
   *
   * @note 这是一个同步操作，会等待检查点完成
   * @note 与自动检查点使用相同的底层逻辑
   * @note 强制检查点也会更新所有统计信息
   */
  bool ForceCheckpoint();

  /**
   * @brief 获取检查点统计信息 - 监控检查点操作性能
   *
   * WHY层 - 设计意图：
   *   检查点统计信息是系统性能监控的重要指标，帮助识别性能瓶颈。
   *   通过详细的统计数据，支持系统的性能调优和故障诊断。
   *
   * WHAT层 - 功能说明：
   *   返回检查点管理器的详细统计信息，包括执行次数、耗时、页面刷新量等。
   *   支持实时监控检查点操作的效率和健康状态。
   *
   * HOW层 - 实现细节：
   *   1. 返回stats_成员的const引用
   *   2. 统计信息是线程安全的原子操作
   *   3. 包含历史累积数据和实时状态
   *
   * 统计指标：
   *   - total_checkpoints: 总检查点执行次数
   *   - total_pages_flushed: 总刷新页面数
   *   - avg_checkpoint_time: 平均检查点耗时
   *   - max_checkpoint_time: 最大检查点耗时
   *   - last_checkpoint: 最后检查点时间
   *
   * 监控价值：
   *   - 检查点频率是否合理
   *   - I/O性能是否正常
   *   - 系统负载对检查点的影响
   *   - 潜在的性能瓶颈识别
   *
   * @return 检查点统计信息的常量引用
   *
   * @note 统计信息是实时更新的
   * @note 返回引用避免数据拷贝开销
   * @note 统计数据在系统重启后重置
   */
  const CheckpointStats& GetStats() const;

  /**
   * @brief 重置统计信息 - 清空历史统计数据
   *
   * WHY层 - 设计意图：
   *   统计信息重置支持性能监控的周期性分析，
   *   避免历史数据对当前性能评估的影响。
   *
   * WHAT层 - 功能说明：
   *   将所有统计信息重置为初始状态，清空历史累积数据。
   *   适用于新的监控周期开始或诊断分析。
   *
   * HOW层 - 实现细节：
   *   1. 获取统计信息锁，保证操作原子性
   *   2. 重置所有原子计数器为0
   *   3. 重置时间戳为当前时间
   *   4. 保持配置信息不变
   *
   * 使用场景：
   *   - 新的性能监控周期开始
   *   - 系统配置变更后的性能评估
   *   - 诊断特定时间段的性能问题
   *   - 基准测试前的环境清理
   *
   * @note 重置操作是原子的，不会影响正在进行的检查点
   * @note 重置后立即开始收集新的统计数据
   * @note 不会影响检查点配置参数
   */
  void ResetStats();

  /**
   * @brief 设置检查点配置 - 动态调整检查点策略
   *
   * WHY层 - 设计意图：
   *   运行时配置调整支持根据工作负载动态优化检查点策略，
   *   平衡性能和可靠性需求，避免配置静态化导致的 suboptimal。
   *
   * WHAT层 - 功能说明：
   *   更新检查点管理器的配置参数，支持运行时动态调整。
   *   配置变更立即生效，影响后续检查点操作。
   *
   * HOW层 - 实现细节：
   *   1. 获取配置锁，保证操作原子性
   *   2. 复制新的配置参数
   *   3. 验证配置参数的有效性
   *   4. 更新内部配置状态
   *   5. 记录配置变更日志
   *
   * 可配置参数：
   *   - interval: 检查点执行间隔
   *   - max_wal_size: 触发检查点的WAL大小阈值
   *   - dirty_page_threshold: 脏页比例阈值
   *   - enable_auto_checkpoint: 是否启用自动检查点
   *
   * 配置验证：
   *   - 间隔时间必须大于0
   *   - 阈值参数必须在合理范围内
   *   - 配置变更不会中断正在进行的检查点
   *
   * @param config 新的检查点配置参数
   *
   * @note 配置变更立即生效
   * @note 无效配置会被拒绝并记录警告
   * @note 支持运行时性能调优
   */
  void SetConfig(const CheckpointConfig& config);

  /**
   * @brief 获取当前检查点配置 - 查看当前运行参数
   *
   * WHY层 - 设计意图：
   *   配置查询支持监控和诊断，帮助理解当前检查点行为。
   *   提供配置一致性验证，确保配置正确应用。
   *
   * WHAT层 - 功能说明：
   *   返回当前检查点管理器使用的配置参数副本。
   *   支持外部监控系统查询当前配置状态。
   *
   * HOW层 - 实现细节：
   *   1. 获取配置锁，保证数据一致性
   *   2. 返回配置参数的副本
   *   3. 避免返回内部状态的直接引用
   *
   * 返回内容：
   *   - 当前生效的所有配置参数
   *   - 参数值的副本，确保线程安全
   *   - 包含默认值和运行时调整值
   *
   * @return 当前检查点配置参数的副本
   *
   * @note 返回副本避免外部修改内部状态
   * @note 配置查询是线程安全的
   * @note 可用于配置一致性验证
   */
  CheckpointConfig GetConfig() const;

  /**
   * @brief 检查是否需要检查点 - 条件判断逻辑
   *
   * WHY层 - 设计意图：
   *   智能判断检查点时机，避免不必要的I/O操作浪费系统资源。
   *   基于多维度条件综合评估，确保检查点在最合适的时机执行。
   *
   * WHAT层 - 功能说明：
   *   根据配置的检查点策略，判断当前是否满足执行检查点的条件。
   *   支持时间间隔、WAL大小、脏页比例等多种触发条件。
   *
   * HOW层 - 实现细节：
   *   1. 检查时间间隔条件：距离上次检查点是否超过配置间隔
   *   2. 检查WAL大小条件：当前WAL日志大小是否超过阈值
   *   3. 检查脏页比例条件：脏页占总页面的比例是否超过阈值
   *   4. 检查系统状态：确保没有其他冲突操作
   *   5. 返回综合判断结果
   *
   * 判断逻辑：
   *   ```cpp
   *   bool needs_checkpoint = (time_since_last_checkpoint >= interval) ||
   *                          (current_wal_size >= max_wal_size) ||
   *                          (dirty_page_ratio >= dirty_page_threshold);
   *   ```
   *
   * 条件组合：
   *   - OR逻辑：任一条件满足即可触发
   *   - 优先级：时间间隔 > WAL大小 > 脏页比例
   *   - 灵活性：支持根据负载调整权重
   *
   * 性能优化：
   *   - 判断操作轻量级，不涉及I/O
   *   - 缓存常用状态信息，减少计算开销
   *   - 支持批量条件检查，提高效率
   *
   * @return 是否满足检查点执行条件
   *
   * @note 判断逻辑基于配置参数动态调整
   * @note 返回true并不意味着立即执行检查点
   * @note 用于定期检查和条件触发机制
   */
  bool ShouldCheckpoint() const;

private:
  /**
   * @brief 检查点工作线程函数
   */
  void CheckpointWorker();

  /**
   * @brief 执行检查点逻辑
   * @return 是否成功
   */
  bool DoCheckpoint();

  /**
   * @brief 刷新所有脏页面
   * @return 刷新页面数
   */
  size_t FlushDirtyPages();

  /**
   * @brief 清理过期的WAL日志
   * @param min_lsn 最小保留LSN
   * @return 清理的日志数
   */
  size_t CleanupWALLogs(uint64_t min_lsn);

  /**
   * @brief 更新检查点元数据
   * @param checkpoint_lsn 检查点LSN
   * @return 是否成功
   */
  bool UpdateCheckpointMetadata(uint64_t checkpoint_lsn);

  // 配置和依赖
  ConfigManager& config_manager_;
  StorageEngine& storage_engine_;
  WALWriter& wal_writer_;

  // 工作线程
  std::thread checkpoint_thread_;
  std::atomic<bool> running_;
  std::condition_variable checkpoint_cv_;
  mutable std::mutex checkpoint_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  CheckpointStats stats_;

  // 配置
  mutable std::mutex config_mutex_;
  CheckpointConfig config_;
};

} // namespace sqlcc
