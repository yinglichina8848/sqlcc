#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief 存储访问器接口
 *
 * 定义存储引擎的基本数据访问操作，用于解耦事务管理器和存储引擎的直接依赖。
 * 通过依赖倒置原则，事务管理器不再直接依赖具体的存储引擎实现，
 * 而是通过此接口进行数据操作。
 */
class StorageAccessor {
public:
    virtual ~StorageAccessor() = default;

    /**
     * @brief 插入记录
     * @param table 表名
     * @param record 记录数据
     * @return 是否成功
     */
    virtual bool insertRecord(const std::string& table, const std::vector<std::string>& record) = 0;

    /**
     * @brief 更新记录
     * @param table 表名
     * @param record_id 记录ID
     * @param record 新记录数据
     * @return 是否成功
     */
    virtual bool updateRecord(const std::string& table, const std::string& record_id, const std::vector<std::string>& record) = 0;

    /**
     * @brief 删除记录
     * @param table 表名
     * @param record_id 记录ID
     * @return 是否成功
     */
    virtual bool deleteRecord(const std::string& table, const std::string& record_id) = 0;

    /**
     * @brief 查询记录
     * @param table 表名
     * @param conditions 查询条件
     * @return 记录列表
     */
    virtual std::vector<std::vector<std::string>> queryRecords(const std::string& table, const std::string& conditions) = 0;

    /**
     * @brief 获取表元数据
     * @param table 表名
     * @return 表元数据
     */
    virtual std::shared_ptr<class TableMetadata> getTableMetadata(const std::string& table) = 0;

    /**
     * @brief 检查表是否存在
     * @param table 表名
     * @return 是否存在
     */
    virtual bool tableExists(const std::string& table) = 0;

    /**
     * @brief 创建表
     * @param table 表名
     * @param schema 表结构定义
     * @return 是否成功
     */
    virtual bool createTable(const std::string& table, const std::string& schema) = 0;

    /**
     * @brief 删除表
     * @param table 表名
     * @return 是否成功
     */
    virtual bool dropTable(const std::string& table) = 0;
};

} // namespace sqlcc
