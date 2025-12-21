#include "storage_engine/b_plus_tree_index.h"
#include "storage_engine/b_plus_tree_internal_node.h"
#include "storage_engine/b_plus_tree_leaf_node.h"
#include "storage_engine.h"
#include "utils/logger.h"

namespace sqlcc {

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
