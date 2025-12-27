#pragma once

#include "storage_engine/b_plus_tree_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class BPlusTreeInternalNode : public BPlusTreeNode {
public:
  BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_new = false);
    ~BPlusTreeInternalNode() override;

    // 实现纯虚函数
    void SerializeToPage() override;
    void DeserializeFromPage() override;
    void Clear() override;
    bool IsFull() const override;

    // 内部节点特有的操作
    void InsertChild(int32_t child_page_id);
    void InsertChild(int32_t child_page_id, const std::string& key);
    void RemoveChild(int32_t child_page_id);
    int32_t FindChildPageId(const std::string& key) const;

    // 分裂和合并操作
    void Split(std::unique_ptr<BPlusTreeInternalNode>& new_node);
    void Merge(std::unique_ptr<BPlusTreeInternalNode> right_node, const std::string& parent_key);

    // 获取内部状态（用于调试和测试）
    const std::vector<std::string>& GetKeys() const { return keys_; }
    const std::vector<int32_t>& GetChildPageIds() const { return child_page_ids_; }

private:
    std::vector<std::string> keys_;           // 分隔键
    std::vector<int32_t> child_page_ids_;     // 子节点页面ID
};

} // namespace sqlcc
