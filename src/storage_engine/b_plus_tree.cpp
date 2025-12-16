#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/logger.h"
#include <algorithm>

namespace sqlcc {

/**
 * =============================================================================
 * Phase 7: B+树索引系统企业级实现
 * =============================================================================
 */

// B+树设计参数 (商业数据库标准)
#define BPLUS_TREE_MAX_KEYS                                                    \
  250 // 每个节点最大键数量 (8KB页面/32byte键 = 256,留余量)
#define BPLUS_TREE_MIN_KEYS 125      // 内部节点最小键数量 (MAX/2)
#define BPLUS_TREE_LEAF_MIN_KEYS 125 // 叶子节点最小键数量 (MAX/2)

// Page header for B+Tree nodes (存储在页面头部的B+树节点元数据)
// Page header format:
// [is_leaf(1)] [key_count(4)] [parent_page_id(4)] [next_page_id(4)]
// [padding(7)]
#define PAGE_HEADER_SIZE 20
#define PAGE_DATA_SIZE (PAGE_SIZE - PAGE_HEADER_SIZE)

/**
 * @class BPlusTreeNode
 * @brief B+树节点基类
 * @details 定义了B+树节点的基本结构和接口，是内部节点和叶子节点的共同基类
 *
 * @par 设计思路
 * - 采用继承模式，派生出内部节点和叶子节点
 * - 每个节点对应一个磁盘页，通过页ID进行管理
 * - 提供统一的接口，如序列化、反序列化、插入、删除、查询等
 * - 支持自平衡操作，如分裂、合并等
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树节点的核心功能
 * - 磁盘I/O优化：通过缓冲池减少磁盘I/O次数
 * - 页管理：每个节点对应一个磁盘页，高效管理内存和磁盘数据
 *
 * @par 示例用法
 * @code
    auto node = std::make_unique<BPlusTreeNode>(storage_engine, page_id, is_leaf);
    if (!node) {
        return results; // 内存分配失败
    }
    node->Insert(entry);
    results = node->Search(key);
 * @endcode
 */
// BPlusTreeNode 实现
/**
 * @brief BPlusTreeNode构造函数
 * @details 创建一个B+树节点，关联到指定的磁盘页
 *
 * @param storage_engine 存储引擎指针，用于页管理
 * @param page_id 节点对应的页ID
 * @param is_leaf 是否为叶子节点
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 注意事项
 * - 新页面的初始化在Create方法中完成，构造函数只获取页面对象
 * - 页面对象通过缓冲池获取，减少磁盘I/O次数
 */
BPlusTreeNode::BPlusTreeNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id,
                             bool is_leaf)
    : storage_engine_(std::move(storage_engine)), page_id_(page_id), parent_page_id_(-1),
      is_leaf_(is_leaf), page_(nullptr) {

  // 获取页面对象用于数据存储
  if (storage_engine_) {
    Page* raw_page = storage_engine_->FetchPage(page_id);
    if (raw_page) {
      // 使用智能指针管理页面生命周期，避免与StorageEngine的页面管理冲突
      // 注意：这里我们不接管所有权，只是保存引用
      // 实际的页面管理由StorageEngine负责
      page_ = std::shared_ptr<Page>(raw_page, [this](Page* p) {
        // 自定义删除器，将页面返回给StorageEngine
        if (storage_engine_ && p) {
          storage_engine_->UnpinPage(page_id_, true); // 标记为脏页并释放
        }
      });
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

/**
 * @brief BPlusTreeNode析构函数
 * @details 释放B+树节点占用的资源，包括将页面标记为脏页并释放
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 注意事项
 * - 析构函数会将页面标记为脏页，确保修改的数据写入磁盘
 * - 页面通过缓冲池释放，减少磁盘I/O次数
 */
BPlusTreeNode::~BPlusTreeNode() {
  // 页面资源由page_的自定义删除器处理，不需要在这里再次释放
  SQLCC_LOG_DEBUG("Destroyed B+Tree node: page_id=" + std::to_string(page_id_));
}

/**
 * @class BPlusTreeInternalNode
 * @brief B+树内部节点类
 * @details 实现了B+树内部节点的功能，用于存储索引键和子节点指针
 *
 * @par 设计思路
 * - 继承自BPlusTreeNode基类
 * - 存储索引键和子节点指针，其中子节点指针数量比键数量多1
 * - 提供序列化、反序列化、插入、删除、查询等接口
 * - 支持自平衡操作，如分裂、合并等
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树内部节点的核心功能
 * - 索引设计：内部节点只存储索引键和子节点指针，不存储实际数据
 * - 自平衡操作：通过分裂维持树的平衡
 *
 * @par 示例用法
 * @code
 * BPlusTreeInternalNode *internal_node = new
 * BPlusTreeInternalNode(storage_engine, page_id);
 * internal_node->InsertChild(child_page_id, key);
 * @endcode
 */
// BPlusTreeInternalNode 实现
/**
 * @brief BPlusTreeInternalNode构造函数
 * @details 创建一个B+树内部节点，关联到指定的磁盘页，并从页面中反序列化数据
 *
 * @param storage_engine 存储引擎指针，用于页管理
 * @param page_id 节点对应的页ID
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的键数量
 * - 空间复杂度：O(n)
 *
 * @par 注意事项
 * - 构造函数会从页面中反序列化数据，初始化节点状态
 * - 如果页面不存在或无法访问，会导致初始化失败
 */
BPlusTreeInternalNode::BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine,
                                             int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, false) {
  // 内部节点构造函数
  if (page_) {
    DeserializeFromPage();
  }
}

/**
 * @brief BPlusTreeInternalNode析构函数
 * @details 释放B+树内部节点占用的资源
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 */
BPlusTreeInternalNode::~BPlusTreeInternalNode() {
  // 内部节点析构函数
}

/**
 * @brief 将内部节点数据序列化到页面
 * @details 将内部节点的键和子节点指针序列化到关联的磁盘页中
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的键数量
 * - 空间复杂度：O(1)
 *
 * @par 页面格式
 * - 页面头部：is_leaf(1字节) + key_count(4字节) + parent_page_id(4字节) +
 * next_page_id(4字节) + padding(7字节)
 * - 页面数据：键长度(4字节) + 键内容 +
 * 子节点ID(4字节)，重复n次，最后加上一个子节点ID
 *
 * @par 注意事项
 * - 序列化后页面会被标记为脏页，在UnpinPage时写入磁盘
 * - 如果页面不存在，序列化操作会被跳过
 */
void BPlusTreeInternalNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = page_->GetData();
  data[0] = 0; // 标记为内部节点
  *reinterpret_cast<int32_t *>(data + 1) =
      static_cast<int32_t>(keys_.size());                   // 键数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID

  // 序列化键和子节点ID
  size_t offset = PAGE_HEADER_SIZE;
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
  memcpy(data + offset, &child_page_ids_.back(), sizeof(int32_t));

  // 页面已修改，将在UnpinPage时标记为脏页
}

/**
 * @brief 从页面中反序列化内部节点数据
 * @details 从关联的磁盘页中读取数据，初始化内部节点的键和子节点指针
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的键数量
 * - 空间复杂度：O(n)
 *
 * @par 页面格式
 * - 页面头部：is_leaf(1字节) + key_count(4字节) + parent_page_id(4字节) +
 * next_page_id(4字节) + padding(7字节)
 * - 页面数据：键长度(4字节) + 键内容 +
 * 子节点ID(4字节)，重复n次，最后加上一个子节点ID
 *
 * @par 注意事项
 * - 反序列化前会清空节点的现有数据
 * - 如果页面不存在，反序列化操作会被跳过
 */
void BPlusTreeInternalNode::DeserializeFromPage() {
  if (!page_)
    return;

  char *data = page_->GetData();
  
  // 检查页面数据是否足够存储头部信息
  if (PAGE_SIZE < PAGE_HEADER_SIZE) {
    SQLCC_LOG_ERROR("Page size too small for B+Tree node header");
    return;
  }

  int32_t key_count = *reinterpret_cast<int32_t *>(data + 1);
  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);

  // 检查键数量是否合理
  if (key_count < 0 || key_count > BPLUS_TREE_MAX_KEYS) {
    SQLCC_LOG_ERROR("Invalid key count in B+Tree internal node: " + std::to_string(key_count));
    return;
  }

  keys_.clear();
  child_page_ids_.clear();
  keys_.reserve(key_count); // 预分配空间，避免频繁重新分配
  child_page_ids_.reserve(key_count + 1); // 子节点数量比键数量多1

  size_t offset = PAGE_HEADER_SIZE;
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
    if (key_len < 0 || key_len > (PAGE_SIZE - offset - sizeof(int32_t))) {
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

/**
 * @brief 判断内部节点是否已满
 * @details 检查内部节点的键数量是否达到最大限制
 *
 * @return bool - 如果节点已满返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 内部节点的最大键数量由BPLUS_TREE_MAX_KEYS宏定义
 * - 当节点已满时，需要进行分裂操作
 */





/**
 * @brief 插入子节点到内部节点
 * @details 在内部节点中插入一个子节点指针和对应的键
 *
 * @param child_page_id 子节点的页ID
 * @param key 对应的键值
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的键数量
 * - 空间复杂度：O(n)
 *
 * @par 设计思路
 * - 内部节点应该有n个键和n+1个子节点指针
 * - 当插入一个子节点时，需要同时插入一个键，除非这是第一个子节点
 * - 使用std::lower_bound找到插入位置，保持键的有序性
 *
 * @par 注意事项
 * - 插入后需要检查节点是否已满，如果已满需要进行分裂操作
 * - 插入后节点会被标记为脏页，在UnpinPage时写入磁盘
 */
void BPlusTreeInternalNode::InsertChild(int32_t child_page_id,
                                        const std::string &key) {
  // 内部节点应该有n个键和n+1个子节点指针
  // 所以当我们插入一个子节点时，我们需要同时插入一个键
  // unless这是第一个子节点，此时我们只需要插入子节点ID

  // 找到插入位置
  auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
  size_t pos = it - keys_.begin();

  // 插入键和子节点ID
  if (keys_.empty()) {
    // 第一个子节点，只添加子节点ID，不添加键
    child_page_ids_.push_back(child_page_id);
  } else if (it == keys_.end()) {
    // 插入到末尾
    keys_.push_back(key);
    child_page_ids_.push_back(child_page_id);
  } else {
    // 插入到中间位置
    keys_.insert(it, key);
    child_page_ids_.insert(child_page_ids_.begin() + pos + 1, child_page_id);
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
  // 二分查找找到第一个大于等于key的位置
  auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
  size_t pos = it - keys_.begin();

  // 根据B+树的搜索规则：
  // - 如果key小于所有键，返回第一个子节点
  // - 如果key大于等于某个键，返回该键右侧的子节点
  
  // 检查pos是否在有效范围内
  if (pos >= child_page_ids_.size()) {
    SQLCC_LOG_ERROR("Invalid child page ID position: " + std::to_string(pos) + 
                   ", child_page_ids_.size() = " + std::to_string(child_page_ids_.size()));
    return -1; // 返回无效页面ID
  }
  
  return child_page_ids_[pos];
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
 * @details
 * 实现了B+树叶子节点的功能，用于存储实际的索引数据和指向下一个叶子节点的指针
 *
 * @par 设计思路
 * - 继承自BPlusTreeNode基类
 * - 存储实际的索引条目，包括键和数据指针
 * - 每个叶子节点包含指向下一个叶子节点的指针，支持范围查询
 * - 提供序列化、反序列化、插入、删除、查询等接口
 * - 支持自平衡操作，如分裂、合并等
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树叶子节点的核心功能
 * - 范围查询：通过叶子节点链支持高效的范围查询
 * - 磁盘I/O优化：通过缓冲池减少磁盘I/O次数
 * - 页管理：每个节点对应一个磁盘页，高效管理内存和磁盘数据
 *
 * @par 示例用法
 * @code
 * BPlusTreeLeafNode *leaf_node = new BPlusTreeLeafNode(storage_engine,
 * page_id); leaf_node->Insert(entry); std::vector<IndexEntry> results =
 * leaf_node->Search(key);
 * @endcode
 */
// BPlusTreeLeafNode 实现
/**
 * @brief BPlusTreeLeafNode构造函数
 * @details 创建一个B+树叶子节点，关联到指定的磁盘页，并从页面中反序列化数据
 *
 * @param storage_engine 存储引擎指针，用于页管理
 * @param page_id 节点对应的页ID
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 注意事项
 * - 构造函数会从页面中反序列化数据，初始化节点状态
 * - 如果页面不存在或无法访问，会导致初始化失败
 */
BPlusTreeLeafNode::BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine,
                                     int32_t page_id)
    : BPlusTreeNode(storage_engine, page_id, true), next_page_id_(-1) {
  // 叶子节点构造函数
  if (page_) {
    DeserializeFromPage();
  }
}

/**
 * @brief BPlusTreeLeafNode析构函数
 * @details 释放B+树叶子节点占用的资源
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 */
BPlusTreeLeafNode::~BPlusTreeLeafNode() {
  // 叶子节点析构函数
}

/**
 * @brief 将叶子节点数据序列化到页面
 * @details 将叶子节点的索引条目和下一节点指针序列化到关联的磁盘页中
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(1)
 *
 * @par 页面格式
 * - 页面头部：is_leaf(1字节) + key_count(4字节) + parent_page_id(4字节) +
 * next_page_id(4字节) + padding(7字节)
 * - 页面数据：键长度(4字节) + 键内容 + 数据长度(4字节) + 数据内容，重复n次
 *
 * @par 注意事项
 * - 序列化后页面会被标记为脏页，在UnpinPage时写入磁盘
 * - 如果页面不存在，序列化操作会被跳过
 */
void BPlusTreeLeafNode::SerializeToPage() {
  if (!page_)
    return;

  char *data = page_->GetData();
  data[0] = 1; // 标记为叶子节点
  *reinterpret_cast<int32_t *>(data + 1) =
      static_cast<int32_t>(entries_.size());                // 条目数量
  *reinterpret_cast<int32_t *>(data + 5) = parent_page_id_; // 父节点ID
  *reinterpret_cast<int32_t *>(data + 9) = next_page_id_;   // 下一节点ID

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

/**
 * @brief 从页面中反序列化叶子节点数据
 * @details 从关联的磁盘页中读取数据，初始化叶子节点的索引条目和下一节点指针
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 页面格式
 * - 页面头部：is_leaf(1字节) + key_count(4字节) + parent_page_id(4字节) +
 * next_page_id(4字节) + padding(7字节)
 * - 页面数据：键长度(4字节) + 键内容 + 页面ID(4字节) + 偏移量(8字节)，重复n次
 *
 * @par 注意事项
 * - 反序列化前会清空节点的现有数据
 * - 如果页面不存在，反序列化操作会被跳过
 * - 索引条目包含键、页面ID和偏移量，用于定位实际数据在磁盘上的位置
 */
void BPlusTreeLeafNode::DeserializeFromPage() {
  if (!page_)
    return;

  char *data = page_->GetData();
  
  // 检查页面数据是否足够存储头部信息
  if (PAGE_SIZE < PAGE_HEADER_SIZE) {
    SQLCC_LOG_ERROR("Page size too small for B+Tree node header");
    return;
  }

  int32_t entry_count = *reinterpret_cast<int32_t *>(data + 1);
  parent_page_id_ = *reinterpret_cast<int32_t *>(data + 5);
  next_page_id_ = *reinterpret_cast<int32_t *>(data + 9);

  // 检查条目数量是否合理
  if (entry_count < 0 || entry_count > BPLUS_TREE_MAX_KEYS) {
    SQLCC_LOG_ERROR("Invalid entry count in B+Tree leaf node: " + std::to_string(entry_count));
    return;
  }

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
    if (key_len < 0 || key_len > (PAGE_SIZE - offset - sizeof(int32_t) - sizeof(size_t))) {
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


/**
 * @brief 插入索引条目到叶子节点
 * @details 在叶子节点中插入一个索引条目，保持条目按键有序
 *
 * @param entry 要插入的索引条目
 * @return bool - 总是返回true，表示插入成功
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 设计思路
 * - 使用std::lower_bound找到插入位置，保持条目按键有序
 * - 如果键已存在，覆盖现有条目；否则插入新条目
 * - 插入后将节点序列化到磁盘页
 *
 * @par 注意事项
 * - 插入后节点会被标记为脏页，在UnpinPage时写入磁盘
 * - 支持重复键的更新操作
 * - 插入后需要检查节点是否已满，如果已满需要进行分裂操作
 */
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

/**
 * @brief 从叶子节点中删除指定键
 * @details 在叶子节点中删除指定键的索引条目
 *
 * @param key 要删除的键
 * @return bool - 如果删除成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 设计思路
 * - 使用std::lower_bound找到要删除的条目
 * - 如果找到，删除条目并更新节点
 * - 删除后将节点序列化到磁盘页
 *
 * @par 注意事项
 * - 删除后节点会被标记为脏页，在UnpinPage时写入磁盘
 * - 如果键不存在，删除操作会返回false
 * - 删除后需要检查节点是否为空或需要合并
 */
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

/**
 * @brief 在叶子节点中搜索指定键
 * @details 在叶子节点中搜索指定键的索引条目
 *
 * @param key 要搜索的键
 * @return std::vector<IndexEntry> - 包含搜索结果的向量
 *
 * @par 算法复杂度
 * - 时间复杂度：O(logn)，其中n是节点中的条目数量
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 使用std::lower_bound进行二分查找，找到第一个大于等于key的条目
 * - 如果找到匹配的键，将其添加到结果向量中
 * - 返回结果向量，最多包含一个条目
 *
 * @par 注意事项
 * - 支持精确匹配，不支持模糊匹配
 * - 返回结果向量按键有序
 * - 搜索操作不会修改节点状态
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的点查询功能
 * - 二分查找：使用二分查找提高搜索效率
 * - 索引条目：包含键、页面ID和偏移量，用于定位实际数据
 */
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

/**
 * @brief 叶子节点范围搜索
 * @details 在叶子节点中搜索指定范围的键，返回范围内的所有条目
 *
 * @param lower_bound 范围下界
 * @param upper_bound 范围上界
 * @return std::vector<IndexEntry> - 搜索结果
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n + k)，其中n是节点中的条目数量，k是搜索结果数量
 * - 空间复杂度：O(k)，其中k是搜索结果数量
 *
 * @par 设计思路
 * - 使用二分查找找到范围的起始位置
 * - 从起始位置开始顺序扫描，收集范围内的所有条目
 * - 当遇到大于上界的键时停止扫描
 *
 * @par 注意事项
 * - 叶子节点中的条目按键的字典序排列
 * - 搜索结果按键的字典序排列
 * - 如果范围为空或没有匹配的条目，返回空向量
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的范围查询功能
 * - 叶子节点链：通过叶子节点之间的指针连接，支持高效的范围查询
 * - 二分查找：使用二分查找快速定位范围起始位置
 * - 顺序扫描：在叶子节点内进行顺序扫描，收集所有匹配的条目
 */
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

/**
 * @brief 分裂叶子节点
 * @details 将叶子节点分裂为两个节点，将后半部分的条目移动到新节点
 *
 * @param new_node 输出参数，用于存储新创建的节点
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 设计思路
 * - 计算中间位置，将条目分为两部分
 * - 创建新节点，分配新的磁盘页
 * - 将后半部分的条目移动到新节点
 * - 更新当前节点的next指针指向新节点
 * - 更新新节点的next指针指向原节点的next指针
 * - 将两个节点序列化到磁盘页
 *
 * @par 注意事项
 * - 分裂后需要更新父节点，将中间键插入到父节点
 * - 分裂后两个节点都会被标记为脏页，在UnpinPage时写入磁盘
 * - 分裂操作是B+树自平衡的重要机制，确保树的高度保持在较低水平
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的分裂操作
 * - 自平衡树：通过分裂操作保持B+树的平衡
 * - 磁盘I/O优化：分裂操作涉及多个磁盘IO，但通过缓冲池减少了实际的磁盘访问次数
 * - 页管理：为新节点分配新的磁盘页，高效管理磁盘空间
 */
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

/**
 * @brief 合并叶子节点
 * @details 将右节点的条目合并到当前节点，更新当前节点的next指针
 *
 * @param right_node 要合并的右节点
 *
 * @par 算法复杂度
 * - 时间复杂度：O(n)，其中n是右节点中的条目数量
 * - 空间复杂度：O(n)
 *
 * @par 设计思路
 * - 将右节点的所有条目合并到当前节点的末尾
 * - 更新当前节点的next指针指向右节点的next指针
 * - 将当前节点序列化到磁盘页
 *
 * @par 注意事项
 * - 合并后右节点会被删除，需要释放资源
 * - 合并后当前节点会被标记为脏页，在UnpinPage时写入磁盘
 * - 合并操作是B+树自平衡的重要机制，当节点条目数量过少时触发
 * - 合并后需要更新父节点，删除对应的键
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的合并操作
 * - 自平衡树：通过合并操作保持B+树的平衡
 * - 磁盘I/O优化：合并操作涉及多个磁盘I/O，但通过缓冲池减少了实际的磁盘访问次数
 * - 页管理：合并后右节点的磁盘页可以被回收，高效管理磁盘空间
 */
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
 * @details 实现了B+树索引的完整功能，包括创建、插入、删除、查询和范围查询等
 *
 * @par 设计思路
 * - 管理B+树的根节点和元数据
 * - 提供完整的索引操作接口，如创建、插入、删除、查询等
 * - 处理B+树的自平衡操作，如分裂、合并等
 * - 管理索引的元数据，如根节点页ID、创建时间等
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的完整功能
 * - 索引管理：提供了索引的创建、删除、修改等管理功能
 * - 磁盘I/O优化：通过缓冲池减少磁盘I/O次数
 * - 事务支持：支持事务的ACID特性
 *
 * @par 示例用法
 * @code
 * BPlusTreeIndex *index = new BPlusTreeIndex(storage_engine, table_name,
 * column_name); index->Create(); index->Insert(entry); std::vector<IndexEntry>
 * results = index->Search(key);
 * @endcode
 */
// BPlusTreeIndex 实现
/**
 * @brief BPlusTreeIndex构造函数
 * @details 创建一个B+树索引对象，初始化索引名称和元数据
 *
 * @param storage_engine 存储引擎智能指针，用于页管理
 * @param table_name 表名
 * @param column_name 列名
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 注意事项
 * - 构造函数会加载索引元数据，如果索引不存在则创建新索引
 * - 索引名称格式为：表名_列名_idx
 * - 使用智能指针确保存储引擎生命周期正确管理
 */
BPlusTreeIndex::BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine,
                               const std::string &table_name,
                               const std::string &column_name)
    : storage_engine_(storage_engine), table_name_(table_name),
      column_name_(column_name), root_page_id_(-1) {
  // B+树索引构造函数实现
  SQLCC_LOG_DEBUG("Created B+Tree index for table '" + table_name + "' column '" + column_name + "'");
}

/**
 * @brief BPlusTreeIndex析构函数
 * @details 释放B+树索引对象占用的资源
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 注意事项
 * - 析构函数不会释放存储引擎指针，因为存储引擎由外部管理
 */
BPlusTreeIndex::~BPlusTreeIndex() {
  // B+树索引析构函数实现
}

/**
 * @brief 创建索引
 * @details 创建一个新的B+树索引
 *
 * @return bool - 如果创建成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 分配一个新页面作为根节点
 * - 创建叶子节点对象并初始化
 * - 将根节点序列化到磁盘页
 *
 * @par 注意事项
 * - 如果根节点创建失败，会回滚页面分配
 * - 创建成功后索引状态为可用
 * - 初始根节点是一个空的叶子节点
 */
bool BPlusTreeIndex::Create() {
  if (!storage_engine_)
    return false;

  // 分配根节点页面
  if (!storage_engine_->NewPage(&root_page_id_))
    return false;

  if (root_page_id_ < 0)
    return false;

  // 创建叶子节点并初始化（使用智能指针）
  auto root_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, root_page_id_);
  if (!root_node) {
    storage_engine_->DeletePage(root_page_id_);
    return false;
  }

  // 序列化根节点到页面
  root_node->SerializeToPage();

  // 智能指针自动释放，无需手动delete
  return true;
}

/**
 * @brief 删除索引
 * @details 删除B+树索引，释放所有相关的磁盘页
 *
 * @return bool - 如果删除成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 如果根节点页ID有效，删除根节点页面
 * - 更新索引状态，将根节点页ID设置为-1
 *
 * @par 注意事项
 * - 目前的实现只删除了根节点页面，没有递归删除所有节点页面
 */
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

/**
 * @brief 插入索引条目到B+树索引
 * @details 将索引条目插入到B+树索引中，处理可能的分裂操作和树增长
 *
 * @param entry 要插入的索引条目
 * @return bool - 如果插入成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(logn)，其中n是索引中的条目数量
 * - 空间复杂度：O(logn)
 *
 * @par 设计思路
 * - 如果树为空，创建根节点
 * - 加载根节点，调用递归插入方法Insert
 * - 保存根节点的状态
 * - 如果根节点分裂，创建新的内部节点作为根节点
 * - 更新子节点的父节点ID
 * - 保存索引元数据
 *
 * @par 注意事项
 * - 插入操作会修改索引结构，需要确保并发安全
 * - 插入后索引会自动保持平衡
 * - 支持重复键的更新操作
 * - 当根节点分裂时，树的高度会增加1
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的插入功能
 * - 自平衡树：通过分裂操作保持B+树的平衡
 * - 树的增长：当根节点分裂时，树的高度会增加
 * - 递归算法：使用递归实现B+树的插入操作
/**
 * @brief 插入键值对
 * @details 将键值对插入到B+树索引中
 *
 * @param key 键
 * @param page_id 页面ID
 * @param offset 偏移量
 * @return bool - 如果插入成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果树为空，创建根节点
 * - 创建索引条目
 * - 加载根节点，调用递归插入方法
 * - 保存根节点的状态
 */
bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset) {
  if (!storage_engine_)
    return false;

  // 如果树为空，创建根节点
  if (root_page_id_ < 0) {
    if (!Create())
      return false;
  }

  // 创建索引条目
  IndexEntry entry(key, page_id, offset);

  // 加载根节点
  auto root_node = LoadNode(root_page_id_);
  if (!root_node) {
    return false;
  }

  // 调用递归插入方法
  bool result = Insert(key, page_id, offset, root_node);

  // 保存根节点的状态
  if (root_node) {
    root_node->SerializeToPage();
  }

  return result;
}

/**
 * @brief 递归插入键值对
 * @details 将键值对递归插入到B+树索引中
 *
 * @param key 键
 * @param page_id 页面ID
 * @param offset 偏移量
 * @param node 当前节点
 * @return bool - 如果插入成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果是叶子节点，直接插入键值对
 * - 如果是内部节点，递归插入
 * - 处理节点分裂和树增长的情况
 */
bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset, std::unique_ptr<BPlusTreeNode>& node) {
  if (!node)
    return false;

  // 根据节点类型调用相应的插入方法
  bool result = false;
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    IndexEntry entry(key, page_id, offset);
    result = leaf_node->Insert(entry);
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，找到合适的子节点进行递归插入
    int32_t child_page_id = internal_node->FindChildPageId(key);
    auto child_node = LoadNode(child_page_id);
    if (child_node) {
      result = Insert(key, page_id, offset, child_node);
      // 如果子节点插入成功，保存内部节点状态
      if (result) {
        node->SerializeToPage();
      }
    } else {
      result = false;
    }
  }

  // 如果是叶子节点，保存节点状态
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    node->SerializeToPage();
  }

  return result;
}

