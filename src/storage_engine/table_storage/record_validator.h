#pragma once

#include <vector>
#include <string>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

class RecordValidator {
public:
    /**
     * @brief 验证记录大小是否有效
     * @param record_size 记录大小
     * @param max_record_size 最大记录大小
     * @return 是否有效
     */
    static bool ValidateRecordSize(size_t record_size, size_t max_record_size);

    /**
     * @brief 验证字段值是否有效
     * @param field_name 字段名
     * @param field_type 字段类型
     * @param value 字段值
     * @return 是否有效
     */
    static bool ValidateFieldValue(const std::string& field_name,
                                  const std::string& field_type,
                                  const std::string& value);

    /**
     * @brief 验证数据完整性
     * @param field_names 字段名列表
     * @param field_types 字段类型列表
     * @param values 值列表
     * @return 是否完整
     */
    static bool ValidateDataIntegrity(const std::vector<std::string>& field_names,
                                    const std::vector<std::string>& field_types,
                                    const std::vector<std::string>& values);
};

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
