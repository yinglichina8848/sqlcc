#include <network/server_network_manager.h>
#include <network/session.h>
#include <network/session_manager.h>
#include <network/connection_handler.h>
#include "utils/file_descriptor.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <system_error>

#ifdef __linux__
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace sqlcc {
namespace network {

ServerNetworkManager::ServerNetworkManager(int port, int max_connections)
    : port_(port),
      max_connections_(max_connections),
      running_(false),
      session_manager_(std::make_shared<SessionManager>()),
      sql_executor_(nullptr),
      user_manager_(std::make_shared<sqlcc::UserManager>("./data"))
#ifdef __linux__
      , listen_fd_(-1),
      epoll_fd_(-1),
      tls_enabled_(false),
      ssl_ctx_(nullptr)
#endif
{
}

ServerNetworkManager::~ServerNetworkManager() {
    Stop();
#ifdef __linux__
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }
#endif
}

bool ServerNetworkManager::Start() {
    if (running_) {
        return true;
    }

#ifdef __linux__
    try {
        // 创建监听socket
        listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (listen_fd_ < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to create listen socket");
        }

        // 设置socket选项
        int opt = 1;
        if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to set socket options");
        }

        // 绑定地址
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);

        if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to bind socket");
        }

        // 监听连接
        if (listen(listen_fd_, SOMAXCONN) < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to listen on socket");
        }

        // 创建epoll实例
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to create epoll instance");
        }

        // 添加监听socket到epoll
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = listen_fd_;

        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event) < 0) {
            throw std::system_error(errno, std::system_category(),
                                  "Failed to add listen socket to epoll");
        }

        running_ = true;
        std::cout << "Server started on port " << port_ << std::endl;
        return true;

    } catch (const std::system_error& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        Stop();
        return false;
    }
#else
    // 非Linux平台不支持
    std::cerr << "Server network manager requires Linux platform" << std::endl;
    return false;
#endif
}

void ServerNetworkManager::Stop() {
    running_ = false;

#ifdef __linux__
    // 关闭所有连接
    connections_.clear();

    // 关闭epoll
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }

    // 关闭监听socket
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
#endif
}

void ServerNetworkManager::ProcessEvents() {
    if (!running_) {
        return;
    }

#ifdef __linux__
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, 100); // 100ms timeout

    if (num_events < 0) {
        if (errno != EINTR) {
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
        }
        return;
    }

    for (int i = 0; i < num_events; ++i) {
        int fd = events[i].data.fd;

        if (fd == listen_fd_) {
            // 新连接
            AcceptConnection();
        } else {
            // 现有连接的事件
            auto it = connections_.find(fd);
            if (it != connections_.end()) {
                // 处理连接事件
                it->second->HandleEvent(events[i].events);
            }
        }
    }
#endif
}

void ServerNetworkManager::AcceptConnection() {
#ifdef __linux__
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept4(listen_fd_,
                           reinterpret_cast<sockaddr*>(&client_addr),
                           &addr_len,
                           SOCK_NONBLOCK);

    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        }
        return;
    }

    // 检查连接数限制
    if (connections_.size() >= static_cast<size_t>(max_connections_)) {
        close(client_fd);
        return;
    }

    // 创建连接处理器
    auto connection_handler = std::make_unique<ConnectionHandler>(sqlcc::FileDescriptor(client_fd), session_manager_, sql_executor_, user_manager_);

    // 如果启用TLS，创建SSL连接
    if (tls_enabled_ && ssl_ctx_) {
        try {
            // 创建SSL对象
            SSL* ssl = SSL_new(ssl_ctx_);
            if (!ssl) {
                std::cerr << "Failed to create SSL object for client connection" << std::endl;
                close(client_fd);
                return;
            }

            // 将SSL绑定到socket
            if (SSL_set_fd(ssl, client_fd) != 1) {
                std::cerr << "Failed to bind SSL to socket" << std::endl;
                SSL_free(ssl);
                close(client_fd);
                return;
            }

            // 执行SSL握手（非阻塞模式）
            int ssl_result = SSL_accept(ssl);
            if (ssl_result == 1) {
                // 握手成功
                connection_handler->SetTLS(ssl, true);
                std::cout << "SSL handshake successful for client connection" << std::endl;
            } else {
                int ssl_error = SSL_get_error(ssl, ssl_result);
                if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                    // 非阻塞，需要等待更多数据
                    connection_handler->SetTLS(ssl, true);
                    std::cout << "SSL handshake in progress for client connection" << std::endl;
                } else {
                    // 握手失败
                    std::cerr << "SSL handshake failed for client connection" << std::endl;
                    SSL_free(ssl);
                    close(client_fd);
                    return;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during SSL setup: " << e.what() << std::endl;
            close(client_fd);
            return;
        }
    }

    connections_[client_fd] = std::move(connection_handler);

    // 添加到epoll
    epoll_event event{};
    event.events = EPOLLIN | EPOLLET; // 边缘触发
    event.data.fd = client_fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) < 0) {
        std::cerr << "Failed to add client socket to epoll: " << strerror(errno) << std::endl;
        connections_.erase(client_fd);
        close(client_fd);
    }
#endif
}

void ServerNetworkManager::SetSqlExecutor(std::shared_ptr<sqlcc::SqlExecutor> sql_executor) {
    sql_executor_ = sql_executor;
}

void ServerNetworkManager::EnableTLS(bool enabled) {
    tls_enabled_ = enabled;
}

#ifdef __linux__
bool ServerNetworkManager::ConfigureTLSServer(const std::string& cert_path,
                                             const std::string& key_path,
                                             const std::string& ca_cert_path) {
    if (!tls_enabled_) {
        return false;
    }

    // 初始化OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // 创建SSL上下文
    const SSL_METHOD* method = TLS_server_method();
    ssl_ctx_ = SSL_CTX_new(method);
    if (!ssl_ctx_) {
        std::cerr << "Failed to create SSL context" << std::endl;
        return false;
    }

    // 设置证书和私钥
    if (SSL_CTX_use_certificate_file(ssl_ctx_, cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Failed to load certificate file" << std::endl;
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Failed to load private key file" << std::endl;
        return false;
    }

    // 验证私钥
    if (!SSL_CTX_check_private_key(ssl_ctx_)) {
        std::cerr << "Private key does not match certificate" << std::endl;
        return false;
    }

    // 设置CA证书（如果提供）
    if (!ca_cert_path.empty()) {
        if (SSL_CTX_load_verify_locations(ssl_ctx_, ca_cert_path.c_str(), nullptr) <= 0) {
            std::cerr << "Failed to load CA certificate" << std::endl;
            return false;
        }
    }

    return true;
}
#endif

} // namespace network
} // namespace sqlcc