#include "network/network.h"
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
ClientConnection::ClientConnection(const std::string& host, int port)
    : host_(host), port_(port), connected_(false) {}

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
    socket_fd_ = sqlcc::FileDescriptor::create_socket(AF_INET, SOCK_STREAM, 0);
    if (!socket_fd_.valid()) {
        return false;
    }

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
        std::memcpy(&server_addr.sin_addr, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(struct in_addr));
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
        SSL_library_init();
        SSL_load_error_strings();
        const SSL_METHOD* method = TLS_client_method();
        ssl_ctx_ = sqlcc::utils::SSLContext::create(method);
        if (!ssl_ctx_.is_valid()) {
            Disconnect();
            return false;
        }
        if (!ca_cert_path_.empty()) {
            if (SSL_CTX_load_verify_locations(ssl_ctx_.get(), ca_cert_path_.c_str(), nullptr) != 1) {
                Disconnect();
                return false;
            }
            SSL_CTX_set_verify(ssl_ctx_.get(), SSL_VERIFY_PEER, nullptr);
        }
        ssl_ = sqlcc::utils::SSLSocket::create(ssl_ctx_.get());
        SSL_set_fd(ssl_.get(), socket_fd_.get());
        if (SSL_connect(ssl_.get()) <= 0) {
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
        size_t total_sent = 0;
        while (total_sent < data.size()) {
            int sent = SSL_write(ssl_.get(), data.data() + total_sent, static_cast<int>(data.size() - total_sent));
            if (sent <= 0) {
                return false;
            }
            total_sent += sent;
        }
        return true;
    }
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
#endif
    return true;
}

std::vector<char> ClientConnection::ReceiveData() {
#ifdef __linux__
    std::vector<char> buffer(4096);
    if (tls_enabled_ && ssl_.is_valid()) {
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
    }
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
#else
    return std::vector<char>();
#endif
}

// ClientNetworkManager实现
ClientNetworkManager::ClientNetworkManager(const std::string& host, int port)
    : connection_(std::make_unique<ClientConnection>(host, port)),
      session_manager_(std::make_shared<SessionManager>()) {}

ClientNetworkManager::~ClientNetworkManager() = default;

bool ClientNetworkManager::Connect() {
    return connection_->Connect();
}

void ClientNetworkManager::EnableTLS(bool enabled) {
    connection_->EnableTLS(enabled);
}

#ifdef __linux__
bool ClientNetworkManager::ConfigureTLSClient(const std::string& ca_cert_path) {
    return connection_->ConfigureTLSClient(ca_cert_path);
}
#endif

void ClientNetworkManager::Disconnect() {
    connection_->Disconnect();
}

bool ClientNetworkManager::IsConnected() const {
    return connection_->IsConnected();
}

bool ClientNetworkManager::SendRequest(const std::vector<char>& request) {
    // 在客户端侧，仅对消息体进行加密并追加HMAC
    if (IsAESEncryptionEnabled()) {
        // 添加输入大小检查，防止过大的数据导致内存分配失败
        if (request.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Request too large for encryption: " << request.size() << " bytes" << std::endl;
            return false;
        }
        
        // 检查请求大小是否足够包含消息头
        if (request.size() < sizeof(MessageHeader)) {
            std::cerr << "Request too small to contain message header: " << request.size() << " bytes" << std::endl;
            return false;
        }
        
        std::vector<char> msg = request;
        // 使用结构体绑定避免raw pointer
        auto& header_ref = *reinterpret_cast<MessageHeader*>(msg.data());
        std::span<char> msg_span(msg);
        std::span<char> body_span = msg_span.subspan(sizeof(MessageHeader));

        auto aes = GetAESEncryptor();
        std::vector<uint8_t> ct = aes->Encrypt(std::vector<uint8_t>(body_span.begin(), body_span.end()));
        
        // 检查加密结果大小
        if (ct.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Encrypted data too large: " << ct.size() << " bytes" << std::endl;
            return false;
        }
        
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes->GetKeyBytes(), ct);
        
        // 检查MAC大小
        if (mac.size() != 32) { // HMAC-SHA256应该总是32字节
            std::cerr << "Invalid MAC size: " << mac.size() << " bytes" << std::endl;
            return false;
        }
        
        // 检查新消息体大小是否会超出限制
        if (ct.size() + mac.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Combined encrypted data and MAC too large: " << (ct.size() + mac.size()) << " bytes" << std::endl;
            return false;
        }
        
        // 检查新消息体大小是否会超出vector的最大大小
        if (ct.size() + mac.size() > std::vector<char>().max_size()) {
            std::cerr << "Combined encrypted data and MAC would exceed max_size()" << std::endl;
            return false;
        }
        
        std::vector<char> new_body;
        // 预留空间以避免多次重新分配
        new_body.reserve(ct.size() + mac.size());
        new_body.insert(new_body.end(), ct.begin(), ct.end());
        new_body.insert(new_body.end(), mac.begin(), mac.end());
        
        // 检查新消息体大小
        if (new_body.size() != ct.size() + mac.size()) {
            std::cerr << "Failed to create new message body" << std::endl;
            return false;
        }
        
        header_ref.length = static_cast<uint32_t>(new_body.size());
        
        // 检查新消息大小是否会超出限制
        if (sizeof(MessageHeader) + new_body.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "New message too large: " << (sizeof(MessageHeader) + new_body.size()) << " bytes" << std::endl;
            return false;
        }
        
        // 检查新消息大小是否会超出vector的最大大小
        if (sizeof(MessageHeader) + new_body.size() > std::vector<char>().max_size()) {
            std::cerr << "New message would exceed max_size()" << std::endl;
            return false;
        }
        
        msg.resize(sizeof(MessageHeader) + new_body.size());
        std::memcpy(msg.data(), &header_ref, sizeof(MessageHeader));
        std::memcpy(msg.data() + sizeof(MessageHeader), new_body.data(), new_body.size());
        return connection_->SendData(msg);
    }
    return connection_->SendData(request);
}

