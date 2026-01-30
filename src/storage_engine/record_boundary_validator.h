/**
 * WHY: 为什么数据库系统需要记录边界验证器？
 *
 * 数据库系统处理的数据来自各种来源，用户输入、网络传输、文件导入等，这些数据可能存在各种问题：
 * 1. 数据完整性问题：数据损坏、截断、不完整
 * 2. 类型不匹配问题：字符串当作数字、日期格式错误
 * 3. 边界值问题：数值超出范围、字符串过长
 * 4. 约束违反问题：唯一性、外键、检查约束违反
 * 5. 并发访问问题：脏读、不可重复读、幻读
 * 6. 安全威胁问题：SQL注入、恶意数据注入
 *
 * 记录边界验证器的价值体现在：
 * - 数据质量保障：确保进入数据库的数据符合规范
 * - 系统稳定性：防止异常数据导致的系统崩溃
 * - 业务逻辑保护：维护业务规则和数据约束
 * - 安全防护：阻挡恶意数据和攻击向量
 * - 错误诊断：提供详细的验证错误信息
 * - 性能优化：及早发现问题避免后续处理开销
 *
 * WHAT: RecordBoundaryValidator - 记录边界验证器
 *
 * 提供企业级数据库系统的完整记录数据验证功能，包括大小验证、类型验证、约束验证等：
 * - 记录大小验证：确保记录大小在合理范围内
 * - 字段类型验证：验证字段值的类型正确性
 * - 数据完整性约束：检查NOT NULL、UNIQUE、FOREIGN KEY等约束
 * - 并发访问控制：验证事务隔离性和并发安全性
 * - 格式验证：验证特殊格式如UUID、邮箱、JSON等
 * - 编码验证：确保字符编码的正确性
 *
 * 核心特性：
 * - 多层次验证：从基本大小验证到复杂约束验证
 * - 严格模式支持：可配置的严格验证级别
 * - 批量验证优化：高效的批量记录验证
 * - 详细错误报告：提供具体的验证失败原因
 * - 性能监控统计：验证性能和错误统计
 * - 扩展性设计：支持自定义验证规则
 *
 * HOW: 记录边界验证器的架构和技术实现
 *
 * 1. 分层验证架构：
 *    - 大小验证层：最基本的记录和字段大小检查
 *    - 类型验证层：字段值的数据类型和格式验证
 *    - 约束验证层：数据库完整性约束的检查
 *    - 并发验证层：事务隔离和并发访问的验证
 *    - 安全验证层：防止恶意数据和安全威胁
 *
 * 2. 验证流程设计：
 *    - 预验证阶段：快速的基本检查
 *    - 深度验证阶段：详细的约束和类型检查
 *    - 并发验证阶段：事务和锁的验证
 *    - 后验证阶段：最终的数据完整性确认
 *
 * 3. 验证策略框架：
 *    - 严格策略：任何验证失败都阻止操作
 *    - 宽松策略：记录问题但允许操作继续
 *    - 警告策略：发出警告但不阻止操作
 *    - 自定义策略：用户定义的验证行为
 *
 * 4. 性能优化机制：
 *    - 验证缓存：缓存常用验证结果
 *    - 并行验证：多线程并发验证
 *    - 增量验证：只验证变更的字段
 *    - 智能跳过：根据历史数据跳过某些验证
 *
 * 5. 错误处理和报告：
 *    - 分层错误码：不同验证层级的错误分类
 *    - 详细错误信息：包含字段名、期望值、实际值等
 *    - 错误上下文：提供验证失败的完整上下文
 *    - 错误统计：按类型和频率的错误统计
 *
 * 6. 扩展和定制：
 *    - 自定义验证器：用户自定义验证逻辑
 *    - 插件架构：支持第三方验证插件
 *    - 配置驱动：XML/JSON配置的验证规则
 *    - 热更新：运行时更新验证规则
 *
 * 🏗️ 设计模式：责任链模式 + 策略模式 + 装饰器模式
 *
 * 责任链模式应用：
 * - 验证链：大小验证→类型验证→约束验证→并发验证
 * - 错误处理链：不同验证器的错误处理责任传递
 * - 后处理链：验证后的数据转换和清理
 *
 * 策略模式应用：
 * - 验证策略：不同场景的验证策略选择
 * - 错误处理策略：不同的错误处理行为
 * - 性能优化策略：不同的性能优化方案
 *
 * 装饰器模式应用：
 * - 验证装饰器：在基本验证基础上添加额外检查
 * - 缓存装饰器：为验证结果添加缓存功能
 * - 监控装饰器：为验证过程添加性能监控
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - RecordSizeValidator只负责大小验证
 *    - FieldTypeBoundaryValidator专注类型验证
 *    - DataIntegrityConstraintValidator处理约束验证
 *    - ConcurrentAccessControlValidator负责并发控制
 *    - 职责分离清晰，功能单一专注
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的验证类型扩展
 *    - 可以通过继承添加新的验证规则
 *    - 验证策略可以独立扩展和定制
 *    - 对扩展开放，对修改关闭
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何验证器实现都可以替代接口使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的验证器接口集合
 *    - 避免客户端依赖不需要的验证功能
 *    - 按需暴露验证器的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 验证器依赖抽象的存储和事务接口
 *    - 不依赖具体的实现细节
 *    - 通过依赖注入提高系统的可测试性
 *
 * 记录边界验证器的性能优化：
 * - 快速预检：最常失败的基本验证优先执行
 * - 验证缓存：缓存成功验证的结果避免重复验证
 * - 批量处理：支持批量记录的并行验证
 * - 增量验证：只验证变更的数据字段
 * - 智能调度：根据数据特征选择验证顺序
 * - 资源控制：验证超时和资源使用限制
 */