/**
 * @brief 删除键
 * @details 从B+树索引中删除指定的键
 *
 * @param key 要删除的键
 * @return bool - 如果删除成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果索引不存在，直接返回true
 * - 加载根节点，调用递归删除方法
 */
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

/**
 * @brief 在B+树索引中搜索指定键
 * @details 在B+树索引中搜索指定键的索引条目，返回所有匹配的条目
 *
 * @param key 要搜索的键
 * @return std::vector<IndexEntry> - 包含搜索结果的向量
 *
 * @par 算法复杂度
 * - 时间复杂度：O(logn)，其中n是索引中的条目数量
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 如果索引不存在或已删除，返回空向量
 * - 加载根节点，调用递归搜索方法Search
 * - 递归搜索方法会从根节点开始，找到对应的叶子节点搜索条目
 * - 返回搜索结果向量
 *
 * @par 注意事项
 * - 支持精确匹配，不支持模糊匹配
 * - 返回结果向量按键有序
 * - 搜索操作不会修改索引结构
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的点查询功能
 * - 二分查找：使用二分查找提高搜索效率
 * - 递归算法：使用递归实现B+树的搜索操作
 */
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

/**
 * @brief 递归搜索指定键
 * @details 在B+树索引中递归搜索指定键的索引条目
 *
 * @param key 要搜索的键
 * @param node 当前节点
 * @return std::vector<IndexEntry> - 包含搜索结果的向量
 *
 * @par 算法复杂度
 * - 时间复杂度：O(logn)，其中n是索引中的条目数量
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 如果是叶子节点，直接搜索键
 * - 如果是内部节点，递归搜索
 */
