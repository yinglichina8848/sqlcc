# SQLCC包定义和依赖关系设计文档

## 📋 文档概述

本文档基于对SQLCC项目源码的全面分析，设计了完整的包定义和依赖关系结构。按照Bazel构建原则，采用分层架构，确保依赖关系清晰、构建高效。

---

## 🔍 SQLCC源码结构分析

### 主要目录结构
```
sqlcc/
├── src/                    # 源代码目录
│   ├── core/              # 核心组件
│   ├── utils/             # 工具库
│   ├── storage_engine/    # 存储引擎
│   ├── sql_parser/        # SQL解析器
│   ├── execution/         # 执行引擎
│   ├── sql_executor/      # SQL执行器
│   ├── network/           # 网络通信
│   ├── config_manager/    # 配置管理
│   ├── procedure/         # 存储过程
│   ├── trigger/           # 触发器
│   ├── transaction/       # 事务管理
│   ├── types/             # 类型系统
│   └── security/          # 安全模块
├── include/               # 头文件目录
│   ├── core/              # 核心头文件
│   ├── utils/             # 工具头文件
│   ├── storage/           # 存储头文件
│   ├── sql_parser/        # 解析器头文件
│   ├── execution/         # 执行头文件
│   ├── network/           # 网络头文件
│   ├── procedure/         # 过程头文件
│   ├── trigger/           # 触发器头文件
│   ├── transaction/       # 事务头文件
│   ├── types/             # 类型头文件
│   └── security/          # 安全头文件
├── tests/                 # 测试目录
│   ├── unit/              # 单元测试
│   ├── integration/       # 集成测试
│   ├── performance/       # 性能测试
│   └── components/        # 组件测试
└── tools/                 # 工具目录
```

### 核心组件分析

#### 1. 工具库 (`utils`)
**功能**: 提供基础工具和日志系统
**主要文件**:
- `logger.h/cpp` - 日志系统
- `config_snapshot.h/cpp` - 配置快照
- `config_lifecycle.h/cpp` - 配置生命周期
- `smart_config_manager.h/cpp` - 智能配置管理
- `ssl_wrapper.h` - SSL包装器

#### 2. 核心库 (`core`)
**功能**: 系统核心组件
**主要文件**:
- `user_manager.h/cpp` - 用户管理(RBAC)
- `execution_context.h/cpp` - 执行上下文
- `unified_executor.h/cpp` - 统一执行器

#### 3. 存储引擎 (`storage_engine`)
**功能**: 数据存储和索引管理
**主要文件**:
- `storage_engine.h/cpp` - 存储引擎主接口
- `b_plus_tree.h/cpp` - B+树实现
- `buffer_pool.h/cpp` - 缓冲池管理
- `wal_buffer.h/cpp` - WAL缓冲
- `checkpoint.h/cpp` - 检查点系统
- `index_manager/` - 索引管理器

#### 4. SQL解析器 (`sql_parser`)
**功能**: SQL语句解析
**主要文件**:
- `parser.h/cpp` - 主解析器
- `lexer.h/cpp` - 词法分析器
- `token.h/cpp` - 标记处理
- `ast_node.h/cpp` - AST节点
- `constraint.h/cpp` - 约束处理
- `function_ast.h/cpp` - 函数AST

#### 5. 执行引擎 (`execution`)
**功能**: 查询执行逻辑
**主要文件**:
- `function_executor.h/cpp` - 函数执行
- `set_operation_executor.h/cpp` - 集合操作
- `window_function_executor.h/cpp` - 窗口函数
- `recursive_query_executor.h/cpp` - 递归查询
- `load_data_executor.h/cpp` - 数据加载

#### 6. SQL执行器 (`sql_executor`)
**功能**: SQL语句执行协调
**主要文件**:
- `sql_executor.h/cpp` - SQL执行器主接口

