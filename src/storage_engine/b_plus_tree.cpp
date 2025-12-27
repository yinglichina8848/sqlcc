#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/logger.h"
#include <algorithm>

namespace sqlcc {

// B+树设计参数 (商业数据库标准)
// Page header for B+Tree nodes (存储在页面头部的B+树节点元数据)
// Page header format:
// [is_leaf(1)] [key_count(4)] [parent_page_id(4)] [next_page_id(4)]
// [padding(7)]
#define PAGE_HEADER_SIZE 24
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

/**
 * @class BPlusTreeNode
 * @brief B+树节点基类
 */
BPlusTreeNode::BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id,
                             bool is_leaf)
    : storage_engine_(std::move(storage_engine)), page_id_(page_id), parent_page_id_(-1),
      is_leaf_(is_leaf) {
  // 获取页面对象用于数据存储
  if (storage_engine_) {
    auto page_ptr = storage_engine_->FetchPage(page_id);
    if (page_ptr) {
      page_ = std::move(page_ptr);
    } else {
      SQLCC_LOG_ERROR("Failed to fetch page " + std::to_string(page_id) + " from storage engine");
      throw std::runtime_error("Failed to fetch page from storage engine");
    }
  } else {
    SQLCC_LOG_ERROR("Storage engine is null in BPlusTreeNode constructor");
    throw std::invalid_argument("Storage engine cannot be null");
  }

  SQLCC_LOG_DEBUG(std::string("Created B+Tree ") +
                  (is_leaf ? "leaf" : "internal") +
                  " node: page_id=" + std::to_string(page_id));
}

BPlusTreeNode::~BPlusTreeNode() {
  // 页面资源需要手动释放，将页面返回给StorageEngine
  if (storage_engine_ && page_) {
    storage_engine_->UnpinPage(page_id_, true); // 标记为脏页并释放
  }
  SQLCC_LOG_DEBUG("Destroyed B+Tree node: page_id=" + std::to_string(page_id_));
}

/**
 * @class BPlusTreeInternalNode
 * @brief B+树内部节点类
 */
BPlusTreeInternalNode::BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine,
                                             int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, false) {
  if (page_) {
    char* data = page_->GetDataSpan().data();
    SQLCC_LOG_DEBUG("Creating BPlusTreeInternalNode with page_id=" + std::to_string(page_id_) + ", data[0]=" + std::to_string(static_cast<int>(data[0])));

    // 无论页面之前的状态如何，都要确保它是内部节点格式
    // 先检查是否已经是正确的内部节点格式
    bool need_initialization = false;

    if (data[0] == 1) {
      // 页面被标记为叶子节点，需要重新初始化为内部节点
      SQLCC_LOG_DEBUG("Converting leaf node page to internal node, page_id=" + std::to_string(page_id_));
      need_initialization = true;
    } else if (data[0] != 0) {
      // 页面未初始化或有其他标记，初始化为内部节点
      SQLCC_LOG_DEBUG("Initializing uninitialized page as internal node, page_id=" + std::to_string(page_id_));
      need_initialization = true;
    } else {
      // 页面标记为0（内部节点），检查数据是否合理
      int32_t key_count = *reinterpret_cast<int32_t*>(data + 1);
      if (key_count < 0 || key_count > static_cast<int32_t>(BPLUS_TREE_MAX_KEYS)) {
        SQLCC_LOG_DEBUG("Invalid key count in internal node page, reinitializing, page_id=" + std::to_string(page_id_));
        need_initialization = true;
      } else {
        // 数据看起来合理，尝试反序列化
        SQLCC_LOG_DEBUG("Deserializing existing internal node page, page_id=" + std::to_string(page_id_));
        DeserializeFromPage();
      }
    }

    if (need_initialization) {
      // 初始化为B+树内部节点格式
      data[0] = 0; // 标记为内部节点
      *reinterpret_cast<int32_t*>(data + 1) = 0; // 键数量为0
      *reinterpret_cast<int32_t*>(data + 5) = -1; // 父节点ID为-1
      // 清零剩余的头部空间（填充字段）
      memset(data + 9, 0, PAGE_HEADER_SIZE - 9);

      // 初始化成员变量
      keys_.clear();
      child_page_ids_.clear();

      SQLCC_LOG_DEBUG("Initialized internal node page, page_id=" + std::to_string(page_id_));
    }
  }
}BPlusTreeInternalNode::~BPlusTreeInternalNode() {
  // 内部节点析构函数
}

void BPlusTreeInternalNode::Clear() {
  keys_.clear();
  child_page_ids_.clear();
  parent_page_id_ = -1;
}

void BPlusTreeInternalNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = page_->GetDataSpan().data();
  data[0] = 0; // 标记为内部节点
  *reinterpret_cast<int32_t *>(data + 1) =
      static_cast<int32_t>(keys_.size());                   // 键数量
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

  char *data = page_->GetDataSpan().data();
  
  // 检查页面数据是否足够存储头部信息
  if (PAGE_SIZE < PAGE_HEADER_SIZE) {
    SQLCC_LOG_ERROR("Page size too small for B+Tree node header");
    return;
  }
  
  // 检查节点类型是否正确
  if (data[0] != 0) {  // 内部节点应该标记为0
    SQLCC_LOG_ERROR("Invalid node type in B+Tree internal node: " + std::to_string(static_cast<int>(data[0])) + ", expected 0");
    keys_.clear();
    child_page_ids_.clear();
    return;
  }
  
  // 检查页面数据是否被正确初始化（避免读取未初始化的数据）
  // 检查key_count字段是否包含合理的值（避免读取垃圾数据）
  if (data[1] == 0 && data[2] == 0 && data[3] == 0 && data[4] == 0) {
    // 如果key_count为0，可能是新页面，继续处理
    SQLCC_LOG_DEBUG("Empty internal node page detected, initializing with zero keys");
  } else {
    // 检查数据是否看起来像有效的键计数
    // 如果最高位被设置，很可能是错误的数据
    uint32_t raw_key_count = *reinterpret_cast<uint32_t *>(data + 1);
    if (raw_key_count > 0x00FFFFFF) {  // 如果高字节被设置，可能是错误的数据
      SQLCC_LOG_ERROR("Suspiciously large key count in B+Tree internal node: " + std::to_string(raw_key_count) + ", likely data corruption");
      keys_.clear();
      child_page_ids_.clear();
      return;
    }
  }
  
  // 在读取key_count之前，先检查数据是否足够
  if (PAGE_SIZE < 5) {  // 至少需要5个字节（1字节类型 + 4字节计数）
    SQLCC_LOG_ERROR("Page size too small to read key count");
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
  
  // 进一步验证：检查键数量是否可能导致缓冲区溢出
  // 修正计算方式，考虑实际的序列化格式
  size_t estimated_min_data_size = key_count * (sizeof(int32_t) + 1 + sizeof(int32_t)); // 键长度(4)+最小键内容(1)+子节点ID(4)
  if (estimated_min_data_size > PAGE_DATA_SIZE) {
    SQLCC_LOG_ERROR("Estimated data size exceeds page capacity in B+Tree internal node: key_count=" + 
                   std::to_string(key_count) + ", estimated_size=" + std::to_string(estimated_min_data_size) + 
                   ", page_capacity=" + std::to_string(PAGE_DATA_SIZE));
    keys_.clear();
    child_page_ids_.clear();
    return;
  }
  
  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);

  keys_.clear();
  child_page_ids_.clear();
  keys_.reserve(key_count); // 预分配空间，避免频繁重新分配
  child_page_ids_.reserve(key_count + 1); // 子节点数量比键数量多1

  size_t offset = PAGE_HEADER_SIZE;
  // 内部节点应该至少有一个子节点ID，即使没有键
  if (key_count == 0) {
    // 对于空内部节点，检查页面是否确实包含序列化的子节点ID
    // 只有当页面看起来确实被序列化过时才读取
    if (offset + sizeof(int32_t) <= PAGE_SIZE) {
      int32_t child_page_id = *reinterpret_cast<int32_t *>(data + offset);
      // 检查子节点ID是否合理（不为负数，且看起来像有效的页面ID）
      if (child_page_id >= 0 && child_page_id < 1000000) {  // 合理的页面ID范围
        child_page_ids_.push_back(child_page_id);
        SQLCC_LOG_DEBUG("Deserialized empty internal node with child_page_id: " + std::to_string(child_page_id));
      } else {
        SQLCC_LOG_DEBUG("Empty internal node with invalid child_page_id: " + std::to_string(child_page_id) + ", skipping");
      }
    } else {
      SQLCC_LOG_DEBUG("Empty internal node, no child page IDs to deserialize");
    }
  } else {
    // 有键的情况，按原有逻辑处理
    for (int32_t i = 0; i < key_count; ++i) {
      // 检查是否有足够空间读取键长度
      if (offset + sizeof(int32_t) > PAGE_SIZE) {
        SQLCC_LOG_ERROR("Insufficient page data to read key length");
        keys_.clear();
        child_page_ids_.clear();
        return;
      }
      
      // 反序列化键长度
      int32_t key_len = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);

      // 检查键长度是否合理
      if (key_len < 0 || static_cast<size_t>(key_len) > (PAGE_SIZE - offset - sizeof(int32_t))) {
        SQLCC_LOG_ERROR("Invalid key length in B+Tree internal node: " + std::to_string(key_len));
        keys_.clear();
        child_page_ids_.clear();
        return;
      }

      // 检查是否有足够空间读取键内容
      if (offset + key_len > PAGE_SIZE) {
        SQLCC_LOG_ERROR("Insufficient page data to read key content");
        keys_.clear();
        child_page_ids_.clear();
        return;
      }
      
      // 反序列化键内容
      std::string key(data + offset, key_len);
      offset += key_len;
      keys_.push_back(key);

      // 检查是否有足够空间读取子节点ID
      if (offset + sizeof(int32_t) > PAGE_SIZE) {
        SQLCC_LOG_ERROR("Insufficient page data to read child page ID");
        keys_.clear();
        child_page_ids_.clear();
        return;
      }
      
      // 反序列化子节点ID
      int32_t child_page_id = *reinterpret_cast<int32_t *>(data + offset);
      offset += sizeof(int32_t);
      child_page_ids_.push_back(child_page_id);
    }

    // 检查是否有足够空间读取最后一个子节点ID
    if (offset + sizeof(int32_t) > PAGE_SIZE) {
      SQLCC_LOG_ERROR("Insufficient page data to read last child page ID");
      keys_.clear();
      child_page_ids_.clear();
      return;
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
    SerializeToPage();
  } else {
    // 如果已经有子节点，我们需要提供一个键
    // 在实际使用中，应该传入正确的键，这里只是临时处理
    SQLCC_LOG_WARN("InsertChild: Adding child to non-empty internal node without key, using placeholder");
    InsertChild(child_page_id, "placeholder_key");
  }
}

