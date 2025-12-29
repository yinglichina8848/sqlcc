#ifndef SQLCC_VIEW_MANAGER_H
#define SQLCC_VIEW_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace sqlcc {

class SqlExecutor;

/**
 * @brief 视图管理器 - 负责管理数据库视图
 *
 * 视图是虚拟表，基于SQL查询的结果集。视图管理器提供创建、删除、
 * 修改和查询视图的功能。
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
