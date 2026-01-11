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

  // 解析握手响应 (HandshakeResponse41结构)
  if (response_packet.size() < 32) {
    return false; // 握手响应包太小
  }

  size_t offset = 0;

  // 解析客户端能力标志 (4字节)
  if (offset + 4 > response_packet.size()) return false;
  uint32_t client_capabilities = response_packet[offset] |
                                 (response_packet[offset + 1] << 8) |
                                 (response_packet[offset + 2] << 16) |
                                 (response_packet[offset + 3] << 24);
  offset += 4;

  // 解析最大包长度 (4字节)
  if (offset + 4 > response_packet.size()) return false;
  uint32_t max_packet_size = response_packet[offset] |
                            (response_packet[offset + 1] << 8) |
                            (response_packet[offset + 2] << 16) |
                            (response_packet[offset + 3] << 24);
  offset += 4;

  // 解析字符集 (1字节)
  if (offset + 1 > response_packet.size()) return false;
  uint8_t charset = response_packet[offset];
  offset += 1;

  // 跳过填充字节 (23字节)
  offset += 23;
  if (offset > response_packet.size()) return false;

  // 解析用户名 (null-terminated string)
  std::string username;
  while (offset < response_packet.size() && response_packet[offset] != 0) {
    username += response_packet[offset];
    offset++;
  }
  if (offset >= response_packet.size()) return false;
  offset++; // 跳过null终止符

  // 解析认证数据长度 (1字节)
  if (offset + 1 > response_packet.size()) return false;
  uint8_t auth_data_len = response_packet[offset];
  offset += 1;

  // 解析认证数据
  std::vector<uint8_t> auth_data;
  size_t auth_data_size = (auth_data_len > 0) ? auth_data_len : 20; // 默认20字节
  if (offset + auth_data_size > response_packet.size()) return false;
  auth_data.insert(auth_data.end(),
                   response_packet.begin() + offset,
                   response_packet.begin() + offset + auth_data_size);
  offset += auth_data_size;

  // 解析数据库名 (null-terminated string，如果有的话)
  std::string database;
  if (offset < response_packet.size() && response_packet[offset] != 0) {
    while (offset < response_packet.size() && response_packet[offset] != 0) {
      database += response_packet[offset];
      offset++;
    }
    offset++; // 跳过null终止符
  }

  // 解析认证插件名 (null-terminated string，如果有的话)
  std::string auth_plugin;
  if (offset < response_packet.size()) {
    while (offset < response_packet.size() && response_packet[offset] != 0) {
      auth_plugin += response_packet[offset];
      offset++;
    }
  }

  // 验证解析结果
  if (username.empty()) {
    return false; // 用户名不能为空
  }

  // 存储认证信息供后续使用
  client_username_ = username;
  client_database_ = database;
  client_auth_data_ = auth_data;

  return true;
}

// 发送认证成功响应
bool MySQLProtocolHandler::send_auth_success() {
  // 发送OK包 (序列号2)
  std::vector<uint8_t> ok_packet;

  // OK包头: 0x00表示OK包
  ok_packet.push_back(0x00);

  // 影响行数 (1字节编码)
  ok_packet.push_back(0x00);

  // 最后插入ID (1字节编码)
  ok_packet.push_back(0x00);

  // 服务器状态 (2字节)
  ok_packet.push_back(0x02); // SERVER_STATUS_AUTOCOMMIT
  ok_packet.push_back(0x00);

  // 警告计数 (2字节)
  ok_packet.push_back(0x00);
  ok_packet.push_back(0x00);

  return send_packet(ok_packet.data(), ok_packet.size(), 2);
}

// 发送认证失败响应
bool MySQLProtocolHandler::send_auth_error(const std::string& error_message) {
  // 发送ERR包 (序列号2)
  std::vector<uint8_t> err_packet;

  // ERR包头: 0xFF表示ERR包
  err_packet.push_back(0xFF);

  // 错误码 (2字节，小端序)
  uint16_t error_code = 1045; // ER_ACCESS_DENIED_ERROR
  err_packet.push_back(error_code & 0xFF);
  err_packet.push_back((error_code >> 8) & 0xFF);

  // SQL状态 (5字节 + null)
  const char* sql_state = "28000"; // ER_ACCESS_DENIED_ERROR的SQL状态
  err_packet.insert(err_packet.end(), sql_state, sql_state + 5);
  err_packet.push_back(0);

  // 错误消息
  err_packet.insert(err_packet.end(), error_message.begin(), error_message.end());

  return send_packet(err_packet.data(), err_packet.size(), 2);
}

