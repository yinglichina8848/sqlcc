#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "utils/version.h"
#include "utils/file_descriptor.h"

/**
 * MySQL协议核心常量定义
 * 参考MySQL 8.0.28协议规范
 */
constexpr uint32_t CAPABILITIES = 0x00000D25; // CLIENT_PROTOCOL_41 |
                                             // CLIENT_SECURE_CONNECTION |
                                             // CLIENT_PLUGIN_AUTH |
                                             // CLIENT_CONNECT_WITH_DB

/**
 * MySQL Handshake V10 响应包结构
 * 用于服务端初始化连接
 */
struct HandshakeV10 {
    uint8_t protocol_version = 0x0A;
    std::string server_version = "sqlcc-" + std::string(SQLCC_VERSION);
    uint32_t thread_id;
    uint8_t scramble_buf[20];
    uint16_t server_capabilities;
    uint8_t server_default_collation = 0x21; // utf8mb4_general_ci
    uint16_t server_status = 0x0002; // SERVER_STATUS_AUTOCOMMIT
    uint8_t auth_plugin_name[20];

    // 生成随机salt（用于密码加密）
    void generate_scramble();
};

/**
 * MySQL协议处理器接口
 * 用于双协议共存支持
 */
class MySQLProtocolHandler {
public:
    explicit MySQLProtocolHandler(sqlcc::FileDescriptor&& client_fd);

    // 发送握手包
    void send_handshake();

    // 处理客户端响应
    bool handle_client_response();

private:
    // 发送MySQL协议包（带包头）
    bool send_packet(const uint8_t* data, size_t length, uint8_t sequence_id);

    // 接收MySQL协议包
    std::vector<uint8_t> receive_packet();

    sqlcc::FileDescriptor client_fd_;
    HandshakeV10 handshake_;
    uint8_t next_sequence_id_; // 包序列号管理
};
