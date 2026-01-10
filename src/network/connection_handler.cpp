#include "network/connection_handler.h"
#include "network/message_serializer.h"
#include "network/session_manager.h"
#include "network/session.h"
#include "utils/file_descriptor.h"
#include "utils/logger.h"
#include "sql_executor.h"
#include "core/user_manager.h"
#include "core/permission_validator.h"
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <random>

#ifdef __linux__
#include <sys/epoll.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

namespace sqlcc {
namespace network {

// ConnectionHandler实现
ConnectionHandler::ConnectionHandler(sqlcc::FileDescriptor&& fd,
                                   std::shared_ptr<SessionManager> session_manager,
                                   std::shared_ptr<sqlcc::SqlExecutor> sql_executor,
                                   std::shared_ptr<sqlcc::UserManager> user_manager)
    : fd_(std::move(fd)),
      session_manager_(session_manager),
      sql_executor_(sql_executor),
      user_manager_(user_manager),
      permission_validator_(std::make_shared<sqlcc::PermissionValidator>(user_manager)),
      closed_(false) {
    // 初始化写队列互斥锁
    // TLS相关初始化
#ifdef __linux__
    ssl_ = nullptr;
    tls_enabled_ = false;
#endif

    // AES加密器初始化（暂时为nullptr，需要密钥交换后设置）
    aes_encryptor_ = nullptr;

    SQLCC_LOG_INFO("Connection handler created for fd: " + std::to_string(GetFd()));
}

ConnectionHandler::~ConnectionHandler() {
    Close();
    SQLCC_LOG_INFO("Connection handler destroyed for fd: " + std::to_string(GetFd()));
}

void ConnectionHandler::SetTLS(SSL* ssl, bool enabled) {
#ifdef __linux__
    ssl_ = ssl;
    tls_enabled_ = enabled;
    if (enabled && ssl_) {
        SQLCC_LOG_INFO("TLS enabled for connection fd: " + std::to_string(GetFd()));
    }
#endif
}

void ConnectionHandler::SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
    if (encryptor) {
        SQLCC_LOG_INFO("AES encryptor set for connection fd: " + std::to_string(GetFd()));
    } else {
        SQLCC_LOG_INFO("AES encryptor cleared for connection fd: " + std::to_string(GetFd()));
    }
}

int ConnectionHandler::GetFd() const {
    return fd_.get();
}

bool ConnectionHandler::IsClosed() const {
    return closed_;
}

void ConnectionHandler::HandleEvent(uint32_t events) {
    try {
        if (events & EPOLLIN) {
            HandleRead();
        }
        if (events & EPOLLOUT) {
            HandleWrite();
        }
        if (events & (EPOLLERR | EPOLLHUP)) {
            SQLCC_LOG_ERROR("Connection error or hangup on fd: " + std::to_string(GetFd()));
            Close();
        }
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Exception in HandleEvent: " + std::string(e.what()));
        Close();
    }
}

void ConnectionHandler::SendMessage(const std::vector<char>& message) {
    if (closed_) {
        SQLCC_LOG_WARN("Attempted to send message on closed connection fd: " + std::to_string(GetFd()));
        return;
    }

    // 加密消息（如果启用了加密）
    std::vector<char> encrypted_message = EncryptMessage(message);

    // 添加到写队列
    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.push(encrypted_message);
    }

    // 尝试立即发送
    TrySendImmediately(encrypted_message);
}

std::vector<char> ConnectionHandler::EncryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;  // 未设置加密器，返回原文
    }

    try {
        // 将char向量转换为uint8_t向量进行加密
        std::vector<uint8_t> input_data(message.begin(), message.end());
        std::vector<uint8_t> encrypted_data = aes_encryptor_->Encrypt(input_data);

        // 转换回char向量返回
        return std::vector<char>(encrypted_data.begin(), encrypted_data.end());
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("AES encryption failed: " + std::string(e.what()));
        return message;  // 加密失败，返回原文
    }
}

std::vector<char> ConnectionHandler::DecryptMessage(const std::vector<char>& message) {
    if (!aes_encryptor_) {
        return message;  // 未设置加密器，返回原文
    }

    try {
        // 将char向量转换为uint8_t向量进行解密
        std::vector<uint8_t> input_data(message.begin(), message.end());
        std::vector<uint8_t> decrypted_data = aes_encryptor_->Decrypt(input_data);

        // 转换回char向量返回
        return std::vector<char>(decrypted_data.begin(), decrypted_data.end());
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("AES decryption failed: " + std::string(e.what()));
        return message;  // 解密失败，返回原文
    }
}

