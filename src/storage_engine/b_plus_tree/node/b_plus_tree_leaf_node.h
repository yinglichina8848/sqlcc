#pragma once

#include "src/storage_engine/b_plus_tree/node/b_plus_tree_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * WHY: 为什么需要B+树叶子节点？
 *
 * B+树索引系统需要专门的叶子节点来存储实际数据：
 * - 叶子节点存储完整的键值对和数据位置指针
 * - 叶子节点间通过链表连接，支持高效范围查询
 * - 所有实际数据都存储在叶子层，保证查询一致性
 * - 叶子节点是索引与数据存储的桥梁层
 *
 * 叶子节点的核心价值：
 * 1. 数据存储：保存索引键和对应的数据位置
 * 2. 范围查询：链表结构支持有序遍历
 * 3. 负载均衡：分裂合并保证节点大小均匀
 * 4. 持久化支持：叶子数据需要可靠的磁盘存储
 *
 * 🏗️ 设计模式：组合模式(Composite Pattern)
 *
 * 叶子节点作为B+树结构的一部分：
 * - 继承节点基类：统一接口和基础功能
 * - 实现叶子特化：数据存储和链表维护
 * - 参与树操作：分裂合并的叶子节点逻辑
 * - 支持遍历访问：范围查询的有序访问
 *
 * 叶子特化的设计考虑：
 * - 数据密度：最大化利用磁盘页面空间
 * - 访问模式：顺序访问和随机访问的平衡
 * - 并发控制：多线程环境的数据安全
 * - 缓存策略：热点数据的内存缓存
 *
 * WHAT: B+树叶子节点 - 数据存储的核心组件
 *
 * 核心功能：
 * - 数据条目管理：存储键值对和位置信息
 * - 链表维护：与相邻叶子节点的双向连接
 * - 插入删除操作：维护有序性的数据操作
 * - 范围查询支持：高效的范围数据检索
 * - 分裂合并处理：节点大小的动态调整
 *
 * 数据结构设计：
 * - 条目数组：有序存储的索引条目集合
 * - 链表指针：前后叶子节点的页面链接
 * - 页面绑定：与磁盘页面的直接映射
 * - 元数据管理：节点状态和边界信息
 *
 * 接口设计：
 * - 数据操作：Insert/Remove/Search的基本CRUD
 * - 范围查询：SearchRange支持区间检索
 * - 结构维护：Split/Merge处理节点重组
 * - 状态查询：IsFull等状态检查方法
 * - 调试接口：内部状态的访问方法
 *
 * HOW: B+树叶子节点的实现机制和优化策略
 *
 * 数据存储架构：
 * - 条目格式：键值+页ID+偏移量的紧凑结构
 * - 排序维护：插入时保持键值的有序性
 * - 二分查找：O(log n)的快速键值定位
 * - 批量操作：支持批量插入提高效率
 *
 * 链表管理机制：
 * - 双向链接：前向和后向指针维护链式结构
 * - 范围遍历：链表支持顺序扫描范围数据
 * - 并发安全：链表操作的原子性和一致性
 * - 断链处理：节点分裂合并时的链式调整
 *
 * 插入算法优化：
 * 1. 查找位置：二分查找确定插入点
 * 2. 空间检查：验证节点是否有足够空间
 * 3. 插入操作：维护有序性的条目插入
 * 4. 分裂处理：节点满载时的智能分裂
 * 5. 链表更新：分裂后链式结构的调整
 *
 * 删除算法优化：
 * 1. 定位条目：快速查找要删除的键值
 * 2. 移除操作：有序数组中的条目删除
 * 3. 合并检查：节点过少时触发合并操作
 * 4. 重平衡载：合并后数据的重新分布
 * 5. 链式修复：合并后链表结构的修复
 *
 * 范围查询优化：
 * 1. 起始定位：快速找到范围起始位置
 * 2. 链表遍历：利用叶子链进行顺序扫描
 * 3. 边界控制：精确的上下界范围限制
 * 4. 预取优化：异步预读提高查询性能
 * 5. 缓存加速：热点范围数据的缓存策略
 *
 * 分裂策略优化：
 * - 中间分裂：数据均匀分布到两个节点
 * - 键值选择：选择合适的分割键值
 * - 父节点更新：向上传播分裂信息
 * - 链表重连：维护叶子节点的链式结构
 * - 空间预留：为未来插入预留空间
 *
 * 合并策略优化：
 * - 兄弟节点检查：相邻节点的合并条件
 * - 数据重分布：合并后的有序性维护
 * - 父节点调整：向上层传播合并信息
 * - 链式修复：合并后的链表结构重建
 * - 空间回收：释放不需要的页面空间
 *
 * 内存管理优化：
 * - 对象池复用：减少节点对象的分配开销
 * - 引用计数：智能指针管理节点生命周期
 * - 延迟加载：按需加载叶子节点数据
 * - 缓存策略：LRU淘汰的节点缓存机制
 *
 * 并发控制策略：
 * - 读写锁分离：读操作共享锁，写操作独占锁
 * - 乐观并发：版本控制的无锁读取
 * - 细粒度锁：只锁定操作涉及的条目范围
 * - 死锁避免：固定的节点访问顺序
 * - MVCC支持：多版本并发控制
 *
 * 磁盘I/O优化：
 * - 页面对齐：磁盘访问的页面边界对齐
 * - 预写日志：WAL保证数据一致性
 * - 异步刷盘：后台的页面写入操作
 * - 批量提交：减少磁盘同步次数
 * - RAID友好：适合RAID的I/O模式
 *
 * 故障恢复机制：
 * - WAL重放：崩溃后通过日志恢复数据
 * - 一致性检查：启动时的叶子节点验证
 * - 自动修复：检测并修复损坏的叶子数据
 * - 备份恢复：冷热备份的数据保护
 * - 版本控制：多版本数据的故障回退
 *
 * 性能监控和调优：
 * - 访问统计：查询频率和命中率的监控
 * - 空间利用率：节点存储效率的分析
 * - 分裂合并频率：结构调整成本的评估
 * - 缓存效果：内存缓存的命中率分析
 * - I/O性能：磁盘访问模式的优化建议
 *
 * 扩展性和兼容性：
 * - 插件架构：支持自定义的叶子节点实现
 * - 配置管理：可配置的节点大小和参数
 * - 多格式支持：不同数据类型的存储格式
 * - 向后兼容：旧版本数据的平滑迁移
 * - API抽象：统一的叶子节点操作接口
 */
