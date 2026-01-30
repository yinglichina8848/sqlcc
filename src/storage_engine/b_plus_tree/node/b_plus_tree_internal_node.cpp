#include "src/storage_engine/b_plus_tree/node/b_plus_tree_internal_node.h"
#include "../../../include/storage_engine.h"
#include "../../../../include/utils/logger.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

BPlusTreeInternalNode::BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_new)
    : BPlusTreeNode(storage_engine, page_id, false) {
  // 如果是新节点，清零页面数据；否则保留现有数据
  if (is_new && page_) {
    char* data = GetData();
    if (data) {
      memset(data, 0, PAGE_SIZE);
    }
  }

  // 确保成员变量为空
  keys_.clear();
  child_page_ids_.clear();
  parent_page_id_ = -1;

  SQLCC_LOG_DEBUG(">BPlusTreeInternalNode constructed: page_id=" + std::to_string(page_id_) +
                 ", is_new=" + std::to_string(is_new) +
                 ", keys_.size()=" + std::to_string(keys_.size()) +
                 ", child_page_ids_.size()=" + std::to_string(child_page_ids_.size()));
}

BPlusTreeInternalNode::~BPlusTreeInternalNode() {
  // 内部节点析构函数
}

void BPlusTreeInternalNode::Clear() {
  keys_.clear();
  child_page_ids_.clear();
  parent_page_id_ = -1;
  
  // 同时清零页面数据
  if (page_) {
    char* data = GetData();
    if (data) {
      memset(data, 0, PAGE_SIZE);
    }
  }
}

bool BPlusTreeInternalNode::IsFull() const {
  return keys_.size() >= 100; // 简化的满载判断
}

void BPlusTreeInternalNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = GetData();
  data[0] = 0; // 标记为内部节点
  *reinterpret_cast<int32_t *>(data + 1) = static_cast<int32_t>(keys_.size()); // 键数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID

  // 清零剩余的头部空间
  memset(data + 9, 0, PAGE_HEADER_SIZE - 9);

  // 序列化键和子节点ID
  size_t offset = PAGE_HEADER_SIZE;

  // 内部节点应该有 n 个键和 n+1 个子节点
  // 键将子节点范围分隔：child[0] < key[0] <= child[1] < key[1] <= child[2] ...
  for (size_t i = 0; i < keys_.size(); ++i) {
    // 序列化键长度
    int32_t key_len = static_cast<int32_t>(keys_[i].size());
    memcpy(data + offset, &key_len, sizeof(int32_t));
    offset += sizeof(int32_t);

    // 序列化键内容
    memcpy(data + offset, keys_[i].c_str(), key_len);
    offset += key_len;

    // 序列化对应的子节点ID（键之前的子节点）
    memcpy(data + offset, &child_page_ids_[i], sizeof(int32_t));
    offset += sizeof(int32_t);
  }

  // 序列化最后一个子节点ID（在最后一个键之后）
  if (!child_page_ids_.empty()) {
    memcpy(data + offset, &child_page_ids_.back(), sizeof(int32_t));
    SQLCC_LOG_DEBUG("Serialized internal node with " + std::to_string(keys_.size()) +
                   " keys and " + std::to_string(child_page_ids_.size()) + " children");
  }
}

