#include "storage_engine/b_plus_tree_internal_node.h"
#include "storage_engine.h"
#include "utils/logger.h"
#include <algorithm>

namespace sqlcc {

// B+树设计参数 (商业数据库标准)
#define PAGE_HEADER_SIZE 24
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

// B+树节点最大键数 (基于页面大小计算)
#define BPLUS_TREE_MAX_KEYS ((PAGE_DATA_SIZE - sizeof(int32_t)) / (sizeof(int32_t) + 10 + sizeof(int32_t)))

BPlusTreeInternalNode::BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, false) {
  if (page_) {
    char* data = GetData();
    SQLCC_LOG_DEBUG("Creating BPlusTreeInternalNode with page_id=" + std::to_string(page_id_) + ", data[0]=" + std::to_string(static_cast<int>(data[0])));

    // 初始化为空的内部节点
    data[0] = 0; // 标记为内部节点
    *reinterpret_cast<int32_t*>(data + 1) = 0; // 键数量为0
    *reinterpret_cast<int32_t*>(data + 5) = -1; // 父节点ID为-1
    *reinterpret_cast<int32_t*>(data + 9) = -1; // 第一个子节点ID为-1（可选）
    // 清零剩余的头部空间（填充字段）
    memset(data + 13, 0, PAGE_HEADER_SIZE - 13);

    // 初始化成员变量
    keys_.clear();
    child_page_ids_.clear();

    SQLCC_LOG_DEBUG("Initialized empty internal node page, page_id=" + std::to_string(page_id_));
  }
}

BPlusTreeInternalNode::~BPlusTreeInternalNode() {
  // 内部节点析构函数
}

void BPlusTreeInternalNode::Clear() {
  keys_.clear();
  child_page_ids_.clear();
  parent_page_id_ = -1;
}

bool BPlusTreeInternalNode::IsFull() const {
  return keys_.size() >= BPLUS_TREE_MAX_KEYS;
}

void BPlusTreeInternalNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = GetData();
  data[0] = 0; // 标记为内部节点
  *reinterpret_cast<int32_t *>(data + 1) = static_cast<int32_t>(keys_.size()); // 键数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID

  // 清零剩余的头部空间（填充字段）
  memset(data + 9, 0, PAGE_HEADER_SIZE - 9);

  // 序列化键和子节点ID
  size_t offset = PAGE_HEADER_SIZE;

  if (keys_.empty()) {
    // 没有键的情况，只序列化唯一的子节点ID
    if (!child_page_ids_.empty()) {
      memcpy(data + offset, &child_page_ids_[0], sizeof(int32_t));
      SQLCC_LOG_DEBUG("Serialized internal node with no keys, child_page_id: " + std::to_string(child_page_ids_[0]));
    }
  } else {
    // 有键的情况，按原逻辑处理
    for (size_t i = 0; i < keys_.size(); ++i) {
      // 序列化键长度
      int32_t key_len = static_cast<int32_t>(keys_[i].size());
      memcpy(data + offset, &key_len, sizeof(int32_t));
      offset += sizeof(int32_t);

      // 序列化键内容
      memcpy(data + offset, keys_[i].c_str(), key_len);
      offset += key_len;

      // 序列化子节点ID
      memcpy(data + offset, &child_page_ids_[i], sizeof(int32_t));
      offset += sizeof(int32_t);
    }

    // 序列化最后一个子节点ID
    if (!child_page_ids_.empty() && child_page_ids_.size() > keys_.size()) {
      memcpy(data + offset, &child_page_ids_.back(), sizeof(int32_t));
      SQLCC_LOG_DEBUG("Serialized internal node with keys, last child_page_id: " + std::to_string(child_page_ids_.back()));
    }
  }

  // 页面已修改，将在UnpinPage时标记为脏页
}