#### 7. 网络模块 (`network`)
**功能**: 网络通信和协议处理
**主要文件**:
- `network.h/cpp` - 网络接口
- `connection_state_machine.h/cpp` - 连接状态机
- `encryption.h/cpp` - 加密处理
- `mysql_protocol.h/cpp` - MySQL协议

#### 8. 配置管理 (`config_manager`)
**功能**: 系统配置管理
**主要文件**:
- `config_manager.h/cpp` - 配置管理器

#### 9. 存储过程和触发器
**功能**: 高级SQL特性
**存储过程** (`procedure`):
- `procedure_parser.h/cpp` - 过程解析
- `procedure_vm.h/cpp` - 过程虚拟机
- `procedure_trigger_executor.h/cpp` - 过程触发执行

**触发器** (`trigger`):
- `trigger_manager.h/cpp` - 触发器管理

#### 10. 事务管理 (`transaction`)
**功能**: 事务处理
**主要文件**:
- `savepoint_manager.h/cpp` - 保存点管理

#### 11. 类型系统 (`types`)
**功能**: 数据类型管理
**主要文件**:
- `domain_manager.h/cpp` - 域管理

#### 12. 安全模块 (`security`)
**功能**: 系统安全
**主要文件**:
- `memory_monitor.h/cpp` - 内存监控

---

## 🏗️ 包定义和依赖关系设计

### 包层次结构

```
sqlcc/
├── //:main_targets          # 根目标 (sqlcc, isql, tests)
├── //src:all_src_libs        # 所有源代码库
├── //include:all_headers     # 所有头文件
├── //tests:all_tests         # 所有测试
└── //tools:all_tools         # 所有工具
```

### 详细包定义

#### 1. 工具库包 (`//src/utils`)
```bazel
cc_library(
    name = "utils",
    srcs = [
        "config_snapshot.cpp",
        "config_lifecycle.cpp",
        "smart_config_manager.cpp",
    ],
    hdrs = [
        "../include/utils/config_snapshot.h",
        "../include/utils/config_lifecycle.h",
        "../include/utils/smart_config_manager.h",
        "../include/utils/ssl_wrapper.h",
    ],
    includes = ["../include"],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    visibility = ["//visibility:public"],
)
```

#### 2. 日志库包 (`//src/logger`)
```bazel
cc_library(
    name = "logger",
    srcs = [
        "logger.cpp",
        "logger_module.cppm",
        "logger_module_impl.cpp",
    ],
    hdrs = ["../include/utils/logger.h"],
    includes = ["../include"],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
        "-DSQLCC_MODERN_CPP=1",
    ],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 3. 核心库包 (`//src/core`)
```bazel
cc_library(
    name = "core",
    srcs = [
        "user_manager.cpp",
        "execution_context.cpp",
        "unified_executor.cpp",
    ],
    hdrs = [
        "../include/core/user_manager.h",
        "../include/core/execution_context.h",
        "../include/core/unified_executor.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/logger",
        "//src/utils",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 4. 存储引擎包 (`//src/storage_engine`)
