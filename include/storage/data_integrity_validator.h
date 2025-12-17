/**
 * @file data_integrity_validator.h
 * @brief 数据完整性约束验证器头文件 - 实现完整的数据完整性约束验证
 */

#ifndef SQLCC_DATA_INTEGRITY_VALIDATOR_H
#define SQLCC_DATA_INTEGRITY_VALIDATOR_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <regex>
#include <mutex>
#include <atomic>

namespace sqlcc {

// 前向声明
class StorageEngine;
class TransactionManager;
class TableMetadata;

// 数据完整性约束类型
enum IntegrityConstraintType {
    NOT_NULL_CONSTRAINT = 1,      // 非空约束
    UNIQUE_CONSTRAINT = 2,        // 唯一约束
    PRIMARY_KEY_CONSTRAINT = 3,   // 主键约束
    FOREIGN_KEY_CONSTRAINT = 4,   // 外键约束
    CHECK_CONSTRAINT = 5,         // 检查约束
    DEFAULT_CONSTRAINT = 6        // 默认值约束
};

// 约束验证结果
enum ConstraintValidationResult {
    CONSTRAINT_VALID = 0,         // 约束验证通过
    CONSTRAINT_VIOLATED = 1,      // 约束违反
    CONSTRAINT_NOT_FOUND = 2,     // 约束不存在
    CONSTRAINT_INVALID = 3,       // 约束无效
    REFERENCE_ERROR = 4          // 引用错误
};

// 约束规则定义
struct ConstraintRule {
    std::string constraint_name;
    IntegrityConstraintType constraint_type;
    std::string table_name;
    std::vector<std::string> column_names;
    std::string expression;       // 检查约束表达式
    std::string referenced_table; // 外键引用表
    std::vector<std::string> referenced_columns; // 外键引用列
    std::string default_value;    // 默认值
    bool enabled = true;          // 约束是否启用
    std::function<bool(const std::vector<std::string>&)> custom_validator; // 自定义验证函数

    ConstraintRule(const std::string& name, IntegrityConstraintType type, const std::string& table)
        : constraint_name(name), constraint_type(type), table_name(table) {}
};

// 约束验证配置
struct IntegrityValidationConfig {
    bool enable_not_null_checking = true;      // 启用非空检查
    bool enable_unique_checking = true;        // 启用唯一检查
    bool enable_foreign_key_checking = true;   // 启用外键检查
    bool enable_check_constraint_checking = true; // 启用检查约束
    bool enable_default_value_setting = true;  // 启用默认值设置
    bool strict_mode = true;                   // 严格模式 - 任何约束违反都阻止操作
    std::chrono::milliseconds validation_timeout = std::chrono::milliseconds(500); // 验证超时
    size_t max_constraint_cache_size = 1000;   // 最大约束缓存大小
};

// 检查约束表达式解析器
class CheckConstraintParser {
public:
    CheckConstraintParser();
    ~CheckConstraintParser() = default;

    // 表达式解析
    bool ParseExpression(const std::string& expression, std::function<bool(const std::vector<std::string>&)>& validator) const;
    bool ValidateExpressionSyntax(const std::string& expression) const;

    // 表达式执行
    bool EvaluateExpression(const std::string& expression, const std::vector<std::string>& values,
                           const std::vector<std::string>& column_names) const;

private:
    // 表达式解析辅助方法
    std::function<bool(const std::vector<std::string>&)> ParseComparisonExpression(const std::string& expr,
                                                                               const std::vector<std::string>& column_names) const;
    std::function<bool(const std::vector<std::string>&)> ParseLogicalExpression(const std::string& expr,
                                                                            const std::vector<std::string>& column_names) const;
    std::function<bool(const std::vector<std::string>&)> ParseFunctionExpression(const std::string& expr,
                                                                             const std::vector<std::string>& column_names) const;

    // 值比较方法
    bool CompareValues(const std::string& left, const std::string& right, const std::string& op) const;
    bool CompareNumeric(const std::string& left, const std::string& right, const std::string& op) const;
    bool CompareString(const std::string& left, const std::string& right, const std::string& op) const;

