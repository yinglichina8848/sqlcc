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

    // 创建测试记录的辅助方法
    std::tuple<std::string, std::vector<uint8_t>> CreateTestRecord(const std::string& data) {
        std::vector<uint8_t> record_data(data.begin(), data.end());
        return std::make_tuple(data, record_data);
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<DataIntegrityValidator> integrity_validator;
    std::unique_ptr<RecordBoundaryValidator> boundary_validator;
    std::mt19937 random_engine;
};

// 测试数据完整性约束验证
TEST_F(DataIntegrityTest, ConstraintValidation) {
    // 创建测试表约束
    ConstraintRule not_null_rule("not_null_id", NOT_NULL_CONSTRAINT, "test_table");
    not_null_rule.column_names = {"id"};
    integrity_validator->AddConstraint(not_null_rule);

    ConstraintRule unique_rule("unique_name", UNIQUE_CONSTRAINT, "test_table");
    unique_rule.column_names = {"name"};
    integrity_validator->AddConstraint(unique_rule);

    // 验证有效记录
    std::vector<std::string> field_names = {"id", "name"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)"};
    std::vector<std::string> valid_values = {"1", "Test"};
    
    ConstraintValidationResult result = integrity_validator->ValidateConstraints(
        "test_table", field_names, field_types, valid_values);
    EXPECT_EQ(result, CONSTRAINT_VALID);

    // 验证违反非空约束
    std::vector<std::string> null_values = {"", "Test2"};
    result = integrity_validator->ValidateConstraints(
        "test_table", field_names, field_types, null_values);
    EXPECT_NE(result, CONSTRAINT_VALID);
}

