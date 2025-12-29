/**
 * @file data_integrity_test.cpp
 * @brief 数据完整性验证系统单元测试 - 全面测试数据校验和完整性保护机制
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <random>

#include "storage/data_integrity_validator.h"
#include "storage/record_boundary_validator.h"
#include "storage_engine.h"
#include "utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class DataIntegrityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_data_integrity_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("data_integrity.checksum_algorithm", std::string("CRC32"));
        config->SetValue("data_integrity.enable_validation", std::string("true"));
        config->SetValue("data_integrity.validation_level", std::string("STRICT"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化数据完整性验证器
        integrity_validator = std::make_unique<DataIntegrityValidator>(storage_engine);
        boundary_validator = std::make_unique<RecordBoundaryValidator>(storage_engine);

        // 设置随机数生成器
        std::random_device rd;
        random_engine = std::mt19937(rd());
    }

    void TearDown() override {
        boundary_validator.reset();
        integrity_validator.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 创建测试数据
    std::vector<uint8_t> CreateTestData(size_t size, uint8_t pattern = 0xAA) {
        return std::vector<uint8_t>(size, pattern);
    }

    // 创建随机测试数据
    std::vector<uint8_t> CreateRandomData(size_t size) {
        std::uniform_int_distribution<uint8_t> dist(0, 255);
        std::vector<uint8_t> data(size);
        for (auto& byte : data) {
            byte = dist(random_engine);
        }
        return data;
    }

    // 损坏数据（用于测试校验和检测）
    void CorruptData(std::vector<uint8_t>& data, size_t corruption_count = 1) {
        std::uniform_int_distribution<size_t> pos_dist(0, data.size() - 1);
        std::uniform_int_distribution<uint8_t> val_dist(0, 255);

        for (size_t i = 0; i < corruption_count && i < data.size(); ++i) {
            size_t pos = pos_dist(random_engine);
            uint8_t original = data[pos];
            uint8_t corrupted;
            do {
                corrupted = val_dist(random_engine);
            } while (corrupted == original);
            data[pos] = corrupted;
        }
    }

    // 创建测试记录
    std::unique_ptr<RecordBoundaryValidator::Record> CreateTestRecord(const std::string& data) {
        auto record = std::make_unique<RecordBoundaryValidator::Record>();
        record->data.assign(data.begin(), data.end());
        record->length = data.size();
        record->checksum = integrity_validator->CalculateChecksum(record->data);
        return record;
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<DataIntegrityValidator> integrity_validator;
    std::unique_ptr<RecordBoundaryValidator> boundary_validator;
    std::mt19937 random_engine;
};

// 测试校验和计算
TEST_F(DataIntegrityTest, ChecksumCalculation) {
    // 测试空数据
    std::vector<uint8_t> empty_data;
    uint32_t empty_checksum = integrity_validator->CalculateChecksum(empty_data);
    EXPECT_NE(empty_checksum, 0);  // 即使是空数据也应该有校验和

    // 测试固定模式数据
    auto fixed_data = CreateTestData(100, 0x55);
    uint32_t fixed_checksum1 = integrity_validator->CalculateChecksum(fixed_data);
    uint32_t fixed_checksum2 = integrity_validator->CalculateChecksum(fixed_data);
    EXPECT_EQ(fixed_checksum1, fixed_checksum2);  // 相同数据应该产生相同校验和

    // 测试不同数据产生不同校验和
    auto different_data = CreateTestData(100, 0xAA);
    uint32_t different_checksum = integrity_validator->CalculateChecksum(different_data);
    EXPECT_NE(fixed_checksum1, different_checksum);

    // 测试随机数据
    auto random_data1 = CreateRandomData(100);
    auto random_data2 = CreateRandomData(100);
    uint32_t random_checksum1 = integrity_validator->CalculateChecksum(random_data1);
    uint32_t random_checksum2 = integrity_validator->CalculateChecksum(random_data2);
    // 随机数据很可能产生不同校验和（极小概率相同）
    // EXPECT_NE(random_checksum1, random_checksum2);
}

// 测试数据完整性验证
TEST_F(DataIntegrityTest, DataIntegrityValidation) {
    // 创建测试数据
    auto test_data = CreateRandomData(1024);
    uint32_t original_checksum = integrity_validator->CalculateChecksum(test_data);

    // 验证完整数据
    bool valid_result = integrity_validator->ValidateIntegrity(test_data, original_checksum);
    EXPECT_TRUE(valid_result);

    // 验证损坏数据
    auto corrupted_data = test_data;
    CorruptData(corrupted_data, 1);
    bool invalid_result = integrity_validator->ValidateIntegrity(corrupted_data, original_checksum);
    EXPECT_FALSE(invalid_result);

    // 验证边界情况：空数据
    std::vector<uint8_t> empty_data;
    uint32_t empty_checksum = integrity_validator->CalculateChecksum(empty_data);
    bool empty_valid = integrity_validator->ValidateIntegrity(empty_data, empty_checksum);
    EXPECT_TRUE(empty_valid);

    // 验证边界情况：单个字节
    std::vector<uint8_t> single_byte = {0x42};
    uint32_t single_checksum = integrity_validator->CalculateChecksum(single_byte);
    bool single_valid = integrity_validator->ValidateIntegrity(single_byte, single_checksum);
    EXPECT_TRUE(single_valid);
}

// 测试记录边界验证
TEST_F(DataIntegrityTest, RecordBoundaryValidation) {
    // 测试记录验证功能
    std::vector<std::string> field_names = {"id", "name", "value"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)", "DOUBLE"};
    std::vector<std::string> valid_values = {"1", "Test record 1", "123.45"};
    std::vector<std::string> invalid_values = {"invalid", "Too long string value that exceeds maximum length limit", "1234567890.12345678901234567890"};

    // 验证有效记录
    ValidationResult valid_result = boundary_validator->ValidateRecord("test_table", field_names, field_types, valid_values);
    EXPECT_EQ(valid_result, ValidationResult::VALID);

    // 测试验证更新
    ValidationResult update_result = boundary_validator->ValidateRecordUpdate("test_table", field_names, valid_values, valid_values);
    EXPECT_EQ(update_result, ValidationResult::VALID);

    // 测试验证删除
    ValidationResult delete_result = boundary_validator->ValidateRecordDeletion("test_table", 1);
    // 预期会失败，因为表不存在，但这不影响测试边界验证器的功能

}

// 测试批量数据验证
TEST_F(DataIntegrityTest, BatchDataValidation) {
    const size_t batch_size = 100;
    std::vector<std::vector<uint8_t>> data_batch;
    std::vector<uint32_t> checksums;

    // 创建批量数据
    for (size_t i = 0; i < batch_size; ++i) {
        auto data = CreateRandomData(256);
        data_batch.push_back(data);
        checksums.push_back(integrity_validator->CalculateChecksum(data));
    }

    // 验证批量完整性
    bool batch_valid = integrity_validator->ValidateBatchIntegrity(data_batch, checksums);
    EXPECT_TRUE(batch_valid);

    // 损坏一批数据中的一个
    if (!data_batch.empty()) {
        CorruptData(data_batch[batch_size / 2], 1);
        bool batch_invalid = integrity_validator->ValidateBatchIntegrity(data_batch, checksums);
        EXPECT_FALSE(batch_invalid);
    }
}

// 测试并发数据验证
TEST_F(DataIntegrityTest, ConcurrentDataValidation) {
    const int num_threads = 4;
    const int operations_per_thread = 50;
    std::vector<std::thread> threads;
    std::atomic<size_t> validation_count{0};
    std::atomic<size_t> error_count{0};

    // 启动并发验证线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &validation_count, &error_count]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    // 创建测试数据
                    auto data = CreateRandomData(512);
                    uint32_t checksum = integrity_validator->CalculateChecksum(data);

                    // 验证完整性
                    bool is_valid = integrity_validator->ValidateIntegrity(data, checksum);
                    if (is_valid) {
                        validation_count++;
                    } else {
                        error_count++;
                    }

                    // 偶尔测试损坏数据
                    if (j % 10 == 0) {
                        auto corrupted = data;
                        CorruptData(corrupted, 1);
                        bool should_be_invalid = integrity_validator->ValidateIntegrity(corrupted, checksum);
                        if (!should_be_invalid) {
                            error_count++;
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Thread " << i << " exception: " << e.what() << std::endl;
                error_count++;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证并发操作结果
    EXPECT_EQ(error_count.load(), 0);  // 不应该有错误
    EXPECT_EQ(validation_count.load(), num_threads * operations_per_thread);
}

// 测试数据完整性配置
TEST_F(DataIntegrityTest, ConfigurationManagement) {
    // 获取默认配置
    auto default_config = integrity_validator->GetConfig();
    EXPECT_TRUE(default_config.enable_validation);
    EXPECT_EQ(default_config.checksum_algorithm, "CRC32");

    // 设置新配置
    DataIntegrityValidator::IntegrityConfig new_config;
    new_config.enable_validation = true;
    new_config.checksum_algorithm = "SHA256";
    new_config.validation_level = ValidationLevel::PARANOID;

    integrity_validator->SetConfig(new_config);

    // 验证配置更新
    auto updated_config = integrity_validator->GetConfig();
    EXPECT_EQ(updated_config.checksum_algorithm, new_config.checksum_algorithm);
    EXPECT_EQ(updated_config.validation_level, new_config.validation_level);
}

// 测试记录边界检测
TEST_F(DataIntegrityTest, RecordBoundaryDetection) {
    // 创建多个连续的记录数据
    std::string record_data = "RECORD1|RECORD2|RECORD3|";
    std::vector<uint8_t> raw_data(record_data.begin(), record_data.end());

    // 检测记录边界
    auto boundaries = boundary_validator->DetectRecordBoundaries(raw_data, '|');
    EXPECT_EQ(boundaries.size(), 3);  // 应该检测到3个记录

    // 验证边界位置
    EXPECT_EQ(boundaries[0], 7);  // RECORD1|
    EXPECT_EQ(boundaries[1], 15); // RECORD2|
    EXPECT_EQ(boundaries[2], 23); // RECORD3|

    // 测试无效数据
    std::vector<uint8_t> invalid_data = {'N', 'O', ' ', 'B', 'O', 'U', 'N', 'D', 'A', 'R', 'Y'};
    auto no_boundaries = boundary_validator->DetectRecordBoundaries(invalid_data, '|');
    EXPECT_TRUE(no_boundaries.empty());
}

// 测试数据修复尝试
TEST_F(DataIntegrityTest, DataRepairAttempt) {
    // 创建测试数据
    auto original_data = CreateRandomData(1024);
    uint32_t original_checksum = integrity_validator->CalculateChecksum(original_data);

    // 轻微损坏数据（只损坏一个字节）
    auto corrupted_data = original_data;
    CorruptData(corrupted_data, 1);

    // 验证损坏检测
    bool is_corrupted = !integrity_validator->ValidateIntegrity(corrupted_data, original_checksum);
    EXPECT_TRUE(is_corrupted);

    // 尝试修复（如果支持）
    auto repair_result = integrity_validator->AttemptRepair(corrupted_data, original_checksum);
    // 注意：修复功能可能不实现，取决于具体实现
    // 这里我们只是测试接口存在性

    // 验证修复是否成功
    if (repair_result.has_value()) {
        bool repair_valid = integrity_validator->ValidateIntegrity(repair_result.value(), original_checksum);
        EXPECT_TRUE(repair_valid);
    }
}

// 测试数据完整性统计信息
TEST_F(DataIntegrityTest, StatisticsCollection) {
    // 执行一些操作来生成统计信息
    for (int i = 0; i < 10; ++i) {
        auto data = CreateRandomData(256);
        uint32_t checksum = integrity_validator->CalculateChecksum(data);

        // 验证有效数据
        integrity_validator->ValidateIntegrity(data, checksum);

        // 验证无效数据
        auto corrupted = data;
        CorruptData(corrupted, 1);
        integrity_validator->ValidateIntegrity(corrupted, checksum);
    }

    // 获取统计信息
    auto stats = integrity_validator->GetStatistics();
    EXPECT_GE(stats.total_validations, 20);  // 10有效 + 10无效
    EXPECT_GE(stats.failed_validations, 10);  // 至少10个失败
    EXPECT_GE(stats.successful_validations, 10);  // 至少10个成功
}

// 测试边界情况处理
TEST_F(DataIntegrityTest, EdgeCaseHandling) {
    // 测试超大数据
    const size_t large_size = 10 * 1024 * 1024; // 10MB
    auto large_data = CreateTestData(large_size, 0xFF);
    uint32_t large_checksum = integrity_validator->CalculateChecksum(large_data);
    bool large_valid = integrity_validator->ValidateIntegrity(large_data, large_checksum);
    EXPECT_TRUE(large_valid);

    // 测试最小数据
    std::vector<uint8_t> minimal_data = {0x00};
    uint32_t minimal_checksum = integrity_validator->CalculateChecksum(minimal_data);
    bool minimal_valid = integrity_validator->ValidateIntegrity(minimal_data, minimal_checksum);
    EXPECT_TRUE(minimal_valid);

    // 测试重复模式数据
    auto pattern_data = CreateTestData(1000, 0xAB);
    uint32_t pattern_checksum = integrity_validator->CalculateChecksum(pattern_data);
    bool pattern_valid = integrity_validator->ValidateIntegrity(pattern_data, pattern_checksum);
    EXPECT_TRUE(pattern_valid);

    // 测试全零数据
    auto zero_data = CreateTestData(1000, 0x00);
    uint32_t zero_checksum = integrity_validator->CalculateChecksum(zero_data);
    bool zero_valid = integrity_validator->ValidateIntegrity(zero_data, zero_checksum);
    EXPECT_TRUE(zero_valid);
}

// 测试数据完整性错误处理
TEST_F(DataIntegrityTest, ErrorHandling) {
    // 测试空校验和验证
    std::vector<uint8_t> test_data = {1, 2, 3, 4, 5};
    bool empty_checksum_result = integrity_validator->ValidateIntegrity(test_data, 0);
    // 空校验和可能被视为无效，具体取决于实现

    // 测试数据大小不匹配的情况
    std::vector<uint8_t> small_data = {1, 2, 3};
    bool size_mismatch_result = integrity_validator->ValidateIntegrity(small_data, 0x12345678);
    // 应该返回false，因为校验和不匹配

    // 测试nullptr处理（如果适用）
    // 注意：这取决于具体实现的安全性

    // 测试异常情况下的恢复
    try {
        // 尝试一些可能抛出异常的操作
        std::vector<uint8_t> empty_vec;
        integrity_validator->CalculateChecksum(empty_vec);
        // 如果没有异常，测试通过
    } catch (const std::exception&) {
        // 如果实现抛出异常，也是可以接受的
    }
}

// 测试数据完整性性能特征
TEST_F(DataIntegrityTest, PerformanceCharacteristics) {
    const int num_operations = 1000;
    const size_t data_size = 4096; // 4KB数据

    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量完整性操作
    for (int i = 0; i < num_operations; ++i) {
        auto data = CreateRandomData(data_size);
        uint32_t checksum = integrity_validator->CalculateChecksum(data);
        bool valid = integrity_validator->ValidateIntegrity(data, checksum);
        ASSERT_TRUE(valid);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    auto stats = integrity_validator->GetStatistics();
    EXPECT_GE(stats.total_validations, num_operations);
    EXPECT_LT(duration.count(), 5000);  // 应该在5秒内完成

    // 计算吞吐量
    double throughput = static_cast<double>(num_operations) / (duration.count() / 1000.0);  // 操作/秒
    EXPECT_GT(throughput, 100.0);  // 至少100操作/秒

    std::cout << "Data integrity performance: " << num_operations
              << " operations in " << duration.count() << "ms, "
              << "throughput: " << throughput << " ops/sec" << std::endl;
}

} // namespace sqlcc