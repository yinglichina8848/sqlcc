# AESE加密API参考手册

## 目录
1. [EncryptionKey API](#encryptionkey-api)
2. [AESEncryptor API](#aesencryptor-api)
3. [SimpleEncryptor API](#simpleencryptor-api)
4. [Session API扩展](#session-api-扩展)
5. [ConnectionHandler API扩展](#connectionhandler-api-扩展)
6. [ClientNetworkManager API扩展](#clientnetworkmanager-api-扩展)
7. [常量和枚举](#常量和枚举)
8. [错误处理](#错误处理)

---

## EncryptionKey API

### 类声明
```cpp
namespace sqlcc::network {
class EncryptionKey {
public:
    // 构造函数
    EncryptionKey(const std::vector<uint8_t>& key, 
                  const std::vector<uint8_t>& iv);
    
    // 静态工厂方法
    static std::shared_ptr<EncryptionKey> GenerateRandom(
        size_t key_size = 32,
        size_t iv_size = 16
    );
    
    // 获取器
    const std::vector<uint8_t>& GetKey() const;
    const std::vector<uint8_t>& GetIV() const;
    std::vector<uint8_t>& GetKey();
    std::vector<uint8_t>& GetIV();
};
}
```

### 构造函数

#### `EncryptionKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv)`
创建包含指定密钥和IV的加密密钥对象。

**参数**:
- `key`: 加密密钥数据（建议32字节用于AES-256）
- `iv`: 初始化向量（建议16字节）

**异常**: 无

**示例**:
```cpp
std::vector<uint8_t> key(32), iv(16);
// 填充key和iv...
auto enc_key = std::make_shared<EncryptionKey>(key, iv);
```

### 静态方法

#### `static std::shared_ptr<EncryptionKey> GenerateRandom(size_t key_size = 32, size_t iv_size = 16)`
生成指定大小的随机密钥和IV。

**参数**:
- `key_size`: 密钥大小（字节），默认32（AES-256）
- `iv_size`: IV大小（字节），默认16

**返回**: 包含随机生成密钥和IV的EncryptionKey指针

**异常**: 
- `std::runtime_error`: OpenSSL随机数生成失败

**示例**:
```cpp
// 生成AES-256密钥
auto key = EncryptionKey::GenerateRandom(32, 16);

// 生成自定义大小的密钥
auto key128 = EncryptionKey::GenerateRandom(16, 16);  // AES-128
```

### 获取器方法

#### `const std::vector<uint8_t>& GetKey() const`
获取加密密钥（常量引用）。

**返回**: 对密钥数据的常量引用

**示例**:
```cpp
const auto& key_data = encryption_key->GetKey();
```

#### `std::vector<uint8_t>& GetKey()`
获取加密密钥（可修改引用）。

**返回**: 对密钥数据的可修改引用

**示例**:
```cpp
auto& key_data = encryption_key->GetKey();
// 修改密钥...
```

#### `const std::vector<uint8_t>& GetIV() const`
获取初始化向量（常量引用）。

**返回**: 对IV数据的常量引用

#### `std::vector<uint8_t>& GetIV()`
获取初始化向量（可修改引用）。

**返回**: 对IV数据的可修改引用

---

## AESEncryptor API

### 类声明
```cpp
namespace sqlcc::network {
class AESEncryptor {
public:
    // 构造函数
    explicit AESEncryptor(std::shared_ptr<EncryptionKey> encryption_key);
    
    // 析构函数
    ~AESEncryptor();
    
    // 加密/解密
    std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data) const;
    
    // 密钥管理
    void UpdateKey(std::shared_ptr<EncryptionKey> encryption_key);
    
    // 静态方法
    static bool IsAvailable();
};
}
```

### 构造函数

#### `explicit AESEncryptor(std::shared_ptr<EncryptionKey> encryption_key)`
创建AES加密器。

**参数**:
- `encryption_key`: 包含密钥和IV的EncryptionKey对象

**异常**:
- `std::invalid_argument`: 如果encryption_key为nullptr

**示例**:
```cpp
auto encryption_key = EncryptionKey::GenerateRandom();
auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
```

### 加密/解密方法

#### `std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data) const`
使用AES-256-CBC加密数据。

**参数**:
- `data`: 要加密的数据

**返回**: 加密后的数据（包括PKCS7填充）

**异常**:
- `std::runtime_error`: 加密初始化或处理失败
- `std::invalid_argument`: 密钥大小不为32字节

**说明**:
- 实现PKCS7填充
- 输出大小为16的倍数
- 每次加密使用相同的IV（可通过UpdateKey改变）

**示例**:
```cpp
std::string message = "SELECT * FROM users;";
std::vector<uint8_t> plaintext(message.begin(), message.end());

auto encrypted = encryptor->Encrypt(plaintext);
std::cout << "Encrypted size: " << encrypted.size() << std::endl;
```

#### `std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data) const`
使用AES-256-CBC解密数据。

**参数**:
- `data`: 要解密的数据（之前由Encrypt生成）

**返回**: 解密后的原始数据（自动移除PKCS7填充）

**异常**:
- `std::runtime_error`: 解密初始化或处理失败
- `std::invalid_argument`: 密钥大小不为32字节

**说明**:
- 自动处理PKCS7填充移除
- 可以解密任何有效的AES-256-CBC密文
- 无效的密钥会导致输出为乱码（不会抛异常）

**示例**:
```cpp
auto decrypted = encryptor->Decrypt(encrypted);
std::string recovered(decrypted.begin(), decrypted.end());
std::cout << "Decrypted: " << recovered << std::endl;
```

### 密钥管理

#### `void UpdateKey(std::shared_ptr<EncryptionKey> encryption_key)`
更新加密器使用的密钥和IV。

**参数**:
- `encryption_key`: 新的EncryptionKey对象

**异常**:
- `std::invalid_argument`: 如果encryption_key为nullptr

**说明**:
- 线程不安全，应在加密/解密操作前调用
- 新密钥立即生效
- 用旧密钥加密的数据无法用新密钥解密

**示例**:
```cpp
auto new_key = EncryptionKey::GenerateRandom();
encryptor->UpdateKey(new_key);
```

### 静态方法

#### `static bool IsAvailable()`
检查AES库是否可用。

**返回**: 
- `true`: AES库可用（Linux系统有OpenSSL）
- `false`: AES库不可用（非Linux系统）

**说明**:
- 应在使用AES前检查此方法
- Linux系统总是返回true（假设安装了OpenSSL）
- 其他平台返回false

**示例**:
```cpp
if (AESEncryptor::IsAvailable()) {
    auto encryptor = std::make_shared<AESEncryptor>(key);
} else {
    std::cerr << "AES not available!" << std::endl;
}
```

---

## SimpleEncryptor API

### 类声明
```cpp
namespace sqlcc::network {
class SimpleEncryptor {
public:
    // 构造函数
    explicit SimpleEncryptor(const std::string& key);
    
    // 加密/解密
    std::vector<char> Encrypt(const std::vector<char>& data) const;
    std::vector<char> Decrypt(const std::vector<char>& data) const;
};
}
```

### 构造函数

#### `explicit SimpleEncryptor(const std::string& key)`
创建XOR加密器（轻量级加密）。

**参数**:
- `key`: XOR加密密钥字符串

**示例**:
```cpp
SimpleEncryptor encryptor("my_secret_key");
```

### 加密/解密方法

#### `std::vector<char> Encrypt(const std::vector<char>& data) const`
使用XOR加密数据。

**参数**:
- `data`: 要加密的数据

**返回**: 加密后的数据

**说明**:
- 使用简单的XOR操作
- 密钥循环使用
- 加密和解密是相同操作

**示例**:
```cpp
std::string message = "Hello";
std::vector<char> data(message.begin(), message.end());
auto encrypted = encryptor.Encrypt(data);
```

#### `std::vector<char> Decrypt(const std::vector<char>& data) const`
使用XOR解密数据（与Encrypt相同）。

**参数**:
- `data`: 要解密的数据

**返回**: 解密后的原始数据

---

## Session API扩展

### 新增方法

#### `void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor)`
为Session设置AES加密器。

**参数**:
- `encryptor`: AESEncryptor对象

**示例**:
```cpp
auto encryption_key = EncryptionKey::GenerateRandom();
auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
session->SetAESEncryptor(aes_encryptor);
```

#### `std::shared_ptr<AESEncryptor> GetAESEncryptor() const`
获取Session的AES加密器。

**返回**: AESEncryptor对象指针，若未设置则为nullptr

#### `bool IsAESEncryptionEnabled() const`
检查Session是否启用了AES加密。

**返回**:
- `true`: AES加密已启用
- `false`: AES加密未启用或已禁用

**示例**:
```cpp
if (session->IsAESEncryptionEnabled()) {
    // 使用加密通信
} else {
    // 使用未加密通信
}
```

---

## ConnectionHandler API扩展

### 新增方法

#### `std::vector<char> EncryptMessage(const std::vector<char>& message)`
对消息体进行加密（如果Session启用了加密）。

**参数**:
- `message`: 要加密的消息体

**返回**: 加密后的消息（如果未启用加密则返回原消息）

**说明**:
- 内部使用，自动调用
- 消息头不加密

#### `std::vector<char> DecryptMessage(const std::vector<char>& message)`
对消息体进行解密（如果Session启用了加密）。

**参数**:
- `message`: 要解密的消息体

**返回**: 解密后的消息（如果未启用加密则返回原消息）

**说明**:
- 内部使用，自动调用
- 自动处理解密异常

---

## ClientNetworkManager API扩展

### 新增方法

#### `bool InitiateKeyExchange()`
向服务器发起密钥交换请求。

**返回**:
- `true`: 密钥交换成功，AES加密已启用
- `false`: 密钥交换失败

**异常**: 无（捕获并返回false）

**说明**:
- 应在连接建立后、发送查询前调用
- 创建并设置AESEncryptor
- 与服务器的KEY_EXCHANGE/KEY_EXCHANGE_ACK通信

**示例**:
```cpp
client_manager->Connect();
if (client_manager->InitiateKeyExchange()) {
    std::cout << "Encryption enabled" << std::endl;
}
```

#### `void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor)`
手动设置AES加密器。

**参数**:
- `encryptor`: AESEncryptor对象

#### `std::shared_ptr<AESEncryptor> GetAESEncryptor() const`
获取当前的AES加密器。

**返回**: AESEncryptor对象指针

#### `bool IsAESEncryptionEnabled() const`
检查是否启用了AES加密。

**返回**:
- `true`: AES加密已启用
- `false`: AES加密未启用

#### `std::vector<char> EncryptMessage(const std::vector<char>& message)`
对消息进行加密。

**参数**:
- `message`: 要加密的消息

**返回**: 加密后的消息

#### `std::vector<char> DecryptMessage(const std::vector<char>& message)`
对消息进行解密。

**参数**:
- `message`: 要解密的消息

**返回**: 解密后的消息

---

## 常量和枚举

### MessageType 扩展

```cpp
enum MessageType {
    // ... 现有类型 ...
    KEY_EXCHANGE = 8,       // 密钥交换请求
    KEY_EXCHANGE_ACK = 9    // 密钥交换确认
};
```

### MessageHeader Flags

```cpp
// KEY_EXCHANGE_ACK中的flags含义
#define MSG_FLAG_AES_ENCRYPTION 0x01  // 已启用AES加密
```

---

## 错误处理

### 异常类型

| 异常类型 | 原因 | 处理方式 |
|---------|------|--------|
| `std::invalid_argument` | 参数无效（如null指针） | 检查参数有效性 |
| `std::runtime_error` | 加密/解密操作失败 | 记录日志，返回错误 |

### 错误检查示例

```cpp
try {
    auto encrypted = encryptor->Encrypt(data);
} catch (const std::invalid_argument& e) {
    std::cerr << "Invalid argument: " << e.what() << std::endl;
} catch (const std::runtime_error& e) {
    std::cerr << "Encryption failed: " << e.what() << std::endl;
}
```

### 返回值检查

```cpp
// 检查密钥交换是否成功
if (!client_manager->InitiateKeyExchange()) {
    std::cerr << "Key exchange failed" << std::endl;
    return false;
}

// 检查加密是否可用
if (!AESEncryptor::IsAvailable()) {
    std::cerr << "AES not available" << std::endl;
    return false;
}
```

---

## 版本信息

- **API版本**: 1.0
- **最后更新**: 2024年12月
- **兼容性**: C++17及以上
- **依赖**: OpenSSL >= 1.1.0（Linux）

---

## 许可证

本API遵循SQLCC项目许可证。