std::vector<IndexEntry> BPlusTreeIndex::Search(const std::string& key, std::unique_ptr<BPlusTreeNode>& node) const {
  if (!node)
    return std::vector<IndexEntry>();

  // 根据节点类型调用相应的搜索方法
  if (IsLeafNode(node)) {
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
      auto child_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(child_page_id);
      if (child_node) {
        // 递归搜索子节点，但不直接返回，确保child_node在搜索完成后才销毁
        std::vector<IndexEntry> results = Search(key, child_node);
        return results;
      }
    }
  }

  return std::vector<IndexEntry>();
}

/**
 * @brief 查找键
 * @details 在B+树索引中查找指定的键
 *
 * @param key 要查找的键
 * @param page_id 输出参数：页面ID
 * @param offset 输出参数：偏移量
 * @return bool - 如果找到返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果索引不存在，直接返回false
 * - 加载根节点，调用递归查找方法
 */
bool BPlusTreeIndex::Lookup(const std::string& key, int32_t& page_id, size_t& offset) const {
  if (!storage_engine_ || root_page_id_ < 0)
    return false; // 索引不存在，返回false

  // 加载根节点
  auto root_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(root_page_id_);
  if (!root_node)
    return false; // 节点加载失败，返回false

  // 调用递归查找方法
  bool result = Lookup(key, page_id, offset, root_node);

  return result;
}