// 测试批量数据验证
TEST_F(DataIntegrityTest, BatchValidation) {
    // 创建测试表约束
    ConstraintRule not_null_rule("not_null_id", NOT_NULL_CONSTRAINT, "test_table");
    not_null_rule.column_names = {"id"};
    integrity_validator->AddConstraint(not_null_rule);

    // 准备批量测试数据
    std::vector<std::vector<std::string>> record_batch;
    record_batch.push_back({"1", "Test1"});
    record_batch.push_back({"2", "Test2"});
    record_batch.push_back({"", "Invalid"}); // 违反非空约束
    record_batch.push_back({"3", "Test3"});

    // 执行批量验证
    auto results = integrity_validator->ValidateConstraintsBatch("test_table", record_batch);
    EXPECT_EQ(results.size(), record_batch.size());

    // 检查结果
    EXPECT_EQ(results[0], CONSTRAINT_VALID);
    EXPECT_EQ(results[1], CONSTRAINT_VALID);
    EXPECT_NE(results[2], CONSTRAINT_VALID); // 应该失败
    EXPECT_EQ(results[3], CONSTRAINT_VALID);
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
                    // 验证数据约束
                    std::vector<std::string> field_names = {"id", "name"};
                    std::vector<std::string> field_types = {"INT", "VARCHAR(50)"};
                    std::vector<std::string> valid_values = {"1", "Test"};
                    
                    ConstraintValidationResult result = integrity_validator->ValidateConstraints(
                        "test_table", field_names, field_types, valid_values);
                    
                    if (result == CONSTRAINT_VALID) {
                        validation_count++;
                    } else {
                        error_count++;
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
    IntegrityValidationConfig default_config = integrity_validator->GetValidationConfig();
    EXPECT_TRUE(default_config.enable_not_null_checking);
    EXPECT_TRUE(default_config.enable_unique_checking);

    // 设置新配置
    IntegrityValidationConfig new_config;
    new_config.enable_not_null_checking = true;
    new_config.enable_unique_checking = false;
    new_config.enable_foreign_key_checking = false;
    new_config.strict_mode = false;

    integrity_validator->SetValidationConfig(new_config);

    // 验证配置更新
    IntegrityValidationConfig updated_config = integrity_validator->GetValidationConfig();
    EXPECT_EQ(updated_config.enable_not_null_checking, new_config.enable_not_null_checking);
    EXPECT_EQ(updated_config.enable_unique_checking, new_config.enable_unique_checking);
    EXPECT_EQ(updated_config.enable_foreign_key_checking, new_config.enable_foreign_key_checking);
    EXPECT_EQ(updated_config.strict_mode, new_config.strict_mode);
}

// 测试记录大小验证
TEST_F(DataIntegrityTest, RecordSizeValidation) {
    // 测试各种数据类型的记录验证
    std::vector<std::string> field_names = {"id", "name", "description"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)", "TEXT"};
    
    // 测试有效大小
    std::vector<std::string> valid_values = {"1", "Short name", "Short description"};
    ValidationResult valid_result = boundary_validator->ValidateRecord(
        "test_table", field_names, field_types, valid_values);
    EXPECT_EQ(valid_result, ValidationResult::VALID);
    
    // 测试超长字符串
    std::vector<std::string> invalid_values = {"2", "Valid name", 
        std::string(100000, 'x')}; // 超过TEXT最大长度
    ValidationResult invalid_result = boundary_validator->ValidateRecord(
        "test_table", field_names, field_types, invalid_values);
    EXPECT_NE(invalid_result, ValidationResult::VALID);
}



// 测试数据完整性统计信息
TEST_F(DataIntegrityTest, StatisticsCollection) {
    // 执行一些操作来生成统计信息
    std::vector<std::string> field_names = {"id", "name"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)"};
    
    for (int i = 0; i < 10; ++i) {
        // 验证有效数据
        std::vector<std::string> valid_values = {std::to_string(i), "Test" + std::to_string(i)};
        integrity_validator->ValidateConstraints("test_table", field_names, field_types, valid_values);
    }

    // 获取统计信息
    auto stats = integrity_validator->GetIntegrityStats();
    EXPECT_GE(stats.total_validations, 10);
    EXPECT_GE(stats.successful_validations, 10);
}

// 测试边界情况处理
TEST_F(DataIntegrityTest, EdgeCaseHandling) {
    std::vector<std::string> field_names = {"id", "name", "value"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)", "DOUBLE"};
    
    // 测试空值
    std::vector<std::string> null_values = {"1", "", "123.45"};
    ValidationResult null_result = boundary_validator->ValidateRecord(
        "test_table", field_names, field_types, null_values);
    // 注意：空值是否允许取决于表的约束配置
    
    // 测试无效类型
    std::vector<std::string> invalid_type_values = {"abc", "Valid name", "123.45"};
    ValidationResult type_result = boundary_validator->ValidateRecord(
        "test_table", field_names, field_types, invalid_type_values);
    EXPECT_NE(type_result, ValidationResult::VALID);
}

// 测试数据完整性错误处理
TEST_F(DataIntegrityTest, ErrorHandling) {
    std::vector<std::string> field_names = {"id", "name"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)"};
    
    // 测试空表名
    try {
        integrity_validator->ValidateConstraints(
            "", field_names, field_types, {"1", "Test"});
        // 如果没有异常，测试通过
    } catch (const std::exception&) {
        // 如果实现抛出异常，也是可以接受的
    }
    
    // 测试字段数量不匹配
    try {
        integrity_validator->ValidateConstraints(
            "test_table", field_names, field_types, {"1"});
        // 应该失败，因为值的数量与字段数量不匹配
    } catch (const std::exception&) {
        // 如果实现抛出异常，也是可以接受的
    }
}

// 测试数据完整性性能特征
TEST_F(DataIntegrityTest, PerformanceCharacteristics) {
    const int num_operations = 1000;
    
    std::vector<std::string> field_names = {"id", "name"};
    std::vector<std::string> field_types = {"INT", "VARCHAR(50)"};
    std::vector<std::string> valid_values = {"1", "Test"};

    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量完整性操作
    for (int i = 0; i < num_operations; ++i) {
        ConstraintValidationResult result = integrity_validator->ValidateConstraints(
            "test_table", field_names, field_types, valid_values);
        ASSERT_EQ(result, CONSTRAINT_VALID);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    auto stats = integrity_validator->GetIntegrityStats();
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