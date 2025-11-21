# SQLCC 单机版功能增强计划

## 概述

本文档描述SQLCC单机版数据库系统的功能增强计划，重点解决当前执行层（SqlExecutor）的根本性缺陷，实现真实的SQL执行逻辑、完整的事务处理和灾难恢复能力。该计划是分布式扩展的基础阶段。

## 当前问题分析

### 🔴 关键缺陷
1. **SqlExecutor执行层完全缺失**：所有方法仅为桩代码
2. **无真实SQL执行能力**：无法处理任何实际数据库操作
3. **事务处理缺失**：无ACID保证机制
4. **灾难恢复不足**：缺乏完整的数据保护机制

### 🟢 架构基础优势
1. **StorageEngine设计良好**：模块化架构，接口清晰
2. **存储层完整**：DiskManager、BufferPool、IndexManager已实现
3. **配置管理完善**：ConfigManager提供灵活配置

## 增强目标

### 核心目标
- **真实SQL执行**：实现完整的CRUD操作能力
- **事务ACID保证**：WAL + 两阶段锁协议
- **灾难恢复**：自动备份和恢复机制
- **性能优化**：查询执行计划和索引优化
- **约束完整**：主键、外键、唯一性约束

### 性能指标
- **查询响应时间**：< 5ms（单表查询）
- **事务吞吐量**：> 1000 TPS
- **数据一致性**：ACID属性完整保证
- **恢复时间**：< 30秒（10GB数据）
- **空间效率**：存储开销 < 20%

## 详细实施计划

### 阶段一：执行层重构（2-3周）

#### 1.1 SqlExecutor核心重写

##### 1.1.1 查询执行架构
```cpp
// 新的查询执行器架构
class QueryExecutor {
private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::unique_ptr<QueryPlanner> query_planner_;
    std::unique_ptr<QueryOptimizer> query_optimizer_;
    std::unique_ptr<PlanExecutor> plan_executor_;
    
public:
    // 查询执行接口
    QueryResult ExecuteQuery(const sql_parser::Statement& statement);
    QueryResult ExecuteSelect(const SelectStatement& select);
    ResultSet ExecuteInsert(const InsertStatement& insert);
    ResultSet ExecuteUpdate(const UpdateStatement& update);
    ResultSet ExecuteDelete(const DeleteStatement& delete_stmt);
};
```

##### 1.1.2 查询计划器
```cpp
class QueryPlanner {
public:
    // 生成执行计划
    ExecutionPlan CreatePlan(const SelectStatement& statement);
    
    // 逻辑计划生成
    LogicalPlan CreateLogicalPlan(const Statement& statement);
    
    // 物理计划生成
    PhysicalPlan CreatePhysicalPlan(const LogicalPlan& logical_plan);
    
private:
    // 子系统接口
    std::unique_ptr<ProjectionPlanner> projection_planner_;
    std::unique_ptr<SelectionPlanner> selection_planner_;
    std::unique_ptr<JoinPlanner> join_planner_;
    std::unique_ptr<SortPlanner> sort_planner_;
};

enum class PlanNodeType {
    SCAN,           // 表扫描
    SELECT,         // 选择过滤
    PROJECT,        // 投影
    JOIN,           // 连接
    SORT,           // 排序
    AGGREGATE,      // 聚合
    INSERT,         // 插入
    UPDATE,         // 更新
    DELETE          // 删除
};

struct ExecutionPlan {
    PlanNodeType node_type;
    std::vector<std::unique_ptr<ExecutionPlan>> children;
    std::map<std::string, std::any> properties;
    double estimated_cost;
};
```

##### 1.1.3 计划执行器
```cpp
class PlanExecutor {
public:
    ResultSet Execute(const ExecutionPlan& plan, TransactionContext* txn);
    
private:
    // 具体的执行方法
    ResultSet ExecuteScan(const ScanPlan& plan, TransactionContext* txn);
    ResultSet ExecuteSelect(const SelectPlan& plan, TransactionContext* txn);
    ResultSet ExecuteJoin(const JoinPlan& plan, TransactionContext* txn);
    ResultSet ExecuteProject(const ProjectPlan& plan, TransactionContext* txn);
};
```

#### 1.2 存储引擎接口增强

##### 1.2.1 表管理器
```cpp
class TableManager {
public:
    // 表操作
    Status CreateTable(const TableDefinition& definition);
    Status DropTable(const std::string& table_name);
    Status AlterTable(const std::string& table_name, const AlterOperation& operation);
    
    // 记录操作
    ResultSet ScanTable(const std::string& table_name, const ScanCondition& condition);
    Status InsertRecord(const std::string& table_name, const Record& record);
    Status UpdateRecords(const std::string& table_name, const Record& new_record, const Condition& condition);
    Status DeleteRecords(const std::string& table_name, const Condition& condition);
    
    // 元数据
    TableSchema GetTableSchema(const std::string& table_name);
    std::vector<std::string> ListTables();
    
private:
    std::map<std::string, std::unique_ptr<Table>> tables_;
    std::unique_ptr<MetadataManager> metadata_manager_;
};
```

