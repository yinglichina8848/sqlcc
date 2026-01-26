# SQLCC 索引系统设计文档

## 概述

SQLCC 使用先进的 B+ 树索引系统，支持高效的点查询、范围查询和排序操作。

## 实现文件

以下是索引系统的相关实现文件：

- `include/mocks/index_manager_mock.h`
- `include/storage_engine/index_manager.h`
- `src/mocks/index_manager_mock.cpp`
- `src/storage/b_plus_tree.h`
- `src/storage/b_plus_tree_nodes.h`
- `src/storage/index_manager.h`
- `src/storage_engine/b_plus_tree.h`
- `src/storage_engine/b_plus_tree/core/b_plus_tree.cpp`
- `src/storage_engine/b_plus_tree/core/b_plus_tree.h`
- `src/storage_engine/b_plus_tree/index/b_plus_tree_index.cpp`
- `src/storage_engine/b_plus_tree/index/b_plus_tree_index.h`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_internal_node.cpp`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_internal_node.h`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_leaf_node.cpp`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_leaf_node.h`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_node.cpp`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_node.h`
- `src/storage_engine/b_plus_tree/node/b_plus_tree_nodes.h`
- `src/storage_engine/b_plus_tree_fixed.cpp`
- `src/storage_engine/b_plus_tree_internal_node_backup.cpp`
- `src/storage_engine/b_plus_tree_nodes.h`
- `src/storage_engine/index_manager.h`
- `src/storage_engine/index_manager/enhanced_index_manager.cpp`
- `src/storage_engine/index_manager/index_manager.cpp`
- `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`
- `src/storage_engine/index_manager/smart_index_cache.cpp`
- `src/storage_engine/index_manager/smart_index_cache.h`
- `src/storage_engine/index_manager/smart_index_factory.cpp`
- `src/storage_engine/index_manager/smart_index_factory.h`
- `src/storage_engine/index_manager/smart_ptr_lifetime_manager.cpp`
- `src/storage_engine/index_manager/transactional_index_manager.cpp`

## 设计特点

- 高效的 B+ 树实现，支持变长键值
- 并发安全的索引操作
- 支持唯一性和非唯一性索引
- 自动平衡和分裂机制

## 性能特征

- O(log n) 查询时间复杂度
- 支持批量插入优化
- 内存友好的节点设计