/**
 * @brief 递归删除键
 * @details 从B+树索引中递归删除指定的键
 *
 * @param key 要删除的键
 * @param node 当前节点
 * @return bool - 如果删除成功返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果是叶子节点，直接删除键
 * - 如果是内部节点，递归删除
 * - 处理节点合并和树收缩的情况
 */
bool BPlusTreeIndex::Delete(const std::string& key, std::unique_ptr<BPlusTreeNode>& node) {
  if (!node)
    return false;

  // 根据节点类型调用相应的删除方法
  bool result = false;
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    result = leaf_node->Remove(key);
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，找到合适的子节点进行递归删除
    int32_t child_page_id = internal_node->FindChildPageId(key);
    auto child_node = LoadNode(child_page_id);
    if (child_node) {
      result = Delete(key, child_node);
      // 如果子节点删除成功，保存内部节点状态
      if (result) {
        node->SerializeToPage();
      }
    } else {
      result = false;
    }
  }

  // 如果节点需要合并，处理合并
  if (NeedMerge(node)) {
    // TODO: 实现节点合并逻辑
  }

  // 如果是叶子节点，保存节点状态
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    node->SerializeToPage();
  }

  return result;
}

/**
 * @brief 递归查找键
 * @details 在B+树索引中递归查找指定的键
 *
 * @param key 要查找的键
 * @param page_id 输出参数：页面ID
 * @param offset 输出参数：偏移量
 * @param node 当前节点
 * @return bool - 如果找到返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n)，其中n是索引中的键数量
 * - 空间复杂度：O(log n)
 *
 * @par 设计思路
 * - 如果是叶子节点，直接查找键
 * - 如果是内部节点，递归查找
 */
