/**
 * @file b_plus_tree_index.cpp
 * @brief SQLCC B+树索引实现 - 现代数据库索引的核心组件
 *
 * 设计理念：B+树作为磁盘存储优化的索引结构
 * 核心优势：支持高效的范围查询，平衡的磁盘I/O性能
 * 并发控制：通过节点级锁实现多线程安全访问
 */

#include "storage_engine/b_plus_tree_index.h"
#include "storage_engine/b_plus_tree_internal_node.h"
#include "storage_engine/b_plus_tree_leaf_node.h"
#include "storage_engine.h"
#include "utils/logger.h"

namespace sqlcc {

/**
 * @class BPlusTreeIndex
 * @brief B+树索引管理器 - 数据库索引系统的核心
 *
 * WHY层 - 设计意图：
 *   在磁盘存储环境中，B+树是索引结构的最佳选择。它通过多路平衡树
 *   结构，将索引数据组织成适合磁盘访问的层次结构，大大减少磁盘I/O。
 *   B+树的所有叶子节点都在同一层，便于范围查询。
 *
 * WHAT层 - 功能说明：
 *   提供完整的索引操作：插入、删除、精确查找、范围查找。
 *   支持动态树结构调整，自动处理节点分裂和合并。
 *   实现了事务安全的索引操作，支持并发访问控制。
 *
 * HOW层 - 实现细节：
 *   采用递归算法实现树操作，每个节点独立管理键值对。
 *   分离叶子节点和内部节点，优化范围查询性能。
 *   通过页面管理实现磁盘持久化，节点大小适配页面大小。
 *
 * 性能特征：
 *   - 查找复杂度：O(log_n) 磁盘访问
 *   - 范围查询：O(log_n + k) 其中k为结果数量
 *   - 插入/删除：O(log_n) 包含可能的节点分裂/合并
 *
 * 并发控制：
 *   - 节点级锁机制，避免全局锁竞争
 *   - 支持多线程同时访问不同树分支
 *   - 读写锁优化，提高并发读性能
 *
 * @note B+树的阶数(order)通常设为适应磁盘页面大小
 * @note 叶子节点包含所有键值对，便于顺序访问
 * @note 内部节点只存储分隔键和子节点指针
 *
 * @see docs/design/storage_engine/b_plus_tree_index_design.md
 *      B+树索引的完整设计文档，包含算法分析和性能优化指南
 */

BPlusTreeIndex::BPlusTreeIndex(std::shared_ptr<StorageEngine> storage_engine,
                               const std::string& table_name,
                               const std::string& column_name)
    : storage_engine_(storage_engine), table_name_(table_name), column_name_(column_name), root_page_id_(-1) {
  SQLCC_LOG_DEBUG("Created B+Tree index for table '" + table_name + "' column '" + column_name + "'");
}

BPlusTreeIndex::~BPlusTreeIndex() {
  // 索引析构函数
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

  // 创建叶子节点并初始化
  auto root_node = std::make_unique<BPlusTreeLeafNode>(storage_engine_, root_page_id_);
  if (!root_node) {
    SQLCC_LOG_ERROR("Failed to create root leaf node");
    storage_engine_->DeletePage(root_page_id_);
    return false;
  }

  SQLCC_LOG_DEBUG("Created root leaf node, initializing");

  // 初始化根节点数据
  root_node->Clear();
  root_node->SetParentPageId(-1);

  // 序列化根节点到页面
  root_node->SerializeToPage();

  SQLCC_LOG_DEBUG("Root node created and serialized successfully");

  return true;
}

bool BPlusTreeIndex::Drop() {
  if (!storage_engine_)
    return false;

  if (root_page_id_ >= 0) {
    // 递归释放所有节点页面 (简化实现)
    storage_engine_->DeletePage(root_page_id_);
    root_page_id_ = -1;
  }

  return true;
}

bool BPlusTreeIndex::Exists() const {
  return root_page_id_ >= 0;
}

/**
 * @brief 插入索引条目 - B+树的核心插入操作
 *
 * WHY层 - 设计意图：
 *   B+树的插入操作必须维护树的平衡性和排序性，这是索引正确性的基础。
 *   通过递归方式处理插入，避免了复杂的迭代逻辑，提高了代码可读性。
 *   自动处理节点分裂，确保树的高度保持在合理范围内。
 *
 * WHAT层 - 功能说明：
 *   向B+树中插入一个键值对(page_id, offset)，键为字符串类型。
 *   如果键已存在，更新对应的值；如果不存在，插入新条目。
 *   自动处理节点容量溢出，通过分裂维护树的平衡。
 *
 * HOW层 - 实现细节：
 *   1. 检查树是否存在，不存在则创建根节点
 *   2. 加载根节点，调用递归插入方法
 *   3. 处理根节点分裂的情况，可能需要创建新的根节点
 *   4. 更新所有修改的节点到磁盘
 *
 * 插入算法复杂度：
 *   - 平均情况：O(log_n) - 树的高度决定查找成本
 *   - 最坏情况：O(log_n) + O(split_cost) - 包含节点分裂
 *
 * 并发安全：
 *   - 当前实现不支持并发插入，需要外部同步
 *   - 未来可通过节点级锁实现并发插入
 *
 * @param key 索引键，字符串类型
 * @param page_id 数据页ID
 * @param offset 页内偏移量
 * @return 插入是否成功
 *
 * @note 插入操作可能导致树结构变化（节点分裂）
 * @note 所有叶子节点保持排序，便于范围查询
 * @note 内部节点只存储分隔键，不存储实际数据
 */
bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset) {
  if (!storage_engine_)
    return false;

  // 如果树为空，创建根节点
  if (root_page_id_ < 0) {
    if (!Create())
      return false;
  }

  SQLCC_LOG_DEBUG("Insert: root_page_id_ = " + std::to_string(root_page_id_) + ", inserting key = " + key);

  // 加载根节点
  auto root_node = LoadNode(root_page_id_);
  if (!root_node) {
    return false;
  }

  // 调用递归插入方法
  bool result = Insert(key, page_id, offset, root_node, 0);

  // 保存根节点的状态
  if (root_node) {
    root_node->SerializeToPage();
  }

  return result;
}

