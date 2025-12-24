# SQLCC 项目头文件索引

**生成时间**: 2025年12月24日
**版本**: v1.2.7
**索引范围**: include/ 目录下所有 .h 文件

## 📁 目录结构概览

```
include/
├── core/                    # 核心组件 (用户管理、权限验证等)
├── sql_parser/             # SQL解析器 (AST、词法分析等)
├── execution/              # 执行引擎 (查询执行、计划等)
├── storage/                # 存储接口 (表、索引等)
├── storage_engine/         # 存储引擎实现 (B+树、缓冲池等)
├── network/                # 网络通信 (连接、加密等)
├── transaction/            # 事务管理 (ACID、并发控制等)
├── procedure/              # 存储过程 (VM、解析等)
├── trigger/                # 触发器 (定义、执行等)
├── exception/              # 异常处理 (各类异常类)
├── utils/                  # 工具类 (配置、日志等)
├── types/                  # 数据类型 (域管理等)
├── security/               # 安全模块 (内存监控等)
├── error_handler.h         # 错误处理接口
├── exception.h             # 异常基类
├── page.h                  # 页面管理
├── unified_query_plan.h    # 统一查询计划
└── wal_manager.h           # WAL管理
```

## 🎯 核心头文件索引

### 1. 核心组件 (core/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `core/user_manager.h` | 用户管理系统 | `UserManager` | `exception.h`, `utils/logger.h` |
| `core/permission_validator.h` | 权限验证系统 | `PermissionValidator` | `core/user_manager.h` |
| `core/error_handler.h` | 错误处理接口 | `ErrorHandler` | `exception.h` |
| `core/sql_executor_interface.h` | SQL执行器接口 | `SqlExecutorInterface` | `execution/` |

### 2. SQL解析器 (sql_parser/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `sql_parser/ast_node.h` | AST节点基类 | `AstNode` | `exception.h` |
| `sql_parser/ast_nodes.h` | AST节点集合 | 各类节点类 | `ast_node.h` |
| `sql_parser/token.h` | 词法分析token | `Token` | - |
| `sql_parser/parser.h` | 语法分析器 | `Parser` | `token.h`, `ast_nodes.h` |
| `sql_parser/data_types.h` | **数据类型定义** | `DataType`, `DataValue` | `exception.h` |
| `sql_parser/datetime.h` | 日期时间类型 | `DateTimeValue` | `data_types.h` |
| `sql_parser/decimal.h` | 十进制类型 | `DecimalValue` | `data_types.h` |
| `sql_parser/json.h` | JSON类型 | `JsonValue` | `data_types.h` |
| `sql_parser/constraint.h` | 约束定义 | 约束类 | `ast_node.h` |
| `sql_parser/set_operation.h` | 集合操作 | `SetOperation` | `ast_nodes.h` |
| `sql_parser/window_function.h` | 窗口函数 | `WindowFunction` | `ast_nodes.h` |
| `sql_parser/recursive_query.h` | 递归查询 | `RecursiveQuery` | `ast_nodes.h` |
| `sql_parser/function_ast.h` | 函数AST | 函数节点 | `ast_nodes.h` |
| `sql_parser/load_data_ast.h` | 加载数据AST | `LoadDataAst` | `ast_nodes.h` |
| `sql_parser/node_visitor.h` | AST访问者模式 | `NodeVisitor` | `ast_node.h` |

