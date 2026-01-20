#ifndef SQLCC_CORE_SYSTEM_DATABASE_H
#define SQLCC_CORE_SYSTEM_DATABASE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sqlcc {

class DatabaseManager;

/**
 * WHY: 为什么需要系统数据库？
 *
 * 数据库系统需要存储大量的元数据信息：
 * - 用户权限和认证信息：用户名、密码、角色权限等
 * - 系统配置参数：数据库设置、优化参数等
 * - 统计信息：查询统计、性能指标等
 * - 元数据：表结构、索引信息、约束定义等
 *
 * 传统方法的问题：
 * 1. 元数据分散：存储在各个组件中，难以统一管理
 * 2. 一致性问题：多处存储导致数据不一致风险
 * 3. 访问复杂：每个组件都要实现自己的元数据访问
 * 4. 备份困难：元数据与业务数据耦合，难以独立备份
 *
 * 系统数据库的核心价值：
 * 1. 统一元数据管理：所有系统信息集中存储和管理
 * 2. 数据一致性保证：单一数据源避免不一致问题
 * 3. 简化访问接口：统一的元数据查询和更新接口
 * 4. 独立备份恢复：元数据可独立于业务数据备份
 * 5. 提高系统性能：优化元数据访问性能
 * 6. 支持系统扩展：便于添加新的系统功能
 *
 * 🏗️ 设计模式：系统数据库架构设计
 *
 * 设计模式应用：
 * 1. 单例模式(Singleton Pattern)：系统数据库全局唯一实例
 * 2. 工厂模式(Factory Pattern)：不同类型元数据的创建
 * 3. 代理模式(Proxy Pattern)：元数据访问的缓存代理
 * 4. 观察者模式(Observer Pattern)：元数据变化通知
 *
 * SOLID原则体现：
 * - 单一职责：专职负责系统元数据管理
 * - 开闭原则：新元数据类型通过扩展实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: 系统数据库 - 数据库系统的元数据管理中心
 *
 * 核心功能：
 * - 用户管理：存储和管理用户账户信息
 * - 权限控制：管理用户权限和访问控制
 * - 配置管理：存储系统配置参数
 * - 统计收集：记录系统运行统计信息
 * - 元数据存储：保存数据库对象的元数据
 *
 * 存储内容：
 * - 用户表：用户信息、认证凭据、权限设置
 * - 配置表：系统参数、优化设置、运行配置
 * - 统计表：性能指标、查询统计、使用情况
 * - 元数据表：表定义、索引信息、约束关系
 *
 * 接口设计：
 * - Initialize(): 初始化系统数据库
 * - IsInitialized(): 检查初始化状态
 * - GetDatabaseManager(): 获取数据库管理器
 * - GetLastError(): 获取最后错误信息
 *
 * HOW: 系统数据库的实现机制
 *
 * 数据库结构：
 * - 系统表：存储用户、配置、统计等系统信息
 * - 元数据表：存储数据库对象的定义和属性
 * - 索引优化：为频繁查询的元数据建立索引
 * - 事务保证：确保元数据操作的事务一致性
 *
 * 访问控制：
 * - 权限验证：检查用户对元数据的访问权限
 * - 审计记录：记录所有元数据访问操作
 * - 安全隔离：防止普通用户访问敏感元数据
 *
 * 性能优化：
 * - 缓存机制：热点元数据的内存缓存
 * - 查询优化：优化元数据查询性能
 * - 批量操作：支持批量元数据操作
 * - 异步更新：非阻塞的元数据更新
 *
 * 扩展性设计：
 * - 插件架构：支持自定义元数据类型
 * - 配置化管理：可配置的元数据策略
 * - 事件驱动：元数据变化事件通知
 * - 多租户支持：租户级别的元数据隔离
 * - 分布式支持：支持分布式元数据管理
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
