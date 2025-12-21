#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

/**
 * @brief 记录安全验证类
 *
 * 提供数据完整性和类型安全的验证功能，确保记录数据的有效性
 * 和一致性。支持多种数据类型的验证和边界检查。
 */
class RecordValidator {
public:
    /**
     * @brief 验证记录大小限制
     * @param record_size 记录大小
     * @param max_record_size 最大记录大小限制（默认65536字节）
     * @return 验证是否通过
     */
    static bool ValidateRecordSize(size_t record_size, size_t max_record_size = 65536);

    /**
     * @brief 验证字段类型边界
     * @param field_name 字段名称
     * @param field_type 字段类型
     * @param value 字段值
     * @return 验证是否通过
     */
    static bool ValidateFieldValue(const std::string& field_name,
                                  const std::string& field_type,
                                  const std::string& value);

    /**
     * @brief 验证数据完整性约束
     * @param field_names 字段名称列表
     * @param field_types 字段类型列表
     * @param values 字段值列表
     * @return 验证是否通过
     */
    static bool ValidateDataIntegrity(const std::vector<std::string>& field_names,
                                    const std::vector<std::string>& field_types,
                                    const std::vector<std::string>& values);

private:
    // 私有构造函数，防止实例化
    RecordValidator() = delete;
    ~RecordValidator() = delete;

    // 禁止拷贝和赋值
    RecordValidator(const RecordValidator&) = delete;
    RecordValidator& operator=(const RecordValidator&) = delete;
};

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