std::vector<char> ClientNetworkManager::ReceiveResponse() {
    auto resp = connection_->ReceiveData();
    if (resp.size() < sizeof(MessageHeader)) return resp;

    // 使用结构体绑定避免raw pointer
    auto& header_ref = *reinterpret_cast<MessageHeader*>(resp.data());
    if (IsAESEncryptionEnabled() && header_ref.length >= 32) {
        const char* body_ptr = resp.data() + sizeof(MessageHeader);
        std::vector<uint8_t> ciphertext(body_ptr, body_ptr + header_ref.length - 32);
        std::vector<uint8_t> mac(body_ptr + header_ref.length - 32, body_ptr + header_ref.length);
        auto aes = GetAESEncryptor();
        if (!HMACSHA256::Verify(aes->GetKeyBytes(), ciphertext, mac)) {
            return resp; // 返回原始响应以便上层处理错误
        }
        std::vector<uint8_t> plaintext = aes->Decrypt(ciphertext);
        MessageHeader new_header = header_ref;
        new_header.length = static_cast<uint32_t>(plaintext.size());
        std::vector<char> out(sizeof(MessageHeader) + plaintext.size());
        std::memcpy(out.data(), &new_header, sizeof(MessageHeader));
        std::memcpy(out.data() + sizeof(MessageHeader), plaintext.data(), plaintext.size());
        return out;
    }
    return resp;
}

bool ClientNetworkManager::SendAuthMessage(const std::string& username, const std::string& password) {
    // 构造认证消息
    // 格式: [uint32_t username_len][uint32_t password_len][username][password]
    uint32_t username_len = static_cast<uint32_t>(username.length());
    uint32_t password_len = static_cast<uint32_t>(password.length());

    size_t body_size = 2 * sizeof(uint32_t) + username_len + password_len;
    std::vector<char> message(sizeof(MessageHeader) + body_size);

    // 填充消息头 - 使用结构体绑定避免raw pointer
    auto& header_ref = *reinterpret_cast<MessageHeader*>(message.data());
    header_ref.magic = 0x53514C43; // 'SQLC'
    header_ref.length = static_cast<uint32_t>(body_size);
    header_ref.type = AUTH;
    header_ref.flags = 0;
    header_ref.sequence_id = 1;

    // 填充消息体 - 使用传统方式避免raw pointer运算
    char* body_start = message.data() + sizeof(MessageHeader);
    *reinterpret_cast<uint32_t*>(body_start) = username_len;
    *reinterpret_cast<uint32_t*>(body_start + sizeof(uint32_t)) = password_len;
    std::memcpy(body_start + 2 * sizeof(uint32_t), username.c_str(), username_len);
    std::memcpy(body_start + 2 * sizeof(uint32_t) + username_len, password.c_str(), password_len);

    return SendRequest(message);
}

bool ClientNetworkManager::InitiateKeyExchange() {
    // 发送密钥交换请求
    MessageHeader header;
    header.magic = 0x53514C43; // 'SQLC'
    header.length = 0;
    header.type = KEY_EXCHANGE;
    header.flags = 0;
    header.sequence_id = 2;
    
    std::vector<char> message(sizeof(MessageHeader));
    std::memcpy(message.data(), &header, sizeof(MessageHeader));
    
    std::cout << "Sending KEY_EXCHANGE request" << std::endl;
    if (!SendRequest(message)) {
        std::cerr << "Failed to send KEY_EXCHANGE" << std::endl;
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 接收密钥交换响应
    std::vector<char> response = ReceiveResponse();
    std::cout << "KEY_EXCHANGE response size: " << response.size() << std::endl;
    if (response.size() < sizeof(MessageHeader)) {
        std::cerr << "Invalid KEY_EXCHANGE response size" << std::endl;
        return false;
    }
    
    // 使用结构体绑定避免raw pointer
    auto& resp_header_ref = *reinterpret_cast<MessageHeader*>(response.data());
    
    // 检查是否是密钥交换确认
    if (resp_header_ref.type != KEY_EXCHANGE_ACK) {
        std::cerr << "Unexpected response type: " << resp_header_ref.type << " (expected " << KEY_EXCHANGE_ACK << ")" << std::endl;
        return false;
    }

    // 提取IV（密钥交换确认消息的体部）
    if (resp_header_ref.length < 16) {
        std::cerr << "Invalid IV length: " << resp_header_ref.length << std::endl;
        return false;
    }
    
    const char* iv_data = response.data() + sizeof(MessageHeader);
    std::vector<uint8_t> iv(iv_data, iv_data + resp_header_ref.length);
    
    try {
        // 带有客户端生成的残余串阮盆密钥
        auto encryption_key = std::make_shared<network::EncryptionKey>(
            network::EncryptionKey::GenerateRandom(32, 16)->GetKey(),  // 生成残余串阮盆密钥
            iv  // 使用服务器发送的IV
        );
        
        aes_encryptor_ = std::make_shared<network::AESEncryptor>(encryption_key);
        std::cout << "KEY_EXCHANGE successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize AES encryptor: " << e.what() << std::endl;
        return false;
    }
}

void ClientNetworkManager::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
}