#ifndef SQLCC_RECORD_BOUNDARY_VALIDATOR_H
#define SQLCC_RECORD_BOUNDARY_VALIDATOR_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <limits>
#include <regex>
#include <mutex>
#include <functional>

namespace sqlcc {

// 前向声明
struct TableMetadata;
class StorageEngine;
class TransactionManager;

// 记录大小限制配置
struct RecordSizeLimits {
    static constexpr size_t MIN_RECORD_SIZE = 1;                    // 最小记录大小（字节）
    static constexpr size_t MAX_RECORD_SIZE = 64 * 1024 * 1024;     // 最大记录大小（64MB）
    static constexpr size_t DEFAULT_MAX_RECORD_SIZE = 65536;        // 默认最大记录大小（64KB）
    static constexpr size_t MAX_VARCHAR_LENGTH = 65535;             // VARCHAR最大长度
    static constexpr size_t MAX_TEXT_LENGTH = 16 * 1024 * 1024;      // TEXT最大长度（16MB）
    static constexpr size_t MAX_BLOB_LENGTH = 64 * 1024 * 1024;     // BLOB最大长度（64MB）
};

// 字段类型验证配置
struct FieldTypeValidation {
    // 数值类型范围
    static constexpr int32_t INT32_MIN_VAL = std::numeric_limits<int32_t>::min();
    static constexpr int32_t INT32_MAX_VAL = std::numeric_limits<int32_t>::max();
    static constexpr int64_t INT64_MIN_VAL = std::numeric_limits<int64_t>::min();
    static constexpr int64_t INT64_MAX_VAL = std::numeric_limits<int64_t>::max();
    static constexpr float FLOAT_MIN_VAL = std::numeric_limits<float>::lowest();
    static constexpr float FLOAT_MAX_VAL = std::numeric_limits<float>::max();
    static constexpr double DOUBLE_MIN_VAL = std::numeric_limits<double>::lowest();
    static constexpr double DOUBLE_MAX_VAL = std::numeric_limits<double>::max();

    // 字符串验证模式
    static const std::regex UUID_PATTERN;
    static const std::regex EMAIL_PATTERN;
    static const std::regex URL_PATTERN;
    static const std::regex PHONE_PATTERN;
    static const std::regex DATE_PATTERN;
    static const std::regex TIME_PATTERN;
    static const std::regex DATETIME_PATTERN;
};

// 验证结果枚举
enum ValidationResult {
    VALID = 0,                    // 验证通过
    INVALID_SIZE = 1,            // 大小无效
    INVALID_TYPE = 2,            // 类型无效
    INVALID_VALUE = 3,           // 值无效
    NULL_VIOLATION = 4,          // 空值违反
    UNIQUE_VIOLATION = 5,        // 唯一性违反
    FOREIGN_KEY_VIOLATION = 6,   // 外键违反
    CHECK_CONSTRAINT_VIOLATION = 7, // 检查约束违反
    INVALID_FORMAT = 8,          // 格式无效
    ENCODING_ERROR = 9,          // 编码错误
    BOUNDARY_VIOLATION = 10      // 边界违反
};

// 字段验证规则
struct FieldValidationRule {
    std::string field_name;
    std::string field_type;
    bool nullable = true;
    bool unique = false;
    std::string check_constraint;      // 检查约束表达式
    std::string foreign_key_table;     // 外键表名
    std::string foreign_key_column;    // 外键列名
    size_t max_length = 0;            // 最大长度（对于字符串类型）
    std::string default_value;         // 默认值
    std::function<bool(const std::string&)> custom_validator; // 自定义验证函数

