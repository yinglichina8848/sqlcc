# AESE加密通信 - 快速开始指南

## 5分钟快速入门

### 1️⃣ 构建项目

```bash
cd /home/liying/sqlcc_qoder
mkdir -p build && cd build
cmake ..
make sqlcc_network
```

### 2️⃣ 运行演示程序

```bash
./aes_demo
```

输出会显示4个演示：
- ✅ 基本AES-256加密
- ✅ SQL查询加密
- ✅ 密钥更新
- ✅ XOR对比

## 🔧 基础用法

### 加密数据

```cpp
#include "network/encryption.h"
using namespace sqlcc::network;

// 1. 生成密钥
auto key = EncryptionKey::GenerateRandom(32, 16);

// 2. 创建加密器
auto encryptor = std::make_shared<AESEncryptor>(key);

// 3. 准备数据
std::string message = "SELECT * FROM users;";
std::vector<uint8_t> plaintext(message.begin(), message.end());

// 4. 加密
auto encrypted = encryptor->Encrypt(plaintext);

// 5. 解密
auto decrypted = encryptor->Decrypt(encrypted);
```

### 在网络中使用

#### 服务器端

```cpp
#include "network/network.h"
using namespace sqlcc::network;

// 在ConnectionHandler中自动加密
// 当收到KEY_EXCHANGE消息时，自动创建加密器
// 之后的消息自动加密/解密
```

#### 客户端

```cpp
ClientNetworkManager client("localhost", 5432);

// 连接到服务器
if (!client.Connect()) {
    std::cerr << "Connection failed" << std::endl;
    return false;
}

// 启动密钥交换
if (!client.InitiateKeyExchange()) {
    std::cerr << "Key exchange failed" << std::endl;
    return false;
}

// 现在通信已加密！
std::string query = "SELECT * FROM users;";
client.SendRequest(std::vector<char>(query.begin(), query.end()));
```

## 📋 常见任务

### 任务1: 检查AES可用性

```cpp
if (AESEncryptor::IsAvailable()) {
    std::cout << "AES加密可用" << std::endl;
} else {
    std::cout << "AES加密不可用" << std::endl;
}
```

### 任务2: 更换加密密钥

```cpp
auto new_key = EncryptionKey::GenerateRandom(32, 16);
encryptor->UpdateKey(new_key);
// 后续加密使用新密钥
```

### 任务3: 加密大文件

```cpp
// 最多100MB
std::vector<uint8_t> large_data(100 * 1024 * 1024);
// 填充数据...

auto encrypted = encryptor->Encrypt(large_data);
auto decrypted = encryptor->Decrypt(encrypted);
```

### 任务4: 为Session启用加密

```cpp
auto session = session_manager->CreateSession();

// 创建加密器
auto key = EncryptionKey::GenerateRandom(32, 16);
auto aes_encryptor = std::make_shared<AESEncryptor>(key);

// 启用加密
session->SetAESEncryptor(aes_encryptor);

// 现在此session的通信已加密
```

## ⚙️ 配置

### 环境要求

- **操作系统**: Linux (推荐Ubuntu 20.04+)
- **C++标准**: C++17或更高
- **编译器**: GCC 9+ 或 Clang 10+
- **依赖**: OpenSSL >= 1.1.0

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# 验证OpenSSL安装
openssl version
# 输出应该显示: OpenSSL 1.1.1 或更高
```

### CMake配置

```cmake
# 自动查找OpenSSL
find_package(OpenSSL REQUIRED)

# 链接到OpenSSL
target_link_libraries(your_target PUBLIC OpenSSL::Crypto)
```

## 🧪 测试

### 运行单元测试

```bash
cd build
g++ -std=c++17 -I../include \
    ../tests/network/aes_encryption_test.cc \
    ./src/libsqlcc_network.a \
    -lgtest -lgtest_main -lpthread -lssl -lcrypto --coverage \
    -o aes_test