std::shared_ptr<class AESEncryptor> ClientNetworkManager::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool ClientNetworkManager::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr;
}

std::vector<char> ClientNetworkManager::EncryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;
    }
    
    // 添加输入大小检查，防止过大的数据导致内存分配失败
    std::cerr << "EncryptMessage called with message.size()=" << message.size() << std::endl;
    if (message.size() > 1024 * 1024) { // 限制为1MB
        std::cerr << "Message too large for encryption: " << message.size() << " bytes" << std::endl;
        return message;
    }
    
    try {
        std::cerr << "Converting message to uint8_t vector" << std::endl;
        std::vector<uint8_t> data(message.begin(), message.end());
        std::cerr << "data.size()=" << data.size() << std::endl;
        
        // 检查转换后的数据大小
        if (data.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Converted data too large: " << data.size() << " bytes" << std::endl;
            return message;
        }
        
        std::cerr << "Calling aes_encryptor_->Encrypt" << std::endl;
        std::vector<uint8_t> ciphertext = aes_encryptor_->Encrypt(data);
        std::cerr << "Encryption succeeded, ciphertext.size()=" << ciphertext.size() << std::endl;
        
        // 检查加密结果大小
        if (ciphertext.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Encrypted data too large: " << ciphertext.size() << " bytes" << std::endl;
            return message;
        }
        
        std::cerr << "Calling HMACSHA256::Compute" << std::endl;
        // 计算HMAC-SHA256并追加
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes_encryptor_->GetKeyBytes(), ciphertext);
        std::cerr << "HMAC computation succeeded, mac.size()=" << mac.size() << std::endl;
        
        // 检查MAC大小
        if (mac.size() != 32) { // HMAC-SHA256应该总是32字节
            std::cerr << "Invalid MAC size: " << mac.size() << " bytes" << std::endl;
            return message;
        }
        
        // 检查最终结果大小
        if (ciphertext.size() + mac.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Combined encrypted data and MAC too large: " << (ciphertext.size() + mac.size()) << " bytes" << std::endl;
            return message;
        }
        
        std::cerr << "Creating output vector" << std::endl;
        // 检查输出向量大小是否会超出限制
        if (ciphertext.size() + mac.size() > std::vector<char>().max_size()) {
            std::cerr << "Output vector size would exceed max_size()" << std::endl;
            return message;
        }
        
        std::vector<char> out;
        // 预留空间以避免多次重新分配
        out.reserve(ciphertext.size() + mac.size());
        out.insert(out.end(), ciphertext.begin(), ciphertext.end());
        std::cerr << "out.size() after copying ciphertext=" << out.size() << std::endl;
        
        std::cerr << "Inserting MAC into output vector" << std::endl;
        out.insert(out.end(), mac.begin(), mac.end());
        std::cerr << "out.size() after inserting MAC=" << out.size() << std::endl;
        
        return out;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed during encryption: " << e.what() << std::endl;
        return message;
    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << std::endl;
        return message;
    } catch (...) {
        std::cerr << "Unknown error during encryption" << std::endl;
        return message;
    }
}

std::vector<char> ClientNetworkManager::DecryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;
    }
    
    std::cerr << "DecryptMessage called with message.size()=" << message.size() << std::endl;
    
    // 添加输入大小检查
    if (message.size() > 1024 * 1024) { // 限制为1MB
        std::cerr << "Message too large for decryption: " << message.size() << " bytes" << std::endl;
        return message;
    }
    
    try {
        if (message.size() < 32) { // 至少需要32字节的MAC
            std::cerr << "Message too small for decryption: " << message.size() << " bytes" << std::endl;
            return message;
        }
        
        std::cerr << "Splitting MAC and ciphertext" << std::endl;
        // 分离MAC
        std::vector<uint8_t> mac(message.end() - 32, message.end());
        std::vector<uint8_t> ciphertext(message.begin(), message.end() - 32);
        std::cerr << "mac.size()=" << mac.size() << ", ciphertext.size()=" << ciphertext.size() << std::endl;
        
        // 检查分离后的数据大小
        if (ciphertext.size() > 1024 * 1024 || mac.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Separated data too large for decryption" << std::endl;
            return message;
        }
        
        std::cerr << "Verifying HMAC" << std::endl;
        // 验证HMAC
        if (!HMACSHA256::Verify(aes_encryptor_->GetKeyBytes(), ciphertext, mac)) {
            std::cerr << "HMAC verification failed" << std::endl;
            return message;
        }
        
        std::cerr << "Decrypting ciphertext" << std::endl;
        std::vector<uint8_t> plaintext = aes_encryptor_->Decrypt(ciphertext);
        std::cerr << "Decryption succeeded, plaintext.size()=" << plaintext.size() << std::endl;
        
        // 检查解密后的数据大小
        if (plaintext.size() > 1024 * 1024) { // 限制为1MB
            std::cerr << "Decrypted data too large: " << plaintext.size() << " bytes" << std::endl;
            return message;
        }
        
        std::cerr << "Creating output vector" << std::endl;
        // 检查输出向量大小是否会超出限制
        if (plaintext.size() > std::vector<char>().max_size()) {
            std::cerr << "Output vector size would exceed max_size()" << std::endl;
            return message;
        }
        
        return std::vector<char>(plaintext.begin(), plaintext.end());
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed during decryption: " << e.what() << std::endl;
        return message;
    } catch (const std::exception& e) {
        std::cerr << "Decryption failed: " << e.what() << std::endl;
        return message;
    } catch (...) {
        std::cerr << "Unknown error during decryption" << std::endl;
        return message;
    }
}// ConnectionHandler实现
ConnectionHandler::ConnectionHandler(sqlcc::FileDescriptor&& fd, std::shared_ptr<SessionManager> session_manager, std::shared_ptr<sqlcc::SqlExecutor> sql_executor)
    : fd_(std::move(fd)), session_manager_(std::move(session_manager)), sql_executor_(std::move(sql_executor)),
      session_(nullptr), closed_(false)