    FieldValidationRule(const std::string& name, const std::string& type)
        : field_name(name), field_type(type) {}
};

// 记录验证配置
struct RecordValidationConfig {
    size_t max_record_size = RecordSizeLimits::DEFAULT_MAX_RECORD_SIZE;
    bool strict_mode = true;           // 严格模式 - 任何验证失败都阻止操作
    bool enable_constraint_checking = true; // 启用约束检查
    bool enable_foreign_key_checking = true; // 启用外键检查
    bool enable_format_validation = true;   // 启用格式验证
    bool enable_encoding_validation = true; // 启用编码验证
    std::chrono::milliseconds validation_timeout = std::chrono::milliseconds(100); // 验证超时
};

// 记录大小验证器
class RecordSizeValidator {
public:
    RecordSizeValidator();
    ~RecordSizeValidator() = default;

    // 记录大小验证
    ValidationResult ValidateRecordSize(size_t record_size, size_t max_size = RecordSizeLimits::DEFAULT_MAX_RECORD_SIZE) const;
    ValidationResult ValidateFieldSize(const std::string& field_type, size_t field_size) const;
    ValidationResult ValidateTotalRecordSize(const std::vector<std::pair<std::string, size_t>>& field_sizes,
                                           size_t header_size = 0) const;

    // 大小计算
    size_t CalculateFieldSize(const std::string& field_type, const std::string& value) const;
    size_t CalculateRecordSize(const std::vector<std::pair<std::string, std::string>>& fields,
                              const std::shared_ptr<TableMetadata>& metadata) const;

    // 配置管理
    void SetMaxRecordSize(size_t max_size);
    void SetStrictMode(bool strict);
    size_t GetMaxRecordSize() const;

private:
    size_t max_record_size_;
    bool strict_mode_;
    mutable std::mutex validator_mutex_;
};

// 字段类型边界验证器
class FieldTypeBoundaryValidator {
public:
    FieldTypeBoundaryValidator();
    ~FieldTypeBoundaryValidator() = default;

    // 数值类型验证
    ValidationResult ValidateInteger(const std::string& value, bool is_bigint = false) const;
    ValidationResult ValidateFloat(const std::string& value, bool is_double = false) const;
    ValidationResult ValidateNumeric(const std::string& value, int precision = -1, int scale = -1) const;

    // 字符串类型验证
    ValidationResult ValidateString(const std::string& value, size_t max_length = 0) const;
    ValidationResult ValidateVarchar(const std::string& value, size_t max_length = RecordSizeLimits::MAX_VARCHAR_LENGTH) const;
    ValidationResult ValidateText(const std::string& value, size_t max_length = RecordSizeLimits::MAX_TEXT_LENGTH) const;

    // 日期时间类型验证
    ValidationResult ValidateDate(const std::string& value) const;
    ValidationResult ValidateTime(const std::string& value) const;
    ValidationResult ValidateDateTime(const std::string& value) const;
    ValidationResult ValidateTimestamp(const std::string& value) const;

    // 特殊类型验证
    ValidationResult ValidateBoolean(const std::string& value) const;
    ValidationResult ValidateUUID(const std::string& value) const;
    ValidationResult ValidateEmail(const std::string& value) const;
    ValidationResult ValidateURL(const std::string& value) const;
    ValidationResult ValidateJSON(const std::string& value) const;

    // 通用字段验证
    ValidationResult ValidateFieldValue(const std::string& field_name, const std::string& field_type,
                                      const std::string& value, const FieldValidationRule* rule = nullptr) const;

