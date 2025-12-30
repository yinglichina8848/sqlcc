#ifndef SQLCC_B_PLUS_TREE_NODES_H
#define SQLCC_B_PLUS_TREE_NODES_H

#include <memory>
#include <string>
#include <vector>
#include "../page.h"

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