#ifdef __linux__
      , tls_enabled_(false)
#endif
{
#ifdef __linux__
    // 设置为非阻塞模式
    int flags = fcntl(fd_.get(), F_GETFL, 0);
    fcntl(fd_.get(), F_SETFL, flags | O_NONBLOCK);
#endif
}

ConnectionHandler::~ConnectionHandler() = default;

void ConnectionHandler::SetTLS(struct ssl_st* ssl, bool enabled) {
#ifdef __linux__
    if (enabled && ssl) {
        ssl_.reset(ssl);
        tls_enabled_ = enabled;
    } else {
        ssl_.reset();
        tls_enabled_ = false;
    }
#else
    (void)ssl;
    (void)enabled;
#endif
}

int ConnectionHandler::GetFd() const {
    return fd_.get();
}

bool ConnectionHandler::IsClosed() const {
    return closed_;
}

void ConnectionHandler::HandleEvent(uint32_t events) {
#ifdef __linux__
    if (events & EPOLLIN) {
        HandleRead();
    }
    if (events & EPOLLOUT) {
        HandleWrite();
    }
    if (events & (EPOLLERR | EPOLLHUP)) {
        Close();
    }
#endif
}

void ConnectionHandler::HandleRead() {
#ifdef __linux__
    // 读取数据并处理
    std::vector<char> buffer(4096);
    ssize_t bytes_read = 0;
    if (tls_enabled_ && ssl_.is_valid()) {
        bytes_read = SSL_read(ssl_.get(), buffer.data(), static_cast<int>(buffer.size()));
    } else {
        bytes_read = recv(fd_.get(), buffer.data(), buffer.size(), 0);
    }
    
    if (bytes_read > 0) {
        buffer.resize(bytes_read);
        ProcessMessage(buffer);
    } else if (bytes_read == 0) {
        // 客户端关闭连接
        Close();
    } else {
        // 错误发生
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Close();
        }
    }
#endif
}

void ConnectionHandler::HandleWrite() {
#ifdef __linux__
    // 处理写事件（如果有待发送的数据）
    std::lock_guard<std::mutex> lock(write_mutex_);
    while (!write_queue_.empty()) {
        const std::vector<char>& data = write_queue_.front();
        ssize_t bytes_sent = 0;
        if (tls_enabled_ && ssl_.is_valid()) {
            bytes_sent = SSL_write(ssl_.get(), data.data(), static_cast<int>(data.size()));
        } else {
            bytes_sent = send(fd_.get(), data.data(), data.size(), 0);
        }
        
        if (bytes_sent > 0) {
            write_queue_.pop();
        } else if (bytes_sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 注册 EPOLLOUT 事件以在可写时继续发送
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.ptr = this;
                // 在 ServerNetworkManager 中需要 epoll_fd，这里暂时不修改，等待后续优化
                break;
            } else {
                Close();
                break;
            }
        }
    }
#endif
}

