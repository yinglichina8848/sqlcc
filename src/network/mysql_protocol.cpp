#include "network/mysql_protocol.h"
#include <cstring>
#include <random>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <endian.h>
#include <vector>

void HandshakeV10::generate_scramble() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 255);

  for (int i = 0; i < 20; i++) {
    scramble_buf[i] = static_cast<uint8_t>(dis(gen));
  }
}

MySQLProtocolHandler::MySQLProtocolHandler(sqlcc::FileDescriptor&& client_fd)
    : client_fd_(std::move(client_fd)), next_sequence_id_(0) {
  handshake_.thread_id = getpid(); // 使用进程ID替代线程ID
  handshake_.server_capabilities = CAPABILITIES;
  handshake_.generate_scramble();
  // 设置认证插件为mysql_native_password（安全复制，避免缓冲区溢出）
  const char* plugin_name = "mysql_native_password";
  size_t copy_len = std::min(strlen(plugin_name), sizeof(handshake_.auth_plugin_name) - 1);
  std::memcpy(handshake_.auth_plugin_name, plugin_name, copy_len);
  handshake_.auth_plugin_name[copy_len] = '\0';
}

void MySQLProtocolHandler::send_handshake() {
  // 构建握手包数据（不包含包头）
  std::vector<uint8_t> payload;

  // 发送协议版本
  uint8_t version = handshake_.protocol_version;
  payload.push_back(version);

  // 发送服务器版本字符串（以\0结尾）
  const std::string& server_version = handshake_.server_version;
  payload.insert(payload.end(), server_version.begin(), server_version.end());
  payload.push_back(0); // null terminator

  // 发送线程ID (4字节，小端序)
  uint32_t thread_id_le = htole32(handshake_.thread_id);
  payload.insert(payload.end(),
                 reinterpret_cast<uint8_t*>(&thread_id_le),
                 reinterpret_cast<uint8_t*>(&thread_id_le) + 4);

  // 发送salt（前8字节）
  payload.insert(payload.end(), handshake_.scramble_buf, handshake_.scramble_buf + 8);

  // 填充字节
  payload.push_back(0);

  // 发送能力标志低16位 (小端序)
  uint16_t cap_low = htole16(static_cast<uint16_t>(handshake_.server_capabilities & 0xFFFF));
  payload.insert(payload.end(),
                 reinterpret_cast<uint8_t*>(&cap_low),
                 reinterpret_cast<uint8_t*>(&cap_low) + 2);

  // 发送字符集
  payload.push_back(handshake_.server_default_collation);

  // 发送状态标志 (小端序)
  uint16_t status_le = htole16(handshake_.server_status);
  payload.insert(payload.end(),
                 reinterpret_cast<uint8_t*>(&status_le),
                 reinterpret_cast<uint8_t*>(&status_le) + 2);

  // 发送能力标志高16位 (小端序)
  uint16_t cap_high = htole16(static_cast<uint16_t>((handshake_.server_capabilities >> 16) & 0xFFFF));
  payload.insert(payload.end(),
                 reinterpret_cast<uint8_t*>(&cap_high),
                 reinterpret_cast<uint8_t*>(&cap_high) + 2);

  // 认证插件数据长度
  payload.push_back(20); // auth_plugin_name长度

  // 填充保留字段 (10字节)
  payload.insert(payload.end(), 10, 0);

  // 发送salt剩余12字节
  payload.insert(payload.end(), handshake_.scramble_buf + 8, handshake_.scramble_buf + 20);

  // 发送认证插件名
  payload.insert(payload.end(), handshake_.auth_plugin_name, handshake_.auth_plugin_name + 20);

  // 发送包头 + 负载
  send_packet(payload.data(), payload.size(), 0); // 序列号从0开始
}

bool MySQLProtocolHandler::send_packet(const uint8_t* data, size_t length, uint8_t sequence_id) {
  if (length > 0xFFFFFF) { // MySQL最大包长度限制
    return false;
  }

  // 构建包头：3字节长度 + 1字节序列号
  uint8_t header[4];
  header[0] = length & 0xFF;           // 长度低字节
  header[1] = (length >> 8) & 0xFF;    // 长度中字节
  header[2] = (length >> 16) & 0xFF;   // 长度高字节
  header[3] = sequence_id;             // 序列号

  // 发送包头
  ssize_t sent = write(client_fd_.get(), header, 4);
  if (sent != 4) {
    return false;
  }

  // 发送数据负载
  if (length > 0) {
    sent = write(client_fd_.get(), data, length);
    if (static_cast<size_t>(sent) != length) {
      return false;
    }
  }

  // 更新下一个序列号
  next_sequence_id_ = sequence_id + 1;

  return true;
}

std::vector<uint8_t> MySQLProtocolHandler::receive_packet() {
  // 读取包头
  uint8_t header[4];
  ssize_t received = read(client_fd_.get(), header, 4);
  if (received != 4) {
    return {};
  }

  // 解析包长度和序列号
  uint32_t length = header[0] | (header[1] << 8) | (header[2] << 16);
  uint8_t sequence_id = header[3];

  // 验证序列号
  if (sequence_id != next_sequence_id_) {
    // 序列号不匹配，可能有协议错误
    return {};
  }

  // 读取数据负载
  std::vector<uint8_t> payload(length);
  if (length > 0) {
    received = read(client_fd_.get(), payload.data(), length);
    if (static_cast<size_t>(received) != length) {
      return {};
    }
  }

  // 更新下一个期望的序列号
  next_sequence_id_ = sequence_id + 1;

  return payload;
}

bool MySQLProtocolHandler::handle_client_response() {
  // 接收客户端握手响应包
  auto response_packet = receive_packet();
  if (response_packet.empty()) {
    return false;
  }

  // 解析握手响应
  if (response_packet.size() < 32) {
    return false; // 握手响应包太小
  }

  // 检查是否是登录请求 (第一个字节应该是能力标志)
  // 这里简化处理，实际应该解析完整的握手响应结构
  return response_packet.size() >= 32; // 基本长度检查
}
