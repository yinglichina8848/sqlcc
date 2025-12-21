#pragma once

#include "storage_engine/b_plus_tree_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class BPlusTreeLeafNode : public BPlusTreeNode {
public:
    BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);
    ~BPlusTreeLeafNode() override;

    // 实现纯虚函数
    void SerializeToPage() override;
    void DeserializeFromPage() override;
    void Clear() override;
    bool IsFull() const override;

    // 叶子节点特有的操作
    bool Insert(const IndexEntry& entry);
    bool Remove(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

    // 分裂和合并操作
    void Split(std::unique_ptr<BPlusTreeLeafNode>& new_node);
    void Merge(std::unique_ptr<BPlusTreeLeafNode> right_node);

    // 叶子节点链操作
    int32_t GetNextPageId() const { return next_page_id_; }
    void SetNextPageId(int32_t page_id) { next_page_id_ = page_id; }

    // 获取内部状态（用于调试和测试）
    const std::vector<IndexEntry>& GetEntries() const { return entries_; }

private:
    std::vector<IndexEntry> entries_;  // 索引条目
    int32_t next_page_id_;             // 下一个叶子节点的页面ID
};

} // namespace sqlcc
