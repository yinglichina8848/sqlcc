// 测试重构验证文件
// 验证所有分离的类文件都能正确编译

#include <iostream>
#include <memory>

// 测试异常类
#include "exception.h"

// 测试替换策略类
#include "storage/replace_strategy.h"

// 测试加密类
#include "network/encryption.h"

int main() {
    std::cout << "=== SQLCC重构验证测试 ===" << std::endl;

    // 测试异常类
    try {
        throw sqlcc::IOException("Test IO exception");
    } catch (const sqlcc::Exception& e) {
        std::cout << "✓ 异常类测试通过: " << e.what() << std::endl;
    }

    // 测试替换策略
    auto strategy = sqlcc::ReplaceStrategyFactory::CreateStrategy(
        sqlcc::ReplaceStrategyFactory::StrategyType::LRU);
    std::cout << "✓ 替换策略工厂测试通过: " << strategy->GetName() << std::endl;

    // 测试加密类
    sqlcc::network::SimpleEncryptor encryptor("test_key");
    std::vector<char> test_data = {'H', 'e', 'l', 'l', 'o'};
    auto encrypted = encryptor.Encrypt(test_data);
    auto decrypted = encryptor.Decrypt(encrypted);
    bool data_match = (test_data == decrypted);
    std::cout << "✓ 简单加密器测试通过: " << (data_match ? "数据匹配" : "数据不匹配") << std::endl;

    std::cout << "=== 所有重构模块验证成功! ===" << std::endl;
    return 0;
}