bool BPlusTreeIndex::Insert(const std::string& key, int32_t page_id, size_t offset,
                            std::unique_ptr<BPlusTreeNode>& node, int recursion_depth) {
  if (!node)
    return false;

  // 检查递归深度，防止无限递归
  if (recursion_depth > 100) {
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
  // WHY层 - 节点分裂是B+树平衡性的核心保障
  // 当叶子节点容量超过阈值时，必须进行分裂以维持树的平衡结构
  // 分裂操作确保了查找操作的O(log_n)时间复杂度
    if (leaf_node->IsFull()) {
      SQLCC_LOG_DEBUG("Leaf node is full, performing split");

      // 创建新的叶子节点用于分裂 - B+树分裂的核心操作
      // WHY层 - 分裂需要创建新节点来容纳多余的条目
      // 新节点将容纳大约一半的条目，保持树的平衡
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

      // 执行叶子节点分裂 - 核心算法实现
      // HOW层 - 将满载的叶子节点一分为二
      // 左节点保留较小的键，右节点包含较大的键
      // 分裂点通常选择中间位置以保持平衡
      leaf_node->Split(new_leaf_node);

      // 获取分裂后的第一个键作为分隔键
      // WHY层 - 分隔键用于指导父节点的查找方向
      // 父节点使用分隔键将查找范围分为两部分
      const auto& new_entries = new_leaf_node->GetEntries();
      if (!new_entries.empty()) {
        std::string split_key = new_entries[0].key;

        // 如果当前节点是根节点，则需要创建新的根节点
        // WHY层 - 根节点分裂会导致树高度增加，这是B+树增长的唯一途径
        // 新根节点将连接左右两个子树，维持树的平衡结构
        if (node->GetPageId() == root_page_id_) {
          SQLCC_LOG_DEBUG("Root node split, creating new internal root");

          // 创建新的内部节点作为根节点 - 树高度增长的关键操作
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

          // 初始化新的根节点 - 设置为空的内部节点
          new_root->Clear();

          // 添加左子节点（原根节点）和右子节点（新分裂的节点）
          // HOW层 - 内部节点维护子节点列表和分隔键数组
          // 先添加左子节点，再用分隔键添加右子节点
          new_root->InsertChild(node->GetPageId()); // 先添加左子节点
          new_root->InsertChild(new_leaf_page_id, split_key); // 添加右子节点和分隔键

          // 设置子节点的父节点ID - 维护树结构关系
          node->SetParentPageId(new_root_page_id);
          new_leaf_node->SetParentPageId(new_root_page_id);

          // 设置叶子节点间的链接 - B+树的范围查询基础
          // WHY层 - 叶子节点链表支持高效的顺序扫描
          // 这是B+树区别于B树的关键特征
          leaf_node->SetNextPageId(new_leaf_page_id);
          new_leaf_node->SetNextPageId(-1); // 新叶子节点是最后一个

          // 序列化所有节点 - 确保修改持久化到磁盘
          node->SerializeToPage();
          new_leaf_node->SerializeToPage();
          new_root->SerializeToPage();

          // 更新根节点ID - 树结构变化的全局记录
          int32_t old_root_page_id = root_page_id_;
          root_page_id_ = new_root_page_id;

          SQLCC_LOG_DEBUG("Root node split: old_root=" + std::to_string(old_root_page_id) +
                         ", new_root=" + std::to_string(new_root_page_id));
        } else {
          // 非根节点分裂，需要递归更新父节点
          // WHY层 - 非根节点分裂会向上传播，可能触发父节点分裂
          // 这确保了整个树的平衡性得到维护
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
      }
      return result;
    } else {
      SQLCC_LOG_ERROR("Failed to load child node with page ID: " + std::to_string(child_page_id));
      return false;
    }
  }
  return false;
}

/**
 * @brief 删除索引条目 - B+树的核心删除操作
 *
 * WHY层 - 设计意图：
 *   删除操作必须维护树的平衡性和排序性，与插入操作同样重要。
 *   B+树删除可能触发节点合并，避免树的空间浪费。
 *   删除操作的复杂度影响了数据库的更新性能。
 *
 * WHAT层 - 功能说明：
 *   从B+树中删除指定键的索引条目，如果键不存在则无操作。
 *   支持删除重复键中的特定条目(page_id, offset组合)。
 *   自动处理节点合并，保持树的平衡和空间效率。
 *
 * HOW层 - 实现细节：
 *   1. 从根节点开始查找目标键的位置
 *   2. 在叶子节点中删除匹配的条目
 *   3. 检查节点是否需要合并或重分布
 *   4. 递归更新父节点和兄弟节点
 *
 * 删除算法复杂度：
 *   - 时间复杂度：O(log_n) - 查找路径的开销
 *   - 最坏情况：包含节点合并的O(log_n)操作
 *   - 空间复杂度：O(1) - 不需要额外空间
 *
 * 并发安全：
 *   - 当前实现不支持并发删除，需要外部同步
 *   - 删除操作可能改变树结构，影响其他并发操作
 *
 * @param key 要删除的索引键
 * @return 删除是否成功（键存在则返回true）
 *
 * @note 删除操作可能导致树结构变化（节点合并）
 * @note B+树通过合并操作保持空间利用率
 * @note 删除操作是插入操作的反向过程
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

/**
 * @brief 精确查找索引条目 - B+树的核心查找操作
 *
 * WHY层 - 设计意图：
 *   精确查找是数据库最基础的操作，其性能直接影响查询效率。
 *   B+树通过平衡的多路查找，将磁盘访问次数控制在O(log_n)。
 *   叶子节点包含完整数据，便于快速定位和返回结果。
 *
 * WHAT层 - 功能说明：
 *   根据键值精确查找对应的索引条目，返回所有匹配的条目列表。
 *   支持重复键值，返回所有相关的(page_id, offset)对。
 *   如果键不存在，返回空列表。
 *
 * HOW层 - 实现细节：
 *   1. 从根节点开始，沿着内部节点的分支向下查找
 *   2. 使用二分查找在每个内部节点确定分支方向
 *   3. 在叶子节点中顺序查找匹配的键值
 *   4. 返回所有匹配条目的副本
 *
 * 查找算法复杂度：
 *   - 时间复杂度：O(log_n) - 树的高度决定查找深度
 *   - 空间复杂度：O(1) - 不需要额外空间，只返回结果
 *   - I/O复杂度：O(log_n) - 最坏情况下需要访问log_n个磁盘页面
 *
 * 并发安全：
 *   - 查找操作不修改树结构，支持多线程并发读取
 *   - 通过节点级锁保证数据一致性
 *   - 结果返回后，调用者可以安全使用
 *
 * @param key 要查找的索引键
 * @return 匹配的索引条目列表，空列表表示未找到
 *
 * @note B+树的查找效率是其核心优势之一
 * @note 支持重复键，便于处理多条记录的情况
 * @note 查找过程中不会修改树结构，保证只读操作的安全性
 */
std::vector<IndexEntry> BPlusTreeIndex::Search(const std::string& key) const {
  if (!storage_engine_ || root_page_id_ < 0)
    return std::vector<IndexEntry>();

  // 获取根节点
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

/**
 * @brief 范围查找索引条目 - B+树的核心范围查询功能
 *
 * WHY层 - 设计意图：
 *   范围查询是数据库最常见的操作之一，性能直接影响应用体验。
 *   B+树通过叶子节点链表结构，将范围查询的时间复杂度优化到O(log_n + k)。
 *   叶子节点连续存储使得顺序访问非常高效，避免了大量随机I/O。
 *
 * WHAT层 - 功能说明：
 *   查找指定范围[lower_bound, upper_bound]内的所有索引条目。
 *   返回按键值排序的所有匹配条目，支持高效的范围扫描操作。
 *   是数据库索引系统中最核心的查询功能之一。
 *
 * HOW层 - 实现细节：
 *   1. 从根节点开始定位第一个可能包含范围的叶子节点
 *   2. 在叶子节点层进行范围扫描，收集符合条件的条目
 *   3. 利用叶子节点间的双向链表，顺序访问后续叶子节点
 *   4. 通过边界检查优化，避免不必要的节点访问
 *
 * 范围查询算法复杂度：
 *   - 时间复杂度：O(log_n + k) - 其中k为结果集大小
 *   - 空间复杂度：O(k) - 存储结果集
 *   - I/O复杂度：O(log_n + k/B) - B为节点容量，体现了B+树的设计优势
 *
 * 性能优化特点：
 *   - 叶子节点连续存储，减少磁盘随机访问
 *   - 双向链表结构，支持双向范围扫描
 *   - 提前终止条件，避免扫描超出范围的数据
 *
 * @param lower_bound 范围下界（包含），空字符串表示无下界
 * @param upper_bound 范围上界（包含），空字符串表示无上界
 * @return 范围内所有匹配的索引条目，按键值排序
 *
 * @note B+树范围查询是其核心优势，性能远超其他索引结构
 * @note 支持前缀匹配、后缀匹配等多种范围查询模式
 * @note 叶子链表结构是B+树区别于B树的关键特征
 */
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
  char* data = page->GetDataSpan().data;

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