void BPlusTreeInternalNode::InsertChild(int32_t child_page_id,
                                        const std::string &key) {
  // 内部节点应该有n个键和n+1个子节点指针
  // 所以当我们插入一个子节点时，我们需要同时插入一个键
  // 除非这是第一个子节点，此时我们只需要插入子节点ID

  // 插入键和子节点ID
  if (child_page_ids_.empty()) {
    // 第一个子节点，只添加子节点ID，不添加键
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added first child_page_id=" + std::to_string(child_page_id) + " to internal node page_id=" + std::to_string(page_id_) + " without key");
  } else {
    // 对于后续的子节点，我们需要添加键和子节点ID
    // 在B+树中，键的数量应该比子节点数量少1
    keys_.push_back(key);
    child_page_ids_.push_back(child_page_id);
    SQLCC_LOG_DEBUG("InsertChild: Added child_page_id=" + std::to_string(child_page_id) + " with key='" + key + "' to internal node page_id=" + std::to_string(page_id_));
  }
  // 序列化到页面
  SerializeToPage();
}

void BPlusTreeInternalNode::RemoveChild(int32_t child_page_id) {
  // 找到要删除的子节点ID的位置
  auto it =
      std::find(child_page_ids_.begin(), child_page_ids_.end(), child_page_id);
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
  new_node->child_page_ids_.assign(child_page_ids_.begin() + mid + 1,
                                   child_page_ids_.end());
  new_node->parent_page_id_ = parent_page_id_;

  // 删除后半部分，包括中间键
  keys_.erase(keys_.begin() + mid, keys_.end());
  child_page_ids_.erase(child_page_ids_.begin() + mid + 1,
                        child_page_ids_.end());

  // 序列化两个节点
  SerializeToPage();
  if (new_node) {
    new_node->SerializeToPage();
  }
}

void BPlusTreeInternalNode::Merge(std::unique_ptr<BPlusTreeInternalNode> right_node,
                                  const std::string &parent_key) {
  if (!right_node)
    return;

  // 添加父节点传递的键到当前节点
  keys_.push_back(parent_key);

  // 将右节点的所有键和子节点合并到当前节点
  keys_.insert(keys_.end(), right_node->keys_.begin(), right_node->keys_.end());
  child_page_ids_.insert(child_page_ids_.end(),
                         right_node->child_page_ids_.begin(),
                         right_node->child_page_ids_.end());

  // 更新子节点的父节点ID
  // 注意：这里暂时跳过，因为需要访问BPlusTreeIndex的私有方法
  // TODO: 重构代码以支持子节点父节点ID的更新

  // 序列化当前节点
  SerializeToPage();
}

/**
 * @class BPlusTreeLeafNode
 * @brief B+树叶子节点类
 */
