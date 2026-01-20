#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <mutex>

namespace sqlcc {

/**
 * WHY: 为什么需要节点大小管理器？
 *
 * B+树索引性能严重依赖节点大小的合理配置：
 * - 固定节点大小无法适应不同数据特征：文本vs数字，短键vs长键
 * - 静态配置导致空间浪费或频繁分裂：无法动态适应工作负载
 * - 不同索引类型需要不同的节点容量：主键vs辅助索引，热点vs冷数据
 * - 运行时特征变化需要动态调整：数据分布变化，访问模式演化
 *
 * 节点大小管理的核心价值：
 * 1. 自适应优化：根据实际使用模式自动调整节点容量
 * 2. 空间效率：减少内部碎片，提高存储利用率
 * 3. 性能提升：减少I/O次数和CPU缓存未命中
 * 4. 运维简化：无需手动调优，系统自主学习和适应
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern)
 *
 * 节点大小管理作为策略组件：
 * - 容量计算策略：基于统计信息计算最优容量
 * - 分裂决策策略：动态判断节点是否需要分裂
 * - 缓存策略：节点数据的内存缓存和淘汰
 * - 监控策略：性能指标的收集和分析
 *
 * 策略模式的优势：
 * - 可插拔算法：不同的容量计算策略
 * - 运行时切换：根据负载特征动态调整
 * - 扩展性良好：新策略易于添加
 * - 测试友好：策略独立测试和验证
 *
 * WHAT: 节点大小管理器 - B+树的自适应优化组件
 *
 * 核心功能：
 * - 统计信息收集：节点大小和使用模式的监控
 * - 动态容量计算：基于统计数据的容量推荐
 * - 分裂决策支持：智能判断节点分裂时机
 * - 性能监控报告：节点使用效率的详细分析
 * - 配置参数管理：容量范围和阈值的可配置
 *
 * 主要接口：
 * - 记录统计：收集节点操作的数据样本
 * - 获取容量：为不同节点类型推荐容量
 * - 判断分裂：基于动态规则的分裂决策
 * - 生成报告：性能和使用情况的详细报告
 * - 重置统计：清理历史数据重新开始学习
 *
 * 数据结构设计：
 * - 统计信息：节点大小分布、访问频率等
 * - 容量配置：最小/最大/默认容量参数
 * - 采样计数：统计数据的置信度管理
 * - 线程安全：并发环境下的数据保护
 *
 * HOW: 节点大小管理的实现机制和自适应算法
 *
 * 统计数据收集架构：
 * - 采样点选择：节点分裂、合并、访问等关键点
 * - 数据聚合：统计平均值、方差、极值等指标
 * - 时间窗口：滑动窗口维护近期数据的权重
 * - 异常过滤：剔除异常值保证统计准确性
 *
 * 动态容量计算算法：
 * 1. 基准容量：根据数据类型设置基础容量
 * 2. 统计调整：基于实际大小分布调整容量
 * 3. 负载因子：考虑CPU缓存和磁盘I/O的影响
 * 4. 边界约束：确保容量在合理范围内
 * 5. 平滑过渡：避免容量剧烈变化的影响
 *
 * 分裂决策优化策略：
 * - 阈值动态调整：基于历史分裂频率调整阈值
 * - 多维度判断：综合考虑大小、条目数、访问频率
 * - 预测性分裂：提前分裂避免性能抖动
 * - 自适应阈值：学习最优的分裂触发点
 *
 * 并发控制机制：
 * - 锁分层设计：读写锁分离提高并发度
 * - 原子操作：统计计数器的原子更新
 * - 缓存一致性：多线程环境下的数据一致性
 * - 死锁避免：固定的锁获取顺序
 *
 * 性能监控和调优：
 * - 实时指标：当前容量使用率和分裂频率
 * - 历史趋势：容量调整效果的时间序列分析
 * - 异常检测：识别异常的容量使用模式
 * - 自动调优：基于反馈的自我优化能力
 *
 * 学习和适应机制：
 * - 在线学习：运行时持续收集和分析数据
 * - 反馈循环：根据性能指标调整策略参数
 * - A/B测试：新策略的灰度发布和效果验证
 * - 冷启动处理：新索引的初始容量设置
 *
 * 故障恢复和容错：
 * - 统计数据持久化：重启后恢复学习状态
 * - 异常值处理：统计计算的鲁棒性保证
 * - 降级策略：学习失败时的保守默认行为
 * - 监控告警：异常情况的及时发现和处理
 *
 * 配置管理和扩展：
 * - 参数热更新：运行时调整配置参数
 * - 多租户支持：不同索引的独立配置
 * - 插件化设计：支持自定义容量计算算法
 * - API抽象：统一的外部访问接口
 *
 * 调试和诊断功能：
 * - 详细日志：容量调整决策的详细记录
 * - 可视化工具：统计数据和趋势的图形展示
 * - 性能分析：容量策略对整体性能的影响分析
 * - 压力测试：不同负载下的容量策略验证
 */

/**
 * @brief 节点大小统计信息结构体
 * @details 收集和分析B+树节点大小分布的统计数据
 */
struct NodeSizeStats {
    size_t total_entries = 0;          /**< 总条目数 */
    size_t total_size = 0;             /**< 总大小（字节） */
    double avg_entry_size = 0.0;       /**< 平均条目大小 */
    double size_variance = 0.0;        /**< 大小方差 */
    size_t min_entry_size = 0;         /**< 最小条目大小 */
    size_t max_entry_size = 0;         /**< 最大条目大小 */
    size_t recommended_node_capacity = 0; /**< 推荐节点容量 */