##### 1.2.2 记录管理器
```cpp
class RecordManager {
public:
    // 记录操作
    Status InsertRecord(int32_t page_id, const Record& record);
    Status UpdateRecord(int32_t page_id, const Record& old_record, const Record& new_record);
    Status DeleteRecord(int32_t page_id, const Record& record);
    std::vector<Record> ScanRecords(int32_t page_id, const ScanCondition& condition);
    
    // 记录格式
    size_t SerializeRecord(const Record& record, char* buffer);
    Record DeserializeRecord(const char* buffer, size_t length);
    
private:
    RecordFormat GetRecordFormat(const TableSchema& schema);
};
```

### 阶段二：事务处理实现（1-2周）

#### 2.1 事务管理器重构

##### 2.1.1 事务上下文
```cpp
class TransactionContext {
public:
    // 事务生命周期
    void BeginTransaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED);
    void CommitTransaction();
    void RollbackTransaction();
    
    // 事务状态
    TransactionState GetState() const { return state_; }
    uint64_t GetTransactionId() const { return transaction_id_; }
    IsolationLevel GetIsolationLevel() const { return isolation_level_; }
    
    // 锁管理
    Status AcquireLock(const LockKey& key, LockType lock_type, Duration timeout);
    void ReleaseLock(const LockKey& key);
    bool HasLock(const LockKey& key, LockType lock_type) const;
    
    // 变更集管理
    void AddChangeSet(const ChangeSet& change_set);
    const std::vector<ChangeSet>& GetChangeSets() const { return change_sets_; }
    
private:
    uint64_t transaction_id_;
    TransactionState state_;
    IsolationLevel isolation_level_;
    std::chrono::steady_clock::time_point start_time_;
    std::map<LockKey, LockType> acquired_locks_;
    std::vector<ChangeSet> change_sets_;
};

enum class TransactionState {
    ACTIVE,        // 活跃
    PREPARING,     // 准备提交
    COMMITTED,     // 已提交
    ABORTED,       // 已中止
    TIMEOUT        // 超时
};
```

##### 2.1.2 锁管理器
```cpp
class LockManager {
public:
    // 锁操作
    Status AcquireLock(TransactionContext* txn, const LockKey& key, LockType lock_type, Duration timeout);
    Status ReleaseLock(TransactionContext* txn, const LockKey& key);
    Status ReleaseAllLocks(TransactionContext* txn);
    
    // 死锁检测
    bool DetectDeadlock();
    TransactionContext* ChooseVictim(const std::set<TransactionContext*>& deadlocked_txns);
    
    // 锁升级
    Status UpgradeLock(TransactionContext* txn, const LockKey& key, LockType new_lock_type);
    
private:
    struct LockInfo {
        TransactionContext* owner;
        LockType lock_type;
        std::chrono::steady_clock::time_point acquire_time;
    };
    
    std::map<LockKey, std::vector<LockInfo>> lock_table_;
    std::map<TransactionContext*, std::set<LockKey>> transaction_locks_;
};
```

#### 2.2 WAL管理器实现

##### 2.2.1 日志记录
```cpp
enum class LogRecordType {
    BEGIN_TRANSACTION,    // 开始事务
    COMMIT_TRANSACTION,   // 提交事务
    ABORT_TRANSACTION,    // 中止事务
    INSERT_RECORD,        // 插入记录
    UPDATE_RECORD,        // 更新记录
    DELETE_RECORD,        // 删除记录
    CHECKPOINT           // 检查点
};

struct LogRecord {
    uint64_t log_sequence_number;
    uint64_t transaction_id;
    LogRecordType type;
    std::vector<uint8_t> data;
    uint64_t previous_lsn;
    uint64_t checksum;
    std::chrono::system_clock::time_point timestamp;
};

class WALManager {
public:
    // 日志操作
    Status AppendLogRecord(const LogRecord& record);
    Status FlushLogs();
    Status TruncateLogs(uint64_t truncation_point);
    
    // 恢复操作
    Status Recover();
    Status RedoTransaction(uint64_t transaction_id);
    Status UndoTransaction(uint64_t transaction_id);
    
    // 检查点
    Status CreateCheckpoint();
    
private:
    std::unique_ptr<LogFile> log_file_;
    std::unique_ptr<LogBuffer> log_buffer_;
    uint64_t current_lsn_;
};
```

### 阶段三：索引系统完善（1周）

#### 3.1 B+树索引增强

