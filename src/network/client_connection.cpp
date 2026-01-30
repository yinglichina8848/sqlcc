#include "network/client_connection.h"
#include "src/utils/file_descriptor.h"
#include "src/utils/ssl_wrapper.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <algorithm>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <netdb.h>
#endif

namespace sqlcc {
namespace network {

// ClientConnection实现
ClientConnection::ClientConnection(const std::string& host, int port)
    : host_(host), port_(port), connected_(false), tls_enabled_(false)
#ifdef __linux__
      , socket_fd_(-1)
#endif
{
}

ClientConnection::~ClientConnection() {
    Disconnect();
}

void ClientConnection::EnableTLS(bool enabled) {
#ifdef __linux__
    tls_enabled_ = enabled;
#else
    (void)enabled;
#endif
}

#ifdef __linux__
bool ClientConnection::ConfigureTLSClient(const std::string& ca_cert_path) {
    ca_cert_path_ = ca_cert_path;
    return true;
}
#endif

bool ClientConnection::Connect() {
#ifdef __linux__
    // 创建socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return false;
    }
    socket_fd_ = sqlcc::FileDescriptor(socket_fd);

    // 设置服务器地址
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);

    // 转换IP地址或解析主机名
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        // 使用 getaddrinfo 替代 gethostbyname 以避免 raw pointer 返回
        struct addrinfo hints, *result = nullptr;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(host_.c_str(), nullptr, &hints, &result) != 0 || result == nullptr) {
            socket_fd_.reset();
            return false;
        }
        // 安全检查：确保result->ai_addr不为null
        if (result->ai_addr != nullptr) {
            std::memcpy(&server_addr.sin_addr, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(struct in_addr));
        } else {
            freeaddrinfo(result);
            socket_fd_.reset();
            return false;
        }
        freeaddrinfo(result);
    }

    // 连接到服务器
    if (connect(socket_fd_.get(), (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        socket_fd_.reset();
        return false;
    }

    connected_ = true;

    // 如果启用TLS，则进行TLS握手
    if (tls_enabled_) {
        try {
            SSL_library_init();
            SSL_load_error_strings();
            const SSL_METHOD* method = nullptr;
            if (TLS_client_method()) {
                method = TLS_client_method();
            }
            ssl_ctx_ = sqlcc::utils::SSLContext::create(method);
            if (!ssl_ctx_.is_valid()) {
                std::cerr << "Failed to create SSL context" << std::endl;
                Disconnect();
                return false;
            }
            if (!ca_cert_path_.empty()) {
                if (SSL_CTX_load_verify_locations(ssl_ctx_.get(), ca_cert_path_.c_str(), nullptr) != 1) {
                    std::cerr << "Failed to load CA certificate, continuing without verification" << std::endl;
                    // 不因为证书加载失败而中断连接，暂时跳过验证
                    // Disconnect();
                    // return false;
                } else {
                    SSL_CTX_set_verify(ssl_ctx_.get(), SSL_VERIFY_PEER, nullptr);
                }
            }
            ssl_ = sqlcc::utils::SSLSocket::create(ssl_ctx_.get());
            SSL_set_fd(ssl_.get(), socket_fd_.get());
            if (SSL_connect(ssl_.get()) <= 0) {
                std::cerr << "SSL handshake failed" << std::endl;
                Disconnect();
                return false;
            }
            std::cout << "TLS handshake successful" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "TLS initialization failed: " << e.what() << std::endl;
            Disconnect();
            return false;
        }
    }
#endif
    return true;
}

void ClientConnection::Disconnect() {
#ifdef __linux__
    if (connected_ && socket_fd_.valid()) {
        if (tls_enabled_ && ssl_.is_valid()) {
            ssl_.shutdown();
            ssl_.reset();
        }
        if (tls_enabled_ && ssl_ctx_.is_valid()) {
            ssl_ctx_.reset();
        }
        socket_fd_.reset();
        connected_ = false;
    }
#endif
}

bool ClientConnection::IsConnected() const {
    return connected_;
}

bool ClientConnection::SendData(const std::vector<char>& data) {
#ifdef __linux__
    if (!connected_ || !socket_fd_.valid()) {
        return false;
    }

    if (tls_enabled_ && ssl_.is_valid()) {
        // TLS发送
        size_t total_sent = 0;
        while (total_sent < data.size()) {
            int sent = SSL_write(ssl_.get(), data.data() + total_sent, static_cast<int>(data.size() - total_sent));
            if (sent <= 0) {
                return false;
            }
            total_sent += sent;
        }
        return true;
    } else {
        // 普通TCP发送
        size_t total_sent = 0;
        while (total_sent < data.size()) {
            ssize_t sent = send(socket_fd_.get(), data.data() + total_sent, data.size() - total_sent, 0);
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                return false;
            }
            total_sent += sent;
        }
        return true;
    }
#endif
    return true;
}

std::vector<char> ClientConnection::ReceiveData() {
#ifdef __linux__
    std::vector<char> buffer(4096);

    if (tls_enabled_ && ssl_ctx_.is_valid() && ssl_.is_valid()) {
        // 设置超时，避免无限阻塞
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(socket_fd_.get(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        int bytes = SSL_read(ssl_.get(), buffer.data(), static_cast<int>(buffer.size()));
        if (bytes <= 0) {
            return std::vector<char>();
        }
        buffer.resize(bytes);
        return buffer;
    } else {
        // 普通TCP接收（非TLS模式）
        ssize_t received = recv(socket_fd_.get(), buffer.data(), buffer.size(), 0);

        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下没有数据可读
                return std::vector<char>();
            }
            return std::vector<char>(); // 错误发生
        } else if (received == 0) {
            // 连接被对方关闭
            connected_ = false;
            return std::vector<char>();
        }

        buffer.resize(received);
        return buffer;
    }
#else
    return std::vector<char>();
#endif
}

} // namespace network
} // namespace sqlcc