void BPlusTreeInternalNode::DeserializeFromPage() {
  if (!page_)
    return;

  char *data = GetData();

  std::cout << ">DeserializeFromPage: starting for page " + std::to_string(page_id_) << std::endl;
  SQLCC_LOG_DEBUG(">DeserializeFromPage: starting for page " + std::to_string(page_id_));

  // 检查节点类型
  uint8_t node_type = static_cast<uint8_t>(data[0]);
  SQLCC_LOG_DEBUG(">DeserializeFromPage: node_type = " + std::to_string(node_type));

  if (node_type != 0) {
    SQLCC_LOG_ERROR("Invalid node type in B+Tree internal node: " + std::to_string(static_cast<int>(node_type)) + ", expected 0");
    Clear();
    return;
  }

  int32_t key_count = *reinterpret_cast<int32_t *>(data + 1);
  SQLCC_LOG_DEBUG(">DeserializeFromPage: key_count = " + std::to_string(key_count));

  // 数据验证
  if (key_count < 0 || key_count > 100) {
    SQLCC_LOG_ERROR("Invalid key count in B+Tree internal node: " + std::to_string(key_count));
    Clear();
    return;
  }

  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);
  SQLCC_LOG_DEBUG(">DeserializeFromPage: parent_page_id = " + std::to_string(parent_page_id_));

  // 清空当前数据
  keys_.clear();
  child_page_ids_.clear();

  if (key_count == 0) {
    // 空节点
    SQLCC_LOG_DEBUG(">Deserialized empty internal node");
  } else {
    // 有键的节点
    size_t offset = PAGE_HEADER_SIZE;
    SQLCC_LOG_DEBUG(">DeserializeFromPage: starting deserialization loop at offset " + std::to_string(offset));

    for (int32_t i = 0; i < key_count; ++i) {
      // 反序列化键
      int32_t key_len = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);
      SQLCC_LOG_DEBUG(">DeserializeFromPage: key_len[" + std::to_string(i) + "] = " + std::to_string(key_len));

      if (key_len < 0 || key_len > 1000) {  // 合理性检查
        SQLCC_LOG_ERROR("Invalid key length: " + std::to_string(key_len));
        Clear();
        return;
      }

      std::string key(data + offset, key_len);
      offset += key_len;
      keys_.push_back(key);
      SQLCC_LOG_DEBUG(">DeserializeFromPage: key[" + std::to_string(i) + "] = '" + key + "'");

      // 反序列化子节点ID
      int32_t child_page_id = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);
      child_page_ids_.push_back(child_page_id);
      SQLCC_LOG_DEBUG(">DeserializeFromPage: child_page_id[" + std::to_string(i) + "] = " + std::to_string(child_page_id));
    }

    // 反序列化最后一个子节点ID
    int32_t last_child_id = *reinterpret_cast<int32_t *>(data + offset);
    child_page_ids_.push_back(last_child_id);
    SQLCC_LOG_DEBUG(">DeserializeFromPage: last_child_id = " + std::to_string(last_child_id));

    SQLCC_LOG_DEBUG(">Deserialized internal node with " + std::to_string(key_count) + " keys and " +
                   std::to_string(child_page_ids_.size()) + " children");
  }
}

void BPlusTreeInternalNode::InsertChild(int32_t child_page_id) {
  // 插入第一个子节点
  if (child_page_ids_.empty()) {
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG(">InsertChild: Added first child_page_id=" + std::to_string(child_page_id));
  } else {
    // 这里应该不会发生，因为测试应该使用带键的版本
    SQLCC_LOG_ERROR("InsertChild called without key on non-empty internal node");
  }
  // 移除自动序列化，由调用者控制
  // SerializeToPage();
}

void BPlusTreeInternalNode::InsertChild(int32_t child_page_id, const std::string &key) {
  // 插入键和子节点ID
  if (child_page_ids_.empty()) {
    // 第一个子节点，只有子节点ID，没有键
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG(">InsertChild: Added first child_page_id=" + std::to_string(child_page_id) + " without key");
  } else {
    // 后续子节点，有键和子节点ID
    keys_.push_back(key);
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG(">InsertChild: Added child_page_id=" + std::to_string(child_page_id) + " with key='" + key + "'");
  }
  // 移除自动序列化，由调用者控制
  // SerializeToPage();
}

void BPlusTreeInternalNode::RemoveChild(int32_t child_page_id) {
  auto it = std::find(child_page_ids_.begin(), child_page_ids_.end(), child_page_id);
  if (it != child_page_ids_.end()) {
    size_t pos = it - child_page_ids_.begin();
    child_page_ids_.erase(it);
    if (pos > 0) {
      keys_.erase(keys_.begin() + pos - 1);
    }
    SerializeToPage();
  }
}