    NodeSizeStats() = default;

    /**
     * @brief 更新统计信息
     * @param entry_sizes 条目大小列表
     * @details 计算平均值、方差等统计指标，并给出容量推荐
     */
    void update_stats(const std::vector<size_t>& entry_sizes);

    /**
     * @brief 计算容量推荐
     * @details 基于统计信息计算最优的节点容量
     */
    void calculate_recommendations();
};

/**
 * @class NodeSizeManager
 * @brief 动态节点大小管理器
 *
 * 负责监控和动态调整B+树节点的大小策略，
 * 基于实际使用模式优化节点容量和内存使用。
 * 通过统计学习和自适应算法，为不同类型的节点提供最优容量配置。
 *
 * 设计理念：
 * - 自适应学习：从实际运行数据中学习最优配置
 * - 动态调整：根据负载变化实时调整节点容量
 * - 性能优化：平衡空间利用率和访问效率
 * - 运维友好：减少手动调优的工作量
 */
class NodeSizeManager {
public:
    /**
     * @brief 获取单例实例
     * @return 节点大小管理器实例
     */
    static NodeSizeManager& get_instance();

    /**
     * @brief 记录节点大小信息
     * @param node_type 节点类型 ("leaf" 或 "internal")
     * @param entry_sizes 条目大小列表
     * @param current_capacity 当前节点容量
     * @details 收集节点操作的统计样本，用于后续容量优化
     */
    void record_node_stats(const std::string& node_type,
                          const std::vector<size_t>& entry_sizes,
                          size_t current_capacity);

    /**
     * @brief 获取推荐的节点容量
     * @param node_type 节点类型
     * @return 推荐的节点容量
     * @details 基于历史统计数据给出容量推荐
     */
    size_t get_recommended_capacity(const std::string& node_type) const;

    /**
     * @brief 检查节点是否已满（动态容量）
     * @param node_type 节点类型
     * @param current_entries 当前条目数
     * @param current_size 当前总大小
     * @return 是否需要分裂
     * @details 使用动态阈值判断是否需要节点分裂
     */
    bool should_split_node(const std::string& node_type,
                          size_t current_entries,
                          size_t current_size) const;

    /**
     * @brief 获取节点大小统计信息
     * @param node_type 节点类型
     * @return 统计信息
     * @details 返回指定类型节点的详细统计数据
     */
    NodeSizeStats get_stats(const std::string& node_type) const;

    /**
     * @brief 重置统计信息
     * @param node_type 节点类型
     * @details 清理历史统计数据，重新开始学习
     */
    void reset_stats(const std::string& node_type);

    /**
     * @brief 获取性能报告
     * @return 性能统计报告
     * @details 生成包含所有节点类型统计信息的详细报告
     */
    std::string get_performance_report() const;

    // 配置参数 - 容量范围约束
    static constexpr size_t MIN_CAPACITY = 4;      /**< 最小节点容量 */
    static constexpr size_t MAX_CAPACITY = 1024;   /**< 最大节点容量 */
    static constexpr size_t DEFAULT_CAPACITY = 64; /**< 默认节点容量 */
    static constexpr double SPLIT_THRESHOLD = 0.85; /**< 分裂阈值 */
    static constexpr size_t MIN_SAMPLES = 100;     /**< 最少采样次数 */

private:
    NodeSizeManager() = default;
    ~NodeSizeManager() = default;

    // 禁止拷贝和赋值，确保单例模式
    NodeSizeManager(const NodeSizeManager&) = delete;
    NodeSizeManager& operator=(const NodeSizeManager&) = delete;

    mutable std::mutex mutex_;                             /**< 线程安全保护 */
    std::unordered_map<std::string, NodeSizeStats> node_stats_;       /**< 各类型节点统计信息 */
    std::unordered_map<std::string, std::atomic<size_t>> sample_counts_; /**< 采样计数器 */

    /**
     * @brief 计算动态容量
     * @param stats 统计信息
     * @return 推荐容量
     * @details 基于统计数据计算最优节点容量
     */
    size_t calculate_dynamic_capacity(const NodeSizeStats& stats) const;

    /**
     * @brief 更新统计信息
     * @param node_type 节点类型
     * @param entry_sizes 条目大小列表
     * @details 增量更新统计数据，维护历史状态
     */
    void update_stats(const std::string& node_type, const std::vector<size_t>& entry_sizes);
};

} // namespace sqlcc