class BPlusTreeLeafNode : public BPlusTreeNode {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎指针，用于磁盘操作
     * @param page_id 叶子节点对应的磁盘页面ID
     */
    BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);

    /**
     * @brief 析构函数
     * @details 释放叶子节点占用的资源，清理链表引用
     */
    ~BPlusTreeLeafNode() override;

    // 实现基类纯虚函数
    void SerializeToPage() override;
    void DeserializeFromPage() override;
    void Clear() override;
    bool IsFull() const override;

    // 叶子节点特有的数据操作方法
    bool Insert(const IndexEntry& entry);
    bool Remove(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

    // 分裂和合并操作（叶子节点特化）
    void Split(std::unique_ptr<BPlusTreeLeafNode>& new_node);
    void Merge(std::unique_ptr<BPlusTreeLeafNode> right_node);

    // 叶子节点链表管理方法
    int32_t GetNextPageId() const { return next_page_id_; }
    void SetNextPageId(int32_t page_id) { next_page_id_ = page_id; }

    // 调试和测试辅助方法
    const std::vector<IndexEntry>& GetEntries() const { return entries_; }

private:
    std::vector<IndexEntry> entries_;  /**< 有序存储的索引条目集合 */
    int32_t next_page_id_;             /**< 下一个叶子节点的页面ID，用于链表维护 */
};

} // namespace sqlcc