void BPlusTreeInternalNode::DeserializeFromPage() {
  if (!page_)
    return;

  char *data = GetData();

  // 检查节点类型是否正确
  if (data[0] != 0) {  // 内部节点应该标记为0
    SQLCC_LOG_ERROR("Invalid node type in B+Tree internal node: " + std::to_string(static_cast<int>(data[0])) + ", expected 0");
    keys_.clear();
    child_page_ids_.clear();
    return;
  }

  int32_t key_count = *reinterpret_cast<int32_t *>(data + 1);

  // 增强数据验证：检查键数量是否合理
  if (key_count < 0 || key_count > static_cast<int32_t>(BPLUS_TREE_MAX_KEYS)) {
    SQLCC_LOG_ERROR("Invalid key count in B+Tree internal node: " + std::to_string(key_count) +
                   ", max allowed: " + std::to_string(BPLUS_TREE_MAX_KEYS));
    keys_.clear();
    child_page_ids_.clear();
    return;
  }

  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);

  keys_.clear();
  child_page_ids_.clear();
  keys_.reserve(key_count);
  child_page_ids_.reserve(key_count + 1);

  size_t offset = PAGE_HEADER_SIZE;
  // 内部节点应该至少有一个子节点ID，即使没有键
  if (key_count == 0) {
    // 对于空内部节点，不读取子节点ID，保持child_page_ids_为空
    // 这是因为新创建的节点还没有子节点
    SQLCC_LOG_DEBUG("Empty internal node, no child page IDs to deserialize");
  } else {
    // 有键的情况，按原有逻辑处理
    for (int32_t i = 0; i < key_count; ++i) {
      // 反序列化键长度
      int32_t key_len = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);

      // 反序列化键内容
      std::string key(data + offset, key_len);
      offset += key_len;
      keys_.push_back(key);

      // 反序列化子节点ID
      int32_t child_page_id = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);
      child_page_ids_.push_back(child_page_id);
    }

    // 反序列化最后一个子节点ID
    int32_t last_child_id = *reinterpret_cast<int32_t *>(data + offset);
    child_page_ids_.push_back(last_child_id);
  }
}

void BPlusTreeInternalNode::InsertChild(int32_t child_page_id) {
  // 插入第一个子节点，不需要键
  if (child_page_ids_.empty()) {
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added first child_page_id=" + std::to_string(child_page_id) + " to internal node page_id=" + std::to_string(page_id_));
  } else {
    // 如果已经有子节点，总是添加新的子节点，而不是替换
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added additional child_page_id=" + std::to_string(child_page_id) + " to internal node page_id=" + std::to_string(page_id_));
  }
  SerializeToPage();
}

void BPlusTreeInternalNode::InsertChild(int32_t child_page_id, const std::string &key) {
  // 插入键和子节点ID
  if (child_page_ids_.empty()) {
    // 第一个子节点，只添加子节点ID，不添加键
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added first child_page_id=" + std::to_string(child_page_id) + " to internal node page_id=" + std::to_string(page_id_) + " without key");
  } else {
    // 对于后续的子节点，我们需要添加键和子节点ID
    keys_.push_back(key);
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added child_page_id=" + std::to_string(child_page_id) + " with key='" + key + "' to internal node page_id=" + std::to_string(page_id_));
  }
  // 序列化到页面
  SerializeToPage();
}

void BPlusTreeInternalNode::RemoveChild(int32_t child_page_id) {
  // 找到要删除的子节点ID的位置
  auto it = std::find(child_page_ids_.begin(), child_page_ids_.end(), child_page_id);
  if (it != child_page_ids_.end()) {
    size_t pos = it - child_page_ids_.begin();

    // 删除子节点ID
    child_page_ids_.erase(it);

    // 如果不是第一个子节点，还需要删除对应的键
    if (pos > 0) {
      keys_.erase(keys_.begin() + pos - 1);
    }

    // 序列化到页面
    SerializeToPage();
  }
}

