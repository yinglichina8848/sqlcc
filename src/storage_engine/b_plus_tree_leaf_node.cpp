#include "storage_engine/b_plus_tree_leaf_node.h"
#include "storage_engine.h"
#include "storage_engine/node_size_manager.h"
#include "utils/logger.h"
#include "page.h"
#include <algorithm>

// Page header for B+Tree nodes (存储在页面头部的B+树节点元数据)
// Page header format:
// [is_leaf(1)] [key_count(4)] [parent_page_id(4)] [next_page_id(4)]
// [padding(7)]
#define PAGE_HEADER_SIZE 24
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

namespace sqlcc {

BPlusTreeLeafNode::BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, true), next_page_id_(-1) {
  SQLCC_LOG_DEBUG("BPlusTreeLeafNode constructor: is_leaf_ should be true, actual value: " + std::to_string(is_leaf_));

  if (page_) {
    char* data = GetData();
    SQLCC_LOG_DEBUG("Creating BPlusTreeLeafNode with page_id=" + std::to_string(page_id_) + ", data[0]=" + std::to_string(static_cast<int>(data[0])));

    // 检查是否已有有效的叶子节点数据
    if (data[0] == 1) {
      // 反序列化现有数据
      DeserializeFromPage();
      SQLCC_LOG_DEBUG("Deserialized existing leaf node data, entries: " + std::to_string(entries_.size()));
    } else {
      // 初始化为空的叶子节点
      data[0] = 1; // 标记为叶子节点
      *reinterpret_cast<int32_t*>(data + 1) = 0; // 条目数量为0
      *reinterpret_cast<int32_t*>(data + 5) = -1; // 父节点ID为-1
      *reinterpret_cast<int32_t*>(data + 9) = -1; // 下一个叶子节点ID为-1
      // 清零剩余的头部空间（填充字段）
      memset(data + 13, 0, PAGE_HEADER_SIZE - 13);

      // 初始化成员变量
      entries_.clear();
      next_page_id_ = -1;

      SQLCC_LOG_DEBUG("Initialized empty leaf node page, page_id=" + std::to_string(page_id_));
    }
  }

  SQLCC_LOG_DEBUG("BPlusTreeLeafNode constructor complete: is_leaf_ = " + std::to_string(is_leaf_));
}

BPlusTreeLeafNode::~BPlusTreeLeafNode() {
  // 叶子节点析构函数
}

void BPlusTreeLeafNode::Clear() {
  entries_.clear();
  next_page_id_ = -1;
  parent_page_id_ = -1;
}

bool BPlusTreeLeafNode::IsFull() const {
  // 使用动态节点大小管理器进行容量检查
  std::vector<size_t> entry_sizes;
  entry_sizes.reserve(entries_.size());

  for (const auto& entry : entries_) {
    // 计算每个条目的实际大小（键长 + 键内容 + 页面ID + 偏移量）
    size_t entry_size = sizeof(int32_t) + entry.key.size() + // 键长度 + 键内容
                       sizeof(int32_t) +                    // 页面ID
                       sizeof(size_t);                      // 偏移量
    entry_sizes.push_back(entry_size);
  }

  // 使用NodeSizeManager进行动态容量判断
  return NodeSizeManager::get_instance().should_split_node(
    "leaf", entries_.size(), PAGE_DATA_SIZE
  );
}

void BPlusTreeLeafNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = GetData();
  data[0] = 1; // 标记为叶子节点
  *reinterpret_cast<int32_t *>(data + 1) = static_cast<int32_t>(entries_.size()); // 条目数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID
  *reinterpret_cast<int32_t *>(data + 9) = next_page_id_; // 下一个叶子节点ID

  // 清零剩余的头部空间（填充字段）
  memset(data + 13, 0, PAGE_HEADER_SIZE - 13);

  // 序列化条目
  size_t offset = PAGE_HEADER_SIZE;

  for (const auto& entry : entries_) {
    // 序列化键长度
    int32_t key_len = static_cast<int32_t>(entry.key.size());
    memcpy(data + offset, &key_len, sizeof(int32_t));
    offset += sizeof(int32_t);

    // 序列化键内容
    memcpy(data + offset, entry.key.c_str(), key_len);
    offset += key_len;

    // 序列化页面ID
    memcpy(data + offset, &entry.page_id, sizeof(int32_t));
    offset += sizeof(int32_t);

    // 序列化偏移量
    memcpy(data + offset, &entry.offset, sizeof(size_t));
    offset += sizeof(size_t);
  }

  // 页面已修改，将在UnpinPage时标记为脏页
}

