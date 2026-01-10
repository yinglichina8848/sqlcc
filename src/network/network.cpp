#include <network/network.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <queue>
#include <mutex>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/epoll.h>
#include <netdb.h>
#endif

#include "network/encryption.h"
#include "utils/file_descriptor.h"

namespace sqlcc {
namespace network {

// Session实现
Session::Session(int session_id) : session_id_(session_id), authenticated_(false), encryption_disabled_(false), authentication_disabled_(false), aes_encryptor_(nullptr) {}

void Session::SetEncryptionDisabled(bool disabled) {
    encryption_disabled_ = disabled;
}

bool Session::IsEncryptionDisabled() const {
    return encryption_disabled_;
}

void Session::SetAuthenticationDisabled(bool disabled) {
    authentication_disabled_ = disabled;
}

bool Session::IsAuthenticationDisabled() const {
    return authentication_disabled_;
}

void Session::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
}

std::shared_ptr<class AESEncryptor> Session::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool Session::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr && !encryption_disabled_;
}

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

// ClientConnection实现
MessageProcessor::MessageProcessor(std::shared_ptr<SessionManager> session_manager)
    : session_manager_(std::move(session_manager)) {}

// ServerNetworkManager实现
ServerNetworkManager::ServerNetworkManager(int port, int max_connections)
    : port_(port), max_connections_(max_connections), running_(false),
      session_manager_(std::make_shared<SessionManager>()),
      user_manager_(std::make_shared<::sqlcc::UserManager>("./data")) {
}

ServerNetworkManager::~ServerNetworkManager() {
    Stop();
}

bool ServerNetworkManager::Start() {
#ifdef __linux__
    // 创建监听socket
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        return false;
    }
    listen_fd_ = ::sqlcc::FileDescriptor(fd);

    // 设置socket选项
    int opt = 1;
    if (setsockopt(listen_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return false;
    }

    // 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_.get(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }

    // 开始监听
    if (listen(listen_fd_.get(), SOMAXCONN) < 0) {
        return false;
    }

    // 创建epoll实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        return false;
    }
    epoll_fd_ = ::sqlcc::FileDescriptor(epoll_fd);

    // 添加监听socket到epoll
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;  // 监听socket用nullptr标识
    if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, listen_fd_.get(), &ev) < 0) {
        return false;
    }

    running_ = true;
    return true;
#endif
    return false;
}

void ServerNetworkManager::Stop() {
#ifdef __linux__
    running_ = false;

    // 关闭所有连接
    connections_.clear();

    // FileDescriptor会自动关闭
#endif
}

void ServerNetworkManager::ProcessEvents() {
#ifdef __linux__
    if (!running_ || epoll_fd_.get() < 0) {
        return;
    }

    struct epoll_event events[64];
    int nfds = epoll_wait(epoll_fd_.get(), events, 64, 0);

    for (int i = 0; i < nfds; ++i) {
        if (events[i].data.ptr == nullptr) {
            // 监听socket有事件，接受新连接
            AcceptConnection();
        } else {
            // 客户端连接有事件
            ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
            if (handler) {
                handler->HandleEvent(events[i].events);
            }
        }
    }
#endif
}

void ServerNetworkManager::AcceptConnection() {
#ifdef __linux__
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept4(listen_fd_.get(), (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
        return;
    }

    // 检查连接数量限制
    if (connections_.size() >= static_cast<size_t>(max_connections_)) {
        close(client_fd);
        return;
    }

    // 创建连接处理器
    std::unique_ptr<ConnectionHandler> handler = std::make_unique<ConnectionHandler>(
        ::sqlcc::FileDescriptor(client_fd),
        session_manager_,
        sql_executor_,
        user_manager_
    );

    // 添加到epoll
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLET;  // 边缘触发
    ev.data.ptr = handler.get();

    if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        return;
    }

    // 添加到连接映射
    connections_[client_fd] = std::move(handler);
#endif
}

void ServerNetworkManager::SetSqlExecutor(std::shared_ptr<sqlcc::SqlExecutor> sql_executor) {
    sql_executor_ = std::move(sql_executor);
}

#ifdef __linux__
void ServerNetworkManager::EnableTLS(bool enabled) {
    // TLS实现
}

bool ServerNetworkManager::ConfigureTLSServer(const std::string& cert_path,
                                              const std::string& key_path,
                                              const std::string& ca_cert_path) {
    // TLS配置实现
    return false;
}
#endif

} // namespace network
} // namespace sqlcc
