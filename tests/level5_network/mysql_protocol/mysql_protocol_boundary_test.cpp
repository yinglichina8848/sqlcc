#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/mysql_protocol.h"
#include "network/connection_state.h"
#include "src/utils/logger.h"

using namespace sqlcc::network;
using namespace sqlcc::utils;

class MySQLProtocolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化协议处理器
        protocol_ = std::make_unique<MySQLProtocol>();
    }

    void TearDown() override {
        protocol_.reset();
    }

    std::unique_ptr<MySQLProtocol> protocol_;
};

// 测试超大包处理 (>16MB)
TEST_F(MySQLProtocolTest, HandleOversizedPacket) {
    // MySQL协议最大包大小为16MB
    const size_t MAX_PACKET_SIZE = 16 * 1024 * 1024; // 16MB
    const size_t OVERSIZED_PACKET_SIZE = 20 * 1024 * 1024; // 20MB

    // 创建超大包数据
    std::vector<char> oversized_packet(OVERSIZED_PACKET_SIZE, 'A');

    // 设置无效的长度头 (超过最大限制)
    uint32_t invalid_length = OVERSIZED_PACKET_SIZE;
    memcpy(oversized_packet.data(), &invalid_length, sizeof(uint32_t));

    // 期望抛出协议异常
    EXPECT_THROW({
        protocol_->ParsePacket(oversized_packet);
    }, ProtocolException);
}

// 测试畸形数据包解析
TEST_F(MySQLProtocolTest, HandleMalformedPacket) {
    // 创建畸形数据包 - 无效长度
    std::vector<char> malformed_packet = {
        0xFF, 0xFF, 0xFF, 0xFF, // 无效长度 (太长)
        0x00,                   // 序列号
        0x01, 0x02, 0x03       // 负载数据
    };

    EXPECT_THROW({
        protocol_->ParsePacket(malformed_packet);
    }, ProtocolException);
}

// 测试空数据包
TEST_F(MySQLProtocolTest, HandleEmptyPacket) {
    std::vector<char> empty_packet;

    EXPECT_THROW({
        protocol_->ParsePacket(empty_packet);
    }, ProtocolException);
}

// 测试不完整的数据包头
TEST_F(MySQLProtocolTest, HandleIncompleteHeader) {
    // 只有2字节的包头 (需要4字节长度 + 1字节序列号)
    std::vector<char> incomplete_header = {0x01, 0x00};

    EXPECT_THROW({
        protocol_->ParsePacket(incomplete_header);
    }, ProtocolException);
}

// 测试多字节字符编码处理
TEST_F(MySQLProtocolTest, UnicodeStringHandling) {
    // 测试各种Unicode字符串
    std::string chinese_text = "中文测试字符串";
    std::string emoji_text = "🚀🌟💻";
    std::string mixed_text = "Hello 中文 🚀 World";

    // 编码测试
    auto encoded_chinese = protocol_->EncodeString(chinese_text);
    auto encoded_emoji = protocol_->EncodeString(emoji_text);
    auto encoded_mixed = protocol_->EncodeString(mixed_text);

    // 解码测试
    EXPECT_EQ(protocol_->DecodeString(encoded_chinese), chinese_text);
    EXPECT_EQ(protocol_->DecodeString(encoded_emoji), emoji_text);
    EXPECT_EQ(protocol_->DecodeString(mixed_text), mixed_text);
}

// 测试NULL终止字符串
TEST_F(MySQLProtocolTest, NullTerminatedString) {
    std::string original = "test string";
    std::string null_terminated = original + '\0';

    auto encoded = protocol_->EncodeString(original, true); // NULL终止
    auto decoded = protocol_->DecodeString(encoded, true);  // NULL终止

    EXPECT_EQ(decoded, null_terminated);
    EXPECT_EQ(decoded.back(), '\0'); // 确保以NULL结尾
}

// 测试大整数编码/解码
TEST_F(MySQLProtocolTest, LargeIntegerEncoding) {
    // 测试64位大整数
    uint64_t large_value = 0xFFFFFFFFFFFFFFFFULL; // 最大64位无符号整数
    int64_t signed_large = INT64_MAX;

    auto encoded_unsigned = protocol_->EncodeLengthEncodedInteger(large_value);
    auto encoded_signed = protocol_->EncodeLengthEncodedInteger(signed_large);

    auto decoded_unsigned = protocol_->DecodeLengthEncodedInteger(encoded_unsigned);
    auto decoded_signed = protocol_->DecodeLengthEncodedInteger(encoded_signed);

    EXPECT_EQ(decoded_unsigned, large_value);
    EXPECT_EQ(decoded_signed, signed_large);
}

// 测试边界长度值
TEST_F(MySQLProtocolTest, BoundaryLengthValues) {
    // 测试各种边界长度值
    std::vector<size_t> boundary_lengths = {
        0,          // 空字符串
        1,          // 单字节
        250,        // 最大单字节长度编码
        251,        // 最小双字节长度编码
        0xFFFF,     // 最大双字节长度
        0xFFFFFF,   // 最大三字节长度
        0x1000000   // 最小四字节长度
    };

    for (size_t length : boundary_lengths) {
        std::string test_data(length, 'X');
        auto encoded = protocol_->EncodeLengthEncodedString(test_data);
        auto decoded = protocol_->DecodeLengthEncodedString(encoded);
        EXPECT_EQ(decoded, test_data);
    }
}

