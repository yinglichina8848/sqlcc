# SQLCC可视化架构设计文档

## 1. 系统架构概览

### 1.1 整体系统架构图

```mermaid
graph TB
    subgraph "客户端层"
        CLI[命令行接口]
        GUI[图形界面]
        API[应用程序接口]
    end
    
    subgraph "协议层"
        NET[网络通信模块]
        AUTH[认证模块]
        ENCRYPT[加密传输]
    end
    
    subgraph "SQL处理层"
        PARSER[SQL解析器]
        OPT[查询优化器]
        PLAN[执行计划生成器]
    end
    
    subgraph "执行层"
        DDL_EXEC[DDL执行器]
        DML_EXEC[DML执行器]
        DCL_EXEC[DCL执行器]
        TXN_MGR[事务管理器]
    end
    
    subgraph "存储层"
        BUF_POOL[缓冲池]
        STORAGE[存储引擎]
        LOG[WAL日志]
        INDEX[索引管理]
    end
    
    subgraph "系统层"
        CONFIG[配置管理]
        METADATA[元数据管理]
        PRIV[权限管理]
        UTIL[工具函数]
    end
    
    CLI --> NET
    GUI --> NET
    API --> NET
    
    NET --> AUTH
    AUTH --> ENCRYPT
    ENCRYPT --> PARSER
    
    PARSER --> OPT
    OPT --> PLAN
    
    PLAN --> DDL_EXEC
    PLAN --> DML_EXEC
    PLAN --> DCL_EXEC
    
    DDL_EXEC --> TXN_MGR
    DML_EXEC --> TXN_MGR
    DCL_EXEC --> TXN_MGR
    
    TXN_MGR --> BUF_POOL
    BUF_POOL --> STORAGE
    STORAGE --> LOG
    STORAGE --> INDEX
    
    TXN_MGR --> METADATA
    TXN_MGR --> PRIV
    TXN_MGR --> CONFIG
    
    DDL_EXEC --> UTIL
    DML_EXEC --> UTIL
    DCL_EXEC --> UTIL
```

## 2. 核心模块类图

### 2.1 核心系统类图

```mermaid
classDiagram
    class DatabaseManager {
        -shared_ptr~ConfigManager~ config_manager_
        -shared_ptr~StorageEngine~ storage_engine_
        -shared_ptr~TransactionManager~ txn_manager_
        -shared_ptr~BufferPoolSharded~ buffer_pool_
        -string db_path_
        -string current_database_
        +bool CreateDatabase(string db_name)
        +bool DropDatabase(string db_name)
        +bool UseDatabase(string db_name)
        +bool CreateTable(string table_name, vector~pair~string, string~~ columns)
        +bool DropTable(string table_name)
        +TransactionId BeginTransaction(IsolationLevel isolation)
        +bool CommitTransaction(TransactionId txn_id)
        +bool RollbackTransaction(TransactionId txn_id)
    }
    
    class StorageEngine {
        -unique_ptr~DiskManager~ disk_manager_
        -unique_ptr~BufferPoolSharded~ buffer_pool_
        -ConfigManager& config_manager_
        +Page* NewPage(int32_t* page_id)
        +Page* FetchPage(int32_t page_id)
        +bool UnpinPage(int32_t page_id, bool is_dirty)
        +bool FlushPage(int32_t page_id)
        +bool DeletePage(int32_t page_id)
        +void FlushAllPages()
    }
    
    class TransactionManager {
        -unordered_map~TransactionId, Transaction~ active_txns_
        -shared_mutex txn_table_mutex_
        -LockManager lock_manager_
        +TransactionId BeginTransaction(IsolationLevel isolation)
        +bool CommitTransaction(TransactionId txn_id)
        +bool RollbackTransaction(TransactionId txn_id)
        +bool AcquireLock(TransactionId txn_id, string key, LockMode mode)
        +bool ReleaseLock(TransactionId txn_id, string key)
    }
    
    class BufferPoolSharded {
        -vector~unique_ptr~Shard~~ shards_
        -DiskManager* disk_manager_
        -ConfigManager& config_manager_
        -size_t num_shards_
        +Page* FetchPage(int32_t page_id, bool exclusive)
        +bool FlushPage(int32_t page_id)
        +bool UnpinPage(int32_t page_id, bool is_dirty)
        +Page* NewPage(int32_t* page_id)
    }
    
    class ExecutionEngine {
        -unique_ptr~SQLParser~ parser_
        -unique_ptr~QueryOptimizer~ optimizer_
        -unique_ptr~PlanExecutor~ executor_
        +unique_ptr~ResultSet~ ExecuteSQL(string sql, TransactionId txn_id)
    }
    
    DatabaseManager --> StorageEngine
    DatabaseManager --> TransactionManager
    DatabaseManager --> BufferPoolSharded
    
    StorageEngine --> BufferPoolSharded
    TransactionManager --> BufferPoolSharded
    
    ExecutionEngine --> TransactionManager
```

