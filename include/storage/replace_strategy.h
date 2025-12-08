#pragma once

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "exception.h"
#include "page.h"
#include "utils/config_manager.h"

namespace sqlcc {

/**
 * @brief 页面替换策略抽象基类
 *
 * 定义了缓冲池页面替换策略的通用接口，支持多种替换算法的可插拔实现。
 * 主要替换策略包括LRU、2Q、LFU、ARC等。
 */
class AbstractReplaceStrategy {
public:
  /**
   * @brief 页面访问记录结构
   */
  struct PageAccessInfo {
    int32_t page_id;                                   // 页面ID
    int ref_count;                                     // 引用计数
    bool is_dirty;                                     // 脏页标记
    std::chrono::steady_clock::time_point access_time; // 最后访问时间
    size_t access_frequency;                           // 访问频率

    PageAccessInfo(int32_t id = -1)
        : page_id(id), ref_count(0), is_dirty(false),
          access_time(std::chrono::steady_clock::now()), access_frequency(1) {}
  };

  /**
   * @brief 替换策略统计信息
   */
  struct StrategyStats {
    std::atomic<size_t> total_accesses{0};  // 总访问次数
    std::atomic<size_t> total_evictions{0}; // 总替换次数
    std::atomic<size_t> strategy_hits{0};   // 策略命中次数
    std::atomic<size_t> strategy_misses{0}; // 策略未命中次数

    // 默认构造函数
    StrategyStats() = default;

    // 拷贝构造函数
    StrategyStats(const StrategyStats &other)
        : total_accesses(other.total_accesses.load()),
          total_evictions(other.total_evictions.load()),
          strategy_hits(other.strategy_hits.load()),
          strategy_misses(other.strategy_misses.load()) {}

    // 赋值操作符
    StrategyStats &operator=(const StrategyStats &other) {
      if (this != &other) {
        total_accesses.store(other.total_accesses.load());
        total_evictions.store(other.total_evictions.load());
        strategy_hits.store(other.strategy_hits.load());
        strategy_misses.store(other.strategy_misses.load());
      }
      return *this;
    }

    double hit_rate() const {
      size_t total = total_accesses.load();
      return total > 0
                 ? (static_cast<double>(strategy_hits.load()) * 100.0) / total
                 : 0.0;
    }
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param max_pages 最大页面数量
   */
  explicit AbstractReplaceStrategy(ConfigManager &config_manager,
                                   size_t max_pages)
      : config_manager_(config_manager), max_pages_(max_pages) {}

  /**
   * @brief 虚析构函数
   */
  virtual ~AbstractReplaceStrategy() = default;

  /**
   * @brief 通知页面被访问
   * @param page_id 页面ID
   * @param is_dirty 是否为脏页
   */
  virtual void NotifyPageAccess(int32_t page_id, bool is_dirty) = 0;

  /**
   * @brief 通知页面被释放
   * @param page_id 页面ID
   */
  virtual void NotifyPageRelease(int32_t page_id) = 0;

  /**
   * @brief 选择要替换的页面
   * @return 要替换的页面ID，如果没有可替换的页面返回-1
   */
  virtual int32_t SelectVictim() = 0;

  /**
   * @brief 检查页面是否在策略中
   * @param page_id 页面ID
   * @return 是否存在
   */
  virtual bool Contains(int32_t page_id) const = 0;

  /**
   * @brief 获取策略中的页面数量
   * @return 页面数量
   */
  virtual size_t Size() const = 0;

  /**
   * @brief 获取策略统计信息
   * @return 统计信息
   */
  virtual StrategyStats GetStats() const = 0;

  /**
   * @brief 重置策略状态
   */
  virtual void Reset() = 0;

  /**
   * @brief 获取策略名称
   * @return 策略名称
   */
  virtual std::string GetStrategyName() const = 0;

  /**
   * @brief 更新配置参数
   */
  virtual void UpdateConfig() = 0;

protected:
  ConfigManager &config_manager_;     // 配置管理器引用
  size_t max_pages_;                  // 最大页面数量
  mutable std::mutex strategy_mutex_; // 策略互斥锁
};

/**
 * @brief LRU (Least Recently Used) 替换策略实现
 *
 * LRU策略选择最近最少使用的页面进行替换。这是最常用的页面替换算法之一，
 * 适用于大多数通用数据库工作负载。
 */
class LRUStrategy : public AbstractReplaceStrategy {
public:
  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param max_pages 最大页面数量
   */
  explicit LRUStrategy(ConfigManager &config_manager, size_t max_pages);