void ConnectionHandler::HandleRead() {
    // 读取数据
    std::vector<char> buffer(4096);

#ifdef __linux__
    ssize_t bytes_read = 0;

    // 如果TLS启用但握手还未完成，先尝试完成握手
    if (tls_enabled_ && ssl_ && !SSL_is_init_finished(ssl_)) {
        int ssl_result = SSL_accept(ssl_);
        if (ssl_result == 1) {
            // 握手成功
            SQLCC_LOG_INFO("SSL handshake completed for connection fd: " + std::to_string(GetFd()));
        } else {
            int ssl_error = SSL_get_error(ssl_, ssl_result);
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                // 握手仍在进行中，等待更多数据
                return;
            } else {
                // 握手失败
                SQLCC_LOG_ERROR("SSL handshake failed on fd: " + std::to_string(GetFd()));
                Close();
                return;
            }
        }
    }

    if (tls_enabled_ && ssl_) {
        // TLS读取
        bytes_read = SSL_read(ssl_, buffer.data(), buffer.size());
        if (bytes_read <= 0) {
            int ssl_error = SSL_get_error(ssl_, bytes_read);
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                return;  // 非阻塞，需要重试
            } else {
                SQLCC_LOG_ERROR("TLS read error on fd: " + std::to_string(GetFd()));
                Close();
                return;
            }
        }
    } else {
        // 普通TCP读取
        bytes_read = read(GetFd(), buffer.data(), buffer.size());
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;  // 非阻塞，无数据
            } else {
                SQLCC_LOG_ERROR("Read error on fd: " + std::to_string(GetFd()) + ", errno: " + std::to_string(errno));
                Close();
                return;
            }
        } else if (bytes_read == 0) {
            // 连接被对方关闭
            SQLCC_LOG_INFO("Connection closed by peer, fd: " + std::to_string(GetFd()));
            Close();
            return;
        }
    }

    buffer.resize(bytes_read);

    // 解密数据（如果启用了加密）
    buffer = DecryptMessage(buffer);

    // 处理消息
    ProcessMessage(buffer);
#endif
}

void ConnectionHandler::HandleWrite() {
    std::lock_guard<std::mutex> lock(write_mutex_);

    while (!write_queue_.empty()) {
        const std::vector<char>& message = write_queue_.front();

        if (!TrySendImmediately(message)) {
            break;  // 发送失败，等待下次EPOLLOUT事件
        }

        write_queue_.pop();
    }
}

void ConnectionHandler::ProcessMessage(const std::vector<char>& data) {
    // 使用MessageSerializer反序列化消息
    MessageSerializer serializer;
    MessageHeader header;

    if (!serializer.DeserializeHeader(data, header)) {
        SQLCC_LOG_ERROR("Failed to deserialize message header on fd: " + std::to_string(GetFd()));
        SendErrorMessage("Invalid message format");
        return;
    }

    // 根据消息类型分发处理
    switch (header.type) {
        case CONNECT:
            HandleConnectMessage(data);
            break;
        case AUTH:
            HandleAuthMessage(data);
            break;
        case QUERY:
            HandleQueryMessage(data);
            break;
        case KEY_EXCHANGE:
            HandleKeyExchangeMessage(data);
            break;
        default:
            SQLCC_LOG_WARN("Unknown message type: " + std::to_string(header.type) +
                       " on fd: " + std::to_string(GetFd()));
            SendErrorMessage("Unknown message type");
            break;
    }
}

void ConnectionHandler::HandleConnectMessage(const std::vector<char>& data) {
    SQLCC_LOG_INFO("Handling CONNECT message on fd: " + std::to_string(GetFd()));

    // 创建会话
    if (session_manager_) {
        session_ = session_manager_->CreateSession();
        if (session_) {
            SQLCC_LOG_INFO("Session created for connection fd: " + std::to_string(GetFd()));
        }
    }

    // 发送连接确认消息
    MessageSerializer serializer;
    std::vector<char> response_payload = {'O', 'K'};  // 简单响应
    std::vector<char> response = serializer.Serialize(CONN_ACK, 0, 0, response_payload);
    SendMessage(response);
}

