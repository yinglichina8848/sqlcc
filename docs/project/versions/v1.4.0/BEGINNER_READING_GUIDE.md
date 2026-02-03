# SQLCC 新手阅读指南 - 给小彝和小药

**版本**: v1.4.0  
**创建日期**: 2026-02-03  
**目标读者**: 高小彝、高小药等初级开发者  
**作者**: 高小原 🌱

---

## 文档说明

**本文档目标**: 帮助初级开发者理解 SQLCC 源码  
**阅读前提**: 了解 C++ 基础语法、面向对象编程  
**学习方式**: 从上到下，按顺序阅读

---

## 目录

1. [SQLCC 是什么？](#一sqlcc-是什么)
2. [从哪里开始阅读？](#二从哪里开始阅读)
3. [第一阶段：整体认知](#三第一阶段整体认知-1-2-天)
4. [第二阶段：核心模块](#四第二阶段核心模块-3-5-天)
5. [第三阶段：深入理解](#五第三阶段深入理解-3-5-天)
6. [常见问题](#六常见问题)
7. [实践任务](#七实践任务)

---

## 一、SQLCC 是什么？

### 1.1 一句话概括

**SQLCC** = **S**imple **Q**L **C**ompact **C**ourse

> 一个教学用的轻量级关系型数据库系统，帮助你理解数据库内部原理。

### 1.2 SQLCC 能做什么？

你可以把 SQLCC 想象成一个**简单的 MySQL**：

| SQL 语句 | SQLCC 支持？ | 说明 |
|---------|-------------|------|
| `CREATE DATABASE mydb` | ✅ | 创建数据库 |
| `CREATE TABLE users (...)` | ✅ | 创建表 |
| `INSERT INTO users VALUES (...)` | ✅ | 插入数据 |
| `SELECT * FROM users` | ✅ | 查询数据 |
| `UPDATE users SET name = 'xxx'` | ✅ | 更新数据 |
| `DELETE FROM users` | ✅ | 删除数据 |
| `BEGIN; ... COMMIT;` | ✅ | 事务控制 |

### 1.3 SQLCC 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                      SQLCC 数据库系统                      │
│                                                         │
│   你 (输入 SQL)                                           │
│       ↓                                                  │
│   ┌─────────────────────────────────────────────────┐   │
│   │              1. SQL 解析器                        │   │
│   │   (把你的 SQL 翻译成计算机能懂的结构)               │   │
│   └─────────────────────────────────────────────────┘   │
│                         ↓                                │
│   ┌─────────────────────────────────────────────────┐   │
│   │              2. SQL 执行器                        │   │
│   │   (根据翻译后的结构，执行实际操作)                   │   │
│   └─────────────────────────────────────────────────┘   │
│                         ↓                                │
│   ┌─────────────────────────────────────────────────┐   │
│   │              3. 存储引擎                          │   │
│   │   (把数据存到磁盘，或从磁盘读取数据)                │   │
│   └─────────────────────────────────────────────────┘   │
│                         ↓                                │
│   磁盘文件 (data.db)                                    │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 1.4 学习 SQLCC 能学到什么？

| 技术 | 实际应用 |
|------|---------|
| **编译原理** | SQL 解析器是怎么把字符串变成结构的？ |
| **数据结构** | B+ 树、哈希表是怎么存储数据的？ |
| **操作系统** | 磁盘读写、内存缓存是怎么工作的？ |
| **并发编程** | 多个用户同时访问怎么办？ |
| **软件工程** | 大型项目是怎么组织的？ |

---

## 二、从哪里开始阅读？

### 2.1 推荐阅读顺序

```
第 1 天: 先看《新手阅读指南》(本文档)
第 2 天: 看《源码分析报告》
第 3 天: 开始读核心源码
```

### 2.2 核心文件清单

| 文件 | 位置 | 说明 | 推荐度 |
|------|------|------|--------|
| **必读** | | | |
| `core_database_manager.h` | `src/core/` | 数据库管理器入口 | ⭐⭐⭐ |
| `parser.h` | `src/sql_parser/` | SQL 解析器 | ⭐⭐⭐ |
| `sql_executor.h` | `src/sql_executor/` | SQL 执行器 | ⭐⭐⭐ |
| `storage_engine.h` | `src/storage_engine/` | 存储引擎 | ⭐⭐⭐ |
| **推荐** | | | |
| `transaction_manager.h` | `src/transaction/` | 事务管理器 | ⭐⭐ |
| `unified_executor.cpp` | `src/` | 统一执行器 | ⭐⭐ |
| `buffer_pool_sharded.h` | `src/storage_engine/buffer_pool/` | 分片缓冲池 | ⭐⭐ |

### 2.3 目录结构速查

```
sqlcc/src/
├── core/                  # 核心管理（数据库、表、事务）
├── sql_parser/           # SQL 解析（把 SQL 变成结构）
├── sql_executor/         # SQL 执行（执行实际操作）
├── storage_engine/       # 存储引擎（存数据到磁盘）
├── transaction_manager/  # 事务管理（保证数据正确）
├── execution/            # 执行引擎
├── network/              # 网络通信
├── page/                 # 页面管理
└── utils/                # 工具类
```

---

## 三、第一阶段：整体认知 (1-2 天)

### 3.1 目标

- 理解 SQLCC 整体架构
- 知道每个模块是干什么的
- 能说出数据从输入到存储的流程

### 3.2 阅读材料

| 材料 | 说明 | 时间 |
|------|------|------|
| 本文档 | 新手阅读指南 | 30 分钟 |
| `docs/architecture/BEGINNER_GUIDE.md` | 架构分层指南 | 1 小时 |
| `docs/project/versions/v1.4.0/SOURCE_CODE_ANALYSIS_REPORT.md` | 源码分析报告 | 2 小时 |

### 3.3 核心概念

#### 概念 1：什么是 AST？

**AST** = Abstract Syntax Tree（抽象语法树）

**通俗解释**: 
> 把句子拆成结构的树

**例子**:
```
SQL: SELECT name FROM users WHERE id = 1

AST 树:
          SELECT
         /    \
      name    FROM
             /    \
          users   WHERE
                 /    \
                id    =
                     |
                    1
```

#### 概念 2：什么是缓冲池？

**缓冲池** = 内存中的缓存

**通俗解释**:
> 就像你把常用的书放在书桌上，不用每次都去书架拿

**好处**:
- 减少磁盘 IO（磁盘比内存慢很多）
- 提高访问速度

#### 概念 3：什么是事务？

**事务** = 一组操作，要么全做，要么全不做

**通俗解释**:
> 转账：从 A 账户转 100 元到 B 账户
> - 如果只扣了 A 的钱，没加到 B 账户 → 问题！
> - 事务保证：要么都成功，要么都失败

---

## 四、第二阶段：核心模块 (3-5 天)

### 4.1 目标

- 能读懂核心源码
- 理解每个模块内部结构
- 知道关键类和关键函数

### 4.2 SQL 解析器模块

**位置**: `src/sql_parser/`

**模块职责**: 把 SQL 字符串转换成 AST

**核心类**:

| 类名 | 职责 |
|------|------|
| `Lexer` | 词法分析（分词） |
| `Parser` | 语法分析（建树） |
| `ASTNode` | AST 节点基类 |
| `Statement` | 语句节点 |

**阅读路径**:

```
1. parser.h (先看这个，了解整体)
   ↓
2. lexer.h (了解词法分析)
   ↓
3. ast/ast_node.h (了解 AST 结构)
```

**关键代码**:

```cpp
// Parser 的核心功能
class Parser {
public:
    // 把 SQL 字符串转换成 Statement AST
    std::unique_ptr<Statement> parseStatement() {
        // 1. 看第一个词是什么类型
        // 2. 调用相应的解析函数
        // 3. 返回 AST
    }
    
private:
    // 具体的解析函数
    std::unique_ptr<Statement> parseSelectStatement();  // 解析 SELECT
    std::unique_ptr<Statement> parseInsertStatement();   // 解析 INSERT
    std::unique_ptr<Statement> parseUpdateStatement();   // 解析 UPDATE
    // ...
};
```

### 4.3 SQL 执行器模块

**位置**: `src/sql_executor/` + `src/unified_executor.cpp`

**模块职责**: 执行 AST 代表的操作

**核心类**:

| 类名 | 职责 |
|------|------|
| `SqlExecutor` | SQL 执行器主类 |
| `UnifiedExecutor` | 统一执行器 |
| `ExecutionStrategy` | 执行策略 |
| `ExecutionContext` | 执行上下文 |

**阅读路径**:

```
1. sql_executor.h (了解整体结构)
   ↓
2. unified_executor.cpp (看具体实现)
   ↓
3. execution/ (看执行策略)
```

**关键代码**:

```cpp
// 执行器核心逻辑
class SqlExecutor {
public:
    ExecutionResult Execute(const std::string& sql) {
        // 1. 调用解析器，把 SQL 变成 AST
        auto ast = parser.parse(sql);
        
        // 2. 根据 AST 类型，调用相应的执行方法
        switch (ast->type) {
            case Statement::SELECT:
                return executeSelect(ast);
            case Statement::INSERT:
                return executeInsert(ast);
            // ...
        }
    }
};
```

### 4.4 存储引擎模块

**位置**: `src/storage_engine/`

**模块职责**: 数据的物理存储和读取

**核心类**:

| 类名 | 职责 |
|------|------|
| `StorageEngine` | 存储引擎主类 |
| `BufferPool` | 缓冲池管理 |
| `DiskManager` | 磁盘读写 |
| `TableStorage` | 表存储 |

**阅读路径**:

```
1. storage_engine.h (了解整体)
   ↓
2. buffer_pool/buffer_pool.h (了解缓存)
   ↓
3. disk_manager.h (了解磁盘读写)
```

**关键代码**:

```cpp
// 存储引擎核心功能
class StorageEngine {
public:
    // 读取页面
    Page* ReadPage(PageId id) {
        // 1. 先看缓冲池里有没有
        if (auto page = buffer_pool_.Get(id)) {
            return page;  // 直接返回
        }
        
        // 2. 缓冲池没有，从磁盘读
        auto page = disk_manager_.Read(id);
        
        3. 放入缓冲池
        buffer_pool_.Put(id, page);
        
        return page;
    }
};
```

---

## 五、第三阶段：深入理解 (3-5 天)

### 5.1 目标

- 理解模块间交互
- 理解关键技术细节
- 能进行小规模修改

### 5.2 事务管理器模块

**位置**: `src/transaction/` + `src/transaction_manager/`

**模块职责**: 保证 ACID 特性

**核心类**:

| 类名 | 职责 |
|------|------|
| `TransactionManager` | 事务管理器 |
| `TransactionContext` | 事务上下文 |
| `WALManager` | 预写日志 |

**ACID 实现**:

| 特性 | 实现方式 |
|------|---------|
| 原子性 | WAL (先写日志再操作) |
| 一致性 | 约束检查 |
| 隔离性 | MVCC、锁 |
| 持久性 | 日志 + 磁盘同步 |

### 5.3 核心数据库管理器

**位置**: `src/core/core_database_manager.h`

**模块职责**: 统一管理入口

**核心类**:

| 类名 | 职责 |
|------|------|
| `DatabaseManager` | 数据库管理器 |
| `SchemaManager` | 模式管理 |
| `TableManager` | 表管理 |

### 5.4 组件交互

```
用户输入 SQL
    ↓
SQL 解析器 (parser.h) → AST
    ↓
SQL 执行器 (sql_executor.h) → 判断类型
    ↓
    ├── SELECT → 存储引擎读取数据
    ├── INSERT → 存储引擎写入数据
    ├── UPDATE → 存储引擎更新数据
    └── DELETE → 存储引擎删除数据
    ↓
事务管理器 (transaction_manager.h) → 保证 ACID
    ↓
存储引擎 (storage_engine.h) → 磁盘读写
    ↓
返回结果
```

---

## 六、常见问题

### Q1: 什么是页面 (Page)？

**答**: 页面是数据库存储的基本单位，通常是 8KB。

**为什么需要页面？**:
- 磁盘读写以块为单位
- 8KB 是一个折中：不太大也不太小
- 内存管理也以页面为单位

### Q2: 什么是 B+ 树？

**答**: B+ 树是一种数据结构，用于索引。

**为什么用它？**:
- 查找效率高 O(log n)
- 支持范围查询
- 适合磁盘存储

### Q3: 什么是 LRU？

**答**: LRU = Least Recently Used（最近最少使用）

**用途**: 缓冲池满了的时候，淘汰最久没用的页面。

### Q4: 头文件里的 `Why`、`What`、`How` 注释是什么意思？

**答**: 这是一种文档风格：

- **Why**: 为什么要这个类/函数？
- **What**: 这个类/函数是什么？
- **How**: 它是怎么工作的？

**例子**:
```cpp
/**
 * WHY: 需要把 SQL 转换成计算机能懂的结构
 * WHAT: SQL 解析器
 * HOW: 先分词，再建树
 */
class Parser { ... };
```

---

## 七、实践任务

### 任务 1：追踪一条 SELECT 语句

**目标**: 理解数据从输入到返回的流程

**步骤**:
1. 在 `unified_executor.cpp` 中找到 `executeSelect` 函数
2. 追踪调用链
3. 画出流程图

**预期输出**: 一张流程图 + 关键代码注释

### 任务 2：添加一个新的 SQL 语句

**目标**: 理解解析器和执行器的协作

**任务**: 添加 `SHOW TABLES` 语句

**步骤**:
1. 在 `parser.h` 中添加 `parseShowTablesStatement`
2. 在 `sql_executor.h` 中添加 `executeShowTables`
3. 测试是否正常工作

### 任务 3：阅读缓冲池代码

**目标**: 理解缓存机制

**任务**: 画出缓冲池的类图

**步骤**:
1. 阅读 `buffer_pool.h` 和 `buffer_pool_sharded.h`
2. 找出关键成员变量和方法
3. 画出类图

---

## 参考资料

### 内部资料

| 文档 | 说明 |
|------|------|
| `docs/architecture/BEGINNER_GUIDE.md` | 架构分层指南 |
| `docs/project/versions/v1.4.0/SOURCE_CODE_ANALYSIS_REPORT.md` | 源码分析报告 |
| `docs/design/sql_parser/sql_parser_design.md` | SQL 解析器设计 |
| `docs/design/storage_engine/storage_engine_overview.md` | 存储引擎概述 |

### 推荐外部资料

| 资源 | 说明 |
|------|------|
| 《数据库系统概念》 | 经典教材 |
| 《编译原理》 | 龙书 |

---

## 学习时间规划

| 阶段 | 时间 | 内容 | 输出 |
|------|------|------|------|
| 第一阶段 | 1-2 天 | 整体认知 | 能说出架构 |
| 第二阶段 | 3-5 天 | 核心模块 | 能读懂代码 |
| 第三阶段 | 3-5 天 | 深入理解 | 能做小修改 |
| 实践 | 1 周 | 实践任务 | 完成 3 个任务 |

---

## 遇到问题怎么办？

1. **先看注释**: 源码里有详细的 `///` 注释
2. **再看设计文档**: `docs/design/` 下有详细设计
3. **画图理解**: 画类图、流程图
4. **写测试**: 加打印语句调试
5. **问高小原**: 实在不懂就问

---

**祝你学习顺利！** 🎉

有任何问题，随时问高小原！💪

---

**文档版本**: 1.0  
**创建时间**: 2026-02-03  
**最后更新**: 2026-02-03  
**作者**: 高小原 🌱
