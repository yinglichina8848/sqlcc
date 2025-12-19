#include "network/mysql_protocol.h"
#include <cstring>
#include <random>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void HandshakeV10::generate_scramble() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 255);

  for (int i = 0; i < 20; i++) {
    scramble_buf[i] = static_cast<uint8_t>(dis(gen));
  }
}

MySQLProtocolHandler::MySQLProtocolHandler(sqlcc::FileDescriptor&& client_fd)
    : client_fd_(std::move(client_fd)) {
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
  // 发送协议版本
  uint8_t version = handshake_.protocol_version;
  write(client_fd_.get(), &version, sizeof(version));

  // 发送服务器版本字符串（以\0结尾）
  write(client_fd_.get(), handshake_.server_version.c_str(),
        handshake_.server_version.size() + 1);

  // 发送线程ID
  write(client_fd_.get(), &handshake_.thread_id, sizeof(handshake_.thread_id));

  // 发送salt（前8字节）
  write(client_fd_.get(), handshake_.scramble_buf, 8);

  // 填充字节
  uint8_t filler = 0;
  write(client_fd_.get(), &filler, 1);

  // 发送能力标志低16位
  uint16_t cap_low =
      static_cast<uint16_t>(handshake_.server_capabilities & 0xFFFF);
  write(client_fd_.get(), &cap_low, sizeof(cap_low));

  // 发送字符集
  write(client_fd_.get(), &handshake_.server_default_collation, 1);

  // 发送状态标志
  write(client_fd_.get(), &handshake_.server_status,
        sizeof(handshake_.server_status));

  // 发送能力标志高16位
  uint16_t cap_high =
      static_cast<uint16_t>((handshake_.server_capabilities >> 16) & 0xFFFF);
  write(client_fd_.get(), &cap_high, sizeof(cap_high));

  // 填充剩余字段
  uint8_t auth_plugin_len = 20;
  write(client_fd_.get(), &auth_plugin_len, 1);

  // 填充保留字段
  uint8_t reserved[10] = {0};
  write(client_fd_.get(), reserved, 10);

  // 发送salt剩余12字节
  write(client_fd_.get(), handshake_.scramble_buf + 8, 12);

  // 发送认证插件名
  write(client_fd_.get(), handshake_.auth_plugin_name, 20);
}

bool MySQLProtocolHandler::handle_client_response() {
  // 读取客户端响应（至少1字节命令）
  uint8_t cmd;
  ssize_t ret = read(client_fd_.get(), &cmd, 1);
  if (ret <= 0)
    return false;

  // 暂时只处理登录命令（0x0E）
  return (cmd == 0x0E);
}