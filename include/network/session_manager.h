/**
 * @file session_manager.h
 * @brief 会话管理器类定义
 *
 * Why: 需要专门的类来管理多个会话的生命周期和权限控制
 * What: SessionManager类提供线程安全的会话管理和权限验证
 * How: 实现会话的创建、销毁和权限检查功能
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include "session.h"

namespace sqlcc {
namespace network {

/**
 * @brief 会话管理器类
 *
 * 管理数据库用户的所有会话，提供线程安全的会话生命周期管理和权限验证
 */
class SessionManager {
public:
    /**
     * @brief 构造函数
     */
    SessionManager();

    /**
     * @brief 析构函数
     */
    ~SessionManager() = default;

    /**
     * @brief 创建新会话
     * @return 新创建的会话指针
     */
    std::shared_ptr<Session> CreateSession();

    /**
     * @brief 获取会话
     * @param session_id 会话ID
     * @return 会话指针，如果不存在返回nullptr
     */
    std::shared_ptr<Session> GetSession(int session_id);

    /**
     * @brief 销毁会话
     * @param session_id 会话ID
     */
    void DestroySession(int session_id);

    /**
     * @brief 认证用户
     * @param session_id 会话ID
     * @param username 用户名
     * @param password 密码
     * @return 认证是否成功
     */
    bool Authenticate(int session_id, const std::string& username,
                     const std::string& password);

    /**
     * @brief 检查权限
     * @param session_id 会话ID
     * @param database 数据库名
     * @param operation 操作名
     * @return 是否有权限
     */
    bool CheckPermission(int session_id, const std::string& database,
                        const std::string& operation);

    /**
     * @brief 获取活动会话数量
     * @return 活动会话数量
     */
    size_t GetActiveSessionCount() const;

    /**
     * @brief 清理过期会话
     */
    void CleanupExpiredSessions();

private:
    std::unordered_map<int, std::weak_ptr<Session>> sessions_; ///< 会话存储
    mutable std::mutex sessions_mutex_;                        ///< 线程安全锁
    int next_session_id_;                                      ///< 下一个会话ID

    /**
     * @brief 生成新的会话ID
     * @return 新会话ID
     */
    int GenerateSessionId();
};

} // namespace network
} // namespace sqlcc
