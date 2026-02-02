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
 * @class MySQLProtocolHandler
 * @brief MySQL 协议兼容层处理器 - 实现 SQLCC 与标准 MySQL 客户端的无缝对接
 *
 * WHY层 - 设计意图：
 *   为了降低用户的迁移成本，SQLCC 必须兼容主流的数据库客户端工具（如 MySQL Workbench, Navicat）。
 *   MySQL 协议是一种基于 TCP 的有状态二进制协议。通过实现该协议，SQLCC 能够伪装成一个
 *   标准的 MySQL 服务器，允许用户使用现有的驱动程序（如 JDBC, PyMySQL）直接连接。
 *
 * WHAT层 - 功能说明：
 *   实现 MySQL 经典的四阶段握手（Handshake V10）。
 *   解析 HandshakeResponse41 包，提取用户名、数据库名及加密后的认证数据（Scramble）。
 *   封装并发送查询结果集（ResultSet），包括列定义（Column Definition）和行数据（Text Resultset Row）。
 *   管理数据包序列号（Sequence ID），确保数据包在 TCP 链路上的顺序一致性。
 *
 * HOW层 - 实现机制：
 *   1. 二进制序列化：按照 MySQL 官方文档定义的字节序（小端序为主）手动构建 Data Payload。
 *   2. 变长编码：实现“长度编码整数（Length Encoded Integer）”算法，以节省元数据传输空间。
 *   3. 状态维护：next_sequence_id_ 记录当前会话的包编号，每发送/接收一个完整包则自增。
 *   4. 安全集成：利用 handshake_.scramble_buf 生成随机盐值，支持 mysql_native_password 认证方式。
 */
class MySQLProtocolHandler {
public:
    explicit MySQLProtocolHandler(sqlcc::FileDescriptor&& client_fd);

    // 发送握手包
    void send_handshake();

    // 处理客户端响应
    bool handle_client_response();

    // 发送认证成功响应
    bool send_auth_success();

    // 发送认证失败响应
    bool send_auth_error(const std::string& error_message);

    // 发送查询结果
    bool send_query_result(const std::vector<std::vector<std::string>>& rows,
                          const std::vector<std::string>& columns);

    // 获取客户端认证信息
    const std::string& get_client_username() const;
    const std::string& get_client_database() const;
    const std::vector<uint8_t>& get_client_auth_data() const;

private:
    // 发送MySQL协议包（带包头）
    bool send_packet(const uint8_t* data, size_t length, uint8_t sequence_id);

    // 接收MySQL协议包
    std::vector<uint8_t> receive_packet();

    // 发送错误包
    bool send_error_packet(const std::string& error_message, uint8_t sequence_id);

    // 长度编码辅助函数
    void encode_length_encoded_string(std::vector<uint8_t>& packet, const std::string& str);
    void encode_length(std::vector<uint8_t>& packet, size_t length);

    sqlcc::FileDescriptor client_fd_;
    HandshakeV10 handshake_;
    uint8_t next_sequence_id_; // 包序列号管理

    // 客户端认证信息
    std::string client_username_;
    std::string client_database_;
    std::vector<uint8_t> client_auth_data_;
};