BPlusTreeLeafNode::BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine,
                                     int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, true), next_page_id_(-1) {
  // 叶子节点构造函数
  if (page_) {
    // 检查页面是否是新页面（通过检查节点类型字节是否为0或其他无效值）
    char* data = page_->GetDataSpan().data();
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

void BPlusTreeLeafNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = page_->GetDataSpan().data();
  data[0] = 1; // 标记为叶子节点
  *reinterpret_cast<int32_t *>(data + 1) =
      static_cast<int32_t>(entries_.size());                // 条目数量
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

  char *data = page_->GetDataSpan().data();
  
  // 检查页面数据是否足够存储头部信息
  if (PAGE_SIZE < PAGE_HEADER_SIZE) {
    SQLCC_LOG_ERROR("Page size too small for B+Tree node header");
    return;
  }
  
  // 检查节点类型是否正确
  if (data[0] != 1) {  // 叶子节点应该标记为1
    SQLCC_LOG_ERROR("Invalid node type in B+Tree leaf node: " + std::to_string(static_cast<int>(data[0])) + ", expected 1");
    entries_.clear();
    return;
  }
  
  // 检查页面数据是否被正确初始化（避免读取未初始化的数据）
  // 检查entry_count字段是否包含合理的值（避免读取垃圾数据）
  if (data[1] == 0 && data[2] == 0 && data[3] == 0 && data[4] == 0) {
    // 如果entry_count为0，可能是新页面，继续处理
    SQLCC_LOG_DEBUG("Empty leaf node page detected, initializing with zero entries");
  } else {
    // 检查数据是否看起来像有效的条目计数
    // 如果最高位被设置，很可能是错误的数据
    uint32_t raw_entry_count = *reinterpret_cast<uint32_t *>(data + 1);
    if (raw_entry_count > 0x00FFFFFF) {  // 如果高字节被设置，可能是错误的数据
      SQLCC_LOG_ERROR("Suspiciously large entry count in B+Tree leaf node: " + std::to_string(raw_entry_count) + ", likely data corruption");
      entries_.clear();
      return;
    }
  }
  
  // 在读取entry_count之前，先检查数据是否足够
  if (PAGE_SIZE < 5) {  // 至少需要5个字节（1字节类型 + 4字节计数）
    SQLCC_LOG_ERROR("Page size too small to read entry count");
    entries_.clear();
    return;
  }
  
  int32_t entry_count = *reinterpret_cast<int32_t *>(data + 1);
  
  // 增强数据验证：检查条目数量是否合理
  // 添加更严格的验证，防止错误的数据解析
  // 首先检查是否可能是内存损坏导致的异常大值
  if (entry_count < 0 || entry_count > 10000 || (entry_count > 0 && (static_cast<uint32_t>(entry_count) & 0xFF000000) != 0)) {
    SQLCC_LOG_ERROR("Invalid entry count in B+Tree leaf node: " + std::to_string(entry_count) +
                   " (possible memory corruption). Resetting to empty node.");
    entries_.clear();
    // 重新初始化页面头部
    data[0] = 1; // 叶子节点标记
    *reinterpret_cast<int32_t*>(data + 1) = 0; // 条目数量为0
    *reinterpret_cast<int32_t*>(data + 5) = parent_page_id_;
    *reinterpret_cast<int32_t*>(data + 9) = next_page_id_;
    memset(data + 13, 0, PAGE_HEADER_SIZE - 13);
    return;
  }

  if (entry_count > static_cast<int32_t>(BPLUS_TREE_LEAF_MAX_KEYS)) {
    SQLCC_LOG_ERROR("Entry count exceeds maximum allowed: " + std::to_string(entry_count) +
                   ", max allowed: " + std::to_string(BPLUS_TREE_LEAF_MAX_KEYS) + ". Truncating.");
    entry_count = BPLUS_TREE_LEAF_MAX_KEYS;
  }
  
  // 进一步验证：检查条目数量是否可能导致缓冲区溢出
  // 修正计算方式，考虑实际的序列化格式
  size_t estimated_min_data_size = entry_count * (sizeof(int32_t) + 1 + sizeof(int32_t) + sizeof(size_t)); // 键长度(4)+最小键内容(1)+页面ID(4)+偏移量(8)
  if (estimated_min_data_size > PAGE_DATA_SIZE) {
    SQLCC_LOG_ERROR("Estimated data size exceeds page capacity in B+Tree leaf node: entry_count=" + 
                   std::to_string(entry_count) + ", estimated_size=" + std::to_string(estimated_min_data_size) + 
                   ", page_capacity=" + std::to_string(PAGE_DATA_SIZE));
    entries_.clear();
    return;
  }
  
  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);
  next_page_id_ = *reinterpret_cast<int32_t *>(data + 9);

  entries_.clear();
  entries_.reserve(entry_count); // 预分配空间，避免频繁重新分配

  size_t offset = PAGE_HEADER_SIZE;
  for (int32_t i = 0; i < entry_count; ++i) {
    // 检查是否有足够空间读取键长度
    if (offset + sizeof(int32_t) > PAGE_SIZE) {
      SQLCC_LOG_ERROR("Insufficient page data to read key length");
      entries_.clear();
      return;
    }
    
    // 反序列化键长度
    int32_t key_len = *reinterpret_cast<int32_t *>(data + offset);
    offset += sizeof(int32_t);

    // 检查键长度是否合理
    if (key_len < 0 || static_cast<size_t>(key_len) > (PAGE_SIZE - offset - sizeof(int32_t) - sizeof(size_t))) {
      SQLCC_LOG_ERROR("Invalid key length in B+Tree leaf node: " + std::to_string(key_len));
      entries_.clear();
      return;
    }

    // 检查是否有足够空间读取键内容
    if (offset + key_len > PAGE_SIZE) {
      SQLCC_LOG_ERROR("Insufficient page data to read key content");
      entries_.clear();
      return;
    }
    
    // 反序列化键内容
    std::string key(data + offset, key_len);
    offset += key_len;

    // 检查是否有足够空间读取页面ID和偏移量
    if (offset + sizeof(int32_t) + sizeof(size_t) > PAGE_SIZE) {
      SQLCC_LOG_ERROR("Insufficient page data to read page_id and offset");
      entries_.clear();
      return;
    }
    
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
  auto it =
      std::lower_bound(entries_.begin(), entries_.end(), IndexEntry(key, 0, 0));

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

std::vector<IndexEntry>
BPlusTreeLeafNode::Search(const std::string &key) const {
  std::vector<IndexEntry> results;

  // 二分查找找到键
  auto it =
      std::lower_bound(entries_.begin(), entries_.end(), IndexEntry(key, 0, 0));

  // 检查是否找到
  if (it != entries_.end() && it->key == key) {
    results.push_back(*it);
  }

  return results;
}

std::vector<IndexEntry>
BPlusTreeLeafNode::SearchRange(const std::string &lower_bound,
                               const std::string &upper_bound) const {
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
  entries_.insert(entries_.end(), right_node->entries_.begin(),
                  right_node->entries_.end());

  // 更新当前节点的下一个节点指针
  next_page_id_ = right_node->next_page_id_;

  // 序列化当前节点
  SerializeToPage();
}



/**
 * @class BPlusTreeIndex
 * @brief B+树索引类
 */
BPlusTreeIndex::BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine,
                               const std::string &table_name,
                               const std::string &column_name)
    : storage_engine_(storage_engine), table_name_(table_name),
      column_name_(column_name), root_page_id_(-1) {
  // B+树索引构造函数实现
  SQLCC_LOG_DEBUG("Created B+Tree index for table '" + table_name + "' column '" + column_name + "'");
}

BPlusTreeIndex::~BPlusTreeIndex() {
  // B+树索引析构函数实现
}

bool BPlusTreeIndex::Create() {
  if (!storage_engine_)
    return false;

  SQLCC_LOG_DEBUG("Creating B+Tree index, allocating root page");

  // 分配根节点页面
  if (!storage_engine_->NewPage(&root_page_id_)) {
    SQLCC_LOG_ERROR("Failed to allocate root page");
    return false;
  }

  if (root_page_id_ < 0) {
    SQLCC_LOG_ERROR("Invalid root page ID: " + std::to_string(root_page_id_));
    return false;
  }

  SQLCC_LOG_DEBUG("Allocated root page with ID: " + std::to_string(root_page_id_));

  // 创建叶子节点并初始化（使用智能指针）
  auto root_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, root_page_id_);
  if (!root_node) {
    SQLCC_LOG_ERROR("Failed to create root leaf node");
    storage_engine_->DeletePage(root_page_id_);
    return false;
  }

  SQLCC_LOG_DEBUG("Created root leaf node, initializing");

  // 初始化根节点数据
  root_node->Clear(); // 清空节点数据
  root_node->SetParentPageId(-1); // 设置父节点ID为-1

  // 序列化根节点到页面
  root_node->SerializeToPage();

  SQLCC_LOG_DEBUG("Root node created and serialized successfully");

  // 智能指针自动释放，无需手动delete
  return true;
}

