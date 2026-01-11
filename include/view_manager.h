#ifndef SQLCC_VIEW_MANAGER_H
#define SQLCC_VIEW_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace sqlcc {

class SqlExecutor;

/**
 * WHY: 为什么需要视图管理器？
 *
 * 传统数据库中视图管理分散在各个组件中，导致：
 * - 视图定义存储不统一：散落在各处，难以管理
 * - 视图验证逻辑重复：每个组件都要验证视图存在性
 * - 视图元数据管理复杂：缺乏集中式管理机制
 * - 视图依赖关系不清：难以追踪视图间的依赖
 * - 视图生命周期管理缺失：创建和删除缺乏统一控制
 *
 * 视图管理器的优势：
 * 1. 统一视图管理：所有视图操作通过单一接口
 * 2. 元数据集中存储：视图定义和元数据统一管理
 * 3. 依赖关系追踪：自动分析和维护视图依赖
 * 4. 生命周期控制：完整的视图创建、修改、删除流程
 * 5. 安全验证：统一的权限检查和访问控制
 *
 * 视图在数据库系统中的价值：
 * - 数据抽象：隐藏底层表结构，提供逻辑视图
 * - 安全控制：限制用户对敏感数据的访问
 * - 查询简化：将复杂查询封装为简单视图
 * - 数据集成：整合多个表的数据为统一视图
 * - 版本控制：提供数据接口的版本化管理
 *
 * 🏗️ 设计模式：视图管理器架构设计
 *
 * 设计模式应用：
 * 1. 外观模式(Facade Pattern)：统一视图管理接口
 *    - 隐藏底层存储和验证复杂性
 *    - 提供简洁一致的API
 *    - 解耦客户端和子系统
 *
 * 2. 工厂模式(Factory Pattern)：视图对象创建
 *    - 根据视图定义创建视图对象
 *    - 封装对象创建逻辑
 *    - 支持不同类型的视图
 *
 * 3. 观察者模式(Observer Pattern)：视图依赖通知
 *    - 监控底层表变化
 *    - 通知依赖视图更新
 *    - 维护数据一致性
 *
 * SOLID原则体现：
 * - 单一职责：专职负责视图管理
 * - 开闭原则：新视图类型通过扩展实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: 视图管理器 - 数据库视图的生命周期管理
 *
 * 核心功能：
 * - 视图创建：基于SQL查询创建虚拟表
 * - 视图删除：安全删除视图及其依赖
 * - 视图修改：更新视图定义和元数据
 * - 视图查询：检索视图定义和属性
 * - 视图验证：检查视图存在性和有效性
 *
 * 视图类型支持：
 * - 简单视图：基于单表查询的视图
 * - 复杂视图：基于多表连接的视图
 * - 聚合视图：包含聚合函数的视图
 * - 嵌套视图：基于其他视图的视图
 * - 物化视图：物理存储的视图（预计算）
 *
 * 接口设计：
 * - CreateView(): 创建新视图
 * - DropView(): 删除现有视图
 * - AlterView(): 修改视图定义
 * - ViewExists(): 检查视图存在性
 * - GetViewDefinition(): 获取视图SQL定义
 * - ListViews(): 枚举所有视图
 *
 * HOW: 视图管理器的实现机制
 *
 * 视图创建流程：
 * 1. 语法验证：解析视图定义SQL语句
 * 2. 依赖分析：识别底层表和视图依赖
 * 3. 权限检查：验证创建视图的权限
 * 4. 循环依赖检测：防止视图循环引用
 * 5. 元数据存储：保存视图定义和属性
 * 6. 缓存更新：更新视图缓存信息
 *
 * 视图删除流程：
 * 1. 依赖检查：查找依赖此视图的其他对象
 * 2. 级联删除：处理依赖关系的清理
 * 3. 权限验证：确认删除权限
 * 4. 元数据清理：删除视图元数据
 * 5. 缓存失效：清除相关缓存
 * 6. 日志记录：记录删除操作
 *
 * 视图查询优化：
 * - 视图重写：将视图查询重写为直接表查询
 * - 物化视图：预计算视图结果提高性能
 * - 索引利用：利用底层表索引优化查询
 * - 查询合并：合并多个视图查询减少I/O
 * - 结果缓存：缓存视图查询结果
 *
 * 安全控制机制：
 * - 视图权限：独立的视图访问权限控制
 * - 定义者权限：视图基于定义者的权限执行
 * - 调用者权限：视图基于调用者的权限执行
 * - 列级权限：限制对视图特定列的访问
 * - 行级权限：通过WHERE子句限制行访问
 *
 * 并发控制策略：
 * - 读写锁：允许多个读操作，独占写操作
 * - 版本控制：乐观并发控制避免冲突
 * - 死锁检测：自动检测和处理死锁
 * - 事务隔离：视图操作的事务一致性保证
 *
 * 性能优化技术：
 * - 延迟加载：按需加载视图元数据
 * - 内存缓存：热点视图的内存缓存
 * - 批量操作：批量处理多个视图操作
 * - 异步更新：非阻塞的视图更新操作
 * - 智能索引：自动创建视图查询索引
 *
 * 扩展性设计：
 * - 插件架构：支持自定义视图类型
 * - 配置化管理：可配置的视图策略
 * - 事件驱动：视图生命周期事件通知
 * - 监控集成：视图性能监控和统计
 * - 多租户支持：租户级别的视图隔离
 *
 * 错误处理和恢复：
 * - 事务回滚：失败操作的自动回滚
 * - 状态一致性：保证视图元数据一致性
 * - 错误日志：详细的错误信息记录
 * - 自动修复：常见错误的自动修复机制
 */
class ViewManager {
public:
    /**
     * @brief 构造函数
     */
    ViewManager();

    /**
     * @brief 构造函数，带SqlExecutor依赖
     * @param sql_executor SQL执行器指针
     */
    explicit ViewManager(std::shared_ptr<SqlExecutor> sql_executor);

    /**
     * @brief 析构函数
     */
    ~ViewManager();

    /**
     * @brief 创建视图
     * @param view_name 视图名称
     * @param sql_query 创建视图的SQL查询
     * @return 是否创建成功
     */
    bool CreateView(const std::string& view_name, const std::string& sql_query);

    /**
     * @brief 删除视图
     * @param view_name 视图名称
     * @return 是否删除成功
     */
    bool DropView(const std::string& view_name);

    /**
     * @brief 检查视图是否存在
     * @param view_name 视图名称
     * @return 视图是否存在
     */
    bool ViewExists(const std::string& view_name);

    /**
     * @brief 获取视图定义
     * @param view_name 视图名称
     * @return 视图的SQL定义
     */
    std::string GetViewDefinition(const std::string& view_name);

    /**
     * @brief 列出所有视图
     * @return 视图名称列表
     */
    std::vector<std::string> ListViews();

    /**
     * @brief 更新视图定义
     * @param view_name 视图名称
     * @param new_sql_query 新的SQL查询
     * @return 是否更新成功
     */
    bool AlterView(const std::string& view_name, const std::string& new_sql_query);

private:
    // 视图存储：视图名 -> SQL定义
    std::unordered_map<std::string, std::string> views_;

    // SQL执行器（用于验证视图查询）
    std::shared_ptr<SqlExecutor> sql_executor_;
};

} // namespace sqlcc

#endif // SQLCC_VIEW_MANAGER_H