### 3. 执行引擎 (execution/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `execution/join_executor.h` | JOIN执行器 | `JoinExecutor` | `execution_ast/` |
| `execution/set_operation_executor.h` | 集合操作执行器 | `SetOperationExecutor` | `sql_parser/set_operation.h` |
| `execution/function_executor.h` | 函数执行器 | `FunctionExecutor` | `sql_parser/function_ast.h` |
| `execution/load_data_executor.h` | 加载数据执行器 | `LoadDataExecutor` | `sql_parser/load_data_ast.h` |
| `execution/window_function_executor.h` | 窗口函数执行器 | `WindowFunctionExecutor` | `sql_parser/window_function.h` |
| `execution/recursive_query_executor.h` | 递归查询执行器 | `RecursiveQueryExecutor` | `sql_parser/recursive_query.h` |
| `execution/subquery_executor.h` | 子查询执行器 | `SubqueryExecutor` | `execution_ast/` |
| `execution/aggregate_engine.h` | 聚合引擎 | `AggregateEngine` | `execution_ast/` |
| `execution/group_by_executor.h` | GROUP BY执行器 | `GroupByExecutor` | `execution_ast/` |
| `execution/unified_executor.h` | 统一执行器 | `UnifiedExecutor` | 所有执行器 |
| `execution/task_executor.h` | 任务执行器 | `TaskExecutor` | `execution/` |
| `execution/comprehensive_task_executor.h` | 综合任务执行器 | `ComprehensiveTaskExecutor` | `task_executor.h` |
| `execution/test_runner.h` | 测试运行器 | `TestRunner` | `execution/` |
| `execution/standalone_test.h` | 独立测试 | `StandaloneTest` | `test_runner.h` |
| `execution/ddl_execution_strategy.h` | DDL执行策略 | `DdlExecutionStrategy` | `execution/` |
| `execution/dml_execution_strategy.h` | DML执行策略 | `DmlExecutionStrategy` | `execution/` |

### 4. 存储引擎 (storage_engine/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `storage_engine/b_plus_tree_node.h` | B+树节点 | `BPlusTreeNode` | `storage/` |
| `storage_engine/b_plus_tree_leaf_node.h` | B+树叶子节点 | `BPlusTreeLeafNode` | `b_plus_tree_node.h` |
| `storage_engine/b_plus_tree_internal_node.h` | B+树内部节点 | `BPlusTreeInternalNode` | `b_plus_tree_node.h` |
| `storage_engine/b_plus_tree_index.h` | B+树索引 | `BPlusTreeIndex` | `b_plus_tree_node.h` |
| `storage_engine/b_plus_tree.h` | B+树实现 | `BPlusTree` | `b_plus_tree_index.h` |
| `storage_engine/buffer_pool.h` | 缓冲池接口 | `BufferPool` | `storage/` |
| `storage_engine/buffer_pool_sharded.h` | 分片缓冲池 | `BufferPoolSharded` | `buffer_pool.h` |
| `storage_engine/buffer_pool/lru_manager.h` | LRU管理器 | `LruManager` | `buffer_pool.h` |
| `storage_engine/buffer_pool/statistics_collector.h` | 统计收集器 | `StatisticsCollector` | `buffer_pool.h` |
| `storage_engine/index_manager/smart_index_cache.h` | 智能索引缓存 | `SmartIndexCache` | `storage/` |
| `storage_engine/index_manager/smart_index_factory.h` | 智能索引工厂 | `SmartIndexFactory` | `storage/` |
| `storage_engine/index_manager/smart_ptr_lifetime_manager.h` | 智能指针生命周期 | `SmartPtrLifetimeManager` | `storage/` |
| `storage_engine/table_storage/page_raii.h` | 页面RAII | `PageRAII` | `storage/` |
| `storage_engine/table_storage/record_validator.h` | 记录验证器 | `RecordValidator` | `storage/` |
| `storage_engine/replace_strategy.h` | 替换策略接口 | 策略接口 | `storage/` |

### 5. 存储接口 (storage/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `storage/b_plus_tree.h` | B+树存储接口 | `BPlusTreeStorage` | `storage_engine/` |
| `storage/buffer_pool.h` | 缓冲池存储接口 | `BufferPoolStorage` | `storage_engine/` |
| `storage/buffer_pool_fixed.h` | 固定缓冲池 | `BufferPoolFixed` | `buffer_pool.h` |
| `storage/buffer_pool_v2.h` | 缓冲池V2 | `BufferPoolV2` | `buffer_pool.h` |
| `storage/advanced_lock_manager.h` | 高级锁管理器 | `AdvancedLockManager` | `transaction/` |
| `storage/b_plus_tree_nodes.h` | B+树节点接口 | 节点接口 | `storage_engine/` |
| `storage/record_boundary_validator.h` | 记录边界验证器 | `RecordBoundaryValidator` | `storage/` |
| `storage/data_integrity_validator.h` | 数据完整性验证器 | `DataIntegrityValidator` | `storage/` |
| `storage/disk_error_handler.h` | 磁盘错误处理器 | `DiskErrorHandler` | `exception.h` |
| `storage/disk_manager.h` | 磁盘管理器 | `DiskManager` | `storage/` |
| `storage/wal_buffer.h` | WAL缓冲区 | `WalBuffer` | `transaction/` |
| `storage/wal_writer.h` | WAL写入器 | `WalWriter` | `wal_buffer.h` |
| `storage/checkpoint.h` | 检查点 | `Checkpoint` | `transaction/` |
| `storage/concurrency_control.h` | 并发控制 | `ConcurrencyControl` | `transaction/` |
| `storage/replace_strategy.h` | 替换策略 | 策略接口 | `storage_engine/` |