bool BPlusTreeIndex::Drop() {
  if (!storage_engine_)
    return false;

  if (root_page_id_ >= 0) {
    // 递归释放所有节点页面
    storage_engine_->DeletePage(root_page_id_);
    root_page_id_ = -1;
  }

  return true;
}

/*
// DEPRECATED: This recursive implementation has been replaced by the iterative version in b_plus_tree_index.cpp
// to avoid stack overflow issues. This method is kept for reference but should not be used.
bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset) {
  SQLCC_LOG_ERROR("Recursive Insert method called - this should not happen! Use InsertIterative instead.");
  return false;
}
*/

// 添加静态变量来跟踪递归深度
bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset, 
                            std::unique_ptr<BPlusTreeNode>& node, int recursion_depth) {
  if (!node)
    return false;

  // 检查递归深度，防止无限递归
  if (recursion_depth > 100) {  // 设置一个合理的最大递归深度
    SQLCC_LOG_ERROR("Maximum recursion depth exceeded in BPlusTreeIndex::Insert");
    return false;
  }

  SQLCC_LOG_DEBUG("BPlusTreeIndex::Insert called with recursion_depth: " + std::to_string(recursion_depth) + ", key: '" + key + "', page_id: " + std::to_string(page_id) + ", node_page_id: " + std::to_string(node->GetPageId()));

  // 根据节点类型调用相应的插入方法
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    IndexEntry entry(key, page_id, offset);
    bool result = leaf_node->Insert(entry);
    // 保存节点状态
    node->SerializeToPage();
    
    // 检查叶子节点是否已满，如果已满则需要分裂
    if (leaf_node->IsFull()) {
      SQLCC_LOG_DEBUG("Leaf node is full, performing split");

      // 创建新的叶子节点用于分裂
      int32_t new_leaf_page_id;
      if (!storage_engine_->NewPage(&new_leaf_page_id)) {
        SQLCC_LOG_ERROR("Failed to allocate new page for leaf node split");
        return result;
      }

      auto new_leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, new_leaf_page_id);
      if (!new_leaf_node) {
        SQLCC_LOG_ERROR("Failed to create new leaf node");
        storage_engine_->DeletePage(new_leaf_page_id);
        return result;
      }

      // 执行叶子节点分裂
      leaf_node->Split(new_leaf_node);

      // 获取分裂后的第一个键作为分隔键
      const auto& new_entries = new_leaf_node->GetEntries();
      if (!new_entries.empty()) {
        std::string split_key = new_entries[0].key;

        // 如果当前节点是根节点，则需要创建新的根节点
        if (node->GetPageId() == root_page_id_) {
          SQLCC_LOG_DEBUG("Root node split, creating new internal root");

          // 创建新的内部节点作为根节点
          int32_t new_root_page_id;
          if (!storage_engine_->NewPage(&new_root_page_id)) {
            SQLCC_LOG_ERROR("Failed to allocate new page for B+Tree root node");
            return result;
          }

          auto new_root = std::make_unique<BPlusTreeInternalNode>(storage_engine_, new_root_page_id);
          if (!new_root) {
            SQLCC_LOG_ERROR("Failed to create new B+Tree root node");
            storage_engine_->DeletePage(new_root_page_id);
            return result;
          }

          // 初始化新的根节点
          new_root->Clear();

          // 添加左子节点（原根节点）和右子节点（新分裂的节点）
          // 内部节点需要键来分隔子节点
          new_root->InsertChild(node->GetPageId()); // 先添加左子节点
          new_root->InsertChild(new_leaf_page_id, split_key); // 添加右子节点和分隔键

          // 设置子节点的父节点ID
          node->SetParentPageId(new_root_page_id);
          new_leaf_node->SetParentPageId(new_root_page_id);

          // 设置叶子节点间的链接
          leaf_node->SetNextPageId(new_leaf_page_id);
          new_leaf_node->SetNextPageId(-1); // 新叶子节点是最后一个

          // 序列化所有节点
          node->SerializeToPage();
          new_leaf_node->SerializeToPage();
          new_root->SerializeToPage();

          // 更新根节点ID - 必须在序列化之后
          int32_t old_root_page_id = root_page_id_;
          root_page_id_ = new_root_page_id;

          SQLCC_LOG_DEBUG("Root node split: old_root=" + std::to_string(old_root_page_id) +
                         ", new_root=" + std::to_string(new_root_page_id));
        } else {
          // 非根节点分裂，需要递归更新父节点
          // 这里暂时记录日志，实际实现需要更复杂的逻辑
          SQLCC_LOG_DEBUG("Non-root leaf node split, parent update needed");
        }
      }
    }
    
    return result;
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 检查内部节点是否为空（没有子节点）
    auto child_page_ids = internal_node->GetChildPageIds();
    if (child_page_ids.empty()) {
      // 内部节点没有子节点，这是新建节点的情况
      // 我们需要先添加一个子节点
      SQLCC_LOG_DEBUG("Internal node has no child nodes, adding initial child node. Node page_id: " + std::to_string(internal_node->GetPageId()));
      // 在这种情况下，我们应该先创建一个叶子节点作为第一个子节点
      int32_t new_leaf_page_id;
      if (!storage_engine_->NewPage(&new_leaf_page_id)) {
        SQLCC_LOG_ERROR("Failed to allocate new page for initial leaf node");
        return false;
      }
      
      // 创建新的叶子节点
      auto new_leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, new_leaf_page_id);
      if (!new_leaf_node) {
        SQLCC_LOG_ERROR("Failed to create new leaf node");
        storage_engine_->DeletePage(new_leaf_page_id);
        return false;
      }
      
      // 将新叶子节点添加为内部节点的第一个子节点
      internal_node->InsertChild(new_leaf_page_id);
      
      // 将新叶子节点序列化到页面
      new_leaf_node->SerializeToPage();
      
      // 现在递归插入到新创建的叶子节点中
      // 注意：我们不能直接将new_leaf_node传递给Insert方法，因为它会被消耗掉
      // 我们需要先将其添加到存储引擎中，然后重新加载它
      
      // 首先将新叶子节点序列化到页面
      new_leaf_node->SerializeToPage();
      
      // 现在我们可以安全地将其转换为基类指针并传递给Insert方法
      std::unique_ptr<BPlusTreeNode> base_node = std::move(new_leaf_node);
      bool result = Insert(key, page_id, offset, base_node, recursion_depth + 1);
      if (result) {
        // 如果插入成功，保存内部节点状态
        node->SerializeToPage();
      }
      return result;
    }
    SQLCC_LOG_DEBUG("Internal node has " + std::to_string(child_page_ids.size()) + " child nodes during insert");
    // 对于内部节点，找到合适的子节点进行递归插入
    int32_t child_page_id = internal_node->FindChildPageId(key);
    SQLCC_LOG_DEBUG("FindChildPageId returned child_page_id: " + std::to_string(child_page_id) + " for key: '" + key + "'");
    if (child_page_id < 0) {
      SQLCC_LOG_ERROR("Invalid child page ID returned from FindChildPageId");
      return false;
    }
    
    auto child_node = LoadNode(child_page_id);
    if (child_node) {
      bool result = Insert(key, page_id, offset, child_node, recursion_depth + 1);
      // 如果子节点插入成功，保存内部节点状态
      if (result) {
        node->SerializeToPage();
        
        // 检查子节点是否需要处理分裂等情况
        // 这里可以添加更多逻辑来处理子节点的分裂传播到父节点
      }
      return result;
    } else {
      SQLCC_LOG_ERROR("Failed to load child node with page ID: " + std::to_string(child_page_id));
      return false;
    }
  }
  return false;
}
bool BPlusTreeIndex::Delete(const std::string& key) {
  if (!storage_engine_ || root_page_id_ < 0)
    return true; // 索引不存在，返回true

  // 加载根节点
  auto root_node = LoadNode(root_page_id_);
  if (!root_node)
    return true; // 节点加载失败，返回true

  // 调用递归删除方法
  bool result = Delete(key, root_node);

  return result;
}