int32_t BPlusTreeInternalNode::FindChildPageId(const std::string &key) const {
  // 检查是否有子节点
  if (child_page_ids_.empty()) {
    SQLCC_LOG_ERROR("No child page IDs in internal node");
    return -1; // 返回无效页面ID
  }

  SQLCC_LOG_DEBUG("FindChildPageId called with key: '" + key + "', node page_id: " + std::to_string(page_id_));
  SQLCC_LOG_DEBUG("Node keys: ");
  for (size_t i = 0; i < keys_.size(); ++i) {
    SQLCC_LOG_DEBUG("  keys_[" + std::to_string(i) + "] = '" + keys_[i] + "'");
  }
  SQLCC_LOG_DEBUG("Node child_page_ids: ");
  for (size_t i = 0; i < child_page_ids_.size(); ++i) {
    SQLCC_LOG_DEBUG("  child_page_ids_[" + std::to_string(i) + "] = " + std::to_string(child_page_ids_[i]));
  }

  // 二分查找找到第一个大于等于key的位置
  auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
  size_t pos = it - keys_.begin();

  SQLCC_LOG_DEBUG("Lower bound position for key '" + key + "': " + std::to_string(pos));

  // 根据B+树的搜索规则：
  // - 如果key小于所有键，返回第一个子节点
  // - 如果key大于等于某个键，返回该键右侧的子节点

  // 检查pos是否在有效范围内
  if (pos >= child_page_ids_.size()) {
    // 如果pos超出范围，返回最后一个子节点
    SQLCC_LOG_WARN("Child page ID position out of bounds: " + std::to_string(pos) +
                   ", child_page_ids_.size() = " + std::to_string(child_page_ids_.size()) +
                   ". Returning last child page ID.");
    return child_page_ids_.back();
  }

  int32_t result = child_page_ids_[pos];
  SQLCC_LOG_DEBUG("Returning child page ID: " + std::to_string(result));
  return result;
}

void BPlusTreeInternalNode::Split(std::unique_ptr<BPlusTreeInternalNode>& new_node) {
  // 创建新节点
  if (!storage_engine_)
    return;

  int32_t new_page_id;
  if (!storage_engine_->NewPage(&new_page_id)) {
    SQLCC_LOG_ERROR("Failed to allocate new page for B+Tree internal node split");
    return;
  }

  new_node = std::make_unique<BPlusTreeInternalNode>(storage_engine_, new_page_id);
  if (!new_node) {
    SQLCC_LOG_ERROR("Failed to create new B+Tree internal node");
    storage_engine_->DeletePage(new_page_id);
    return;
  }

  // 计算中间位置
  size_t mid = keys_.size() / 2;

  // 将后半部分的键和子节点移动到新节点
  new_node->keys_.assign(keys_.begin() + mid + 1, keys_.end());
  new_node->child_page_ids_.assign(child_page_ids_.begin() + mid + 1, child_page_ids_.end());
  new_node->parent_page_id_ = parent_page_id_;

  // 删除后半部分，包括中间键
  keys_.erase(keys_.begin() + mid, keys_.end());
  child_page_ids_.erase(child_page_ids_.begin() + mid + 1, child_page_ids_.end());

  // 序列化两个节点
  SerializeToPage();
  if (new_node) {
    new_node->SerializeToPage();
  }
}

void BPlusTreeInternalNode::Merge(std::unique_ptr<BPlusTreeInternalNode> right_node, const std::string &parent_key) {
  if (!right_node)
    return;

  // 添加父节点传递的键到当前节点
  keys_.push_back(parent_key);

  // 将右节点的所有键和子节点合并到当前节点
  keys_.insert(keys_.end(), right_node->keys_.begin(), right_node->keys_.end());
  child_page_ids_.insert(child_page_ids_.end(), right_node->child_page_ids_.begin(), right_node->child_page_ids_.end());

  // 序列化当前节点
  SerializeToPage();
}

} // namespace sqlcc
