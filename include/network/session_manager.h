#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <unordered_map>

namespace sqlcc {
namespace network {

class Session;

/**
 * @brief 会话管理器类，负责管理所有客户端会话
 *
 * SessionManager类提供线程安全的会话管理功能，包括：
 * - 会话的创建、查找和销毁
 * - 用户认证和权限检查
 * - 会话ID的唯一性保证
 */
class SessionManager {
public:
    /**
     * @brief 构造函数
     */
    SessionManager();

    /**
     * @brief 创建新会话
     * @return 新创建的会话智能指针
     */
    std::shared_ptr<Session> CreateSession();

    /**
     * @brief 根据会话ID查找会话
     * @param session_id 会话ID
     * @return 会话智能指针，如果不存在返回nullptr
     */
    std::shared_ptr<Session> GetSession(int session_id);

    /**
     * @brief 销毁指定会话
     * @param session_id 要销毁的会话ID
     */
    void DestroySession(int session_id);

    /**
     * @brief 用户认证
     * @param session_id 会话ID
     * @param username 用户名
     * @param password 密码
     * @return true表示认证成功，false表示认证失败
     */
    bool Authenticate(int session_id, const std::string& username,
                     const std::string& password);

    /**
     * @brief 检查用户权限
     * @param session_id 会话ID
     * @param database 数据库名
     * @param operation 操作名
     * @return true表示有权限，false表示无权限
     */
    bool CheckPermission(int session_id, const std::string& database,
                        const std::string& operation);

private:
    std::mutex sessions_mutex_;                                    ///< 会话映射表的互斥锁
    std::unordered_map<int, std::weak_ptr<Session>> sessions_;    ///< 会话映射表（使用弱引用避免循环依赖）
    int next_session_id_;                                          ///< 下一个会话ID
};

} // namespace network
} // namespace sqlcc
