/**
 * @file disk_manager.h
 * @brief SQLCC底层存储抽象 - 物理磁盘与逻辑页面映射引擎
 *
 * DiskManager 是数据库持久化层的核心。它直接与操作系统文件系统交互，
 * 负责将 PageID 转换为物理文件偏移量，并提供高性能的块读写操作。
 *
 * 📚 配套教材参考：
 * - [第6章：物理存储管理](../../textbook/《数据库系统原理与开发实践》.md#第六章物理存储管理)
 * - [6.1 磁盘I/O模型](../../textbook/《数据库系统原理与开发实践》.md#61-磁盘io模型)
 * - [6.4 文件管理与存取方法](../../textbook/《数据库系统原理与开发实践》.md#64-文件管理与存取方法)
 *
 * WHY层 - 设计意图：
 *   数据库系统不能直接依赖操作系统的文件缓冲（Buffered I/O），因为这会导致事务持久性（Durability）不可控。
 *   DiskManager 存在的意义是：
 *   1. **逻辑抽象**：将杂乱的字节流文件抽象为连续的、固定大小的页面序列。
 *   2. **性能优化**：通过预读（Prefetching）和批量读写减少系统调用次数。
 *   3. **可靠性**：显式控制 Sync（fsync）操作，保证 WAL（预写日志）先于数据落盘。
 *   4. **故障注入**：支持在测试环境中模拟I/O错误，验证系统的稳健性。
 *
 * WHAT层 - 功能说明：
 *   - 页面读写：ReadPage/WritePage 实现基础块I/O。
 *   - 空间管理：AllocatePage/DeallocatePage 管理文件增长。
 *   - 性能监控：实时收集读写次数、耗时、失败率等统计信息（DiskIOStats）。
 *   - 预读架构：支持异步预读请求，为顺序扫描提供硬件级优化。
 *
 * HOW层 - 实现机制：
 *   - 物理映射：Offset = PageID * PAGE_SIZE。
 *   - 原子操作：使用 std::atomic 维护 next_page_id_，确保多线程下页面分配的唯一性。
 *   - 锁策略：采用 std::recursive_timed_mutex。递归锁允许同一线程多次进入（如嵌套调用），
 *     限时锁（timed_mutex）防止在高竞争场景下出现永久死锁，提高系统响应性。
 *   - 异常处理：封装文件系统异常，提供结构化的错误返回。
 *
 * 性能优化考量：
 *   - **顺序写入**：尽量通过顺序 PageID 分配来利用磁盘的顺序写性能。
 *   - **零拷贝倾向**：读写接口直接操作 raw pointer/span，避免中间缓冲区。
 *   - **统计开销**：DiskIOStats 采用原子变量，将性能监控的同步开销降至最低。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../page/page.h"
#include "../utils/config_manager.h"
#include "../exception/exception.h"

namespace sqlcc {

/**
 * @brief 磁盘I/O实时统计信息
 * 
 * WHY: 生产环境需要了解存储引擎的负载情况。
 * WHAT: 记录所有成功和失败的I/O操作及其延迟。
 * HOW: 使用 std::atomic 保证多线程累加的原子性。
 */
struct DiskIOStats {
  std::atomic<size_t> total_reads{0};       ///< 总成功读取次数
  std::atomic<size_t> total_writes{0};      ///< 总成功写入次数
  std::atomic<size_t> total_syncs{0};       ///< 总同步次数 (fsync)
  std::atomic<size_t> failed_reads{0};      ///< 读取失败次数 (如坏块)
  std::atomic<size_t> failed_writes{0};     ///< 写入失败次数 (如空间不足)
  std::atomic<size_t> failed_syncs{0};      ///< 同步失败次数
  std::chrono::microseconds total_read_time{0};  ///< 累积读取耗时
  std::chrono::microseconds total_write_time{0}; ///< 累积写入耗时
};

/**
 * @class DiskManager
 * @brief 磁盘管理器 - 物理I/O的终点，持久化的起点
 */
class DiskManager {
public:
    /**
     * @brief 构造函数
     * @param db_file 物理数据库文件路径
     * @param config_manager 用于获取I/O超时等运行时配置
     */
    DiskManager(const std::string &db_file, ConfigManager &config_manager);

    /**
     * @brief 析构函数 - 确保关闭文件句柄
     */
    ~DiskManager();

    /**
     * @brief 将8KB页面数据原子地写入磁盘
     * 
     * WHY: 它是持久化的核心操作。
     * HOW: 定位到 Offset = page_id * 8192，执行文件写入。
     * @param page_id 目标页面ID
     * @param page_data 指向8KB内存块的指针
     * @return 写入是否成功
     */
    bool WritePage(int32_t page_id, const char *page_data);

    /**
     * @brief 从磁盘物理地址加载页面数据
     * 
     * @param page_id 源页面ID
     * @param page_data 目标内存块指针
     */
    bool ReadPage(int32_t page_id, char *page_data);

    /**
     * @brief 逻辑空间分配
     * 
     * WHY: 实现文件空间的自动扩展。
     * WHAT: 返回一个新的 PageID，物理上通过递增文件指针实现。
     * @return 新分配的页面ID，-1表示分配失败
     */
    int32_t AllocatePage();

    /**
     * @brief 逻辑空间释放
     * 
     * WHAT: 将页面ID加入空闲列表供后续复用，而不是立即缩减文件大小。
     */
    bool DeallocatePage(int32_t page_id);

    /**
     * @brief 获取物理文件字节数
     */
    int32_t GetFileSize() const;

    /**
     * @brief 批量读取优化
     * 
     * WHY: 减少系统调用（System Call）次数。
     * HOW: 将不连续的读请求合并或并行下发。
     */
    bool BatchReadPages(const std::vector<int32_t> &page_ids,
                        std::vector<char *> &data_buffers);

    /**
     * @brief 异步预读触发
     * 
     * WHY: 隐藏磁盘I/O延迟，预加载数据到BufferPool。
     */
    bool PrefetchPage(int32_t page_id);

    /**
     * @brief 批量预读
     */
    bool BatchPrefetchPages(const std::vector<int32_t> &page_ids);

    /**
     * @brief 强制物理刷盘 (fsync)
     * 
     * WHY: 绕过操作系统缓存，确保数据真正到达物理介质，满足ACID的D属性。
     */
    bool Sync();

    /**
     * @brief 配置热更新回调
     */
    void OnConfigChange(const std::string &key, const ConfigValue &value);

    // I/O统计信息
    DiskIOStats io_stats_;

private:
    std::string db_file_name_;
    std::fstream db_io_;
    ConfigManager &config_manager_;
    std::atomic<size_t> file_size_;
    std::atomic<int32_t> next_page_id_;
    int lock_timeout_ms_;

    /**
     * @brief 线程安全互斥锁
     * 使用 recursive_timed_mutex 以支持：
     * 1. 同一线程重入（递归）
     * 2. 防止无限等待（限时）
     */
    mutable std::recursive_timed_mutex io_mutex_;

    std::vector<int32_t> free_pages_;

    // 故障注入标志位（仅用于自动化测试）
    bool simulate_seek_failure_ = false;
    bool simulate_read_failure_ = false;
    bool simulate_write_failure_ = false;
    bool simulate_flush_failure_ = false;

    friend class DiskManagerTest;
};

} // namespace sqlcc

