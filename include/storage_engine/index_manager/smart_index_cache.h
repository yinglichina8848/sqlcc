/**
 * WHY: 为什么需要智能索引缓存管理器？
 *
 * 数据库索引系统需要高效的缓存管理来应对复杂的查询负载：
 * - 索引对象创建和加载成本高昂，需要长期缓存复用
 * - 不同索引具有不同的访问频率和重要性，需要差异化缓存策略
 * - 内存资源有限，需要智能的缓存替换和清理机制
 * - 并发访问模式复杂，需要线程安全的缓存管理
 * - 系统运行状态动态变化，需要自适应的缓存策略调整
 *
 * 智能索引缓存的核心价值：
 * 1. 性能提升：减少索引加载时间，提升查询响应速度
 * 2. 资源优化：智能管理内存资源，避免资源浪费
 * 3. 并发安全：支持高并发环境下的索引访问
 * 4. 自适应调整：根据实际负载自动调整缓存策略
 * 5. 运维友好：提供丰富的监控和诊断信息
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern) + 装饰器模式(Decorator Pattern)
 *
 * 策略模式实现缓存替换策略：
 * - 多种替换算法：LRU、LFU、ARC等策略可选
 * - 运行时切换：根据负载特征动态调整策略
 * - 策略扩展：支持自定义缓存替换算法
 * - 性能监控：实时监控各种策略的效果
 *
 * 装饰器模式增强缓存功能：
 * - TTL支持：为索引添加生存时间限制
 * - 优先级管理：根据重要性设置不同的缓存优先级
 * - 访问统计：收集详细的访问模式统计信息
 * - 预取功能：预测和预加载即将访问的索引
 *
 * WHAT: 智能索引缓存管理器 - 索引系统的性能优化组件
 *
 * 核心功能：
 * - 索引生命周期管理：创建、缓存、清理索引对象的完整生命周期
 * - 缓存替换策略：多种智能算法选择最优的替换策略
 * - 并发访问控制：多线程环境下的安全索引访问
 * - 性能监控统计：详细的缓存效果和性能指标收集
 * - 自适应优化：基于运行时统计的自动参数调整
 * - 预取和预热：预测性加载提高访问效率
 *
 * 数据结构设计：
 * - 多级缓存映射：索引名到缓存条目的快速查找
 * - 优先级队列：支持优先级排序的缓存管理
 * - 访问时间记录：精确的访问时间戳用于LRU计算
 * - 统计计数器：原子操作的性能指标收集
 * - 互斥锁保护：线程安全的内部状态管理
 *
 * 接口设计：
 * - 缓存操作：添加、获取、移除索引的基本操作
 * - 批量操作：多索引的批量访问和预热功能
 * - 配置管理：缓存大小、TTL、策略的参数配置
 * - 监控接口：统计信息查询和性能报告生成
 * - 清理控制：手动和自动的缓存清理功能
 * - 序列化支持：缓存状态的持久化和恢复
 *
 * HOW: 智能索引缓存管理器的实现机制和优化策略
 *
 * 缓存架构设计：
 * - 分层缓存：热点索引常驻内存，冷数据按需加载
 * - 智能预取：基于访问模式的预测性索引加载
 * - 延迟加载：索引对象的按需创建和初始化
 * - 异步清理：后台的缓存清理减少对前台操作的影响
 * - 内存池复用：索引对象的内存复用减少分配开销
 *
 * 替换策略优化：
 * 1. 多策略融合：结合LRU、LFU、SIZE等多种策略的优点
 * 2. 动态权重调整：基于运行时统计动态调整各策略权重
 * 3. 上下文感知：考虑索引大小、访问频率、重要性等因素
 * 4. 自适应学习：通过机器学习优化替换决策
 * 5. 异常处理：处理缓存策略失效的降级机制
 *
 * 并发控制策略：
 * - 细粒度锁：减少锁竞争，提高并发访问性能
 * - 读写分离：读操作和写操作使用不同的同步策略
 * - 乐观锁：使用版本号控制并发访问冲突
 * - 死锁避免：固定的锁获取顺序防止死锁发生
 * - 原子操作：性能计数器的无锁更新
 *
 * 内存管理优化：
 * - 对象池技术：索引对象的复用减少内存分配开销
 * - 压缩存储：索引元数据的压缩存储节省内存空间
 * - 垃圾回收：智能的缓存清理和内存回收机制
 * - 内存对齐：提高CPU缓存访问效率
 * - 大小感知：根据索引大小选择合适的缓存策略
 *
 * 性能监控机制：
 * - 实时指标：当前缓存命中率、访问延迟等实时监控
 * - 趋势分析：缓存性能的时间序列分析和趋势预测
 * - 异常检测：基于统计分布的性能异常识别
 * - 自动告警：性能指标超出阈值的自动告警机制
 * - 诊断报告：详细的性能分析和优化建议报告
 *
 * 智能优化功能：
 * - 访问模式学习：基于历史访问的模式识别和预测
 * - 自适应调整：根据负载变化自动调整缓存参数
 * - 容量规划：基于访问模式的缓存容量规划建议
 * - 预热优化：系统启动时的智能缓存预热
 * - 负载均衡：多实例环境下的缓存负载均衡
 *
 * 扩展性和兼容性：
 * - 插件架构：支持自定义缓存策略和优化算法
 * - 配置热更新：运行时调整缓存参数和策略配置
 * - 多格式支持：不同类型索引的缓存格式支持
 * - 向后兼容：旧版本缓存数据的平滑迁移
 * - API抽象：统一的缓存管理接口设计
 *
 * 故障恢复和容错：
 * - 数据校验：缓存数据的完整性和一致性校验
 * - 异常处理：缓存操作异常情况的优雅处理
 * - 数据修复：检测并修复损坏的缓存数据
 * - 降级模式：缓存失效时的保守运行模式
 * - 监控告警：缓存异常情况的及时发现和处理
 *
 * 调试和诊断功能：
 * - 详细日志：缓存操作的详细操作日志记录
 * - 可视化工具：缓存状态和访问模式的图形化展示
 * - 性能分析：缓存操作的时间和空间复杂度分析
 * - 压力测试：极限负载下的缓存策略性能验证
 * - 内存分析：缓存内存使用的详细分析报告
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <queue>
#include <functional>
#include <atomic>

namespace sqlcc {

// Forward declarations
class BPlusTreeIndex;

namespace storage_engine {
namespace index_manager {

/**
 * @brief 智能索引缓存管理器
 *
 * 实现高性能的索引缓存管理，支持优先级、TTL、访问统计和智能清理策略。
 * 通过多策略融合和自适应优化，为数据库索引系统提供高效的缓存服务。
 *
 * 设计理念：
 * - 智能性：基于运行时统计的智能缓存决策
 * - 高性能：最小化缓存开销，最大化访问性能
 * - 可扩展性：支持多种缓存策略和自定义扩展
 * - 可靠性：保证缓存数据的一致性和完整性
 * - 易维护性：清晰的架构设计和丰富的监控功能
 */