void ConnectionHandler::SendMessage(const std::vector<char>& message) {
#ifdef __linux__
    std::vector<char> to_send = message;
    // 如果AES已启用，则仅对消息体进行加密并追加HMAC（除 KEY_EXCHANGE_ACK 外）
    MessageHeader* msg_header = reinterpret_cast<MessageHeader*>(to_send.data());
    if (session_ && session_->IsAESEncryptionEnabled() && msg_header->type != KEY_EXCHANGE_ACK) {
        MessageHeader* header = reinterpret_cast<MessageHeader*>(to_send.data());
        std::vector<char> body(to_send.begin() + sizeof(MessageHeader), to_send.end());
        // 加密体并追加MAC
        auto aes = session_->GetAESEncryptor();
        std::vector<uint8_t> ct = aes->Encrypt(std::vector<uint8_t>(body.begin(), body.end()));
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes->GetKeyBytes(), ct);
        std::vector<char> new_body(ct.begin(), ct.end());
        new_body.insert(new_body.end(), mac.begin(), mac.end());
        header->length = static_cast<uint32_t>(new_body.size());
        to_send.resize(sizeof(MessageHeader) + new_body.size());
        std::memcpy(to_send.data(), header, sizeof(MessageHeader));
        std::memcpy(to_send.data() + sizeof(MessageHeader), new_body.data(), new_body.size());
    }

    // 对于重要的消息（查询结果、错误），尝试同步发送
    bool is_important = (msg_header->type == QUERY_RESULT || msg_header->type == ERROR ||
                        msg_header->type == CONN_ACK || msg_header->type == AUTH_ACK);

    if (is_important && TrySendImmediately(to_send)) {
        // 成功同步发送，无需排队
        return;
    }

    // 否则排队异步发送
    std::lock_guard<std::mutex> lock(write_mutex_);
    bool queue_was_empty = write_queue_.empty();
    write_queue_.push(to_send);

    // 如果队列之前为空，尝试立即发送，否则等待 EPOLLOUT 事件
    if (queue_was_empty) {
        // 释放锁后调用 HandleWrite 避免死锁
        write_mutex_.unlock();
        HandleWrite();
        write_mutex_.lock();
    }
#endif
}

bool ConnectionHandler::TrySendImmediately(const std::vector<char>& data) {
#ifdef __linux__
    if (closed_ || !fd_.valid()) {
        return false;
    }

    // 设置为阻塞模式进行同步发送
    int original_flags = fcntl(fd_.get(), F_GETFL, 0);
    fcntl(fd_.get(), F_SETFL, original_flags & ~O_NONBLOCK);

    ssize_t bytes_sent = 0;
    if (tls_enabled_ && ssl_.is_valid()) {
        bytes_sent = SSL_write(ssl_.get(), data.data(), static_cast<int>(data.size()));
    } else {
        bytes_sent = send(fd_.get(), data.data(), data.size(), MSG_NOSIGNAL);
    }

    // 恢复非阻塞模式
    fcntl(fd_.get(), F_SETFL, original_flags);

    if (bytes_sent == static_cast<ssize_t>(data.size())) {
        return true;  // 成功发送
    } else if (bytes_sent < 0) {
        if (errno == EPIPE || errno == ECONNRESET) {
            Close();  // 连接已断开
        }
        return false;
    } else {
        // 部分发送，不应该发生，因为我们设置为阻塞模式
        return false;
    }
#else
    return false;
#endif
}

void ConnectionHandler::Close() {
    if (!closed_) {
        closed_ = true;
#ifdef __linux__
        fd_.reset();
#endif
    }
}

void ConnectionHandler::ProcessMessage(const std::vector<char>& data) {
    std::cout << "[SERVER] ProcessMessage called, data size: " << data.size() << std::endl;

    // 处理接收到的消息
    if (data.empty() || data.size() < sizeof(MessageHeader)) {
        std::cout << "[SERVER] Message too small, ignoring" << std::endl;
        return;
    }

    // 使用常量引用避免const_cast
    const MessageHeader& header = *reinterpret_cast<const MessageHeader*>(data.data());
    std::cout << "[SERVER] Message header - magic: " << std::hex << header.magic
              << ", length: " << header.length
              << ", type: " << (int)header.type
              << ", flags: " << (int)header.flags
              << ", seq: " << header.sequence_id << std::dec << std::endl;

    if (header.magic != 0x53514C43) {
        std::cout << "[SERVER] Invalid magic number: " << std::hex << header.magic << std::dec << std::endl;
        return;
    }
    
    // 若启用AES，则尝试将消息体解密（除密钥交换外）
    std::vector<char> working = data;
    const MessageHeader* current_header = &header;
    
    if (session_ && session_->IsAESEncryptionEnabled() && current_header->type != KEY_EXCHANGE && current_header->length >= 32) {
        // 检查消息总长度是否足够
        if (working.size() < sizeof(MessageHeader) + current_header->length) {
            SendErrorMessage("Incomplete encrypted message");
            return;
        }
        
        const char* body_ptr = working.data() + sizeof(MessageHeader);
        
        // 确保消息长度足够包含MAC
        if (current_header->length < 32) {
            SendErrorMessage("Encrypted message too short for MAC verification");
            return;
        }
        
        std::vector<uint8_t> ciphertext(body_ptr, body_ptr + current_header->length - 32);
        std::vector<uint8_t> mac(body_ptr + current_header->length - 32, body_ptr + current_header->length);
        
        auto aes = session_->GetAESEncryptor();
        if (!aes) {
            SendErrorMessage("Encryption service not available");
            return;
        }
        
        if (!HMACSHA256::Verify(aes->GetKeyBytes(), ciphertext, mac)) {
            SendErrorMessage("HMAC verification failed");
            return;
        }
        
        try {
            std::vector<uint8_t> plaintext = aes->Decrypt(ciphertext);
            // 重建消息，将明文作为新体
            MessageHeader new_header = *current_header;
            new_header.length = static_cast<uint32_t>(plaintext.size());
            working.resize(sizeof(MessageHeader) + plaintext.size());
            std::memcpy(working.data(), &new_header, sizeof(MessageHeader));
            std::memcpy(working.data() + sizeof(MessageHeader), plaintext.data(), plaintext.size());
            current_header = reinterpret_cast<const MessageHeader*>(working.data());
        } catch (const std::exception& e) {
            SendErrorMessage(std::string("Decryption failed: ") + e.what());
            return;
        }
    }
    
    // 根据消息类型处理
    switch (current_header->type) {
        case CONNECT:
            HandleConnectMessage(working);
            break;
        case AUTH:
            HandleAuthMessage(working);
            break;
        case QUERY:
            HandleQueryMessage(working);
            break;
        case KEY_EXCHANGE:
            HandleKeyExchangeMessage(working);
            break;
        default:
            std::cout << "[SERVER] Unknown message type: " << (int)current_header->type << std::endl;
            break;
    }
}