### 2.2 SQL执行器类图

```mermaid
classDiagram
    class SQLExecutor {
        <<abstract>>
        #DatabaseManager* db_manager_
        #TransactionId txn_id_
        +Execute(unique_ptr~ASTNode~ node) unique_ptr~ResultSet~~
    }
    
    class DDLExecutor {
        -shared_ptr~SystemDatabase~ sys_db_
        +Execute(unique_ptr~ASTNode~ node) unique_ptr~ResultSet~~
        +ExecuteCreateDatabase(unique_ptr~CreateDatabaseNode~ node) bool
        +ExecuteDropDatabase(unique_ptr~DropDatabaseNode~ node) bool
        +ExecuteCreateTable(unique_ptr~CreateTableNode~ node) bool
        +ExecuteDropTable(unique_ptr~DropTableNode~ node) bool
        +ExecuteCreateIndex(unique_ptr~CreateIndexNode~ node) bool
        +ExecuteDropIndex(unique_ptr~DropIndexNode~ node) bool
    }
    
    class DMLExecutor {
        -shared_ptr~QueryExecutor~ query_executor_
        -shared_ptr~IndexManager~ index_manager_
        +Execute(unique_ptr~ASTNode~ node) unique_ptr~ResultSet~~
        +ExecuteSelect(unique_ptr~SelectNode~ node) unique_ptr~ResultSet~~
        +ExecuteInsert(unique_ptr~InsertNode~ node) unique_ptr~ResultSet~~
        +ExecuteUpdate(unique_ptr~UpdateNode~ node) unique_ptr~ResultSet~~
        +ExecuteDelete(unique_ptr~DeleteNode~ node) unique_ptr~ResultSet~~
    }
    
    class DCLExecutor {
        -shared_ptr~UserManager~ user_manager_
        +Execute(unique_ptr~ASTNode~ node) unique_ptr~ResultSet~~
        +ExecuteGrant(unique_ptr~GrantNode~ node) bool
        +ExecuteRevoke(unique_ptr~RevokeNode~ node) bool
        +ExecuteCreateUser(unique_ptr~CreateUserNode~ node) bool
        +ExecuteDropUser(unique_ptr~DropUserNode~ node) bool
    }
    
    SQLExecutor <|-- DDLExecutor
    SQLExecutor <|-- DMLExecutor
    SQLExecutor <|-- DCLExecutor
    
    DDLExecutor --> SystemDatabase
    DMLExecutor --> QueryExecutor
    DMLExecutor --> IndexManager
    DCLExecutor --> UserManager
```

## 3. 组件协作图

### 3.1 SQL查询执行协作图