##### 3.1.1 索引操作
```cpp
class BPlusTreeIndex {
public:
    // 索引操作
    Status Insert(const IndexKey& key, const RID& rid);
    Status Delete(const IndexKey& key, const RID& rid);
    std::vector<RID> Search(const SearchKey& key);
    std::vector<RID> RangeSearch(const RangeKey& range);
    
    // 索引维护
    Status RebuildIndex();
    Status CompactIndex();
    
    // 统计信息
    IndexStatistics GetStatistics() const;
    
private:
    struct BPlusTreeNode {
        std::vector<IndexKey> keys;
        std::vector<int32_t> child_page_ids;
        bool is_leaf;
        int32_t next_leaf_page;
    };
    
    std::unique_ptr<BufferPool> buffer_pool_;
    int32_t root_page_id_;
    IndexSchema schema_;
};
```

### 阶段四：约束系统实现（1周）

#### 4.1 约束管理器

```cpp
class ConstraintManager {
public:
    // 约束操作
    Status AddPrimaryKey(const std::string& table_name, const std::vector<std::string>& columns);
    Status AddForeignKey(const std::string& table_name, const std::string& column, 
                        const std::string& referenced_table, const std::string& referenced_column);
    Status AddUniqueConstraint(const std::string& table_name, const std::vector<std::string>& columns);
    Status AddCheckConstraint(const std::string& table_name, const std::string& column, 
                             const std::string& check_expression);
    
    // 约束验证
    Status ValidateInsert(const std::string& table_name, const Record& record);
    Status ValidateUpdate(const std::string& table_name, const Record& old_record, const Record& new_record);
    Status ValidateDelete(const std::string& table_name, const Record& record);
    
private:
    std::map<std::string, std::vector<std::unique_ptr<Constraint>>> table_constraints_;
};
```

### 阶段五：灾难恢复机制（1周）

#### 5.1 备份恢复系统

##### 5.1.1 备份管理器
```cpp
class BackupManager {
public:
    // 全量备份
    Status CreateFullBackup(const std::string& backup_path);
    Status RestoreFromFullBackup(const std::string& backup_path);
    
    // 增量备份
    Status CreateIncrementalBackup(const std::string& backup_path, uint64_t base_lsn);
    Status ApplyIncrementalBackup(const std::string& backup_path);
    
    // 在线备份
    Status StartOnlineBackup(const std::string& backup_path);
    Status CompleteOnlineBackup();
    
    // 恢复点
    Status CreateRestorePoint(const std::string& point_name);
    Status RestoreToPoint(const std::string& point_name);
    
private:
    std::unique_ptr<BackupStorage> backup_storage_;
    std::unique_ptr<BackupCatalog> backup_catalog_;
};
```

##### 5.1.2 恢复管理器
```cpp
class RecoveryManager {
public:
    // 恢复操作
    Status PerformRecovery();
    Status RecoverToTimestamp(const std::chrono::system_clock::time_point& timestamp);
    Status RecoverToLSN(uint64_t target_lsn);
    
    // 介质恢复
    Status RecoverFromBackup(const std::string& backup_id);
    Status ApplyRedoLogs(uint64_t from_lsn, uint64_t to_lsn);
    
    // 一致性检查
    Status CheckDatabaseConsistency();
    Status RepairInconsistencies();
    
private:
    std::unique_ptr<LogAnalyzer> log_analyzer_;
    std::unique_ptr<DataValidator> data_validator_;
};
```

## 技术实现细节

### 内存管理优化

#### 1. 查询执行缓存
```cpp
class QueryCache {
public:
    // 缓存管理
    void CacheQueryPlan(const std::string& query_hash, const ExecutionPlan& plan);
    std::unique_ptr<ExecutionPlan> GetCachedPlan(const std::string& query_hash);
    void InvalidateCache(const std::string& table_name);
    
private:
    std::unordered_map<std::string, std::unique_ptr<ExecutionPlan>> plan_cache_;
    LRUCache cache_eviction_policy_;
};
```

#### 2. 缓冲区管理增强
```cpp
class EnhancedBufferPool {
public:
    // 页面预取
    Status PrefetchPages(const std::vector<int32_t>& page_ids);
    
    // 智能淘汰
    void SetEvictionPolicy(EvictionPolicy policy);
    
    // 统计信息
    BufferPoolStatistics GetStatistics() const;
    
private:
    enum class EvictionPolicy {
        LRU,        // 最近最少使用
        LFU,        // 最少使用频率
        CLOCK,      // 时钟算法
        ADAPTIVE    // 自适应
    };
};
```

### 性能优化策略