int32_t BPlusTreeInternalNode::FindChildPageId(const std::string &key) const {
  if (child_page_ids_.empty()) {
    SQLCC_LOG_ERROR("No child page IDs available, keys_.size()=" + std::to_string(keys_.size()) +
                   ", child_page_ids_.size()=" + std::to_string(child_page_ids_.size()));
    return -1;
  }

  // 验证内部节点的一致性：子节点数量应该比键数量多1
  if (child_page_ids_.size() != keys_.size() + 1) {
    SQLCC_LOG_ERROR("Invalid internal node state: keys_.size()=" + std::to_string(keys_.size()) +
                   ", child_page_ids_.size()=" + std::to_string(child_page_ids_.size()) +
                   ", should have keys.size() + 1 children");
    return -1;
  }

  if (keys_.empty()) {
    // 如果没有键，但有子节点（比如只有两个子节点的根节点）
    // 这种情况通常意味着只有一个分隔键，我们需要决定去哪个子节点
    SQLCC_LOG_DEBUG("No keys but have child page IDs, this shouldn't happen in a well-formed tree");
    return child_page_ids_[0];
  }

  // 使用二分查找找到第一个 >= key 的键位置
  auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
  size_t pos = it - keys_.begin();

  // B+树内部节点规则：
  // keys_[i] 是分隔键，左子树的所有键 < keys_[i]，右子树的所有键 >= keys_[i]
  // child_page_ids_[i] 是第i个子节点
  // 如果pos == 0，表示key < keys_[0]，应该去最左边的子节点
  // 如果pos == keys_.size()，表示key >= keys_[back]，应该去最右边的子节点
  // 否则，表示keys_[pos-1] <= key < keys_[pos]，应该去child_page_ids_[pos]

  int32_t result_page_id;
  if (pos == 0) {
    // key < keys_[0]，去最左边的子节点
    result_page_id = child_page_ids_[0];
    SQLCC_LOG_DEBUG("Key '" + key + "' < first key '" + keys_[0] + "', going to leftmost child: " + std::to_string(result_page_id));
  } else if (pos >= keys_.size()) {
    // key >= keys_[back]，去最右边的子节点
    result_page_id = child_page_ids_.back();
    SQLCC_LOG_DEBUG("Key '" + key + "' >= last key '" + keys_.back() + "', going to rightmost child: " + std::to_string(result_page_id));
  } else {
    // keys_[pos-1] <= key < keys_[pos]，去第pos个子节点
    result_page_id = child_page_ids_[pos];
    std::string range_start = (pos > 0) ? keys_[pos-1] : "(-inf)";
    std::string range_end = keys_[pos];
    SQLCC_LOG_DEBUG("Key '" + key + "' in range [" + range_start + ", " + range_end + "), going to child: " + std::to_string(result_page_id));
  }

  // 验证返回的页面ID有效性
  if (result_page_id < 0) {
    SQLCC_LOG_ERROR("Invalid child page ID returned: " + std::to_string(result_page_id));
    return -1;
  }

  return result_page_id;
}

void BPlusTreeInternalNode::Split(std::unique_ptr<BPlusTreeInternalNode>& new_node) {
  if (!storage_engine_)
    return;

  int32_t new_page_id;
  if (!storage_engine_->NewPage(&new_page_id)) {
    return;
  }

  new_node = std::make_unique<BPlusTreeInternalNode>(storage_engine_, new_page_id);
  if (!new_node) {
    storage_engine_->DeletePage(new_page_id);
    return;
  }

  size_t mid = keys_.size() / 2;

  new_node->keys_.assign(keys_.begin() + mid + 1, keys_.end());
  new_node->child_page_ids_.assign(child_page_ids_.begin() + mid + 1, child_page_ids_.end());
  new_node->parent_page_id_ = parent_page_id_;

  keys_.erase(keys_.begin() + mid, keys_.end());
  child_page_ids_.erase(child_page_ids_.begin() + mid + 1, child_page_ids_.end());

  SerializeToPage();
  if (new_node) {
    new_node->SerializeToPage();
  }
}

void BPlusTreeInternalNode::Merge(std::unique_ptr<BPlusTreeInternalNode> right_node, const std::string &parent_key) {
  if (!right_node)
    return;

  keys_.push_back(parent_key);
  keys_.insert(keys_.end(), right_node->keys_.begin(), right_node->keys_.end());
  child_page_ids_.insert(child_page_ids_.end(), right_node->child_page_ids_.begin(), right_node->child_page_ids_.end());

  SerializeToPage();
}

} // namespace sqlcc