  /**
   * @brief 析构函数
   */
  ~LRUStrategy() override = default;

  // 实现基类接口
  void NotifyPageAccess(int32_t page_id, bool is_dirty) override;
  void NotifyPageRelease(int32_t page_id) override;
  int32_t SelectVictim() override;
  bool Contains(int32_t page_id) const override;
  size_t Size() const override;
  StrategyStats GetStats() const override;
  void Reset() override;
  std::string GetStrategyName() const override;
  void UpdateConfig() override;

private:
  // LRU链表：最近使用的在头部，最少使用的在尾部
  std::list<int32_t> lru_list_;

  // 页面ID到LRU链表迭代器的映射，用于快速查找和更新
  std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;

  // 页面访问信息
  std::unordered_map<int32_t, PageAccessInfo> page_info_;

  // 统计信息
  StrategyStats stats_;
};

/**
 * @brief 2Q (Two Queues) 替换策略实现
 *
 * 2Q策略是LRU的改进版本，使用两个队列来管理页面：
 * 1. A1in队列：最近被访问一次的页面
 * 2. Am队列：被访问多次的页面
 *
 * 这种策略能够更好地处理循环扫描和突发访问的工作负载。
 */
class TwoQStrategy : public AbstractReplaceStrategy {
public:
  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param max_pages 最大页面数量
   */
  explicit TwoQStrategy(ConfigManager &config_manager, size_t max_pages);

  /**
   * @brief 析构函数
   */
  ~TwoQStrategy() override = default;

  // 实现基类接口
  void NotifyPageAccess(int32_t page_id, bool is_dirty) override;
  void NotifyPageRelease(int32_t page_id) override;
  int32_t SelectVictim() override;
  bool Contains(int32_t page_id) const override;
  size_t Size() const override;
  StrategyStats GetStats() const override;
  void Reset() override;
  std::string GetStrategyName() const override;
  void UpdateConfig() override;

private:
  // A1in队列：最近被访问一次的页面（FIFO）
  std::list<int32_t> a1in_list_;
  std::unordered_map<int32_t, std::list<int32_t>::iterator> a1in_map_;

  // Am队列：被访问多次的页面（LRU）
  std::list<int32_t> am_list_;
  std::unordered_map<int32_t, std::list<int32_t>::iterator> am_map_;

  // A1out历史队列：记录从A1in队列移出的页面ID
  std::list<int32_t> a1out_history_;
  std::unordered_map<int32_t, std::list<int32_t>::iterator> a1out_map_;

  // 页面访问信息
  std::unordered_map<int32_t, PageAccessInfo> page_info_;

  // 队列大小配置
  size_t a1in_capacity_;  // A1in队列容量
  size_t am_capacity_;    // Am队列容量
  size_t a1out_capacity_; // A1out历史队列容量

  // 统计信息
  StrategyStats stats_;

  /**
   * @brief 将页面移动到Am队列
   * @param page_id 页面ID
   */
  void MoveToAmQueue(int32_t page_id);

  /**
   * @brief 从A1in队列移除页面
   * @param page_id 页面ID
   */
  void RemoveFromA1inQueue(int32_t page_id);

  /**
   * @brief 更新队列容量配置
   */
  void UpdateQueueCapacities();
};

/**
 * @brief 替换策略工厂类
 *
 * 负责创建和管理不同类型的替换策略实例。
 */
class ReplaceStrategyFactory {
public:
  /**
   * @brief 策略类型枚举
   */
  enum class StrategyType {
    LRU,   // LRU策略
    TWO_Q, // 2Q策略
    LFU,   // LFU策略（未来扩展）
    ARC    // ARC策略（未来扩展）
  };

  /**
   * @brief 创建替换策略实例
   * @param type 策略类型
   * @param config_manager 配置管理器引用
   * @param max_pages 最大页面数量
   * @return 替换策略智能指针
   */
  static std::unique_ptr<AbstractReplaceStrategy>
  CreateStrategy(StrategyType type, ConfigManager &config_manager,
                 size_t max_pages);

  /**
   * @brief 从字符串解析策略类型
   * @param strategy_name 策略名称字符串
   * @return 策略类型
   */
  static StrategyType ParseStrategyType(const std::string &strategy_name);

  /**
   * @brief 获取策略类型的字符串表示
   * @param type 策略类型
   * @return 策略名称字符串
   */
  static std::string GetStrategyName(StrategyType type);
};

} // namespace sqlcc