    // 批量验证
    std::vector<ValidationResult> ValidateFields(const std::vector<std::string>& field_names,
                                                const std::vector<std::string>& field_types,
                                                const std::vector<std::string>& values,
                                                const std::vector<FieldValidationRule>& rules = {}) const;

private:
    // 内部验证辅助方法
    bool IsValidIntegerFormat(const std::string& value) const;
    bool IsValidFloatFormat(const std::string& value) const;
    bool IsValidDateFormat(const std::string& value) const;
    bool IsValidTimeFormat(const std::string& value) const;
    bool IsValidUUIDFormat(const std::string& value) const;
    bool IsValidEmailFormat(const std::string& value) const;
    bool IsValidURLFormat(const std::string& value) const;
    bool IsValidJSONFormat(const std::string& value) const;
    bool ContainsInvalidCharacters(const std::string& value) const;
    std::string TrimWhitespace(const std::string& value) const;

    mutable std::mutex validator_mutex_;
};

// 数据完整性约束验证器
class DataIntegrityConstraintValidator {
public:
    DataIntegrityConstraintValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~DataIntegrityConstraintValidator() = default;

    // 空值约束验证
    ValidationResult ValidateNotNull(const std::string& field_name, const std::string& value,
                                   bool nullable = false) const;

    // 唯一性约束验证
    ValidationResult ValidateUnique(const std::string& table_name, const std::string& field_name,
                                  const std::string& value, int32_t exclude_record_id = -1) const;

    // 检查约束验证
    ValidationResult ValidateCheckConstraint(const std::string& field_name, const std::string& value,
                                           const std::string& constraint_expr) const;

    // 外键约束验证
    ValidationResult ValidateForeignKey(const std::string& field_name, const std::string& value,
                                      const std::string& foreign_table, const std::string& foreign_column) const;

    // 默认值处理
    std::string ApplyDefaultValue(const std::string& field_name, const std::string& value,
                                const std::string& default_value) const;

    // 约束规则管理
    void AddValidationRule(const std::string& table_name, const FieldValidationRule& rule);
    void RemoveValidationRule(const std::string& table_name, const std::string& field_name);
    const std::vector<FieldValidationRule>* GetValidationRules(const std::string& table_name) const;

    // 批量约束验证
    ValidationResult ValidateRecordConstraints(const std::string& table_name,
                                             const std::vector<std::string>& field_names,
                                             const std::vector<std::string>& values,
                                             int32_t record_id = -1) const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;

    // 约束规则存储：table_name -> rules
    std::unordered_map<std::string, std::vector<FieldValidationRule>> constraint_rules_;

    // 唯一性索引缓存：table_name.field_name -> unique_values_set
    std::unordered_map<std::string, std::unordered_set<std::string>> unique_value_cache_;

    // 检查约束表达式缓存
    std::unordered_map<std::string, std::function<bool(const std::string&)>> check_constraint_cache_;

    mutable std::mutex validator_mutex_;

    // 辅助方法
    bool EvaluateCheckConstraint(const std::string& constraint_expr, const std::string& value) const;
    bool ForeignKeyExists(const std::string& foreign_table, const std::string& foreign_column,
                         const std::string& value) const;
    void UpdateUniqueValueCache(const std::string& table_name, const std::string& field_name,
                               const std::string& old_value, const std::string& new_value);
};

// 并发访问控制验证器
class ConcurrentAccessControlValidator {
public:
    ConcurrentAccessControlValidator(std::shared_ptr<TransactionManager> transaction_manager);
    ~ConcurrentAccessControlValidator() = default;

    // 事务隔离级别验证
    ValidationResult ValidateIsolationLevel(int32_t transaction_id) const;

    // 并发冲突检测
    ValidationResult ValidateConcurrentAccess(const std::string& table_name, int32_t record_id,
                                            int32_t transaction_id, bool is_write = false) const;

    // 锁等待验证
    ValidationResult ValidateLockWait(int32_t transaction_id, std::chrono::milliseconds max_wait_time) const;

    // 死锁检测集成
    ValidationResult ValidateDeadlockFreedom(int32_t transaction_id) const;

    // MVCC版本验证
    ValidationResult ValidateMVCCVersion(const std::string& table_name, int32_t record_id,
                                       int32_t transaction_id, uint64_t expected_version) const;

    // 乐观并发控制
    ValidationResult ValidateOptimisticLock(const std::string& table_name, int32_t record_id,
                                          const std::string& version_field, const std::string& expected_version) const;

private:
    std::shared_ptr<TransactionManager> transaction_manager_;
    mutable std::mutex validator_mutex_;
};

// 记录边界验证器主类
class RecordBoundaryValidator {
public:
    RecordBoundaryValidator(std::shared_ptr<StorageEngine> storage_engine,
                          std::shared_ptr<TransactionManager> transaction_manager = nullptr);
    ~RecordBoundaryValidator() = default;