void BPlusTreeLeafNode::DeserializeFromPage() {
  if (!page_)
    return;

  char *data = GetData();

  // 检查节点类型是否正确
  if (data[0] != 1) {  // 叶子节点应该标记为1
    SQLCC_LOG_ERROR("Invalid node type in B+Tree leaf node: " + std::to_string(static_cast<int>(data[0])) + ", expected 1");
    entries_.clear();
    return;
  }

  // 确保is_leaf_标志正确设置
  is_leaf_ = true;

  int32_t entry_count = *reinterpret_cast<int32_t *>(data + 1);

  // 增强数据验证：检查条目数量是否合理
  if (entry_count < 0 || entry_count > 1000) { // 假设最大1000个条目
    SQLCC_LOG_ERROR("Invalid entry count in B+Tree leaf node: " + std::to_string(entry_count));
    entries_.clear();
    return;
  }

  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);
  next_page_id_ = *reinterpret_cast<int32_t *>(data + 9);

  entries_.clear();
  entries_.reserve(entry_count);

  size_t offset = PAGE_HEADER_SIZE;
  for (int32_t i = 0; i < entry_count; ++i) {
    // 反序列化键长度
    int32_t key_len = *reinterpret_cast<int32_t *>(data + offset);
    offset += sizeof(int32_t);

    // 反序列化键内容
    std::string key(data + offset, key_len);
    offset += key_len;

    // 反序列化页面ID
    int32_t page_id = *reinterpret_cast<int32_t *>(data + offset);
    offset += sizeof(int32_t);

    // 反序列化偏移量
    size_t entry_offset = *reinterpret_cast<size_t*>(data + offset);
    offset += sizeof(size_t);

    entries_.emplace_back(key, page_id, entry_offset);
  }
}

bool BPlusTreeLeafNode::Insert(const IndexEntry& entry) {
  SQLCC_LOG_DEBUG("BPlusTreeLeafNode::Insert called with key '" + entry.key + "'");

  // 检查是否已存在相同的键
  for (auto& existing_entry : entries_) {
    if (existing_entry.key == entry.key) {
      // 更新现有的条目
      existing_entry.page_id = entry.page_id;
      existing_entry.offset = entry.offset;
      SQLCC_LOG_DEBUG("Updated existing entry for key '" + entry.key + "'");
      return true;
    }
  }

  // 插入新的条目，保持排序
  auto it = std::lower_bound(entries_.begin(), entries_.end(), entry,
                           [](const IndexEntry& a, const IndexEntry& b) {
                             return a.key < b.key;
                           });
  
  entries_.insert(it, entry);
  SQLCC_LOG_DEBUG("Inserted new entry for key '" + entry.key + "', total entries: " + std::to_string(entries_.size()));

  // 序列化到页面
  SerializeToPage();
  return true;
}

bool BPlusTreeLeafNode::Remove(const std::string& key) {
  SQLCC_LOG_DEBUG("BPlusTreeLeafNode::Remove called with key '" + key + "'");

  auto it = std::find_if(entries_.begin(), entries_.end(),
                        [&key](const IndexEntry& entry) {
                          return entry.key == key;
                        });

  if (it != entries_.end()) {
    entries_.erase(it);
    SQLCC_LOG_DEBUG("Removed entry for key '" + key + "', remaining entries: " + std::to_string(entries_.size()));
    // 序列化到页面
    SerializeToPage();
    return true;
  }

  SQLCC_LOG_DEBUG("Key '" + key + "' not found in leaf node");
  return false;
}

std::vector<IndexEntry> BPlusTreeLeafNode::Search(const std::string& key) const {
  std::vector<IndexEntry> results;

  for (const auto& entry : entries_) {
    if (entry.key == key) {
      results.push_back(entry);
    }
  }

  return results;
}

std::vector<IndexEntry> BPlusTreeLeafNode::SearchRange(const std::string& lower_bound, const std::string& upper_bound) const {
  std::vector<IndexEntry> results;

  for (const auto& entry : entries_) {
    bool in_range = true;
    
    if (!lower_bound.empty() && entry.key < lower_bound) {
      in_range = false;
    }
    
    if (!upper_bound.empty() && entry.key > upper_bound) {
      in_range = false;
    }
    
    if (in_range) {
      results.push_back(entry);
    }
  }

  return results;
}

void BPlusTreeLeafNode::Split(std::unique_ptr<BPlusTreeLeafNode>& new_node) {
  if (!new_node)
    return;

  // 计算分裂点 - 将后半部分移动到新节点
  size_t mid = entries_.size() / 2;
  
  // 将后半部分的条目移动到新节点
  new_node->entries_.assign(entries_.begin() + mid, entries_.end());
  new_node->parent_page_id_ = parent_page_id_;
  new_node->next_page_id_ = next_page_id_;

  // 删除后半部分
  entries_.erase(entries_.begin() + mid, entries_.end());

  // 设置链表关系
  new_node->next_page_id_ = next_page_id_;
  next_page_id_ = new_node->GetPageId();

  SQLCC_LOG_DEBUG("Split leaf node " + std::to_string(page_id_) + 
                 " into " + std::to_string(page_id_) + " and " + std::to_string(new_node->GetPageId()));

  // 序列化两个节点
  SerializeToPage();
  new_node->SerializeToPage();
}

void BPlusTreeLeafNode::Merge(std::unique_ptr<BPlusTreeLeafNode> right_node) {
  if (!right_node)
    return;

  // 将右节点的条目追加到当前节点
  entries_.insert(entries_.end(), right_node->entries_.begin(), right_node->entries_.end());

  // 更新链表关系
  next_page_id_ = right_node->next_page_id_;

  SQLCC_LOG_DEBUG("Merged leaf node " + std::to_string(right_node->GetPageId()) + 
                 " into " + std::to_string(page_id_));

  // 序列化当前节点
  SerializeToPage();
}

} // namespace sqlcc
