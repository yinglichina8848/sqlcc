#pragma once

#include "storage_engine/b_plus_tree_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;

/**
 * @class BPlusTreeIndex
 * @brief B+树索引类，管理整个索引的生命周期
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
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

private:
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
};

} // namespace sqlcc