#### 1. 查询优化
```cpp
class QueryOptimizer {
public:
    // 优化规则
    std::unique_ptr<ExecutionPlan> OptimizePlan(const LogicalPlan& plan);
    
    // 成本估算
    double EstimateCost(const ExecutionPlan& plan);
    
    // 统计信息收集
    Status UpdateStatistics(const std::string& table_name);
    
private:
    // 优化规则集合
    std::vector<std::unique_ptr<OptimizationRule>> optimization_rules_;
    std::unique_ptr<StatisticsManager> statistics_manager_;
};
```

#### 2. 并发控制优化
```cpp
class OptimisticConcurrencyControl {
public:
    // 版本控制
    Status ValidateRead(TransactionContext* txn, const ReadSet& read_set);
    Status ValidateWrite(TransactionContext* txn, const WriteSet& write_set);
    
    // 冲突解决
    Status ResolveConflict(TransactionContext* txn, const ConflictInfo& conflict);
    
private:
    std::unique_ptr<VersionManager> version_manager_;
    std::unique_ptr<ConflictDetector> conflict_detector_;
};
```

## 测试验证计划

### 单元测试
- [ ] SqlExecutor所有方法的功能测试
- [ ] 事务ACID属性的验证测试
- [ ] 索引操作的正确性测试
- [ ] 约束系统的完整性测试

### 集成测试
- [ ] 复杂查询的执行测试
- [ ] 并发事务的正确性测试
- [ ] 备份恢复的完整性测试
- [ ] 性能基准测试

### 压力测试
- [ ] 大数据量查询性能
- [ ] 高并发事务处理能力
- [ ] 长时间运行的稳定性
- [ ] 故障恢复时间测试

## 部署和迁移

### 数据迁移
```cpp
class DataMigrator {
public:
    // 数据导入导出
    Status ExportData(const std::string& table_name, const std::string& file_path);
    Status ImportData(const std::string& table_name, const std::string& file_path);
    
    // 格式转换
    Status ConvertDataFormat(const std::string& source_format, const std::string& target_format);
    
    // 验证迁移
    Status ValidateMigration(const std::string& source_table, const std::string& target_table);
};
```

### 配置更新
```yaml
# 增强版配置
database:
  name: "sqlcc_enhanced"
  version: "1.0.0"
  
storage:
  page_size: 8192
  buffer_pool_size: 1000
  data_directory: "/var/lib/sqlcc/data"
  
transaction:
  isolation_level: "READ_COMMITTED"
  lock_timeout_ms: 5000
  deadlock_detection_interval_ms: 1000
  
recovery:
  wal_enabled: true
  wal_buffer_size: 64MB
  checkpoint_interval_seconds: 300
  backup_enabled: true
  backup_retention_days: 30
  
performance:
  query_cache_size: 100
  index_cache_size: 500
  enable_query_optimization: true
  enable_statistics_collection: true
```

## 实施时间表

| 阶段 | 任务 | 预计时间 | 依赖关系 |
|------|------|----------|----------|
| 阶段一 | 执行层重构 | 2-3周 | 无 |
| 阶段二 | 事务处理 | 1-2周 | 阶段一 |
| 阶段三 | 索引系统 | 1周 | 阶段一 |
| 阶段四 | 约束系统 | 1周 | 阶段一、三 |
| 阶段五 | 灾难恢复 | 1周 | 阶段一、二 |
| 测试 | 功能测试 | 1周 | 所有阶段 |
| 文档 | 文档完善 | 持续 | 整个过程 |

**总预计时间：6-8周**

## 成功指标

### 功能完整性
- [ ] 所有标准SQL语句支持
- [ ] 完整的事务ACID保证
- [ ] 约束系统的完整性
- [ ] 灾难恢复的可靠性

### 性能指标
- [ ] 查询响应时间 < 5ms
- [ ] 事务吞吐量 > 1000 TPS
- [ ] 数据恢复时间 < 30秒
- [ ] 存储效率 > 80%

### 质量指标
- [ ] 测试覆盖率 > 90%
- [ ] 代码质量评级 > A
- [ ] 内存泄漏检测 0
- [ ] 死锁检测和处理 100%

## 风险评估

### 技术风险
- **复杂度风险**：SQL执行引擎实现复杂
- **性能风险**：可能引入性能退化
- **一致性风险**：事务处理可能导致一致性问题

### 缓解措施
- **分阶段实施**：降低复杂度风险
- **性能基准测试**：持续监控性能
- **充分测试**：确保事务正确性

## 总结

这个单机版功能增强计划将为sqlcc打下坚实的功能基础，实现从桩代码到真实数据库系统的转变。通过系统性的重构和增强，sqlcc将具备：

1. **完整的SQL执行能力**
2. **可靠的事务处理机制**
3. **强大的灾难恢复能力**
4. **良好的性能表现**

这为后续的分布式扩展奠定了必要的基础。