```mermaid
sequenceDiagram
    participant Client
    participant ExecutionEngine
    participant Parser
    participant Optimizer
    participant PlanExecutor
    participant TransactionManager
    participant StorageEngine
    
    Client->>ExecutionEngine: ExecuteSQL(sql)
    ExecutionEngine->>Parser: Parse(sql)
    Parser-->>ExecutionEngine: AST
    ExecutionEngine->>Optimizer: Optimize(AST)
    Optimizer-->>ExecutionEngine: ExecutionPlan
    ExecutionEngine->>TransactionManager: BeginTransaction()
    TransactionManager-->>ExecutionEngine: txn_id
    ExecutionEngine->>PlanExecutor: Execute(plan, txn_id)
    
    PlanExecutor->>StorageEngine: FetchPage(page_id)
    StorageEngine-->>PlanExecutor: Page*
    
    PlanExecutor->>TransactionManager: AcquireLock(key, mode)
    TransactionManager-->>PlanExecutor: Lock granted
    
    PlanExecutor-->>ExecutionEngine: ResultSet
    ExecutionEngine->>TransactionManager: CommitTransaction(txn_id)
    TransactionManager-->>ExecutionEngine: Success
    
    ExecutionEngine-->>Client: ResultSet
```

### 3.2 事务处理协作图

```mermaid
sequenceDiagram
    participant Client
    participant TransactionManager
    participant LockManager
    participant StorageEngine
    participant WALManager
    
    Client->>TransactionManager: BeginTransaction()
    TransactionManager-->>Client: txn_id
    
    Client->>TransactionManager: AcquireLock(txn_id, key, exclusive)
    TransactionManager->>LockManager: Acquire(txn_id, key, exclusive)
    LockManager-->>TransactionManager: Granted
    TransactionManager-->>Client: Success
    
    Client->>TransactionManager: WriteData(txn_id, data)
    TransactionManager->>WALManager: WriteLog(txn_id, operation)
    WALManager-->>TransactionManager: LSN
    TransactionManager->>StorageEngine: WriteData(page, data)
    StorageEngine-->>TransactionManager: Success
    TransactionManager-->>Client: Success
    
    Client->>TransactionManager: CommitTransaction(txn_id)
    TransactionManager->>WALManager: Commit(txn_id)
    WALManager-->>TransactionManager: Committed
    TransactionManager->>LockManager: ReleaseAll(txn_id)
    LockManager-->>TransactionManager: Released
    TransactionManager-->>Client: Success
```

## 4. 状态图

### 4.1 事务状态图

```mermaid
stateDiagram-v2
    [*] --> ACTIVE: BeginTransaction()
    ACTIVE --> COMMITTED: Commit()
    ACTIVE --> ABORTED: Rollback()
    
    ACTIVE --> PARTIALLY_COMMITTED: Prepare()
    PARTIALLY_COMMITTED --> COMMITTED: Commit()
    PARTIALLY_COMMITTED --> ABORTED: Rollback()
    
    ABORTED --> [*]
    COMMITTED --> [*]
    
    note right of ACTIVE: 事务执行中
    note right of PARTIALLY_COMMITTED: 两阶段提交准备阶段
    note right of COMMITTED: 事务已提交
    note right of ABORTED: 事务已回滚
```

### 4.2 页面状态图

```mermaid
stateDiagram-v2
    [*] --> NEW: NewPage()
    NEW --> CLEAN: UnpinPage(dirty=false)
    NEW --> DIRTY: Write()
    
    CLEAN --> PINNED: FetchPage()
    DIRTY --> PINNED: FetchPage()
    
    PINNED --> CLEAN: UnpinPage(dirty=false)
    PINNED --> DIRTY: Write()
    
    DIRTY --> FLUSHING: FlushPage()
    FLUSHING --> CLEAN: Flush Complete
    FLUSHING --> DIRTY: Flush Failed
    
    CLEAN --> DELETED: DeletePage()
    PINNED --> DELETED: DeletePage()
    DELETED --> [*]
    
    note right of NEW: 新创建的页面
    note right of CLEAN: 干净页面
    note right of DIRTY: 脏页面
    note right of PINNED: 被固定的页面
    note right of FLUSHING: 正在刷盘的页面
```

## 5. 活动图

### 5.1 SQL查询执行活动图