### 6. 网络通信 (network/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `network/network.h` | 网络接口 | `Network` | `exception.h` |
| `network/connection_state.h` | 连接状态 | `ConnectionState` | `network.h` |
| `network/connection_state_machine.h` | 连接状态机 | `ConnectionStateMachine` | `connection_state.h` |
| `network/session.h` | 会话管理 | `Session` | `network.h` |
| `network/session_manager.h` | 会话管理器 | `SessionManager` | `session.h` |
| `network/client_connection.h` | 客户端连接 | `ClientConnection` | `network.h` |
| `network/client_network_manager.h` | 客户端网络管理器 | `ClientNetworkManager` | `client_connection.h` |
| `network/server_network_manager.h` | 服务端网络管理器 | `ServerNetworkManager` | `network.h` |
| `network/connection_handler.h` | 连接处理器 | `ConnectionHandler` | `network.h` |
| `network/encryption.h` | 加密接口 | 加密接口 | `security/` |
| `network/network_exception.h` | 网络异常 | `NetworkException` | `exception.h` |
| `network/network_exception_handler.h` | 网络异常处理器 | `NetworkExceptionHandler` | `network_exception.h` |
| `network/network_monitor.h` | 网络监控 | `NetworkMonitor` | `network.h` |
| `network/network_stability_guard.h` | 网络稳定性守护 | `NetworkStabilityGuard` | `network.h` |
| `network/data_transmission_validator.h` | 数据传输验证器 | `DataTransmissionValidator` | `network.h` |
| `network/message_processor.h` | 消息处理器 | `MessageProcessor` | `network.h` |
| `network/key_rotation_policy.h` | 密钥轮换策略 | `KeyRotationPolicy` | `encryption.h` |

### 7. 事务管理 (transaction/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `transaction/savepoint_manager.h` | 保存点管理器 | `SavepointManager` | `transaction/` |
| `transaction/BUILD.bazel` | 事务构建配置 | - | - |

### 8. 存储过程 (procedure/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `procedure/procedure_parser.h` | 存储过程解析器 | `ProcedureParser` | `sql_parser/` |
| `procedure/procedure_vm.h` | 存储过程虚拟机 | `ProcedureVM` | `procedure_parser.h` |
| `procedure/procedure_trigger_executor.h` | 存储过程触发器执行器 | `ProcedureTriggerExecutor` | `procedure_vm.h` |
| `procedure/BUILD.bazel` | 存储过程构建配置 | - | - |

### 9. 触发器 (trigger/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `trigger/trigger_definition.h` | 触发器定义 | `TriggerDefinition` | `sql_parser/` |
| `trigger/trigger_executor.h` | 触发器执行器 | `TriggerExecutor` | `trigger_definition.h` |
| `trigger/trigger_manager.h` | 触发器管理器 | `TriggerManager` | `trigger_executor.h` |
| `trigger/recursion_guard.h` | 递归守护 | `RecursionGuard` | `trigger_definition.h` |
| `trigger/sql_trigger_executor.h` | SQL触发器执行器 | `SqlTriggerExecutor` | `trigger_executor.h` |
| `trigger/BUILD.bazel` | 触发器构建配置 | - | - |

### 10. 工具类 (utils/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `utils/config_manager.h` | 配置管理器 | `ConfigManager` | `exception.h` |
| `utils/config_lifecycle.h` | 配置生命周期 | `ConfigLifecycle` | `config_manager.h` |
| `utils/config_snapshot.h` | 配置快照 | `ConfigSnapshot` | `config_manager.h` |
| `utils/smart_config_manager.h` | 智能配置管理器 | `SmartConfigManager` | `config_manager.h` |
| `utils/logger.h` | 日志接口 | `Logger` | `exception.h` |
| `utils/BUILD.bazel` | 工具构建配置 | - | - |