void ConnectionHandler::HandleConnectMessage(const std::vector<char>& data) {
    std::cout << "[SERVER] Handling CONNECT message, data size: " << data.size() << std::endl;

    // 创建会话
    session_ = session_manager_->CreateSession();
    std::cout << "[SERVER] Created session" << std::endl;

    // 检查客户端连接消息中的标志
    uint32_t client_flags = 0;
    if (data.size() >= sizeof(MessageHeader)) {
        MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
        client_flags = header->flags;
        std::cout << "[SERVER] Client flags: " << client_flags << std::endl;

        // 如果客户端请求禁用加密，记录到会话中
        if (client_flags & 0x01) {
            session_->SetEncryptionDisabled(true);
        }

        // 如果客户端请求禁用认证，记录到会话中
        if (client_flags & 0x02) {
            session_->SetAuthenticationDisabled(true);
            // 自动通过认证
            session_->SetAuthenticated("anonymous");
        }
    }

    // 发送连接确认消息，包含相同的标志
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = 0;
    ack_header.type = CONN_ACK;
    ack_header.flags = client_flags; // 回显客户端的标志
    ack_header.sequence_id = 1;

    std::vector<char> ack_msg(sizeof(MessageHeader));
    std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
    std::cout << "[SERVER] Sending CONN_ACK message, type: " << (int)ack_header.type << std::endl;
    SendMessage(ack_msg);
    std::cout << "[SERVER] CONN_ACK message sent" << std::endl;
}

void ConnectionHandler::HandleAuthMessage(const std::vector<char>& data) {
    if (data.size() < sizeof(MessageHeader)) {
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    if (data.size() < sizeof(MessageHeader) + header->length) {
        return;
    }

    // 解析新的认证数据格式
    // 格式: [uint32_t username_len][uint32_t password_len][username][password]
    const char* body = data.data() + sizeof(MessageHeader);
    if (header->length < 2 * sizeof(uint32_t)) {
        return;
    }

    uint32_t username_len = *reinterpret_cast<const uint32_t*>(body);
    uint32_t password_len = *reinterpret_cast<const uint32_t*>(body + sizeof(uint32_t));
    
    if (header->length != 2 * sizeof(uint32_t) + username_len + password_len) {
        return;
    }

    std::string username(body + 2 * sizeof(uint32_t), username_len);
    std::string password(body + 2 * sizeof(uint32_t) + username_len, password_len);

    bool authenticated = session_manager_->Authenticate(session_->GetSessionId(), username, password);
    
    // 发送认证确认消息
    MessageHeader ack_header;
    ack_header.magic = 0x53514C43; // 'SQLC'
    ack_header.length = 0;
    ack_header.type = AUTH_ACK;
    ack_header.flags = authenticated ? 0 : 1; // 使用flags表示认证结果
    ack_header.sequence_id = header->sequence_id;

    std::vector<char> ack_msg(sizeof(MessageHeader));
    std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
    SendMessage(ack_msg);
}

void ConnectionHandler::HandleQueryMessage(const std::vector<char>& data) {
    if (!session_) {
        // 会话不存在
        SendErrorMessage("Session not found");
        return;
    }

    // 检查是否需要认证（只有在未禁用认证的情况下才要求认证）
    if (!session_->IsAuthenticationDisabled() && !session_->IsAuthenticated()) {
        // 未禁用认证但用户未认证，拒绝请求
        SendErrorMessage("Not authenticated");
        return;
    }

    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));

    // 确保有足够的数据
    if (data.size() < sizeof(MessageHeader) + header->length) {
        SendErrorMessage("Invalid query message");
        return;
    }

    // 获取查询语句
    std::string query(data.data() + sizeof(MessageHeader), header->length);

    // 执行SQL查询 - 使用实际的SQL执行器而不是回显
    std::string result;
    try {
        result = sql_executor_->Execute(query);
        // 检查结果是否以"Error:"开头来判断执行是否成功
        bool success = (result.find("Error:") != 0);
        if (!success) {
            // 如果执行失败，从错误信息中提取实际错误
            result = result.substr(6); // 去掉"Error:"前缀
        }

        // 构造查询结果消息
        MessageHeader result_header;
        result_header.magic = 0x53514C43; // 'SQLC'
        result_header.length = static_cast<uint32_t>(result.length());
        result_header.type = QUERY_RESULT;
        result_header.flags = success ? 0 : 1; // 使用flags表示执行结果
        result_header.sequence_id = header->sequence_id;

        std::vector<char> result_msg(sizeof(MessageHeader) + result.length());
        std::memcpy(result_msg.data(), &result_header, sizeof(MessageHeader));
        std::memcpy(result_msg.data() + sizeof(MessageHeader), result.c_str(), result.length());
        SendMessage(result_msg);
    } catch (const std::exception& e) {
        // 处理执行过程中的异常
        SendErrorMessage(std::string("Query execution failed: ") + e.what());
    }
}