```mermaid
activityDiagram
    start
    :接收SQL查询;
    :解析SQL;
    :生成语法树;
    
    if (语法正确?) then (是)
        :语义分析;
        :查询优化;
        :生成执行计划;
        
        if (需要事务?) then (是)
            :开始事务;
        else (否)
            :使用现有事务;
        endif
        
        :执行计划;
        
        while (还有操作?) then (是)
            :获取数据锁;
            :执行操作;
            :释放锁;
        endwhile (否)
        
        :提交事务;
        :返回结果;
    else (否)
        :返回语法错误;
    endif
    
    end
```

### 5.2 缓冲池管理活动图

```mermaid
activityDiagram
    start
    :接收页面请求;
    
    if (页面在缓冲池?) then (是)
        :增加引用计数;
        :移动到LRU头部;
        :返回页面指针;
    else (否)
        if (缓冲池已满?) then (是)
            :选择替换页面;
            
            if (页面为脏页?) then (是)
                :写入磁盘;
            else (否)
                :直接替换;
            endif
            
            :从磁盘加载页面;
            :更新缓冲池;
        else (否)
            :从磁盘加载页面;
            :添加到缓冲池;
        endif
        
        :设置引用计数为1;
        :返回页面指针;
    endif
    
    :等待页面释放;
    :减少引用计数;
    
    if (引用计数为0?) then (是)
        if (页面为脏页?) then (是)
            :标记为脏页;
        endif
        
        :移动到LRU尾部;
    endif
    
    end
```

## 6. 部署图

### 6.1 单机部署图

```mermaid
deployment
    node "应用服务器" {
        artifact "SQLCC Server" {
            component "SQL处理层" {
                component "SQL解析器"
                component "查询优化器"
                component "执行计划生成器"
            }
            
            component "执行层" {
                component "DDL执行器"
                component "DML执行器"
                component "DCL执行器"
                component "事务管理器"
            }
            
            component "存储层" {
                component "缓冲池"
                component "存储引擎"
                component "索引管理器"
            }
            
            component "系统层" {
                component "配置管理器"
                component "元数据管理器"
                component "权限管理器"
            }
        }
        
        artifact "SQL文件" {
            [数据文件]
            [日志文件]
            [配置文件]
        }
    }
    
    node "客户端设备" {
        artifact "SQLCC CLI"
        artifact "SQLCC GUI"
    }
    
    "客户端设备" --> "应用服务器": 网络连接
```

### 6.2 分布式部署图

```mermaid
deployment
    node "负载均衡器" {
        component "负载均衡服务"
    }
    
    node "SQLCC节点1" {
        artifact "SQLCC Server1"
        artifact "本地存储1"
    }
    
    node "SQLCC节点2" {
        artifact "SQLCC Server2"
        artifact "本地存储2"
    }
    
    node "SQLCC节点3" {
        artifact "SQLCC Server3"
        artifact "本地存储3"
    }
    
    node "协调服务集群" {
        component "配置管理服务"
        component "元数据管理服务"
        component "分布式锁服务"
    }
    
    node "客户端设备" {
        artifact "SQLCC客户端"
    }
    
    "客户端设备" --> "负载均衡器"
    "负载均衡器" --> "SQLCC节点1"
    "负载均衡器" --> "SQLCC节点2"
    "负载均衡器" --> "SQLCC节点3"
    
    "SQLCC节点1" --> "协调服务集群"
    "SQLCC节点2" --> "协调服务集群"
    "SQLCC节点3" --> "协调服务集群"
    
    "SQLCC节点1" -.-> "SQLCC节点2" : 数据同步
    "SQLCC节点2" -.-> "SQLCC节点3" : 数据同步
    "SQLCC节点3" -.-> "SQLCC节点1" : 数据同步
```

## 7. 继承关系图

### 7.1 AST节点继承关系图