void ConnectionHandler::HandleAuthMessage(const std::vector<char>& data) {
    SQLCC_LOG_INFO("Handling AUTH message on fd: " + std::to_string(GetFd()));

    // 反序列化认证消息
    MessageSerializer serializer;
    uint8_t type, flags;
    uint32_t sequence_id;
    std::vector<char> payload;

    if (!serializer.DeserializeMessage(data, type, flags, sequence_id, payload)) {
        SendErrorMessage("Invalid auth message format");
        return;
    }

    // 解析用户名和密码 (格式: username:password)
    std::string auth_data(payload.begin(), payload.end());
    size_t colon_pos = auth_data.find(':');
    if (colon_pos == std::string::npos) {
        SendErrorMessage("Invalid auth format");
        return;
    }

    std::string username = auth_data.substr(0, colon_pos);
    std::string password = auth_data.substr(colon_pos + 1);

    // 使用UserManager验证用户认证
    bool auth_success = false;
    if (user_manager_) {
        auth_success = user_manager_->AuthenticateUser(username, password);
    } else {
        SQLCC_LOG_ERROR("UserManager not available for authentication");
        SendErrorMessage("Authentication service unavailable");
        return;
    }

    if (auth_success) {
        SQLCC_LOG_INFO("Authentication successful for user: " + username +
                 " on fd: " + std::to_string(GetFd()));

        // 更新会话认证状态
        if (session_) {
            session_->SetAuthenticated(username);
            SQLCC_LOG_DEBUG("Session authenticated for user: " + username);
        }

        // 发送认证确认
        MessageSerializer response_serializer;
        std::vector<char> response_payload = {'O', 'K'};
        std::vector<char> response = response_serializer.Serialize(AUTH_ACK, 0, sequence_id, response_payload);
        SendMessage(response);
    } else {
        SQLCC_LOG_WARN("Authentication failed for user: " + username +
                    " on fd: " + std::to_string(GetFd()));

        // 发送认证失败错误
        SendErrorMessage("Authentication failed: invalid credentials");
    }
}

void ConnectionHandler::HandleQueryMessage(const std::vector<char>& data) {
    SQLCC_LOG_INFO("Handling QUERY message on fd: " + std::to_string(GetFd()));

    // 反序列化查询消息
    MessageSerializer serializer;
    uint8_t type, flags;
    uint32_t sequence_id;
    std::vector<char> payload;

    if (!serializer.DeserializeMessage(data, type, flags, sequence_id, payload)) {
        SendErrorMessage("Invalid query message format");
        return;
    }

    if (!sql_executor_) {
        SendErrorMessage("SQL executor not available");
        return;
    }

    // 获取查询语句
    std::string query(payload.begin(), payload.end());
    SQLCC_LOG_DEBUG("Processing query: " + query);

    // 权限检查 - 检查用户是否有执行此查询的权限
    if (permission_validator_ && session_ && session_->IsAuthenticated()) {
        // 分析查询类型并检查权限
        std::string username = session_->GetUsername();
        PermissionOperation operation = AnalyzeQueryOperation(query);

        // 提取数据库和表名（简化版）
        std::string database = ExtractDatabaseFromQuery(query);
        std::string table = ExtractTableFromQuery(query);

        // 权限验证
        auto permission_result = permission_validator_->validate(operation, table, username, database);

        if (!permission_result.allowed) {
            SQLCC_LOG_WARN("Permission denied for user '" + username + "' on operation '" +
                          std::to_string(static_cast<int>(operation)) + "' on table '" + table + "'");
            SendErrorMessage("Permission denied: " + permission_result.message);
            return;
        }

        SQLCC_LOG_DEBUG("Permission granted for user '" + username + "' to execute query");
    } else if (!session_ || !session_->IsAuthenticated()) {
        SQLCC_LOG_WARN("Query attempted by unauthenticated user on fd: " + std::to_string(GetFd()));
        SendErrorMessage("Authentication required");
        return;
    }

    // 执行SQL查询
    try {
        std::string result = sql_executor_->Execute(query);
        SQLCC_LOG_DEBUG("Query executed successfully");

        // 发送查询结果
        std::vector<char> result_payload(result.begin(), result.end());
        std::vector<char> response = serializer.Serialize(QUERY_RESULT, 0, sequence_id, result_payload);
        SendMessage(response);

    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Query execution failed: " + std::string(e.what()));
        SendErrorMessage("Query execution failed: " + std::string(e.what()));
    }
}