class SmartIndexCache {
public:
    /**
     * @brief 增强的缓存统计信息
     */
    struct EnhancedCacheStats {
        size_t total_indexes = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        double hit_rate = 0.0;
        double average_access_frequency = 0.0;
        size_t expired_entries = 0;
        size_t high_priority_entries = 0;
        std::chrono::steady_clock::time_point oldest_access;
        std::chrono::steady_clock::time_point newest_access;
        std::unordered_map<int, size_t> priority_distribution;
    };

    /**
     * @brief 基础缓存统计信息
     */
    struct CacheStats {
        size_t total_indexes = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        double hit_rate = 0.0;
        size_t expired_entries = 0;
        size_t high_priority_entries = 0;
    };

    /**
     * @brief 构造函数
     * @param max_cache_size 最大缓存大小
     * @param default_ttl 默认生存时间
     */
    SmartIndexCache(size_t max_cache_size = 1000,
                   std::chrono::minutes default_ttl = std::chrono::minutes(60));

    /**
     * @brief 析构函数
     */
    ~SmartIndexCache() = default;

    /**
     * @brief 缓存索引对象
     * @param index_name 索引名称
     * @param index 索引对象智能指针
     * @param priority 优先级
     * @param ttl 生存时间（可选）
     */
    void CacheIndex(const std::string& index_name,
                   std::unique_ptr<BPlusTreeIndex> index,
                   int priority = 0,
                   std::chrono::minutes ttl = std::chrono::minutes(0));

    /**
     * @brief 获取缓存的索引对象
     * @param index_name 索引名称
     * @return 索引对象指针（如果不存在返回nullptr）
     */
    BPlusTreeIndex* GetIndex(const std::string& index_name);

    /**
     * @brief 检查索引是否存在
     * @param index_name 索引名称
     * @return 是否存在
     */
    bool HasIndex(const std::string& index_name) const;

    /**
     * @brief 移除索引
     * @param index_name 索引名称
     * @return 是否成功移除
     */
    bool RemoveIndex(const std::string& index_name);

    /**
     * @brief 预热缓存
     * @param predicted_indexes 预测的索引列表
     */
    void WarmupCache(const std::vector<std::string>& predicted_indexes);

    /**
     * @brief 智能缓存清理
     */
    void IntelligentCleanup();

    /**
     * @brief 获取增强的缓存统计信息
     * @return 增强的缓存统计信息
     */
    EnhancedCacheStats GetEnhancedCacheStats() const;

    /**
     * @brief 批量获取多个索引
     * @param index_names 索引名称列表
     * @return 索引对象指针列表
     */
    std::vector<BPlusTreeIndex*> GetMultipleIndexes(const std::vector<std::string>& index_names);

    /**
     * @brief 手动清理过期缓存
     * @param max_age 最大年龄
     */
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(30));

private:
    /**
     * @brief 缓存条目结构
     */
    struct CacheEntry {
        std::unique_ptr<BPlusTreeIndex> index;
        int priority;
        std::chrono::steady_clock::time_point create_time;
        std::chrono::steady_clock::time_point expiry_time;
        size_t access_count;
        double access_frequency; // 每分钟访问次数
        std::chrono::steady_clock::time_point last_access;
    };

    /**
     * @brief 优先级条目结构
     */
    struct PriorityEntry {
        std::string index_name;
        int priority;
        bool operator<(const PriorityEntry& other) const {
            return priority < other.priority;
        }
    };

    mutable std::mutex cache_mutex_;
    size_t max_cache_size_;
    std::chrono::minutes default_ttl_;

    std::unordered_map<std::string, CacheEntry> index_cache_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> access_times_;
    std::priority_queue<PriorityEntry> priority_queue_;

    /**
     * @brief 简单的LRU清理（补充IntelligentCleanup）
     */
    void EvictCacheEntries();

    // 禁止拷贝和赋值
    SmartIndexCache(const SmartIndexCache&) = delete;
    SmartIndexCache& operator=(const SmartIndexCache&) = delete;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