```bazel
cc_library(
    name = "storage_engine",
    srcs = [
        "storage_engine.cpp",
        "b_plus_tree.cpp",
        "buffer_pool.cpp",
        "wal_buffer.cpp",
        "wal_writer.cpp",
        "checkpoint.cpp",
        "page_allocator.cpp",
        "concurrent_access_validator.cpp",
        "disk_error_handler.cpp",
        "partition_manager.cpp",
        "record_boundary_validator.cpp",
        "replace_strategy.cpp",
        "cache_consistency_manager.cpp",
        "advanced_lock_manager.cpp",
        "data_integrity_validator.cpp",
        "buffer_pool_sharded.cpp",
        "concurrency_control.cpp",
        "table_storage_complete.cpp",
    ],
    hdrs = [
        "../include/storage_engine.h",
        "../include/storage/b_plus_tree.h",
        "../include/storage/b_plus_tree_nodes.h",
        "../include/storage/buffer_pool.h",
        "../include/storage/buffer_pool_fixed.h",
        "../include/storage/buffer_pool_v2.h",
        "../include/storage/buffer_pool_sharded.h",
        "../include/storage/wal_buffer.h",
        "../include/storage/wal_writer.h",
        "../include/storage/checkpoint.h",
        "../include/storage/page_allocator.h",
        "../include/storage/concurrent_access_validator.h",
        "../include/storage/disk_error_handler.h",
        "../include/storage/partition_manager.h",
        "../include/storage/record_boundary_validator.h",
        "../include/storage/replace_strategy.h",
        "../include/storage/cache_consistency_manager.h",
        "../include/storage/advanced_lock_manager.h",
        "../include/storage/data_integrity_validator.h",
        "../include/storage/concurrency_control.h",
        "../include/storage/disk_manager.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/logger",
        "//src/utils",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 5. 索引管理器包 (`//src/index_manager`)
```bazel
cc_library(
    name = "index_manager",
    srcs = [
        "index_manager/index_manager.cpp",
        "index_manager/index_manager_smart_ptr_enhancement.cpp",
    ],
    hdrs = ["../include/storage/index_manager.h"],
    includes = ["../include"],
    deps = ["//src/storage_engine"],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 6. SQL解析器包 (`//src/sql_parser`)
```bazel
cc_library(
    name = "sql_parser",
    srcs = [
        "parser.cpp",
        "lexer.cpp",
        "token.cpp",
        "ast_node.cpp",
        "ast_nodes.cpp",
        "constraint.cpp",
        "set_operation.cpp",
        "window_function.cpp",
        "recursive_query.cpp",
        "decimal.cpp",
        "json.cpp",
        "datetime.cpp",
        "function_ast.cpp",
    ],
    hdrs = [
        "../include/sql_parser/parser.h",
        "../include/sql_parser/lexer.h",
        "../include/sql_parser/token.h",
        "../include/sql_parser/ast_node.h",
        "../include/sql_parser/ast_nodes.h",
        "../include/sql_parser/constraint.h",
        "../include/sql_parser/set_operation.h",
        "../include/sql_parser/window_function.h",
        "../include/sql_parser/recursive_query.h",
        "../include/sql_parser/decimal.h",
        "../include/sql_parser/json.h",
        "../include/sql_parser/datetime.h",
        "../include/sql_parser/function_ast.h",
        "../include/sql_parser/node_visitor.h",
        "../include/sql_parser/load_data_ast.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/logger",
        "//src/utils",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 7. 执行引擎包 (`//src/execution`)
```bazel
cc_library(
    name = "execution",
    srcs = [
        "function_executor.cpp",
        "set_operation_executor.cpp",
        "window_function_executor.cpp",
        "recursive_query_executor.cpp",
        "load_data_executor.cpp",
    ],
    hdrs = [
        "../include/execution/function_executor.h",
        "../include/execution/set_operation_executor.h",
        "../include/execution/window_function_executor.h",
        "../include/execution/recursive_query_executor.h",
        "../include/execution/load_data_executor.h",
        "../include/execution/join_executor.h",
        "../include/execution/procedure_trigger_task.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/sql_parser",
        "//src/storage_engine",
        "//src/logger",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 8. SQL执行器包 (`//src/sql_executor`)
```bazel
cc_library(
    name = "sql_executor",
    srcs = ["sql_executor.cpp"],
    hdrs = ["../include/sql_executor.h"],
    includes = ["../include"],
    deps = [
        "//src/sql_parser",
        "//src/execution",
        "//src/storage_engine",
        "//src/core",
        "//src/logger",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 9. 网络库包 (`//src/network`)