// 测试协议握手包解析
TEST_F(MySQLProtocolTest, HandshakePacketParsing) {
    // 构造有效的握手包 (简化版本)
    std::vector<char> handshake_packet = {
        0x4A, 0x00, 0x00, // 长度 (74字节) + 序列号
        0x0A,             // 协议版本 10
        '8', '.', '0', '.', '32', '-', 'MySQL', '-', 'Community', '-', 'GPL', '\0', // 服务器版本
        0x01, 0x00, 0x00, 0x00, // 连接ID
        // ... 其他握手数据 (简化)
    };

    // 填充到正确长度
    handshake_packet.resize(77, 0);

    EXPECT_NO_THROW({
        auto result = protocol_->ParseHandshakePacket(handshake_packet);
        EXPECT_GE(result.protocol_version, 10);
    });
}

// 测试认证数据包
TEST_F(MySQLProtocolTest, AuthenticationPacket) {
    std::string username = "testuser";
    std::string password = "testpass";
    std::string database = "testdb";

    auto auth_packet = protocol_->CreateAuthenticationPacket(username, password, database);

    EXPECT_FALSE(auth_packet.empty());
    EXPECT_GE(auth_packet.size(), 32); // 最小的认证包大小

    // 验证包结构
    uint32_t length = *reinterpret_cast<const uint32_t*>(auth_packet.data());
    EXPECT_GT(length, 0);
    EXPECT_LT(length, 16 * 1024 * 1024); // 小于16MB
}

// 测试结果集包解析
TEST_F(MySQLProtocolTest, ResultSetPacketParsing) {
    // 构造结果集包 (列定义)
    std::vector<char> column_packet = {
        0x1F, 0x00, 0x00, // 长度 + 序列号
        0x03,             // 目录 (def)
        'd', 'b', '\0',   // 数据库
        't', 'a', 'b', 'l', 'e', '\0', // 表名
        'o', 'r', 'i', 'g', 't', 'a', 'b', 'l', 'e', '\0', // 原始表名
        'c', 'o', 'l', '\0', // 列名
        'o', 'r', 'i', 'g', 'c', 'o', 'l', '\0', // 原始列名
        0x0C,             // 长度编码的长度
        0x21, 0x00,       // 字符集
        0x00, 0x00, 0x00, 0x00, // 列长度
        0xFD,             // 类型 (VARCHAR)
        0x00, 0x00,       // 标志
        0x00              // 小数位
    };

    EXPECT_NO_THROW({
        auto column_def = protocol_->ParseColumnDefinition(column_packet);
        EXPECT_EQ(column_def.name, "col");
        EXPECT_EQ(column_def.type, MYSQL_TYPE_VAR_STRING);
    });
}

// 测试错误包解析
TEST_F(MySQLProtocolTest, ErrorPacketParsing) {
    std::vector<char> error_packet = {
        0x17, 0x00, 0x00, // 长度 + 序列号
        0xFF,             // 错误标记
        0x48, 0x04,       // 错误码 (0x0448 = 1096)
        '#', '4', '2', 'S', '0', '2', // SQL状态
        'T', 'e', 's', 't', ' ', 'e', 'r', 'r', 'o', 'r', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'
    };

    EXPECT_NO_THROW({
        auto error_info = protocol_->ParseErrorPacket(error_packet);
        EXPECT_EQ(error_info.error_code, 1096);
        EXPECT_EQ(error_info.sql_state, "42S02");
        EXPECT_TRUE(error_info.message.find("Test error message") != std::string::npos);
    });
}

// 测试压缩包处理
TEST_F(MySQLProtocolTest, CompressedPacketHandling) {
    std::string original_data = "This is a test message for compression.";
    std::string large_data(10000, 'A'); // 10KB重复数据，适合压缩

    // 测试压缩
    auto compressed = protocol_->CompressPacket(original_data);
    EXPECT_FALSE(compressed.empty());

    // 测试解压
    auto decompressed = protocol_->DecompressPacket(compressed);
    EXPECT_EQ(decompressed, original_data);

    // 测试大数据压缩
    auto compressed_large = protocol_->CompressPacket(large_data);
    auto decompressed_large = protocol_->DecompressPacket(compressed_large);
    EXPECT_EQ(decompressed_large, large_data);
}

// 测试SSL/TLS握手包
TEST_F(MySQLProtocolTest, SSLRequestPacket) {
    uint32_t client_flags = CLIENT_SSL | CLIENT_PROTOCOL_41;
    uint32_t max_packet_size = 16 * 1024 * 1024; // 16MB
    uint8_t charset = 33; // utf8_general_ci

    auto ssl_packet = protocol_->CreateSSLRequestPacket(client_flags, max_packet_size, charset);

    EXPECT_FALSE(ssl_packet.empty());
    EXPECT_GE(ssl_packet.size(), 32); // SSL请求包的最小大小

    // 验证包头
    uint32_t length = *reinterpret_cast<const uint32_t*>(ssl_packet.data());
    EXPECT_GT(length, 0);
}