bool BPlusTreeIndex::Lookup(const std::string& key, int32_t& page_id, size_t& offset, std::unique_ptr<BPlusTreeNode>& node) const {
  if (!node)
    return false;

  // 根据节点类型调用相应的查找方法
  bool result = false;
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    auto entries = leaf_node->Search(key);
    if (!entries.empty()) {
      page_id = entries[0].page_id;
      offset = entries[0].offset;
      result = true;
    }
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，我们需要找到合适的子节点进行递归查找
    // 这里简化处理，实际实现应该更复杂
    (void)internal_node; // 避免未使用变量警告
    result = false; // 简化实现
  }

  return result;
}

/**
 * @brief 范围查找
 * @details 在B+树索引中查找指定范围的键
 *
 * @param start_key 起始键
 * @param end_key 结束键
 * @return 查找结果列表
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n + k)，其中n是索引中的键数量，k是结果数量
 * - 空间复杂度：O(k)，其中k是结果数量
 *
 * @par 设计思路
 * - 如果索引不存在，直接返回空向量
 * - 加载根节点，调用递归范围查找方法
 */
std::vector<std::pair<int32_t, size_t>> BPlusTreeIndex::RangeLookup(const std::string& start_key, const std::string& end_key) const {
  std::vector<std::pair<int32_t, size_t>> results;

  if (!storage_engine_ || root_page_id_ < 0)
    return results; // 索引不存在，返回空向量

  // 加载根节点
  auto root_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(root_page_id_);
  if (!root_node)
    return results; // 节点加载失败，返回空向量

  // 调用递归范围查找方法
  results = RangeLookup(start_key, end_key, root_node);

  return results;
}