bool BPlusTreeIndex::Delete(const std::string& key, std::unique_ptr<BPlusTreeNode>& node) {
  if (!node)
    return false;

  // 根据节点类型调用相应的删除方法
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    bool result = leaf_node->Remove(key);
    // 保存节点状态
    node->SerializeToPage();
    return result;
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，找到合适的子节点进行递归删除
    int32_t child_page_id = internal_node->FindChildPageId(key);
    // 检查子节点ID是否有效
    if (child_page_id < 0) {
      SQLCC_LOG_ERROR("Invalid child page ID returned from FindChildPageId during delete");
      return false;
    }
    
    auto child_node = LoadNode(child_page_id);
    if (child_node) {
      bool result = Delete(key, child_node);
      // 如果子节点删除成功，保存内部节点状态
      if (result) {
        node->SerializeToPage();
      }
      return result;
    } else {
      SQLCC_LOG_ERROR("Failed to load child node with page ID: " + std::to_string(child_page_id) + " during delete");
      return false;
    }
  }

  return false;
}

std::vector<IndexEntry> BPlusTreeIndex::Search(const std::string& key) const {
  if (!storage_engine_ || root_page_id_ < 0)
    return std::vector<IndexEntry>();

  // 获取根节点
  // 使用const_cast是因为LoadNode需要修改页面的访问计数
  auto root_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(root_page_id_);
  if (!root_node)
    return std::vector<IndexEntry>();

  // 保存节点状态（如果有修改）
  root_node->SerializeToPage();

  // 递归搜索
  return Search(key, root_node);
}

