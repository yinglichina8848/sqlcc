#pragma once

#include "../node/b_plus_tree_node.h"
#include "../node/b_plus_tree_leaf_node.h"
#include "../node/b_plus_tree_internal_node.h"
#include "../../storage_engine.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;

class BPlusTreeIndex {
public:
    BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine,
                   const std::string& table_name,
                   const std::string& column_name);
    ~BPlusTreeIndex();

    BPlusTreeIndex(const BPlusTreeIndex&) = delete;
    BPlusTreeIndex& operator=(const BPlusTreeIndex&) = delete;

    bool Create();
    bool Drop();
    bool Exists() const;

    bool Insert(const std::string& key, int32_t page_id, size_t offset);
    bool Delete(const std::string& key);
    std::vector<IndexEntry> Search(const std::string& key) const;
    std::vector<IndexEntry> SearchRange(const std::string& lower_bound,
                                         const std::string& upper_bound) const;

    const std::string& GetTableName() const { return table_name_; }
    const std::string& GetColumnName() const { return column_name_; }
    int32_t GetRootPageId() const { return root_page_id_; }

private:
    bool InsertIterative(const std::string& key, int32_t page_id, size_t offset);
    int32_t FindLeafPageId(const std::string& key);
    bool HandleLeafSplit(BPlusTreeLeafNode* leaf_node, int recursion_depth);
    bool HandleInternalSplit(BPlusTreeInternalNode* internal_node, BPlusTreeNode* child_node, int recursion_depth);
    bool HandleRootSplit(int32_t left_child_id, int32_t right_child_id, const std::string& split_key);
    bool UpdateParentForSplit(int32_t parent_page_id, int32_t left_child_id,
                             int32_t right_child_id, const std::string& split_key,
                             int recursion_depth);
    bool InsertRecursive(const std::string& key, int32_t page_id, size_t offset,
                        std::unique_ptr<BPlusTreeNode>& node, int recursion_depth);
    bool DeleteRecursive(const std::string& key, std::unique_ptr<BPlusTreeNode>& node);
    std::vector<IndexEntry> SearchRecursive(const std::string& key,
                                             std::unique_ptr<BPlusTreeNode>& node) const;
    std::vector<IndexEntry> SearchRangeRecursive(const std::string& lower_bound,
                                                  const std::string& upper_bound,
                                                  std::unique_ptr<BPlusTreeNode>& node) const;
    std::unique_ptr<BPlusTreeNode> LoadNode(int32_t page_id);

    std::shared_ptr<StorageEngine> storage_engine_;
    std::string table_name_;
    std::string column_name_;
    int32_t root_page_id_;
    std::unique_ptr<BPlusTreeNode> root_node_;
};

} // namespace sqlcc