./aes_test
```

### 验证演示程序

```bash
./aes_demo
# 应该看到4个演示全部通过 ✓
```

## 🚀 最佳实践

### ✅ 推荐做法

1. **总是检查可用性**
   ```cpp
   if (!AESEncryptor::IsAvailable()) {
       use_simple_encryption();
   }
   ```

2. **妥善处理异常**
   ```cpp
   try {
       auto encrypted = encryptor->Encrypt(data);
   } catch (const std::exception& e) {
       std::cerr << "加密失败: " << e.what() << std::endl;
   }
   ```

3. **定期更换密钥**
   ```cpp
   if (should_rotate_key()) {
       auto new_key = EncryptionKey::GenerateRandom();
       encryptor->UpdateKey(new_key);
   }
   ```

4. **使用智能指针**
   ```cpp
   auto encryptor = std::make_shared<AESEncryptor>(key);
   // 自动管理生命周期
   ```

### ❌ 避免

1. **硬编码密钥**
   ```cpp
   // 不要这样做！
   auto key = EncryptionKey(..., "hardcoded_key");
   ```

2. **重用IV**
   ```cpp
   // 不要用相同的IV加密多条消息
   // 应该生成随机IV
   ```

3. **忽略异常**
   ```cpp
   // 不要这样做！
   auto encrypted = encryptor->Encrypt(data);
   // 直接使用，可能失败
   ```

4. **在线程间共享加密器**
   ```cpp
   // UpdateKey不是线程安全的
   // 应该为每个线程创建独立的加密器
   ```

## 📚 更多资源

| 资源 | 位置 |
|------|------|
| 完整API文档 | [AESE_API_REFERENCE.md](AESE_API_REFERENCE.md) |
| 功能指南 | [AESE_ENCRYPTION_GUIDE.md](AESE_ENCRYPTION_GUIDE.md) |
| 实现总结 | [AESE_IMPLEMENTATION_SUMMARY.md](AESE_IMPLEMENTATION_SUMMARY.md) |
| 演示程序 | [examples/aes_demo.cpp](examples/aes_demo.cpp) |
| 单元测试 | [tests/network/aes_encryption_test.cc](tests/network/aes_encryption_test.cc) |
| 集成测试 | [tests/network/aes_network_integration_test.cc](tests/network/aes_network_integration_test.cc) |

## 🐛 故障排查

### 问题1: AES库不可用

```
错误: AES encryption not available on this platform
原因: OpenSSL未安装
解决:
  sudo apt-get install libssl-dev
  重新编译项目
```

### 问题2: 编译错误 - 找不到encryption.h

```
错误: cannot find encryption.h
原因: 头文件路径不正确
解决:
  确保 -I/path/to/include/network 在编译命令中
```

### 问题3: 链接错误 - 找不到libcrypto

```
错误: undefined reference to EVP_aes_256_cbc
原因: 未链接OpenSSL
解决:
  确保 -lssl -lcrypto 在链接命令中
```

### 问题4: 解密后数据不匹配

```
原因: 使用了不同的密钥或IV
解决:
  确保解密使用与加密相同的EncryptionKey对象
  检查IV是否被篡改
```

## 📞 获取帮助

1. **查看演示程序**: `./aes_demo`
2. **阅读API文档**: [AESE_API_REFERENCE.md](AESE_API_REFERENCE.md)
3. **查看测试用例**: `tests/network/aes_encryption_test.cc`
4. **查看源代码**: `src/network/encryption.cpp`

## 💡 示例代码集合

### 例1: 完整的加密-解密循环

```cpp
#include "network/encryption.h"
using namespace sqlcc::network;

int main() {
    // 生成密钥
    auto key = EncryptionKey::GenerateRandom(32, 16);
    
    // 创建加密器
    auto encryptor = std::make_shared<AESEncryptor>(key);
    
    // 准备数据
    std::string message = "Secret message";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    // 加密
    auto encrypted = encryptor->Encrypt(data);
    std::cout << "Encrypted size: " << encrypted.size() << std::endl;
    
    // 解密
    auto decrypted = encryptor->Decrypt(encrypted);
    std::string recovered(decrypted.begin(), decrypted.end());
    
    std::cout << "Recovered: " << recovered << std::endl;
    return 0;
}
```

### 例2: 服务器启用加密

```cpp
void HandleKeyExchange(ConnectionHandler* handler, Session* session) {
    // 生成密钥
    auto key = EncryptionKey::GenerateRandom(32, 16);
    
    // 创建加密器
    auto aes = std::make_shared<AESEncryptor>(key);
    
    // 为session启用加密
    session->SetAESEncryptor(aes);
    
    // 发送IV给客户端
    // ...
}
```

### 例3: 客户端启用加密

```cpp
bool EnableEncryption(ClientNetworkManager* client) {
    // 发起密钥交换
    if (!client->InitiateKeyExchange()) {
        std::cerr << "Key exchange failed" << std::endl;
        return false;
    }
    
    std::cout << "Encryption enabled" << std::endl;
    return true;
}
```

## 🎯 下一步

1. ✅ 阅读本快速开始指南 (5分钟)
2. ✅ 运行演示程序 (1分钟)
3. 📖 阅读完整API文档 (15分钟)
4. 💻 在你的代码中集成加密 (30分钟)
5. 🧪 编写测试用例 (1小时)
6. 🚀 部署到生产环境 (1天)

---

**版本**: 1.0
**最后更新**: 2024年12月
**作者**: AI Assistant (Qoder)
