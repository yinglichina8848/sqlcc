#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "disk_manager.h"
#include "page.h"
#include "storage/concurrency_control.h"
#include "storage/replace_strategy.h"
#include "utils/config_manager.h"

namespace sqlcc {

/**
 * @brief 页面缓存条目
 */
struct BufferPage {
  Page data;                                         // 页面数据
  int32_t page_id;                                   // 页面ID
  int ref_count;                                     // 引用计数
  bool is_dirty;                                     // 是否脏页
  std::chrono::steady_clock::time_point access_time; // 最后访问时间
  std::shared_mutex page_mutex;                      // 页面级别的读写锁

  BufferPage() : page_id(-1), ref_count(0), is_dirty(false) {}
  BufferPage(int32_t id) : page_id(id), ref_count(0), is_dirty(false) {}
};

/**
 * @brief 重构后的BufferPool类，支持可插拔替换策略和高级并发控制
 */
class BufferPool {
public:
  /**
   * @brief 构造函数
   * @param disk_manager 磁盘管理器
   * @param config_manager 配置管理器
   * @param pool_size 缓冲池大小（页面数）
   */
  BufferPool(std::shared_ptr<DiskManager> disk_manager,
             ConfigManager &config_manager, size_t pool_size = 1024);

  /**
   * @brief 析构函数，刷新所有脏页
   */
  ~BufferPool();

  /**
   * @brief 获取页面
   * @param page_id 页面ID
   * @param transaction_id 事务ID（用于并发控制）
   * @return 页面指针，如果失败返回nullptr
   */
  std::shared_ptr<BufferPage> FetchPage(int32_t page_id,
                                        int32_t transaction_id = -1);

  /**
   * @brief 释放页面引用
   * @param page_id 页面ID
   * @param transaction_id 事务ID（用于并发控制）
   * @return 是否成功
   */
  bool UnpinPage(int32_t page_id, int32_t transaction_id = -1);

  /**
   * @brief 创建新页面
   * @param transaction_id 事务ID（用于并发控制）
   * @return 新页面ID，如果失败返回-1
   */
  int32_t NewPage(int32_t transaction_id = -1);

  /**
   * @brief 刷新页面到磁盘
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool FlushPage(int32_t page_id);

  /**
   * @brief 删除页面
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool DeletePage(int32_t page_id);

  /**
   * @brief 刷新所有脏页到磁盘
   * @return 是否成功
   */
  bool FlushAllPages();

  /**
   * @brief 调整缓冲池大小
   * @param new_pool_size 新的缓冲池大小
   * @return 是否成功
   */
  bool Resize(size_t new_pool_size);

  /**
   * @brief 获取缓冲池统计信息
   */
  struct BufferPoolStats {
    size_t pool_size;                                      // 缓冲池大小
    size_t allocated_pages;                                // 已分配页面数
    size_t dirty_pages;                                    // 脏页数
    size_t pinned_pages;                                   // 被钉住的页面数
    double hit_ratio;                                      // 命中率
    size_t total_requests;                                 // 总请求数
    size_t cache_hits;                                     // 缓存命中数
    std::chrono::microseconds avg_access_time;             // 平均访问时间
    ReplaceStrategyFactory::StrategyType strategy_type;    // 替换策略类型
    AbstractReplaceStrategy::StrategyStats strategy_stats; // 替换策略统计
    HierarchicalLockManager::LockManagerStats lock_stats;  // 锁管理器统计
    Prefetcher::PrefetcherStats prefetch_stats;            // 预取器统计

    // 默认构造函数
    BufferPoolStats() = default;

    // 拷贝构造函数
    BufferPoolStats(const BufferPoolStats &other) = default;

    // 赋值操作符
    BufferPoolStats &operator=(const BufferPoolStats &other) = default;
  };

  BufferPoolStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 更改替换策略
   * @param strategy_type 新的替换策略类型
   * @return 是否成功
   */
  bool
  ChangeReplaceStrategy(ReplaceStrategyFactory::StrategyType strategy_type);

  /**
   * @brief 启用/禁用预取器
   * @param enabled 是否启用
   */
  void SetPrefetcherEnabled(bool enabled);

private:
  /**
   * @brief 从磁盘加载页面
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool LoadPageFromDisk(int32_t page_id);

  /**
   * @brief 将页面写入磁盘
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool WritePageToDisk(int32_t page_id);

  /**
   * @brief 替换一个页面
   * @return 被替换的页面ID，如果没有可替换的页面返回-1
   */
  int32_t EvictPage();

  /**
   * @brief 添加页面到缓冲池
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool AddPageToPool(int32_t page_id);

  /**
   * @brief 从缓冲池移除页面
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool RemovePageFromPool(int32_t page_id);

  /**
   * @brief 更新统计信息
   */
  void UpdateStats(bool is_hit, std::chrono::microseconds access_time);

  // 配置和依赖
  ConfigManager &config_manager_;
  std::shared_ptr<DiskManager> disk_manager_;
  size_t pool_size_;

  // 页面缓存
  std::unordered_map<int32_t, std::shared_ptr<BufferPage>> page_table_;
  mutable std::shared_mutex page_table_mutex_;

  // 替换策略
  std::unique_ptr<AbstractReplaceStrategy> replace_strategy_;

  // 并发控制 - 使用智能指针确保自动内存管理
  std::unique_ptr<HierarchicalLockManager> lock_manager_;
  std::unique_ptr<Prefetcher> prefetcher_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  BufferPoolStats stats_;

  // 下一个页面ID
  std::atomic<int32_t> next_page_id_;

  // 是否已关闭
  std::atomic<bool> shutdown_;
};

} // namespace sqlcc