    mutable std::mutex parser_mutex_;
};

// 外键约束验证器
class ForeignKeyValidator {
public:
    ForeignKeyValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~ForeignKeyValidator() = default;

    // 外键验证
    ConstraintValidationResult ValidateForeignKey(const std::string& table_name,
                                                const std::string& column_name,
                                                const std::string& value,
                                                const std::string& referenced_table,
                                                const std::string& referenced_column) const;

    // 级联操作
    bool ExecuteCascadeDelete(const std::string& table_name, const std::string& column_name,
                             const std::string& value, const std::string& referenced_table) const;
    bool ExecuteCascadeUpdate(const std::string& table_name, const std::string& column_name,
                             const std::string& old_value, const std::string& new_value,
                             const std::string& referenced_table) const;

    // 外键引用检查
    bool HasReferencingRecords(const std::string& table_name, const std::string& column_name,
                              const std::string& value) const;
    std::vector<std::pair<std::string, std::string>> GetReferencingRecords(const std::string& table_name,
                                                                          const std::string& column_name,
                                                                          const std::string& value) const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    mutable std::mutex validator_mutex_;

    // 外键关系缓存
    std::unordered_map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> fk_relationships_;
};

// 唯一约束验证器
class UniqueConstraintValidator {
public:
    UniqueConstraintValidator(std::shared_ptr<StorageEngine> storage_engine);
    ~UniqueConstraintValidator() = default;

    // 唯一性验证
    ConstraintValidationResult ValidateUniqueConstraint(const std::string& table_name,
                                                      const std::vector<std::string>& column_names,
                                                      const std::vector<std::string>& values,
                                                      int32_t exclude_record_id = -1) const;

    // 唯一索引管理
    bool CreateUniqueIndex(const std::string& table_name, const std::vector<std::string>& column_names);
    bool DropUniqueIndex(const std::string& table_name, const std::vector<std::string>& column_names);
    bool HasUniqueIndex(const std::string& table_name, const std::vector<std::string>& column_names) const;

    // 唯一值缓存管理
    void UpdateUniqueValueCache(const std::string& table_name, const std::vector<std::string>& column_names,
                               const std::vector<std::string>& old_values, const std::vector<std::string>& new_values);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    mutable std::mutex validator_mutex_;

    // 唯一约束索引缓存
    std::unordered_map<std::string, std::unordered_set<std::string>> unique_indexes_;
};

// 默认值处理器
class DefaultValueHandler {
public:
    DefaultValueHandler();
    ~DefaultValueHandler() = default;

    // 默认值应用
    std::string ApplyDefaultValue(const std::string& column_type, const std::string& default_expr) const;
    std::vector<std::string> ApplyDefaultValues(const std::vector<std::string>& column_types,
                                              const std::vector<std::string>& values,
                                              const std::vector<std::string>& default_exprs) const;

    // 默认值验证
    bool IsValidDefaultValue(const std::string& column_type, const std::string& default_value) const;
    std::string GenerateDefaultValue(const std::string& column_type) const;

private:
    // 默认值生成器
    std::string GenerateNumericDefault(const std::string& column_type) const;
    std::string GenerateStringDefault(const std::string& column_type) const;
    std::string GenerateDateTimeDefault(const std::string& column_type) const;
    std::string GenerateBooleanDefault(const std::string& column_type) const;

    mutable std::mutex handler_mutex_;
};

// 数据完整性约束验证器主类
class DataIntegrityValidator {
public:
    DataIntegrityValidator(std::shared_ptr<StorageEngine> storage_engine,
                          std::shared_ptr<TransactionManager> transaction_manager = nullptr);
    ~DataIntegrityValidator() = default;

    // 约束验证
    ConstraintValidationResult ValidateConstraints(const std::string& table_name,
                                                 const std::vector<std::string>& column_names,
                                                 const std::vector<std::string>& column_types,
                                                 const std::vector<std::string>& values,
                                                 int32_t record_id = -1) const;