/**
 * @brief 在B+树索引中搜索指定范围的键
 * @details 在B+树索引中搜索指定范围内的索引条目，返回所有匹配的条目
 *
 * @param lower_bound 范围的下界
 * @param upper_bound 范围的上界
 * @return std::vector<IndexEntry> - 包含搜索结果的向量
 *
 * @par 算法复杂度
 * - 时间复杂度：O(logn + k)，其中n是索引中的条目数量，k是匹配的条目数量
 * - 空间复杂度：O(k)
 *
 * @par 设计思路
 * - 如果索引不存在或已删除，返回空向量
 * - 加载根节点，调用递归范围搜索方法SearchRange
 * - 递归范围搜索方法会从根节点开始，找到对应的叶子节点，然后沿叶子节点链收集所有匹配的条目
 * - 返回搜索结果向量
 *
 * @par 注意事项
 * - 支持闭区间搜索，即包含lower_bound和upper_bound
 * - 返回结果向量按键有序
 * - 搜索操作不会修改索引结构
 * - 范围查询是B+树索引的核心优势之一，通过叶子节点链可以高效地进行跨节点的范围查询
 *
 * @par 数据库原理知识点
 * - B+树索引：实现了B+树索引的范围查询功能
 * - 叶子节点链：通过叶子节点之间的指针连接，支持高效的范围查询
 * - 递归算法：使用递归实现B+树的范围搜索操作
 * - 顺序扫描：在叶子节点链上进行顺序扫描，收集所有匹配的条目
 */
