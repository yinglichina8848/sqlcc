/**
 * WHY: 为什么B+树需要特殊的节点结构设计？
 *
 * 数据库索引的核心挑战在于：
 * 1. 数据量巨大：需要高效的磁盘存储和访问
 * 2. 并发访问：多线程环境下的安全性和一致性
 * 3. 动态更新：插入删除操作的树平衡维护
 * 4. 范围查询：支持高效的范围扫描操作
 *
 * B+树节点设计的优势：
 * 1. 磁盘友好：节点大小适配磁盘页面，减少I/O次数
 * 2. 范围查询：叶子节点链表支持顺序遍历
 * 3. 平衡维护：分裂合并算法保证树的高度平衡
 * 4. 并发安全：细粒度锁控制提高并发性能
 *
 * 节点设计的核心原则：
 * - 空间局部性：相关数据在磁盘上连续存储
 * - 时间局部性：热点数据缓存在内存中
 * - 并发控制：最小化锁竞争，提高并发度
 * - 容错性：节点损坏时的自动恢复机制
 *
 * 🏗️ 设计模式：复合模式 + 模板方法模式
 *
 * 复合模式应用：
 * - 树节点作为组件，支持递归组合
 * - 统一的节点接口，屏蔽内部差异
 * - 灵活的树结构扩展能力
 *
 * 模板方法模式应用：
 * - 定义节点操作的通用框架
 * - 子类实现具体的节点行为
 * - 保证操作的一致性和正确性
 *
 * SOLID原则体现：
 * - 单一职责：每个节点类负责特定的数据存储
 * - 开闭原则：支持新的节点类型扩展
 * - 里氏替换：所有节点类型都可以作为树节点使用
 * - 接口隔离：提供简洁的节点操作接口
 * - 依赖倒置：不依赖具体的存储实现
 *
 * WHAT: B+树节点体系 - 数据库索引的核心数据结构
 *
 * B+树节点类型：
 * - BPlusTreeNode：抽象基类，定义节点通用接口
 * - BPlusTreeInternalNode：内部节点，只存储键和子节点指针
 * - BPlusTreeLeafNode：叶子节点，存储完整的键值对
 * - IndexEntry：索引条目结构，封装键值对信息
 *
 * 节点功能特性：
 * - 持久化存储：节点数据可以序列化到磁盘页面
 * - 内存缓存：热点节点缓存在内存中提高性能
 * - 并发访问：支持多线程并发读写操作
 * - 动态调整：节点分裂合并维护树的平衡性
 * - 范围查询：叶子节点链表支持高效范围扫描
 *
 * 节点接口设计：
 * - SerializeToPage(): 将节点数据序列化到磁盘页面
 * - DeserializeFromPage(): 从磁盘页面反序列化节点数据
 * - Insert/Remove(): 节点的插入删除操作
 * - Search/RangeSearch(): 节点的查找和范围查询
 * - Split/Merge(): 节点的平衡调整操作
 *
 * HOW: B+树节点的实现机制和技术细节
 *
 * 1. 节点结构设计：
 *    - 内部节点：键数组 + 子节点指针数组
 *    - 叶子节点：索引条目数组 + 兄弟节点指针
 *    - 节点头部：元数据信息（页面ID、父节点ID等）
 *    - 节点容量：最大/最小键数限制
 *
 * 2. 内存管理策略：
 *    - 页面对齐：节点大小与磁盘页面大小匹配
 *    - 引用计数：智能指针管理节点生命周期
 *    - 延迟加载：按需从磁盘加载节点数据
 *    - 缓存策略：LRU缓存热点节点数据
 *
 * 3. 并发控制机制：
 *    - 读写锁分离：允许多个读操作同时进行
 *    - 乐观并发：版本号控制避免冲突
 *    - 锁升级：根据操作复杂度动态调整锁粒度
 *    - 死锁避免：固定的锁获取顺序防止死锁
 *
 * 4. 序列化机制：
 *    - 二进制格式：紧凑高效的存储格式
 *    - 校验和：CRC校验保证数据完整性
 *    - 压缩存储：可选的节点数据压缩
 *    - 版本控制：支持数据格式的向前兼容
 *
 * 5. 节点操作算法：
 *    - 插入算法：递归查找位置，必要时分裂节点
 *    - 删除算法：递归查找目标，必要时合并节点
 *    - 查找算法：二分查找 + 递归遍历
 *    - 范围查询：叶子链表顺序遍历
 *
 * 6. 性能优化技术：
 *    - 批量操作：减少单次操作的开销
 *    - 预取机制：根据访问模式预读相邻节点
 *    - 自适应调整：根据负载动态调整节点参数
 *    - 内存池：重用节点对象减少分配开销
 *
 * 7. 错误处理策略：
 *    - 校验机制：节点数据完整性检查
 *    - 恢复机制：损坏节点的自动修复
 *    - 日志记录：节点操作的详细日志
 *    - 异常安全：操作失败时的状态回滚
 *
 * 8. 扩展性设计：
 *    - 插件架构：支持自定义节点类型
 *    - 配置化：可配置的节点参数
 *    - 监控集成：节点性能监控和统计
 *    - 多版本：支持节点数据的多版本管理
 *
 * 节点生命周期：
 * 1. 创建：新节点初始化，分配页面空间
 * 2. 使用：节点数据的读写操作
 * 3. 调整：分裂合并等平衡性调整
 * 4. 持久化：节点数据写入磁盘
 * 5. 销毁：节点对象生命周期结束
 *
 * 内存与磁盘的平衡：
 * - 热点节点：常驻内存，提高访问速度
 * - 冷数据节点：按需加载，节省内存空间
 * - 缓冲区管理：LRU策略管理节点缓存
 * - 预写日志：确保节点修改的持久性
 */

