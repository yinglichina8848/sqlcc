#ifndef SQLCC_B_PLUS_TREE_H
#define SQLCC_B_PLUS_TREE_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "storage/b_plus_tree_nodes.h"

// 前向声明解决循环依赖
namespace sqlcc {
class StorageEngine;
class Page;
} // namespace sqlcc

namespace sqlcc {

/**
 * WHY: 为什么数据库索引选择B+树而不是B树或哈希表？
 *
 * 数据库索引需要支持：
 * 1. 范围查询：SELECT * FROM t WHERE id BETWEEN 1 AND 100
 * 2. 顺序访问：ORDER BY id
 * 3. 高效的插入/删除：维持平衡结构
 *
 * B+树优势：
 * - 叶子节点串联，支持高效范围查询
 * - 所有数据都在叶子节点，内部节点只存键
 * - 磁盘I/O优化：节点大小适配页面大小
 * - 高度平衡，保证查询性能稳定
 *
 * 性能特点：
 * - 查找复杂度：O(log_m N)
 * - 范围查询：O(log_m N + K)
 * - 插入/删除：O(log_m N)
 *
 * WHAT: B+树索引实现类
 *
 * B+树是一种自平衡的树数据结构，保持数据排序并允许在对数时间内进行搜索、顺序访问、插入和删除。
 * 在B+树中，所有记录都存储在叶节点中，内部节点只存储键值用于导航。
 */
class BPlusTreeIndex {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎智能指针
     * @param table_name 表名
     * @param column_name 列名
     */
    BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine, const std::string& table_name, const std::string& column_name);

    /**
     * @brief 析构函数
     */
    ~BPlusTreeIndex();

    /**
     * @brief 创建索引
     * @return 是否创建成功
     */
    bool Create();

    /**
     * @brief 删除索引
     * @return 是否删除成功
     */
    bool Drop();

    /**
     * WHAT: Insert - B+树插入操作
     *
     * 插入键值对到B+树索引中，维护树的平衡性。
     * 如果节点溢出，会触发分裂操作。
     *
     * HOW: 递归插入算法
     * 1. 从根节点开始，递归查找插入位置
     * 2. 在叶子节点找到插入点，插入键值对
     * 3. 如果叶子节点溢出（超过最大键数），执行分裂：
     *    - 创建新叶子节点
     *    - 将键值对平均分配到两个节点
     *    - 在父节点插入中间键和新节点指针
     * 4. 如果父节点也溢出，递归向上分裂
     * 5. 如果根节点分裂，树高度增加
     *
     * @param key 键
     * @param page_id 页面ID
     * @param offset 偏移量
     * @return 是否插入成功
     */
    bool Insert(const std::string& key, int32_t page_id, size_t offset);

    /**
     * @brief 删除键
     * @param key 键
     * @return 是否删除成功
     */
    bool Delete(const std::string& key);

    /**
     * @brief 查找键
     * @param key 键
     * @param page_id 输出参数：页面ID
     * @param offset 输出参数：偏移量
     * @return 是否找到
     */
    bool Lookup(const std::string& key, int32_t& page_id, size_t& offset) const;

    /**
     * @brief 范围查找
     * @param start_key 起始键
     * @param end_key 结束键
     * @return 查找结果列表
     */
    std::vector<std::pair<int32_t, size_t>> RangeLookup(const std::string& start_key, const std::string& end_key) const;

    /**
     * WHAT: Search - 多路查找和叶子节点搜索
     *
     * 从根节点开始，沿着树结构向下查找指定的键。
     * 在叶子节点中找到匹配的键值对。
     *
     * HOW: 多路查找算法
     * 1. 从根节点开始
     * 2. 在当前节点中进行二分查找，确定子节点索引
     * 3. 递归进入相应的子节点
     * 4. 重复步骤2-3直到到达叶子节点
     * 5. 在叶子节点中顺序查找匹配的键
     * 6. 返回所有匹配的索引条目
     *
     * @param key 要搜索的键
     * @return 匹配的索引条目列表
     */
    std::vector<IndexEntry> Search(const std::string& key) const;

    /**
     * WHAT: SearchRange - 范围查询和叶子链表遍历
     *
     * 执行范围查询，从lower_bound到upper_bound之间的所有键。
     * 利用B+树叶子节点链表的特性，实现高效的范围遍历。
     *
     * HOW: 范围查询算法
     * 1. 找到范围起始位置：Search(lower_bound)
     * 2. 从起始叶子节点开始，沿叶子链表向右遍历
     * 3. 收集所有在[lower_bound, upper_bound]范围内的键值对
     * 4. 当遇到超出upper_bound的键或到达链表末尾时停止
     * 5. 返回所有匹配的索引条目
     *
     * 性能优势：
     * - 一次查找定位起始点：O(log N)
     * - 顺序遍历匹配范围：O(K)
     * - 总复杂度：O(log N + K)
     *
     * @param lower_bound 范围下界
     * @param upper_bound 范围上界
     * @return 匹配的索引条目列表
     */
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

    /**
     * @brief 获取表名
     * @return 表名
     */
    const std::string& GetTableName() const { return table_name_; }

    /**
     * @brief 获取列名
     * @return 列名
     */
    const std::string& GetColumnName() const { return column_name_; }

    /**
     * @brief 检查索引是否存在
     * @return 索引是否存在
     */
    bool Exists() const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;  ///< 存储引擎智能指针
    std::string table_name_;         ///< 表名
    std::string column_name_;        ///< 列名
    int32_t root_page_id_;           ///< 根节点页面ID
    mutable std::shared_ptr<Page> root_page_; ///< 根节点页面（缓存）

    // 内部辅助方法
    std::unique_ptr<BPlusTreeNode> LoadNode(int32_t page_id);
    void SaveNode(std::shared_ptr<Page> page) const;
    std::unique_ptr<BPlusTreeNode> GetNode(int32_t page_id) const;
    std::unique_ptr<BPlusTreeNode> CreateNewNode(bool is_leaf);
    void DeleteNode(int32_t page_id);
    bool NeedMerge(const std::unique_ptr<BPlusTreeNode>& node);
    void LoadMetadata();
    void SaveMetadata();

public:  // 添加公共接口用于查询优化
    
    // 递归操作方法
    bool Insert(const std::string& key, int32_t page_id, size_t offset, std::unique_ptr<BPlusTreeNode>& node, int recursion_depth = 0);
    bool Delete(const std::string& key, std::unique_ptr<BPlusTreeNode>& node);
    bool Lookup(const std::string& key, int32_t& page_id, size_t& offset, std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<IndexEntry> Search(const std::string& key, std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound, std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<std::pair<int32_t, size_t>> RangeLookup(const std::string& start_key, const std::string& end_key, std::unique_ptr<BPlusTreeNode>& node) const;

    // 节点操作方法
    bool IsLeafNode(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<std::string> GetKeys(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<std::pair<int32_t, size_t>> GetValues(std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<int32_t> GetChildren(std::unique_ptr<BPlusTreeNode>& node) const;
};

} // namespace sqlcc

#endif // SQLCC_B_PLUS_TREE_H