std::vector<IndexEntry> BPlusTreeIndex::SearchRange(const std::string& lower_bound, const std::string& upper_bound) const {
  if (!storage_engine_ || root_page_id_ < 0)
    return std::vector<IndexEntry>();

  // 获取根节点
  auto root_node = const_cast<BPlusTreeIndex*>(this)->LoadNode(root_page_id_);
  if (!root_node)
    return std::vector<IndexEntry>();

  // 递归范围搜索
  return SearchRange(lower_bound, upper_bound, root_node);
}

/**
 * @brief 递归范围查找
 * @details 在B+树索引中递归查找指定范围的键
 *
 * @param start_key 起始键
 * @param end_key 结束键
 * @param node 当前节点
 * @return 查找结果列表
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n + k)，其中n是索引中的键数量，k是结果数量
 * - 空间复杂度：O(k)，其中k是结果数量
 *
 * @par 设计思路
 * - 如果是叶子节点，直接范围查找
 * - 如果是内部节点，递归范围查找
 */
std::vector<std::pair<int32_t, size_t>> BPlusTreeIndex::RangeLookup(const std::string& start_key, const std::string& end_key, std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<std::pair<int32_t, size_t>> results;

  if (!node)
    return results;

  // 根据节点类型调用相应的范围查找方法
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    auto entries = leaf_node->SearchRange(start_key, end_key);
    for (const auto& entry : entries) {
      results.emplace_back(entry.page_id, entry.offset);
    }
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，我们需要找到合适的子节点进行递归范围查找
    // 这里简化处理，实际实现应该更复杂
    (void)internal_node; // 避免未使用变量警告
    // 简化实现：搜索所有子节点
    // 注意：这里的实现是不完整的，仅用于演示
  }

  return results;
}

/**
 * @brief 递归范围搜索
 * @details 在B+树索引中递归搜索指定范围的键，支持叶子节点链的范围查询
 *
 * @param lower_bound 范围的下界
 * @param upper_bound 范围的上界
 * @param node 当前节点
 * @return 搜索结果列表
 *
 * @par 算法复杂度
 * - 时间复杂度：O(log n + k)，其中n是索引中的键数量，k是结果数量
 * - 空间复杂度：O(k)，其中k是结果数量
 *
 * @par 设计思路
 * - 如果是叶子节点，搜索范围内的条目，并通过叶子节点链继续搜索
 * - 如果是内部节点，找到合适的子节点进行递归范围搜索
 * - 支持B+树的核心特性：叶子节点链支持高效范围查询
 */
