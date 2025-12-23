#include "../../include/network/session_manager.h"
#include "../../include/network/session.h"

namespace sqlcc {
namespace network {

// SessionManager实现
SessionManager::SessionManager() : next_session_id_(1) {}

std::shared_ptr<Session> SessionManager::CreateSession() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    // 检查会话ID溢出
    if (next_session_id_ <= 0) {
        next_session_id_ = 1;  // 重置为1
    }
    
    int session_id = next_session_id_++;
    auto session = std::make_shared<Session>(session_id);
    sessions_[session_id] = session;
    return session;
}

std::shared_ptr<Session> SessionManager::GetSession(int session_id) {
    if (session_id <= 0) {  // 会话ID应该是正数
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        auto session = it->second.lock();
        if (session) {
            return session;
        } else {
            // 弱引用已失效，从映射中移除
            sessions_.erase(it);
        }
    }
    return nullptr;
}

void SessionManager::DestroySession(int session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(session_id);
}

bool SessionManager::Authenticate(int session_id, const std::string& username, 
                                const std::string& password) {
    // 更严格的认证逻辑，防止特殊字符导致的问题
    if (username.empty() || password.empty()) {
        return false;
    }
    
    // 检查用户名和密码长度限制
    if (username.length() > 255 || password.length() > 255) {
        return false;
    }
    
    // 简单的身份验证逻辑
    if (username == "admin" && password == "password") {
        auto session = GetSession(session_id);
        if (session) {
            session->SetAuthenticated(username);
            return true;
        }
    }
    return false;
}

bool SessionManager::CheckPermission(int session_id, const std::string& database,
                                   const std::string& operation) {
    auto session = GetSession(session_id);
    if (!session || !session->IsAuthenticated()) {
        return false;
    }
    // 简单的权限检查逻辑
    return true;
}

} // namespace network
} // namespace sqlcc