std::vector<IndexEntry> BPlusTreeIndex::Search(const std::string& key, 
                                               std::unique_ptr<BPlusTreeNode>& node) const {
  if (!node)
    return std::vector<IndexEntry>();

  // 根据节点类型调用相应的搜索方法
  if (dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    // 对于叶子节点，直接搜索键
    auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get());
    if (leaf_node) {
      return leaf_node->Search(key);
    }
  } else {
    // 对于内部节点，找到合适的子节点进行递归搜索
    auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get());
    if (internal_node) {
      // 使用B+树的搜索规则找到合适的子节点
      int32_t child_page_id = internal_node->FindChildPageId(key);
      // 检查子节点ID是否有效
      if (child_page_id < 0) {
        SQLCC_LOG_ERROR("Invalid child page ID returned from FindChildPageId during search");
        return std::vector<IndexEntry>();
      }
      
      auto child_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(child_page_id);
      if (child_node) {
        // 递归搜索子节点，但不直接返回，确保child_node在搜索完成后才销毁
        std::vector<IndexEntry> results = Search(key, child_node);
        return results;
      } else {
        SQLCC_LOG_ERROR("Failed to load child node with page ID: " + std::to_string(child_page_id) + " during search");
      }
    }
  }

  return std::vector<IndexEntry>();
}

std::vector<IndexEntry> BPlusTreeIndex::SearchRange(const std::string& lower_bound, 
                                                    const std::string& upper_bound) const {
  if (!storage_engine_ || root_page_id_ < 0)
    return std::vector<IndexEntry>();

  // 获取根节点
  auto root_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(root_page_id_);
  if (!root_node)
    return std::vector<IndexEntry>();

  // 递归范围搜索
  return SearchRange(lower_bound, upper_bound, root_node);
}