void ConnectionHandler::HandleKeyExchangeMessage(const std::vector<char>& data) {
    SQLCC_LOG_INFO("Handling KEY_EXCHANGE message on fd: " + std::to_string(GetFd()));

    try {
        // 反序列化密钥交换消息
        MessageSerializer serializer;
        uint8_t type, flags;
        uint32_t sequence_id;
        std::vector<char> payload;

        if (!serializer.DeserializeMessage(data, type, flags, sequence_id, payload)) {
            SendErrorMessage("Invalid key exchange message format");
            return;
        }

        // 简化实现：生成随机AES密钥（32字节）
        // 在实际应用中，应该使用Diffie-Hellman交换
        std::vector<uint8_t> aes_key(32);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 255);

        for (auto& byte : aes_key) {
            byte = static_cast<uint8_t>(dis(gen));
        }

        // 生成随机IV（16字节）
        std::vector<uint8_t> aes_iv(16);
        for (auto& byte : aes_iv) {
            byte = static_cast<uint8_t>(dis(gen));
        }

        // 创建加密密钥对象
        auto encryption_key = std::make_shared<EncryptionKey>(aes_key, aes_iv);

        // 创建AES加密器
        auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);

        // 设置到连接处理器
        SetAESEncryptor(aes_encryptor);

        // 将密钥和IV组合成响应payload（密钥32字节 + IV16字节 = 48字节）
        std::vector<char> response_payload;
        response_payload.reserve(48);
        response_payload.insert(response_payload.end(), aes_key.begin(), aes_key.end());
        response_payload.insert(response_payload.end(), aes_iv.begin(), aes_iv.end());

        // 发送密钥交换确认
        std::vector<char> response = serializer.Serialize(KEY_EXCHANGE_ACK, 0, sequence_id, response_payload);
        SendMessage(response);

        SQLCC_LOG_INFO("Key exchange completed successfully for fd: " + std::to_string(GetFd()));

    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Key exchange failed: " + std::string(e.what()));
        SendErrorMessage("Key exchange failed: " + std::string(e.what()));
    }
}

void ConnectionHandler::SendErrorMessage(const std::string& error) {
    SQLCC_LOG_ERROR("Sending error message: " + error + " on fd: " + std::to_string(GetFd()));

    MessageSerializer serializer;
    std::vector<char> error_payload(error.begin(), error.end());
    std::vector<char> error_message = serializer.Serialize(ERROR, 0, 0, error_payload);
    SendMessage(error_message);
}

bool ConnectionHandler::TrySendImmediately(const std::vector<char>& data) {
    if (closed_) {
        return false;
    }

#ifdef __linux__
    size_t total_sent = 0;
    const char* buffer = data.data();

    while (total_sent < data.size()) {
        ssize_t sent = 0;

        if (tls_enabled_ && ssl_) {
            // TLS发送
            sent = SSL_write(ssl_, buffer + total_sent, data.size() - total_sent);
            if (sent <= 0) {
                int ssl_error = SSL_get_error(ssl_, sent);
                if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                    return false;  // 需要重试
                } else {
                    SQLCC_LOG_ERROR("TLS write error on fd: " + std::to_string(GetFd()));
                    Close();
                    return false;
                }
            }
        } else {
            // 普通TCP发送
            sent = write(GetFd(), buffer + total_sent, data.size() - total_sent);
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return false;  // 需要重试
                } else {
                    SQLCC_LOG_ERROR("Write error on fd: " + std::to_string(GetFd()) + ", errno: " + std::to_string(errno));
                    Close();
                    return false;
                }
            }
        }

        total_sent += sent;
    }
#endif

    return true;
}

void ConnectionHandler::Close() {
    if (closed_) {
        return;
    }

    closed_ = true;

    // 清理会话
    if (session_ && session_manager_) {
        session_manager_->DestroySession(session_->GetSessionId());
        session_.reset();
    }

    // 清理写队列
    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        while (!write_queue_.empty()) {
            write_queue_.pop();
        }
    }

    // 关闭文件描述符
    fd_.reset();

    SQLCC_LOG_INFO("Connection closed, fd: " + std::to_string(GetFd()));
}

