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
 * @brief B+树索引实现类
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
     * @brief 插入键值对
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
     * @brief 搜索指定键
     * @param key 要搜索的键
     * @return 匹配的索引条目列表
     */
    std::vector<IndexEntry> Search(const std::string& key) const;

    /**
     * @brief 范围搜索
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
