#pragma once

#include "page.h"
#include "storage_engine.h"
#include "config_manager.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

// 前向声明
namespace sqlcc {
class StorageEngine;
class ConfigManager;
}

namespace sqlcc {

/**
 * @brief B+树节点类型枚举
 */
enum class BPlusTreeNodeType {
    INTERNAL_NODE,
    LEAF_NODE
};

/**
 * @brief B+树键值对
 * 键是索引的键值，值是记录所在的页面ID和偏移量
 */
struct IndexEntry {
    std::string key;              // 索引键值
    int32_t page_id;              // 记录所在页面ID
    size_t offset;                // 记录在页面中的偏移量

    IndexEntry() : page_id(-1), offset(0) {}
    IndexEntry(const std::string& k, int32_t pid, size_t off) 
        : key(k), page_id(pid), offset(off) {}

    bool operator<(const IndexEntry& other) const {
        return key < other.key;
    }

    bool operator==(const IndexEntry& other) const {
        return key == other.key;
    }
};

/**
 * @brief B+树节点基类
 */
class BPlusTreeNode {
public:
    BPlusTreeNode(StorageEngine* storage_engine, int32_t page_id, bool is_leaf);
    virtual ~BPlusTreeNode();

    // 序列化和反序列化节点到页面
    virtual void SerializeToPage() = 0;
    virtual void DeserializeFromPage() = 0;

    // 节点基本操作
    virtual bool IsFull() const = 0;
    virtual bool Insert(const IndexEntry& entry) = 0;
    virtual bool Remove(const std::string& key) = 0;
    virtual std::vector<IndexEntry> Search(const std::string& key) const = 0;
    virtual std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const = 0;

    // 获取节点信息
    bool IsLeaf() const { return is_leaf_; }
    int32_t GetPageId() const { return page_id_; }
    int32_t GetParentPageId() const { return parent_page_id_; }
    void SetParentPageId(int32_t parent_id) { parent_page_id_ = parent_id; }

protected:
    StorageEngine* storage_engine_; // 存储引擎引用
    int32_t page_id_;              // 节点所在页面ID
    int32_t parent_page_id_;       // 父节点页面ID
    bool is_leaf_;                 // 是否为叶子节点
    Page* page_;                   // 节点对应的页面
};

/**
 * @brief B+树内部节点
 */
class BPlusTreeInternalNode : public BPlusTreeNode {
public:
    BPlusTreeInternalNode(StorageEngine* storage_engine, int32_t page_id);
    ~BPlusTreeInternalNode() override;

    void SerializeToPage() override;
    void DeserializeFromPage() override;
    bool IsFull() const override;
    bool Insert(const IndexEntry& entry) override;
    bool Remove(const std::string& key) override;
    std::vector<IndexEntry> Search(const std::string& key) const override;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const override;

    // 内部节点特有操作
    void InsertChild(int32_t child_page_id, const std::string& key);
    void RemoveChild(int32_t child_page_id);
    int32_t FindChildPageId(const std::string& key) const;
    void Split(BPlusTreeInternalNode*& new_node);

private:
    std::vector<std::string> keys_;         // 键值列表
    std::vector<int32_t> child_page_ids_;   // 子节点页面ID列表
};

/**
 * @brief B+树叶子节点
 */
class BPlusTreeLeafNode : public BPlusTreeNode {
public:
    BPlusTreeLeafNode(StorageEngine* storage_engine, int32_t page_id);
    ~BPlusTreeLeafNode() override;

    void SerializeToPage() override;
    void DeserializeFromPage() override;
    bool IsFull() const override;
    bool Insert(const IndexEntry& entry) override;
    bool Remove(const std::string& key) override;
    std::vector<IndexEntry> Search(const std::string& key) const override;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const override;

    // 叶子节点特有操作
    void SetNextPageId(int32_t next_page_id) { next_page_id_ = next_page_id; }
    int32_t GetNextPageId() const { return next_page_id_; }
    void Split(BPlusTreeLeafNode*& new_node);

private:
    std::vector<IndexEntry> entries_;  // 索引条目列表
    int32_t next_page_id_;             // 下一个叶子节点页面ID
};

/**
 * @brief B+树索引类
 * 管理B+树索引的创建、删除、查找等操作
 */
class BPlusTreeIndex {
public:
    BPlusTreeIndex(StorageEngine* storage_engine, const std::string& table_name, const std::string& column_name);
    ~BPlusTreeIndex();

    // 索引基本操作
    bool Create();
    bool Drop();
    bool Insert(const IndexEntry& entry);
    bool Delete(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound, const std::string& upper_bound) const;

    // 获取索引信息
    const std::string& GetTableName() const { return table_name_; }
    const std::string& GetColumnName() const { return column_name_; }
    bool Exists() const; // 检查索引是否存在

private:
    sqlcc::StorageEngine* storage_engine_;  // 存储引擎引用
    std::string table_name_;         // 表名
    std::string column_name_;        // 列名
    std::string index_name_;         // 索引名
    int32_t root_page_id_;           // 根节点页面ID
    int32_t metadata_page_id_;       // 元数据页面ID

    // 辅助方法
    void LoadMetadata();
    void SaveMetadata();
    BPlusTreeNode* GetNode(int32_t page_id) const;
    BPlusTreeNode* CreateNewNode(bool is_leaf);
    void DeleteNode(int32_t page_id);
};

/**
 * @brief 索引管理器类
 * 管理所有表的索引，提供索引的创建、删除、查询等功能
 */
class IndexManager {
public:
    IndexManager(sqlcc::StorageEngine* storage_engine, sqlcc::ConfigManager& config_manager);
    ~IndexManager();

    // 索引管理操作
    bool CreateIndex(const std::string& index_name, const std::string& table_name, const std::string& column_name, bool is_unique = false);
    bool DropIndex(const std::string& index_name, const std::string& table_name);
    bool IndexExists(const std::string& index_name, const std::string& table_name) const;
    class BPlusTreeIndex* GetIndex(const std::string& index_name, const std::string& table_name);

    // 获取表的所有索引
    std::vector<class BPlusTreeIndex*> GetTableIndexes(const std::string& table_name) const;
    
    // 加载所有索引
    void LoadAllIndexes();

private:
    StorageEngine* storage_engine_;  // 存储引擎引用
    std::unordered_map<std::string, std::unique_ptr<class BPlusTreeIndex>> indexes_; // 索引映射表
    
    // 辅助方法
    std::string GetIndexName(const std::string& table_name, const std::string& column_name) const;
};

} // namespace sqlcc