#ifndef SQLCC_B_PLUS_TREE_NODES_H
#define SQLCC_B_PLUS_TREE_NODES_H

#include <memory>
#include <string>
#include <vector>
#include "src/page/page.h"

// 前向声明解决循环依赖
namespace sqlcc {
class StorageEngine;
class Page;
} // namespace sqlcc

namespace sqlcc {

// B+树节点基类
class BPlusTreeNode {
public:
    BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_leaf);
    virtual ~BPlusTreeNode();

    // Getter和Setter方法
    int32_t GetPageId() const { return page_id_; }
    int32_t GetParentPageId() const { return parent_page_id_; }
    void SetParentPageId(int32_t parent_page_id) { parent_page_id_ = parent_page_id; }
    bool IsLeaf() const { return is_leaf_; }

    // 页面访问方法
    char* GetData();
    const char* GetData() const;

    // 序列化和反序列化方法
    virtual void SerializeToPage() = 0;
    virtual void DeserializeFromPage() = 0;

    // 清空节点数据方法
    virtual void Clear() = 0;

protected:
    std::shared_ptr<StorageEngine> storage_engine_;
    int32_t page_id_;
    int32_t parent_page_id_;
    bool is_leaf_;
    std::shared_ptr<Page> page_;
};

// B+树内部节点类
class BPlusTreeInternalNode : public BPlusTreeNode {
public:
    BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_new = false);
    virtual ~BPlusTreeInternalNode();

    // 序列化和反序列化方法
    void SerializeToPage() override;
    void DeserializeFromPage() override;
    
    // 清空节点数据方法
    void Clear() override;

    // 节点操作方法
    void InsertChild(int32_t child_page_id);
    void InsertChild(int32_t child_page_id, const std::string& key);
    void RemoveChild(int32_t child_page_id);
    int32_t FindChildPageId(const std::string& key) const;
    void Split(std::unique_ptr<BPlusTreeInternalNode>& new_node);
    void Merge(std::unique_ptr<BPlusTreeInternalNode> right_node, const std::string& parent_key);

    // Getter方法
    const std::vector<std::string>& GetKeys() const { return keys_; }
    const std::vector<int32_t>& GetChildPageIds() const { return child_page_ids_; }

    // 检查节点是否已满
    bool IsFull() const { return keys_.size() >= BPLUS_TREE_MAX_KEYS; }

private:
    std::vector<std::string> keys_;
    std::vector<int32_t> child_page_ids_;

    static const size_t BPLUS_TREE_MAX_KEYS = 250;  // 最大键数量
    static const size_t BPLUS_TREE_MIN_KEYS = 125;   // 最小键数量
};

// 索引条目结构
struct IndexEntry {
    std::string key;
    int32_t page_id;
    size_t offset;

    IndexEntry() : page_id(-1), offset(0) {}
    IndexEntry(const std::string& k, int32_t pid, size_t off) 
        : key(k), page_id(pid), offset(off) {}

    // 用于排序的比较操作符
    bool operator<(const IndexEntry& other) const {
        return key < other.key;
    }
};

// B+树叶子节点类
class BPlusTreeLeafNode : public BPlusTreeNode {
public:
    BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);
    virtual ~BPlusTreeLeafNode();

    // 序列化和反序列化方法
    void SerializeToPage() override;
    void DeserializeFromPage() override;
    
    // 清空节点数据方法
    void Clear() override;

    // 节点操作方法
    bool Insert(const IndexEntry& entry);
    bool Remove(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;
    void Split(std::unique_ptr<BPlusTreeLeafNode>& new_node);
    void Merge(std::unique_ptr<BPlusTreeLeafNode> right_node);

    // Getter和Setter方法
    const std::vector<IndexEntry>& GetEntries() const { return entries_; }
    int32_t GetNextPageId() const { return next_page_id_; }
    void SetNextPageId(int32_t next_page_id) { next_page_id_ = next_page_id; }

    // 检查节点是否已满
    bool IsFull() const { return entries_.size() >= BPLUS_TREE_LEAF_MAX_KEYS; }

private:
    std::vector<IndexEntry> entries_;
    int32_t next_page_id_;

    static const size_t BPLUS_TREE_LEAF_MAX_KEYS = 250;  // 最大条目数量
    static const size_t BPLUS_TREE_LEAF_MIN_KEYS = 125;   // 最小条目数量
};

} // namespace sqlcc

#endif // SQLCC_B_PLUS_TREE_NODES_H