void ConnectionHandler::HandleKeyExchangeMessage(const std::vector<char>& data) {
    // 处理密钥交换消息
    MessageHeader* header = reinterpret_cast<MessageHeader*>(const_cast<char*>(data.data()));
    
    // 检查是否有session
    if (!session_) {
        SendErrorMessage("Session not found");
        return;
    }
    
    try {
        // 生成AES-256密钥和IV
        auto encryption_key = network::EncryptionKey::GenerateRandom(32, 16); // AES-256 = 32字节
        auto aes_encryptor = std::make_shared<network::AESEncryptor>(encryption_key);
        
        // 将AES加密器设置到session中
        session_->SetAESEncryptor(aes_encryptor);
        
        // 发送密钥交换确认消息，含有了IV
        std::string ack_data(reinterpret_cast<const char*>(encryption_key->GetIV().data()), 
                            encryption_key->GetIV().size());
        
        MessageHeader ack_header;
        ack_header.magic = 0x53514C43; // 'SQLC'
        ack_header.length = ack_data.length();
        ack_header.type = KEY_EXCHANGE_ACK;
        ack_header.flags = 0x01; // 使用flag表示已含有AES加密
        ack_header.sequence_id = header->sequence_id;

        std::vector<char> ack_msg(sizeof(MessageHeader) + ack_data.length());
        std::memcpy(ack_msg.data(), &ack_header, sizeof(MessageHeader));
        std::memcpy(ack_msg.data() + sizeof(MessageHeader), ack_data.c_str(), ack_data.length());
        SendMessage(ack_msg);
    } catch (const std::exception& e) {
        SendErrorMessage(std::string("Key exchange failed: ") + e.what());
    }
}

void ConnectionHandler::SendErrorMessage(const std::string& error) {
    MessageHeader error_header;
    error_header.magic = 0x53514C43; // 'SQLC'
    error_header.length = error.length();
    error_header.type = ERROR;
    error_header.flags = 0;
    error_header.sequence_id = 0;

    std::vector<char> error_msg(sizeof(MessageHeader) + error.length());
    std::memcpy(error_msg.data(), &error_header, sizeof(MessageHeader));
    std::memcpy(error_msg.data() + sizeof(MessageHeader), error.c_str(), error.length());
    SendMessage(error_msg);
}

std::vector<char> ConnectionHandler::EncryptMessage(const std::vector<char>& message) {
    if (!session_ || !session_->IsAESEncryptionEnabled()) {
        return message;
    }
    try {
        auto aes_encryptor_ = session_->GetAESEncryptor();
        if (!aes_encryptor_) {
            return message;
        }
        std::vector<uint8_t> data(message.begin(), message.end());
        std::vector<uint8_t> ciphertext = aes_encryptor_->Encrypt(data);
        std::vector<uint8_t> mac = HMACSHA256::Compute(aes_encryptor_->GetKeyBytes(), ciphertext);
        std::vector<char> out(ciphertext.begin(), ciphertext.end());
        out.insert(out.end(), mac.begin(), mac.end());
        return out;
    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << std::endl;
        return message;
    }
}

std::vector<char> ConnectionHandler::DecryptMessage(const std::vector<char>& message) {
    if (!session_ || !session_->IsAESEncryptionEnabled()) {
        return message;
    }
    try {
        auto aes_encryptor_ = session_->GetAESEncryptor();
        if (!aes_encryptor_) {
            return message;
        }
        if (message.size() < 32) {
            return message;
        }
        std::vector<uint8_t> mac(message.end() - 32, message.end());
        std::vector<uint8_t> ciphertext(message.begin(), message.end() - 32);
        if (!HMACSHA256::Verify(aes_encryptor_->GetKeyBytes(), ciphertext, mac)) {
            std::cerr << "HMAC verification failed" << std::endl;
            return message;
        }
        std::vector<uint8_t> plaintext = aes_encryptor_->Decrypt(ciphertext);
        return std::vector<char>(plaintext.begin(), plaintext.end());
    } catch (const std::exception& e) {
        std::cerr << "Decryption failed: " << e.what() << std::endl;
        return message;
    }
}

// MessageProcessor实现
MessageProcessor::MessageProcessor(std::shared_ptr<SessionManager> session_manager)
    : session_manager_(std::move(session_manager)) {}

// ServerNetworkManager实现
ServerNetworkManager::ServerNetworkManager(int port, int max_connections)
    : port_(port), max_connections_(max_connections), running_(false),
      session_manager_(std::make_shared<SessionManager>()) {}

ServerNetworkManager::~ServerNetworkManager() {
    Stop();
}