```mermaid
classDiagram
    class ASTNode {
        <<abstract>>
        #NodeType node_type_
        +Accept(NodeVisitor* visitor)
        +NodeType GetType()
    }
    
    class StatementNode {
        <<abstract>>
        +Accept(NodeVisitor* visitor)
    }
    
    class ExpressionNode {
        <<abstract>>
        +Accept(NodeVisitor* visitor)
    }
    
    class DDLNode {
        <<abstract>>
        +Accept(NodeVisitor* visitor)
    }
    
    class DMLNode {
        <<abstract>>
        +Accept(NodeVisitor* visitor)
    }
    
    class DCLNode {
        <<abstract>>
        +Accept(NodeVisitor* visitor)
    }
    
    class CreateTableNode {
        -string table_name_
        -vector~ColumnDef~ columns_
        -vector~Constraint~ constraints_
        +Accept(NodeVisitor* visitor)
    }
    
    class DropTableNode {
        -string table_name_
        +Accept(NodeVisitor* visitor)
    }
    
    class SelectNode {
        -vector~unique_ptr~ExpressionNode~~ select_list_
        -vector~unique_ptr~TableRefNode~~ from_list_
        -unique_ptr~ExpressionNode~ where_clause_
        -vector~unique_ptr~OrderByNode~~ order_by_
        +Accept(NodeVisitor* visitor)
    }
    
    class InsertNode {
        -string table_name_
        -vector~string~ columns_
        -vector~unique_ptr~ExpressionNode~~ values_
        +Accept(NodeVisitor* visitor)
    }
    
    class GrantNode {
        -vector~PrivilegeType~ privileges_
        -string object_name_
        -string user_name_
        +Accept(NodeVisitor* visitor)
    }
    
    ASTNode <|-- StatementNode
    ASTNode <|-- ExpressionNode
    
    StatementNode <|-- DDLNode
    StatementNode <|-- DMLNode
    StatementNode <|-- DCLNode
    
    DDLNode <|-- CreateTableNode
    DDLNode <|-- DropTableNode
    
    DMLNode <|-- SelectNode
    DMLNode <|-- InsertNode
    
    DCLNode <|-- GrantNode
```

### 7.2 存储组件继承关系图

```mermaid
classDiagram
    class Page {
        <<abstract>>
        #int32_t page_id_
        #bool is_dirty_
        +int32_t GetPageId()
        +bool IsDirty()
        +void MarkDirty()
    }
    
    class DataPage {
        -int32_t record_count_
        -vector~Slot~ slots_
        +bool InsertRecord(const Record& record)
        +bool DeleteRecord(int slot_id)
        +bool UpdateRecord(int slot_id, const Record& record)
    }
    
    class IndexPage {
        -bool is_leaf_
        -int32_t key_count_
        -vector~IndexEntry~ entries_
        +bool InsertKey(const KeyType& key, int32_t child_page_id)
        +bool DeleteKey(const KeyType& key)
    }
    
    class MetadataPage {
        -int32_t table_id_
        -string table_name_
        -vector~ColumnMeta~ column_metas_
        +TableMetadata GetMetadata()
    }
    
    class BufferFrame {
        -Page* page_
        -int ref_count_
        -bool is_dirty_
        -list_iterator lru_iter_
        +Page* GetPage()
        +void IncrementRefCount()
        +void DecrementRefCount()
    }
    
    Page <|-- DataPage
    Page <|-- IndexPage
    Page <|-- MetadataPage
```

## 8. 数据流图

### 8.1 查询处理数据流图

```mermaid
graph TD
    subgraph "输入数据"
        A[SQL查询字符串]
        B[连接参数]
    end
    
    subgraph "处理过程"
        C[词法分析]
        D[语法分析]
        E[语义分析]
        F[查询重写]
        G[查询优化]
        H[执行计划生成]
        I[计划执行]
    end
    
    subgraph "输出数据"
        J[结果集]
        K[执行状态]
        L[错误信息]
    end
    
    subgraph "中间数据"
        M[Token序列]
        N[抽象语法树]
        O[查询树]
        P[逻辑计划]
        Q[物理计划]
        R[执行状态]
    end
    
    A --> C
    B --> C
    C --> M
    M --> D
    D --> N
    N --> E
    E --> O
    O --> F
    F --> P
    P --> G
    G --> Q
    Q --> H
    H --> I
    I --> J
    I --> K
    I --> L
    
    E -.-> L
    F -.-> L
    G -.-> L
    I -.-> R
```