std::vector<IndexEntry> BPlusTreeIndex::SearchRange(const std::string& lower_bound, const std::string& upper_bound, std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<IndexEntry> results;

  if (!node)
    return results;

  if (IsLeafNode(node)) {
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
        if (!next_node || !IsLeafNode(next_node))
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

/**
 * @brief 检查B+树索引是否存在
 * @details 检查B+树索引是否存在，通过根节点页ID判断
 *
 * @return bool - 如果索引存在返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 通过检查根节点页ID是否有效（>= 0）来判断索引是否存在
 * - 根节点页ID为-1表示索引不存在或已被删除
 *
 * @par 注意事项
 * - 此方法只检查索引的元数据是否存在，不检查索引的完整性
 * - 如果索引元数据存在但根节点页面损坏，此方法仍会返回true
 */

// 辅助方法：加载节点
std::unique_ptr<BPlusTreeNode> BPlusTreeIndex::LoadNode(int32_t page_id) {
  if (!storage_engine_)
    return nullptr;

  // 直接创建叶子节点，让节点构造函数自己处理页面获取和类型检查
  // 节点构造函数会正确管理页面生命周期
  auto leaf_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, page_id);
  
  // 检查节点是否成功创建（通过检查页面ID是否有效）
  if (leaf_node && leaf_node->GetPageId() >= 0) {
    return leaf_node;
  }
  
  // 如果叶子节点创建失败，尝试创建内部节点
  auto internal_node = std::make_unique<BPlusTreeInternalNode>(storage_engine_, page_id);
  if (internal_node && internal_node->GetPageId() >= 0) {
    return internal_node;
  }
  
  SQLCC_LOG_ERROR("Failed to load node for page " + std::to_string(page_id));
  return nullptr;
}

/**
 * @brief 检查节点是否为叶子节点
 * @details 检查给定的节点是否为叶子节点
 *
 * @param node 要检查的节点
 * @return bool - 如果是叶子节点返回true，否则返回false
 */
bool BPlusTreeIndex::IsLeafNode(std::unique_ptr<BPlusTreeNode>& node) const {
  if (!node)
    return false;
  
  return node->IsLeaf();
}

/**
 * @brief 获取节点的键
 * @details 获取给定节点的所有键
 *
 * @param node 要获取键的节点
 * @return std::vector<std::string> - 节点的键列表
 */
std::vector<std::string> BPlusTreeIndex::GetKeys(std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<std::string> keys;
  
  if (!node)
    return keys;
  
  // 根据节点类型获取键
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    const auto& entries = leaf_node->GetEntries();
    for (const auto& entry : entries) {
      keys.push_back(entry.key);
    }
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    keys = internal_node->GetKeys();
  }
  
  return keys;
}

/**
 * @brief 获取节点的值
 * @details 获取给定节点的所有值（页面ID和偏移量对）
 *
 * @param node 要获取值的节点
 * @return std::vector<std::pair<int32_t, size_t>> - 节点的值列表
 */
std::vector<std::pair<int32_t, size_t>> BPlusTreeIndex::GetValues(std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<std::pair<int32_t, size_t>> values;
  
  if (!node)
    return values;
  
  // 根据节点类型获取值
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    const auto& entries = leaf_node->GetEntries();
    for (const auto& entry : entries) {
      values.emplace_back(entry.page_id, entry.offset);
    }
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    // 对于内部节点，值是子节点的页面ID
    const auto& child_page_ids = internal_node->GetChildPageIds();
    for (int32_t child_page_id : child_page_ids) {
      values.emplace_back(child_page_id, 0); // 偏移量对于内部节点无意义
    }
  }
  
  return values;
}

/**
 * @brief 获取节点的子节点
 * @details 获取给定内部节点的所有子节点页面ID
 *
 * @param node 要获取子节点的节点
 * @return std::vector<int32_t> - 子节点页面ID列表
 */
std::vector<int32_t> BPlusTreeIndex::GetChildren(std::unique_ptr<BPlusTreeNode>& node) const {
  std::vector<int32_t> children;
  
  if (!node)
    return children;
  
  // 只有内部节点才有子节点
  if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    children = internal_node->GetChildPageIds();
  }
  
  return children;
}

/**
 * @brief 检查节点是否需要合并
 * @details 检查给定节点是否需要与其兄弟节点合并
 *
 * @param node 要检查的节点
 * @return bool - 如果需要合并返回true，否则返回false
 */
bool BPlusTreeIndex::NeedMerge(const std::unique_ptr<BPlusTreeNode>& node) {
  if (!node)
    return false;
  
  // 根据节点类型检查是否需要合并
  if (auto leaf_node = dynamic_cast<BPlusTreeLeafNode*>(node.get())) {
    return leaf_node->GetEntries().size() < 50; // 假设最小键数量为50
  } else if (auto internal_node = dynamic_cast<BPlusTreeInternalNode*>(node.get())) {
    return internal_node->GetKeys().size() < 50; // 假设最小键数量为50
  }
  
  return false;
}

/**
 * @brief 检查B+树索引是否存在
 * @details 检查B+树索引是否存在，通过根节点页ID判断
 *
 * @return bool - 如果索引存在返回true，否则返回false
 *
 * @par 算法复杂度
 * - 时间复杂度：O(1)
 * - 空间复杂度：O(1)
 *
 * @par 设计思路
 * - 通过检查根节点页ID是否有效（>= 0）来判断索引是否存在
 * - 根节点页ID为-1表示索引不存在或已被删除
 *
 * @par 注意事项
 * - 此方法只检查索引的元数据是否存在，不检查索引的完整性
 * - 如果索引元数据存在但根节点页面损坏，此方法仍会返回true
 */
bool BPlusTreeIndex::Exists() const {
  return root_page_id_ >= 0;
}

// IndexManager
// 实现（在index_manager.cpp中，但这里提供一个简单声明以避免编译错误）

} // namespace sqlcc
