#include "network/client_network_manager.h"
#include "network/client_connection.h"
#include "network/message_serializer.h"
#include "network/message_types.h"  // for MessageType
#include "network/session.h"
#include "network/session_manager.h"
#include "network/encryption.h"
#include "src/utils/logger.h"
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace sqlcc {
namespace network {

ClientNetworkManager::ClientNetworkManager(const std::string& host, int port)
    : connection_(std::make_unique<ClientConnection>(host, port)),
      session_manager_(std::make_shared<SessionManager>()),
      aes_encryptor_(nullptr) {
    // 初始化序列号生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(1, UINT32_MAX);
    sequence_counter_ = dis(gen);
}

ClientNetworkManager::~ClientNetworkManager() {
    Disconnect();
}

bool ClientNetworkManager::Connect() {
    if (!connection_) {
        return false;
    }

    bool success = connection_->Connect();
    if (success) {
        // 创建会话
        if (session_manager_) {
            session_ = session_manager_->CreateSession();
            if (session_) {
                SQLCC_LOG_INFO("Successfully connected to server, session created");
            } else {
                SQLCC_LOG_ERROR("Failed to create session after connecting to server");
                connection_->Disconnect();
                return false;
            }
        }
        SQLCC_LOG_INFO("Successfully connected to server");
    } else {
        SQLCC_LOG_ERROR("Failed to connect to server");
    }

    return success;
}

void ClientNetworkManager::EnableTLS(bool enabled) {
    if (connection_) {
        connection_->EnableTLS(enabled);
    }
}

#ifdef __linux__
bool ClientNetworkManager::ConfigureTLSClient(const std::string& ca_cert_path) {
    if (connection_) {
        return connection_->ConfigureTLSClient(ca_cert_path);
    }
    return false;
}
#endif

void ClientNetworkManager::Disconnect() {
    if (connection_ && connection_->IsConnected()) {
        // 发送断开连接消息（可选）
        // SendDisconnectMessage();

        connection_->Disconnect();
        session_.reset();
        SQLCC_LOG_INFO("Disconnected from server");
    }
}

bool ClientNetworkManager::IsConnected() const {
    return connection_ && connection_->IsConnected();
}

bool ClientNetworkManager::SendRequest(const std::vector<char>& request) {
    if (!IsConnected()) {
        SQLCC_LOG_ERROR("Cannot send request: not connected");
        return false;
    }

    // 使用MessageSerializer序列化请求
    MessageSerializer serializer;
    uint8_t message_type = QUERY;  // 默认查询请求
    uint8_t flags = 0;
    uint32_t sequence_id = GenerateSequenceId();

    std::vector<char> serialized_message = serializer.Serialize(message_type, flags, sequence_id, request);

    // 如果启用了AES加密，先加密消息
    if (aes_encryptor_) {
        serialized_message = EncryptMessage(serialized_message);
    }

    // 发送消息
    bool success = connection_->SendData(serialized_message);
    if (success) {
        SQLCC_LOG_DEBUG("Request sent successfully, sequence_id: " + std::to_string(sequence_id));
    } else {
        SQLCC_LOG_ERROR("Failed to send request, sequence_id: " + std::to_string(sequence_id));
    }

    return success;
}

std::vector<char> ClientNetworkManager::ReceiveResponse() {
    if (!IsConnected()) {
        SQLCC_LOG_ERROR("Cannot receive response: not connected");
        return std::vector<char>();
    }

    // 接收原始数据
    std::vector<char> raw_data = connection_->ReceiveData();
    if (raw_data.empty()) {
        return std::vector<char>();  // 无数据或连接已断开
    }

    // 如果启用了AES加密，先解密数据
    if (aes_encryptor_) {
        raw_data = DecryptMessage(raw_data);
    }

    // 使用MessageSerializer反序列化响应
    MessageSerializer serializer;
    uint8_t message_type;
    uint8_t flags;
    uint32_t sequence_id;
    std::vector<char> payload;

    if (serializer.DeserializeMessage(raw_data, message_type, flags, sequence_id, payload)) {
        SQLCC_LOG_DEBUG("Response received successfully, sequence_id: " + std::to_string(sequence_id));
        return payload;
    } else {
        SQLCC_LOG_ERROR("Failed to deserialize response message");
        return std::vector<char>();
    }
}

bool ClientNetworkManager::SendAuthMessage(const std::string& username, const std::string& password) {
    if (!IsConnected()) {
        SQLCC_LOG_ERROR("Cannot send auth message: not connected");
        return false;
    }

    // 构造认证负载数据 (username:password)
    std::string auth_data = username + ":" + password;
    std::vector<char> payload(auth_data.begin(), auth_data.end());

    MessageSerializer serializer;
    std::vector<char> serialized_message = serializer.Serialize(AUTH, 0, GenerateSequenceId(), payload);

    // 如果启用了AES加密，先加密消息
    if (aes_encryptor_) {
        serialized_message = EncryptMessage(serialized_message);
    }

    bool success = connection_->SendData(serialized_message);
    if (success) {
        SQLCC_LOG_INFO("Authentication message sent for user: " + username);
    } else {
        SQLCC_LOG_ERROR("Failed to send authentication message");
    }

    return success;
}

bool ClientNetworkManager::InitiateKeyExchange() {
    if (!IsConnected()) {
        SQLCC_LOG_ERROR("Cannot initiate key exchange: not connected");
        return false;
    }

    // 生成客户端随机数（用于简化密钥交换）
    std::vector<char> client_random(32);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);
    for (auto& byte : client_random) {
        byte = static_cast<char>(dis(gen));
    }

    MessageSerializer serializer;
    std::vector<char> serialized_message = serializer.Serialize(KEY_EXCHANGE, 0, GenerateSequenceId(), client_random);

    // 发送密钥交换请求（不加密，因为还没有密钥）
    bool success = connection_->SendData(serialized_message);
    if (!success) {
        SQLCC_LOG_ERROR("Failed to send key exchange request");
        return false;
    }

    SQLCC_LOG_INFO("Key exchange request sent, waiting for response...");

    // 等待并处理密钥交换响应
    std::vector<char> response = ReceiveResponse();
    if (response.empty()) {
        SQLCC_LOG_ERROR("No response received for key exchange");
        return false;
    }

    try {
        // 解析响应
        MessageSerializer response_serializer;
        uint8_t message_type;
        uint8_t flags;
        uint32_t sequence_id;
        std::vector<char> payload;

        if (!response_serializer.DeserializeMessage(response, message_type, flags, sequence_id, payload)) {
            SQLCC_LOG_ERROR("Failed to deserialize key exchange response");
            return false;
        }

        if (message_type != KEY_EXCHANGE_ACK) {
            SQLCC_LOG_ERROR("Unexpected message type in key exchange response: " + std::to_string(message_type));
            return false;
        }

        if (payload.size() != 48) {  // 32字节密钥 + 16字节IV
            SQLCC_LOG_ERROR("Invalid key exchange response payload size: " + std::to_string(payload.size()));
            return false;
        }

        // 提取AES密钥和IV
        std::vector<uint8_t> aes_key(payload.begin(), payload.begin() + 32);
        std::vector<uint8_t> aes_iv(payload.begin() + 32, payload.end());

        // 创建加密密钥对象
        auto enc_key = std::make_shared<EncryptionKey>(aes_key, aes_iv);

        // 创建AES加密器
        auto aes_encryptor = std::make_shared<AESEncryptor>(enc_key);

        // 设置到客户端网络管理器
        SetAESEncryptor(aes_encryptor);

        SQLCC_LOG_INFO("Key exchange completed successfully");
        return true;

    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Exception during key exchange response processing: " + std::string(e.what()));
        return false;
    }
}

void ClientNetworkManager::SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor) {
    aes_encryptor_ = encryptor;
    if (encryptor) {
        SQLCC_LOG_INFO("AES encryptor set");
    } else {
        SQLCC_LOG_INFO("AES encryptor cleared");
    }
}

std::shared_ptr<class AESEncryptor> ClientNetworkManager::GetAESEncryptor() const {
    return aes_encryptor_;
}

bool ClientNetworkManager::IsAESEncryptionEnabled() const {
    return aes_encryptor_ != nullptr;
}

std::vector<char> ClientNetworkManager::EncryptMessage(const std::vector<char>& message) {
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

std::vector<char> ClientNetworkManager::DecryptMessage(const std::vector<char>& message) {
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

uint32_t ClientNetworkManager::GenerateSequenceId() {
    return sequence_counter_++;
}

} // namespace network
} // namespace sqlcc