```bazel
cc_library(
    name = "network",
    srcs = [
        "network.cpp",
        "connection_state_machine.cpp",
        "encryption.cpp",
        "data_transmission_validator.cpp",
        "mysql_protocol.cpp",
    ],
    hdrs = [
        "../include/network/network.h",
        "../include/network/connection_state.h",
        "../include/network/connection_state_machine.h",
        "../include/network/encryption.h",
        "../include/network/data_transmission_validator.h",
        "../include/network/mysql_protocol.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/logger",
        "//src/utils",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = [
        "-stdlib=libc++",
        "-lssl",
        "-lcrypto",
    ],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 10. 配置管理包 (`//src/config_manager`)
```bazel
cc_library(
    name = "config_manager",
    srcs = ["../config_manager.cpp"],
    hdrs = ["../include/config_manager.h"],
    includes = ["../include"],
    deps = ["//src/logger"],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 11. 存储过程包 (`//src/procedure`)
```bazel
cc_library(
    name = "procedure",
    srcs = [
        "procedure_parser.cpp",
        "procedure_vm.cpp",
        "procedure_trigger_executor.cpp",
    ],
    hdrs = [
        "../include/procedure/procedure_parser.h",
        "../include/procedure/procedure_vm.h",
        "../include/procedure/procedure_trigger_executor.h",
    ],
    includes = ["../include"],
    deps = [
        "//src/sql_parser",
        "//src/execution",
        "//src/logger",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 12. 触发器包 (`//src/trigger`)
```bazel
cc_library(
    name = "trigger",
    srcs = ["trigger_manager.cpp"],
    hdrs = ["../include/trigger/trigger_manager.h"],
    includes = ["../include"],
    deps = [
        "//src/procedure",
        "//src/execution",
        "//src/logger",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 13. 事务管理包 (`//src/transaction`)
```bazel
cc_library(
    name = "transaction",
    srcs = ["savepoint_manager.cpp"],
    hdrs = ["../include/transaction/savepoint_manager.h"],
    includes = ["../include"],
    deps = [
        "//src/storage_engine",
        "//src/logger",
    ],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 14. 类型系统包 (`//src/types`)
```bazel
cc_library(
    name = "types",
    srcs = ["domain_manager.cpp"],
    hdrs = ["../include/types/domain_manager.h"],
    includes = ["../include"],
    deps = ["//src/logger"],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

#### 15. 安全模块包 (`//src/security`)
```bazel
cc_library(
    name = "security",
    srcs = ["memory_monitor.cpp"],
    hdrs = ["../include/security/memory_monitor.h"],
    includes = ["../include"],
    deps = ["//src/logger"],
    copts = ["-std=c++20", "-stdlib=libc++"],
    linkopts = ["-stdlib=libc++"],
    defines = ["SQLCC_MODERN_CPP=1"],
    visibility = ["//visibility:public"],
)
```

### 头文件包定义

#### 工具头文件包 (`//include/utils`)
```bazel
cc_library(
    name = "utils",
    hdrs = [
        "utils/logger.h",
        "utils/config_snapshot.h",
        "utils/config_lifecycle.h",
        "utils/smart_config_manager.h",
        "utils/ssl_wrapper.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 核心头文件包 (`//include/core`)
```bazel
cc_library(
    name = "core",
    hdrs = [
        "core/user_manager.h",
        "core/execution_context.h",
        "core/unified_executor.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 存储头文件包 (`//include/storage`)
```bazel
cc_library(
    name = "storage",
    hdrs = [
        "storage/b_plus_tree.h",
        "storage/b_plus_tree_nodes.h",
        "storage/buffer_pool.h",
        "storage/buffer_pool_fixed.h",
        "storage/buffer_pool_v2.h",
        "storage/buffer_pool_sharded.h",
        "storage/wal_buffer.h",
        "storage/wal_writer.h",
        "storage/checkpoint.h",
        "storage/page_allocator.h",
        "storage/concurrent_access_validator.h",
        "storage/disk_error_handler.h",
        "storage/partition_manager.h",
        "storage/record_boundary_validator.h",
        "storage/replace_strategy.h",
        "storage/cache_consistency_manager.h",
        "storage/advanced_lock_manager.h",
        "storage/data_integrity_validator.h",
        "storage/concurrency_control.h",
        "storage/disk_manager.h",
        "storage/index_manager.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### SQL解析器头文件包 (`//include/sql_parser`)
