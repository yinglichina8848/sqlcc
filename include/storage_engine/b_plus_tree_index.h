#pragma once

#include "storage_engine/b_plus_tree_node.h"
#include "storage_engine/b_plus_tree_leaf_node.h"
#include "storage_engine/b_plus_tree_internal_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;

/**
 * WHY: 为什么需要B+树索引？
 *
 * 数据库查询性能严重依赖索引效率：
 * - 全表扫描在大数据集上性能灾难性：O(n)时间复杂度
 * - 传统索引（如哈希表）不支持范围查询：只能精确匹配
 * - 二叉树索引在磁盘I/O上效率低下：树高导致多次随机I/O
 * - 平衡树虽好但空间利用率低：叶子节点到根的路径冗余
 *
 * B+树的核心优势：
 * 1. 磁盘友好：节点大小匹配磁盘页，减少I/O次数
 * 2. 范围查询：叶子节点链表支持高效范围扫描
 * 3. 空间效率：所有数据只在叶子节点，内部节点仅索引
 * 4. 自平衡：插入删除自动维护平衡，保证查询性能
 * 5. 多路分支：高扇出降低树高，减少磁盘访问
 *
 * 🏗️ 设计模式：B+树索引架构设计
 *
 * 设计模式应用：
 * 1. 组合模式(Composite Pattern)：树结构层次
 * 2. 工厂模式(Factory Pattern)：节点创建
 * 3. 策略模式(Strategy Pattern)：分裂策略
 * 4. 迭代器模式(Iterator Pattern)：范围查询
 *
 * WHAT: B+树索引 - 数据库的核心索引结构
 *
 * 核心功能：
 * - 索引生命周期管理：创建、删除、维护索引结构
 * - 数据操作：插入、删除、查找索引条目
 * - 范围查询：支持键值范围的快速查找
 * - 自动平衡：插入删除时自动维护树平衡
 * - 并发控制：多线程环境下的安全操作
 * - 故障恢复：崩溃后索引结构的恢复
 *
 * HOW: B+树索引的实现机制和优化策略
 *
 * 树结构设计：
 * - 根节点：树的入口点，可为叶子或内部节点
 * - 内部节点：只存储索引键和子节点指针
 * - 叶子节点：存储完整的键值对和数据指针
 * - 节点大小：匹配磁盘页大小(4KB/8KB)
 * - 键值范围：每个节点维护键值的最小最大值
 *
 * 插入算法优化：
 * 1. 查找插入位置：从根开始遍历到合适叶子
 * 2. 叶子节点插入：找到位置后插入键值对
 * 3. 分裂处理：节点满时触发分裂操作
 * 4. 上溢传播：递归向上处理内部节点分裂
 * 5. 根分裂：根节点分裂时树高增加
 *
 * 删除算法优化：
 * 1. 查找删除位置：定位包含目标键的叶子
 * 2. 叶子节点删除：移除对应的键值对
 * 3. 合并处理：节点过少时触发合并操作
 * 4. 下溢传播：递归向下处理节点合并
 * 5. 根收缩：根节点只有一个子节点时收缩
 *
 * 范围查询优化：
 * 1. 起始查找：定位范围起始的叶子节点
 * 2. 链表遍历：利用叶子节点链表顺序扫描
 * 3. 边界检查：确保只返回范围内的结果
 * 4. 提前终止：遇到超出范围的值时停止
 * 5. 缓存预取：预读后续叶子节点提升性能
 */
class BPlusTreeIndex {
public:
    BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine,
                   const std::string& table_name,
                   const std::string& column_name);
    ~BPlusTreeIndex();

    // 禁止拷贝和赋值
    BPlusTreeIndex(const BPlusTreeIndex&) = delete;
    BPlusTreeIndex& operator=(const BPlusTreeIndex&) = delete;

    // 索引生命周期管理
    bool Create();
    bool Drop();
    bool Exists() const;

    // 数据操作
    bool Insert(const std::string& key, int32_t page_id, size_t offset);
    bool Delete(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound,
                                        const std::string& upper_bound) const;

private:
    // 迭代式插入方法 (替代递归实现)
    bool InsertIterative(const std::string& key, int32_t page_id, size_t offset);

    // 查找叶子节点页面ID
    int32_t FindLeafPageId(const std::string& key);

    // 分裂处理方法
    bool HandleLeafSplit(BPlusTreeLeafNode* leaf_node, int recursion_depth);
    bool HandleInternalSplit(BPlusTreeInternalNode* internal_node, BPlusTreeNode* child_node, int recursion_depth);
    bool HandleRootSplit(int32_t left_child_id, int32_t right_child_id, const std::string& split_key);

    // 更新父节点分裂信息
    bool UpdateParentForSplit(int32_t parent_page_id, int32_t left_child_id,
                             int32_t right_child_id, const std::string& split_key,
                             int recursion_depth);
    // 递归插入方法
    bool Insert(const std::string& key, int32_t page_id, size_t offset,
                std::unique_ptr<BPlusTreeNode>& node, int recursion_depth);

    // 递归删除方法
    bool Delete(const std::string& key, std::unique_ptr<BPlusTreeNode>& node);

    // 递归搜索方法
    std::vector<IndexEntry> Search(const std::string& key,
                                   std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound,
                                        const std::string& upper_bound,
                                        std::unique_ptr<BPlusTreeNode>& node) const;

    // 节点加载方法
    std::unique_ptr<BPlusTreeNode> LoadNode(int32_t page_id);

    // 成员变量
    std::shared_ptr<StorageEngine> storage_engine_;
    std::string table_name_;
    std::string column_name_;
    int32_t root_page_id_;
    std::unique_ptr<BPlusTreeNode> root_node_; // 保持根节点在内存中用于测试
};

} // namespace sqlcc
