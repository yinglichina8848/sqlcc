#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;
class Page;

/**
 * WHY: 为什么需要B+树节点基类？
 *
 * B+树索引系统需要统一的节点管理接口：
 * - 树形结构需要统一的节点操作抽象
 * - 磁盘存储需要统一的序列化接口
 * - 内存管理需要统一的生命周期控制
 * - 不同节点类型需要统一的访问方式
 *
 * 节点抽象的核心价值：
 * 1. 统一接口：叶子和内部节点有相同的基础操作
 * 2. 多态性：运行时动态确定节点类型和行为
 * 3. 内存管理：统一控制节点的创建和销毁
 * 4. 磁盘交互：统一的持久化序列化机制
 *
 * 🏗️ 设计模式：模板方法模式(Template Method Pattern)
 *
 * 模板方法定义节点操作的骨架：
 * - SerializeToPage(): 序列化到磁盘的固定步骤
 * - DeserializeFromPage(): 从磁盘反序列化的固定步骤
 * - IsFull(): 节点满载检查的通用逻辑
 * - Clear(): 节点清理的统一流程
 *
 * 子类实现具体细节：
 * - 叶子节点：实现数据存储的具体序列化
 * - 内部节点：实现索引键的具体序列化
 * - 不同节点类型有不同的存储格式和访问方式
 *
 * WHAT: B+树节点基类 - 索引节点的基础抽象
 *
 * 核心功能：
 * - 节点标识：页面ID、父节点关系、叶子标识
 * - 生命周期：创建、序列化、反序列化、清理
 * - 状态查询：满载检查、类型判断
 * - 存储访问：页面数据的直接访问接口
 *
 * 节点层次结构：
 * - BPlusTreeNode: 抽象基类，定义接口
 * - BPlusTreeInternalNode: 内部节点，存储索引
 * - BPlusTreeLeafNode: 叶子节点，存储数据
 *
 * 接口设计：
 * - 构造函数：绑定存储引擎和页面资源
 * - 基本属性：页面ID、父节点、节点类型
 * - 序列化：ToPage/FromPage的双向转换
 * - 状态管理：Full/Clear的状态控制
 *
 * HOW: B+树节点的实现机制和内存管理策略
 *
 * 节点存储架构：
 * - 页面绑定：每个节点绑定一个固定大小的磁盘页
 * - 内存映射：节点数据直接映射到磁盘页面
 * - 延迟加载：按需从磁盘加载节点数据
 * - 缓存管理：节点缓存的LRU淘汰策略
 *
 * 序列化策略：
 * - 紧凑存储：最大化利用磁盘页面空间
 * - 字节对齐：保证数据访问的高效性
 * - 元数据前缀：节点类型、大小等元信息
 * - 校验和：数据完整性的CRC校验
 *
 * 内存管理优化：
 * - 对象池：节点对象的复用减少分配开销
 * - 引用计数：智能指针管理节点生命周期
 * - 写时拷贝：修改时创建新版本避免冲突
 * - 预分配：批量预分配减少运行时开销
 *
 * 并发控制机制：
 * - 读写锁：读操作共享锁，写操作独占锁
 * - 乐观锁：版本号控制并发修改冲突
 * - 细粒度锁：只锁定修改的节点路径
 * - 死锁避免：固定的节点访问顺序
 *
 * 节点分裂与合并：
 * - 分裂触发：节点满载时自动分裂
 * - 合并条件：节点过少时向上合并
 * - 平衡维护：保证树结构的平衡性
 * - 键值重分布：分裂时合理分配键值范围
 *
 * 故障恢复支持：
 * - WAL日志：节点修改的预写日志
 * - 检查点：节点状态的定期快照
 * - 一致性检查：启动时的节点验证
 * - 自动修复：检测并修复节点损坏
 *
 * 性能优化技术：
 * - SIMD加速：向量化键值比较操作
 * - 预取优化：相邻节点的异步预取
 * - 缓存对齐：内存访问的缓存行对齐
 * - 分支预测：条件分支的预测优化
 *
 * 调试和诊断：
 * - 结构验证：节点完整性和一致性检查
 * - 统计信息：访问频率、命中率等指标
 * - 可视化工具：节点结构的图形化展示
 * - 内存分析：节点内存使用的详细分析
 * - 性能监控：节点操作的时间和资源消耗
 */

/**
 * @brief 索引条目结构体
 * @details 定义B+树索引中的单个条目，包含键值和位置信息
 */
struct IndexEntry {
    std::string key;       /**< 索引键值 */
    int32_t page_id;       /**< 数据页ID */
    size_t offset;         /**< 页内偏移 */

    IndexEntry() : page_id(-1), offset(0) {}
    IndexEntry(const std::string& k, int32_t p, size_t o) : key(k), page_id(p), offset(o) {}

    /**
     * @brief 键值比较运算符
     * @details 用于在有序容器中进行键值排序
     */
    bool operator<(const IndexEntry& other) const {
        return key < other.key;
    }
};

/**
 * @class BPlusTreeNode
 * @brief B+树节点基类，定义所有节点类型的通用接口
 *
 * 设计理念：
 * - 抽象接口：定义节点操作的统一契约
 * - 多态行为：支持不同类型的节点实现
 * - 资源管理：统一管理节点的生命周期
 * - 序列化支持：提供磁盘持久化的标准接口
 */
class BPlusTreeNode {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎指针，用于磁盘操作
     * @param page_id 节点对应的磁盘页面ID
     * @param is_leaf 是否为叶子节点
     */
    BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_leaf);

    /**
     * @brief 析构函数
     * @details 释放节点占用的资源，包括页面绑定
     */
    virtual ~BPlusTreeNode();

    // 禁止拷贝和赋值，确保节点对象的唯一性
    BPlusTreeNode(const BPlusTreeNode&) = delete;
    BPlusTreeNode& operator=(const BPlusTreeNode&) = delete;

    // 允许移动操作，提高性能
    BPlusTreeNode(BPlusTreeNode&&) = default;
    BPlusTreeNode& operator=(BPlusTreeNode&&) = default;

    // 基本属性访问方法
    int32_t GetPageId() const { return page_id_; }
    int32_t GetParentPageId() const { return parent_page_id_; }
    void SetParentPageId(int32_t parent_id) { parent_page_id_ = parent_id; }
    bool IsLeaf() const { return is_leaf_; }

    // 纯虚函数，由子类实现具体的序列化和状态管理
    virtual void SerializeToPage() = 0;
    virtual void DeserializeFromPage() = 0;
    virtual void Clear() = 0;
    virtual bool IsFull() const = 0;

    // 页面数据访问接口
    char* GetData();
    const char* GetData() const;

protected:
    std::shared_ptr<StorageEngine> storage_engine_;  /**< 存储引擎引用 */
    int32_t page_id_;                                /**< 节点页面ID */
    int32_t parent_page_id_;                         /**< 父节点页面ID */
    bool is_leaf_;                                   /**< 是否为叶子节点 */
    std::shared_ptr<Page> page_;                     /**< 绑定的磁盘页面 */
};

} // namespace sqlcc