```bazel
cc_library(
    name = "sql_parser",
    hdrs = [
        "sql_parser/parser.h",
        "sql_parser/lexer.h",
        "sql_parser/token.h",
        "sql_parser/ast_node.h",
        "sql_parser/ast_nodes.h",
        "sql_parser/constraint.h",
        "sql_parser/set_operation.h",
        "sql_parser/window_function.h",
        "sql_parser/recursive_query.h",
        "sql_parser/decimal.h",
        "sql_parser/json.h",
        "sql_parser/datetime.h",
        "sql_parser/function_ast.h",
        "sql_parser/node_visitor.h",
        "sql_parser/load_data_ast.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 执行头文件包 (`//include/execution`)
```bazel
cc_library(
    name = "execution",
    hdrs = [
        "execution/function_executor.h",
        "execution/set_operation_executor.h",
        "execution/window_function_executor.h",
        "execution/recursive_query_executor.h",
        "execution/load_data_executor.h",
        "execution/join_executor.h",
        "execution/procedure_trigger_task.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 网络头文件包 (`//include/network`)
```bazel
cc_library(
    name = "network",
    hdrs = [
        "network/network.h",
        "network/connection_state.h",
        "network/connection_state_machine.h",
        "network/encryption.h",
        "network/data_transmission_validator.h",
        "network/mysql_protocol.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 过程头文件包 (`//include/procedure`)
```bazel
cc_library(
    name = "procedure",
    hdrs = [
        "procedure/procedure_parser.h",
        "procedure/procedure_vm.h",
        "procedure/procedure_trigger_executor.h",
    ],
    visibility = ["//visibility:public"],
)
```

#### 触发器头文件包 (`//include/trigger`)
```bazel
cc_library(
    name = "trigger",
    hdrs = ["trigger/trigger_manager.h"],
    visibility = ["//visibility:public"],
)
```

#### 事务头文件包 (`//include/transaction`)
```bazel
cc_library(
    name = "transaction",
    hdrs = ["transaction/savepoint_manager.h"],
    visibility = ["//visibility:public"],
)
```

#### 类型头文件包 (`//include/types`)
```bazel
cc_library(
    name = "types",
    hdrs = ["types/domain_manager.h"],
    visibility = ["//visibility:public"],
)
```

#### 安全头文件包 (`//include/security`)
```bazel
cc_library(
    name = "security",
    hdrs = ["security/memory_monitor.h"],
    visibility = ["//visibility:public"],
)
```

#### 主接口头文件包
```bazel
cc_library(
    name = "storage_engine",
    hdrs = ["storage_engine.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "sql_executor",
    hdrs = ["sql_executor.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "config_manager",
    hdrs = ["config_manager.h"],
    visibility = ["//visibility:public"],
)
```

### 综合包定义

#### 所有源代码库包 (`//src:all_libs`)
```bazel
cc_library(
    name = "all_libs",
    deps = [
        "//src/logger",
        "//src/core",
        "//src/storage_engine",
        "//src/index_manager",
        "//src/sql_parser",
        "//src/execution",
        "//src/sql_executor",
        "//src/network",
        "//src/config_manager",
        "//src/procedure",
        "//src/trigger",
        "//src/transaction",
        "//src/types",
        "//src/security",
        "//src/utils",
    ],
    visibility = ["//visibility:public"],
)
```

