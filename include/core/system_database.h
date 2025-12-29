#ifndef SQLCC_CORE_SYSTEM_DATABASE_H
#define SQLCC_CORE_SYSTEM_DATABASE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sqlcc {

class DatabaseManager;

/**
 * @brief 系统数据库 - 管理系统级数据和元数据
 *
 * 系统数据库负责存储和管理数据库系统的元数据信息，
 * 包括用户权限、系统配置、统计信息等。
 */
class SystemDatabase {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器指针
     */
    explicit SystemDatabase(std::shared_ptr<DatabaseManager> db_manager);

    /**
     * @brief 析构函数
     */
    ~SystemDatabase();

    /**
     * @brief 初始化系统数据库
     * @return 初始化是否成功
     */
    bool Initialize();

    /**
     * @brief 获取数据库管理器
     * @return 数据库管理器指针
     */
    std::shared_ptr<DatabaseManager> GetDatabaseManager();

    /**
     * @brief 检查系统数据库是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;

    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

private:
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    bool is_initialized_;                         // 是否已初始化
    std::string last_error_;                      // 最后一次错误信息
};

} // namespace sqlcc

#endif // SQLCC_CORE_SYSTEM_DATABASE_H
