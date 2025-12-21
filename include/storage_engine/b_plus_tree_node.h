#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;
class Page;

/**
 * @brief 索引条目结构体
 */
struct IndexEntry {
    std::string key;
    int32_t page_id;
    size_t offset;

    IndexEntry() : page_id(-1), offset(0) {}
    IndexEntry(const std::string& k, int32_t p, size_t o) : key(k), page_id(p), offset(o) {}

    bool operator<(const IndexEntry& other) const {
        return key < other.key;
    }
};

/**
 * @class BPlusTreeNode
 * @brief B+树节点基类
 */
class BPlusTreeNode {
public:
    BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_leaf);
    virtual ~BPlusTreeNode();

    // 禁止拷贝和赋值
    BPlusTreeNode(const BPlusTreeNode&) = delete;
    BPlusTreeNode& operator=(const BPlusTreeNode&) = delete;

    // 允许移动
    BPlusTreeNode(BPlusTreeNode&&) = default;
    BPlusTreeNode& operator=(BPlusTreeNode&&) = default;

    // 基本操作
    int32_t GetPageId() const { return page_id_; }
    int32_t GetParentPageId() const { return parent_page_id_; }
    void SetParentPageId(int32_t parent_id) { parent_page_id_ = parent_id; }
    bool IsLeaf() const { return is_leaf_; }

    // 纯虚函数，由子类实现
    virtual void SerializeToPage() = 0;
    virtual void DeserializeFromPage() = 0;
    virtual void Clear() = 0;
    virtual bool IsFull() const = 0;

    // 页面访问
    char* GetData();
    const char* GetData() const;

protected:
    std::shared_ptr<StorageEngine> storage_engine_;
    int32_t page_id_;
    int32_t parent_page_id_;
    bool is_leaf_;
    std::shared_ptr<Page> page_;
};

} // namespace sqlcc