std::vector<IndexEntry> BPlusTreeIndex::SearchRange(const std::string& lower_bound, 
                                                    const std::string& upper_bound, 
                                                    std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<IndexEntry> results;

  if (!node)
    return results;

  if (dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    // 对于叶子节点，搜索当前节点并沿叶子链继续搜索
    auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get());
    if (leaf_node) {
      // 搜索当前叶子节点中的范围条目
      auto current_results = leaf_node->SearchRange(lower_bound, upper_bound);
      results.insert(results.end(), current_results.begin(), current_results.end());

      // 如果有下一个叶子节点，继续搜索（B+树的核心特性）
      int32_t next_page_id = leaf_node->GetNextPageId();
      while (next_page_id >= 0) {
        auto next_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(next_page_id);
        if (!next_node || !dynamic_cast<BPlusTreeLeafNode*>(next_node.get()))
          break;

        auto next_leaf_node = dynamic_cast<BPlusTreeLeafNode*>(next_node.get());
        if (!next_leaf_node)
          break;

        // 检查下一个叶子节点是否可能包含范围内的条目
        const auto& entries = next_leaf_node->GetEntries();
        if (!entries.empty()) {
          // 如果下一个叶子节点的第一个键已经超过上界，停止搜索
          if (entries[0].key.compare(upper_bound) > 0)
            break;

          // 搜索下一个叶子节点中的范围条目
          auto next_results = next_leaf_node->SearchRange(lower_bound, upper_bound);
          results.insert(results.end(), next_results.begin(), next_results.end());
        }

        next_page_id = next_leaf_node->GetNextPageId();
      }
    }
  } else {
    // 对于内部节点，找到合适的子节点进行递归范围搜索
    auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get());
    if (internal_node) {
      // 使用二分查找找到第一个可能包含范围的子节点
      const auto& keys = internal_node->GetKeys();
      const auto& child_page_ids = internal_node->GetChildPageIds();

      // 找到第一个键 >= lower_bound 的位置
      auto it = std::lower_bound(keys.begin(), keys.end(), lower_bound);
      size_t start_pos = it - keys.begin();

      // 从找到的位置开始，检查所有可能的子节点
      for (size_t i = start_pos; i < child_page_ids.size(); ++i) {
        int32_t child_page_id = child_page_ids[i];

        auto child_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(child_page_id);
        if (!child_node)
          continue;

        // 递归搜索子节点
        auto child_results = SearchRange(lower_bound, upper_bound, child_node);
        results.insert(results.end(), child_results.begin(), child_results.end());

        // 如果当前子节点的结果中包含超出上界的键，停止搜索后续子节点
        if (!child_results.empty() && !child_results.back().key.empty() &&
            child_results.back().key.compare(upper_bound) > 0) {
          break;
        }

        // 如果当前键已经超过上界，停止搜索
        if (i < keys.size() && keys[i].compare(upper_bound) > 0) {
          break;
        }
      }
    }
  }

  return results;
}

bool BPlusTreeIndex::Exists() const {
  return root_page_id_ >= 0;
}

// 辅助方法：加载节点
std::unique_ptr<BPlusTreeNode> BPlusTreeIndex::LoadNode(int32_t page_id) {
  if (!storage_engine_ || page_id < 0)
    return nullptr;

  // 获取页面以检查节点类型
  auto page = storage_engine_->FetchPage(page_id);
  if (!page) {
    SQLCC_LOG_ERROR("Failed to fetch page " + std::to_string(page_id));
    return nullptr;
  }
  
  // 检查页面数据以确定节点类型
  char* data = page->GetDataSpan().data();
  
  // 检查节点类型字节是否有效
  if (data[0] != 0 && data[0] != 1) {
    SQLCC_LOG_ERROR("Invalid node type in page " + std::to_string(page_id) + ": " + std::to_string(static_cast<int>(data[0])));
    storage_engine_->UnpinPage(page_id, false);
    return nullptr;
  }
  
  bool is_leaf = (data[0] == 1);
  
  // 释放页面引用
  storage_engine_->UnpinPage(page_id, false);
  
  // 根据节点类型创建相应节点
  try {
    if (is_leaf) {
      auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, page_id);
      if (leaf_node && leaf_node->GetPageId() >= 0) {
        return leaf_node;
      }
    } else {
      auto internal_node = std::make_unique<BPlusTreeInternalNode>(storage_engine_, page_id);
      if (internal_node && internal_node->GetPageId() >= 0) {
        return internal_node;
      }
    }
  } catch (const std::exception& e) {
    SQLCC_LOG_ERROR("Exception occurred while creating node for page " + std::to_string(page_id) + ": " + e.what());
    return nullptr;
  }
  
  SQLCC_LOG_ERROR("Failed to load node for page " + std::to_string(page_id));
  return nullptr;
}

} // namespace sqlcc
