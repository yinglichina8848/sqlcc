#include "storage_engine/b_plus_tree_leaf_node.h"
#include "storage_engine.h"
#include "utils/logger.h"
#include <algorithm>

namespace sqlcc {

// B+树设计参数 (商业数据库标准)
#define PAGE_HEADER_SIZE 24
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

// B+树叶子节点最大键数 (基于页面大小计算)
#define BPLUS_TREE_LEAF_MAX_KEYS ((PAGE_DATA_SIZE - sizeof(int32_t)) / (sizeof(int32_t) + 10 + sizeof(int32_t) + sizeof(size_t)))

BPlusTreeLeafNode::BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, true), next_page_id_(-1) {
  // 叶子节点构造函数
  if (page_) {
    // 检查页面是否是新页面（通过检查节点类型字节是否为0或其他无效值）
    char* data = GetData();
    SQLCC_LOG_DEBUG("Creating BPlusTreeLeafNode with page_id=" + std::to_string(page_id_) + ", data[0]=" + std::to_string(static_cast<int>(data[0])));
    if (data[0] != 0 && data[0] != 1) {
      // 新页面或未初始化页面，初始化为B+树叶子节点格式
      SQLCC_LOG_DEBUG("Initializing new leaf node page");
      data[0] = 1; // 标记为叶子节点
      *reinterpret_cast<int32_t*>(data + 1) = 0; // 条目数量为0
      *reinterpret_cast<int32_t*>(data + 5) = -1; // 父节点ID为-1
      *reinterpret_cast<int32_t*>(data + 9) = -1; // 下一节点ID为-1
      // 清零剩余的头部空间（填充字段）
      memset(data + 13, 0, PAGE_HEADER_SIZE - 13);
    } else if (data[0] == 0) {
      // 页面被标记为0，这可能是一个新页面（全零）或者是内部节点
      // 我们需要检查其他字段来区分这两种情况
      int32_t key_count = *reinterpret_cast<int32_t*>(data + 1);
      int32_t parent_page_id = *reinterpret_cast<int32_t*>(data + 5);
      int32_t next_page_id = *reinterpret_cast<int32_t*>(data + 9);

      // 如果所有字段都是0或-1，我们认为这是一个新页面
      if (key_count == 0 && (parent_page_id == 0 || parent_page_id == -1) && (next_page_id == 0 || next_page_id == -1)) {
        SQLCC_LOG_DEBUG("Initializing new leaf node page (was all zeros)");
        data[0] = 1; // 标记为叶子节点
        *reinterpret_cast<int32_t*>(data + 1) = 0; // 条目数量为0
        *reinterpret_cast<int32_t*>(data + 5) = -1; // 父节点ID为-1
        *reinterpret_cast<int32_t*>(data + 9) = -1; // 下一节点ID为-1
        // 清零剩余的头部空间（填充字段）
        memset(data + 13, 0, PAGE_HEADER_SIZE - 13);
      } else {
        // 页面被标记为内部节点但试图创建叶子节点，这是一个错误
        SQLCC_LOG_ERROR("Trying to create leaf node on page marked as internal node, page_id=" + std::to_string(page_id_));
        throw std::runtime_error("Cannot create leaf node on page marked as internal node");
      }
    } else {
      // 现有页面，从页面中反序列化数据
      SQLCC_LOG_DEBUG("Deserializing existing leaf node page");
      DeserializeFromPage();
    }
  }
}

BPlusTreeLeafNode::~BPlusTreeLeafNode() {
  // 叶子节点析构函数
}

void BPlusTreeLeafNode::Clear() {
  entries_.clear();
  parent_page_id_ = -1;
  next_page_id_ = -1;
}

bool BPlusTreeLeafNode::IsFull() const {
  return entries_.size() >= BPLUS_TREE_LEAF_MAX_KEYS;
}

void BPlusTreeLeafNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = GetData();
  data[0] = 1; // 标记为叶子节点
  *reinterpret_cast<int32_t *>(data + 1) = static_cast<int32_t>(entries_.size()); // 条目数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID
  *reinterpret_cast<int32_t *>(data + 9) = next_page_id_;   // 下一节点ID

  // 清零剩余的头部空间（填充字段）
  memset(data + 13, 0, PAGE_HEADER_SIZE - 13);

  // 序列化条目
  size_t offset = PAGE_HEADER_SIZE;
  for (const auto &entry : entries_) {
    // 序列化键长度
    int32_t key_len = static_cast<int32_t>(entry.key.size());
    memcpy(data + offset, &key_len, sizeof(int32_t));
    offset += sizeof(int32_t);

    // 序列化键内容
    memcpy(data + offset, entry.key.c_str(), key_len);
    offset += key_len;

    // 序列化页面ID和偏移量
    memcpy(data + offset, &entry.page_id, sizeof(int32_t));
    offset += sizeof(int32_t);
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

  int32_t entry_count = *reinterpret_cast<int32_t *>(data + 1);

  // 增强数据验证：检查条目数量是否合理
  if (entry_count < 0 || entry_count > static_cast<int32_t>(BPLUS_TREE_LEAF_MAX_KEYS)) {
    SQLCC_LOG_ERROR("Invalid entry count in B+Tree leaf node: " + std::to_string(entry_count) +
                   ", max allowed: " + std::to_string(BPLUS_TREE_LEAF_MAX_KEYS));
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

    // 反序列化页面ID和偏移量
    int32_t page_id = *reinterpret_cast<int32_t *>(data + offset);
    offset += sizeof(int32_t);
    size_t off = *reinterpret_cast<size_t *>(data + offset);
    offset += sizeof(size_t);

    entries_.emplace_back(key, page_id, off);
  }
}

bool BPlusTreeLeafNode::Insert(const IndexEntry &entry) {
  // 找到插入位置
  auto it = std::lower_bound(entries_.begin(), entries_.end(), entry);

  // 检查是否已存在相同的键
  if (it != entries_.end() && it->key == entry.key) {
    // 可以选择覆盖或返回false表示插入失败
    *it = entry; // 覆盖现有条目
  } else {
    // 插入新条目
    entries_.insert(it, entry);
  }

  // 序列化到页面
  SerializeToPage();
  return true;
}

bool BPlusTreeLeafNode::Remove(const std::string &key) {
  // 找到要删除的条目
  auto it = std::lower_bound(entries_.begin(), entries_.end(), IndexEntry(key, 0, 0));

  // 检查是否找到
  if (it != entries_.end() && it->key == key) {
    // 删除条目
    entries_.erase(it);

    // 序列化到页面
    SerializeToPage();
    return true;
  }

  return false;
}

std::vector<IndexEntry> BPlusTreeLeafNode::Search(const std::string &key) const {
  std::vector<IndexEntry> results;

  // 二分查找找到键
  auto it = std::lower_bound(entries_.begin(), entries_.end(), IndexEntry(key, 0, 0));

  // 检查是否找到
  if (it != entries_.end() && it->key == key) {
    results.push_back(*it);
  }

  return results;
}

std::vector<IndexEntry> BPlusTreeLeafNode::SearchRange(const std::string &lower_bound, const std::string &upper_bound) const {
  std::vector<IndexEntry> results;

  // 找到范围的起始位置
  auto start_it = std::lower_bound(entries_.begin(), entries_.end(),
                                   IndexEntry(lower_bound, 0, 0));

  // 收集范围内的所有条目
  for (auto it = start_it; it != entries_.end(); ++it) {
    // 使用字典序比较，当键大于upper_bound时停止
    if (it->key.compare(upper_bound) > 0)
      break;
    results.push_back(*it);
  }

  return results;
}

void BPlusTreeLeafNode::Split(std::unique_ptr<BPlusTreeLeafNode>& new_node) {
  // 创建新节点
  if (!storage_engine_)
    return;

  int32_t new_page_id;
  if (!storage_engine_->NewPage(&new_page_id)) {
    SQLCC_LOG_ERROR("Failed to allocate new page for B+Tree leaf node split");
    return;
  }

  new_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, new_page_id);
  if (!new_node) {
    SQLCC_LOG_ERROR("Failed to create new B+Tree leaf node");
    storage_engine_->DeletePage(new_page_id);
    return;
  }

  // 计算中间位置
  size_t mid = entries_.size() / 2;

  // 将后半部分的条目移动到新节点
  new_node->entries_.assign(entries_.begin() + mid, entries_.end());
  new_node->parent_page_id_ = parent_page_id_;
  new_node->next_page_id_ = next_page_id_;

  // 更新当前节点的下一个节点指针
  next_page_id_ = new_page_id;

  // 删除当前节点的后半部分条目
  entries_.erase(entries_.begin() + mid, entries_.end());

  // 序列化两个节点
  SerializeToPage();
  if (new_node) {
    new_node->SerializeToPage();
  }
}

void BPlusTreeLeafNode::Merge(std::unique_ptr<BPlusTreeLeafNode> right_node) {
  if (!right_node)
    return;

  // 将右节点的所有条目合并到当前节点
  entries_.insert(entries_.end(), right_node->entries_.begin(), right_node->entries_.end());

  // 更新当前节点的下一个节点指针
  next_page_id_ = right_node->next_page_id_;

  // 序列化当前节点
  SerializeToPage();
}

} // namespace sqlcc
