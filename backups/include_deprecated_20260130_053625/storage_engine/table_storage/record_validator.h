#pragma once

#include <vector>
#include <string>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

/**
 * @brief 记录数据验证器类
 *
 * 该类提供对表记录数据的完整性验证功能，
 * 包括记录大小验证、字段类型验证、数据完整性验证等。
 */
class RecordValidator {
public:
    /**
     * @brief 验证记录大小
     *
     * @param record_size 记录大小（字节）
     * @param max_record_size 最大允许记录大小
     * @return 验证是否通过
     */
    static bool ValidateRecordSize(size_t record_size, size_t max_record_size);

    /**
     * @brief 验证字段值
     *
     * 根据字段类型验证字段值的有效性
     *
     * @param field_name 字段名称
     * @param field_type 字段类型
     * @param value 字段值字符串
     * @return 验证是否通过
     */
    static bool ValidateFieldValue(const std::string& field_name,
                                   const std::string& field_type,
                                   const std::string& value);

    /**
     * @brief 验证数据完整性
     *
     * 验证整个记录的数据完整性
     *
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
    RecordValidator(const RecordValidator&) = delete;
    RecordValidator& operator=(const RecordValidator&) = delete;
};

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