bool ServerNetworkManager::Start() {
#ifdef __linux__
    // 创建监听socket
    listen_fd_ = sqlcc::FileDescriptor::create_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (!listen_fd_.valid()) {
        return false;
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(listen_fd_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        listen_fd_.reset();
        return false;
    }

    // 绑定地址
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_.get(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        listen_fd_.reset();
        return false;
    }

    // 开始监听
    if (listen(listen_fd_.get(), SOMAXCONN) < 0) {
        listen_fd_.reset();
        return false;
    }

    // 创建epoll实例
    epoll_fd_ = sqlcc::FileDescriptor::create_epoll(0);
    if (!epoll_fd_.valid()) {
        listen_fd_.reset();
        return false;
    }

    // 添加监听socket到epoll
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev)); // 初始化事件结构
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, listen_fd_.get(), &ev) < 0) {
        epoll_fd_.reset();
        listen_fd_.reset();
        return false;
    }

    running_ = true;
    return true;
#else
    return false; // 非Linux平台不支持
#endif
}

void ServerNetworkManager::Stop() {
    running_ = false;
    
#ifdef __linux__
    epoll_fd_.reset();
    listen_fd_.reset();
#endif
}

void ServerNetworkManager::ProcessEvents() {
#ifdef __linux__
    if (!running_ || !epoll_fd_.valid()) {
        return;
    }

    struct epoll_event events[64];
    int nfds = epoll_wait(epoll_fd_.get(), events, 64, 0);

    if (nfds > 0) {
        std::cout << "[SERVER] Processing " << nfds << " events" << std::endl;
    }

    for (int i = 0; i < nfds; i++) {
        if (events[i].data.ptr == nullptr) {
            // 监听socket有事件，接受新连接
            std::cout << "[SERVER] Accepting new connection" << std::endl;
            AcceptConnection();
        } else {
            // 客户端连接有事件
            std::cout << "[SERVER] Handling client event on fd" << std::endl;
            ConnectionHandler* handler = static_cast<ConnectionHandler*>(events[i].data.ptr);
            handler->HandleEvent(events[i].events);

            if (handler->IsClosed()) {
                // 从epoll中移除并删除连接处理器
                int fd = handler->GetFd();
                epoll_ctl(epoll_fd_.get(), EPOLL_CTL_DEL, fd, nullptr);
                // 从智能指针容器中移除并自动释放资源
                connections_.erase(fd);
            }
        }
    }
#endif
}

void ServerNetworkManager::SetSqlExecutor(std::shared_ptr<sqlcc::SqlExecutor> sql_executor) {
    sql_executor_ = std::move(sql_executor);
}

void ServerNetworkManager::EnableTLS(bool enabled) {
#ifdef __linux__
    tls_enabled_ = enabled;
#else
    (void)enabled;
#endif
}

bool ServerNetworkManager::ConfigureTLSServer(const std::string& cert_path,
                                              const std::string& key_path,
                                              const std::string& ca_cert_path) {
#ifdef __linux__
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_server_method();
    ssl_ctx_ = sqlcc::utils::SSLContext::create(method);
    if (!ssl_ctx_.is_valid()) return false;
    if (SSL_CTX_use_certificate_file(ssl_ctx_.get(), cert_path.c_str(), SSL_FILETYPE_PEM) != 1) return false;
    if (SSL_CTX_use_PrivateKey_file(ssl_ctx_.get(), key_path.c_str(), SSL_FILETYPE_PEM) != 1) return false;
    // 服务端不强制校验客户端证书，避免握手失败
    if (!ca_cert_path.empty()) {
        // 可选地加载CA以支持链验证，但不设置SSL_VERIFY_PEER
        SSL_CTX_load_verify_locations(ssl_ctx_.get(), ca_cert_path.c_str(), nullptr);
    }
    return true;
#else
    (void)cert_path; (void)key_path; (void)ca_cert_path; return false;
#endif
}

void ServerNetworkManager::AcceptConnection() {
#ifdef __linux__
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // 使用FileDescriptor的accept静态方法创建RAII文件描述符
    auto client_fd = sqlcc::FileDescriptor::accept(listen_fd_.get(),
                                                         (struct sockaddr*)&client_addr, 
                                                         &client_len, SOCK_NONBLOCK);
    if (!client_fd.valid()) {
        return;
    }

    // 创建连接处理器，传入SQL执行器
    auto handler = std::make_unique<ConnectionHandler>(std::move(client_fd), session_manager_, sql_executor_);
    int fd = handler->GetFd(); // 获取文件描述符值用于epoll

    // 若启用TLS，在该连接上进行握手
    if (tls_enabled_ && ssl_ctx_.is_valid()) {
        auto ssl = sqlcc::utils::SSLSocket::create(ssl_ctx_.get());
        if (!ssl.is_valid()) {
            return;
        }
        SSL_set_fd(ssl.get(), fd);
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        int ret = SSL_accept(ssl.get());
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        if (ret <= 0) {
            return;
        }
        handler->SetTLS(ssl.release(), true);
    }
    
    // 添加到epoll（水平触发以简化处理）
    struct epoll_event ev;
    ev.events = EPOLLIN; // 水平触发
    ev.data.ptr = handler.get();
    if (epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, fd, &ev) < 0) {
        return;
    }

    // 添加到连接映射，使用智能指针管理ConnectionHandler对象
    connections_[fd] = std::move(handler);
#endif
}

} // namespace network
} // namespace sqlcc