#### 所有头文件包 (`//include:all_headers`)
```bazel
cc_library(
    name = "all_headers",
    deps = [
        "//include/utils",
        "//include/core",
        "//include/storage",
        "//include/sql_parser",
        "//include/execution",
        "//include/network",
        "//include/procedure",
        "//include/trigger",
        "//include/transaction",
        "//include/types",
        "//include/security",
        "//include/storage_engine",
        "//include/sql_executor",
        "//include/config_manager",
    ],
    visibility = ["//visibility:public"],
)
```

---

## 🔗 依赖关系图

### 依赖层次结构
```
高层应用 (sqlcc, isql)
    ↓
SQL执行器 (sql_executor)
    ↓
├── 执行引擎 (execution)
├── SQL解析器 (sql_parser)
├── 存储引擎 (storage_engine)
├── 网络模块 (network)
└── 核心模块 (core)
    ↓
├── 日志系统 (logger)
├── 配置管理 (config_manager)
├── 存储过程 (procedure)
├── 触发器 (trigger)
├── 事务管理 (transaction)
├── 类型系统 (types)
├── 安全模块 (security)
└── 工具库 (utils)
```

### 详细依赖关系

#### 1. 基础层 (无依赖)
- `//src/logger` - 日志系统
- `//src/utils` - 工具库
- `//include/*` - 所有头文件包

#### 2. 核心层 (依赖基础层)
- `//src/core` → `//src/logger`, `//src/utils`
- `//src/storage_engine` → `//src/logger`, `//src/utils`
- `//src/sql_parser` → `//src/logger`, `//src/utils`

#### 3. 执行层 (依赖核心层)
- `//src/execution` → `//src/sql_parser`, `//src/storage_engine`, `//src/logger`
- `//src/network` → `//src/logger`, `//src/utils`

#### 4. 应用层 (依赖执行层)
- `//src/sql_executor` → `//src/sql_parser`, `//src/execution`, `//src/storage_engine`, `//src/core`
- `//src/config_manager` → `//src/logger`

#### 5. 扩展层 (依赖应用层)
- `//src/procedure` → `//src/sql_parser`, `//src/execution`
- `//src/trigger` → `//src/procedure`, `//src/execution`
- `//src/transaction` → `//src/storage_engine`, `//src/logger`
- `//src/types` → `//src/logger`
- `//src/security` → `//src/logger`

#### 6. 主应用 (依赖所有层)
- `//:sqlcc` → `//src/network`, `//src/core`, `//src/sql_executor`, `//src/sql_parser`, `//src/storage_engine`
- `//:isql` → `//src/network`, `//src/core`, `//src/sql_executor`

---

## 🧪 测试包定义

### 单元测试包结构
```bazel
# tests/unit/BUILD.bazel
cc_test(
    name = "logger_test",
    srcs = ["logger_test.cpp"],
    deps = [
        "//src/logger",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "user_manager_test",
    srcs = ["user_manager_test.cpp"],
    deps = [
        "//src/core",
        "@com_google_googletest//:gtest_main",
    ],
)
```

### 集成测试包结构
```bazel
# tests/integration/BUILD.bazel
cc_test(
    name = "sql_executor_integration_test",
    srcs = ["sql_executor_integration_test.cpp"],
    deps = [
        "//src/sql_executor",
        "//src/storage_engine",
        "@com_google_googletest//:gtest_main",
    ],
)
```

### 组件测试包结构
```bazel
# tests/components/BUILD.bazel
cc_test(
    name = "storage_engine_component_test",
    srcs = ["storage_engine_component_test.cpp"],
    deps = [
        "//src/storage_engine",
        "@com_google_googletest//:gtest_main",
    ],
)
```

---

## 🛠️ 工具包定义

### 构建工具
```bazel
# tools/BUILD.bazel
cc_binary(
    name = "bazel_debug",
    srcs = ["bazel_debug.sh"],
    data = ["//:.bazel-errors.md"],
)

cc_binary(
    name = "bazel_fixer",
    srcs = ["bazel_fixer.sh"],
)

cc_binary(
    name = "dependency_graph",
    srcs = ["bazel_dependency_graph.sh"],
)
```

