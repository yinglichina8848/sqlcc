#pragma once

#include "storage_accessor.h"
#include "storage_engine.h"

namespace sqlcc {

/**
 * @brief StorageAccessor接口的具体实现
 *
 * 基于StorageEngine实现存储访问器接口，
 * 通过组合模式将StorageEngine的功能适配为StorageAccessor接口。
 */
class StorageAccessorImpl : public StorageAccessor {
public:
    /**
     * @brief 构造函数
     * @param storage_engine StorageEngine的智能指针
     */
    explicit StorageAccessorImpl(std::shared_ptr<StorageEngine> storage_engine);

    /**
     * @brief 插入记录
     * @param table 表名
     * @param record 记录数据
     * @return 是否成功
     */
    bool insertRecord(const std::string& table, const std::vector<std::string>& record) override;

    /**
     * @brief 更新记录
     * @param table 表名
     * @param record_id 记录ID
     * @param record 新记录数据
     * @return 是否成功
     */
    bool updateRecord(const std::string& table, const std::string& record_id, const std::vector<std::string>& record) override;

    /**
     * @brief 删除记录
     * @param table 表名
     * @param record_id 记录ID
     * @return 是否成功
     */
    bool deleteRecord(const std::string& table, const std::string& record_id) override;

    /**
     * @brief 查询记录
     * @param table 表名
     * @param conditions 查询条件
     * @return 记录列表
     */
    std::vector<std::vector<std::string>> queryRecords(const std::string& table, const std::string& conditions) override;

    /**
     * @brief 获取表元数据
     * @param table 表名
     * @return 表元数据
     */
    std::shared_ptr<class TableMetadata> getTableMetadata(const std::string& table) override;

    /**
     * @brief 检查表是否存在
     * @param table 表名
     * @return 是否存在
     */
    bool tableExists(const std::string& table) override;

    /**
     * @brief 创建表
     * @param table 表名
     * @param schema 表结构定义
     * @return 是否成功
     */
    bool createTable(const std::string& table, const std::string& schema) override;

    /**
     * @brief 删除表
     * @param table 表名
     * @return 是否成功
     */
    bool dropTable(const std::string& table) override;

private:
    /// StorageEngine的智能指针
    std::shared_ptr<StorageEngine> storage_engine_;
};

} // namespace sqlcc