### 8.2 存储层数据流图

```mermaid
graph TD
    subgraph "客户端请求"
        A[读请求]
        B[写请求]
        C[扫描请求]
    end
    
    subgraph "缓冲池处理"
        D[查找页面]
        E[页面命中]
        F[页面未命中]
        G[替换策略]
        H[磁盘加载]
        I[页面更新]
        J[刷盘策略]
    end
    
    subgraph "存储引擎"
        K[磁盘I/O]
        L[WAL写入]
        M[文件管理]
    end
    
    subgraph "返回结果"
        N[数据页面]
        O[操作结果]
    end
    
    A --> D
    B --> D
    C --> D
    
    D --> E
    E --> N
    
    D --> F
    F --> G
    G --> H
    H --> N
    
    B --> I
    I --> L
    I --> J
    J --> K
    
    L --> O
    K --> O
```

## 9. 系统交互图

### 9.1 完整SQL执行交互图

```mermaid
sequenceDiagram
    participant Client
    participant Connection
    participant Parser
    participant AuthManager
    participant Executor
    participant TransactionManager
    participant LockManager
    participant BufferPool
    participant StorageEngine
    participant WALManager
    
    Client->>Connection: Connect(username, password)
    Connection->>AuthManager: Authenticate(username, password)
    AuthManager-->>Connection: AuthenticationResult
    Connection-->>Client: Connection established
    
    Client->>Connection: ExecuteSQL(sql)
    Connection->>Parser: Parse(sql)
    Parser-->>Connection: ParseTree
    
    Connection->>Executor: Execute(ParseTree)
    Executor->>TransactionManager: BeginTransaction()
    TransactionManager-->>Executor: txn_id
    
    loop For each operation
        Executor->>LockManager: AcquireLock(txn_id, resource, mode)
        LockManager-->>Executor: Lock granted
        
        Executor->>BufferPool: GetPage(page_id)
        
        alt Page in buffer
            BufferPool-->>Executor: Page
        else Page not in buffer
            BufferPool->>StorageEngine: ReadPage(page_id)
            StorageEngine-->>BufferPool: PageData
            BufferPool-->>Executor: Page
        end
        
        Executor->>WALManager: WriteLog(txn_id, operation)
        WALManager-->>Executor: LSN
        
        Executor->>BufferPool: UpdatePage(page_id, data)
        BufferPool-->>Executor: Success
    end
    
    Executor->>TransactionManager: CommitTransaction(txn_id)
    TransactionManager->>WALManager: Commit(txn_id)
    WALManager-->>TransactionManager: Committed
    TransactionManager->>LockManager: ReleaseAll(txn_id)
    LockManager-->>TransactionManager: Released
    TransactionManager-->>Executor: Success
    
    Executor-->>Connection: ResultSet
    Connection-->>Client: ResultSet
    
    Client->>Connection: Close()
    Connection-->>Client: Connection closed
```

## 10. 结论

本文档通过多种UML和Mermaid图直观展示了SQLCC系统的架构设计，包括：

1. **整体架构图**：展示了系统各层次之间的关系
2. **类图**：详细描述了核心类及其关系
3. **协作图**：展示了组件间的交互流程
4. **状态图**：描述了关键对象的状态转换
5. **活动图**：展示了业务流程的执行步骤
6. **部署图**：描述了系统的部署方式
7. **继承关系图**：展示了类之间的继承层次
8. **数据流图**：展示了数据在系统中的流动
9. **交互图**：完整展示了系统各组件的交互流程

这些可视化图表有助于开发团队更好地理解系统架构，也为后续的系统优化和扩展提供了清晰的参考。通过图表方式展示，避免了复杂的源码细节，更加直观地呈现了系统的设计思路和实现方案。