---

## 📋 构建配置

### .bazelrc 配置
```bash
# C++20 现代化配置
build:modern --cxxopt=-std=c++20
build:modern --cxxopt=-stdlib=libc++
build:modern --linkopt=-stdlib=libc++
build:modern --linkopt=-lc++abi
build:modern --define=SQLCC_MODERN_CPP=1
build:modern --define=SQLCC_CLANG18_FEATURES=1

# 调试配置
build:debug --compilation_mode=dbg
build:debug --copt=-g
build:debug --copt=-O0

# 发布配置
build:release --compilation_mode=opt
build:release --copt=-O3
build:release --copt=-DNDEBUG

# 测试配置
build:test --compilation_mode=fastbuild
build:test --test_output=errors
build:test --test_verbose_timeout_warnings
```

### WORKSPACE 配置
```bazel
# WORKSPACE
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# GoogleTest
http_archive(
    name = "com_google_googletest",
    urls = ["https://github.com/google/googletest/archive/release-1.12.1.tar.gz"],
    sha256 = "...",
    strip_prefix = "googletest-release-1.12.1",
)

# Abseil (可选)
http_archive(
    name = "com_google_absl",
    urls = ["https://github.com/abseil/abseil-cpp/archive/20220623.1.tar.gz"],
    sha256 = "...",
    strip_prefix = "abseil-cpp-20220623.1",
)
```

---

## 🎯 构建命令

### 开发构建
```bash
# 构建主应用
bazel build //:sqlcc
bazel build //:isql

# 构建特定库
bazel build //src/storage_engine
bazel build //src/sql_parser

# 现代化构建
bazel build --config=modern //:sqlcc
```

### 测试构建
```bash
# 运行所有测试
bazel test //...

# 运行特定测试
bazel test //tests/unit:logger_test
bazel test //tests/integration:sql_executor_test

# 测试覆盖率
bazel coverage //...
```

### 调试构建
```bash
# 详细错误信息
bazel build --verbose_failures //:sqlcc

# 依赖分析
bazel query "deps(//:sqlcc)" --output=graph

# 构建过程跟踪
bazel build --subcommands //:sqlcc
```

---

## 🔄 维护和更新

### 包依赖检查
```bash
# 定期运行依赖检查
bazel query "deps(//...)" > dependencies.txt

# 检查循环依赖
bazel query "somepath(//..., //...)" | grep -v "^$"
```

### BUILD文件更新
```bash
# 格式化所有BUILD文件
find . -name "BUILD*" -exec buildifier --lint=fix {} \;

# 验证BUILD文件
bazel query //... > /dev/null
```

---

## 📊 性能优化

### 构建缓存策略
- 使用 `--disk_cache` 启用磁盘缓存
- 配置 `--repository_cache` 缓存外部依赖
- 设置 `--experimental_remote_cache` 用于分布式构建

### 增量构建优化
- 合理组织包结构，减少不必要的依赖
- 使用 `cc_library` 的 `srcs` 和 `hdrs` 明确分离
- 避免过度使用 `glob()` 函数

### 并行构建配置
```bash
# 设置并行作业数
bazel build --jobs=8 //...

# 内存限制
bazel build --local_resources=4096,4.0,1.0 //...
```

---

## 🎊 总结

本设计文档提供了SQLCC项目的完整包定义和依赖关系结构：

1. **清晰的分层架构** - 从基础工具到高层应用的逻辑层次
2. **明确的依赖关系** - 每个包的依赖关系清晰定义，避免循环依赖
3. **标准化的包结构** - 统一的命名和组织方式
4. **完整的构建配置** - 支持多种构建模式和优化选项
5. **全面的测试支持** - 单元测试、集成测试和性能测试的完整框架

这个设计确保了SQLCC项目的可维护性、可扩展性和构建效率，为项目的持续发展提供了坚实的技术基础。