    // 综合记录验证
    ValidationResult ValidateRecord(const std::string& table_name,
                                  const std::vector<std::string>& field_names,
                                  const std::vector<std::string>& field_types,
                                  const std::vector<std::string>& values,
                                  int32_t transaction_id = -1,
                                  int32_t record_id = -1);

    // 记录更新验证
    ValidationResult ValidateRecordUpdate(const std::string& table_name,
                                        const std::vector<std::string>& field_names,
                                        const std::vector<std::string>& old_values,
                                        const std::vector<std::string>& new_values,
                                        int32_t transaction_id = -1,
                                        int32_t record_id = -1);

    // 记录删除验证
    ValidationResult ValidateRecordDeletion(const std::string& table_name,
                                          int32_t record_id,
                                          int32_t transaction_id = -1) const;

    // 批量验证
    std::vector<ValidationResult> ValidateRecords(const std::string& table_name,
                                                const std::vector<std::vector<std::string>>& record_batch,
                                                int32_t transaction_id = -1);

    // 配置管理
    void SetValidationConfig(const RecordValidationConfig& config);
    const RecordValidationConfig& GetValidationConfig() const;

    // 统计信息
    struct ValidationStats {
        size_t total_validations = 0;
        size_t successful_validations = 0;
        size_t failed_validations = 0;
        size_t size_validation_failures = 0;
        size_t type_validation_failures = 0;
        size_t constraint_violations = 0;
        size_t concurrency_conflicts = 0;
        double average_validation_time_us = 0.0;
        std::chrono::steady_clock::time_point last_validation_time;
    };

    ValidationStats GetValidationStats() const;

    // 验证规则管理
    void AddFieldValidationRule(const std::string& table_name, const FieldValidationRule& rule);
    void RemoveFieldValidationRule(const std::string& table_name, const std::string& field_name);
    void ClearValidationRules(const std::string& table_name);

private:
    // 组件验证器
    RecordSizeValidator size_validator_;
    FieldTypeBoundaryValidator type_validator_;
    DataIntegrityConstraintValidator constraint_validator_;
    ConcurrentAccessControlValidator access_validator_;
    
    // 存储引擎和事务管理器
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;

    // 配置
    RecordValidationConfig config_;

    // 统计信息
    mutable std::mutex stats_mutex_;
    ValidationStats stats_;
    std::vector<double> validation_times_; // 验证时间记录，用于计算平均值

    // 私有辅助方法
    ValidationResult PerformSizeValidation(const std::vector<std::string>& values,
                                         const std::shared_ptr<TableMetadata>& metadata) const;
    ValidationResult PerformTypeValidation(const std::vector<std::string>& field_names,
                                         const std::vector<std::string>& field_types,
                                         const std::vector<std::string>& values,
                                         const std::vector<FieldValidationRule>& rules) const;
    ValidationResult PerformConstraintValidation(const std::string& table_name,
                                               const std::vector<std::string>& field_names,
                                               const std::vector<std::string>& values,
                                               int32_t record_id) const;
    ValidationResult PerformConcurrencyValidation(const std::string& table_name,
                                                int32_t record_id, int32_t transaction_id,
                                                bool is_write) const;

    void UpdateValidationStats(ValidationResult result, std::chrono::microseconds duration);  // 移除const
    std::shared_ptr<TableMetadata> GetTableMetadata(const std::string& table_name) const;
};

// 验证器工厂
class RecordBoundaryValidatorFactory {
public:
    static std::shared_ptr<RecordBoundaryValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine);

    static std::shared_ptr<RecordBoundaryValidator> CreateStrictValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager);

    static std::shared_ptr<RecordBoundaryValidator> CreateEnterpriseValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager,
        const RecordValidationConfig& config);
};

// 验证结果格式化器
class ValidationResultFormatter {
public:
    static std::string FormatResult(ValidationResult result);
    static std::string FormatDetailedResult(ValidationResult result, const std::string& field_name = "",
                                          const std::string& details = "");
    static std::string GetResultSeverity(ValidationResult result);
    static bool IsCriticalError(ValidationResult result);
};

} // namespace sqlcc

#endif // SQLCC_RECORD_BOUNDARY_VALIDATOR_H
