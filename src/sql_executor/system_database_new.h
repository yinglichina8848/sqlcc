#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace sqlcc {

// 前向声明
class DatabaseManager;
class SystemSchemaManager;
class SystemDataInitializer;
class SystemPermissionManager;
class SystemMetadataManager;

// 系统数据结构前向声明
struct SysUser;
struct SysRole;
struct SysDatabase;
struct SysTable;
struct SysView;

/**
 * @brief 系统数据库管理类
 * 
 * 该类负责管理整个数据库系统的元数据和权限信息，包括：
 * - 系统数据库的初始化
 * - 数据库、用户、角色、表等对象的管理
 * - 权限控制
 * 
 * 设计模式：
 * - 单例模式：确保全局只有一个系统数据库实例
 * - 工厂模式：创建和管理各种系统对象
 * - 策略模式：根据不同类型的对象执行不同的管理策略
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
     * @brief 检查系统数据库是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;
    
    /**
     * @brief 获取数据库管理器
     * @return 数据库管理器指针
     */
    std::shared_ptr<DatabaseManager> GetDatabaseManager() const;
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string GetLastError() const;
    
    // 数据库操作
    bool CreateDatabase(const std::string& db_name, const std::string& owner = "", const std::string& description = "");
    bool DropDatabase(const std::string& db_name);
    
    // 用户操作
    bool CreateUser(const std::string& username, const std::string& password, const std::string& email = "", const std::string& role = "");
    bool DropUser(const std::string& username);
    bool UpdateUser(const std::string& username, const std::string& password, const std::string& email = "");
    
    // 角色操作
    bool CreateRole(const std::string& role_name, const std::string& description = "");
    bool DropRole(const std::string& role_name);
    
    // 权限操作
    bool GrantPrivilege(const std::string& username, const std::string& object_type, const std::string& object_name, const std::string& privilege);
    bool RevokePrivilege(const std::string& username, const std::string& object_type, const std::string& object_name, const std::string& privilege);
    
    // 查询操作
    SysUser GetUser(const std::string& username);
    SysRole GetRole(const std::string& role_name);
    SysDatabase GetDatabase(const std::string& db_name);
    
    std::vector<SysUser> ListUsers();
    std::vector<SysRole> ListRoles();
    std::vector<SysDatabase> ListDatabases();
    std::vector<SysTable> ListTables(const std::string& db_name);
    std::vector<SysView> ListViews(const std::string& db_name);
    
    // 验证操作
    bool ValidateUser(const std::string& username, const std::string& password);
    bool HasPrivilege(const std::string& username, const std::string& object_type, const std::string& object_name, const std::string& privilege);
    
    // 获取管理器
    SystemSchemaManager* GetSchemaManager() const;
    SystemDataInitializer* GetDataInitializer() const;
    SystemPermissionManager* GetPermissionManager() const;
    SystemMetadataManager* GetMetadataManager() const;

private:
    // 私有成员变量
    std::shared_ptr<DatabaseManager> db_manager_;                    // 数据库管理器
    std::unique_ptr<SystemSchemaManager> schema_manager_;             // 系统表管理器
    std::unique_ptr<SystemDataInitializer> data_initializer_;        // 数据初始化器
    std::unique_ptr<SystemPermissionManager> permission_manager_;     // 权限管理器
    std::unique_ptr<SystemMetadataManager> metadata_manager_;         // 元数据管理器
    
    bool is_initialized_;                                            // 是否已初始化
    std::string last_error_;                                          // 最后一次错误信息
    
    // 私有方法
    bool Exists();                                                    // 检查系统数据库是否存在
    std::string GetCurrentTimeString();                              // 获取当前时间字符串
    uint64_t GenerateId();                                           // 生成唯一ID
};

// 系统数据库名称常量
const std::string SYSTEM_DB_NAME = "system";

} // namespace sqlcc