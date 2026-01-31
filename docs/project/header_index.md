# SQLCC 项目头文件与源码索引 v1.3.9

**版本**: v1.3.9  
**更新日期**: 2026-01-30  
**索引范围**: include/ 和 src/ 目录下所有核心文件

---

## 📁 目录结构概览

```
sqlcc/
├── include/               # 公共头文件
│   ├── core/             # 核心组件
│   ├── sql_parser/       # SQL解析器
│   ├── execution/        # 执行引擎
│   ├── storage/          # 存储接口
│   ├── storage_engine/   # 存储引擎实现
│   ├── network/          # 网络通信
│   ├── transaction/      # 事务管理
│   ├── procedure/        # 存储过程
│   ├── trigger/          # 触发器
│   ├── exception/        # 异常处理
│   ├── utils/            # 工具类
│   ├── types/            # 数据类型
│   └── security/         # 安全模块
├── src/                  # 源码实现
│   └── (结构与 include/ 对应)
└── tests/                # 测试代码
    ├── level1_foundation/# 基础层测试
    ├── level2_core/      # 核心层测试
    ├── level2_storage_engine/ # 存储引擎测试
    ├── level3_transaction_manager/ # 事务测试
    ├── level4_sql_processing/ # SQL处理测试
    ├── level5_network/   # 网络测试
    └── level6_integration/ # 集成测试
```

---

## 🎯 核心模块索引

### 1. 基础组件 (exception/, utils/, types/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **Exception** | `include/exception/exception.h` | `src/exception/exception.cpp` | `level1_foundation/exception/` | 异常基类 |
| **Logger** | `include/utils/logger.h` | `src/utils/logger.cpp` | `level1_foundation/logger/` | 日志系统 |
| **Config** | `include/utils/config_manager.h` | `src/utils/config_manager.cpp` | `level1_foundation/config/` | 配置管理 |
| **Types** | `include/types/value.h` | `src/types/value.cpp` | `level1_foundation/types/` | 数据类型 |

### 2. 核心组件 (core/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **UserManager** | `include/core/user_manager.h` | `src/core/user_manager.cpp` | `level2_core/` | 用户管理 |
| **PermissionValidator** | `include/core/permission_validator.h` | `src/core/permission_validator.cpp` | `level2_core/` | 权限验证 |
| **DatabaseManager** | `include/core/database_manager.h` | `src/core/database_manager.cpp` | `level2_core/` | 数据库管理 |

### 3. SQL 解析器 (sql_parser/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **Lexer** | `include/sql_parser/lexer.h` | `src/sql_parser/lexer.cpp` | `level4_sql_processing/lexer/` | 词法分析 |
| **Parser** | `include/sql_parser/parser.h` | `src/sql_parser/parser.cpp` | `level4_sql_processing/parser/` | 语法分析 |
| **AST** | `include/sql_parser/ast.h` | `src/sql_parser/ast.cpp` | `level4_sql_processing/ast/` | AST节点 |
| **DataTypes** | `include/sql_parser/data_types.h` | `src/sql_parser/data_types.cpp` | `level1_foundation/types/` | 数据类型定义 |

### 4. 执行引擎 (execution/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **UnifiedExecutor** | `include/execution/unified_executor.h` | `src/execution/unified_executor.cpp` | `level4_sql_processing/execution/` | 统一执行器 |
| **JoinExecutor** | `include/execution/join_executor.h` | `src/execution/join_executor.cpp` | `level4_sql_processing/execution/` | JOIN执行 |
| **SetOperationExecutor** | `include/execution/set_operation_executor.h` | `src/execution/set_operation_executor.cpp` | `level4_sql_processing/execution/` | 集合操作 |
| **WindowFunctionExecutor** | `include/execution/window_function_executor.h` | `src/execution/window_function_executor.cpp` | `level4_sql_processing/execution/` | 窗口函数 |

### 5. 存储引擎 (storage_engine/, storage/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **BufferPool** | `include/storage_engine/buffer_pool.h` | `src/storage_engine/buffer_pool.cpp` | `level2_storage_engine/buffer_pool/` | 缓冲池 |
| **BPlusTree** | `include/storage_engine/b_plus_tree.h` | `src/storage_engine/b_plus_tree.cpp` | `level2_storage_engine/b_plus_tree/` | B+树索引 |
| **DiskManager** | `include/storage/disk_manager.h` | `src/storage/disk_manager.cpp` | `level2_storage_engine/disk_manager/` | 磁盘管理 |
| **IndexManager** | `include/storage_engine/index_manager.h` | `src/storage_engine/index_manager.cpp` | `level2_storage_engine/index_manager/` | 索引管理 |

### 6. 事务管理 (transaction/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **TransactionManager** | `include/transaction/transaction_manager.h` | `src/transaction/transaction_manager.cpp` | `level3_transaction_manager/` | 事务管理器 |
| **LockManager** | `include/transaction/lock_manager.h` | `src/transaction/lock_manager.cpp` | `level3_transaction_manager/` | 锁管理器 |
| **WALManager** | `include/transaction/wal_manager.h` | `src/transaction/wal_manager.cpp` | `level3_transaction_manager/` | WAL日志 |

### 7. 网络通信 (network/)

| 模块 | 头文件 | 源文件 | 测试 | 说明 |
|------|--------|--------|------|------|
| **NetworkManager** | `include/network/network_manager.h` | `src/network/network_manager.cpp` | `level5_network/` | 网络管理 |
| **Connection** | `include/network/connection.h` | `src/network/connection.cpp` | `level5_network/` | 连接管理 |
| **Protocol** | `include/network/protocol.h` | `src/network/protocol.cpp` | `level5_network/` | 协议处理 |

---

## 🔗 依赖关系图

