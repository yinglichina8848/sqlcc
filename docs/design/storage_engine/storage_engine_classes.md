# Storage Engine 类设计文档

## 类列表

### StorageEngine

**定义位置**: `include/storage_engine/index_manager.h`

**定义**:
```cpp
class StorageEngine;
class BPlusTreeIndex;
class ConfigManager;
} // namespace sqlcc

namespace sqlcc {

class IndexManager {
public:
  IndexManager(std::shared_ptr<StorageEngine> storage_engine, Conf...
```

**构造函数**:
- `IndexManager`
- `IndexManager`
- `CreateIndex`
- `CreateCompositeIndex`
- `DropIndex`
- `GetIndex`
- `LoadAllIndexes`

**析构函数**:
- `IndexManager`

**公有方法**:
- `索引管理
  bool CreateIndex`
- `bool CreateCompositeIndex`
- `bool DropIndex`
- `内部方法
  void LoadAllIndexes`

---

### DeadlockDetector

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class DeadlockDetector {
public:
    DeadlockDetector();
    ~DeadlockDetector() = default;

    // 死锁检测
    bool DetectDeadlock(int32_t transaction_id, std::vector<int32_t>& deadlock_chain);
    void...
```

**构造函数**:
- `DeadlockDetector`
- `DetectDeadlock`
- `AddWaitFor`
- `RemoveWaitFor`
- `RemoveTransaction`
- `HasCycle`

**公有方法**:
- `死锁检测
    bool DetectDeadlock`
- `void AddWaitFor`
- `void RemoveWaitFor`
- `void RemoveTransaction`
- `bool HasCycle`

---

### LockManager

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class LockManager {
public:
    LockManager(std::shared_ptr<TransactionManager> transaction_manager);
    ~LockManager() = default;

    // 锁操作
    LockState AcquireLock(const LockRequest& request);
 ...
```

**构造函数**:
- `LockManager`
- `AcquireLock`
- `ReleaseLock`
- `ReleaseAllLocks`
- `UpgradeLock`
- `CleanupExpiredLocks`
- `NotifyWaitingTransactions`

**公有方法**:
- `锁操作
    LockState AcquireLock`
- `LockState ReleaseLock`
- `LockState ReleaseAllLocks`
- `锁升级
    LockState UpgradeLock`
- `锁清理
    void CleanupExpiredLocks`
- `void NotifyWaitingTransactions`

---

### MVCCVersionManager

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class MVCCVersionManager {
public:
    MVCCVersionManager(std::shared_ptr<StorageEngine> storage_engine);
    ~MVCCVersionManager() = default;

    // 版本管理
    uint64_t GetNextVersion();
    bool IsVe...
```

**构造函数**:
- `MVCCVersionManager`
- `GetNextVersion`
- `CleanupOldVersions`

**公有方法**:
- `版本管理
    uint64_t GetNextVersion`
- `版本清理
    void CleanupOldVersions`

---

### ConcurrentAccessValidator

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class ConcurrentAccessValidator {
public:
    ConcurrentAccessValidator(std::shared_ptr<StorageEngine> storage_engine,
                            std::shared_ptr<TransactionManager> transaction_manag...
```

**构造函数**:
- `ConcurrentAccessValidator`
- `ValidateConcurrentAccess`
- `BeginTransaction`
- `CommitTransaction`
- `RollbackTransaction`
- `AcquireRecordLock`
- `ReleaseRecordLock`
- `ValidateMVCCVersion`
- `SetConcurrencyConfig`
- `UpdateConcurrencyStats`
- `CleanupTransactionContext`

**公有方法**:
- `并发访问验证
    LockState ValidateConcurrentAccess`
- `事务管理
    void BeginTransaction`
- `void CommitTransaction`
- `void RollbackTransaction`
- `锁管理
    LockState AcquireRecordLock`
- `LockState ReleaseRecordLock`
- `MVCC版本验证
    bool ValidateMVCCVersion`
- `配置管理
    void SetConcurrencyConfig`
- `void UpdateConcurrencyStats`
- `void CleanupTransactionContext`

---

### ConcurrentAccessValidatorFactory

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class ConcurrentAccessValidatorFactory {
public:
    static std::shared_ptr<ConcurrentAccessValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_...
```

**构造函数**:
- `CreateBasicValidator`
- `CreateStrictValidator`
- `CreateEnterpriseValidator`

---

### ConcurrentAccessResultFormatter

**定义位置**: `src/storage_engine/concurrent_access_validator.h`

**定义**:
```cpp
class ConcurrentAccessResultFormatter {
public:
    static std::string FormatLockState(LockState state);
    static std::string FormatAccessControlType(AccessControlType type);
    static std::string ...
```

**构造函数**:
- `FormatLockState`
- `FormatAccessControlType`
- `FormatLockType`
- `FormatIsolationLevel`
- `IsCriticalError`

**公有方法**:
- `string FormatLockState`
- `string FormatAccessControlType`
- `string FormatLockType`
- `string FormatIsolationLevel`
- `static bool IsCriticalError`

---

### TransactionManager

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**定义**:
```cpp
class TransactionManager;

// 使用concurrency_control.h中的LockType作为锁模式

// 锁请求状态
enum LockRequestStatus {
    GRANTED = 0,          // 已授予
    WAITING = 1,          // 等待中
    TIMED_OUT = 2,        // 超...
```

---

### AdvancedDeadlockDetector

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**定义**:
```cpp
class AdvancedDeadlockDetector {
public:
    AdvancedDeadlockDetector();
    ~AdvancedDeadlockDetector() = default;

    // 检测死锁
    bool DetectDeadlock(const std::unordered_map<int32_t, PageLockState...
```

**构造函数**:
- `AdvancedDeadlockDetector`
- `DetectDeadlock`
- `AddWaitFor`
- `RemoveWaitFor`

**公有方法**:
- `检测死锁
    bool DetectDeadlock`
- `添加等待关系
    void AddWaitFor`
- `移除等待关系
    void RemoveWaitFor`

---

### LockUpgradeManager

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**定义**:
```cpp
class LockUpgradeManager {
public:
    LockUpgradeManager();
    ~LockUpgradeManager() = default;

    // 锁升级策略
    enum UpgradeStrategy {
        IMMEDIATE_UPGRADE = 0,    // 立即升级
        DEFERRED_UP...
```

**构造函数**:
- `LockUpgradeManager`
- `PerformUpgrade`
- `PerformDowngrade`

**公有方法**:
- `执行锁升级
    bool PerformUpgrade`
- `执行锁降级
    bool PerformDowngrade`

---

### AdvancedLockManager

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**定义**:
```cpp
class AdvancedLockManager {
public:
    AdvancedLockManager(size_t max_locks = 10000,
                       std::chrono::milliseconds default_timeout = std::chrono::milliseconds(5000));
    ~Advanced...
```

**构造函数**:
- `AdvancedLockManager`
- `AdvancedLockManager`
- `AcquireLock`
- `ReleaseLock`
- `AcquireLocks`
- `ReleaseAllLocks`
- `AcquireLockAsync`
- `UpgradeLock`
- `DowngradeLock`
- `DetectAndResolveDeadlock`
- `SetDeadlockResolutionStrategy`
- `CleanupExpiredLocks`
- `SetLockTimeout`
- `SetMaxLocks`
- `EnableDeadlockDetection`
- `SetDeadlockCheckInterval`
- `GetOrCreatePageLockState`
- `TryAcquireLock`
- `GrantWaitingRequests`
- `UpdateLockWaitTime`
- `ProcessAsyncRequests`
- `StartAsyncWorker`
- `StopAsyncWorker`

**析构函数**:
- `AdvancedLockManager`

**公有方法**:
- `同步锁操作
    LockRequestStatus AcquireLock`
- `LockRequestStatus ReleaseLock`
- `size_t ReleaseAllLocks`
- `异步锁操作
    bool AcquireLockAsync`
- `降级
    LockRequestStatus UpgradeLock`
- `LockRequestStatus DowngradeLock`
- `死锁检测和处理
    bool DetectAndResolveDeadlock`
- `void SetDeadlockResolutionStrategy`
- `锁超时管理
    void CleanupExpiredLocks`
- `void SetLockTimeout`
- `配置管理
    void SetMaxLocks`
- `void EnableDeadlockDetection`
- `void SetDeadlockCheckInterval`
- `LockRequestStatus TryAcquireLock`
- `void GrantWaitingRequests`
- `void UpdateLockWaitTime`
- `void ProcessAsyncRequests`
- `void StartAsyncWorker`
- `void StopAsyncWorker`

---

### LockManagerFactory

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**定义**:
```cpp
class LockManagerFactory {
public:
    static std::shared_ptr<AdvancedLockManager> CreateBasicLockManager(
        std::chrono::milliseconds default_timeout = std::chrono::milliseconds(5000));

    st...
```

**构造函数**:
- `CreateBasicLockManager`
- `CreateHighConcurrencyLockManager`
- `CreateStrictLockManager`

---

### CheckConstraintParser

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class CheckConstraintParser {
public:
    CheckConstraintParser();
    ~CheckConstraintParser() = default;

    // 表达式解析
    bool ParseExpression(const std::string& expression, std::function<bool(cons...
```

**构造函数**:
- `CheckConstraintParser`

---

### ForeignKeyValidator

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class ForeignKeyValidator {
public:
    ForeignKeyValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~ForeignKeyValidator() = default;

    // 外键验证
    ConstraintValidationResult ValidateFo...
```

**构造函数**:
- `ForeignKeyValidator`

---

### UniqueConstraintValidator

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class UniqueConstraintValidator {
public:
    UniqueConstraintValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~UniqueConstraintValidator() = default;

    // 唯一性验证
    ConstraintValidati...
```

**构造函数**:
- `UniqueConstraintValidator`
- `CreateUniqueIndex`
- `DropUniqueIndex`
- `UpdateUniqueValueCache`

**公有方法**:
- `唯一索引管理
    bool CreateUniqueIndex`
- `bool DropUniqueIndex`
- `唯一值缓存管理
    void UpdateUniqueValueCache`

---

### DefaultValueHandler

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class DefaultValueHandler {
public:
    DefaultValueHandler();
    ~DefaultValueHandler() = default;

    // 默认值应用
    std::string ApplyDefaultValue(const std::string& column_type, const std::string& ...
```

**构造函数**:
- `DefaultValueHandler`

---

### DataIntegrityValidator

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class DataIntegrityValidator {
public:
    DataIntegrityValidator(std::shared_ptr<StorageEngine> storage_engine,
                          std::shared_ptr<TransactionManager> transaction_manager = nul...
```

**构造函数**:
- `DataIntegrityValidator`
- `AddConstraint`
- `RemoveConstraint`
- `EnableConstraint`
- `DisableConstraint`
- `SetValidationConfig`

**公有方法**:
- `约束管理
    bool AddConstraint`
- `bool RemoveConstraint`
- `bool EnableConstraint`
- `bool DisableConstraint`
- `配置管理
    void SetValidationConfig`

---

### DataIntegrityValidatorFactory

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class DataIntegrityValidatorFactory {
public:
    static std::shared_ptr<DataIntegrityValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine);

    static std::shared_p...
```

**构造函数**:
- `CreateBasicValidator`
- `CreateStrictValidator`
- `CreateEnterpriseValidator`

---

### ConstraintValidationResultFormatter

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**定义**:
```cpp
class ConstraintValidationResultFormatter {
public:
    static std::string FormatResult(ConstraintValidationResult result);
    static std::string FormatDetailedResult(ConstraintValidationResult resul...
```

**构造函数**:
- `FormatResult`
- `FormatDetailedResult`
- `GetResultSeverity`
- `IsCriticalError`

**公有方法**:
- `string FormatResult`
- `string FormatDetailedResult`
- `string GetResultSeverity`
- `static bool IsCriticalError`

---

### TableMetadata

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class TableMetadata;
class StorageEngine;
class TransactionManager;

// 记录大小限制配置
struct RecordSizeLimits {
    static constexpr size_t MIN_RECORD_SIZE = 1;                    // 最小记录大小（字节）
    static ...
```

---

### RecordSizeValidator

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class RecordSizeValidator {
public:
    RecordSizeValidator();
    ~RecordSizeValidator() = default;

    // 记录大小验证
    ValidationResult ValidateRecordSize(size_t record_size, size_t max_size = Record...
```

**构造函数**:
- `RecordSizeValidator`
- `SetMaxRecordSize`
- `SetStrictMode`

**公有方法**:
- `配置管理
    void SetMaxRecordSize`
- `void SetStrictMode`

---

### FieldTypeBoundaryValidator

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class FieldTypeBoundaryValidator {
public:
    FieldTypeBoundaryValidator();
    ~FieldTypeBoundaryValidator() = default;

    // 数值类型验证
    ValidationResult ValidateInteger(const std::string& value, ...
```

**构造函数**:
- `FieldTypeBoundaryValidator`

---

### DataIntegrityConstraintValidator

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class DataIntegrityConstraintValidator {
public:
    DataIntegrityConstraintValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~DataIntegrityConstraintValidator() = default;

    // 空值约束验证
...
```

**构造函数**:
- `DataIntegrityConstraintValidator`
- `AddValidationRule`
- `RemoveValidationRule`
- `UpdateUniqueValueCache`

**公有方法**:
- `约束规则管理
    void AddValidationRule`
- `void RemoveValidationRule`
- `void UpdateUniqueValueCache`

---

### ConcurrentAccessControlValidator

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class ConcurrentAccessControlValidator {
public:
    ConcurrentAccessControlValidator(std::shared_ptr<TransactionManager> transaction_manager);
    ~ConcurrentAccessControlValidator() = default;

    ...
```

**构造函数**:
- `ConcurrentAccessControlValidator`

---

### RecordBoundaryValidator

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class RecordBoundaryValidator {
public:
    RecordBoundaryValidator(std::shared_ptr<StorageEngine> storage_engine,
                          std::shared_ptr<TransactionManager> transaction_manager = n...
```

**构造函数**:
- `RecordBoundaryValidator`
- `ValidateRecord`
- `ValidateRecordUpdate`
- `ValidateRecords`
- `SetValidationConfig`
- `AddFieldValidationRule`
- `RemoveFieldValidationRule`
- `ClearValidationRules`
- `UpdateValidationStats`

**公有方法**:
- `综合记录验证
    ValidationResult ValidateRecord`
- `记录更新验证
    ValidationResult ValidateRecordUpdate`
- `配置管理
    void SetValidationConfig`
- `验证规则管理
    void AddFieldValidationRule`
- `void RemoveFieldValidationRule`
- `void ClearValidationRules`
- `void UpdateValidationStats`

---

### RecordBoundaryValidatorFactory

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class RecordBoundaryValidatorFactory {
public:
    static std::shared_ptr<RecordBoundaryValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine);

    static std::shared...
```

**构造函数**:
- `CreateBasicValidator`
- `CreateStrictValidator`
- `CreateEnterpriseValidator`

---

### ValidationResultFormatter

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**定义**:
```cpp
class ValidationResultFormatter {
public:
    static std::string FormatResult(ValidationResult result);
    static std::string FormatDetailedResult(ValidationResult result, const std::string& field_na...
```

**构造函数**:
- `FormatResult`
- `FormatDetailedResult`
- `GetResultSeverity`
- `IsCriticalError`

**公有方法**:
- `string FormatResult`
- `string FormatDetailedResult`
- `string GetResultSeverity`
- `static bool IsCriticalError`

---

### Metric

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class Metric {
public:
  Metric(const std::string &name, const std::string &description,
         MetricType type, MetricUnit unit);
  virtual ~Metric() = default;

  const std::string &GetName() cons...
```

**构造函数**:
- `Metric`

---

### CounterMetric

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class CounterMetric : public Metric {
public:
  CounterMetric(const std::string &name, const std::string &description);

  void Increment() { value_.fetch_add(1); }
  void Increment(uint64_t delta) { ...
```

**构造函数**:
- `CounterMetric`
- `Increment`
- `Increment`
- `GetValue`
- `Reset`

**公有方法**:
- `void Increment`
- `void Increment`
- `uint64_t GetValue`
- `void Reset`

---

### GaugeMetric

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class GaugeMetric : public Metric {
public:
  GaugeMetric(const std::string &name, const std::string &description);

  void Set(uint64_t value) { value_.store(value); }
  uint64_t GetValue() const { r...
```

**构造函数**:
- `GaugeMetric`
- `Set`
- `GetValue`
- `Reset`

**公有方法**:
- `void Set`
- `uint64_t GetValue`
- `void Reset`

---

### HistogramMetric

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class HistogramMetric : public Metric {
public:
  HistogramMetric(const std::string &name, const std::string &description,
                  const std::vector<double> &bucket_bounds);

  void Observe(...
```

**构造函数**:
- `HistogramMetric`
- `Observe`
- `GetCount`
- `GetSum`

**公有方法**:
- `void Observe`
- `uint64_t GetCount`
- `double GetSum`

---

### TimerMetric

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class TimerMetric : public Metric {
public:
  TimerMetric(const std::string &name, const std::string &description);

  void Record(std::chrono::microseconds duration);
  std::chrono::microseconds GetA...
```

**构造函数**:
- `TimerMetric`
- `Record`
- `GetMin`
- `GetMax`
- `GetCount`

**公有方法**:
- `void Record`
- `uint64_t GetMin`
- `uint64_t GetMax`
- `uint64_t GetCount`

---

### PerformanceMonitor

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class PerformanceMonitor {
public:
  static PerformanceMonitor &GetInstance();

  // 注册指标
  void RegisterMetric(std::shared_ptr<Metric> metric);

  // 获取指标
  std::shared_ptr<Metric> GetMetric(const st...
```

**构造函数**:
- `GetInstance`
- `RegisterMetric`
- `ResetAllMetrics`
- `CreateCounter`
- `CreateGauge`
- `CreateHistogram`
- `CreateTimer`
- `ScopedTimer`
- `ScopedTimer`
- `CreateScopedTimer`

**析构函数**:
- `ScopedTimer`

**公有方法**:
- `注册指标
  void RegisterMetric`
- `重置所有指标
  void ResetAllMetrics`

---

### StorageEngineMetrics

**定义位置**: `src/storage_engine/performance_monitor.h`

**定义**:
```cpp
class StorageEngineMetrics {
public:
  static StorageEngineMetrics &GetInstance();

  // 初始化指标
  void Initialize();

  // BufferPool指标
  std::shared_ptr<CounterMetric> buffer_pool_hits;
  std::shared_...
```

**构造函数**:
- `GetInstance`
- `Initialize`

**公有方法**:
- `初始化指标
  void Initialize`

---

### DiskErrorHandler

**定义位置**: `src/storage_engine/disk_error_handler.h`

**定义**:
```cpp
class DiskErrorHandler {
public:
    DiskErrorHandler(std::shared_ptr<DiskManager> disk_manager);
    ~DiskErrorHandler() = default;

    // 错误处理和恢复
    RecoveryStrategy HandleDiskError(const DiskErro...
```

**构造函数**:
- `DiskErrorHandler`
- `HandleDiskError`
- `AttemptRecovery`
- `ValidateDataIntegrity`
- `ComputeAndStoreChecksum`
- `VerifyChecksum`
- `UpdateChecksum`
- `AssessDiskHealth`
- `RecordError`
- `SetMaxRetries`
- `SetRetryDelay`
- `SetHealthCheckInterval`
- `EnableChecksumValidation`
- `EnableAutoRecovery`
- `SetErrorCallback`
- `SetRecoveryCallback`
- `SetHealthChangeCallback`
- `milliseconds`
- `minutes`
- `ExecuteRecoveryAction`
- `UpdateHealthStatus`
- `LogError`
- `AlertOnCriticalError`

**公有方法**:
- `错误处理和恢复
    RecoveryStrategy HandleDiskError`
- `bool AttemptRecovery`
- `bool ValidateDataIntegrity`
- `校验和管理
    bool ComputeAndStoreChecksum`
- `bool VerifyChecksum`
- `bool UpdateChecksum`
- `磁盘健康监控
    DiskHealthStatus AssessDiskHealth`
- `错误统计和报告
    void RecordError`
- `自动恢复配置
    void SetMaxRetries`
- `void SetRetryDelay`
- `void SetHealthCheckInterval`
- `void EnableChecksumValidation`
- `void EnableAutoRecovery`
- `void SetErrorCallback`
- `void SetRecoveryCallback`
- `void SetHealthChangeCallback`
- `bool ExecuteRecoveryAction`
- `void UpdateHealthStatus`
- `void LogError`
- `void AlertOnCriticalError`

---

### DiskRedundancyManager

**定义位置**: `src/storage_engine/disk_error_handler.h`

**定义**:
```cpp
class DiskRedundancyManager {
public:
    DiskRedundancyManager(std::shared_ptr<DiskManager> primary_disk,
                         std::vector<std::shared_ptr<DiskManager>> backup_disks);
    ~DiskRe...
```

**构造函数**:
- `DiskRedundancyManager`
- `WriteWithRedundancy`
- `ReadWithRedundancy`
- `DetectAndRecoverFromFailure`
- `SetRedundancyLevel`
- `EnableAutomaticFailover`

**公有方法**:
- `冗余写入
    bool WriteWithRedundancy`
- `bool ReadWithRedundancy`
- `故障检测和恢复
    bool DetectAndRecoverFromFailure`
- `冗余配置
    void SetRedundancyLevel`
- `三副本
    void EnableAutomaticFailover`

---

### DiskSpaceManager

**定义位置**: `src/storage_engine/disk_error_handler.h`

**定义**:
```cpp
class DiskSpaceManager {
public:
    DiskSpaceManager(std::shared_ptr<DiskManager> disk_manager);
    ~DiskSpaceManager() = default;

    // 空间监控
    bool CheckAvailableSpace(size_t required_bytes) co...
```

**构造函数**:
- `DiskSpaceManager`
- `PerformSpaceCleanup`
- `ReclaimSpace`
- `PreallocateSpace`
- `ShrinkFile`
- `SetSpaceThresholds`

**公有方法**:
- `空间清理
    bool PerformSpaceCleanup`
- `size_t ReclaimSpace`
- `空间预分配
    bool PreallocateSpace`
- `bool ShrinkFile`
- `空间预警
    void SetSpaceThresholds`

---

### DiskIOMonitor

**定义位置**: `src/storage_engine/disk_error_handler.h`

**定义**:
```cpp
class DiskIOMonitor {
public:
    DiskIOMonitor(std::shared_ptr<DiskManager> disk_manager);
    ~DiskIOMonitor() = default;

    // 性能监控
    void RecordReadOperation(int32_t page_id, std::chrono::micr...
```

**构造函数**:
- `DiskIOMonitor`
- `RecordReadOperation`
- `RecordWriteOperation`
- `RecordSeekOperation`
- `SetMonitoringInterval`
- `SetPerformanceThresholds`
- `seconds`

**公有方法**:
- `性能监控
    void RecordReadOperation`
- `void RecordWriteOperation`
- `void RecordSeekOperation`
- `配置
    void SetMonitoringInterval`
- `void SetPerformanceThresholds`

---

### DiskErrorHandlerFactory

**定义位置**: `src/storage_engine/disk_error_handler.h`

**定义**:
```cpp
class DiskErrorHandlerFactory {
public:
    static std::shared_ptr<DiskErrorHandler> CreateBasicErrorHandler(
        std::shared_ptr<DiskManager> disk_manager);

    static std::shared_ptr<DiskErrorH...
```

**构造函数**:
- `CreateBasicErrorHandler`
- `CreateResilientErrorHandler`
- `CreateEnterpriseErrorHandler`

---

### BPlusTreeIndex

**定义位置**: `src/storage_engine/b_plus_tree/core/b_plus_tree.h`

**定义**:
```cpp
class BPlusTreeIndex {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎智能指针
     * @param table_name 表名
     * @param column_name 列名
     */
    BPlusTreeIndex(std::shared_ptr<Stor...
```

**构造函数**:
- `BPlusTreeIndex`
- `BPlusTreeIndex`
- `Create`
- `Drop`
- `Insert`
- `Delete`
- `LoadNode`
- `CreateNewNode`
- `DeleteNode`
- `NeedMerge`
- `LoadMetadata`
- `SaveMetadata`
- `Insert`
- `Delete`

**析构函数**:
- `BPlusTreeIndex`

**公有方法**:
- `bool Create`
- `bool Drop`
- `bool Insert`
- `bool Delete`
- `void DeleteNode`
- `bool NeedMerge`
- `void LoadMetadata`
- `void SaveMetadata`
- `递归操作方法
    bool Insert`
- `bool Delete`

---

### Prefetcher

**定义位置**: `src/storage_engine/concurrency_control.h`

**定义**:
```cpp
class Prefetcher {
public:
  /**
   * @brief 预取器统计信息 - 预取性能的量化指标
   *
   * WHY层 - 设计意图：
   *   预取统计信息帮助评估预取策略的有效性，为性能调优提供依据。
   *   通过详细的统计数据，系统可以自适应调整预取行为。
   *
   * WHAT层 - 统计指标：
   *   - 请求统计：预取请求和...
```

**构造函数**:
- `Prefetcher`
- `Prefetcher`
- `RecordPageAccess`
- `PrefetchPage`
- `PrefetchPages`
- `SetEnabled`
- `PrefetchWorker`

**析构函数**:
- `Prefetcher`

**公有方法**:
- `explicit Prefetcher`
- `void RecordPageAccess`
- `bool PrefetchPage`
- `size_t PrefetchPages`
- `void SetEnabled`
- `void PrefetchWorker`

---

### Partition

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class Partition {
public:
    enum class Type {
        RANGE,      // 范围分区
        HASH,       // 哈希分区
        LIST,       // 列表分区
        COMPOSITE   // 复合分区
    };

    enum class State {
        A...
```

**构造函数**:
- `Partition`

---

### RangePartition

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class RangePartition : public Partition {
public:
    struct Range {
        std::string min_value;
        std::string max_value;
        bool is_min_inclusive;
        bool is_max_inclusive;
    };
...
```

**构造函数**:
- `RangePartition`

---

### HashPartition

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class HashPartition : public Partition {
public:
    struct HashInfo {
        int32_t partition_count;
        std::hash<std::string> hasher;
    };

    HashPartition(int32_t partition_id,
         ...
```

**构造函数**:
- `HashPartition`

---

### ListPartition

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class ListPartition : public Partition {
public:
    struct ValueList {
        std::vector<std::string> values;
        bool is_default;
    };

    ListPartition(int32_t partition_id,
              ...
```

**构造函数**:
- `ListPartition`

---

### CompositePartition

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class CompositePartition : public Partition {
public:
    struct CompositeInfo {
        Type primary_type;    // 主分区类型
        Type secondary_type;  // 子分区类型
        int32_t sub_partition_count;
    ...
```

**构造函数**:
- `CompositePartition`

---

### PartitionManager

**定义位置**: `src/storage_engine/partition_manager.h`

**定义**:
```cpp
class PartitionManager {
public:
    PartitionManager(std::shared_ptr<StorageEngine> storage_engine,
                    std::shared_ptr<TableStorage> table_storage,
                    std::shared_pt...
```

**构造函数**:
- `PartitionManager`
- `createPartitionedTable`
- `dropPartitionedTable`
- `addPartition`
- `dropPartition`
- `updatePartitionStatistics`
- `truncatePartition`
- `addPartitionChangeListener`
- `notifyPartitionChange`

**公有方法**:
- `分区表管理
    bool createPartitionedTable`
- `bool dropPartitionedTable`
- `分区管理
    bool addPartition`
- `bool dropPartition`
- `分区统计和维护
    bool updatePartitionStatistics`
- `分区管理操作
    bool truncatePartition`
- `void addPartitionChangeListener`
- `void notifyPartitionChange`

---

### NodeSizeManager

**定义位置**: `include/storage_engine/node_size_manager.h`

**定义**:
```cpp
class NodeSizeManager {
public:
    // 常量定义
    static constexpr size_t MIN_CAPACITY = 4;      // 最小节点容量
    static constexpr size_t MAX_CAPACITY = 4096;   // 最大节点容量
    static constexpr size_t DEFAUL...
```

**构造函数**:
- `get_instance`
- `record_node_stats`
- `reset_stats`
- `update_stats`

**公有方法**:
- `void record_node_stats`
- `void reset_stats`
- `void update_stats`

---

### BufferPoolSharded

**定义位置**: `src/storage_engine/cache_consistency_manager.h`

**定义**:
```cpp
class BufferPoolSharded;
class Page;

// 页面版本信息
struct PageVersion {
    uint64_t version = 0;                    // 页面版本号
    std::chrono::steady_clock::time_point last_modified; // 最后修改时间
    std::c...
```

---

### CacheConsistencyManager

**定义位置**: `src/storage_engine/cache_consistency_manager.h`

**定义**:
```cpp
class CacheConsistencyManager {
public:
    CacheConsistencyManager(std::shared_ptr<BufferPoolSharded> buffer_pool,
                           CacheConsistencyStrategy strategy = STRICT_CONSISTENCY);
...
```

**构造函数**:
- `CacheConsistencyManager`
- `CheckReadConsistency`
- `CheckWriteConsistency`
- `AcquireReadLock`
- `AcquireWriteLock`
- `ReleaseReadLock`
- `ReleaseWriteLock`
- `UpdatePageVersion`
- `MarkPageDirty`
- `FlushDirtyPage`
- `InvalidatePage`
- `InvalidateAllPages`
- `PropagatePageUpdate`
- `PerformConsistencyCheck`
- `RepairConsistency`
- `CheckAllPagesConsistency`
- `SetConsistencyStrategy`
- `SetLockTimeout`
- `SetVersionCheckEnabled`
- `SetAutoRepairEnabled`
- `SetConsistencyViolationCallback`
- `SetPageInvalidationCallback`
- `SetVersionUpdateCallback`
- `GetOrCreatePageVersion`
- `GetOrCreatePageLock`
- `TryUpgradeLock`
- `UpdateLockWaitTime`
- `CheckConsistencyForStrategy`
- `RepairConsistencyForStrategy`
- `NotifyConsistencyViolation`
- `NotifyPageInvalidation`
- `NotifyVersionUpdate`

**公有方法**:
- `页面访问控制
    ConsistencyCheckResult CheckReadConsistency`
- `ConsistencyCheckResult CheckWriteConsistency`
- `bool AcquireReadLock`
- `bool AcquireWriteLock`
- `void ReleaseReadLock`
- `void ReleaseWriteLock`
- `bool UpdatePageVersion`
- `脏页管理
    bool MarkPageDirty`
- `bool FlushDirtyPage`
- `缓存失效和更新传播
    void InvalidatePage`
- `void InvalidateAllPages`
- `void PropagatePageUpdate`
- `一致性检查和修复
    ConsistencyCheckResult PerformConsistencyCheck`
- `bool RepairConsistency`
- `配置管理
    void SetConsistencyStrategy`
- `void SetLockTimeout`
- `void SetVersionCheckEnabled`
- `void SetAutoRepairEnabled`
- `void SetConsistencyViolationCallback`
- `void SetPageInvalidationCallback`
- `void SetVersionUpdateCallback`
- `bool TryUpgradeLock`
- `void UpdateLockWaitTime`
- `ConsistencyCheckResult CheckConsistencyForStrategy`
- `bool RepairConsistencyForStrategy`
- `void NotifyConsistencyViolation`
- `void NotifyPageInvalidation`
- `void NotifyVersionUpdate`

---

### AtomicVersionManager

**定义位置**: `src/storage_engine/cache_consistency_manager.h`

**定义**:
```cpp
class AtomicVersionManager {
public:
    AtomicVersionManager();
    ~AtomicVersionManager() = default;

    // 版本操作
    uint64_t GetVersion(int32_t page_id) const;
    uint64_t IncrementVersion(int32...
```

**构造函数**:
- `AtomicVersionManager`
- `IncrementVersion`
- `CompareAndSetVersion`
- `ResetVersion`
- `UpdateVersions`

**公有方法**:
- `uint64_t IncrementVersion`
- `bool CompareAndSetVersion`
- `void ResetVersion`
- `void UpdateVersions`

---

### MemoryBarrierManager

**定义位置**: `src/storage_engine/cache_consistency_manager.h`

**定义**:
```cpp
class MemoryBarrierManager {
public:
    MemoryBarrierManager();
    ~MemoryBarrierManager() = default;

    // 内存屏障操作
    void ReadBarrier();   // 读屏障 - 确保之前的读操作完成
    void WriteBarrier();  // 写屏障 - ...
```

**构造函数**:
- `MemoryBarrierManager`
- `ReadBarrier`
- `WriteBarrier`
- `FullBarrier`
- `PageReadBarrier`
- `PageWriteBarrier`
- `PageFullBarrier`

**公有方法**:
- `内存屏障操作
    void ReadBarrier`
- `确保之前的读操作完成
    void WriteBarrier`
- `确保之前的写操作对其他线程可见
    void FullBarrier`
- `页面特定的屏障
    void PageReadBarrier`
- `void PageWriteBarrier`
- `void PageFullBarrier`

---

### BPlusTreeNode

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_node.h`

**定义**:
```cpp
class BPlusTreeNode {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎指针，用于磁盘操作
     * @param page_id 节点对应的磁盘页面ID
     * @param is_leaf 是否为叶子节点
     */
    BPlusTreeNode(std::share...
```

**构造函数**:
- `BPlusTreeNode`
- `BPlusTreeNode`
- `GetData`

**析构函数**:
- `BPlusTreeNode`

---

### BPlusTreeInternalNode

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_nodes.h`

**定义**:
```cpp
class BPlusTreeInternalNode : public BPlusTreeNode {
public:
    BPlusTreeInternalNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id, bool is_new = false);
    virtual ~BPlusTreeInter...
```

**构造函数**:
- `BPlusTreeInternalNode`
- `BPlusTreeInternalNode`
- `InsertChild`
- `InsertChild`
- `RemoveChild`
- `Split`
- `Merge`

**析构函数**:
- `BPlusTreeInternalNode`

**公有方法**:
- `节点操作方法
    void InsertChild`
- `void InsertChild`
- `void RemoveChild`
- `void Split`
- `void Merge`

---

### BPlusTreeLeafNode

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_nodes.h`

**定义**:
```cpp
class BPlusTreeLeafNode : public BPlusTreeNode {
public:
    BPlusTreeLeafNode(std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);
    virtual ~BPlusTreeLeafNode();

    // 序列化和反序列化方法
   ...
```

**构造函数**:
- `BPlusTreeLeafNode`
- `BPlusTreeLeafNode`
- `Insert`
- `Remove`
- `Split`
- `Merge`

**析构函数**:
- `BPlusTreeLeafNode`

**公有方法**:
- `节点操作方法
    bool Insert`
- `bool Remove`
- `void Split`
- `void Merge`

---

### Page

**定义位置**: `src/storage_engine/page_allocator.h`

**定义**:
```cpp

class Page;

// 页面类型枚举
enum class PageType {
    DATA = 0,       // 数据页面
    INDEX = 1,      // 索引页面
    METADATA = 2,   // 元数据页面
    LOG = 3,        // 日志页面
    TEMP = 4        // 临时页面
};
```

---

### MemoryMonitor

**定义位置**: `src/storage_engine/page_allocator.h`

**定义**:
```cpp
    class MemoryMonitor {
    public:
        MemoryMonitor() = default;
        ~MemoryMonitor() = default;

        MemoryStats GetMemoryStats() const;
    };
```

---

### BufferFrame

**定义位置**: `src/storage_engine/buffer_pool/lru_manager.h`

**定义**:
```cpp

class BufferFrame;

class LRUManager {
public:
    explicit LRUManager(size_t capacity);
    ~LRUManager();

    // Access a buffer frame
    void access(BufferFrame* frame);

    // Remove a buffer ...
```

**构造函数**:
- `LRUManager`
- `LRUManager`
- `access`
- `remove`
- `get_victim`

**析构函数**:
- `LRUManager`

**公有方法**:
- `explicit LRUManager`
- `frame
    void access`
- `frame
    void remove`

---

### StatisticsCollector

**定义位置**: `src/storage_engine/buffer_pool/statistics_collector.h`

**定义**:
```cpp

class StatisticsCollector {
public:
    explicit StatisticsCollector(const std::string& name);
    ~StatisticsCollector();

    // Hit/miss statistics
    void record_hit();
    void record_miss();
 ...
```

**构造函数**:
- `StatisticsCollector`
- `StatisticsCollector`
- `record_hit`
- `record_miss`
- `record_eviction`
- `record_read`
- `record_write`
- `reset`

**析构函数**:
- `StatisticsCollector`

**公有方法**:
- `explicit StatisticsCollector`
- `statistics
    void record_hit`
- `void record_miss`
- `void record_eviction`
- `statistics
    void record_read`
- `void record_write`
- `statistics
    void reset`

---

### RecordValidator

**定义位置**: `include/storage_engine/table_storage/record_validator.h`

**定义**:
```cpp
class RecordValidator {
public:
    /**
     * @brief 验证记录大小
     *
     * @param record_size 记录大小（字节）
     * @param max_record_size 最大允许记录大小
     * @return 验证是否通过
     */
    static bool ValidateReco...
```

**构造函数**:
- `ValidateRecordSize`
- `ValidateFieldValue`
- `ValidateDataIntegrity`

**公有方法**:
- `static bool ValidateRecordSize`
- `static bool ValidateFieldValue`
- `static bool ValidateDataIntegrity`

---

### IndexManager

**定义位置**: `src/storage_engine/index_manager/smart_index_factory.h`

**定义**:
```cpp

class IndexManager;
class SmartIndexCache;

// Factory class for creating index managers with smart caching
class SmartIndexFactory {
public:
    explicit SmartIndexFactory(std::shared_ptr<SmartIndex...
```

**构造函数**:
- `SmartIndexFactory`
- `SmartIndexFactory`
- `create_index_manager`

**析构函数**:
- `SmartIndexFactory`

**公有方法**:
- `explicit SmartIndexFactory`

---

### IndexCacheEntry

**定义位置**: `src/storage_engine/index_manager/smart_index_cache.h`

**定义**:
```cpp

class IndexCacheEntry {
public:
    explicit IndexCacheEntry(const std::string& key);
    virtual ~IndexCacheEntry() = default;

    const std::string& get_key() const { return key_; }
    virtual si...
```

**构造函数**:
- `IndexCacheEntry`

**公有方法**:
- `explicit IndexCacheEntry`

---

### SmartIndexCache

**定义位置**: `src/storage_engine/index_manager/smart_index_cache.h`

**定义**:
```cpp

class SmartIndexCache {
public:
    explicit SmartIndexCache(size_t max_size);
    ~SmartIndexCache();

    // Cache operations
    bool put(const std::string& key, std::shared_ptr<IndexCacheEntry> e...
```

**构造函数**:
- `SmartIndexCache`
- `SmartIndexCache`
- `put`
- `get`
- `remove`
- `clear`

**析构函数**:
- `SmartIndexCache`

**公有方法**:
- `explicit SmartIndexCache`
- `operations
    bool put`
- `bool remove`
- `void clear`

---

### ClockReplaceStrategy

**定义位置**: `src/storage_engine/replace_strategy/clock_strategy.h`

**定义**:
```cpp
class ClockReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    ClockReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~ClockReplaceStrategy() ove...
```

**构造函数**:
- `ClockReplaceStrategy`

---

### ARCReplaceStrategy

**定义位置**: `src/storage_engine/replace_strategy/arc_strategy.h`

**定义**:
```cpp
class ARCReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     * @param p 初始T1和T2的大小参数（默认：总缓存大小的1/32）
     * @param total_size 总缓存大小
     */
    ARCReplaceStrategy...
```

**构造函数**:
- `ARCReplaceStrategy`
- `GetIterator`

**公有方法**:
- `iterator GetIterator`

---

### ReplaceStrategyFactory

**定义位置**: `src/storage_engine/replace_strategy/strategy_factory.h`

**定义**:
```cpp
class ReplaceStrategyFactory {
public:
    /**
     * @brief 策略类型枚举
     */
    enum class StrategyType {
        LRU,     // 最近最少使用
        LFU,     // 最不经常使用
        CLOCK,   // 时钟算法
        ARC,   ...
```

**构造函数**:
- `CreateStrategy`
- `GetStrategyName`
- `GetStrategyType`

**公有方法**:
- `string GetStrategyName`
- `static StrategyType GetStrategyType`

---

### LFUReplaceStrategy

**定义位置**: `src/storage_engine/replace_strategy/lfu_strategy.h`

**定义**:
```cpp
class LFUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LFUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LFUReplaceStrategy() override ...
```

**构造函数**:
- `LFUReplaceStrategy`

---

### LRUReplaceStrategy

**定义位置**: `src/storage_engine/replace_strategy/lru_strategy.h`

**定义**:
```cpp
class LRUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LRUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LRUReplaceStrategy() override ...
```

**构造函数**:
- `LRUReplaceStrategy`
- `UpdateLRU`

**公有方法**:
- `void UpdateLRU`

---

### PageRAII

**定义位置**: `include/storage_engine/table_storage/page_raii.h`

**定义**:
```cpp

class PageRAII {
public:
    // Constructor - takes ownership of the page through RAII pattern
    PageRAII(Page* page, std::shared_ptr<StorageEngine> storage_engine, int32_t page_id);

    // Destru...
```

**构造函数**:
- `PageRAII`
- `PageRAII`
- `GetData`
- `Unpin`

**析构函数**:
- `PageRAII`

**公有方法**:
- `RAII pattern
    PageRAII`
- `method
    void Unpin`

---