### 11. 数据类型 (types/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `types/domain_manager.h` | 域管理器 | `DomainManager` | `sql_parser/data_types.h` |

### 12. 安全模块 (security/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `security/memory_monitor.h` | 内存监控 | `MemoryMonitor` | `exception.h` |
| `security/BUILD` | 安全构建配置 | - | - |

### 13. 异常处理 (exception/)

| 文件名 | 功能描述 | 主要类/接口 | 依赖关系 |
|--------|----------|-------------|----------|
| `exception/base_exception.h` | 异常基类 | `BaseException` | - |
| `exception/io_exception.h` | IO异常 | `IOException` | `base_exception.h` |
| `exception/buffer_exception.h` | 缓冲区异常 | `BufferException` | `base_exception.h` |
| `exception/page_exception.h` | 页面异常 | `PageException` | `base_exception.h` |
| `exception/disk_exception.h` | 磁盘异常 | `DiskException` | `base_exception.h` |
| `exception/lock_exception.h` | 锁异常 | `LockException` | `base_exception.h` |
| `exception/feature_exception.h` | 功能异常 | `FeatureException` | `base_exception.h` |
| `exception/argument_exception.h` | 参数异常 | `ArgumentException` | `base_exception.h` |
| `exception.h` | 异常头文件 | 所有异常类 | `exception/` |

## 🔍 关键文件定位

### 重点关注的头文件

#### **data_types.h** (测试相关)
- **路径**: `include/sql_parser/data_types.h`
- **功能**: 定义数据类型系统
- **主要类**:
  - `DataType` - 数据类型枚举
  - `DataValue` - 数据值封装
  - `DecimalValue` - 十进制数值
  - `DateTimeValue` - 日期时间值
  - `DataTypeManager` - 类型管理器
- **测试**: `tests/unit/basic/data_types_test.cpp`
- **依赖**: `exception.h`

#### **ast_node.h** (AST相关)
- **路径**: `include/sql_parser/ast_node.h`
- **功能**: AST节点基类
- **主要类**: `AstNode`
- **依赖**: `exception.h`

#### **buffer_pool.h** (存储相关)
- **路径**: `include/storage/buffer_pool.h`
- **功能**: 缓冲池接口
- **主要类**: `BufferPool`
- **实现**: `storage_engine/buffer_pool*.h`

## 📊 统计信息

| 类别 | 文件数量 | 主要功能 |
|------|----------|----------|
| 核心组件 | 4 | 用户管理、权限、错误处理 |
| SQL解析器 | 14 | AST、词法分析、数据类型 |
| 执行引擎 | 16 | 查询执行、任务管理 |
| 存储接口 | 13 | 表、索引、缓冲池 |
| 存储引擎 | 12 | B+树、缓冲池实现 |
| 网络通信 | 17 | 连接、加密、传输 |
| 事务管理 | 2 | 保存点管理 |
| 存储过程 | 4 | 解析器、虚拟机 |
| 触发器 | 6 | 定义、执行、管理 |
| 工具类 | 5 | 配置、日志 |
| 数据类型 | 1 | 域管理 |
| 安全模块 | 2 | 内存监控 |
| 异常处理 | 9 | 各类异常类 |
| **总计** | **105** | **完整的数据库系统** |

## 🎯 使用指南

### 查找特定功能
1. **根据功能关键词搜索**: 如 "B+树" -> `storage_engine/b_plus_tree*.h`
2. **根据组件分类**: 如 "网络" -> `network/` 目录
3. **根据测试文件**: 如 `data_types_test.cpp` -> `sql_parser/data_types.h`

### 依赖关系分析
- **自底向上**: 基础组件 -> 核心功能 -> 高级特性
- **横向扩展**: 同层组件之间的接口关系

### 维护建议
- 新增头文件时需更新此索引
- 定期验证依赖关系的准确性
- 与 `src/` 源码目录保持同步

---

**最后更新**: 2025年12月24日
**维护者**: SQLCC AI Agent
**索引版本**: v1.0