```
                    ┌─────────────────────────────────────┐
                    │           Application               │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │      Network / Server               │
                    │   (network/, server/)               │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │      SQL Parser / Executor          │
                    │   (sql_parser/, execution/)         │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │      Transaction Manager            │
                    │   (transaction/)                    │
                    └─────────────┬───────────────────────┘
                                  │
        ┌─────────────────────────┼─────────────────────────┐
        │                         │                         │
┌───────▼───────┐     ┌───────────▼────────┐    ┌──────────▼────────┐
│   Core        │     │  Storage Engine    │    │   Security        │
│  (core/)      │     │ (storage_engine/)  │    │  (security/)      │
└───────────────┘     └────────────────────┘    └───────────────────┘
        │                         │                         │
        └─────────────────────────┼─────────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │      Foundation Layer               │
                    │ (exception/, utils/, types/)        │
                    └─────────────────────────────────────┘
```

---

## 📂 BUILD.bazel 索引

### 关键 BUILD 文件位置

| 目录 | BUILD 文件 | 说明 |
|------|-----------|------|
| `src/core/` | BUILD.bazel | 核心组件构建配置 |
| `src/sql_parser/` | BUILD.bazel | SQL解析器构建配置 |
| `src/storage_engine/` | BUILD.bazel | 存储引擎构建配置 |
| `src/execution/` | BUILD.bazel | 执行引擎构建配置 |
| `tests/level1_foundation/` | BUILD.bazel | 基础层测试配置 |
| `tests/level2_core/` | BUILD.bazel | 核心层测试配置 |
| `tests/level2_storage_engine/` | BUILD.bazel | 存储引擎测试配置 |

---

## 🧪 测试文件映射

### Level 1 - Foundation 测试

| 被测模块 | 测试文件 | 测试范围 |
|----------|----------|----------|
| Exception | `tests/level1_foundation/exception/exception_test.cpp` | 异常基类、继承、捕获 |
| Types | `tests/level1_foundation/types/types_test.cpp` | Value类型、域管理 |
| Logger | `tests/level1_foundation/logger/logger_test.cpp` | 日志级别、输出、线程安全 |
| Config | `tests/level1_foundation/config/config_test.cpp` | 配置加载、解析、持久化 |

### Level 2 - Core & Storage 测试

| 被测模块 | 测试文件 | 测试范围 |
|----------|----------|----------|
| UserManager | `tests/level2_core/user_manager_test.cpp` | 用户CRUD、权限 |
| BufferPool | `tests/level2_storage_engine/buffer_pool/buffer_pool_test.cpp` | 页面管理、替换策略 |
| BPlusTree | `tests/level2_storage_engine/b_plus_tree/b_plus_tree_test.cpp` | 索引操作、并发 |

### Level 4 - SQL Processing 测试

| 被测模块 | 测试文件 | 测试范围 |
|----------|----------|----------|
| Lexer | `tests/level4_sql_processing/lexer/lexer_test.cpp` | Token识别、关键字 |
| Parser | `tests/level4_sql_processing/parser/parser_test.cpp` | 语法分析、AST生成 |
| Executor | `tests/level4_sql_processing/execution/executor_test.cpp` | 查询执行 |

---

## 🔍 快速查找指南

### 按功能查找

| 功能 | 头文件位置 | 源文件位置 |
|------|-----------|-----------|
| **异常处理** | `include/exception/*.h` | `src/exception/*.cpp` |
| **日志系统** | `include/utils/logger.h` | `src/utils/logger.cpp` |
| **数据类型** | `include/types/*.h` | `src/types/*.cpp` |
| **SQL解析** | `include/sql_parser/*.h` | `src/sql_parser/*.cpp` |
| **查询执行** | `include/execution/*.h` | `src/execution/*.cpp` |
| **存储引擎** | `include/storage_engine/*.h` | `src/storage_engine/*.cpp` |
| **事务管理** | `include/transaction/*.h` | `src/transaction/*.cpp` |
| **网络通信** | `include/network/*.h` | `src/network/*.cpp` |

### 按层次查找

| 层次 | 目录 | 说明 |
|------|------|------|
| **Foundation** | `include/exception/`, `include/utils/`, `include/types/` | 基础工具类 |
| **Core** | `include/core/` | 核心组件 |
| **Storage** | `include/storage_engine/`, `include/storage/` | 存储引擎 |
| **SQL** | `include/sql_parser/`, `include/execution/` | SQL处理 |
| **Network** | `include/network/` | 网络通信 |

---

## 📊 统计信息

| 类别 | 头文件数 | 源文件数 | 测试文件数 |
|------|---------|---------|-----------|
| Foundation | 15 | 15 | ~40 |
| Core | 8 | 8 | ~20 |
| SQL Parser | 20 | 20 | ~30 |
| Execution | 15 | 15 | ~25 |
| Storage Engine | 25 | 25 | ~35 |
| Transaction | 8 | 8 | ~15 |
| Network | 12 | 12 | ~20 |
| **总计** | **~103** | **~103** | **~185** |

---

## 🔧 维护说明

### 新增文件流程

1. **头文件**: 添加到 `include/<module>/<file>.h`
2. **源文件**: 添加到 `src/<module>/<file>.cpp`
3. **BUILD**: 更新 `src/<module>/BUILD.bazel`
4. **测试**: 添加 `tests/<level>/<module>/<file>_test.cpp`
5. **索引**: 更新本文档

### 验证命令

```bash
# 验证索引完整性
bazel build //...
bazel test //...

# 检查头文件依赖
bazel query 'deps(//src/<module>:<target>)' --output graph

# 生成依赖图
bazel query 'allpaths(//src/core:core, //include/exception:headers)' --output graph
```

---

**维护者**: SQLCC 开发团队  
**最后更新**: 2026-01-30  
**版本**: v1.3.9