// 权限分析辅助函数实现
sqlcc::PermissionOperation ConnectionHandler::AnalyzeQueryOperation(const std::string& query) {
    // 转换为大写进行匹配
    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);

    // 去除前后的空白字符
    upper_query.erase(upper_query.begin(), std::find_if(upper_query.begin(), upper_query.end(),
                       [](unsigned char ch) { return !std::isspace(ch); }));
    upper_query.erase(std::find_if(upper_query.rbegin(), upper_query.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); }).base(), upper_query.end());

    // 分析查询类型
    if (upper_query.find("SELECT") == 0) {
        return sqlcc::PermissionOperation::SELECT;
    } else if (upper_query.find("INSERT") == 0) {
        return sqlcc::PermissionOperation::INSERT;
    } else if (upper_query.find("UPDATE") == 0) {
        return sqlcc::PermissionOperation::UPDATE;
    } else if (upper_query.find("DELETE") == 0) {
        return sqlcc::PermissionOperation::DELETE;
    } else if (upper_query.find("CREATE TABLE") == 0 || upper_query.find("CREATE DATABASE") == 0) {
        return sqlcc::PermissionOperation::CREATE_TABLE;
    } else if (upper_query.find("DROP TABLE") == 0 || upper_query.find("DROP DATABASE") == 0) {
        return sqlcc::PermissionOperation::DROP_TABLE;
    } else if (upper_query.find("ALTER TABLE") == 0) {
        return sqlcc::PermissionOperation::ALTER_TABLE;
    } else if (upper_query.find("GRANT") == 0) {
        return sqlcc::PermissionOperation::GRANT;
    } else if (upper_query.find("REVOKE") == 0) {
        return sqlcc::PermissionOperation::REVOKE;
    } else if (upper_query.find("CREATE USER") == 0) {
        return sqlcc::PermissionOperation::CREATE_USER;
    } else if (upper_query.find("DROP USER") == 0) {
        return sqlcc::PermissionOperation::DROP_USER;
    } else if (upper_query.find("SHOW") == 0) {
        if (upper_query.find("DATABASES") != std::string::npos) {
            return sqlcc::PermissionOperation::SHOW_DATABASES;
        } else if (upper_query.find("TABLES") != std::string::npos) {
            return sqlcc::PermissionOperation::SHOW_TABLES;
        }
    }

    // 默认返回SELECT权限（最宽松的权限）
    return sqlcc::PermissionOperation::SELECT;
}

std::string ConnectionHandler::ExtractDatabaseFromQuery(const std::string& query) {
    // 简化实现：查找USE语句或database.table格式
    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);

    // 检查USE语句
    size_t use_pos = upper_query.find("USE ");
    if (use_pos != std::string::npos) {
        size_t start = use_pos + 4;
        size_t end = upper_query.find_first_of(" ;", start);
        if (end != std::string::npos) {
            std::string db = query.substr(start, end - start);
            // 去除空白字符
            db.erase(db.begin(), std::find_if(db.begin(), db.end(),
                       [](unsigned char ch) { return !std::isspace(ch); }));
            db.erase(std::find_if(db.rbegin(), db.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); }).base(), db.end());
            return db;
        }
    }

    // 检查database.table格式（简化版）
    size_t dot_pos = query.find('.');
    if (dot_pos != std::string::npos) {
        std::string potential_db = query.substr(0, dot_pos);
        // 简化检查：如果不包含空格，可能是一个数据库名
        if (potential_db.find(' ') == std::string::npos) {
            return potential_db;
        }
    }

    // 返回空字符串表示使用默认数据库
    return "";
}

std::string ConnectionHandler::ExtractTableFromQuery(const std::string& query) {
    // 简化实现：查找FROM、INTO、UPDATE等关键字后的表名
    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);

    std::vector<std::string> keywords = {"FROM ", "INTO ", "UPDATE ", "TABLE "};
    std::string table_name;

    for (const auto& keyword : keywords) {
        size_t pos = upper_query.find(keyword);
        if (pos != std::string::npos) {
            size_t start = pos + keyword.length();
            // 查找下一个空格或分号或结束
            size_t end = query.find_first_of(" ;(", start);
            if (end == std::string::npos) {
                end = query.length();
            }

            table_name = query.substr(start, end - start);
            // 去除空白字符
            table_name.erase(table_name.begin(), std::find_if(table_name.begin(), table_name.end(),
                             [](unsigned char ch) { return !std::isspace(ch); }));
            table_name.erase(std::find_if(table_name.rbegin(), table_name.rend(),
                             [](unsigned char ch) { return !std::isspace(ch); }).base(), table_name.end());

            // 处理database.table格式
            size_t dot_pos = table_name.find('.');
            if (dot_pos != std::string::npos) {
                table_name = table_name.substr(dot_pos + 1);
            }

            if (!table_name.empty()) {
                return table_name;
            }
        }
    }

    // 如果没找到，返回默认表名
    return "unknown_table";
}

} // namespace network
} // namespace sqlcc