// 发送查询结果
bool MySQLProtocolHandler::send_query_result(const std::vector<std::vector<std::string>>& rows,
                                           const std::vector<std::string>& columns) {
  try {
    uint8_t sequence_id = 1; // 从序列号1开始

    // 发送列计数包
    std::vector<uint8_t> column_count_packet;
    column_count_packet.push_back(static_cast<uint8_t>(columns.size())); // 列数
    if (!send_packet(column_count_packet.data(), column_count_packet.size(), sequence_id++)) {
      return false;
    }

    // 发送每个列的定义
    for (const auto& column : columns) {
      std::vector<uint8_t> column_def_packet;

      // 目录 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, "def");

      // 数据库 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, client_database_);

      // 表 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, ""); // 简化实现

      // 原始表 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, "");

      // 列名 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, column);

      // 原始列名 (长度编码字符串)
      encode_length_encoded_string(column_def_packet, column);

      // 填充
      column_def_packet.push_back(0x0C); // 接下来的固定长度字段长度

      // 字符集 (2字节)
      column_def_packet.push_back(0x21); // utf8mb4
      column_def_packet.push_back(0x00);

      // 列长度 (4字节)
      uint32_t col_length = 100; // 简化实现
      column_def_packet.push_back(col_length & 0xFF);
      column_def_packet.push_back((col_length >> 8) & 0xFF);
      column_def_packet.push_back((col_length >> 16) & 0xFF);
      column_def_packet.push_back((col_length >> 24) & 0xFF);

      // 列类型 (1字节)
      column_def_packet.push_back(0xFD); // VARCHAR

      // 标志 (2字节)
      column_def_packet.push_back(0x00);
      column_def_packet.push_back(0x00);

      // 小数位数 (1字节)
      column_def_packet.push_back(0x00);

      // 填充 (2字节)
      column_def_packet.push_back(0x00);
      column_def_packet.push_back(0x00);

      if (!send_packet(column_def_packet.data(), column_def_packet.size(), sequence_id++)) {
        return false;
      }
    }

    // 发送EOF包 (列定义结束)
    std::vector<uint8_t> eof_packet;
    eof_packet.push_back(0xFE); // EOF标记
    eof_packet.push_back(0x00); // 警告计数
    eof_packet.push_back(0x00); // 状态标志
    eof_packet.push_back(0x02);
    if (!send_packet(eof_packet.data(), eof_packet.size(), sequence_id++)) {
      return false;
    }

    // 发送数据行
    for (const auto& row : rows) {
      std::vector<uint8_t> row_packet;

      // 每个字段作为长度编码字符串
      for (const auto& field : row) {
        encode_length_encoded_string(row_packet, field);
      }

      if (!send_packet(row_packet.data(), row_packet.size(), sequence_id++)) {
        return false;
      }
    }

    // 发送最终EOF包
    std::vector<uint8_t> final_eof;
    final_eof.push_back(0xFE);
    final_eof.push_back(0x00);
    final_eof.push_back(0x00);
    final_eof.push_back(0x02);
    if (!send_packet(final_eof.data(), final_eof.size(), sequence_id++)) {
      return false;
    }

    return true;
  } catch (const std::exception& e) {
    // 发送错误包
    std::string error_msg = "Query execution failed: " + std::string(e.what());
    send_error_packet(error_msg, sequence_id_);
    return false;
  }
}

// 发送错误包
bool MySQLProtocolHandler::send_error_packet(const std::string& error_message, uint8_t sequence_id) {
  std::vector<uint8_t> err_packet;

  // ERR包头: 0xFF
  err_packet.push_back(0xFF);

  // 错误码 (2字节，小端序)
  uint16_t error_code = 1064; // ER_PARSE_ERROR
  err_packet.push_back(error_code & 0xFF);
  err_packet.push_back((error_code >> 8) & 0xFF);

  // SQL状态 (5字节 + null)
  const char* sql_state = "42000"; // ER_PARSE_ERROR的SQL状态
  err_packet.insert(err_packet.end(), sql_state, sql_state + 5);
  err_packet.push_back(0);

  // 错误消息
  err_packet.insert(err_packet.end(), error_message.begin(), error_message.end());

  return send_packet(err_packet.data(), err_packet.size(), sequence_id);
}

// 长度编码字符串编码
void MySQLProtocolHandler::encode_length_encoded_string(std::vector<uint8_t>& packet, const std::string& str) {
  // 长度编码
  encode_length(packet, str.size());

  // 字符串数据
  packet.insert(packet.end(), str.begin(), str.end());
}

// 长度编码
void MySQLProtocolHandler::encode_length(std::vector<uint8_t>& packet, size_t length) {
  if (length < 251) {
    packet.push_back(static_cast<uint8_t>(length));
  } else if (length < 65536) {
    packet.push_back(0xFC);
    packet.push_back(length & 0xFF);
    packet.push_back((length >> 8) & 0xFF);
  } else if (length < 16777216) {
    packet.push_back(0xFD);
    packet.push_back(length & 0xFF);
    packet.push_back((length >> 8) & 0xFF);
    packet.push_back((length >> 16) & 0xFF);
  } else {
    packet.push_back(0xFE);
    packet.push_back(length & 0xFF);
    packet.push_back((length >> 8) & 0xFF);
    packet.push_back((length >> 16) & 0xFF);
    packet.push_back((length >> 24) & 0xFF);
  }
}

// 获取客户端用户名
const std::string& MySQLProtocolHandler::get_client_username() const {
  return client_username_;
}

// 获取客户端数据库
const std::string& MySQLProtocolHandler::get_client_database() const {
  return client_database_;
}

// 获取客户端认证数据
const std::vector<uint8_t>& MySQLProtocolHandler::get_client_auth_data() const {
  return client_auth_data_;
}