    // 约束管理
    bool AddConstraint(const ConstraintRule& rule);
    bool RemoveConstraint(const std::string& table_name, const std::string& constraint_name);
    bool EnableConstraint(const std::string& table_name, const std::string& constraint_name);
    bool DisableConstraint(const std::string& table_name, const std::string& constraint_name);

    // 约束查询
    const ConstraintRule* GetConstraint(const std::string& table_name, const std::string& constraint_name) const;
    std::vector<ConstraintRule> GetTableConstraints(const std::string& table_name) const;
    std::vector<ConstraintRule> GetColumnConstraints(const std::string& table_name, const std::string& column_name) const;

    // 配置管理
    void SetValidationConfig(const IntegrityValidationConfig& config);
    const IntegrityValidationConfig& GetValidationConfig() const;

    // 统计信息
    struct IntegrityStats {
        size_t total_validations = 0;
        size_t successful_validations = 0;
        size_t constraint_violations = 0;
        size_t not_null_violations = 0;
        size_t unique_violations = 0;
        size_t foreign_key_violations = 0;
        size_t check_constraint_violations = 0;
        double average_validation_time_us = 0.0;
        std::chrono::steady_clock::time_point last_validation_time;
    };

    IntegrityStats GetIntegrityStats() const;

    // 批量验证
    std::vector<ConstraintValidationResult> ValidateConstraintsBatch(
        const std::string& table_name,
        const std::vector<std::vector<std::string>>& record_batch) const;

private:
    // 组件验证器
    CheckConstraintParser check_parser_;
    ForeignKeyValidator fk_validator_;
    UniqueConstraintValidator unique_validator_;
    DefaultValueHandler default_handler_;
    
    // 存储引擎和事务管理器
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;

    // 约束存储
    std::unordered_map<std::string, std::vector<ConstraintRule>> table_constraints_;

    // 配置
    IntegrityValidationConfig config_;

    // 统计信息
    mutable std::mutex stats_mutex_;
    mutable IntegrityStats stats_;
    mutable std::vector<double> validation_times_; // 验证时间记录

    // 私有辅助方法
    ConstraintValidationResult ValidateNotNullConstraints(const std::string& table_name,
                                                        const std::vector<std::string>& column_names,
                                                        const std::vector<std::string>& values) const;
    ConstraintValidationResult ValidateUniqueConstraints(const std::string& table_name,
                                                       const std::vector<std::string>& column_names,
                                                       const std::vector<std::string>& values,
                                                       int32_t record_id) const;
    ConstraintValidationResult ValidateForeignKeyConstraints(const std::string& table_name,
                                                           const std::vector<std::string>& column_names,
                                                           const std::vector<std::string>& values) const;
    ConstraintValidationResult ValidateCheckConstraints(const std::string& table_name,
                                                      const std::vector<std::string>& column_names,
                                                      const std::vector<std::string>& values) const;

    void UpdateIntegrityStats(ConstraintValidationResult result, std::chrono::microseconds duration) const;
    std::string GenerateConstraintKey(const std::string& table_name, const std::string& constraint_name) const;
};

// 约束验证器工厂
class DataIntegrityValidatorFactory {
public:
    static std::shared_ptr<DataIntegrityValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine);

    static std::shared_ptr<DataIntegrityValidator> CreateStrictValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager);

    static std::shared_ptr<DataIntegrityValidator> CreateEnterpriseValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager,
        const IntegrityValidationConfig& config);
};

// 约束验证结果格式化器
class ConstraintValidationResultFormatter {
public:
    static std::string FormatResult(ConstraintValidationResult result);
    static std::string FormatDetailedResult(ConstraintValidationResult result,
                                          const std::string& table_name = "",
                                          const std::string& constraint_name = "",
                                          const std::string& details = "");
    static std::string GetResultSeverity(ConstraintValidationResult result);
    static bool IsCriticalError(ConstraintValidationResult result);
};

} // namespace sqlcc

#endif // SQLCC_DATA_INTEGRITY_VALIDATOR_H
